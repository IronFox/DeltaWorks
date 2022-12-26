#include "json.h"
#include <string/string_buffer.h>

namespace DeltaWorks
{

	namespace JSON
	{
		typedef StringEncoding::UTF8::TChar		UTF8Char;
		typedef StringEncoding::UTF16::TChar	UTF16Char;

		String Entity::CastToString() const
		{
			switch (type)
			{
				case Int64:
					return String(intBoolValue);
				case Double:
					return String(doubleValue);
				case StringValue:
					return stringValue;
				case Bool:
					return String(intBoolValue!=0);
				default:
					return "";	//no useful conversion
			}
		}
		INT64 Entity::CastToInt() const
		{
			switch (type)
			{
				case Int64:
				case Bool:
					return intBoolValue;
				case Double:
					return (INT64)doubleValue;
				case StringValue:
				{
					INT64 rs=0;
					convert(stringValue.c_str(),stringValue.GetLength(),rs);
					return rs;
				}
				default:
					return 0;	//no useful conversion
			}	
		}

		double Entity::CastToDouble() const
		{
			switch (type)
			{
				case Int64:
				case Bool:
					return (double)intBoolValue;
				case Double:
					return doubleValue;
				case StringValue:
				{
					double rs=0;
					convert(stringValue.c_str(),stringValue.GetLength(),rs);
					return rs;
				}
				default:
					return 0;	//no useful conversion
			}	
		}

		void Entity::Clear()
		{
			stringValue.Clear();
			arrayValue.Clear();
			objectValue.reset();
			intBoolValue = 0;
			doubleValue = 0;
			type = Null;
		}

		Object & Entity::SetObject()
		{
			Clear();
			type = ObjectValue;
			objectValue.reset(new Object());
			return *objectValue;
		}

		bool Matches(const UTF8Char&c0, char c1)
		{
			return c0.numCharsUsed == 1 && c0.encoded[0] == c1;
		}


		static String		ReadString(IReadStream&stream, LineCounter&lineCounter)
		{
			bool escaped = false;
			StringBuffer temp;
			for(;;)
			{
				char ch;
				stream.ReadPrimitive(ch);
				switch (ch)
				{
					case '"':
						if (!escaped)
							return temp.CopyToString();
						break;
					case '\\':
						if (escaped)
							break;
						escaped = true;
						continue;
					case 'u':
						if (escaped)
						{
							escaped = false;
							UTF16Char ch16;
							char32_t utf32 = 0;
							for (int c = 0; c < 2; c++)
							{
								if (c == 1)
								{
									stream.ReadPrimitive(ch);
									if (ch != '\\')
										throw Except::Program::DataConsistencyFault(CLOCATION,"\\uXXXX\\uYYYY sequence expected intermediate \\u in line "+String(lineCounter.inLine));
									stream.ReadPrimitive(ch);
									if (ch != 'u')
										throw Except::Program::DataConsistencyFault(CLOCATION,"\\uXXXX\\uYYYY sequence expected intermediate \\u in line "+String(lineCounter.inLine));
								}
								for (int k = 0; k < 4; k++)
								{
									stream.ReadPrimitive(ch);
								
									ch16.encoded[c] <<= 4;
									if (ch >= '0' && ch <= '9')
									{
										ch16.encoded[c] |= (ch - '0');
										continue;
									}
									if (ch >= 'a' && ch <= 'f')
									{
										ch16.encoded[c] |= ((ch - 'a')+10);
										continue;
									}
									if (ch >= 'A' && ch <= 'F')
									{
										ch16.encoded[c] |= ((ch - 'a')+10);
										continue;
									}
									throw Except::Program::DataConsistencyFault(CLOCATION,"\\uXXXX\\uYYYY sequence expected four hexadecimal number characters, but got non-hexadecimal number in sequence in line "+String(lineCounter.inLine));
								}
								ch16.numCharsUsed ++;

								if (StringEncoding::IsValid(ch16))
									break;
							}
							UTF8Char ch8;
							StringEncoding::UTF32::ToUTF8(StringEncoding::UTF16::ToUTF32(ch16),ch8);
							for (BYTE i = 0; i < ch8.numCharsUsed; i++)
								temp << ch8.encoded[i];

							continue;
						}
						break;
				}
				temp << ch;
				escaped = false;
			}
		}


