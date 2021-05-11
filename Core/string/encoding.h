#pragma once

#include "../global_string.h"

namespace DeltaWorks
{
	/**
	Support for various string encodings and the conversion between them.
	*/
	namespace StringEncoding
	{


		/**
		Attributes of and conversion from UTF-8
		*/
		namespace UTF8
		{

			/**
			Character container that can hold all valid UTF-8 characters
			*/
			struct TChar
			{
				char		encoded[4];
				BYTE		numCharsUsed=0;	//0-4

				StringRef	ToRef() const {return StringRef(encoded,numCharsUsed);};
			};

			/**
			Determines the total length (in 8bit characters) of a variable-length UTF-8 encoded character, by looking at the first character.
			Throws an exception if the detected length exceeds 4.
			@return Character length in [1,4], including this first character
			*/
			BYTE			GetLength(char firstChar);

			/**
			Parses a given UTF8 string segment to a single UTF32 character
			@param[inout] begin Starting character. Incremented automatically. Left unchanged if the function returns false
			@param end One past the last character available
			@param[out] outChar Output character to write the parsed character into. Left unchanged if the function fails
			@return True if the given character segment has at least one character and all of the possibly multi-byte character is present in the segment
			*/
			bool			ToUTF32(const char*&begin, const char*const end, char32_t&outChar);
			char32_t		ToUTF32(const TChar&c);
			void			ToUTF32(const StringType::Sequence<char>&utf8Source, UTF32String&utf32Dest);
			UTF32String		ToUTF32(const StringType::Sequence<char>&utf8Source);
			char			ToCP1252(const TChar&ch);
			void			ToCP1252(const StringType::Sequence<char>&utf8Source, String&cp1252Dest);
			void			ToUTF16(const StringType::Sequence<char>&utf8Source, StringType::Template<char16_t>&utf16Dest);
			StringW			ToUTF16(const StringType::Sequence<char>&utf8Source);
			#ifdef WIN32
				void		ToUTF16(const StringType::Sequence<char>&utf8Source, StringType::Template<wchar_t>&utf16Dest);
			#endif


			/**
			Sequential character scanner for a UTF8 base string
			*/
			class Scanner
			{
			public:
				Scanner(const StringType::Sequence<char>&source):current(source.GetPointer()),end(source.GetPointer()+source.GetLength())
				{}

				/**
				Checks if there are more characters in the source string
				*/
				bool		HasNext() const {return current < end;}
				/**
				Gets the next unicode character in the source string.
				Returns 0 on failure of if the scanner has reached the end
				*/
				char32_t	Next()
				{
					char32_t rs;
					if (!ToUTF32(current,end,rs))
					{
						current = end;
						return 0;
					}
					return rs;
				}

				bool		Next(TChar&outChar)
				{
					if (current >= end)
						return false;
					outChar.numCharsUsed = GetLength(*current);
					if (outChar.numCharsUsed == 0)
					{
						current = end;
						return false;
					}
					memcpy(outChar.encoded, current, outChar.numCharsUsed);
					current += outChar.numCharsUsed;
					return true;
				}

			private:
				const char*	current, *const end;
			};

		};

		/**
		Attributes of and conversion from UTF-16
		*/
		namespace UTF16
		{
			/**
			Character container that can hold all valid UTF-16 characters
			*/
			struct TChar
			{
				char16_t	encoded[2];
				BYTE		numCharsUsed=0;	//0, 1 or 2
			};


			/**
			Determines the total length (in 8bit characters) of a variable-length UTF-16 encoded character, by looking at the first character.
			@return Character length in [1,2], including this first character
			*/
			BYTE			GetLength(char16_t firstChar);

			/**
			Parses a given UTF-16 string segment to a single UTF-32 character
			@param[inout] begin Starting character. Incremented automatically. Left unchanged if the function returns false
			@param end One past the last character available
			@param[out] outChar Output character to write the parsed character into. Left unchanged if the function fails
			@return True if the given character segment has at least one character and all of the possibly multi-byte character is present in the segment
			*/
			bool			ToUTF32(const char16_t*&begin, const char16_t*const end, char32_t&outChar);

			char32_t		ToUTF32(const TChar&c);
			void			ToUTF32(const StringType::Sequence<char16_t>&utf16Source, UTF32String&utf32Dest);
			void			ToUTF8(const StringType::Sequence<char16_t>&utf16Source, String&utf8Dest);
			UTF32String		ToUTF32(const StringType::Sequence<char16_t>&utf16Source);
			String			ToUTF8(const StringType::Sequence<char16_t>&utf16Source);
			#ifdef WIN32
				static_assert(sizeof(wchar_t)==sizeof(char16_t),"Expected wchar_t to be 16 bit on windows");
				void		ToUTF32(const StringType::Sequence<wchar_t>&utf16Source, UTF32String&utf32Dest);
				void		ToUTF8(const StringType::Sequence<wchar_t>&utf16Source, String&utf8Dest);
				UTF32String	ToUTF32(const StringType::Sequence<wchar_t>&utf16Source);
				String		ToUTF8(const StringType::Sequence<wchar_t>&utf16Source);

				inline BYTE	GetLength(wchar_t firstChar)	{return GetLength((char16_t)firstChar);}
				bool		ToUTF32(const wchar_t*&begin, const wchar_t*const end, char32_t&outChar);

			#endif
		}

		/**
		Conversion functions from UTF-32
		*/
		namespace UTF32
		{
			BYTE			ToUTF8Length(char32_t utf32Char);
			void			ToUTF8(char32_t c, UTF8::TChar&rs);
			UTF8::TChar		ToUTF8(char32_t c);
			void			ToUTF8(const StringType::Sequence<char32_t>&utf32Source, String&utf8Dest);
			String			ToUTF8(const StringType::Sequence<char32_t>&utf32Source);
			char			ToCP1252(char32_t c);
			bool			ToCP1252(char32_t c, char&outChar);
			void			ToCP1252(const StringType::Sequence<char32_t>&utf32Source, String&cp1525Dest);
			void			ToUTF16(char32_t c, UTF16::TChar&rs);
			void			ToUTF16(const StringType::Sequence<char32_t>&utf32Source, UTF16String&utf16Dest);
			#ifdef WIN32
				void		ToUTF16(const StringType::Sequence<char32_t>&utf32Source, StringType::Template<wchar_t>&utf16Dest);
			#endif
		}

		/**
		Conversion functions from Windows Code Page 1252 (sometimes mislabeled ANSI, or ISO-8859-1)
		*/
		namespace CP1252
		{
			void			ToUTF8(char, UTF8::TChar&ch);
			void			ToUTF8(const StringType::Sequence<char>&cp1252Source, String&utf8Dest);
			char32_t		ToUTF32(char);
			void			ToUTF32(const StringType::Sequence<char>&cp1252Source, UTF32String&utf32Dest);
		}

		/**
		Deserializes a UTF-8 character from an arbitrary binary stream.
		The stream is advanced by the exact length of the extracted character
		*/
		void	Deserialize(IReadStream&source, UTF8::TChar&utf8Dest);

	
		bool	IsValid(const UTF16::TChar&);
		bool	IsValid(const UTF8::TChar&);
		bool	IsValidUnicode(char32_t c);
		void	AssertValidity(const UTF16::TChar&);
		void	AssertValidity(const UTF8::TChar&);

	
	}
}
