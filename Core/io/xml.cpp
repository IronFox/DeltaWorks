#include "../global_root.h"
#include "xml.h"
#include "../string/encoding.h"
#include "file_system.h"

/******************************************************************

E:\include\io\xml.cpp

******************************************************************/

namespace DeltaWorks
{
//	#define IsWhitespace(c)	((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')

	//#include "log.h"

	//static Log	log_file("xml.log",true);

	static char* findCharacter(char*from, char c)
	{
		while ((*from) && (*from)!=c)
			from++;
		if (!*from)
			return NULL;
		return from;
	}

	static StringRef GetWord(char*from)
	{
		while (IsWhitespace(*from))
			from++;
		char*c = from,quote;
		bool in_string(false);
		while (*c)
		{
			if (*c == '\"' || *c == '\'')
			{
				if (!in_string)
				{
					in_string = true;
					quote = *c;
				}
				else
				{
					in_string = (*c) != quote;
					if (!in_string)
					{
						c++;
						break;
					}
				}
			}
			else
			{
				if ((IsWhitespace(*c) || (*c) == '>' || (*c) == '=') && !in_string)
					break;
			}
			c++;
		}
		return StringRef(from,c-from);
		//char temp = *c;
		//(*c) = 0;
		//String rs = from;
		//(*c) = temp;
		//return rs;
	}

	static unsigned findWord(char*from)
	{
		char*c = from,quote;
		while (IsWhitespace(*c))
			c++;
		bool in_string(false),
				found_word(false);
		while (*c)
		{
			if (*c == '\"' || *c == '\'')
			{
				if (!in_string)
				{
					in_string = true;
					quote = *c;
				}
				else
				{
					in_string = (*c) != quote;
					if (!in_string)
					{
						found_word = true;
						c++;
						break;
					}
				}
				found_word = true;
			}
			else
			{
				if ((IsWhitespace(*c) || (*c) == '>' || (*c) == '=' /*|| (*c) == '/'*/) && !in_string)
					break;
				found_word = true;
			}
			c++;
		}
		if (!*c || !found_word)
			return 0;
		return c-from;
	}



	template <typename Stream>
		static void EncodeCharacterToStream(Stream&stream, XML::Encoding enc, XML::Encoding hostEnc, const char*c,const char*const end)
		{
			if (hostEnc == enc)
				stream << *c;
			else
			{
				if (enc == XML::Encoding::Windows1252)
				{
					ASSERT__(hostEnc == XML::Encoding::UTF8);

					StringEncoding::UTF8::TChar ch;
					ch.numCharsUsed = StringEncoding::UTF8::GetLength(*c);
					if (c + ch.numCharsUsed > end)
						throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Remaining string does not contain all characters of a decoded UTF8 multi-byte string");

					memcpy(ch.encoded,c,ch.numCharsUsed);
					c += ch.numCharsUsed;
					stream << StringEncoding::UTF8::ToCP1252(ch);
				}
				elif (enc == XML::Encoding::UTF8)
				{
					ASSERT__(hostEnc == XML::Encoding::Windows1252);
					StringEncoding::UTF8::TChar ch;
					StringEncoding::CP1252::ToUTF8(*c,ch);
					stream << ch.ToRef();
				}
				else
					throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Unsupported encoding encountered");
			}
		}

	template <typename Stream>
	static void EncodeValueToStreamP(Stream&stream, XML::Encoding enc, XML::Encoding hostEnc, const String&content)
	{
		DBG_ASSERT_EQUAL1__(content.GetLength(),strlen(content.c_str()),content.c_str());
	
		//String rs = content;
		const char*c = content.c_str(),
				*const end = content.end();
		for (;c != end; ++c)
		{
			switch (*c)
			{
				case '&':
					stream << "&amp;";
				break;
				case '<':
					stream << "&lt;";
				break;
				case '>':
					stream << "&gt;";
				break;
				case '\'':
				{
					stream << "&apos;";
				}
				break;
				case '\"':
					stream << "&quot;";
				break;
				default:
					EncodeCharacterToStream(stream,enc,hostEnc,c,end);
				break;
			}
		}
	}

	template <typename Stream>
	static void EncodeValueToStreamB(Stream&stream, XML::Encoding enc, XML::Encoding hostEnc, const String&content)
	{
		DBG_ASSERT_EQUAL1__(content.GetLength(),strlen(content.c_str()),content.c_str());
	
		//String rs = content;
		const char*c = content.c_str(),
				*const end = content.end();
		for (;c != end; ++c)
		{
			switch (*c)
			{
				case '&':
					stream << "&amp;";
				break;
				case '<':
					stream << "&lt;";
				break;
				default:
					EncodeCharacterToStream(stream,enc,hostEnc,c,end);
				break;
			}
		}
	}


