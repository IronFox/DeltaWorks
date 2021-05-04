#include "encoding.h"

namespace DeltaWorks
{

	namespace StringEncoding
	{

		bool	IsValidUnicode(char32_t c)
		{
			const UINT32 u = (UINT32)c;

			return u < 0xD800 || (u>=0xE000 && u <= 0x10FFFF);
		}

		static bool	CompleteUtf16(char16_t c, char32_t&out)
		{
			UINT16 header = ((UINT16)c) & 0xFC00;
			if (header != 0xD800)
			{
				out = c;
				return true;
			}
			return false;
		}

		static char32_t	Utf16ToUTF32Char(char16_t c0, char16_t c1)
		{
			char32_t rs;
			UINT16 header0 = ((UINT16)c0) & 0xFC00;
			UINT16 header1 = ((UINT16)c1) & 0xFC00;
			if (header0 != 0xD800)
				throw Except::Program::DataConsistencyFault(CLOCATION,"Invalid UTF16 header #"+String(header0));
			if (header1 != 0xDC00)
				throw Except::Program::DataConsistencyFault(CLOCATION,"Invalid UTF16 header #"+String(header0));
			UINT16 high = (UINT16)c0 - 0xD800;
			UINT16 low = (UINT16)c1 - 0xDC00;

			UINT32 combined = (((UINT32)high) << 10) | low;
			return (char32_t)(combined + 0x10000);
		}

	
		template <typename Base>
			struct UTF16Code
			{
				typedef Base		CharType;
				typedef UTF16::TChar	EncodedType;


				static String	GetName() {return "UTF16";}

				static count_t	UTF32ToEncodedLength(char32_t utf32)
				{
					if (!IsValidUnicode(utf32))
						throw Except::Program::DataConsistencyFault(CLOCATION, GetName()+" encoding error. Given UTF32 character #"+String(utf32)+" is not valid");

					if ((UINT32)utf32 <= 0xFFFF)
						return 1;
					return 2;
				}


				static void		UTF32ToEncoded(char32_t utf32, EncodedType&rs)
				{
					UINT32 uni = utf32;
					if (!IsValidUnicode(utf32))
						throw Except::Program::DataConsistencyFault(CLOCATION, GetName()+" encoding error. Given UTF32 character #"+String(utf32)+" is not valid");
					if (uni <= 0xFFFF)
					{
						rs.numCharsUsed = 1;
						rs.encoded[0] = (Base)uni;
					}
					else
					{
						uni -= 0x10000;
						rs.numCharsUsed = 2;
						rs.encoded[0] = (Base)((uni >> 10) + 0xD800);
						rs.encoded[1] = (Base)((uni & 0x3FF) + 0xDC00);
					}
					ASSERT_EQUAL__(DecipherVariableLength(rs.encoded[0]),rs.numCharsUsed);
				}

				static BYTE		DecipherVariableLength(CharType c)
				{
					UINT16 header = ((UINT16)c) & 0xFC00;
					if (header != 0xD800)
						return 1;
					return 2;
				}

				static char32_t		DeserializeUTF32(const CharType*&source)
				{
					UTF16::TChar ch;
					ch.numCharsUsed = DecipherVariableLength(*source);
					memcpy(ch.encoded,source,ch.numCharsUsed*sizeof(Base));
					source += ch.numCharsUsed;
					return UTF16::ToUTF32(ch);
				}

				static char32_t		DeserializeUTF32(const CharType*&source, const CharType*const end)
				{
					UTF16::TChar ch;
					ch.numCharsUsed = DecipherVariableLength(*source);
					if (source + ch.numCharsUsed > end)
						throw Except::Program::DataConsistencyFault(CLOCATION, GetName()+" decoding error. String incomplete");
					memcpy(ch.encoded,source,ch.numCharsUsed*sizeof(Base));
					source += ch.numCharsUsed;
					return UTF16::ToUTF32(ch);
				}
			};


		struct UTF8Code
		{
			typedef char		CharType;
			typedef UTF8::TChar	EncodedType;

			static String	GetName() {return "UTF8";}


