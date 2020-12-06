#ifndef cliTplH
#define cliTplH


/******************************************************************

E:\include\string\cli.tpl.h

******************************************************************/

template <typename F>
	CLI::PCommand				CLI::Parser::DefineCommand(const StringRef& def, const F&f, eCommandCompletion completion/*=NoCompletion*/)
	{
		Tokenizer::Tokenize(def,lookup.pathConfig,lookup.pathSegments);
		if (lookup.pathSegments.IsEmpty())
			return nullptr;
		PFolder parent = _Resolve(def.BeginsWith('/'));
		if (!parent)
			return nullptr;
		Command::Name name;
		if (!name.Parse(lookup.pathSegments.Last()))
			return nullptr;

		PCommand result = parent->commands.Query(name.coreName);
		if (result)
			return result;
		PCommand command(new Command(name,f, completion));
		parent->commands.Insert(command->name, command);
		return command;
	}

template <typename F>
	CLI::PCommand				CLI::Parser::DefineGlobalCommand(const StringRef& def, const F&f, eCommandCompletion completion/*=NoCompletion*/)
	{
		Command::Name name;
		if (!name.Parse(def))
			return nullptr;
		PCommand result = state->GetGlobalCommand(name.coreName);
		if (result)
			return result;
		PCommand command (new Command(name,f, completion));
		state->InsertGlobalCommand(command);
		return command;
	}



template <typename T, count_t Components>
	count_t				DecodeVectorContent(T*content,const String&value)
	{
		if (value.IsEmpty())
			return 0;
		index_t	begin = value.Find('('),
				end = value.Find(')');
		if (begin ==InvalidIndex || end == InvalidIndex)
		{
			begin = value.Find('{');
			end = value.Find('}');
		}
		
		if (begin ==InvalidIndex || end == InvalidIndex)
		{
			begin = value.Find('[');
			end = value.Find(']');
		}
		
		if (begin ==InvalidIndex|| end==InvalidIndex)
		{
			begin = 0;
			end = value.GetLength()-1;
		}
		else
		{
			begin++;
			end--;
		}
		index_t	last = begin,
				count = 0;
		for (index_t i = begin; i <= end && count < Components; i++)
			if (value.GetChar(i)==',' || value.GetChar(i)==';')
			{
				String sub = value.subString(last,i-last);
				sub.TrimThis();
				if (!convert(sub.c_str(),content[count++]))
					return 0;
				last = i+1;
			}
		{
			String sub = value.subString(last,end-last+1);
			sub.TrimThis();
			if (!convert(sub.c_str(),content[count++]))
				return 0;
		}
		return count;
	}

template <typename T, count_t Components>
	bool				SetVectorVariableContent(T*content, const String&component, const String&value)
	{
		T temp[Components];
		count_t count = DecodeVectorContent<T,Components>(temp,value);
		if (count == 0)
			return false;

		if (count == 1 && component.GetLength()>1)
		{
			count = M::vmin(component.GetLength(),Components);

			for (index_t i = 1; i < count; i++)
				temp[i] = temp[0];
		}
		index_t at = 0;
		for (index_t i = 0; i < component.GetLength() && at < count; i++)
		{
			switch (component.GetChar(i))
			{
				case 'r':
				case 'R':
				case 'x':
				case 'X':
				case 'u':
				case 'U':
					if (Components < 1)
						return false;
					content[0] = temp[at++];
				break;
				case 'g':
				case 'G':
				case 'y':
				case 'Y':
				case 'v':
				case 'V':
					if (Components < 2)
						return false;
					content[1] = temp[at++];
				break;
				case 'b':
				case 'B':
				case 'z':
				case 'Z':
					if (Components < 3)
						return false;
					content[2] = temp[at++];
				break;
				case 'a':
				case 'A':
				case 'w':
				case 'W':
					if (Components < 4)
						return false;
					content[3] = temp[at++];
				break;
			}
		}
		return true;
	}
	

template <typename T, count_t Components>
	bool				SetVectorVariableContent(T*content, const String&value)
	{
		count_t count = DecodeVectorContent<T,Components>(content,value);
		if (count == 1)
		{
			for (index_t i = 1; i < Components; i++)
				content[i] = content[0];
			return true;
		}
		return count == Components;
	}

	
template <typename T, count_t Components>
	VectorVariable<T, Components>::VectorVariable(const String&name, unsigned protection):Super(name,"Vector",Components,protection)
	{}
	
template <typename T, count_t Components>
	VectorVariable<T, Components>::VectorVariable(const String&name, const M::TVec<T,Components>&values, unsigned protection):Super(name,"Vector",Components,protection)
	{
		content = values;
	}
	
template <typename T, count_t Components>
	String			VectorVariable<T, Components>::ConvertToString()					const
	{
		return M::VecUnroll<Components>::toString(content.v);
	}
	
template <typename T, count_t Components>
	bool			VectorVariable<T, Components>::Set(const String&value)
	{
		return SetVectorVariableContent<float,Components>(content.v,value);
	}

template <typename T, count_t Components>	
	bool	VectorVariable<T, Components>::Set(const String&component, const String&value)
	{
		return SetVectorVariableContent<float,Components>(content.v,component,value);
	}




#endif
