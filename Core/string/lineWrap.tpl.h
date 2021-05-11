#pragma once
#include "encoding.h"


namespace DeltaWorks::LineWrap
{
	namespace Details
	{
		
		template <typename Char, class Wrapper>
			void Wrap(const ArrayRef<Char>&string, ArrayData<StringType::ReferenceExpression<Char> >&result, Wrapper wrapper)
			{
				count_t numLines=0;
				wrapper.Scan(string,[&numLines](const StringType::ReferenceExpression<Char>&)	{numLines++;});

				result.SetSize(numLines);
				index_t at = 0;
				wrapper.Scan(string,[&result,&at](const StringType::ReferenceExpression<Char>&exp)
				{
					result[at] = exp;
					at++;
				});
				ASSERT_EQUAL__(at,numLines);
			}	
	
		static inline count_t	ConstantCharWidthFunction(char32_t)
		{
			return 1;
		}

	}


	namespace UTF32
	{
		
		template <typename Width>
			class Wrapper
			{
			public:
				typedef char32_t Char;
				typedef Width(*FCharWidth)(Char);

				const FCharWidth	charWidth;
				const Width			lineWidth;

				/**/		Wrapper(const Width&lineWidth, const FCharWidth&charWidth):charWidth(charWidth),lineWidth(lineWidth)
				{}

				template <typename CallbackFunctor>
					void	Scan(const ArrayRef<Char>&line, const CallbackFunctor&callback)
					{
						if (line.IsEmpty())
							return;
						Width		lineWidthToCurrent = 0,	//effective width from firstLineChar to current
									wordWithToCurrent = 0;	//effective width from firstWordChar to current
						const Char	*current = line.begin(),
									*firstWordChar = current,
									*firstLineChar = current;
						const Char*const end = line.end();

						bool wordEnded = false;
						bool forceNewlineAtNextChar = false;

						while (current < end)
						{
							const Width charW = charWidth(*current);
							lineWidthToCurrent += charW;
							wordWithToCurrent += charW;
							if (
								firstWordChar < current	//have at least one char (otherwise: empty line)
								&&
								(
									forceNewlineAtNextChar //encountered new line
									|| 
									lineWidthToCurrent >= lineWidth	//exceeded line width
								)
							)	
							{
								//const bool forcedBreak = lineWidthToCurrent < lineWidth;	//ONLY forceNewLineAtNextChar is true
								if (forceNewlineAtNextChar)
								{
									firstWordChar = current-1;	//last char was newline. we don't pass that to the callback function, so reduce line limit (firstWordChar)
								}
								else
								{
									if (firstWordChar == firstLineChar)	//we are still inside the first word of this line, so we need to split the word
									{
										Width currentW = charWidth(*firstWordChar);
										firstWordChar++;	//always include first char

										for(;;)
										{
											ASSERT_LESS_OR_EQUAL__(firstWordChar,current);
											const Width chrWidth = charWidth(*firstWordChar);
											if (firstWordChar < current && (currentW + chrWidth <= lineWidth))	//this char still fits
											{
												firstWordChar++;
												currentW += chrWidth;
												wordWithToCurrent -= chrWidth;
											}
											else
												break;
										}
										if (firstWordChar - firstLineChar > 1)
											ASSERT_LESS__(currentW,lineWidth);	//one char may exceed limit (can't do anything here). but 2+ must never
									}
								}
								callback(StringType::ReferenceExpression<Char>(firstLineChar,firstWordChar-firstLineChar));

								if (forceNewlineAtNextChar)
								{
									firstWordChar = current;	//skip newline char
									wordWithToCurrent = charW;
								}
								firstLineChar = firstWordChar;
								lineWidthToCurrent = wordWithToCurrent;
								forceNewlineAtNextChar = false;
							}
		
		
							if (IsWhitespace(*current))
							{
								if (IsNewline(*current))
									forceNewlineAtNextChar = true;

								if (firstLineChar == current && !forceNewlineAtNextChar)
								{
									firstLineChar++;	//auto-trim beginning
									firstWordChar++;
								}
								wordEnded = true;
							}
							else
								if (wordEnded)
								{
									wordEnded = false;
									firstWordChar = current;
									wordWithToCurrent = charW;
								}
							current++;
						}
						if (current > firstLineChar)	//some leftovers?
						{
							callback(StringType::ReferenceExpression<Char>(firstLineChar, current - firstLineChar));
						}
					}
			};

		template <typename Width>
			static Wrapper<Width> MakeWrapper(const Width&lineWidth, Width(*charWidthFunction)(char32_t))
			{
				return Wrapper<Width>(lineWidth,charWidthFunction);
			}
	
	


		template <typename Width>
			void	Wrap(const ArrayRef<char32_t>&string, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<char32_t> >&result)
			{
				Details::Wrap(string,result, MakeWrapper(lineWidth,charWidthFunction));
			}


	}




	namespace UTF8
	{
		