			static count_t	UTF32ToEncodedLength(char32_t utf32)
			{
				return UTF32::ToUTF8Length(utf32);
			}

			static void		UTF32ToEncoded(char32_t utf32, EncodedType&rs)
			{
				StringEncoding::UTF32::ToUTF8(utf32,rs);
			}

			static BYTE		DecipherVariableLength(CharType c)
			{
				return UTF8::GetLength(c);
			}

			static char32_t		DeserializeUTF32(const CharType*&source)
			{
				UTF8::TChar ch;
				ch.numCharsUsed = DecipherVariableLength(*source);
				memcpy(ch.encoded,source,ch.numCharsUsed);
				source += ch.numCharsUsed;
				return UTF8::ToUTF32(ch);
			}

			static char32_t		DeserializeUTF32(const CharType*&source, const CharType*const end)
			{
				UTF8::TChar ch;
				ch.numCharsUsed = DecipherVariableLength(*source);
				if (source + ch.numCharsUsed > end)
					throw Except::Program::DataConsistencyFault(CLOCATION, GetName()+" decoding error. String incomplete");
				memcpy(ch.encoded,source,ch.numCharsUsed);
				source += ch.numCharsUsed;
				return UTF8::ToUTF32(ch);
			}
		};



		struct UTF32FixedCode
		{
			typedef char32_t	CharType;

			static char32_t	ConvertUTF32(char32_t c) {return c;}
			static char32_t	ConvertToUTF32(char32_t c) {return c;}
		};

		struct CP1252FixedCode
		{
			typedef char	CharType;

			static char		ConvertUTF32(char32_t c) {return UTF32::ToCP1252(c);}
			static char32_t	ConvertToUTF32(char c) {return CP1252::ToUTF32(c);}
		};


		template <typename Code0, typename Code1>
			static void	VariableLengthToVariableLength(const StringType::ReferenceExpression<typename Code0::CharType>&variableSource, StringType::Template<typename Code1::CharType>&variableDest)
			{
				count_t len = 0;
				const auto*const end = variableSource.end();
				{
					const auto*read = variableSource.pointer();
					while (read < end)
					{

						auto ch = Code0::DeserializeUTF32(read,end);
						len += Code1::UTF32ToEncodedLength(ch);
					}
				}

				variableDest.setLength(len);
				{
					const auto*read = variableSource.pointer();
					auto*out = variableDest.mutablePointer();
					while (read < end)
					{
						typename Code1::EncodedType encoded;
						Code1::UTF32ToEncoded( Code0::DeserializeUTF32(read),encoded);
						for (BYTE k = 0; k < encoded.numCharsUsed; k++)
						{
							*out = encoded.encoded[k];
							out++;
						}
					}
					ASSERT_EQUAL__(out,variableDest.end());
				}
			}

		template <typename FixedCode, typename Code>
			static void	FixedLengthToVariableLength(const StringType::ReferenceExpression<typename FixedCode::CharType>&fixedSource, StringType::Template<typename Code::CharType>&variableDest)
			{
				count_t len = 0;

				auto*s = fixedSource.pointer(),
					*const end = fixedSource.end();
				for (; s != end; ++s)
					len += Code::UTF32ToEncodedLength( FixedCode::ConvertToUTF32(*s) );

				variableDest.setLength(len);
				s = fixedSource.pointer();
				auto*out = variableDest.mutablePointer();
				for (; s != end; ++s)
				{
					typename Code::EncodedType encoded;
					Code::UTF32ToEncoded( FixedCode::ConvertToUTF32(*s),encoded);
					for (BYTE k = 0; k < encoded.numCharsUsed; k++)
					{
						*out = encoded.encoded[k];
						out++;
					}
				}
				ASSERT_EQUAL__(out,variableDest.end());
			}


