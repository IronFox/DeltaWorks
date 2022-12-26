#include "cli_defaults.h"

/******************************************************************

E:\include\string\cli_defaults.cpp

******************************************************************/

namespace DeltaWorks
{

	namespace CLI
	{
	
		static void Describe(const PParser&parser, const CLI::Variable&v, bool more)
		{
			String msg = "'"+v.name+"' ("+v.type;
			if (v.components != 1)
				msg += "["+String(v.components)+"]";
			msg += ") ";
			//println(msg);
			if (v.protection != NoProtection)
			{
				if ( (v.protection & EraseProtection) && (v.protection & WriteProtection))
					msg += "is fully protected (You may neither modify nor unset this variable)";
				elif (v.protection & EraseProtection)
					msg += "may be modified but not unset";
				else
					msg += "may be unset but not modified";
			}
			else
				msg += "may be modified and/or unset as required";
			String supported = v.GetSupportedValues();
			if (supported.IsNotEmpty())
			{
				msg += '.';
				if ((v.protection & WriteProtection) == 0)
					msg += "\nSupported values are "+supported;
				else
					msg += "\nPossible values are "+supported;
				const auto def = v.GetDefault();
				if (def.IsNotEmpty())
					msg += ". Default is '"+def+"'";
			}
			if (more)
				msg += '.';
			parser->PrintLine(msg);
		}

		static void	PrintHelp(const PParser&parser, const PCommand&, Command::ArgumentList arguments)
		{
			StringRef parameter;
			if (arguments.Count() > 1)
				parameter = arguments[1];

			if (parameter.IsEmpty())
				parameter = "help";
		
			{
				String message;
				if (PVariable var = parser->FindVar(parameter))
				{
					Describe(parser,*var,true);
					String supported = var->GetSupportedValues();
					if (var->help)
						message = *var->help;
					else
					{
						message = "No additional information available :/";
					}
				}
				elif (PCommand cmd = parser->Find(parameter))
				{
					if (cmd->help)
						message = cmd->fullSpecification+":\n"+*cmd->help;
					else
						message = "No additional information available for command '"+cmd->fullSpecification+"' :/";
				}
				elif (PFolder folder = parser->FindFolder(parameter))
				{
					if (folder->help)
						message = folder->name+":\n"+*folder->help;
					else
						message = "No additional information available for folder '"+folder->name+"' :/";
				}
			
				if (message.IsEmpty())
				{
					message ="Unable to find requested entry '"+parameter+"' :/";
				}
				parser->PrintLine(message);
			}
		}


		static void	PrintCommands(const PParser&parser, const CLI::ItemTable<CLI::Command>&commands, const String&appendix)
		{
			PointerTable<bool>	listed_commands;

			for (index_t i = 0; i < commands.Count(); i++)
			{
				CLI::PCommand cmd = commands[i];
				if (!listed_commands.IsSet(cmd.get()))
				{
					PreAllocVector<CLI::PCommand>	temp;
					temp.append(cmd);
					for (index_t j = i+1; j < commands.Count(); j++)
						if (commands[j] == cmd)
							temp.append(commands[j]);
					String line = " *";
					for (index_t j = 0; j < temp.Count()-1; j++)
						line+=temp[j]->name+"/";
					line+=temp.Last()->fullSpecification+appendix;

					parser->PrintLine(line);
					listed_commands.Set(cmd.get(),true);
				}
			}
		}

		static void 	List(const PParser&parser, const PCommand&, Command::ArgumentList arguments)
		{
			StringRef f;
			if (arguments.Count() > 1)
				f = arguments[1];

			CLI::PFolder folder = f.IsNotEmpty() ? parser->FindFolder(f) : parser->GetFocus();
			if (!folder)
			{
				parser->FailCommandExecution("'"+f+"' is not a folder");
				return;
			}
		
			//println(" ["+folder->path()+"]:");
			parser->PrintLine("-");
			for (index_t i = 0; i < folder->variables.Count(); i++)
				parser->PrintLine(" "+folder->variables[i]->name);
			if (folder->variables.Count())
				parser->PrintLine("-");
		
			PrintCommands(parser,folder->commands,"");
			if (folder->parent.expired())
				PrintCommands(parser,parser->GetGlobalCommands()," (global)");
			if (folder->commands.Count())
				parser->PrintLine("-");
			if (folder->folders.Count() < 6)
				for (index_t i = 0; i < folder->folders.Count(); i++)
					parser->PrintLine(" ["+folder->folders[i]->name+"]");
			else
			{
				count_t groups = folder->folders.Count()/3;
				for (index_t i = 0; i < groups; i++)
					parser->PrintLine(" ["+folder->folders[i*3]->name+"] ["+folder->folders[i*3+1]->name+"] ["+folder->folders[i*3+2]->name+"]");
				if (folder->folders.Count()%3)
					if (groups*3+1 == folder->folders.Count())
						parser->PrintLine(" ["+folder->folders[groups*3]->name+"]");
					else
						parser->PrintLine(" ["+folder->folders[groups*3]->name+"] ["+folder->folders[groups*3+1]->name+"]");
			}
			if (folder->folders.Count())
				parser->PrintLine("-");
		}

		static void	ChangeDirectory(const PParser&parser, const PCommand&, CLI::Command::ArgumentList arguments)
		{
			if (arguments.Count() != 2)	//command+argument
			{
				parser->FailCommandExecution("Must specify exactly one folder to change to");
				return;
			}
			parser->MoveFocus(arguments[1]);
		}