		template <typename Width>
			class Wrapper
			{
			public:
				typedef Width(*FCharWidth)(char32_t);

				const FCharWidth	charWidth;
				const Width			lineWidth;

				/**/		Wrapper(const Width&lineWidth, const FCharWidth&charWidth):charWidth(charWidth),lineWidth(lineWidth)
				{}

				template <typename CallbackFunctor>
					void	Scan(const ArrayRef<char>&line, const CallbackFunctor&callback)
					{
						if (line.IsEmpty())
							return;
						Width		lineWidthToCurrent = 0,	//effective width from firstLineChar to current
									wordWithToCurrent = 0;	//effective width from firstWordChar to current
						const char	*cursor = line.begin(),
									*firstWordChar = cursor,
									*firstLineChar = cursor;
						const char*const end = line.end();

						bool wordEnded = false;
						bool forceNewlineAtNextChar = false;
						
						auto previous = cursor;
						auto current = cursor;
						while (cursor < end)
						{
							previous = current;
							current = cursor;
							char32_t c32;
							if (!StringEncoding::UTF8::ToUTF32(cursor,end,c32))
								return;

							const Width charW = charWidth(c32);
							lineWidthToCurrent += charW;
							wordWithToCurrent += charW;
							if (
								firstWordChar < current	//have at least one char (otherwise: empty line)
								&&
								(
									forceNewlineAtNextChar //encountered new line
									|| 
									lineWidthToCurrent >= lineWidth	//exceeded line width
								)
							)	
							{
								//const bool forcedBreak = lineWidthToCurrent < lineWidth;	//ONLY forceNewLineAtNextChar is true
								if (forceNewlineAtNextChar)
								{
									firstWordChar = previous;	//last char was newline. we don't pass that to the callback function, so reduce line limit (firstWordChar)
								}
								else
								{
									if (firstWordChar == firstLineChar)	//we are still inside the first word of this line, so we need to split the word
									{
										char32_t firstWordChar32;
										if (!StringEncoding::UTF8::ToUTF32(firstWordChar,end,firstWordChar32))	//always include first char
											return;
										Width currentW = charWidth(firstWordChar32);
										

										for(;;)
										{
											ASSERT_LESS_OR_EQUAL__(firstWordChar,current);
											auto firstWordCharNext = firstWordChar;
											if (!StringEncoding::UTF8::ToUTF32(firstWordCharNext,end,firstWordChar32))
												return;
											const Width chrWidth = charWidth(firstWordChar32);
											if (firstWordChar < current && (currentW + chrWidth <= lineWidth))	//this char still fits
											{
												firstWordChar = firstWordCharNext;
												currentW += chrWidth;
												wordWithToCurrent -= chrWidth;
											}
											else
												break;
										}

										if (firstWordChar > firstLineChar + StringEncoding::UTF8::GetLength(*firstLineChar))
										//if (firstWordChar - firstLineChar > 1)
											ASSERT_LESS__(currentW,lineWidth);	//one char may exceed limit (can't do anything here). but 2+ must never
									}
								}
								callback(StringRef(firstLineChar,firstWordChar-firstLineChar));

								if (forceNewlineAtNextChar)
								{
									firstWordChar = current;	//skip newline char
									wordWithToCurrent = charW;
								}
								firstLineChar = firstWordChar;
								lineWidthToCurrent = wordWithToCurrent;
								forceNewlineAtNextChar = false;
							}
		
		
							if (IsWhitespace(*current))
							{
								if (IsNewline(*current))
									forceNewlineAtNextChar = true;

								if (firstLineChar == current && !forceNewlineAtNextChar)
								{
									firstLineChar += StringEncoding::UTF8::GetLength(*firstLineChar);//auto-trim beginning
									firstWordChar += StringEncoding::UTF8::GetLength(*firstWordChar);
									//firstLineChar++;	//auto-trim beginning
									//firstWordChar++;
								}
								wordEnded = true;
							}
							else
								if (wordEnded)
								{
									wordEnded = false;
									firstWordChar = current;
									wordWithToCurrent = charW;
								}
						}
						if (current > firstLineChar)	//some leftovers?
						{
							callback(StringRef(firstLineChar, current - firstLineChar));
						}
					}
			};

		template <typename Width>
			static Wrapper<Width> MakeWrapper(const Width&lineWidth, Width(*charWidthFunction)(char32_t))
			{
				return Wrapper<Width>(lineWidth,charWidthFunction);
			}
	
	
		template <typename Width>
			void	Wrap(const ArrayRef<char>&string, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<char> >&result)
			{
				Details::Wrap(string,result, MakeWrapper(lineWidth,charWidthFunction));
			}



	}


	
	
	namespace UTF16
	{
		
		template <typename Width, typename Char>	//unfortunately, char16_t and wchar_t do not map to the same type even where of same size
			class Wrapper
			{
			public:
				typedef Width(*FCharWidth)(char32_t);

				static_assert(sizeof(Char) == 2, "Given character type must be UTF-16");