		template <typename Code, typename FixedCode>
			static void	VariableLengthToFixedLength(const StringType::ReferenceExpression<typename Code::CharType>&variableSource, StringType::Template<typename FixedCode::CharType>&fixedDest)
			{
				count_t length = 0;
				{
					const auto*read = variableSource.pointer();
					const auto*const end = variableSource.end();
					while (read < end)
					{
						count_t len = Code::DecipherVariableLength(*read);
						read += len;
						if (read > end)
							throw Except::Program::DataConsistencyFault(CLOCATION, Code::GetName()+" decoding error. String incomplete");
						length++;
					}
				}

				fixedDest.setLength(length);

				auto*write = fixedDest.mutablePointer();
				{
					const auto*read = variableSource.pointer();
					const auto*const end = variableSource.end();
					while (read < end)
					{
						*write = FixedCode::ConvertUTF32( Code::DeserializeUTF32(read) );
						write++;
					}
					ASSERT_EQUAL__(read,end);
				}
				ASSERT_EQUAL__(write,fixedDest.end());
			}



		bool	Is1252MultiByte(char csource)
		{
			return ((signed char)csource) < 0;
		}

		bool	UTF8::ToUTF32(const char*&begin, const char*const end, char32_t&outChar)
		{
			if (begin >= end)
				return false;
			const BYTE length = GetLength(*begin);
			if (begin + length > end || length == 0)
				return false;

			unsigned char first = (BYTE) *begin;


			UINT32 rs = first & ((0xFF>>(length)));

			for (index_t at = 1; at < length; at++)
			{
				if ((begin[at] & 0xC0) != 0x80)
					throw Except::Program::DataConsistencyFault(CLOCATION, "UTF8 Encoding error. Expected following byte "+String(at)+" to contain 10xxxxxx header (encountered "+DataToBinary(&begin[at],1)+")");
				rs <<= 6;
				rs |= (begin[at] & 0x3F);
			}
			
			begin += length;
			outChar = (char32_t)rs;
			return true;
		}


		BYTE	UTF8::GetLength(char firstChar)
		{
			unsigned char usource = firstChar;
			BYTE numOnes = 0;
			while (usource & 0x80)	//look for first bit that is zero (length-terminator). may be first bit
			{
				usource <<= 1;	//otherwise shift up (replace test-bit with next in line)
				numOnes ++;	//increment length
			}
			if (numOnes == 1)	//not defined. encodings always either start with 0* or 11*, never 10* for the first char
				throw Except::Program::DataConsistencyFault(CLOCATION, "Invalid leading UTF-8 character "+DataToBinary(&firstChar,1)); //make sure nothing went wrong. In theory the length _could_ be 1+8, if the byte is 255, but that is not utf8
		
			const BYTE length = std::max<BYTE>(numOnes,1);
			if ((count_t)length > ARRAYSIZE(UTF8::TChar::encoded))
				throw Except::Program::DataConsistencyFault(CLOCATION, "Invalid leading UTF-8 character "+DataToBinary(&firstChar,1)); //make sure nothing went wrong. In theory the length _could_ be 1+8, if the byte is 255, but that is not utf8
			return length;
		}


		void	Deserialize(IReadStream&source, UTF8::TChar&utf8Dest)
		{
			count_t len = 0;
			unsigned char first;
			source.Read(&first,1);

			BYTE length = UTF8::GetLength(first);
			utf8Dest.numCharsUsed = length;
			utf8Dest.encoded[0] = first;
			if (length > 1)
				source.Read(utf8Dest.encoded+1,length-1);
		}



		char32_t	CP1252::ToUTF32(char c)
		{
			if ((BYTE)c < 128 || (BYTE)c >= 160)
				return (char32_t)(BYTE)c;
			switch ((BYTE)c)
			{
				case 128:
					return 0x20AC;
				case 130:
					return 0x201A;
				case 131:
					return 0x0192;
				case 132:
					return 0x201E;
				case 133:
					return 0x2026;
				case 134:
					return 0x2020;
				case 135:
					return 0x2021;
				case 136:
					return 0x02C6;
				case 137:
					return 0x2030;
				case 138:
					return 0x0160;
				case 139:
					return 0x2039;
				case 140:
					return 0x0152;
				case 142:
					return 0x017D;
				case 145:
					return 0x2018;
				case 146:
					return 0x2019;
				case 147:
					return 0x201C;
				case 148:
					return 0x201D;
				case 149:
					return 0x2022;
				case 150:
					return 0x2013;
				case 151:
					return 0x2014;
				case 152:
					return 0x02DC;
				case 153:
					return 0x2122;
				case 154:
					return 0x0161;
				case 155:
					return 0x203A;
				case 156:
					return 0x0153;
				case 158:
					return 0x017E;
				case 159:
					return 0x0178;
				default:
					return (char32_t)(BYTE)c;
			};
		}
		