	//static String Decode(const StringRef&content, XML::Encoding enc)
	//{
	//	switch (enc)
	//	{
	//		case XML::Encoding::Windows1252:
	//			return content;
	//		case XML::Encoding::UTF8:
	//		{
	//			String rs;
	//			StringEncoding::UTF8::ToCP1252(content,rs);
	//			return std::move(rs);
	//		}
	//		default:
	//			throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Unsupported encoding encountered");
	//	}
	//	return "";
	//}


	static void DecodeCharacter(const char*&source, const char*const end, StringBuffer&buffer, XML::Encoding enc, XML::Encoding hostEnc)
	{
		if (enc == hostEnc)
		{
			buffer << *source;
			source++;
		}
		else
		{
			if (enc == XML::Encoding::UTF8)
			{
				ASSERT__(hostEnc == XML::Encoding::Windows1252);
				char c;

				StringEncoding::UTF8::TChar ch;
				ch.numCharsUsed = StringEncoding::UTF8::GetLength(*source);
				if (source + ch.numCharsUsed > end)
					throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Remaining string does not contain all characters of a decoded UTF8 multi-byte character");

				memcpy(ch.encoded,source,ch.numCharsUsed);
				source += ch.numCharsUsed;
				buffer << StringEncoding::UTF8::ToCP1252(ch);
			}
			elif (enc == XML::Encoding::Windows1252)
			{
				ASSERT__(hostEnc == XML::Encoding::UTF8);
				StringEncoding::UTF8::TChar ch;
				StringEncoding::CP1252::ToUTF8(*source,ch);
				buffer << ch.ToRef();
				source++;
			}
			else
				throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Unsupported encoding encountered: #"+String((int)enc));
		}
	}

	static String DecodeValue(const StringRef&content, XML::Encoding enc, XML::Encoding hostEnc, StringBuffer&buffer)
	{
		buffer.Clear();

		const char*source = content.pointer();
		const char*const end = source + content.GetLength();
		while (source < end)
		{
			if (*source != '&')
			{
				DecodeCharacter(source,end,buffer,enc,hostEnc);
			}
			else
			{
				source++;
				if (source >= end)
					break;
				const char*begin = source;
				while (source < end && *source != ';')
					source++;
				if (source >= end)
					throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Entity is not terminated in value '"+String(content)+"'");
				StringRef sub(begin,source-begin);
				if (sub == "amp")
					buffer << '&';
				elif (sub == "lt")
					buffer << '<';
				elif (sub == "gt")
					buffer << '>';
				elif (sub == "quot")
					buffer << '\"';
				elif (sub == "apos")
					buffer << '\'';
				else
				{
					buffer << '&';
					while (begin != source)
						DecodeCharacter(begin,source,buffer,enc,hostEnc);
					buffer<<';';
				}
				source++;
			}
		}
		return buffer.ToStringRef();
	}


	static String DecodeWord(const StringRef&content, XML::Encoding enc, XML::Encoding hostEnc, StringBuffer&buffer)
	{
		if (enc == hostEnc)
			return content;
		buffer.Clear();
		const char*source = content.pointer();
		const char*const end = source + content.GetLength();
		while (source != end)
			DecodeCharacter(source,end,buffer,enc,hostEnc);
		return buffer.ToStringRef();
	}

	/*
	XML::Node::Node(const Node&other):name(other.name),inner_content(other.inner_content),following_content(other.following_content),parent(NULL)
	{
		for (unsigned i = 0; i < other.children; i++)
			children.append(SignalNew(new Node(*other.children[i])))->parent = this;
		attributes = other.attributes;
	}*/

	void		XML::Node::swap(Node&other)
	{
		name.swap(other.name);
		inner_content.swap(other.inner_content);
		following_content.swap(other.following_content);
		//swp(parent,other.parent);
		children.swap(other.children);
		attributes.swap(other.attributes);
	}



	XML::Node&		XML::Node::NewChild(const String&name)
	{
		Node&rs = children.Append();
		rs.name = name;
		return rs;
	}

	void	XML::AttributeTable::Set(const String&pname, const String&pval)
	{
		index_t at = attributeList.Count();
		TAttribute& p = attributeList.Append();
		attributeMap.Set(pname,at);
		p.name = pname;
		p.value = pval;
	}


	void				XML::AttributeTable::Unset(const String&pname)
	{
		index_t at;
		if (!attributeMap.QueryAndUnset(pname,at))
			return;
		attributeMap.VisitAllValues([at](index_t&entry)
		{
			if (entry > at)
				entry--;
		});
		attributeList.Erase(at);
	}
	/*
	String			XML::Node::path()	const
	{
		if (!this)
			return "NULL";
		String result = name;
		const XML::Node*at = parent;
		while (at)
		{
			result = at->name+'/'+result;
			at = at->parent;
		}
		return result;
	}
	*/