				const FCharWidth	charWidth;
				const Width			lineWidth;

				/**/		Wrapper(const Width&lineWidth, const FCharWidth&charWidth):charWidth(charWidth),lineWidth(lineWidth)
				{}

				template <typename CallbackFunctor>
					void	Scan(const ArrayRef<Char>&line, const CallbackFunctor&callback)
					{
						if (line.IsEmpty())
							return;
						Width		lineWidthToCurrent = 0,	//effective width from firstLineChar to current
									wordWithToCurrent = 0;	//effective width from firstWordChar to current
						const Char	*cursor = line.begin(),
									*firstWordChar = cursor,
									*firstLineChar = cursor;
						const Char*const end = line.end();

						bool wordEnded = false;
						bool forceNewlineAtNextChar = false;
						
						auto previous = cursor;
						auto current = cursor;
						while (cursor < end)
						{
							previous = current;
							current = cursor;
							char32_t c32;
							if (!StringEncoding::UTF16::ToUTF32(cursor,end,c32))
								return;

							const Width charW = charWidth(c32);
							lineWidthToCurrent += charW;
							wordWithToCurrent += charW;
							if (
								firstWordChar < current	//have at least one char (otherwise: empty line)
								&&
								(
									forceNewlineAtNextChar //encountered new line
									|| 
									lineWidthToCurrent >= lineWidth	//exceeded line width
								)
							)	
							{
								//const bool forcedBreak = lineWidthToCurrent < lineWidth;	//ONLY forceNewLineAtNextChar is true
								if (forceNewlineAtNextChar)
								{
									firstWordChar = previous;	//last char was newline. we don't pass that to the callback function, so reduce line limit (firstWordChar)
								}
								else
								{
									if (firstWordChar == firstLineChar)	//we are still inside the first word of this line, so we need to split the word
									{
										char32_t firstWordChar32;
										if (!StringEncoding::UTF16::ToUTF32(firstWordChar,end,firstWordChar32))	//always include first char
											return;
										Width currentW = charWidth(firstWordChar32);
										

										for(;;)
										{
											ASSERT_LESS_OR_EQUAL__(firstWordChar,current);
											auto firstWordCharNext = firstWordChar;
											if (!StringEncoding::UTF16::ToUTF32(firstWordCharNext,end,firstWordChar32))
												return;
											const Width chrWidth = charWidth(firstWordChar32);
											if (firstWordChar < current && (currentW + chrWidth <= lineWidth))	//this char still fits
											{
												firstWordChar = firstWordCharNext;
												currentW += chrWidth;
												wordWithToCurrent -= chrWidth;
											}
											else
												break;
										}

										if (firstWordChar > firstLineChar + StringEncoding::UTF16::GetLength(*firstLineChar))
										//if (firstWordChar - firstLineChar > 1)
											ASSERT_LESS__(currentW,lineWidth);	//one char may exceed limit (can't do anything here). but 2+ must never
									}
								}
								callback(StringType::ReferenceExpression<Char>(firstLineChar,firstWordChar-firstLineChar));

								if (forceNewlineAtNextChar)
								{
									firstWordChar = current;	//skip newline char
									wordWithToCurrent = charW;
								}
								firstLineChar = firstWordChar;
								lineWidthToCurrent = wordWithToCurrent;
								forceNewlineAtNextChar = false;
							}
		
		
							if (IsWhitespace(*current))
							{
								if (IsNewline(*current))
									forceNewlineAtNextChar = true;

								if (firstLineChar == current && !forceNewlineAtNextChar)
								{
									firstLineChar += StringEncoding::UTF16::GetLength(*firstLineChar);//auto-trim beginning
									firstWordChar += StringEncoding::UTF16::GetLength(*firstWordChar);
									//firstLineChar++;	//auto-trim beginning
									//firstWordChar++;
								}
								wordEnded = true;
							}
							else
								if (wordEnded)
								{
									wordEnded = false;
									firstWordChar = current;
									wordWithToCurrent = charW;
								}
						}
						if (current > firstLineChar)	//some leftovers?
						{
							callback(StringType::ReferenceExpression<Char>(firstLineChar, current - firstLineChar));
						}
					}
			};



		template <typename Width, typename Char>
			static Wrapper<Width, Char> MakeWrapper(const Width&lineWidth, Width(*charWidthFunction)(char32_t))
			{
				return Wrapper<Width,Char>(lineWidth,charWidthFunction);
			}
		
	
	


		template <typename Char>
			void 	Wrap(const ArrayRef<Char>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<Char> >&result)
			{
				Details::Wrap(string,result, MakeWrapper<count_t,Char>(lineCharCount,Details::ConstantCharWidthFunction));
			}

		template <typename Width, typename Char>
			void	Wrap(const ArrayRef<Char>&string, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<Char> >&result)
			{
				Details::Wrap(string,result, MakeWrapper<Width,Char>(lineWidth,charWidthFunction));
			}


	}
}