		bool	UTF32::ToCP1252(char32_t c, char&outChar)
		{
			if ((UINT32)c < 0x100)
			{
				outChar = (char)(BYTE)(UINT32)c;
				return true;
			}
			switch ((UINT32)c)
			{
				case 0x20AC:
					outChar = (char)(BYTE)128;
					break;
				case 0x201A:
					outChar = (char)(BYTE)130;
					break;
				case 0x0192:
					outChar = (char)(BYTE)131;
					break;
				case 0x201E:
					outChar = (char)(BYTE)132;
					break;
				case 0x2026:
					outChar = (char)(BYTE)133;
					break;
				case 0x2020:
					outChar = (char)(BYTE)134;
					break;
				case 0x2021:
					outChar = (char)(BYTE)135;
					break;
				case 0x02C6:
					outChar = (char)(BYTE)136;
					break;
				case 0x2030:
					outChar = (char)(BYTE)137;
					break;
				case 0x0160:
					outChar = (char)(BYTE)138;
					break;
				case 0x2039:
					outChar = (char)(BYTE)139;
					break;
				case 0x0152:
					outChar = (char)(BYTE)140;
					break;
				case 0x017D:
					outChar = (char)(BYTE)142;
					break;
				case 0x2018:
					outChar = (char)(BYTE)145;
					break;
				case 0x2019:
					outChar = (char)(BYTE)146;
					break;
				case 0x201C:
					outChar = (char)(BYTE)147;
					break;
				case 0x201D:
					break;
					outChar = (char)(BYTE)148;
					break;
				case 0x2022:
					outChar = (char)(BYTE)149;
					break;
				case 0x2013:
					outChar = (char)(BYTE)150;
					break;
				case 0x2014:
					outChar = (char)(BYTE)151;
					break;
				case 0x02DC:
					outChar = (char)(BYTE)152;
					break;
				case 0x2122:
					outChar = (char)(BYTE)153;
					break;
				case 0x0161:
					outChar = (char)(BYTE)154;
					break;
				case 0x203A:
					outChar = (char)(BYTE)155;
					break;
				case 0x0153:
					outChar = (char)(BYTE)156;
					break;
				case 0x017E:
					outChar = (char)(BYTE)158;
					break;
				case 0x0178:
					outChar = (char)(BYTE)159;
					break;
				default:
					if ((UINT32)c > 255)
						return false;
					outChar = (char)(BYTE)(UINT32)c;
					break;

			};

			return true;

		}

		char	UTF32::ToCP1252(char32_t c)
		{
			char rs;
			if (!ToCP1252(c,rs))
				throw Except::Program::DataConsistencyFault(CLOCATION, "UTF32 character is out of range for CP-1252");
			return rs;
		}

		char UTF8::ToCP1252(const UTF8::TChar&ch)
		{
			if (!ch.numCharsUsed)
				return 0;
			return UTF32::ToCP1252(
					UTF8::ToUTF32(ch));
		}

		void	CP1252::ToUTF8(char c, UTF8::TChar&ch)
		{
			UTF32::ToUTF8(
				CP1252::ToUTF32(c),ch);
		}


		UTF32String	UTF8::ToUTF32(const String&utf8Source)
		{
			UTF32String rs;
			ToUTF32(utf8Source,rs);
			return rs;
		}


