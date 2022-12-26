#include "../global_root.h"
#include "config.h"
#include "../string/lineWrap.h"
#include "file_system.h"

/******************************************************************

Extended configuration-file-parser.

******************************************************************/


#define err(x,code) {errors.add(String(x)+" in line "+IntToStr(frame.line)+" of '"+frame.filename+"' "+makePath(file_history));error=code;frame.error_context=frame.filename;}
#define _err(x,code) {err(x,code);return code;}
#define serr(x,code) {err_to=code; err_list.add("syntax-error in section '"+dif+"': "+String(x));}

//(included from '"+file_history.Last()+"')
//

namespace DeltaWorks
{

	namespace Config
	{

		Container::Config::Config()
		{
			main_separator="";
			operators = "= := += -= *= /=";
			quotations="\"'";
			quotation_escape_character = '\\';
			recursion_up = "([{";
			recursion_down = ")]}";
			trim_characters = " \t";
		}

		Container::Config						Container::configuration;


		Context::Attribute&		Context::protectedDefine(const String&name, const String&value)
		{
			ASSERT_NOT_NULL__(this);
			ASSERT__(name.IsNotEmpty());
			//Attribute*result;
			index_t resultAt;
			if (attributeMap.Query(name,resultAt))
			{
				Attribute&rs = attributes[resultAt];
				rs.value = value;
				return rs;
			}
			resultAt = attributes.Count();
			Attribute&rs = attributes.Append();
			rs.name = name;
			rs.value = value;
			rs.commented = false;
			rs.assignment_operator = "=";
			attributeMap.Set(name,resultAt);
			return rs;
		}


		Context::Attribute&		Context::Change(const String&path, const String&newValue)
		{
			auto attrib = this->GetAttrib(path);
			if (attrib == nullptr)
				throw Except::Program::DataConsistencyFault("Configuration Context: Unable to find '"+path+"' in the local configuration. Unable to update");
			attrib->value = newValue;
			return *attrib;
		}

		Context::Attribute&		Context::Define(const String&name, const String&value)
		{
			if (!name.Contains('/'))
				return protectedDefine(name.Trim(),value);

			Ctr::Array<String>	components;
			Explode('/',name,components);
			return protectedDefineContext(components.pointer(),components.Count()-1).protectedDefine(components.Last().Trim(),value);
		}

		Context&					Context::protectedDefineContext(const String*names, count_t num_names)
		{
			Context*ctx = this;
			for (index_t i = 0; i < num_names; i ++)
				ctx = &ctx->protectedDefineContext(names[i].Trim());
			return *ctx;
		}
	
		Context&					Context::protectedDefineContext(const String&name)
		{
			ASSERT_NOT_NULL__(this);
			ASSERT__(name.IsNotEmpty());
			index_t resultAt;
			if (childMap.Query(name,resultAt))
				return children[resultAt];
			resultAt = children.Count();
			Context&rs = children.Append();
			rs.name = name;
			rs.isMode = false;
			childMap.Set(name,resultAt);
			return rs;
		}
		Context&					Context::DefineContext(const String&name)
		{
			if (!name.Contains('/'))
				return protectedDefineContext(name.Trim());

			Ctr::Array<String>	components;
			Explode('/',name,components);
			return protectedDefineContext(components.pointer(),components.Count());
		}
		
	
		Context&					Context::CreateContext(const String&name)
		{
			index_t resultAt = children.Count();
			Context&rs = children.Append();
			rs.name = name;
			rs.isMode = false;
			childMap.Set(name,resultAt);
			return rs;
		}
	
		Context&					Context::DefineMode(const String&name)
		{
			index_t resultAt;
			if (modeMap.Query(name,resultAt))
				return modes[resultAt];
			resultAt = modes.Count();
			Context&rs = modes.Append();
			rs.name = name;
			rs.isMode = true;
			modeMap.Set(name,resultAt);
			return rs;
		}
	
		Context&					Context::DefineModeContext(const String&mode_name, const String&context_name)
		{
			return DefineMode(mode_name).DefineContext(context_name);
		}
	
		Context&					Context::CreateModeContext(const String&mode_name, const String&context_name)
		{
			return DefineMode(mode_name).CreateContext(context_name);
		}
	
		void						Context::Clear()
		{
			children.clear();
			childMap.clear();
			modes.clear();
			modeMap.clear();
			attributes.clear();
			attributeMap.clear();
		}


		void						Context::ExportModes(Ctr::ArrayData<Context*>&out)
		{
			out.SetSize(modes.Count());
			for (index_t i = 0; i < modes.Count(); i++)
				out[i] = modes + i;
		}
	
		void						Context::ExportModes(Ctr::ArrayData<const Context*>&out)				const
		{
			out.SetSize(modes.Count());
			for (index_t i = 0; i < modes.Count(); i++)
				out[i] = modes+i;
		}
	
		void						Context::ExportChildren(Ctr::ArrayData<Context*>&out)
		{
			out.SetSize(children.Count());
			for (index_t i = 0; i < children.Count(); i++)
				out[i] = children + i;
		}
	