		static void					DeserializePrimitive(Entity&e,char firstChar,IReadStream&stream,LineCounter&counter)
		{
			StringBuffer word;
			word << firstChar;
			for (;;)
			{
				char ch;
				stream.ReadPrimitive(ch);
				counter.Feed(ch);
				if ((IsWhitespace(ch)) || ch == ',')
					break;
				word << ch;
			}

			StringRef myWord = word.ToStringRef();
			bool b;
			INT64 in;
			double d;
			if (myWord == "null")
				e.SetNull();
			elif (convert(myWord.pointer(),myWord.GetLength(),b))
				e.Set(b);
			elif (convert(myWord.pointer(),myWord.GetLength(),in))
				e.Set(in);
			elif (convert(myWord.pointer(),myWord.GetLength(),d))
				e.Set(d);
			else
				throw Except::Program::DataConsistencyFault(CLOCATION,"'"+String(myWord)+"' is no valid JSON value");
		}

		static	void				DeserializeArray(Ctr::Array<Entity>&target, IReadStream&stream,LineCounter&lineCounter);

		static void DeserializeAny(Entity&e, char firstChar, IReadStream&stream,LineCounter&counter)
		{
			if (firstChar == '"')
				e.Set(ReadString(stream,counter));
			elif (firstChar == '{')
				e.SetObject().Deserialize(stream,counter);
			elif (firstChar == '[')
				DeserializeArray(e.SetArray(),stream,counter);
			else
				DeserializePrimitive(e,firstChar,stream,counter);
		}


		char SearchFor(IReadStream&stream, const std::function<bool(char)>&f, LineCounter&lineCounter)
		{
			for (;;)
			{
				char ch;
				stream.ReadPrimitive(ch);
				lineCounter.Feed(ch);
				if (f(ch))
					return ch;
			}
		}
		char SearchFor(IReadStream&stream, char c, LineCounter&lineCounter)
		{
			for (;;)
			{
				char ch;
				stream.ReadPrimitive(ch);
				lineCounter.Feed(ch);
				if (ch == c)
					return ch;
			}
		}

		static	void				DeserializeArray(Ctr::Array<Entity>&target, IReadStream&stream,LineCounter&lineCounter)
		{
			Container::Vector<Entity,Swap> tmp;
			char ch;
			for (;;)
			{
				for (;;)
				{
					stream.ReadPrimitive(ch);
					lineCounter.Feed(ch);
					if (!IsWhitespace(ch))	//search through whitespace
						break;
				}
				if (ch == ']')
				{
					tmp.MoveToArray(target);
					return;
				}

				Entity&e = tmp.Append();

				DeserializeAny(e,ch,stream,lineCounter);

				char ch = SearchFor(stream,[](char ch){return ch == ',' || ch == ']';},lineCounter);
				if (ch == ']')
				{
					tmp.MoveToArray(target);
					return;
				}
			}
		}


		void Object::Deserialize(IReadStream & stream)
		{
			LineCounter counter;
			SearchFor(stream,'{',counter);
			Deserialize(stream,counter);
		}

		void Object::Deserialize(IReadStream & stream,LineCounter&lineCounter)
		{
		
			//SearchForASCII(stream,'{',counter);
		
			for (;;)
			{
				char ch = SearchFor(stream,[](char ch){return ch == '\"' || ch == '}';},lineCounter);
				if (ch == '}')
					return;
				String key = ReadString(stream,lineCounter);
				SearchFor(stream,':',lineCounter);
				for (;;)
				{
					stream.ReadPrimitive(ch);
					lineCounter.Feed(ch);
					if (!IsWhitespace(ch))	//search through whitespace
						break;
				}

				Entity&e = entities.Set(key);

				DeserializeAny(e,ch,stream,lineCounter);

				ch = SearchFor(stream,[](char ch){return ch == ',' || ch == '}';},lineCounter);
				if (ch == '}')
					return;
			}
		}
	}
}
