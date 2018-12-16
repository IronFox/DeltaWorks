#include	"../global_root.h"
#include	"cli.h"

namespace DeltaWorks
{


	namespace CLI
	{
		static inline bool isUmlaut(char c)
		{
			return c == 'ä' || c == 'ü' || c == 'ö'
					||
					c == 'Ä' || c == 'Ü' || c == 'Ö'
					||
					c == 'ß';
		}

		static inline bool isAllowedCharacter(char c)
		{
			return (isUmlaut(c) || isalnum(c) || c == '_') && (c != '\n' && c != '\r' && c != ' ');
		}

		String	Quote(const String&source)
		{
			String rs = source;
			rs.FindAndReplace('\'',"\\'");
			return '\''+ rs + '\'';
		}


		namespace detail
		{
			void	makeValidName(String&name)
			{
				for (index_t i = 0; i < name.GetLength(); i++)
					if (!isAllowedCharacter(name.GetChar(i)))
						name.Set(i,'_');
			}
		}

		static Command::Argument Get(const Command::ArgumentList&list, index_t at)
		{
			if (at >= list.Count())
				return Command::Argument();
			return list[at];
		}


		Command::Command(const Name&name):callback([](const PParser&parser,const PCommand&command, ArgumentList arguments) {}),
							NamedItem(name.coreName), fullSpecification(name.fullSpecification)
		{
			construct(NoCompletion);
		}