		void						Context::ExportChildren(Ctr::ArrayData<const Context*>&out)			const
		{
			out.SetSize(children.Count());
			for (index_t i = 0; i < children.Count(); i++)
				out[i] = children + i;
		}
	
		void						Context::ExportAttributes(Ctr::ArrayData<Attribute*>&out)
		{
			out.SetSize(attributes.Count());
			for (index_t i = 0; i < attributes.Count(); i++)
				out[i] = attributes + i;
		}
	
		void						Context::ExportAttributes(Ctr::ArrayData<const Attribute*>&out)		const
		{
			out.SetSize(attributes.Count());
			for (index_t i = 0; i < attributes.Count(); i++)
				out[i] = attributes + i;
		}


		Context::Attribute*					Context::GetAttrib(const String&path)
		{
			Ctr::Array<String,Adopt>	segments;
			Explode('/',path,segments);
			if (!segments.Count())
				return nullptr;
			String mode;
			index_t at= segments.First().Find(':');
			if ( at != InvalidIndex)
			{
				mode = segments.First().subString(0,at);
				segments.First().Erase(0,at+1).TrimThis();
				if (!segments.First().GetLength())
					segments.Erase(index_t(0));
			
			}
			//std::cout << "retrieving attribute '"+implode(", ",segments)+"' ("+mode+")"<<std::endl;
			Context*context = (segments.Count()>1||mode.GetLength())?GetContext(segments.pointer(),segments.Count()-1,mode):this;
			if (!context)
			{
				//std::cout << " context could not be located"<<std::endl;
				return nullptr;
			}
			index_t  resultAt;
			if (context->attributeMap.Query(segments.Last(),resultAt))
				return context->attributes + resultAt;
			/*std::cout << " attribute '"+segments.Last()+"' could not be located:"<<std::endl;
			context->attributes.reset();
			while (Attribute*a = context->attributes.each())
			{
				std::cout << "  "<<a->name<<std::endl;
				if (a->name == segments.Last())
					std::cout << "  ERROR: found unmapped attribute '"<<a->name<<"'!!!"<<std::endl;
			}*/

			return nullptr;
		}


		const Context::Attribute*				Context::GetAttrib(const String&path)						const
		{
			Ctr::Array<String,Adopt>	segments;
			Explode('/',path,segments);
			if (!segments.Count())
				return nullptr;
			String mode;
			index_t at = segments.First().Find(':');
			if (at != InvalidIndex)
			{
				mode = segments.First().subString(0,at);
				segments.First().Erase(0,at+1).TrimThis();
				if (!segments.First().GetLength())
					segments.Erase(index_t(0));
			
			}
			const Context*context = (segments.Count()>1||mode.GetLength())?GetContext(segments.pointer(),segments.Count()-1,mode):this;
			if (!context)
				return nullptr;
			index_t  resultAt;
			if (context->attributeMap.Query(segments.Last(),resultAt))
				return context->attributes + resultAt;
			return nullptr;
		}
	
		const String&				Context::Get(const String&path, const String&except)	const
		{
			const Attribute*rs = GetAttrib(path);
			return rs?rs->value:except;
		}
		bool				Context::QueryString(const String&path, String&outResult)	const
		{
			const Attribute*rs = GetAttrib(path);
			if (rs)
			{
				outResult = rs->value;
				return true;
			}
			return false;
		}
	
		const String&				Context::RequireString(const String&path)	const
		{
			const Attribute*rs = GetAttrib(path);
			if (!rs)
			{
				static String bad = "baadf00d";
				FATAL__("Bad configuration file: Unable to retrieve value of '"+path+"' from '"+this->name+"'");
				return bad;
			}
			return rs->value;
		}
	
		int							Context::GetInt(const String&path, int except)			const
		{
			const Attribute*rs = GetAttrib(path);
			int result;
			if (!rs || !convert(rs->value.c_str(),result))
				return except;
			return result;
		}

		int							Context::RequireInt(const String&path)			const
		{
			const String&svalue = RequireString(path);
			int result;
			if (!convert(svalue.c_str(),result))
			{
				FATAL__("Bad configuration file: Unable to parse '"+svalue+"' of '"+name+"/"+path+ "' to int");
				return -1;
			}
			return result;
		}
	
		unsigned					Context::GetUnsigned(const String&path, unsigned except)	const
		{
			const Attribute*rs = GetAttrib(path);
			unsigned result;
			if (!rs || !convert(rs->value.c_str(),result))
				return except;
			return result;
		}

		unsigned							Context::RequireUnsigned(const String&path)			const
		{
			const String&svalue = RequireString(path);
			unsigned result;
			if (!convert(svalue.c_str(),result))
			{
				FATAL__("Bad configuration file: Unable to parse '"+svalue+"' of '"+name+"/"+path+ "' to unsigned int");
				return -1;
			}
			return result;
		}