		void	UTF8::ToUTF32(const String&utf8Source, UTF32String&utf32Dest)
		{
			UTF8::ToUTF32(utf8Source.ref(),utf32Dest);
		}
		void	UTF32::ToUTF8(const StringType::ReferenceExpression<char32_t>&utf32Source, String&utf8Dest)
		{
			FixedLengthToVariableLength<UTF32FixedCode,UTF8Code>(utf32Source,utf8Dest);
		}

		void	UTF32::ToUTF8(const UTF32String&utf32Source, String&utf8Dest)
		{
			UTF32::ToUTF8(utf32Source.ref(),utf8Dest);
		}

		String	UTF32::ToUTF8(const UTF32StringRef&utf32Source)
		{
			String rs;
			ToUTF8(utf32Source,rs);
			return rs;
		}




		void	UTF8::ToUTF32(const StringRef&utf8Source, UTF32String&utf32Dest)
		{
			VariableLengthToFixedLength<UTF8Code,UTF32FixedCode>(utf8Source,utf32Dest);
		}





		char32_t	UTF16::ToUTF32(const UTF16::TChar&c)
		{
			if (c.numCharsUsed == 0)
				return 0;

			if (c.numCharsUsed == 1)
			{
				char32_t rs;
				if (!CompleteUtf16(c.encoded[0],rs))
					throw Except::Program::DataConsistencyFault(CLOCATION,"Invalid UTF16 character "+BinaryToHex(&c.encoded[0],2));
				return rs;
			}

			return Utf16ToUTF32Char(c.encoded[0],c.encoded[1]);
		}

		bool	IsValid(const UTF16::TChar&c)
		{
			if (c.numCharsUsed == 0)
				return true;

			if (c.numCharsUsed == 1)
			{
				char32_t rs;
				return CompleteUtf16(c.encoded[0],rs);
			}

			UINT16 header0 = ((UINT16)c.encoded[0]) & 0xFC00;
			UINT16 header1 = ((UINT16)c.encoded[1]) & 0xFC00;

			return header0 == 0xD800 && header1 == 0xDC00;
		}

		void	AssertValidity(const UTF16::TChar&c)
		{
			if (c.numCharsUsed == 0)
				return;

			if (c.numCharsUsed == 1)
			{
				UINT16 header = ((UINT16)c.encoded[0]) & 0xFC00;
				if (header != 0xD800)
					return;
				throw Except::Program::DataConsistencyFault(CLOCATION,"Only encoded UTF16 character contains reserved header");
			}

			UINT16 header0 = ((UINT16)c.encoded[0]) & 0xFC00;
			UINT16 header1 = ((UINT16)c.encoded[1]) & 0xFC00;
			if (header0 != 0xD800)
				throw Except::Program::DataConsistencyFault(CLOCATION,"First encoded UTF16 character contains invalid header");
			if (header1 != 0xDC00)
				throw Except::Program::DataConsistencyFault(CLOCATION,"Second encoded UTF16 character contains invalid header");
		}

		bool	IsValid(const UTF8::TChar&c)
		{
			if (c.numCharsUsed == 0)
				return true;

			const BYTE length = UTF8::GetLength(c.encoded[0]);
			if (length != c.numCharsUsed)
				return false;
			for (BYTE k = 1; k < c.numCharsUsed; k++)
				if ((c.encoded[k] & 0xC0) != 0x80)
					return false;
			return true;
		}
	
		void	AssertValidity(const UTF8::TChar&c)
		{
			if (c.numCharsUsed == 0)
				return;

			const BYTE length = UTF8::GetLength(c.encoded[0]);
			if (length != c.numCharsUsed)
				throw Except::Program::DataConsistencyFault(CLOCATION, "UTF8 Encoding error. Expected "+String(length)+" bytes in encoded string, but got "+String(c.numCharsUsed));
			for (BYTE k = 1; k < c.numCharsUsed; k++)
				if ((c.encoded[k] & 0xC0) != 0x80)
					throw Except::Program::DataConsistencyFault(CLOCATION, "UTF8 Encoding error. Expected following byte "+String(k)+" to contain 10xxxxxx header");
		}