	const String&	XML::AttributeTable::Require(const String&attribName) const
	{
		index_t at;
		if (!attributeMap.Query(attribName,at))
			throw Except::Program::MemberNotFound(CLOCATION,"Required XML attribute '"+attribName+"' not found in this context");
		DBG_ASSERT_EQUAL__(attributeList[at].name,attribName);
		return attributeList[at].value;
	}


	bool			XML::AttributeTable::Query(const String&name, String&out) const
	{
		index_t at;
		if (!attributeMap.Query(name,at))
			return false;
		DBG_ASSERT_EQUAL__(attributeList[at].name,name);
		out = attributeList[at].value;
		return true;
	}

	bool			XML::AttributeTable::Query(const char*name, String&out)		const
	{
		index_t at;
		if (!attributeMap.Query(name,at))
			return false;
		DBG_ASSERT_EQUAL__(attributeList[at].name,String(name));
		out = attributeList[at].value;
		return true;
	}

	bool			XML::AttributeTable::Query(const String&name, StringRef&out) const
	{
		index_t at;
		if (!attributeMap.Query(name,at))
			return false;
		DBG_ASSERT_EQUAL__(attributeList[at].name,name);
		out = attributeList[at].value.ref();
		return true;
	}

	bool			XML::AttributeTable::Query(const char*name, StringRef&out)		const
	{
		index_t at;
		if (!attributeMap.Query(name,at))
			return false;
		DBG_ASSERT_EQUAL__(attributeList[at].name,String(name));
		out = attributeList[at].value.ref();
		return true;
	}


	/*
	bool			XML::Node::Query(const String&attrib_name, String&val_out) const
	{
		if (!this)
			return false;
		if (const TAttribute*attrib = attributes.lookup(attrib_name))
		{
			val_out = attrib->value;
			return true;
		}
		return false;
	}*/

	void	XML::Container::Clear()
	{
		root_node.name = "root";
		root_node.inner_content = "";
		root_node.following_content = "";
		root_node.children.clear(0);
		root_node.attributes.Clear();
	}

	static XML::Encoding	DecodeEncoding(const StringRef&name)
	{
		if (name.CompareToIgnoreCase("UTF-8") == 0)
			return XML::Encoding::UTF8;
		if (name.CompareToIgnoreCase("ANSI")==0 
			|| name.CompareToIgnoreCase("ISO-8859-1")==0 
			|| name.CompareToIgnoreCase("windows-1252")==0
			|| name.CompareToIgnoreCase("cp-1252")==0)
			return XML::Encoding::Windows1252;
		throw Except::IO::StructureCompositionFault(CLOCATION,("XML: Unsupported encoding encountered: '"+String(name)+"'"));
	//	return XML::Encoding::UTF8;
	}