		float						Context::GetFloat(const String&path, float except)		const
		{
			const Attribute*rs = GetAttrib(path);
			float result;
			if (!rs || !convert(rs->value.c_str(),result))
				return except;
			return result;
		}

		float							Context::RequireFloat(const String&path)			const
		{
			const String&svalue = RequireString(path);
			float result;
			if (!convert(svalue.c_str(),result))
			{
				FATAL__("Bad configuration file: Unable to parse '"+svalue+"' of '"+name+"/"+path+ "' to float");
				return NAN;
			}
			return result;
		}

		bool						Context::GetBool(const String&path, bool except)		const
		{
			const Attribute*rs = GetAttrib(path);
			bool result;
			if (!rs || !convert(rs->value.c_str(),result))
				return except;
			return result;
		}

		bool						Context::RequireBool(const String&path)		const
		{
			const String&svalue = RequireString(path);
			bool result;
			if (!convert(svalue.c_str(),result))
			{
				FATAL__("Bad configuration file: Unable to parse '"+svalue+"' of '"+name+"/"+path+ "' to bool");
				return false;
			}
			return result;
		}

		Key::Name					Context::GetKey(const String&path, Key::Name except)		const
		{
			const Attribute*rs = GetAttrib(path);
			Key::Name result;
			if (!rs || !convert(rs->value.c_str(),result))
				return except;
			return result;
		}

		Key::Name					Context::RequireKey(const String&path)		const
		{
			const String&svalue = RequireString(path);
			Key::Name result;
			if (!convert(svalue.c_str(),result))
			{
				FATAL__("Bad configuration file: Unable to parse '"+svalue+"' of '"+name+"/"+path+ "' to key");
				return Key::A;
			}
			return result;
		}

		Context*					Context::GetContext(const String&path)
		{
			Ctr::Array<String,Adopt>	segments;
			Explode('/',path,segments);
			if (!segments.Count())
				return NULL;
			String mode;
			index_t at = segments.First().Find(':');
			if (at != InvalidIndex)
			{
				mode = segments.First().subString(0,at);
				segments.First().Erase(0,at+1);
			}
			return GetContext(segments.pointer(),segments.Count(),mode);
		}
	
		Context*					Context::GetContext(const String*path, size_t path_len, const String&mode)
		{
			index_t at;
			if (mode.GetLength() && modeMap.Query(mode,at))
				if (Context*rs = modes[at].GetContext(path,path_len,""))
					return rs;
			if (path_len && childMap.Query(*path,at))
				return children[at].GetContext(path+1,path_len-1,mode);
			return path_len || mode.GetLength()?nullptr:this;
		}
	
		const Context*					Context::GetContext(const String&path) const
		{
			Ctr::Array<String,Adopt>	segments;
			Explode('/',path,segments);
			if (!segments.Count())
				return nullptr;
			String mode;
			index_t at = segments.First().Find(':');
			if (at != InvalidIndex)
			{
				mode = segments.First().subString(0,at);
				segments.First().Erase(0,at+1);
			}
			return GetContext(segments.pointer(),segments.Count(),mode);
		}
	
		const Context*					Context::GetContext(const String*path, size_t path_len, const String&mode) const
		{
			index_t at;
			if (mode.GetLength() && modeMap.Query(mode,at))
				if (const Context*rs = modes[at].GetContext(path,path_len,""))
					return rs;
			if (path_len && childMap.Query(*path,at))
				return children[at].GetContext(path+1,path_len-1,mode);
			return path_len || mode.GetLength()?nullptr:this;
		}
	
		const String&					Context::GetOperator(const String&path)						const
		{
			static String empty = "";
			const Attribute*attrib = GetAttrib(path);
			if (!attrib)
				return empty;
			return attrib->assignment_operator;
		}
	
		void		Context::Cleanup()
		{
			for (index_t i = 0; i < children.Count(); i++)
			{
				Context&child = children[i];
				child.Cleanup();
				if (child.IsEmpty())
				{
					childMap.Unset(child.name);
					children.Erase(i--);
				}
			}
		
			for (index_t i = 0; i < modes.Count(); i++)
			{
				Context&mode = modes[i];
				mode.Cleanup();
				if (mode.IsEmpty())
				{
					modeMap.Unset(mode.name);
					modes.Erase(i--);
				}
			}
		}
	
		bool		Context::IsEmpty() const
		{
			return children.IsEmpty() && attributes.IsEmpty() && modes.IsEmpty();
		}
	
		void		Context::RetrieveMaxNameValueLength(size_t&name_len, size_t&value_len, size_t indent)	const
		{
			foreach (attributes, attrib)
			{
				name_len = std::max(name_len,attrib->name.GetLength() + indent);
				value_len = std::max(value_len,attrib->value.GetLength() + 2 + indent);
			}
			foreach (children,child)
				child->RetrieveMaxNameValueLength(name_len,value_len,indent+4);
		
			foreach (modes,mode)
			{
				if (mode->children.Count() == 1 && mode->attributes.IsEmpty())
					mode->children.First().RetrieveMaxNameValueLength(name_len,value_len,indent+4);
				else
					mode->RetrieveMaxNameValueLength(name_len,value_len,indent+4);
			}
		}