		char32_t	UTF8::ToUTF32(const UTF8::TChar&c)
		{
			if (c.numCharsUsed == 0)
				return 0;
			count_t len = 0;
			unsigned char first = (BYTE) c.encoded[0];
		

			const BYTE length = UTF8::GetLength(first);
			if (length != c.numCharsUsed)
				throw Except::Program::DataConsistencyFault(CLOCATION, "UTF8 Encoding error. Expected "+String(length)+" bytes in encoded string, but got "+String(c.numCharsUsed));


			UINT32 rs = first & ((0xFF>>(length)));

			for (index_t at = 1; at < length; at++)
			{
				if ((c.encoded[at] & 0xC0) != 0x80)
					throw Except::Program::DataConsistencyFault(CLOCATION, "UTF8 Encoding error. Expected following byte "+String(at)+" to contain 10xxxxxx header (encountered "+DataToBinary(&c.encoded[at],1)+")");
				rs <<= 6;
				rs |= (c.encoded[at] & 0x3F);
			}
		
			return (char32_t)rs;
		}



		BYTE	UTF32::ToUTF8Length(char32_t c)
		{
			UINT32 u = (UINT32)c;
			if (u <= 0x007F)
				return 1;
			if (u <= 0x07FF)
				return 2;
			if (u <= 0xFFFF)
				return 3;
			if (u <= 0x10FFFF)
				return 4;
			throw Except::Program::DataConsistencyFault(CLOCATION,"Invalid UTF32 character "+BinaryToHex(&c,4));
		}

		void	UTF32::ToUTF8(char32_t c, UTF8::TChar&rs)
		{
			UINT32 u = (UINT32)c;
			if (u <= 0x007F)
			{
				rs.numCharsUsed = 1;
				rs.encoded[0] = (char)(BYTE)u;
				return;
			}

			if (u <= 0x07FF)
			{
				rs.numCharsUsed = 2;
				rs.encoded[1] = 0x80 | (u& 0x3F);	//smallest 6 bits
				rs.encoded[0] = 0xC0 | (u>>6);
				return;
			}

			if (u <= 0xFFFF)
			{
				rs.numCharsUsed = 3;
				rs.encoded[2] = 0x80 | (u& 0x3F);	//smallest 6 bits
				rs.encoded[1] = 0x80 | ((u>>6)& 0x3F);	//next smallest 6 bits
				rs.encoded[0] = 0xE0 | (u>>12);
				return;
			}
			if (u <= 0x10FFFF)
			{
				rs.numCharsUsed = 4;
				rs.encoded[3] = 0x80 | (u& 0x3F);	//smallest 6 bits
				rs.encoded[2] = 0x80 | ((u>>6)& 0x3F);	//next smallest 6 bits
				rs.encoded[1] = 0x80 | ((u>>12)& 0x3F);	//next smallest 6 bits
				rs.encoded[0] = 0xF0 | (u>>18);
				return;
			}

			throw Except::Program::DataConsistencyFault(CLOCATION,"Invalid UTF32 character "+BinaryToHex(&c,4));
		}

		void	UTF32::ToUTF16(char32_t c, UTF16::TChar&rs)
		{
			UINT32 uni = (UINT32)c;

			//technically we're supposed to exclude [U+D800 to U+DFFF] here, but for testing purposes this is counter-productive
			//if (uni <= 0xD7FF || (uni >= 0xE000 && uni <= 0xFFFF))
			if (uni <= 0xFFFF)
			{
				rs.numCharsUsed = 1;
				rs.encoded[0] = (char16_t)uni;

				if ((uni & 0xFC00) == 0xD800)
					throw Except::Program::DataConsistencyFault(CLOCATION, "UTF-16 encoding error. Given UTF32 character #"+String(uni)+" is not valid");
			}
			else
			{
				rs.numCharsUsed = 2;
				uni -= 0x10000;
				rs.encoded[0] = (char16_t)((uni >> 10) + 0xD800);
				rs.encoded[1] = (char16_t)((uni & 0x3FF) + 0xDC00);
			}

		}

	