	void	XML::Container::LoadFromCharArray(Ctr::ArrayData<char>&field)
	{
		encoding = Encoding::UTF8;
		PreAllocVector<Node*,4>	parse_stack;

		bool in_block(false),/*in_string(false),*/open(false);
	
		Node	*active_entry = NULL,
				*last(NULL);
	
		char	*c = field.pointer(),
				*last_c=NULL;
		while (*c)
		{
			ASSERT__(c!=last_c);
			last_c = c;
			char*next = strchr(c,'<');
			if (!next)
			{
				if (!active_entry)
					return;
				throw Except::IO::DriveAccess::FileFormatFault("XML: Did not return to root level at end of file");
			}
			
			{
	//			*next = 0;
				if (open)
				{
					if (active_entry)
						active_entry->inner_content += DecodeValue(StringRef(c,next-c),encoding,hostEncoding,myBuffer);
				}
				else
					if (last)
						last->following_content += DecodeValue(StringRef(c,next-c),encoding,hostEncoding,myBuffer);
	//			*next = '<';
			}
			c = next;

			c++;

			if (*c == '?')
			{
				char*end = strchr(c, '>');
				if (!end)
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Cannot find enclosing '>' character to '<?...'"));
				const char*enc = strstr(c,"encoding");
				if (!enc || enc > end)
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Missing encoding"));
				enc += 8;
				while (IsWhitespace(*enc))
					enc++;
				if (*enc != '=')
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Encoding assignment broken"));
				enc++;
				while (IsWhitespace(*enc))
					enc++;
				if (*enc != '"' && *enc != '\'')
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Encoding assignment broken"));
				enc++;
				const char*encStart = enc;
				while (*enc != '"' && *enc != '\'')
					enc++;
				encoding = DecodeEncoding(StringRef(encStart,enc-encStart));
				c = end+1;
			}
			elif (c[0] == '!' && c[1] == '-' && c[2] == '-')
			{
				c = strstr(c+3,"-->");
				if (!c)
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Cannot find enclosing '-->' character to '<!--...'"));

				c+=3;
			}
			elif (*c == '/')
			{
				String name = DecodeWord(GetWord(c+1),encoding,hostEncoding,myBuffer);
				last = active_entry;
				if (active_entry && name == active_entry->name)
					active_entry = parse_stack.pop();

				c = strchr(c,'>');
				if (!c)
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Cannot find enclosing '>' character to '</...'"));

				c++;
				open = false;
			}
			else
			{
				Node		*sub = active_entry?&active_entry->children.Append():&root_node;
				//sub->parent = active_entry;
				parse_stack << active_entry;
				active_entry = sub;
				active_entry->name = DecodeWord(GetWord(c),encoding,hostEncoding,myBuffer);
	//			ShowMessage("opening "+active_entry->name);
				in_block = true;
				c += active_entry->name.GetLength();
				open = true;
				while (unsigned len = findWord(c))
				{
					String pname = DecodeWord(GetWord(c),encoding,hostEncoding,myBuffer);
					c += len;
					if (pname == "/")
					{
						last = active_entry;
						active_entry = parse_stack.pop();//active_entry->parent;
	//					ShowMessage("breaking down");
						open = false;
						break;
					}
					c = strchr(c,'=');
					if (!c)
						throw Except::IO::DriveAccess::FileFormatFault("XML: Cannot find assignment operator '=' for attribute '"+pname+"'");

					c++;
					len = findWord(c);
					if (!len)
						throw Except::IO::DriveAccess::FileFormatFault("XML: Missing attribute value of attribute '"+pname+"'");

					StringRef pvalue = GetWord(c);
					c += len;
					if (pvalue.GetFirstChar() == '\"' || pvalue.GetFirstChar() == '\'')
						pvalue.DropFirstChar();
					if (pvalue.GetLength() && (pvalue.GetLastChar() == '\"' || pvalue.GetLastChar() == '\''))
						pvalue.DropLastChar();
					active_entry->Set(pname,DecodeValue(pvalue,encoding,hostEncoding,myBuffer));
	//				ShowMessage("specifying parameter "+pname+" = "+pvalue);
				}
				c = strchr(c,'>');
				if (!c)
					throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML: Cannot find enclosing '>' character to '<...'"));

				if (*(c-1) == '/' && open)
				{
					last = active_entry;
					if (active_entry)
						active_entry = parse_stack.pop();//active_entry->parent;
	//					ShowMessage("breaking down");
					open = false;
				}
				c++;
			}
		}

	}

	void	XML::Container::LoadFromString(const String&content)
	{
		Clear();
	
		Ctr::Array<char>	field(content.c_str(),content.GetLength()+1);
		size_t len = content.GetLength();
	
		field[len] = 0;
	
		LoadFromCharArray(field);
	}

	void	XML::Container::LoadFromFile(const PathString&filename, String*content_out)
	{
		#if SYSTEM==WINDOWS
			FILE*f = _wfopen(filename.c_str(),L"rb");
		#else
			FILE*f = fopen(filename.c_str(),"rb");
		#endif
		if (!f)
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"XML: Unable to open file '"+FileSystem::PathToString(filename)+"'");

		Clear();
		fseek(f,0,SEEK_END);
		unsigned len = ftell(f);
		fseek(f,0,SEEK_SET);
	
		Ctr::Array<char>	field;
		field.SetSize(len+1);
	
		if (fread(field.pointer(),1,len,f)!=len)
		{
			fclose(f);

			throw Except::IO::DriveAccess::DataReadFault(CLOCATION,"XML: Unable to read data from file");
		}
		field[len] = 0;
		fclose(f);
	
		if (content_out)
			(*content_out) = field;
	