		void		Context::WriteContent(StringBuffer&buffer, size_t name_len, size_t value_len, const String&indent) const
		{
			foreach (attributes,attrib)
			{
				if (attrib->commented)
					buffer << "//";

				if (attrib->comment.IsNotEmpty())
				{
					//count_t tabs = (value_len - len)/4;
					//if ((value_len - len)%4)
					//	tabs++;
					//for (index_t i = 0; i <= tabs; i++)
					//	buffer << '\t';
					Array<StringRef>	lines;
					String fullLine = attrib->name+": "+ attrib->comment.TrimRef();
					LineWrap::UTF8::Wrap(fullLine,80,lines);
					foreach (lines,line)
						buffer << indent << ";"<<*line<<nl;
				}

				buffer << indent << attrib->name;
				count_t tabs = (name_len-attrib->name.GetLength()-indent.GetLength()*4)/4;
				if ((name_len-attrib->name.GetLength()-indent.GetLength()*4)%4)
					tabs++;
				for (index_t i = 0; i <= tabs; i++)
					buffer << '\t';
			
				buffer<<attrib->assignment_operator<<"\t";
			
				size_t len = attrib->value.GetLength();
				if (!isBool(attrib->value.c_str()) && !isFloat(attrib->value.c_str()))
				{
					String value = attrib->value;
					//std::cout << "'"<<value<<"'"<<std::endl;
					for (index_t i = 0; i < value.GetLength(); i++)
						if (value.GetChar(i) == '\"')
						{
							value.Insert(i,'\\');
							//std::cout << "'"<<value<<"'"<<std::endl;
							i++;
						}
					//std::cout << "'"<<value<<"'"<<std::endl;
					buffer << "\""<<value<<"\"";
					len = value.GetLength()+2;
				}
				else
					buffer << attrib->value;
			
				buffer << nl;
			}
			foreach (modes,mode)
			{
				if (mode->children.Count() == 1 && mode->attributes.IsEmpty() && mode->modes.IsEmpty())
				{
					buffer << indent << "["<<mode->name<<":"<<mode->children.First().name<<"]"<<nl;
					mode->children.First().WriteContent(buffer,name_len,value_len,indent+"\t");
				}
				else
				{
					buffer << indent << mode->name<<":"<<nl;
					mode->WriteContent(buffer,name_len,value_len,indent+"\t");
				}
			}
			foreach (children,child)
			{
				if (child->comment.IsNotEmpty())
				{
					Array<StringRef>	lines;
					String fullLine = child->name+": "+child->comment.TrimRef();
					LineWrap::UTF8::Wrap(fullLine,80,lines);
					foreach (lines,line)
						buffer << indent << ";"<<*line<<nl;
				}
				buffer << indent << "["<<child->name<<"]"<<nl;
				child->WriteContent(buffer,name_len,value_len,indent+"\t");
			}
		}
	
	
		Container::Container()
		{
		}
	
		Container::Container(const PathString&filename)
		{
			LoadFromFile(filename);
	
		}
	
		bool	Container::IsOperator(const String&string)
		{
			if (string == '=')
				return true;
			if (string.GetLength() != 2 || string.GetChar(1) != '=')
				return false;
			char c = string.FirstChar();
			switch (c)
			{
				case ':':
				case '+':
				case '-':
				case '*':
				case '/':
					return true;
			}
			return false;
		}
	
		unsigned	Container::GetDepth(const char*line)
		{
			unsigned depth = 0;
			for(;;)
			{
				if (*line == ' ')
					depth ++;
				elif (*line == '\t')
					depth += 4;
				else
					break;
				line++;
			}
			return depth/4;
		}
	
	
		void	Container::Clear()
		{
			Context::Clear();
			error = "";
		}
	
		LogFile	logfile(FileSystem::StringToPath("configuration.log"),true);
	
		bool	Container::LoadFromFile(const PathString&filename)
		{
			StringFile file(CM_STRIP_COMMENTS|CM_RECORD_COMMENTS|CM_STRIP_LINE_START_COMMENTS);
			try
			{
				file.Open(filename);
			}
			catch (...)
			{
				error = "File not found: "+String(filename);
				return false;
			}
			return LoadFromFile(file);
		}