		static bool UpdateComponent(const PParser&parser, const StringRef&variable, const StringRef&value, bool echoSetOperation)
		{
			const index_t lastFolderSlash = variable.FindLast('/');

			const index_t p = variable.FindLast('.');
			if (p == InvalidIndex || (lastFolderSlash != InvalidIndex && lastFolderSlash > p))
				return false;

			const auto var = variable.SubStringRef(0,p);
			const auto component = variable.SubStringRef(p+1);
			CLI::PVariable v = parser->FindVar(var);
			if (!v)
			{
				parser->FailCommandExecution("Unknown variable: '"+var+"'. Variable must be set for component access");
				return true;
			}
			if (v->IsWriteProtected())
			{
				parser->FailCommandExecution("Variable '"+var+"' is write-protected");
				return true;
			}
			if (!v->Set(component,value))
			{
				parser->FailCommandExecution("Failed to update '"+variable+"'");
				return true;
			}
			if (echoSetOperation)
				parser->PrintLine(v->name+" set to "+v->ConvertToString());
			return true;
		}
		

		template <bool EchoSetOperation>
		static void	Set(const PParser&parser, const PCommand&, Command::ArgumentList arguments)
		{
			if (arguments.Count() != 3)	//command+name+value
			{
				parser->FailCommandExecution("Must specify name and value to update");
				return;
			}
			if (UpdateComponent(parser,arguments[1],arguments[2],EchoSetOperation))
				return;
			CLI::PVariable v;
			if (!(v=parser->Set(arguments[1],arguments[2])))
				return;

			if (EchoSetOperation)
				parser->PrintLine(v->name+" set to "+v->ConvertToString());
		}

		static void	VariableCallBack(const PParser&parser, const PFolder&containingFolder, const CLI::PVariable&v)
		{
			auto ctx = parser->GetFocus()->GetPath();
			auto fld = containingFolder->GetPath();

			String msg;

			if (ctx.IsEmpty())
			{
				if (fld.IsEmpty())
					msg = "/"+v->name+": "+v->ConvertToString();
				else
					msg = "/"+fld+"/"+v->name+": "+v->ConvertToString();
			}
			else
				msg = v->name+": "+v->ConvertToString();	//don't care
			parser->PrintLine(msg);
		}

		static void	ShowTypeof(const PParser&parser, const PCommand&, Command::ArgumentList arguments)
		{
			if (arguments.Count() != 2)	//command+name
			{
				parser->FailCommandExecution("Must specify name to get type of variable");
				return;
			}
			CLI::PVariable v = parser->FindVar(arguments[1]);
			if (!v)
			{
				if (parser->FindFolder(arguments[1]))
					parser->PrintLine("'"+arguments[1]+"' is of type folder");
				else
					parser->FailCommandExecution("Undefined variable: '"+arguments[1]+"'");
				return;
			}
			Describe(parser,*v,false);
		}

		static void	Unset(const PParser&parser, const PCommand&, Command::ArgumentList arguments)
		{
			if (arguments.Count() != 2)	//command+name
			{
				parser->FailCommandExecution("Must specify name to unset variable");
				return;
			}
			parser->Unset(arguments[1]);
		}

		typedef std::shared_ptr<const String>	PString;
	
		static PString	list_help = PString(new String("Displays the content of the current folder.\nCommands are displayed with a leading asterisk (e.g. '*command'), folders in brackets (e.g. '[folder]'), and variables just plain (e.g. 'variable').")),
								general_help = PString(new String("Enter 'ls' or 'list' to list all available commands.\nAdditionally, 'help <variable/command>' will query the respective help entry.")),
								set_help = PString(new String("Registers a new variable or updates the value of an existing one.\nVariables may be read-only preventing you from changing their value, others may execute arbitrary code upon modification.")),
								unset_help = PString(new String("Unregisters a variable. Variables may be protected against this kind of operation.")),
								cd_help = PString(new String("Changes the active folder. Use 'cd <folder>' to change into a specific folder.\nEntering 'cd ..' will change into the respective parent folder and 'cd /' will change the active directory to the root folder. You may also specify an entire path (e.g. 'cd ../path0/subpath/../othersubpath'). The respective target folder(s) must exist for this operation to be successful.")),
								typeof_help = PString(new String("Queries the type and protection of a variable.\nVariable types may vary from primitive types like integers, booleans, strings, or floats to very complex ones."));
							
							
	
		void			InitDefaults(const PState&state, bool echoSetOperation)
		{
			
			state->onVariableCall = VariableCallBack;

			const count_t state0 = state->CountLinkedParsers();
			{
				auto parser = state->NewParser();
		
				PCommand help = parser->DefineGlobalCommand("help command/variable/folder",PrintHelp,CLI::CommandCompletion);
				parser->DefineGlobalCommand("ls [folder]",List,CLI::FolderCompletion)->help = list_help;
				parser->DefineGlobalCommand("list [folder]",List,CLI::FolderCompletion)->help = list_help;
				//parser.defineGlobalCommand("list",list);

				parser->DefineGlobalCommand("set variable value", echoSetOperation ? Set<true> : Set<false>,CLI::VariableCompletion)->help = set_help;
				parser->DefineGlobalCommand("unset variable",Unset,CLI::VariableCompletion)->help = unset_help;
				parser->DefineGlobalCommand("cd folder",ChangeDirectory,CLI::FolderCompletion)->help = cd_help;
				parser->DefineGlobalCommand("typeof variable",ShowTypeof,CLI::VariableCompletion)->help = typeof_help;
		
				help->help = general_help;

				ASSERT_EQUAL__(state0+1,state->CountLinkedParsers());
			}
			ASSERT_EQUAL__(state0,state->CountLinkedParsers());
		}

	}
}