		template <typename T>
		static void	Utf8ToUtf16T(const StringRef&utf8Source, StringType::Template<T>&utf16Dest)
		{
			VariableLengthToVariableLength<UTF8Code,UTF16Code<T> >(utf8Source,utf16Dest);
		}

		void	UTF16::ToUTF8(const StringType::ReferenceExpression<char16_t>&utf16Source, String&utf8Dest)
		{
			VariableLengthToVariableLength<UTF16Code<char16_t>, UTF8Code>(utf16Source,utf8Dest);
		}

		void	UTF16::ToUTF8(const StringType::Template<char16_t>&utf16Source, String&utf8Dest)
		{
			UTF16::ToUTF8(utf16Source.ref(),utf8Dest);
		}

		void	UTF16::ToUTF32(const StringType::ReferenceExpression<char16_t>&utf16Source, UTF32String&utf32Dest)
		{
			VariableLengthToFixedLength<UTF16Code<char16_t>,UTF32FixedCode>(utf16Source,utf32Dest);
		}

		void	UTF16::ToUTF32(const StringType::Template<char16_t>&utf16Source, UTF32String&utf32Dest)
		{
			UTF16::ToUTF32(utf16Source.ref(),utf32Dest);
		}


		UTF32String	UTF16::ToUTF32(const StringType::ReferenceExpression<char16_t>&utf16Source)
		{
			UTF32String rs;
			ToUTF32(utf16Source,rs);
			return rs;
		}
		UTF32String	UTF16::ToUTF32(const StringType::Template<char16_t>&utf16Source)
		{
			UTF32String rs;
			ToUTF32(utf16Source,rs);
			return rs;
		}
		String		UTF16::ToUTF8(const StringType::ReferenceExpression<char16_t>&utf16Source)
		{
			String rs;
			ToUTF8(utf16Source,rs);
			return rs;
		}
		String		UTF16::ToUTF8(const StringType::Template<char16_t>&utf16Source)
		{
			String rs;
			ToUTF8(utf16Source,rs);
			return rs;
		}


		#ifdef WIN32
			static_assert(sizeof(wchar_t)==sizeof(char16_t),"Expected wchar_t to be 16 bit on windows");
			void	UTF32::ToUTF16(const StringType::ReferenceExpression<char32_t>&utf32Source, StringType::Template<wchar_t>&utf16Dest)
			{
				FixedLengthToVariableLength<UTF32FixedCode, UTF16Code<wchar_t> >(utf32Source,utf16Dest);
			}
			void	UTF32::ToUTF16(const UTF32String&utf32Source, StringType::Template<wchar_t>&utf16Dest)
			{
				UTF32::ToUTF16(utf32Source.ref(),utf16Dest);
			}

			void	UTF8::ToUTF16(const StringRef&utf8Source, StringType::Template<wchar_t>&utf16Dest)
			{
				Utf8ToUtf16T(utf8Source,utf16Dest);
			}

			void	UTF8::ToUTF16(const String&utf8Source, StringType::Template<wchar_t>&utf16Dest)
			{
				Utf8ToUtf16T(utf8Source.ref(),utf16Dest);
			}

			void	UTF16::ToUTF32(const StringType::ReferenceExpression<wchar_t>&utf16Source, UTF32String&utf32Dest)
			{
				VariableLengthToFixedLength<UTF16Code<wchar_t>,UTF32FixedCode>(utf16Source,utf32Dest);
			}

			void	UTF16::ToUTF32(const StringW&utf16Source, UTF32String&utf32Dest)
			{
				UTF16::ToUTF32(utf16Source.ref(),utf32Dest);
			}

			void	UTF16::ToUTF8(const StringType::ReferenceExpression<wchar_t>&utf16Source, String&utf8Dest)
			{
				VariableLengthToVariableLength<UTF16Code<wchar_t>, UTF8Code>(utf16Source,utf8Dest);
			}

			void	UTF16::ToUTF8(const StringW&utf16Source, String&utf8Dest)
			{
				UTF16::ToUTF8(utf16Source.ref(),utf8Dest);
			}