		bool	Container::LoadFromFile(StringFile&file)
		{
			Clear();
			Vector<Context*>	stack_elements;
			stack_elements.append(this);
		
			//logfile << "Now processing file "<<filename<<nl;
			file.lineStartComment = ";";


			bool errors = false;
			String line;
			StringList		tokens;
			StringBuffer	bufferedComments;
			while (file>>line)
			{
				if (!line.Trim().GetLength())
				{
	//				String cmt = file.comment;
					bufferedComments << ' ' << file.comment.TrimRef();
					continue;
				}
				//logfile << ' '<<line<<nl;
				unsigned depth = GetDepth(line.c_str());
				String group = line.GetBetween('[',']').TrimThis();
				if (depth >= stack_elements.Count())
				{
					error += "\nNesting depth "+String(depth)+" of line "+String(file.root_line)+" exceeds context nesting depth ("+String(stack_elements.Count())+"):\n "+line.Trim();
					errors = true;
					continue;
				}
			
				if (depth+1 != stack_elements.Count())
					bufferedComments.Clear();
				while (depth < stack_elements.Count()-1)
					stack_elements.EraseLast();
			
				Context*context = stack_elements.Last();
			
				String mode;
				if (line.TrimRef().EndsWith(':'))
				{
					group = "";
					mode = line.Trim();
					mode.Erase(mode.GetLength()-1);
				}
			
				index_t at = group.Find(':');
				if (at != InvalidIndex)
				{
					mode = group.subString(0,at);
					mode.TrimThis();
					group.Erase(0,at+1).TrimThis();
				}
			
				if (mode.GetLength())
				{
					bool exists = false;
					foreach (stack_elements,ctx)
						if ((*ctx)->IsMode())
						{
							exists = true;
							break;
						}
					if (exists)
					{
						error += "Mode in mode detected: '"+mode+"' in line "+IntToStr(file.root_line)+":\n "+line;
						errors = true;
					}
					else
					{
						Context&child = context->DefineMode(mode);
						if (!group.GetLength())
							stack_elements.append(&child);
						context = &child;
					}
				}
				if (group.GetLength())
				{
					Context&child = context->CreateContext(group);
					stack_elements.append(&child);
					context = &child;
					child.comment = bufferedComments.ToStringRef();
					child.comment.TrimThis();
					while (child.comment.BeginsWith(child.name+":"))
						child.comment = child.comment.subString(child.name.GetLength()+1).TrimRef();
					bufferedComments.Clear();
				}
			
				if (!group.GetLength() && !mode.GetLength())
				{
					Tokenizer::tokenize(line,configuration,tokens);
					if (!tokens)
						continue;
					if (tokens != 3 || !IsOperator(tokens[1]))
					{
						error += "\nSyntax error in line "+String(file.root_line)+(IsOperator(tokens[1])?"":" (invalid operator)")+":\n "+line;
						errors = true;
						continue;
					}
					Attribute&attrib = context->Define(tokens.First());
					if (attrib.value.GetLength())
					{
						error += "\nRedefining attribute '"+attrib.name+"' to '"+tokens[2]+"' in line "+String(file.root_line)+":\n "+line;
						errors = true;
					}
					attrib.assignment_operator = tokens[1];
					attrib.value = Tokenizer::Dequote(tokens[2],configuration);
					attrib.comment = bufferedComments.ToStringRef();
					attrib.comment += ' ';
					attrib.comment += file.comment.Trim();
					attrib.comment.TrimThis();
					while (attrib.comment.BeginsWith(attrib.name+":"))
						attrib.comment = attrib.comment.subString(attrib.name.GetLength()+1).TrimRef();
					bufferedComments.Clear();
				}
			}
			//logfile << "Reached end of file "<<filename<<nl;
			return !errors;
		}
	
	
		bool						Container::SaveToFile(const PathString&filename, const String&generatorName)
		{
			StringFile	file;
			try
			{
				file.Create(filename);
			}
			catch (...)
			{
				error = "Unable to create/overwrite file '"+String(filename)+"'";
				return false;
			}
			error = "";

			SaveToFile(file,generatorName);
			return true;
		}


		void						Container::SaveToStringBuffer(StringBuffer&buffer, const String&generatorName)
		{
			time_t tt = time(NULL);
			const tm*t = localtime(&tt);
			char time_buffer[256];
			strftime(time_buffer,sizeof(time_buffer),"%B %d. %Y %H:%M:%S",t);

			StringList	comments;
			comments << "Extended initialization file generated by "+generatorName +" on "+String(time_buffer)+".";
		
			for (index_t i = 0; i < comments.Count(); i++)
			{
				String&line = comments[i];
				while (line.GetLength() > 80)
				{
					const char	*begin = line.c_str(),
								*c = begin+80;
					while (c != begin && (*c) != ' ')
						c--;
					if (c==begin)
						c = begin+line.GetLength()/2;
					comments.Insert(i+1,line.subString(c-begin+1));
					line.Erase(c-begin);
				}
			}
		
			buffer << "/*";
			buffer << nl;
			for (index_t i = 0; i < comments.Count(); i++)
				buffer << "; "<<comments[i]<<nl;
			buffer << "*/"<<nl<<nl;
		
			size_t	name_len = 0,
					value_len = 0;
			RetrieveMaxNameValueLength(name_len,value_len);
			if (name_len%4)
				name_len += 4-(name_len%4);
			if (value_len%4)
				value_len += 4-(value_len%4);

			Context::WriteContent(buffer,name_len,value_len);
		}