		LoadFromCharArray(field);
	}

	inline static bool isNewLine(char c)
	{
		return c == '\n' || c == '\r';
	}

	inline static void trim(Ctr::Array<String,Adopt>&field)
	{
		for (unsigned i = 0; i < field.GetLength(); i++)
		{
			field[i].TrimThis();
			if (!field[i].GetLength())
				field.Erase(i--);
		}
	}

	template <class OutStream>
	static void EncodeToStream(OutStream&outfile, XML::Encoding enc, const String&identifier)
	{
		if (enc == XML::Encoding::Windows1252)
			outfile << identifier;
		String encoded;
		switch (enc)
		{
			case XML::Encoding::UTF8:
				StringEncoding::CP1252::ToUTF8(identifier,encoded);
			break;
			default:
				throw Except::IO::StructureCompositionFault(CLOCATION,"XML: Unsupported encoding encountered");
		}
		outfile << encoded;
	}


	template <class OutStream>
	static void writeToStream(OutStream&outfile, XML::Encoding enc, XML::Encoding hostEnc, const XML::Node*entry, XML::export_style_t style, unsigned indent=0)
	{
		StringRef trimmedName = entry->name.TrimRef();
		//entry->name.TrimThis();
		if (trimmedName.IsEmpty())
			throw Except::IO::StructureCompositionFault("XML: Local entry is empty");

		if (style==XML::Nice)
			outfile << tabSpace(indent);
		outfile << "<";
		EncodeToStream(outfile, enc,trimmedName);
		foreach (entry->attributes,p)
		{
			if (style != XML::Tidy || p == entry->attributes.begin())
				outfile << " ";
			EncodeToStream(outfile, enc,p->name);
			outfile << "=\"";
			EncodeValueToStreamP(outfile,enc,hostEnc, p->value);
			outfile<<"\"";
		}
		if (entry->children.IsEmpty() && entry->inner_content.IsEmpty())
		{
			if (style != XML::Tidy)
				outfile << " /";
			else
				outfile << '/';
		}
		outfile << ">";
		//if (style==XML::Raw || !entry->children)
		{
			switch (style)
			{
				case XML::Raw:
					EncodeValueToStreamB(outfile,enc,hostEnc, entry->inner_content);
				break;
				case XML::Nice:
				{
					Ctr::Array<String,Adopt>	lines;
					ExplodeCallback(isNewLine,entry->inner_content,lines);
					trim(lines);
					if (lines.Count() > 1)
					{
						outfile << nl;
						for (index_t i = 0; i < lines.Count(); i++)
						{
							outfile << tabSpace(indent+1);
							EncodeValueToStreamB(outfile,enc,hostEnc,lines[i]);
							outfile << nl;
						}
						if (entry->children.IsEmpty())
							outfile << tabSpace(indent);
					}
					else
					{
						if (lines.IsNotEmpty())
						{
							EncodeValueToStreamB(outfile,enc,hostEnc,lines.First());
						}
						if (entry->children.IsNotEmpty())
							outfile << nl;
					}
				}
				break;
				case XML::Tidy:
				{
					Ctr::Array<String,Adopt>	lines;
					ExplodeCallback(isNewLine,entry->inner_content,lines);
					trim(lines);
					for (index_t i = 0; i < lines.Count(); i++)
					{
						if (i)
							outfile << nl;
						EncodeValueToStreamB(outfile,enc,hostEnc,lines[i]);
					}
				}
				break;
			}
		}

		for (index_t i = 0; i < entry->children.Count(); i++)
			writeToStream(outfile,enc,hostEnc,entry->children +i,style,indent+1);
	
		if (entry->children.IsNotEmpty() || entry->inner_content.IsNotEmpty())
		{
			if (style==XML::Nice && entry->children.IsNotEmpty())
				outfile << tabSpace(indent);
			outfile << "</";
			EncodeToStream(outfile,enc,trimmedName);
			outfile << ">";
		}

		{
			switch (style)
			{
				case XML::Raw:
					EncodeValueToStreamB(outfile,enc,hostEnc,entry->following_content);
					//outfile << encode(entry->following_content);
				break;
				case XML::Nice:
				{
					Ctr::Array<String,Adopt>	lines;
					ExplodeCallback(isNewLine,entry->following_content,lines);
					trim(lines);
					outfile << nl;
					for (index_t i = 0; i < lines.Count(); i++)
					{
						outfile << tabSpace(indent);
						EncodeValueToStreamB(outfile,enc,hostEnc,lines[i]);
						outfile<<nl;
					}
				}
				break;
				case XML::Tidy:
				{
					Ctr::Array<String,Adopt>	lines;
					ExplodeCallback(isNewLine,entry->following_content,lines);
					trim(lines);
					for (index_t i = 0; i < lines.Count(); i++)
					{
						if (i)
							outfile << nl;
						EncodeValueToStreamB(outfile,enc,hostEnc,lines[i]);
					}
				}
				break;
			}
		}
	}

	void				XML::Container::SaveToStringBuffer(StringBuffer&target, export_style_t style) const
	{
		writeToStream(target,encoding,hostEncoding, &root_node,style);
	}

	void				XML::Container::SaveToFile(const PathString&filename, export_style_t style) const
	{
		StringFile outfile;
		outfile.Create(filename);

		outfile << "<?xml version=\"1.0\" encoding=\"";
		switch (encoding)
		{
			case Encoding::Windows1252:
				outfile << "windows-1252";
			break;
			case Encoding::UTF8:
				outfile << "UTF-8";
			break;
			default:
				throw Except::Program::DataConsistencyFault(CLOCATION,"XML: Unsupported encoding for export");
		}
		outfile <<"\" ?>"<<nl;
		writeToStream(outfile,encoding,hostEncoding, &root_node, style);
	}

	static XML::Node*	findIn(Container::BasicVector<XML::Node,SwapStrategy>&children, const StringRef&path)
	{
		StringRef local,sub;
		index_t p = path.Find('/');
		if (p != InvalidIndex)
		{
			local = path.SubStringRef(0,p);
			sub = path.SubStringRef(p+1);
		}
		else
			local = path;

		//cout << "searching for local='"<<local<<"' among "<<children.Count()<<" children"<<endl;
		XML::Node*result;
		for (index_t i = 0; i < children.Count(); i++)
			if (children[i].name == local)
			{
				if (sub.IsEmpty())
					return children+i;
				return findIn(children[i].children,sub);
			}
		return NULL;
	}

	static const XML::Node*	findInConst(const Container::BasicVector<XML::Node,SwapStrategy>&children, const StringRef&path)
	{
		StringRef local,sub;
		index_t p = path.Find('/');
		if (p != InvalidIndex)
		{
			local = path.SubStringRef(0,p);
			sub = path.SubStringRef(p+1);
		}
		else
			local = path;

		//cout << "searching for local='"<<local<<"' among "<<children.Count()<<" children"<<endl;
		XML::Node*result;
		for (index_t i = 0; i < children.Count(); i++)
			if (children[i].name == local)
			{
				if (sub.IsEmpty())
					return children+i;
				return findInConst(children[i].children,sub);
			}
		return NULL;
	}

	static XML::Node*	findFrom(XML::Node*context, const StringRef&path)
	{
		StringRef local,sub;
		index_t p = path.Find('/');
		if (p != InvalidIndex)
		{
			local = path.SubStringRef(0,p);
			sub = path.SubStringRef(p+1);
		}
		else
			local = path;

		if (context->name != local)
		{
			//cout << "context name is '"<<context->name<<"', local is '"<<local<<"'"<<endl;
			return NULL;
		}
		if (sub.IsEmpty())
			return context;
		//cout << "delegating search to "<<context->children.Count()<<" children using sub path '"<<sub<<"'"<<endl;
		return findIn(context->children,sub);
	}

	static const XML::Node*	findFromConst(const XML::Node*context, const StringRef&path)
	{
		StringRef local,sub;
		index_t p = path.Find('/');
		if (p != InvalidIndex)
		{
			local = path.SubStringRef(0,p);
			sub = path.SubStringRef(p+1);
		}
		else
			local = path;

		if (context->name != local)
			return NULL;
		if (sub.IsEmpty())
			return context;
		return findInConst(context->children,sub);
	}

	static XML::Node&	createIn(XML::Node*context, const String&path, const String&inner_content)
	{
		ASSERT_NOT_NULL__(context);
		Ctr::Array<String,Adopt>	components;
		Explode('/',path,components);
		if (!components.Count())
			throw Except::IO::ParameterFault("Trying to create empty XML segment");

		for (index_t i = 0; i < components.Count()-1; i++)
		{
			index_t next = InvalidIndex;
			for (index_t j = 0; j < context->children.Count() && next == InvalidIndex; j++)
				if (context->children[j].name == components[i])
					next = j;

			if (next == InvalidIndex)
			{
				context = &context->children.Append();
				context->name = components[i];
			}
			else
				context = &context->children[next];
		}
		ASSERT_NOT_NULL__(context);
		context = &context->children.Append();
		context->name = components.Last();
		context->inner_content = inner_content;
		return *context;
	}




	XML::Node* XML::Node::Find(const String&path)
	{
		return findIn(children,path.ref());
	}

	const XML::Node* XML::Node::Find(const String&path) const
	{
		return findInConst(children,path.ref());
	}

	XML::Node* XML::Node::Find(const StringRef&path)
	{
		return findIn(children,path);
	}

	const XML::Node* XML::Node::Find(const StringRef&path) const
	{
		return findInConst(children,path);
	}


	XML::Node*	XML::Container::Find(const String&path)
	{
		return findFrom(&root_node,path.ref());
	}

	const XML::Node*	XML::Container::Find(const String&path) const
	{
		return findFromConst(&root_node,path.ref());
	}


	XML::Node&	XML::Node::Create(const String&path, const String&inner_content)
	{
		ASSERT_NOT_NULL__(this);	//kinda dumb, no?
		return createIn(this, path, inner_content);
	}
	XML::Node&	XML::Container::Create(const String&path, const String&inner_content)
	{
		String local,sub;
		index_t p = path.Find('/');
		if (p != InvalidIndex)
		{
			local = path.subString(0,p);
			sub = path.subString(p+1);
		}
		else
			local = path;
	
		if (root_node.name=="xml")	//!< empty name
		{
			root_node.name = local;
		}
		elif (root_node.name != local)
			throw Except::IO::ParameterFault("Path component mismatch: '"+local+"'");

		if (sub.IsEmpty())
			return root_node;

		return createIn(&root_node, sub, inner_content);
	}


	XML::ScannerRule::ScannerRule(const String&name, bool want_content_, pNodeCallback on_enter_, pNodeCallback on_exit_):
									name(name),want_content(want_content_),on_enter(on_enter_),on_exit(on_exit_)	{}
								
	XML::PScannerRule	XML::ScannerRule::Map(const String&name, bool want_content,pNodeCallback on_enter, pNodeCallback on_exit)
	{
		PScannerRule result;
		WScannerRule wresult;
		if (connections.Query(name,wresult))
		{
			result = wresult.lock();
			result->want_content = want_content;
			result->on_enter = on_enter;
			result->on_exit = on_exit;
			return result;
		}
		result.reset(new ScannerRule(name,want_content,on_enter,on_exit));
		connections.Set(name,result);
		return result;
	}

	XML::PScannerRule	XML::ScannerRule::Map(const PScannerRule&node)
	{
		connections.Set(node->name,node);
		return node;
	}

	bool	XML::ScannerRule::Unmap(const String&name)
	{
		return connections.Unset(name);
	}

	bool	XML::ScannerRule::Unmap(ScannerRule*node)
	{
		return connections.Unset(node->name);
	}
	
	
	bool	XML::Scanner::read(char until, StringBuffer&to, FILE*f)
	{
		to.reset();
		while (true)
		{
			if (buffer_fill_state)
			{
				char*offset = strnchr(buffer,buffer_fill_state,until);
				if (offset)
				{
					to.Write(buffer,offset-buffer);
					memcpy(buffer,offset+1,buffer_fill_state-(offset-buffer)-1);
					buffer_fill_state -= (offset-buffer)+1;
					return true;
				}
				else
				{
					to.Write(buffer,buffer_fill_state);
					buffer_fill_state = 0;
				}
			}
			buffer_fill_state = fread(buffer,1,sizeof(buffer),f);
			if (!buffer_fill_state)
				return false;
		}
		return false;
	}


	bool	XML::Scanner::skip(char until, FILE*f)
	{
		while (true)
		{
			if (buffer_fill_state)
			{
				char*offset = strnchr(buffer,buffer_fill_state,until);
				if (offset)
				{
					memcpy(buffer,offset+1,buffer_fill_state-(offset-buffer)-1);
					buffer_fill_state -= (offset-buffer)+1;
					return true;
				}
			}
			buffer_fill_state = fread(buffer,1,sizeof(buffer),f);
			if (!buffer_fill_state)
				return false;
		}
		return false;
	}

	bool	XML::Scanner::skip(const char*until, FILE*f)
	{
		size_t len = strlen(until);
		while (true)
		{
			if (buffer_fill_state)
			{
				char*offset = strnstr(buffer,until,buffer_fill_state),
					*first = strnchr(buffer,buffer_fill_state,until[0]);
				if (offset)
				{
					memcpy(buffer,offset+1,buffer_fill_state-(offset-buffer)-1);
					buffer_fill_state -= (offset-buffer)+1;
					return true;
				}
				elif (first && (unsigned)(first-buffer) > (unsigned)(buffer_fill_state-len))
				{
					memcpy(buffer,first,buffer_fill_state-(first-buffer)-1);
					buffer_fill_state -= (first-buffer)+1;
				}
				else
					buffer_fill_state = 0;
			}
			size_t delta = fread(buffer+buffer_fill_state,1,sizeof(buffer)-buffer_fill_state,f);
			buffer_fill_state += delta;
			if (!delta)
				return false;
		}
		return false;
	}

	XML::Scanner::Scanner(Encoding hostEncoding):root(new ScannerRule("xml:document",false,NULL,NULL)),except(NULL),buffer_fill_state(0),hostEncoding(hostEncoding)
	{}


	const String&	XML::Scanner::GetError()						const
	{
		return error_string;
	}


	void	XML::Scanner::SetExcept(ScannerRule::pNodeCallback callback_on_except)
	{
		except = callback_on_except;
	}

	bool	XML::Scanner::Scan(const PathString&filename)
	{
		#if SYSTEM==WINDOWS
			FILE*f = _wfopen(filename.c_str(),L"rb");
		#else
			FILE*f = fopen(filename.c_str(),"rb");
		#endif
		if (!f)
		{
			error_string = "Unable to open file '"+String(filename)+"'";
			error_is_filenotfound = true;
		}
		error_is_filenotfound = false;

		XML::Encoding encoding = XML::Encoding::UTF8;
	
		Vector<TScannerNode>	stack;
		{
			TScannerNode&current = stack.Append();
			current.rule = root;
		}
		PScannerRule rule = root;
	
		StringBuffer	buffer,tempBuffer;
		bool 	error = false,
				working = true;
	
		if (!skip('<',f))
		{
			error = true;
			working = false;
			error_string = "Empty or non XML file.";
		}	
		while (working)
		{
			//cursor is now behind the '<'-character of the new tag.
			if (!read('>',buffer,f) || !buffer.GetLength())	//read out all data until the end of the tag
			{
				error = true;
				working = false;
				error_string = "Unexpected end of file.";
			}
		
			char*field = buffer.pointer(),						//extract working buffer
				*c = field,
				*end = field+buffer.GetLength();
			while (!IsWhitespace(*c) && c != end)	//find end of name
				c++;
			unsigned name_len = c-field;
			field[name_len] = 0;	//zero terminate name
			if (field[0] == '?')	//skip xml header tags for now
			{
				if (!skip('<',f))	//go to next tag
				{
					error = stack.Count()!=1;
					working = false;
					error_string = "Unexpected end of file.";
				}
				continue;
			}
			elif (field[0] == '!' && field[1] == '-' && field[2] == '-')
			{
				if (!skip("-->",f))
				{
					error = stack.Count() != 1;
					working = false;
					error_string = "Unexpected end of file.";
				}
			}
			
			if (field[0] == '/')	//closing tag
			{
				String tag_name = field+1;
				if (stack.Last().rule && tag_name != stack.Last().rule->name)	//tag hierarchy broken
				{
					error_string = "Closing tag '"+tag_name+"' does not match opening tag '"+stack.Last().rule->name+"' of this level";
					error = true;
					working = false;
					continue;
				}
				if (rule && rule->on_exit)
					rule->on_exit(stack);		//execute rule
				stack.Erase(stack.Count()-1);	//pop top stack element
				rule = stack.Last().rule;
			
			
				if (!skip('<',f))
				{
					error = stack.Count()!=1;
					working = false;
					error_string = "Unexpected end of file.";
				}
			
				continue;
			}
		
			String tag_name = field;
			WScannerRule wnext;
			PScannerRule next;
			if (rule && rule->connections.Query(tag_name,wnext))
				next = wnext.lock();

			{
				TScannerNode&inner = stack.Append();
				inner.rule = next;
			}
			//read attributes
			char*name_offset = field+name_len+1;
			if (next && name_offset < end)
				while (char*attrib = strnchr(name_offset,end-name_offset,'='))
				{
					while (IsWhitespace(*name_offset))
						name_offset++;
					(*attrib) = 0;
					const char*name_end = attrib;
					attrib++;
					field[buffer.GetLength()] = 0;
					char quote = *attrib;
					if ((quote != '\"' && quote != '\'') || attrib-name_offset<2)
					{
						error_string = "Attribute '"+String(name_offset)+"' corrupted ('\"' or '\'' expected but '"+String(*attrib)+"' encountered).";
						working = false;
						error = true;
						break;
					}
					char*data_offset = ++attrib;
					while ((*attrib)!=quote && attrib != end)
						attrib++;
					if (attrib == end)
					{
						error_string = "Attribute quotation not closed.";
						working = false;
						error = true;
						break;
					}
					(*attrib) = 0;
					stack.Last().attributes
						.Set(
							DecodeWord(StringRef(name_offset,name_end - name_offset),encoding,hostEncoding,tempBuffer),
							DecodeValue(StringRef(data_offset,attrib - data_offset),encoding,hostEncoding,tempBuffer)
						);

					name_offset = attrib+1;
				}
			bool do_enter = field[buffer.GetLength()-1] != '/';
		
			if ((next && next->want_content)||(!next && except))
			{
				if (!read('<',buffer,f))
				{
					error = true;
					working = false;
					error_string = "Unexpected end of file.";
				}
				stack.Last().content = DecodeValue(buffer.ToStringRef(),encoding,hostEncoding,tempBuffer);
			}
			else
				if (!skip('<',f))
				{
					error = true;
					working = false;
					error_string = "Unexpected end of file.";
				}
		
			if (next)
			{
				if (next->on_enter)
					next->on_enter(stack);
			}
			else
				if (except)
					except(stack);
		
			if (do_enter)
			{
				rule = next;
			}
			else
			{
				if (next && next->on_exit)
					next->on_exit(stack);
				stack.EraseLast();
			}
		}
	
		fclose(f);
		if (!error && stack.Count()!=1)
		{
			error = true;
			error_string = "Unexpected end of file.";
		}
		return !error;
	}
}