		Command::Command(const Name&name, const full_function_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback(f)
		{
			construct(completion);
		}
		Command::Command(const Name&name, const function0_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f();})
		{
			construct(completion);

		}
		Command::Command(const Name&name, const function1_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const function2_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const function3_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const function4_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3),Get(arguments,4));})

		{
			construct(completion);
		}

		Command::Command(const Name&name, const function5_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3),Get(arguments,4),Get(arguments,5));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const function6_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3),Get(arguments,4),Get(arguments,5),Get(arguments,6));})

		{
			construct(completion);
		}


		Command::Command(const Name&name, const full_func_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback(f)
		{
			construct(completion);
		}
		Command::Command(const Name&name, const func0_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f();})
		{
			construct(completion);
		}
		Command::Command(const Name&name, const func1_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const func2_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const func3_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const func4_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3),Get(arguments,4));})

		{
			construct(completion);
		}

		Command::Command(const Name&name, const func5_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3),Get(arguments,4),Get(arguments,5));})

		{
			construct(completion);
		}
		Command::Command(const Name&name, const func6_t&f, eCommandCompletion completion):
						NamedItem(name.coreName), fullSpecification(name.fullSpecification),
						callback([f](const PParser&parser,const PCommand&command, ArgumentList arguments){f(Get(arguments,1),Get(arguments,2),Get(arguments,3),Get(arguments,4),Get(arguments,5),Get(arguments,6));})

		{
			construct(completion);
		}


		void Command::construct(eCommandCompletion completion)
		{
			this->completion = completion;
		}

		void		Command::Invoke(const PParser&parser,ArgumentList arguments)
		{
			callback(parser,shared_from_this(),arguments);
		}


		Variable::Variable(const String&name_, const String&type_, count_t components_, UINT32 protection_):NamedItem(name_),type(type_),components(components_),protection(protection_)
		{}



		PParser						State::NewParser()
		{
			return PParser(new Parser(shared_from_this()));
		}


		void Parser::_Setup()
		{
			lookup.pathConfig.main_separator="/";
			lookup.segmentConfig.quotations = "\"'";
			lookup.segmentConfig.main_separator = " ";
			lookup.segmentConfig.quotation_escape_character = '\\';
			lookup.segmentConfig.recursion_break = false;
			lookup.lineConfig.quotations = "\"'";
			lookup.lineConfig.main_separator = "\n;";
			lookup.lineConfig.quotation_escape_character = '\\';
			lookup.lineConfig.recursion_break = false;
		}
	

		Parser::Parser()
		{
			_Setup();
		}

		Parser::Parser(const PState&st)
		{
			_Setup();
			LinkTo(st);
		}



		void	Parser::OnFolderDeleted(const PFolder&deletedFolder)
		{
			if (focused->IsChildOf(deletedFolder))
			{
				focused = deletedFolder->parent.lock();
				if (onFocusChanged)
					onFocusChanged();
			}
		}


		void	Parser::PrintLine(const String&line) const
		{
			if (onPrintLine)
				onPrintLine(line);
		}
		
		void	Parser::LinkTo(const PState&newState)
		{
			if (state == newState)
				return;
			if (state)
				state->UnlinkParser(this);
			newState->LinkParser(this);
			state = newState;
			focused = newState->GetRoot();
			if (onFocusChanged)
				onFocusChanged();
		}
	
		bool Command::Name::Parse(const StringRef&def)
		{
			StringList	items(def);
			if (!items)
				return false;
			detail::makeValidName(items.First());
			fullSpecification = items.First();
			if (items > 1)
				fullSpecification += " <"+items.fuse(1,"> <")+">";
		
			coreName = items.First();
			return true;
		}
	


		bool	Parser::InsertCommand(const PCommand&cmd, const StringRef&targetPath)
		{
			if (!cmd)
				return false;
			PFolder folder = FindFolder(targetPath);
			if (!folder)
				return false;

			PCommand result = folder->commands.Query(cmd->name);
			if (result)
				return false;
			folder->commands.Set(cmd->name, cmd);
			return true;
		}

		/**/						State::State():root(new Folder(PFolder(),""))
		{}

	
		bool	State::InsertGlobalCommand(const PCommand&cmd)
		{
			if (!cmd)
				return false;
			PCommand result = globalCommands.Query(cmd->name);
			if (result)
				return false;
			globalCommands.Set(cmd->name, cmd);
			return true;
		}
	

		bool	Parser::_EntryLookup(const StringRef&name, bool may_exist)
		{
			error = "";
			lookup.folder.reset();
			lookup.variable.reset();
			lookup.command.reset();
			Tokenizer::Tokenize(name,lookup.pathConfig,lookup.pathSegments);
			if (lookup.pathSegments.IsEmpty())
			{
				error = "Path empty";
				return false;
			}
			String entryName = lookup.pathSegments.Last();
			detail::makeValidName(entryName);
			lookup.folder = _Resolve(name.BeginsWith('/'));
			if (!lookup.folder)
			{
				error = "Unable to _Resolve parent folder";
				return false;
			}
			lookup.variable = lookup.folder->variables.Query(entryName);
			lookup.command = lookup.folder->commands.Query(entryName);
			if (!may_exist)
				if (lookup.variable)
				{
					error = "Variable '"+entryName+"' already defined";
					return false;
				}
				elif (lookup.command)
				{
					error = "Command '"+entryName+"' already defined";
					return false;
			
				}
			return true;
			
		}

		PVariable	Parser::SetString(const StringRef&name, unsigned protection)
		{
			if (!_EntryLookup(name,false))
				return nullptr;
			ASSERT_IS_NULL__(lookup.variable);
			ASSERT_IS_NULL__(lookup.command);
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(), new StringVariable(lookup.pathSegments.Last(),"",protection));
		
		}

		PVariable	Parser::SetString(const StringRef& name, const String& value, unsigned protection, bool allow_overwrite)
		{
			if (!_EntryLookup(name,allow_overwrite))
				return nullptr;

			if (lookup.command)
			{
				error = "A command of that name ('"+lookup.pathSegments.Last()+"') already exists";
				return nullptr;
			}
		
			if (!lookup.variable)
			{
				return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new StringVariable(lookup.pathSegments.Last(),value,protection));
			}
			else
			{
				return lookup.variable->Set(value)?lookup.variable:PVariable();
			}
		}


		PVariable	Parser::SetFloat(const StringRef& name, unsigned protection)
		{
			if (!_EntryLookup(name,false))
				return PVariable();
			ASSERT__(lookup.variable);
			ASSERT__(lookup.command);
		
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new FloatVariable(lookup.pathSegments.Last(),0,protection));
		}

		PVariable	Parser::SetFloat(const StringRef& name, float value, unsigned protection)
		{
			if (!_EntryLookup(name,false))
				return PVariable();
			ASSERT_IS_NULL__(lookup.variable);
			ASSERT_IS_NULL__(lookup.command);
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new FloatVariable(lookup.pathSegments.Last(),value,protection));
		}

	
		PVariable	Parser::SetInt(const StringRef& name, unsigned protection)
		{
	
			if (!_EntryLookup(name,false))
				return PVariable();
			ASSERT_IS_NULL__(lookup.variable);
			ASSERT_IS_NULL__(lookup.command);
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new IntVariable(lookup.pathSegments.Last(),0,protection));
		}

		PVariable	Parser::SetInt(const StringRef& name, int value, unsigned protection)
		{
			if (!_EntryLookup(name,false))
				return PVariable();
			ASSERT_IS_NULL__(lookup.variable);
			ASSERT_IS_NULL__(lookup.command);
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new IntVariable(lookup.pathSegments.Last(),value,protection));
		}

		PVariable	Parser::SetBool(const StringRef& name, unsigned protection)
		{
			if (!_EntryLookup(name,false))
				return PVariable();
			ASSERT_IS_NULL__(lookup.variable);
			ASSERT_IS_NULL__(lookup.command);
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new BoolVariable(lookup.pathSegments.Last(),false,protection));
		}

		PVariable	Parser::SetBool(const StringRef& name, bool value, unsigned protection)
		{
			if (!_EntryLookup(name,false))
				return PVariable();
			ASSERT_IS_NULL__(lookup.variable);
			ASSERT_IS_NULL__(lookup.command);
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),new BoolVariable(lookup.pathSegments.Last(),value,protection));
		}

	
	
	

		PVariable	Parser::SetVariable(const PVariable&v)
		{
			return focused->SetVariable(v);
		}


		PFolder		Folder::CreateOrGetChild(const String&folderName)
		{
			for (index_t i = 0; i < folderName.GetLength(); i++)
				if (!isAllowedCharacter(folderName.GetChar(i)))
					throw Except::Program::ParameterFault("Character 0x"+IntToHex((int)folderName.GetChar(i),2)+" ('"+folderName.GetChar(i)+"') of variable '"+folderName+"' not allowed");
			PFolder f = folders.Query(folderName);
			if (f)
				return f;
			f.reset(new Folder(shared_from_this(),folderName));
			folders.Insert(folderName,f);
			return f;
		}

		PVariable	Folder::SetVariable(const PVariable&v)
		{
			for (index_t i = 0; i < v->name.GetLength(); i++)
				if (!isAllowedCharacter(v->name.GetChar(i)))
					throw Except::Program::ParameterFault(CLOCATION,"Character 0x"+IntToHex((int)v->name.GetChar(i),2)+" ('"+v->name.GetChar(i)+"') of variable '"+v->name+"' not allowed");

			if (variables.IsSet(v->name))
				throw Except::Program::UniquenessViolation(CLOCATION,"Variable '"+v->name+"' already defined in context '"+GetPath()+"'");
			variables.Insert(v->name, v);
			return v;
		}
	
	
	
	
	
	
	
	
	
		PVariable	Parser::Set(const StringRef& name, const StringRef& value)
		{
			if (!_EntryLookup(name,true))
				return PVariable();
			
			if (lookup.command || state->IsGlobalCommand(lookup.pathSegments.Last()))
			{
				error = "The requested variable name ('"+lookup.pathSegments.Last()+"') is already assigned to a command";
				return PVariable();
			}
		
			if (lookup.variable)
			{
				if (lookup.variable->IsWriteProtected())
				{
					error = "Variable '"+name+"' is write-protected";
					return	PVariable();
				}
				if (!lookup.variable->Set(value))
				{
					error = "Assignment failed";
					return PVariable();
				}
				return lookup.variable;
			}
			if (!state->setMayCreateVariables)
			{
				error = "Creation of new variables via assignment is not allowed by configuration";
				return PVariable();
			}

			int	ival;
			float vval;
		
		
			if (convertToInt(value.GetPointer(),value.GetLength(),ival))
				return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),(new IntVariable(lookup.pathSegments.Last(),ival)));
		
			if (convertToFloat(value.GetPointer(),value.GetLength(),vval))
				return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),(new FloatVariable(lookup.pathSegments.Last(),vval)));
		
			VectorVariable<float,3>	f3(lookup.pathSegments.Last());
			VectorVariable<float,4>	f4(lookup.pathSegments.Last());
		
			if (f3.Set(value))
				return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),(new VectorVariable<float,3>(f3)));
			if (f4.Set(value))
				return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),(new VectorVariable<float,4>(f4)));
			
			return lookup.folder->variables.InsertNew(lookup.pathSegments.Last(),(new StringVariable(lookup.pathSegments.Last(),value)));
		}

	
	
	
	

		PFolder	Parser::_Resolve(bool from_root)
		{
			error = "";
			if (lookup.pathSegments.IsEmpty())
			{
				error = "Missing argument";
				return nullptr;
			}
		
			PFolder current = from_root?state->GetRoot():focused;

			for	(index_t i = 0; i < lookup.pathSegments.Count()-1; i++)
			{
				if	(lookup.pathSegments[i] == '.')
					continue;
				if	(lookup.pathSegments[i] == "..")
				{
					if	(!current->parent.lock())
					{
						error = "Can't exit folder. Folder is root";
						return nullptr;
					}
					current = current->parent.lock();
				}
				elif (lookup.pathSegments[i] == '~' || lookup.pathSegments[i] == '-')
				{
					if	(!current->parent.lock())
					{
						error = "Can't exit folder. Folder is root";
						return nullptr;
					}
					while (current->parent.lock()->parent.lock())
						current = current->parent.lock();
				}
				else
				{
					PFolder folder = current->folders.Query(lookup.pathSegments[i]);
					if	(!folder)
					{
						error = "Folder does not exist: '"+lookup.pathSegments[i]+"'";
						return nullptr;
					}
					current = folder;
				}
			}
			return current;
		}
	
		PFolder	Parser::_ResolveFull(bool from_root)
		{
			if (lookup.pathSegments.IsEmpty())
			{
				error = "Missing argument";
				return PFolder();
			}
		
			PFolder current = from_root?state->GetRoot():focused;

			for	(index_t i = 0; i < lookup.pathSegments.Count(); i++)
			{
				if	(lookup.pathSegments[i] == '.')
					continue;
				if	(lookup.pathSegments[i] == "..")
				{
					if	(!current->parent.lock())
					{
						error = "Can't exit folder. Folder is root";
						return	PFolder();
					}
					current = current->parent.lock();
				}
				elif (lookup.pathSegments[i] == '~' || lookup.pathSegments[i] == '-')
				{
					if	(!current->parent.lock())
					{
						error = "Can't exit folder. Folder is root";
						return	PFolder();
					}
					while (current->parent.lock()->parent.lock())
						current = current->parent.lock();
				}
				else
				{
					PFolder folder = current->folders.Query(lookup.pathSegments[i]);
					if	(!folder)
					{
						error = "Folder does not exist: '"+lookup.pathSegments[i]+"'";
						return	PFolder();
					}
					current = folder;
				}
			}
			return	current;
		}

		bool	Parser::Unset(const StringRef& name)
		{
			if (!_EntryLookup(name,true))
				return false;
			if (!lookup.variable)
			{
				error = "Variable not defined: '"+lookup.pathSegments.Last()+"'";
				return false;
			}
	
		
		
		
			if (lookup.variable->protection&EraseProtection)
			{
				error = "Variable '"+name+"' is protected";
				return false;
			}
			lookup.folder->variables.Unset(lookup.variable->name);
			return true;
		}


		void	Parser::UnsetIgnoreProtection(const StringRef&name)
		{
			if (!_EntryLookup(name,true) || !lookup.variable)
				return;
		
		
			lookup.folder->variables.Unset(lookup.variable->name);
		}

		bool	Parser::Unset(const PFolder&folder, const PVariable&var, bool ignore_protection)
		{
			if (!var || !folder || !folder->variables.IsSet(var->name))
			{
				error = "Parameters invalid";
				return false;
			}
			if (ignore_protection || !(var->protection&EraseProtection))
			{
				folder->variables.Unset(var->name);
				return true;
			}
			error = "'"+var->name+"' is protected";
			return false;
		}



		unsigned	Parser::GetAsUnsigned(const StringRef& name, unsigned exception)
		{
			PVariable v = FindVar(name);
			if (!v)
				return exception;
			if (v->type == "Integer")
				return (unsigned)((IntVariable*)v.get())->content;
			if (v->type == "Float")
				return (unsigned)((FloatVariable*)v.get())->content;

			unsigned val;
			if (!convert(v->ConvertToString().c_str(),val))
				return exception;
			return val;
		}

		int	Parser::GetAsInt(const StringRef& name, int exception)
		{
			PVariable v = FindVar(name);
			if (!v)
				return exception;
			if (v->type == "Integer")
				return (int)((IntVariable*)v.get())->content;
			if (v->type == "Float")
				return (int)((FloatVariable*)v.get())->content;
			int val;
			if (!convert(v->ConvertToString().c_str(),val))
				return exception;
			return val;
		}

		float	Parser::GetAsFloat(const StringRef& name, float exception)
		{
			PVariable v = FindVar(name);
			if (!v)
				return exception;
			if (v->type == "Integer")
				return ((IntVariable*)v.get())->content;
			if (v->type == "Float")
				return ((FloatVariable*)v.get())->content;
			float val;
			if (!convert(v->ConvertToString().c_str(),val))
				return exception;
			return val;
		}

		Key::Name	Parser::GetAsKey(const StringRef& name, Key::Name exception)
		{
			PVariable v = FindVar(name);
			if (!v)
				return exception;
			Key::Name val;
			if (!convert(v->ConvertToString().c_str(),val))
				return exception;
			return val;
		}

		bool	Parser::GetAsBool(const StringRef& name, bool exception)
		{
			PVariable v = FindVar(name);
			if (!v)
				return exception;
			if (v->type == "Integer")
				return ((IntVariable*)v.get())->content!=0;
			if (v->type == "Float")
				return ((FloatVariable*)v.get())->content!=0;
			bool val;
			if (!convert(v->ConvertToString().c_str(),val))
				return exception;
			return val;
		}


		const String& Parser::GetAsString(const StringRef& name, const String& exception)
		{
			static String content;
			PVariable v = FindVar(name);
			if (!v)
				return exception;
			content = v->ConvertToString();
			return content;
		}

		const	String&	Parser::GetErrorStr()	const
		{
			return	error;
		}

		const	String&	Parser::GetError()	const
		{
			return	error;
		}


		PCommand Parser::Find(const StringRef&name,PFolder*folder_out/*=NULL*/)
		{
			Tokenizer::Tokenize(name,lookup.pathConfig,lookup.pathSegments);
		
			if (lookup.pathSegments.IsEmpty())
				return PCommand();
			PFolder folder = _Resolve(name.BeginsWith('/'));
			if (!folder)
				return PCommand();
			PCommand rs= folder->commands.Query(lookup.pathSegments.Last());
			if (rs)
			{
				if (folder_out)
					(*folder_out) = folder;
				return rs;
			}
			if (lookup.pathSegments.Count()==1)
			{
				if (folder_out)
					(*folder_out) = state->GetRoot();
				return state->GetGlobalCommand(lookup.pathSegments.First());
			}
			return PCommand();
		}

		PCommand Parser::Find(const String&name,PFolder*folder_out/*=NULL*/)
		{
			return Find(name.ToRef(),folder_out);
		}

		PVariable Parser::FindVar(const StringRef&name,PFolder*folder_out/*=NULL*/)
		{
			Tokenizer::Tokenize(name,lookup.pathConfig,lookup.pathSegments);
			if (lookup.pathSegments.IsEmpty())
				return PVariable();
			PFolder folder = _Resolve(name.BeginsWith('/'));
			if (!folder)
				return PVariable();
			if (folder_out)
				(*folder_out) = folder;
			return folder->variables.Query(lookup.pathSegments.Last());
		}

		PVariable Parser::FindVar(const String&name,PFolder *folder_out/*=NULL*/)
		{
			return FindVar(name.ToRef(),folder_out);
		}

		PFolder 		Parser::FindFolder(const StringRef&path)
		{
			Tokenizer::Tokenize(path,lookup.pathConfig,lookup.pathSegments);
			if (lookup.pathSegments.IsEmpty())
			{
				if (path == '/')
					return state->GetRoot();
				error = "No segments given in '"+path+"'";
				return PFolder();
			}
			lookup.pathSegments<<".";
			return _Resolve(path.BeginsWith('/'));
		}


		bool	Parser::Interpret(const String&line, bool allow_global_commands/*=true*/)
		{
			return Parse(line.ToRef(),allow_global_commands);
		}
	
		bool	Parser::Interpret(const StringRef&line, bool allow_global_commands/*=true*/)
		{
			return Parse(line,allow_global_commands);
		}

		bool	Parser::Parse(const String&line, bool allow_global_commands/*=true*/)
		{
			return Parse(line.ToRef(),allow_global_commands);
		}

		PCommand		Parser::GetExecutingCommand()	const
		{
			return executing;
		}

		PFolder 		Parser::GetExecutionContext()	const
		{
			return executingFolder;
		}

		String			Parser::ExtendedComplete(const StringRef&line, StringList&out)
		{
			out.clear();
			Buffer0<StringRef> segments;
			Tokenizer::Tokenize(line,GetSegmentizerConfig(),segments);

			if (segments.IsEmpty())
				return "";
		
			if (segments.Count() == 1)
			{
				return StandardComplete(segments[0],out);
			}
	
			CLI::PCommand cmd = Find(segments[0]);
			if (!cmd || cmd->completion == CLI::NoCompletion)
			{
				return line;
			}
		
			String prefix = implode(' ',segments.SubRef(0,segments.Count()-1))+" ";
	
			switch (cmd->completion)
			{
				case CLI::FolderCompletion:
				{
					String line = CompleteFolders(segments.Last(),out);
					if (line.IsEmpty())
						return line;
					//for (index_t i = 0; i < out; i++)
					//	out[i] = prefix + out[i];
					return prefix + line;
				}
				case CLI::VariableCompletion:
				{
					String line = CompleteVariables(segments.Last(),out);
					if (line.IsEmpty())
						return line;
					//for (index_t i = 0; i < out; i++)
					//	out[i] = prefix + out[i];
					return prefix + line;
				}
				case CLI::CommandCompletion:
				{
					String line = StandardComplete(segments.Last(),out);
					if (line.IsEmpty())
						return line;
					//for (index_t i = 0; i < out; i++)
					//	out[i] = prefix + out[i];
					return prefix + line;
				}
				case CLI::FileCompletion:
				{
				
				}
			}
	
	

			return prefix;
		}







		void	State::AppendPrefixedMatchingGlobalCommands(const StringRef&needle, StringList&appendTo) const
		{
			for (index_t i = 0; i < globalCommands.Count(); i++)
				if (globalCommands[i]->name.BeginsWith(needle))
					appendTo << "/"+globalCommands[i]->name;
		}


		String			Parser::_Complete(const StringRef&line, const std::function<void(const PFolder&,const String&prefix, StringRef&needle,StringList&)>&include, StringList&out)
		{
				out.clear();
			StringRef longestCommon;
			Tokenizer::Tokenize(line,lookup.pathConfig,lookup.pathSegments);
		
			if (lookup.pathSegments.IsEmpty())
				return "";
			
			PFolder parent = _Resolve(line.BeginsWith('/'));
			if (!parent)
				return "";
		
			String	prefix = lookup.JoinAllButLastSegments();
			StringRef	inner = lookup.pathSegments.Last();
			if (prefix.GetLength())
				prefix+= "/";
				
			if (line.BeginsWith('/'))
				prefix = "/"+prefix;
		
			include(parent,prefix,inner,out);

			if (out.IsEmpty())
				return "";
		
			if (out.Count() == 1)
			{
				return out.First();
			}
			count_t longest = prefix.GetLength()+1;
			bool same(true);
			while (same)
			{
				longestCommon = out.First().SubStringRef(0,longest);
				if (longest > longestCommon.GetLength())
					same = false;
				for (index_t i = 1; i < out.Count() && same; i++)
					same = out[i].BeginsWith(longestCommon);
				if (same)
					longest++;
			}
			longestCommon = longestCommon.SubStringRef(0,longest-1);
		
			return longestCommon;
		}
		

		String			Parser::StandardComplete(const StringRef&line, StringList&out)
		{
			return _Complete(line,[this](const PFolder&parent, const String&prefix, const StringRef&inner, StringList&out)
			{
				if (prefix == '/')
					state->AppendPrefixedMatchingGlobalCommands(inner,out);
		
				for (index_t i = 0; i < parent->commands.Count(); i++)
					if (parent->commands[i]->name.BeginsWith(inner))
						out << prefix+parent->commands[i]->name;
				for (index_t i = 0; i < parent->variables.Count(); i++)
					if (parent->variables[i]->name.BeginsWith(inner))
						out << prefix+parent->variables[i]->name;
				for (index_t i = 0; i < parent->folders.Count(); i++)
					if (parent->folders[i]->name.BeginsWith(inner))
						out << prefix+parent->folders[i]->name+"/";
			},out);
		}

		String	Parser::TLookup::JoinAllButLastSegments() const
		{
			return implode('/',pathSegments.SubRef(0,pathSegments.Count()-1));
		}


		String Parser::CompleteVariables(const StringRef&line,StringList&out)
		{
			return _Complete(line,[this](const PFolder&parent, const String&prefix, const StringRef&inner, StringList&out)
			{
				for (index_t i = 0; i < parent->variables.Count(); i++)
					if (parent->variables[i]->name.BeginsWith(inner))
						out << prefix+parent->variables[i]->name;
				for (index_t i = 0; i < parent->folders.Count(); i++)
					if (parent->folders[i]->name.BeginsWith(inner))
						out << prefix+parent->folders[i]->name+'/';
			},out);
		}



		String Parser::CompleteFolders(const StringRef&line, StringList&out)
		{
			return _Complete(line,[this](const PFolder&parent, const String&prefix, const StringRef&inner, StringList&out)
			{
				for (index_t i = 0; i < parent->folders.Count(); i++)
					if (parent->folders[i]->name.BeginsWith(inner))
						out << prefix+parent->folders[i]->name+"/";
			},out);
		}

		const ArrayRef<StringRef>	Parser::GetExecutionSegments()		const
		{
			return lookup.cmdSegments.ToRef();
		}

		void	Parser::FailCommandExecution(const String&error)
		{
			this->error = error;
		}

	
		bool	Parser::_ParseLine(const StringRef&line,bool allow_global_commands)
		{
			//cout << "parse("<<line<<")"<<endl;
			Tokenizer::Tokenize(line,lookup.segmentConfig,lookup.cmdSegments);
			if (lookup.cmdSegments.IsEmpty())
				return true;
			
			Tokenizer::Tokenize(lookup.cmdSegments[0],lookup.pathConfig,lookup.pathSegments);
			if	(lookup.pathSegments.IsEmpty())
				return	true;	//this would be rather odd...
			
			executingFolder = _Resolve(line[0]=='/');
			if	(!executingFolder)
				return	false;
		
			//cout << "cmdSegments:"<<endl;
			//lookup.cmdSegments.printLines(cout," ");
			//cout << "pathSegments:"<<endl;
			//lookup.pathSegments.printLines(cout," ");

			if	(lookup.cmdSegments.Count() == 1)
				if (PVariable v = executingFolder->variables.Query(lookup.pathSegments.Last()))
				{
					if (state->onVariableCall)
						state->onVariableCall(shared_from_this(),executingFolder,v);
					return true;
				}
		
			executing = executingFolder->commands.Query(lookup.pathSegments.Last());
			if (!executing && allow_global_commands)
				executing = state->GetGlobalCommand(lookup.pathSegments.Last());
		
			if (executing)
			{

				//lookup.segmentConfig
				bool dequote = false;
				for (index_t i = 1; i < lookup.cmdSegments.Count(); i++)
					if (Tokenizer::DequoteChangesString(lookup.cmdSegments[i],lookup.segmentConfig))
					{
						dequote = true;
						break;
					}
				if (dequote)
				{
					lookup.dequotedCMDSegments.Clear();
					lookup.dequotedCMDSegments << lookup.cmdSegments.First();
					for (index_t i = 1; i < lookup.cmdSegments.Count(); i++)
						Tokenizer::Dequote(lookup.cmdSegments[i],lookup.segmentConfig,lookup.dequotedCMDSegments.Append());
					lookup.cmdSegments.Clear();
					foreach (lookup.dequotedCMDSegments,seg)
						lookup.cmdSegments << seg->ToRef();
				}
				executing->Invoke(shared_from_this(),lookup.cmdSegments.ToRef());

				executing.reset();
				executingFolder.reset();
				return error.IsEmpty();
			}
		
			executingFolder.reset();
			error = "Command or Variable unknown: '"+lookup.pathSegments.Last()+"'";
			return false;
		}

		bool	Parser::Parse(const StringRef&line, bool allow_global_commands /*=true*/)
		{
			Buffer0<StringRef>	lines;
			Tokenizer::Tokenize(line,lookup.lineConfig,lines);
			for	(index_t i = 0; i < lines.Count(); i++)
				if (!_ParseLine(lines[i],allow_global_commands))
					return false;
			return true;
		}
	


		count_t	Folder::CountCommands() const
		{
			count_t rs = commands.Count();
			for (index_t i = 0; i < folders.Count(); i++)
				rs += folders[i]->CountCommands();
			return	rs;
		}

		count_t	Folder::CountVariables() const
		{
			count_t rs = variables.Count();
			for (index_t i = 0; i < folders.Count(); i++)
				rs += folders[i]->CountVariables();
			return	rs;
		}


		Folder::Folder(const PFolder&p,const String&n):parent(p),NamedItem(n),customFlags(0)
		{}

		String Folder::GetPath()	const
		{
			String rs = name;
			PFolder folder = parent.lock();
			while (folder)
			{
				if (folder->parent.lock() || folder->name.GetLength())
					rs = folder->name+'/'+rs;
				folder = folder->parent.lock();
			}
			return rs;
		}
	
		String Folder::GetPath(unsigned max_depth)	const
		{
			String rs = name;
			PFolder folder = parent.lock();
			unsigned depth(0);
			while (folder && ++depth<max_depth)
			{
				if (folder->parent.lock() || folder->name.GetLength())
				{
					//cout << "////concatenating '"<<folder->name<<"' and '"<<rs<<"'"<<endl;
					rs = folder->name+'/'+rs;
					//cout << "////rs now '"<<rs<<"'"<<endl;
				}
				folder = folder->parent.lock();
			}
			if (folder && (folder->parent.lock() || folder->name.GetLength()))
				rs = "(...)/"+rs;
			return rs;
		}
	
		PFolder		Folder::GetRoot()
		{
			PFolder p = this->parent.lock();
			if (!p)
				return shared_from_this();
			return p->GetRoot();
		}

		bool Folder::IsChildOf(const PFolder&other)	const
		{
			if (!other)
				return false;
			if (other.get() == this)
				return true;
			const Folder* folder = this;
			for (;;)
			{
				PFolder parent = folder->parent.lock();
				if (parent == other)
					return true;
				if (!parent)
					return false;
				folder = parent.get();
			}
			//return false;
		}


		PFolder 		Parser::EnterNewFolder(const StringRef& name, bool enter_existing)
		{
			Tokenizer::Tokenize(name,lookup.pathConfig,lookup.pathSegments);
			if (lookup.pathSegments.IsEmpty())
				return PFolder();
		
			PFolder parent = _Resolve(name.BeginsWith('/'));
			if (!parent)
				return PFolder();
			
			PFolder result = parent->folders.Query(lookup.pathSegments.Last());
			if (result)
			{
				if (enter_existing)
				{
					focused = result;
					if (onFocusChanged)
						onFocusChanged();
				}
				return PFolder();
			}
			
			result.reset(new Folder(parent,lookup.pathSegments.Last()));
			parent->folders.Insert(result->name,result);
			focused = result;
			if (onFocusChanged)
				onFocusChanged();
			return result;
		}

		void				State::EraseFolder(const PFolder&folder)
		{
			if (!folder)
				return;
			PFolder parent = folder->parent.lock();
			if (!parent)
				return;
			foreach (this->activeParsers,p)
			{
				(*p)->OnFolderDeleted(folder);
			}
			parent->folders.Unset(folder->name);
		}
	
		void				Parser::EraseFolder(const StringRef& name)
		{
			Tokenizer::Tokenize(name,lookup.pathConfig,lookup.pathSegments);
			if (lookup.pathSegments.IsEmpty())
				return;
			
			if (PFolder parent = _Resolve(name.BeginsWith('/')))
				state->EraseFolder(parent->folders.Query(lookup.pathSegments.Last()));
		}


		bool	Parser::MoveFocus(const StringRef&context)
		{
			Tokenizer::Tokenize(context,lookup.pathConfig,lookup.pathSegments);
			lookup.pathSegments << ".";	//dummy
			PFolder new_focus = _Resolve(context.BeginsWith('/'));
			if (new_focus)
			{
				if (new_focus != focused)
				{
					focused = new_focus;
					if (onFocusChanged)
						onFocusChanged();
				}
				return true;
			}
			return false;
		}

		bool	Parser::ExitFolder()
		{
			return ExitFocus();
		}
	
		bool	Parser::ExitFocus()
		{
			PFolder parent = focused->parent.lock();
			if (parent)
			{
				focused = parent;
				if (onFocusChanged)
					onFocusChanged();
				return true;
			}
			return false;
		}

		void	Parser::ResetFocus()
		{
			focused = state->GetRoot();
			if (onFocusChanged)
				onFocusChanged();
		}

		PFolder 	Parser::GetFocus()	const
		{
			return focused;
		}
	
		void		Parser::SetFocus(const PFolder&folder)
		{
			if (folder)
			{
				ASSERT__(folder->GetRoot() == state->GetRoot());
				focused = folder;
				if (onFocusChanged)
					onFocusChanged();
			}
			else
			{
				FATAL__("Trying to set NULL focus");
				//focused = root;
			}
		}

		void		Parser::PushFocus()
		{
			focusStack << focused;
		}

		void		Parser::PushAndReplaceFocus(const PFolder&new_focus)
		{
			PushFocus();
			//if (new_focus)
				SetFocus(new_focus);
		}


		void		Parser::PopFocus()
		{
			ASSERT__(focusStack.IsNotEmpty());
			SetFocus(focusStack.pop());
		}


	
		String			Parser::GetContext(unsigned depth)	const
		{
			return focused->GetPath(depth);
		}

		const Tokenizer::Config&	Parser::GetSegmentizerConfig()	const
		{
			return lookup.segmentConfig;
		}


		Script::Script(const String&Name):StringList(64),name(Name)
		{}

		Script::Script(const String&Name, const PathString&filename):StringList(64),name(Name)
		{
			Load(filename);
		}

		void	Script::LoadFromStream(char*buffer,	size_t len)
		{
			char*pntr=buffer;
			do
			{
				char*next = strstr(pntr,"\n");
				if	(next)
				{
					if	(pntr[next-pntr-1] == 13)
						pntr[next-pntr-1] = 0;
					else
						next[0] = 0;
				}
				add(pntr);
				if	(next)
					pntr = &next[1];
				else
					pntr = NULL;
			}
			while	(pntr);
		}

		void	Script::Load(const PathString&filename)
		{
			clear();
			FILE*f = FOPEN(filename.c_str(),"rb");
			if (!f)
				throw Except::IO::DriveAccess(CLOCATION,"Unable to open '"+filename+"'");
			fseek(f,0,SEEK_END);
			size_t size = ftell(f);
			fseek(f,0,SEEK_SET);
			char*buffer = alloc<char>(size+1);
			fread(buffer,size,1,f);
			buffer[size] = 0;
			fclose(f);
			LoadFromStream(buffer,size+1);
			dealloc(buffer);
		}


		void	ScriptList::Load(const PathString&filename, const String&alias)
		{
			Script*script = Find(alias);
			if (!script)
			{
				script = &Append();
				script->name = alias;
			}
			script->Load(filename);
		}

		bool	ScriptList::Erase(const	String&alias)
		{
			for	(index_t i = 0; i < Count(); i++)
				if (at(i).name == alias)
				{
					Erase(i);
					return true;
				}
			return	false;
		}


		Script*ScriptList::Find(const String&alias)
		{
			for	(index_t i = 0;	i < Count(); i++)
				if (at(i).name == alias)
					return&at(i);
			return nullptr;
		}

		bool	ScriptList::Execute(const	String&alias,Parser&parser)
		{
			Script*script = Find(alias);
			if (!script)
				return false;
			for	(index_t i = 0;	i < script->Count(); i++)
				if (!parser.Parse(script->Get(i)))
					return false;
			return true;
		}
	}
}