		void						Container::SaveToFile(StringFile&file, const String&generatorName)
		{
			StringBuffer buffer;
			SaveToStringBuffer(buffer,generatorName);
			file << buffer.ToStringRef();
			//return true;
		}
	
		bool						Container::HasErrors()	const
		{
			return error.IsNotEmpty();
	
		}
	
		const String&				Container::GetError()	const
		{
			return error;
		}



		#if 0
	
		bool									CXContext::Process(String&expression, bool singular, String*error_out)	const
		{
			Buffer<index_t>	segment_start(2);
			Buffer<bool>		Is_segment(2);
			for (index_t i = 0; i < expression.GetLength(); i++)
			{
				if (expression.GetChar(i) == '(')
				{
					Is_segment << !(i+1 == expression.GetLength() || expression.GetChar(i+1)!='$');
					segment_start << i;
				}
				elif (expression.GetChar(i) == ')')
				{
					if (!segment_start.Count())
					{
						if (error_out)
							(*error_out) = "Variable end at offset "+String(i)+" of expression '"+expression+"' lacks beginning";
						return false;
					}
					index_t start = segment_start.pop();
					if (!Is_segment.pop())
						continue;
					String key = expression.subString(start+2,i-start-2);
					if (key == '*')
					{
						if (singular)
						{
							expression.Erase(start, i-start+1);
							i = start;
						}
					}
					else
					{
						const Variable*variable = Lookup(key,error_out);
						if (!variable)
						{
							if (error_out)
								(*error_out) += "\nVariable not found: '"+key+"'";
							return false;
						}
						if (singular)
						{
							expression.ReplaceSubString(start, i-start+1, variable->First());
							i = start+variable->First().GetLength();
						}
						else
						{
							String imploded = implode(", ",*variable);
							expression.ReplaceSubString(start, i-start+1, imploded);
							i = start+imploded.GetLength();
						}
					}
				}
			}
			return true;
		}
	
		CXContext::Variable*					CXContext::innerFindVariable(const Ctr::ArrayData<String>&segments)
		{
			CXContext *context = this;
			for (index_t i = 0; i+1 < segments.Count(); i++)
			{
				//context = 
				PCXContext next;
				if (!context->children.Query(segments[i],next))
					return NULL;
				context = next.get();
			}
			std::cout << "checking for variable '"<<segments.Last()<<"' among "<<context->variables.Count()<<" variables"<<std::endl;
			return context->variables.QueryPointer(segments.Last());
		}
	
		const CXContext::Variable*					CXContext::innerFindVariable(const Ctr::ArrayData<String>&segments) const
		{
			const CXContext*context = this;
			for (index_t i = 0; i+1 < segments.Count(); i++)
			{
				PCXContext next;
				if (!context->children.Query(segments[i],next))
					return NULL;
				context = next.get();
			}
			std::cout << "checking for variable '"<<segments.Last()<<"' among "<<context->variables.Count()<<" variables of "<<context->name<<std::endl;
			return context->variables.QueryPointer(segments.Last());
		}
	
		CXContext::Variable*					CXContext::FindVariable(const String&path)
		{
			Ctr::Array<String,Adopt>	segments;
			explodeCallback(IsPathSeparator,path,segments);
			CXContext*context = this;
			while (context->parent)
				context = context->parent;
			return context->innerFindVariable(segments);
		}
	
		const CXContext::Variable*					CXContext::FindVariable(const String&path)	const
		{
			Ctr::Array<String,Adopt>	segments;
			explodeCallback(IsPathSeparator,path,segments);
			const CXContext*context = this;
			while (context->parent)
				context = context->parent;
			return context->innerFindVariable(segments);
		}
	
	
		PCXContext					CXContext::innerFindContext(const Ctr::ArrayData<String>&segments)
		{
			CXContext*context = this;
			for (index_t i = 0; i < segments.Count(); i++)
			{
				context = context->children.lookup(segments[i]);
				if (!context)
					return NULL;
			}
			return context;
		}
	
		const CXContext*					CXContext::innerFindContext(const Ctr::ArrayData<String>&segments) const
		{
			const CXContext*context = this;
			for (index_t i = 0; i < segments.Count(); i++)
			{
				context = context->children.lookup(segments[i]);
				if (!context)
					return NULL;
			}
			return context;
		}
	
		CXContext*					CXContext::FindContext(const String&path)
		{
			Ctr::Array<String,Adopt>	segments;
			explodeCallback(IsPathSeparator,path,segments);
			CXContext*context = this;
			while (context->parent)
				context = context->parent;
			return context->innerFindContext(segments);
		}
	
		const CXContext*					CXContext::FindContext(const String&path)	const
		{
			Ctr::Array<String,Adopt>	segments;
			explodeCallback(IsPathSeparator,path,segments);
			const CXContext*context = this;
			while (context->parent)
				context = context->parent;
			return context->innerFindContext(segments);
		}
	
	
		CXContext::Variable*					CXContext::Lookup(String path, String*error_out)
		{
			if (!Process(path,true,error_out))
				return NULL;
			Variable*rs = FindVariable(path);
			if (!rs && error_out)
				(*error_out) = "Unable to locate variable '"+path+"'";
			return rs;
		}
	