			UTF32String	UTF16::ToUTF32(const StringType::ReferenceExpression<wchar_t>&utf16Source)
			{
				UTF32String rs;
				ToUTF32(utf16Source,rs);
				return rs;
			}
			UTF32String	UTF16::ToUTF32(const StringW&utf16Source)
			{
				UTF32String rs;
				ToUTF32(utf16Source,rs);
				return rs;
			}
			String		UTF16::ToUTF8(const StringType::ReferenceExpression<wchar_t>&utf16Source)
			{
				String rs;
				ToUTF8(utf16Source,rs);
				return rs;
			}
			String		UTF16::ToUTF8(const StringW&utf16Source)
			{
				String rs;
				ToUTF8(utf16Source,rs);
				return rs;
			}


		#endif

		void	UTF32::ToUTF16(const UTF32String&utf32Source, StringType::Template<char16_t>&utf16Dest)
		{
			UTF32::ToUTF16(utf32Source.ref(),utf16Dest);
		}

		void	UTF32::ToUTF16(const StringType::ReferenceExpression<char32_t>&utf32Source, StringType::Template<char16_t>&utf16Dest)
		{
			FixedLengthToVariableLength<UTF32FixedCode, UTF16Code<char16_t> >(utf32Source,utf16Dest);
		}

		void	UTF8::ToUTF16(const StringRef&utf8Source, StringType::Template<char16_t>&utf16Dest)
		{
			Utf8ToUtf16T(utf8Source,utf16Dest);
		}

		void	UTF8::ToUTF16(const String&utf8Source, StringType::Template<char16_t>&utf16Dest)
		{
			ToUTF16(utf8Source.ToRef(),utf16Dest);
		}

		StringW	UTF8::ToUTF16(const String&utf8Source)
		{
			StringW rs;
			ToUTF16(utf8Source,rs);
			return rs;
		}





		void	UTF8::ToCP1252(const StringRef&utf8Source, String&cp1252Dest)
		{
			VariableLengthToFixedLength<UTF8Code,CP1252FixedCode>(utf8Source,cp1252Dest);
		}


		void	UTF8::ToCP1252(const String&utf8Source, String&cp1252Dest)
		{
			UTF8::ToCP1252(utf8Source.ref(),cp1252Dest);
		}

		void	CP1252::ToUTF8(const StringRef&cp1252Source, String&utf8Dest)
		{
			FixedLengthToVariableLength<CP1252FixedCode, UTF8Code>(cp1252Source,utf8Dest);
		}

		void	CP1252::ToUTF8(const String&cp1252Source, String&utf8Dest)
		{
			CP1252::ToUTF8(cp1252Source.ref(),utf8Dest);
		}

		void	CP1252::ToUTF32(const StringRef&cp1252Source, UTF32String&utf32Dest)
		{
			utf32Dest.SetLength(cp1252Source.GetLength());
			auto*out = utf32Dest.mutablePointer();
			const auto*in = cp1252Source.pointer();
			const auto*const end = cp1252Source.end();
			for (; in != end; ++in, ++out)
			{
				*out = CP1252::ToUTF32(*in);
			}
			ASSERT_EQUAL__(out,utf32Dest.end());
		}

		void	CP1252::ToUTF32(const String&cp1252Source, UTF32String&utf32Dest)
		{
			CP1252::ToUTF32(cp1252Source.ref(),utf32Dest);
		}

		void	UTF32::ToCP1252(const StringType::ReferenceExpression<char32_t>&utf32Source, String&cp1525Dest)
		{
			cp1525Dest.SetLength(utf32Source.GetLength());
			auto*out = cp1525Dest.mutablePointer();
			const auto*in = utf32Source.pointer();
			const auto*const end = utf32Source.end();
			for (; in != end; ++in, ++out)
			{
				*out = UTF32::ToCP1252(*in);
			}
			ASSERT_EQUAL__(out,cp1525Dest.end());
		}

		void	UTF32::ToCP1252(const UTF32String&utf32Source, String&cp1525Dest)
		{
			UTF32::ToCP1252(utf32Source.ref(),cp1525Dest);
		}


	}




}