		const CXContext::Variable*				CXContext::Lookup(String path, String*error_out)	const
		{
			if (!Process(path,true,error_out))
				return NULL;
			const Variable*rs = FindVariable(path);
			if (!rs && error_out)
				(*error_out) = "Unable to locate variable '"+path+"'";
			return rs;
		}
	
		void	CXContext::Clear()
		{
			variables_finalized = false;
			children.clear();
			variables.clear();
			pre_finalize_children.clear();
			name = "";
		}
	
		bool	CXContext::validNameChar(char c)
		{
			return (c>='a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='_' || c=='+' || c == '-' || c == '#' || c == '~' || c == '%' || c == '[' || c == ']';
		}
	
		void	CXContext::parse(const String&content)
		{
			std::cout << "parsing content '"<<content<<"'"<<std::endl;
			Ctr::Array<String,Adopt>	lines,segments;
			explode(';',content,lines);
			if (!lines.Count())
				return;
			lines.Last().TrimThis();
			if (!lines.Last().IsEmpty())
				throw Except::IO::DriveAccess::FileDataFault("; expected at end of line '"+lines.Last()+"'");
		
			if (lines.Count() == 1)
				return;
		
			for (index_t i = 0; i+1 < lines.Count(); i++)
			{
				index_t at = lines[i].GetIndexOf('=');
				if (!at)
					throw Except::IO::DriveAccess::FileDataFault("Expression lacks '=': '"+lines[i]+"'");

				String	name = lines[i].subString(0,at-1).Trim(),
						value = lines[i].subString(at).Trim();
				if (name.IsEmpty() || !name.IsValid(validNameChar))
					throw Except::IO::DriveAccess::FileDataFault("Name validation failed. '"+name+"' Is no valid variable name");

				std::cout << "defining variable '"<<name<<"' in "<<this->name<<std::endl;
				Variable*var = variables.append(name);
			
				if (at = value.GetIndexOf("($*)"))
				{
					if (var->Count())
						value.ReplaceSubString(at-1,4,var->First());
					else
						value.Erase(at-1,4);
					value.Insert(at-1,"($*),");
				}
				var->SetSize(1);
				var->First() = value;
			}

		}
	
		void	CXContext::LoadStacked(const XML::Node*node)
		{
			if (!node)
				throw Except::IO::ParameterFault(Except::globalString("Provided node Is NULL"));


			std::cout << "now processing node '"<<node->name<<"'"<<std::endl;
			conditions.SetSize(node->attributes.Count());
			for (index_t i = 0; i < conditions.Count(); i++)
				conditions[i] = *node->attributes.GetChar(i);
			name = node->name;
			parse(node->inner_content);
		
			for (index_t i = 0; i < node->children.Count(); i++)
			{
				std::cout << "processing child #"<<i<<" '"<<node->children[i].name<<"'"<<std::endl;
				const XML::Node&xchild = node->children[i];
				CXContext*child = pre_finalize_children.Append();
				child->parent = this;
				child->LoadStacked(&xchild);
		
				parse(xchild.following_content);
			}
		}
	
	
		void	CXContext::finalizeVariables()
		{
			if (variables_finalized)
				return;
			foreach (variables,it)
			{
				//std::cout << "rendering variable #"<<i<<std::endl;
				Variable*variable = *it;
				ASSERT_NOT_NULL__(variable);
				ASSERT__(variable->Count() > 0);
				String	value = variable->First();
				String error;
				std::cout << "finalizing variable = '"<<value<<"'"<<std::endl;
				if (!Process(value,false,&error))
					throw Except::IO::DriveAccess::FileDataFault("Expression processing failed for expression '"+value+"' ("+error+")");
			
				explode(',',value,*variable);
				for (index_t i = 0; i < variable->Count(); i++)
					(*variable)[i].TrimThis();
			}
			variables_finalized = true;
		}
	
		void	CXContext::trimVariables()
		{
			foreach (variables,it)
			{
				for (unsigned i = 0; i < (*it)->Count(); i++)
				{
					(*(*it))[i].TrimThis();
					if ((*(*it))[i].IsEmpty() || (*(*it))[i] == "($*)")
					{
						(*it)->Erase(i--);
					}
				}
			}
		}
	
		void	CXContext::Finalize()
		{
			bool succeeded = true;
			String error_message;
			finalizeVariables();
			for (index_t i = 0; i < pre_finalize_children.Count(); i++)
			{
				CXContext*child = pre_finalize_children[i];
				bool do_map = true;
				for (index_t j = 0; j < child->conditions.Count(); j++)
				{
					const XML::TAttribute&attrib = child->conditions[j];
					if (attrib.name == "inherit")
					{
						String value = attrib.value;
						String error;
						if (!Process(value,false,&error))
						{
							error_message = "Failed to process inheritance '"+value+"' ("+error+")";
							succeeded = false;
							continue;
						}

						Ctr::Array<String,Adopt>	inherit;
						explode(',',value,inherit);
						child->finalizeVariables();

						for (index_t k = 0; k < inherit.Count(); k++)
						{
							inherit[k].TrimThis();
							if (inherit[k].IsEmpty())
								continue;
							CXContext*target = FindContext(inherit[k]);
							if (!target)
							{
								error_message = "Failed to locate inheritance '"+inherit[k]+"'";
								succeeded = false;
								continue;
							}
							if (target->IsChildOf(child) || child->IsChildOf(target))
								continue;
							Ctr::Array<String,Adopt>		names;
							Ctr::Array<Variable*>	variables;
							target->variables.ExportTo(names, variables);
							for (index_t i = 0; i < names.Count(); i++)
							{
								std::cout << "inheriting variable "<<names[i]<<" = '"<<implode("', '",*(variables[i]))<<"' into "<<child->name<<std::endl;
								if (!child->variables.IsSet(names[i]))
									(*child->variables.append(names[i])) = *(variables[i]);
								else
								{
									Variable*var = child->variables[names[i]];
									ASSERT_NOT_NULL__(var);
									std::cout <<" variable already exists with value '"<<implode("', '",*(var))<<"'"<<std::endl;
									index_t at;
									for (index_t j = 0; j < var->Count(); j++)
										while (at = (*var)[j].GetIndexOf("($*)"))
										{
											std::cout << "found ($*) in parameter #"<<j<<std::endl;
											(*var)[j].ReplaceSubString(at-1,4,implode(',',*variables[i]));
											Ctr::Array<String,Adopt>	sub;
											explode(',',(*var)[j],sub);
											for (index_t k = 0; k < sub.Count(); k++)
												sub[k].TrimThis();
											var->Erase(j);
											var->insertImport(j,sub);
											break;
										}
								}
							}
						}
						continue;
					}
					const Variable*var = FindVariable(attrib.name);
					if (!var)
					{
						error_message += "\nUnable to check for compliance of conditional attribute '"+attrib.name+"="+attrib.value+"'";
						succeeded = false;
						do_map = false;
						continue;
					}
				
					if (var->First() != attrib.value)
					{
						do_map = false;
						break;
					}
				}
				if (do_map)
				{
					if (children.IsSet(child->name))
					{
						error_message = "Trying to redefine child context '"+child->name+"'";
						Discard(child);
						succeeded = false;
					}
					else
					{
						children.append(child->name,child);
						if (succeeded)
							try
							{
								child->Finalize();
							}
							catch (const std::exception&except)
							{
								error_message = except.what();
								succeeded = false;
							}
					}
				}
				else
					Discard(child);
			}
			pre_finalize_children.flush();
		
			trimVariables();
		
			if (!succeeded)
				throw Except::IO::DriveAccess::FileDataFault(error_message);
		}
	
	
		void	CXContext::LoadFromXML(const XML::Container&container, bool stack)
		{
			if (!stack)
				Clear();
			LoadStacked(&container.root_node);
			if (!stack)
				return Finalize();
		}
	
		void	CXContext::LoadFromFile(const PathString&filename, bool stack)
		{
			XML::Container	container;
			container.LoadFromFile(filename);
			LoadFromXML(container,stack);
		}
	
		bool	CXContext::IsPathSeparator(char c)
		{
			return c == '.' || c == '/';
		}
	
		bool	CXContext::Set(const String&variable_name, const String&variable_value)
		{
			Ctr::Array<String,Adopt>	segments;
			explodeCallback(IsPathSeparator,variable_name,segments);
			CXContext*context = this;
			CXContext*result;
			String var_name = segments.Last();
			segments.Erase(segments.Count()-1);
			while (context)
			{
				if (result = context->innerFindContext(segments))
				{
					Variable*var = result->variables.append(var_name);
					var->SetSize(1);
					var->First() = variable_value;
					return true;
				}
				context = context->parent;
			}
			return false;
		}
	
		bool	CXContext::IsChildOf(const CXContext*other)	const
		{
			const CXContext*ctx = this;
			while (ctx && ctx != other)
				ctx = ctx->parent;
			return ctx == other;
		}
	
		void	CXContext::PrintToCOut(unsigned indent)	const
		{
			std::cout << tabSpace(indent)<<name<<std::endl;
			Ctr::Array<String,Adopt>		names;
			Ctr::Array<const Variable*>	variables;
			this->variables.ExportTo(names, variables);
			for (index_t i = 0; i < names.Count(); i++)
				std::cout << tabSpace(indent+1)<<names[i]<<" := '"<<implode("', '",*(variables[i]))<<"'"<<std::endl;
				
			for (StringMappedList<CXContext>::const_iterator it = children.begin(); it != children.end(); ++it)
				(*it)->PrintToCOut(indent+1);
		}
		#endif /*0*/
	
	
	}

}
