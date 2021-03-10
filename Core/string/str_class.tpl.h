#ifndef str_classTplH
#define str_classTplH

#include <wchar.h>



template <typename T>
	inline std::ostream& operator<<(std::ostream&stream, const StringType::Template<T>&string)
	{
		const T*c = string.c_str();
		while (*c)
			stream << (char)*c++;
		return stream;
	}

template <>
	inline std::ostream& operator<<(std::ostream&stream, const StringType::Template<char>&string)
	{
		return stream << string.c_str();
	}


template <typename T>
	inline std::ostream& operator<<(std::ostream&stream, const StringType::ReferenceExpression<T>&expression)
	{
		expression.Print(stream);
		return stream;
	}


template <typename E0, typename E1>
	inline std::ostream& operator<<(std::ostream&stream, const StringType::ConcatExpression<E0, E1>&expression)
	{
		expression.Print(stream);
		return stream;
	}



template <typename T>
	inline std::ostream& operator<<(std::ostream&stream, const StringType::CharacterExpression<T>&expression)
	{
		expression.Print(stream);
		return stream;
	}
	


#ifdef WCOUT

	template <typename T>
		inline std::wostream& operator<<(std::wostream&stream, const StringType::Template<T>&string)
		{
			const T*c = string.c_str();
			while (*c)
				stream << (wchar_t)*c++;
			return stream;
		}

	template <>
		inline std::wostream& operator<<(std::wostream&stream, const StringType::Template<char>&string)
		{
			return stream << string.c_str();
		}
	
	template <>
		inline std::wostream& operator<<(std::wostream&stream, const StringType::Template<wchar_t>&string)
		{
			return stream << string.c_str();
		}
		


	template <typename T>
		inline std::wostream& operator<<(std::wostream&stream, const StringType::ReferenceExpression<T>&expression)
		{
			expression.Print(stream);
			return stream;
		}


	template <typename E0, typename E1>
		inline std::wostream& operator<<(std::wostream&stream, const StringType::ConcatExpression<E0, E1>&expression)
		{
			expression.Print(stream);
			return stream;
		}



	template <typename T>
		inline std::wostream& operator<<(std::wostream&stream, const StringType::CharacterExpression<T>&expression)
		{
			expression.Print(stream);
			return stream;
		}
		


#endif



#endif

template <typename T0, typename T1, typename T2>
	void	explode(const T0*delimiter, size_t delimiter_length, const T1*string, Ctr::ArrayData<T2>&result)
	{
		if (!string)
			return;
		explode(delimiter, delimiter_length,ReferenceExpression<T1>(string,CharFunctions::strlen(string)),result);
	}

	
template <typename T0, typename T1, typename T2>
	void	explode(const T0*delimiter, size_t delimiter_length, const StringType::ReferenceExpression<T1>&string, Ctr::ArrayData<T2>&result)
	{
		if (!delimiter || !delimiter_length)
			return;
		if (!string.GetLength())
		{
			result.SetSize(1);
			result[0] = string;
			return;
		}
		const T0*const delimiterEnd = delimiter+delimiter_length;
		
		//Ctr::ArrayData<T1>	buffer(string);
		const T1*at = string.pointer();
		const T1*const end = at+string.GetLength();
		size_t count = 0;
		while (at != end)
		{
			count ++;
			if (const T1*found = CharFunctions::strstr<T1,T0>(at,end,delimiter,delimiterEnd))
			{
				at = found + delimiter_length;
			}
			else
				break;
		}
		result.SetSize(count);
		at = string.pointer();
		count = 0;
		while (at != end)
		{
			if (const T1*found = CharFunctions::strstr<T1,T0>(at,end,delimiter,delimiterEnd))
			{
				result[count++] = T2(at,found-at);
				/*(*found) = 0;
				result[count++] = at;*/
				at = found + delimiter_length;
			}
			else
			{
				result[count++] = T2(at,end-at);
				return;
			}
		}
	}

template <typename T0, typename T1, typename T2>
	void	explode(const T0*delimiter, size_t delimiter_length, const StringType::Template<T1>&string, Ctr::ArrayData<T2>&result)
	{
		explode(delimiter,delimiter_length,string.ref(),result);
	}





template <typename T0, typename T1, typename T2>
	void	explode(const StringType::Template<T0>&delimiter, const StringType::Template<T1>&string, Ctr::ArrayData<T2>&result)
	{
		explode(delimiter.c_str(),delimiter.GetLength(),string,result);
	}


template <typename T0, typename T1, typename T2>
	void	explode(const T0*delimiter, const StringType::Template<T1>&string, Ctr::ArrayData<T2>&result)
	{
		explode(delimiter,CharFunctions::strlen(delimiter),string,result);
	}


template <typename T0, typename T1, typename T2>
	void	explode(const T0*delimiter, const T1*string, Ctr::ArrayData<T2>&result)
	{
		explode(delimiter,CharFunctions::strlen(delimiter),string,result);
	}

template <typename T0, typename T1>
	void	explode(T0 delimiter, const T0*string, Ctr::ArrayData<T1>&result)
	{
		if (!string)
			return;
		explode(delimiter, StringType::ReferenceExpression<T0>(string,CharFunctions::strlen(string)),result);
	}

template <typename T0, typename T1>
	void	explode(T0 delimiter, const StringType::ReferenceExpression<T0>&string, Ctr::ArrayData<T1>&result)
	{
		//Ctr::ArrayData<T0>	buffer(string);
		const T0*at = string.pointer();
		const T0*const end = at + string.GetLength();
		size_t count = 0;
		while (at != end)
		{
			count ++;
			if (const T0*found = CharFunctions::strchr(at,end,delimiter))
			{
				at = found +1;
			}
			else
				break;
		}
		result.SetSize(count);
		at = string.pointer();
		count = 0;
		while (at != end)
		{
			if (const T0*found = CharFunctions::strchr(at,end,delimiter))
			{
				/*(*found) = 0;
				result[count++] = at;*/
				result[count++] = T1(at,found-at);
				at = found + 1;
			}
			else
			{
				result[count++] = T1(at,end-at);
				return;
			}
		}
	}

template <typename T0, typename T1>
	void	explode(T0 delimiter, const StringType::Template<T0>&string, Ctr::ArrayData<T1>&result)
	{
		explode(delimiter,string.ref(),result);
	}

template <typename T0, typename T1>
	void	explodeCallback(bool isDelimiter(T0), const T0*string, Ctr::ArrayData<T1>&result)
	{
		if (!string)
			return;
		explodeCallback(isDelimiter, StringType::ReferenceExpression<T0>(string,CharFunctions::strlen(string)),result);
	}

template <typename T0, typename T1>
	void	explodeCallback(bool isDelimiter(T0), const StringType::ReferenceExpression<T0>&string, Ctr::ArrayData<T1>&result)
	{
		//Ctr::ArrayData<T0>	buffer(string);
		const T0*at = string.pointer();
		const T0*const end = at + string.GetLength();
		size_t count = 0;
		while (at!=end)
		{
			count ++;
			if (const T0*found = CharFunctions::strchr(at,end,isDelimiter))
			{
				at = found +1;
			}
			else
				break;
		}
		result.SetSize(count);
		at = string.pointer();
		count = 0;
		while (*at)
		{
			if (const T0*found = CharFunctions::strchr(at,end,isDelimiter))
			{
				/*(*found) = 0;
				result[count++] = at;*/
				result[count++] = T1(at,found-at);
				at = found + 1;
			}
			else
			{
				result[count++] = T1(at,end-at);
				return;
			}
		}
	}

template <typename T0, typename T1>
	void	explodeCallback(bool isDelimiter(T0), const StringType::Template<T0>&string, Ctr::ArrayData<T1>&result)
	{
		explodeCallback(isDelimiter,string.ref(),result);
	}




	



template <typename T0, typename T1>
	StringType::Template<T1>		implode(const StringType::Template<T0>&glue, const StringType::ReferenceExpression<T1>*pieces, count_t numPieces)
	{
		if (!numPieces)
			return "";
		size_t len = 0;
		for (size_t i = 0; i < numPieces; i++)
			len += pieces[i].GetLength();
		const T0*glue_str = glue.c_str();
		size_t glue_len = glue.GetLength();
		len += (numPieces-1)*glue_len;
		StringType::Template<T1> result = TStringLength(len);
		T1*out = result.mutablePointer();
		for (size_t i = 0; i < numPieces-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].pointer(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			CharFunctions::strncpy(out,glue_str,glue_len);
			out+=glue_len;
		}
		CharFunctions::strncpy(out,pieces[numPieces-1].pointer(),pieces[numPieces-1].GetLength());
		//out+=pieces[numPieces-1].GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}

template <typename T>
	StringType::Template<T>		implode(T glue, const StringType::ReferenceExpression<T>*pieces, count_t numPieces)
	{
		if (!numPieces)
			return "";
		size_t len = 0;
		for (size_t i = 0; i < numPieces; i++)
			len += pieces[i].GetLength();
		len += (numPieces-1);
		StringType::Template<T> result = TStringLength(len);
		T*out = result.mutablePointer();
		for (size_t i = 0; i < numPieces-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].pointer(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			(*out++) = glue;
		}
		CharFunctions::strncpy(out,pieces[numPieces-1].pointer(),pieces[numPieces-1].GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}
	
template <typename T0, typename T1>
	StringType::Template<T1>		implode(const T0*glue_str, const StringType::ReferenceExpression<T1>*pieces, count_t numPieces)
	{
		if (!numPieces)
			return "";
		size_t len = 0;
		for (size_t i = 0; i < numPieces; i++)
			len += pieces[i].GetLength();
		size_t glue_len = CharFunctions::strlen(glue_str);
		len += (numPieces-1)*glue_len;
		StringType::Template<T1> result = TStringLength(len);
		T1*out = result.mutablePointer();
		for (size_t i = 0; i < numPieces-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].pointer(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			CharFunctions::strncpy(out,glue_str,glue_len);
			out+=glue_len;
		}
		CharFunctions::strncpy(out,pieces[numPieces-1].pointer(),pieces[numPieces-1].GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}




template <typename T0, typename T1>
	StringType::Template<T1>		implode(const StringType::Template<T0>&glue, const StringType::Template<T1>*pieces, count_t numPieces)
	{
		if (!numPieces)
			return "";
		size_t len = 0;
		for (size_t i = 0; i < numPieces; i++)
			len += pieces[i].GetLength();
		const T0*glue_str = glue.c_str();
		size_t glue_len = glue.GetLength();
		len += (numPieces-1)*glue_len;
		StringType::Template<T1> result = TStringLength(len);
		T1*out = result.mutablePointer();
		for (size_t i = 0; i < numPieces-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].c_str(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			CharFunctions::strncpy(out,glue_str,glue_len);
			out+=glue_len;
		}
		CharFunctions::strncpy(out,pieces[numPieces-1].c_str(),pieces[numPieces-1].GetLength());
		//out+=pieces[numPieces-1].GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}

template <typename T>
	StringType::Template<T>		implode(T glue, const StringType::Template<T>*pieces, count_t numPieces)
	{
		if (!numPieces)
			return "";
		size_t len = 0;
		for (size_t i = 0; i < numPieces; i++)
			len += pieces[i].GetLength();
		len += (numPieces-1);
		StringType::Template<T> result = StringType::TStringLength(len);
		T*out = result.mutablePointer();
		for (size_t i = 0; i < numPieces-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].c_str(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			(*out++) = glue;
		}
		CharFunctions::strncpy(out,pieces[numPieces-1].c_str(),pieces[numPieces-1].GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}

template <typename T>
	StringType::Template<T>		implode(T glue, const Ctr::ArrayRef<StringType::ReferenceExpression<T> >&pieces)
	{
		return implode(glue, pieces.GetPointer(), pieces.Count());
	}

	
template <typename T0, typename T1>
	StringType::Template<T1>		implode(const T0*glue_str, const StringType::Template<T1>*pieces, count_t numPieces)
	{
		if (!numPieces)
			return "";
		size_t len = 0;
		for (size_t i = 0; i < numPieces; i++)
			len += pieces[i].GetLength();
		size_t glue_len = CharFunctions::strlen(glue_str);
		len += (numPieces-1)*glue_len;
		StringType::Template<T1> result = TStringLength(len);
		T1*out = result.mutablePointer();
		for (size_t i = 0; i < numPieces-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].c_str(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			CharFunctions::strncpy(out,glue_str,glue_len);
			out+=glue_len;
		}
		CharFunctions::strncpy(out,pieces[numPieces-1].c_str(),pieces[numPieces-1].GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}

template <typename T0, typename T1>
	StringType::Template<T1>		implode(const StringType::Template<T0>&glue, const Ctr::ArrayRef<StringType::Template<T1> >&pieces)
	{
		if (!pieces.Count())
			return "";
		size_t len = 0;
		for (size_t i = 0; i < pieces.Count(); i++)
			len += pieces[i].GetLength();
		const T0*glue_str = glue.c_str();
		size_t glue_len = glue.GetLength();
		len += (pieces.Count()-1)*glue_len;
		StringType::Template<T1> result = TStringLength(len);
		T1*out = result.mutablePointer();
		for (size_t i = 0; i < pieces.Count()-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].c_str(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			CharFunctions::strncpy(out,glue_str,glue_len);
			out+=glue_len;
		}
		CharFunctions::strncpy(out,pieces.Last().c_str(),pieces.Last().GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}

template <typename T>
	StringType::Template<T>		implode(T glue, const Ctr::ArrayRef<StringType::Template<T> >&pieces)
	{
		if (!pieces.Count())
			return "";
		size_t len = 0;
		for (size_t i = 0; i < pieces.Count(); i++)
			len += pieces[i].GetLength();
		len += (pieces.Count()-1);
		StringType::Template<T> result = StringType::TStringLength(len);
		T*out = result.mutablePointer();
		for (size_t i = 0; i < pieces.Count()-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].c_str(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			(*out++) = glue;
		}
		CharFunctions::strncpy(out,pieces.Last().c_str(),pieces.Last().GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}
	
template <typename T0, typename T1>
	StringType::Template<T1>		implode(const T0*glue_str, const Ctr::ArrayRef<StringType::Template<T1> >&pieces)
	{
		if (!pieces.Count())
			return "";
		size_t len = 0;
		for (size_t i = 0; i < pieces.Count(); i++)
			len += pieces[i].GetLength();
		size_t glue_len = CharFunctions::strlen(glue_str);
		len += (pieces.Count()-1)*glue_len;
		StringType::Template<T1> result = StringType::TStringLength(len);
		T1*out = result.mutablePointer();
		for (size_t i = 0; i < pieces.Count()-1;i++)
		{
			CharFunctions::strncpy(out,pieces[i].c_str(),pieces[i].GetLength());
			out+=pieces[i].GetLength();
			CharFunctions::strncpy(out,glue_str,glue_len);
			out+=glue_len;
		}
		CharFunctions::strncpy(out,pieces.Last().c_str(),pieces.Last().GetLength());
		//out+=pieces.Last().GetLength();
		//(*out) = 0;
		//ASSERT_EQUAL__(out, (result.c_str()+result.GetLength()));
		return result;
	}
	

template <typename Char, typename Width>
	class Wrapper
	{
	public:
		typedef Width(*FCharWidth)(Char);

		const FCharWidth	charWidth;
		const Width			lineWidth;

		/**/		Wrapper(const Width&lineWidth, const FCharWidth&charWidth):charWidth(charWidth),lineWidth(lineWidth)
		{}

		template <typename CallbackFunctor>
			void	Scan(const ArrayRef<const Char>&line, const CallbackFunctor&callback)
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

template <typename Char, typename Width>
	static Wrapper<Char,Width> MakeWrapper(const Width&lineWidth, Width(*charWidthFunction)(Char))
	{
		return Wrapper<Char,Width>(lineWidth,charWidthFunction);
	}
	
template <typename Char, typename Width>
	void Wrap(const ArrayRef<const Char>&string, const Width&lineWidth, Width (*charWidthFunction)(Char),ArrayData<StringType::ReferenceExpression<Char> >&result)
	{
		auto wrapper = MakeWrapper(lineWidth,charWidthFunction);

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
	
	

template <typename T>
	static count_t	ConstantCharWidthFunction(T)
	{
		return 1;
	}


template <typename Char>
	void 	Wrap(const ArrayRef<const Char>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<Char> >&result)
	{
		Wrap(string,lineCharCount,ConstantCharWidthFunction,result);
	}

template <typename Char>
	void 	Wrap(const ArrayRef<Char>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<Char> >&result)
	{
		ArrayRef<const Char> cp = string.Const();
		Wrap(cp,lineCharCount,ConstantCharWidthFunction,result);
	}

template <typename Char, typename Width>
	void	Wrap(const ArrayRef<Char>&string, const Width&lineWidth, Width (*charWidthFunction)(Char),ArrayData<StringType::ReferenceExpression<Char> >&result)
	{
		ArrayRef<const Char> cp = string.Const();
		Wrap(cp,lineWidth,charWidthFunction,result);
	}



namespace StringType
{

	
	
	template <typename T>
		inline	Template<char>		ReferenceExpression<T>::ToString()			const
							{
								return Template<char>("Reference<")+Template<char>(Super::elements)+">";
							}

	template <typename E0, typename E1>
		inline	Template<char>		ConcatExpression<E0,E1>::ToString()			const
							{
								return Template<char>("Concat<")+exp0.ToString()+", "+exp1.ToString()+">";
							}
						
						
	



	template <typename T>
		inline	Template<char>		StringExpression<T>::ToString()			const
							{
								return Template<char>("Temporary<")+Template<char>(Template<T>::Super::elements)+">";
							}
						
	template <typename T>
		template <class Stream>
			inline	void		StringExpression<T>::Print(Stream&stream)			const
								{
									stream << Template<T>::c_str();
								}						

	template <typename T>
		template <typename T2>
			inline	int		StringExpression<T>::CompareSegment(const T2*string, size_t string_length_)	const
			{
				int result = CharFunctions::strncmp(Template<T>::Super::data,string,string_length_<Template<T>::Super::elements?string_length_:Template<T>::Super::elements);
				if (result)
					return result;
				if (string_length_ < Template<T>::Super::elements)
					return 1;
				return 0;
			}
		
	template <typename T>
		template <typename T2>
			inline	int		StringExpression<T>::CompareTo(const T2*string, size_t string_length_)	const
			{
				int result = CompareSegment(string,string_length_);
				if (!result && string_length_ > Template<T>::Super::elements)
					result = -1;
				return result;
			}

	template <typename T>
		template <typename T2>
			inline	int		StringExpression<T>::CompareSegmentIgnoreCase(const T2*string, size_t string_length_)	const
			{
				int result = CharFunctions::strncmpi(Template<T>::Super::data,string,string_length_<Template<T>::Super::elements?string_length_:Template<T>::Super::elements);
				if (result)
					return result;
				if (string_length_ < Template<T>::Super::elements)
					return 1;
				return 0;
			}
		
	template <typename T>
		template <typename T2>
			inline	int		StringExpression<T>::CompareToIgnoreCase(const T2*string, size_t string_length_)	const
			{
				int result = CompareSegmentIgnoreCase(string,string_length_);
				if (!result && string_length_ > Template<T>::Super::elements)
					result = -1;
				return result;
			}
						

	
	template <typename T>
		T	Template<T>::zero	= 0;
	template <typename T>
		T	*Template<T>::sz		= &Template<T>::zero;

	template <typename T>
		template <typename Expression>
			inline void Template<T>::makeFromExpression(const Expression&e)
			{
				Super::elements = e.GetLength();
				Super::data = allocate(Super::elements);
				e.WriteTo(Super::data);
				//ASSERT_NOT_NULL__(Super::data);
			}

	template <typename T>
		template <typename T0, typename T1>
			Template<T>::Template(const ConcatExpression<T0,T1>&expression)
			{
				makeFromExpression(expression);
				//ASSERT_NOT_NULL__(Super::data);
			}
		
	template <typename T>
		template <class T0>
			Template<T>::Template(const ReferenceExpression<T0>&expression)
			{
				makeFromExpression(expression);
				//ASSERT_NOT_NULL__(Super::data);
			}

	template <typename T>
		template <class T0>
			Template<T>::Template(const StringExpression<T0>&expression)
			{
				makeFromExpression(expression);
				//ASSERT_NOT_NULL__(Super::data);
			}

	
	template <typename T>
		template <typename T2>
			Template<T>::Template(const T2*string, size_t length)
			{
				while (length > 0 && string[length-1] == 0)
					length--;
				Super::data = allocate(length);
				Super::elements = length;
				CharFunctions::Cast(string,Super::data,length);
				//ASSERT_NOT_NULL__(Super::data);
			}	
		
	template <typename T>
			Template<T>::Template(const ArrayRef<T>&array)
			{
				size_t len = array.Length();
				while (len > 0 && array[len-1] == (T)0)
					len --;
				Super::data = allocate(len);
				Super::elements = len;
				for (size_t i = 0; i < len; i++)
					Super::data[i] = array[i];
				//ASSERT_NOT_NULL__(Super::data);
			}
	

	template <typename T>
		void						Template<T>::SetupFromCharArray(const T*string)
		{
			size_t len = CharFunctions::strlen(string);
			Super::data = allocate(len);
			Super::elements = len;
			memcpy(Super::data,string,len*sizeof(T));
		}
	template <typename T>
		template <typename T2>
			void					Template<T>::SetupFromCastCharArray(const T2*string)
			{
				size_t len = CharFunctions::strlen(string);
				Super::data = allocate(len);
				Super::elements = len;
				CharFunctions::Cast(string,Super::data,len);
			}


	template <>
		inline	Template<char>::Template(const char*string)
		{
			SetupFromCharArray(string);
		}
	
	template <typename T>
		inline	Template<T>::Template(const char*string)
		{
			SetupFromCastCharArray(string);
		}


	
	template <>
		template <size_t MaxLength>
			inline	Template<char>::Template(const TFixedString<MaxLength>&string)
			{
				SetupFromCharArray(string.begin);
			}

	
	template <typename T>
		template <size_t MaxLength>
			inline	Template<T>::Template(const TFixedString<MaxLength>&string)
			{
				SetupFromCastCharArray(string.begin);
			}

	template <>
		inline	Template<char>::Template(const TCodeLocation&loc)
		{
			TCodeLocation::TOutString str;
			loc.Format(str);
			SetupFromCharArray(str.begin);
		}

	template <typename T>
		inline	Template<T>::Template(const TCodeLocation&loc)
		{
			TCodeLocation::TOutString str;
			loc.Format(str);
			SetupFromCastCharArray(str.begin);
		}

	template <>
		inline	Template<wchar_t>::Template(const wchar_t*string)
		{
			SetupFromCharArray(string);
		}

	template <typename T>
		inline	Template<T>::Template(const wchar_t*string)
		{
			SetupFromCastCharArray(string);
		}



	template <typename T>
		static inline T* pointerToHexT(const void*pointer, int min_len, T*end, T*first)
		{
			size_t value = (size_t)pointer;
			while (value && end != first)
			{
				BYTE c = value & (0xF);
				if (c < 10)
					(*--end) = (T)('0'+c);
				else
					(*--end) = (T)('A'+c-10);
				value>>=4;
				min_len--;
			}
			while (min_len-->0 && end!=first)
				(*--end) = (T)'0';
			return end;
		}

	template <typename T>
		Template<T>::Template(const void*pointer)
		{
			T buffer[257];
			buffer[256] = 0;
			const T*first = pointerToHexT<T>(pointer,sizeof(pointer)*2,buffer+256,buffer);
			size_t len = buffer+256-first;
		
			Super::data = allocate(len);
			memcpy(Super::data,first,len);
			Super::elements = len;
			//ASSERT_NOT_NULL__(Super::data);
		}


	template <typename T>
		Template<T>::Template(bool b)
		{
			if (b)
			{
				Super::elements = 4;
				Super::data = allocate(4);
				Super::data[0] = (T)'t';
				Super::data[1] = (T)'r';
				Super::data[2] = (T)'u';
				Super::data[3] = (T)'e';
			}
			else
			{
				Super::elements = 5;
				Super::data = allocate(5);
				Super::data[0] = (T)'f';
				Super::data[1] = (T)'a';
				Super::data[2] = (T)'l';
				Super::data[3] = (T)'s';
				Super::data[4] = (T)'e';
			}
		}

	template <typename T>
		Template<T>::Template(char c)
		{
			Super::elements = 1;
			Super::data = allocate(1);
			Super::data[0] = (T)c;
			//ASSERT_NOT_NULL__(Super::data);
		}

	template <typename T>
		Template<T>::Template(wchar_t c)
		{
			Super::elements = 1;
			Super::data = allocate(1);
			Super::data[0] = (T)c;
			//ASSERT_NOT_NULL__(Super::data);
		}
	
	
	template <typename T>
		Template<T>::Template(short value)
		{
			makeSigned<short,unsigned short>(value);
		}
	
	template <typename T>
		Template<T>::Template(unsigned short value)
		{
			makeUnsigned<unsigned short>(value);
		}
	
	template <typename T>
		Template<T>::Template(long value)
		{
			makeSigned<long,unsigned long>(value);
		}
	
	template <typename T>
		Template<T>::Template(int value)
		{
			makeSigned<int,unsigned int>(value);
		}
	
	template <typename T>
		Template<T>::Template(unsigned value)
		{
			makeUnsigned(value);
		}
	
	template <typename T>
		Template<T>::Template(unsigned long value)
		{
			makeUnsigned(value);
		}
	
	template <typename T>
		Template<T>::Template(float value, unsigned char precision, bool force_trailing_zeros)
		{
			makeFloat(value,precision,force_trailing_zeros);
		}

	template <typename T>
		Template<T>::Template(double value, unsigned char precision, bool force_trailing_zeros)
		{
			makeFloat(value,precision,force_trailing_zeros);
		}
	
	template <typename T>
		Template<T>::Template(long double value, unsigned char precision, bool force_trailing_zeros)
		{
			makeFloat(value,precision,force_trailing_zeros);
		}
	
	template <typename T>
		Template<T>::Template(long long value)
		{
			makeSigned<long long, unsigned long long>(value);
		}
	
	template <typename T>
		Template<T>::Template(unsigned long long value)
		{
			makeUnsigned(value);
		}

	#ifdef DSTRING_H
		template <typename T>
			Template<T>::Template(const AnsiString&string)
			{
				Super::elements = string.GetLength();
				Super::data = allocate(Super::elements);
				CharFunctions::strncpy(Super::data,string.c_str(),Super::elements);
			}
	#endif

	
	template <typename T>
		Template<T>::~Template()
		{
			delocate(Super::data);
		}

	template <typename T>
		void			Template<T>::swap(Template<T>&other)
		{
			swp(Super::data,other.Super::data);
			swp(Super::elements,other.Super::elements);
		}

	template <typename T>
		void			Template<T>::adoptData(Template<T>&other)
		{
			if (&other == this)
				return;
			/*
				case 1: fields are identical:
					situation: count(Super::data) = count(other.Super::data) >= 2
					delocate(Super::data) same as delocate(other.Super::data) decreases counter but does not reach 0 (actual memory freeing)
					Super::data copies other.Super::data (which generates an unnecessary copy, though harmless)
					same for Super::elements
					other.Super::data and other.Super::elements are set to empty
					valid situation assuming it was valid before
				case 2: fields are not identical
					trivial
			*/
			delocate(Super::data);
			Super::data = other.Super::data;
			Super::elements = other.Super::elements;
		
			other.Super::data = sz;
			other.Super::elements = 0;
		}
	
	
	template <typename T>
		inline void Template<T>::Resize(size_t new_length)
		{
			//ASSERT_NOT_NULL__(Super::data);
			STRING_DEBUG("invoking resize. current length is "<<(Super::elements+1)<<", new length is "<<(new_length+1));
		
			#if __STR_BLOCK_ALLOC__
				size_t		current_alloc = Super::data!=sz?capacity(Super::data):1,
							new_alloc = pad(new_length+1);
			#else
				size_t		current_alloc = Super::elements+1,
							new_alloc = new_length+1;
			#endif

			STRING_DEBUG(" current allocation is "<<current_alloc<<" at "<<(void*)Super::data);
			STRING_DEBUG(" new allocation is "<<new_alloc);
		
			if (current_alloc == new_alloc
				#if __STR_REFERENCE_COUNT__
					&& (new_alloc==1 || count(Super::data) == 1)
				#endif
				)
			{
				STRING_DEBUG(" nothing new. aborting");
		
				#if __STR_BLOCK_ALLOC__
					if (new_length != Super::elements)
					{
						STRING_DEBUG(" replacing trailing zero and updating Super::elements");

						Super::data[new_length] = 0;
						Super::elements = new_length;
					}
				#endif
				return;
			}
			STRING_DEBUG(" reallocating and copying");
		

		
			delocate(Super::data);
		
			if (!new_length)
			{
				STRING_DEBUG("clearing");
				Super::data = sz;
				Super::elements = 0;
				return;
			}
			Super::data = allocateField(new_alloc);
			Super::data[new_length] = 0;	
			Super::elements = new_length;
		
			STRING_DEBUG("resize done");
		}
	
	template <typename T>
		inline void Template<T>::resizeCopy(size_t new_length)
		{
			STRING_DEBUG("invoking resize copy. current length is "<<(Super::elements+1)<<", new length is "<<(new_length+1));
			#if __STR_BLOCK_ALLOC__
				size_t		current_alloc = Super::data!=sz?capacity(Super::data):1,
							new_alloc = pad(new_length+1);
			#else
				size_t		current_alloc = (Super::elements+1),
							new_alloc = (new_length+1);
			#endif
			STRING_DEBUG(" current allocation is "<<current_alloc<<" at "<<(void*)Super::data);
			STRING_DEBUG(" new allocation is "<<new_alloc);
			if (current_alloc == new_alloc
				#if __STR_REFERENCE_COUNT__
					&& (new_alloc==1 || count(Super::data) == 1)
				#endif
				)
			{
				STRING_DEBUG(" nothing new. aborting");
				#if __STR_BLOCK_ALLOC__
					if (new_length != Super::elements)
					{
						STRING_DEBUG(" replacing trailing zero and updating Super::elements");
						Super::data[new_length] = 0;
						Super::elements = new_length;
					}
				#endif
				return;
			}
		
			if (!new_length)
			{
				STRING_DEBUG(" reducing to empty string");
				delocate(Super::data);
				Super::data = sz;
				Super::elements = 0;
				STRING_DEBUG(" resize done");
				return;
			}
			STRING_DEBUG(" reallocating and copying");
			T*new_field = allocateField(new_alloc);
		
			STRING_DEBUG("copying");
		
			if (new_length > Super::elements)
				memcpy(new_field,Super::data,Super::elements*sizeof(T));
			else
				memcpy(new_field,Super::data,new_length*sizeof(T));
			new_field[new_length] = 0;	
		
			delocate(Super::data);
			Super::data = new_field;
			Super::elements = new_length;
			STRING_DEBUG("resize done");
		}

	
	template <typename T>
		const T*			Template<T>::c_str()		const
		{
			return Super::data;
		}
	
	template <typename T>
		T*					Template<T>::mutablePointer()
		{
			#if __STR_REFERENCE_COUNT__
				duplicate();
			#endif
			return Super::data;
		}

	template <typename T>
		void				Template<T>::free()
		{
			Resize(0);
		}
		/*
	template <typename T>
		inline				Template<T>::operator const T*()					const
		{
			return Super::data;
		}*/


	template <typename T>
		template <class Marker>
			count_t				Template<T>::genericEraseCharacters(Marker marked, bool erase_matches)
			{
				index_t erase_count = 0;
				const T*read = Super::data;
				while (*read)
					erase_count += (marked(*read++) == erase_matches);
				if (!erase_count)
					return 0;
				if (erase_count == Super::elements)
				{
					free();
					return erase_count;
				}

				T*n = allocate(Super::elements-erase_count);
				T*write = n;
				read = Super::data;
				while (*read)
				{
					if (marked(*read) != erase_matches)
						(*write++) = *read;
					read++;
				}
				ASSERT_EQUAL__(read,Super::data+Super::elements);
				ASSERT_EQUAL__(write,n+Super::elements-erase_count);

				Super::elements -= erase_count;
				delocate(Super::data);
				Super::data = n;

				return erase_count;
			}

	template <typename T>
		count_t		Template<T>::EraseCharacter(T chr)
		{
			return genericEraseCharacters(Marker::CharacterMarker<T>(chr),true);
		}

	template <typename T>
		count_t		Template<T>::EraseCharacters(const Template<T>& characters, bool erase_matches)
		{
			if (!characters.Super::elements)
				return 0;
			return genericEraseCharacters(Marker::FieldMarker<T>(characters.Super::data,characters.Super::elements),erase_matches);
		}

	template <typename T>
		count_t		Template<T>::EraseCharacters(const T* characters, bool erase_matches)
		{
			if (!characters)
				return 0;
			size_t len = CharFunctions::strlen(characters);
			if (!len)
				return 0;
			return genericEraseCharacters(Marker::FieldMarker<T>(characters,len),erase_matches);
		}

	template <typename T>
		count_t		Template<T>::EraseCharacters(const T* characters, count_t character_count, bool erase_matches)
		{
			if (!characters || !character_count)
				return 0;
			return genericEraseCharacters(Marker::FieldMarker<T>(characters,character_count),erase_matches);
		}

	template <typename T>
		count_t		Template<T>::EraseCharacters(bool doReplace(T character), bool erase_matches)
		{
			return genericEraseCharacters(doReplace,erase_matches);
		}

	template <typename T>
		count_t		Template<T>::EraseCharactersIgnoreCase(const Template<T>& characters, bool erase_matches)
		{
			if (!characters.Super::elements)
				return 0;
			return genericEraseCharacters(Marker::CaseInsensitiveFieldMarker<T>(characters.Super::data,characters.Super::elements),erase_matches);
		}

	template <typename T>
		count_t		Template<T>::EraseCharactersIgnoreCase(const T* characters, bool erase_matches)										//!< Erases any character that is contained in the zero-terminated string specified by @a characters . Case insensitive
		{
			if (!characters)
				return 0;
			size_t len = CharFunctions::strlen(characters);
			if (!len)
				return 0;
		
			return genericEraseCharacters(Marker::CaseInsensitiveFieldMarker<T>(characters,len),erase_matches);
		}

	template <typename T>
		count_t		Template<T>::EraseCharactersIgnoreCase(const T* characters, count_t character_count, bool erase_matches)
		{
			if (!characters || !character_count)
				return 0;
			return genericEraseCharacters(Marker::CaseInsensitiveFieldMarker<T>(characters,character_count),erase_matches);
		}



	template <typename T>
		template <class Marker>
			void					Template<T>::genericAddSlashes(const Marker&marked)
			{
				count_t matches = Super::GenericCountCharacters(marked,true);
				if (!matches)
					return;

				T*n = allocate(Super::elements+matches);
				T*write = n;
				const T*read = Super::data;
				while (*read)
				{
					if (marked(*read))
						(*write++) = T('\\');
					(*write++) = *read;
					read++;
				}
				ASSERT_EQUAL__(read,Super::data+Super::elements);
				ASSERT_EQUAL__(write,n+Super::elements+matches);

				Super::elements += matches;
				delocate(Super::data);
				Super::data = n;
			}


	template <typename T>
		Template<T>		Template<T>::Escape()	const
		{
			return Template<T>(*this).EscapeThis();
		}
	template <typename T>
		Template<T>		Template<T>::Escape(const T*	before_characters)	const
		{
			return Template<T>(*this).EscapeThis(before_characters);
		}

	template <typename T>
		Template<T>		Template<T>::Escape(const T*	before_characters, count_t before_character_count)	const
		{
			return Template<T>(*this).EscapeThis(before_characters,before_character_count);
		}
	template <typename T>
		Template<T>		Template<T>::Escape(bool isMatch(T character))	const
		{
			return Template<T>(*this).EscapeThis(isMatch);
		}


	template <typename T>
		Template<T>&		Template<T>::EscapeThis()
		{
			genericAddSlashes(CharFunctions::isEscapable<T>);
			return *this;
		}

	template <typename T>
		Template<T>&		Template<T>::EscapeThis(const T*	before_characters)
		{
			if (!before_characters)
				return *this;
			size_t len = CharFunctions::strlen(before_characters);
			if (!len)
				return *this;		
			genericAddSlashes(Marker::FieldMarker<T>(before_characters,len));
			return *this;
		}

	template <typename T>
		Template<T>&		Template<T>::EscapeThis(const T*	before_characters, count_t before_character_count)
		{
			if (!before_characters || !before_character_count)
				return *this;
			genericAddSlashes(Marker::FieldMarker<T>(before_characters,before_character_count));
			return *this;
		}

	template <typename T>
		Template<T>&		Template<T>::EscapeThis(bool isMatch(T character))
		{
			genericAddSlashes(isMatch);
			return *this;
		}

	template <typename T>
		Template<T>		Template<T>::StripSlashes()	const
		{
			Template<T> rs(*this);
			rs.StripSlashesFromThis();
			return rs;
		}

	template <typename T>
		count_t		Template<T>::StripSlashesFromThis()
		{
			return genericEraseCharacters(Marker::UnEscapeMarker<T>(),true);
		}




	template <typename T>
		Template<T>&		Template<T>::Erase(size_t index,size_t count)
		{
			size_t old_len = Super::elements;
			if (index >= Super::elements)
				return *this;
			if (count > Super::elements-index)
				count = Super::elements-index;
			if (!count)
				return *this;

			T*n = allocate(Super::elements-count);
			memcpy(n,Super::data,index*sizeof(T));
			memcpy(n+index,Super::data+(index+count),(Super::elements-index-count)*sizeof(T));
			Super::elements -= count;
			delocate(Super::data);
			Super::data = n;
			return *this;
		}

	template <typename T>
		Template<T>&		Template<T>::EraseLeft(size_t count_)
		{
			if (!count_)
				return *this;
			if (count_ >= Super::elements)
			{
				delocate(Super::data);
				Super::data = sz;
				Super::elements = 0;
				return *this;
			}
			size_t	new_length = Super::elements-count_;

		
			STRING_DEBUG("invoking eraseLeft. current length is "<<(Super::elements+1)<<", new length is "<<(new_length+1));
			#if __STR_BLOCK_ALLOC__
				size_t	current_alloc = Super::data!=sz?capacity(Super::data):1,
						new_alloc = pad(new_length+1);
				STRING_DEBUG(" current allocation is "<<current_alloc<<" at "<<(void*)Super::data);
			#else
				size_t	new_alloc = (new_length+1);
			#endif
		
			STRING_DEBUG(" new allocation is "<<new_alloc);
			#if __STR_BLOCK_ALLOC__
				if (current_alloc == new_alloc
					#if __STR_REFERENCE_COUNT__
						&& count(Super::data) == 1
					#endif
					)
				{
					STRING_DEBUG(" no re-allocation necessary. simply moving to front");
					memcpy(Super::data, Super::data+count_,new_length);
					Super::data[new_length] = 0;
					Super::elements = new_length;
				
					return *this;
				}
			#endif
		
			T*new_field = allocateField(new_alloc);
		
			STRING_DEBUG("copying");
		
			memcpy(new_field,Super::data+count_,new_length*sizeof(T));
			new_field[new_length] = 0;	
		
			delocate(Super::data);
			Super::data = new_field;
			Super::elements = new_length;
			STRING_DEBUG("operation done");
		
		
			return *this;
		}

	template <typename T>
		Template<T>&		Template<T>::Truncate(size_t maxLength)
		{
			if (Super::elements <= maxLength)
				return *this;
			return EraseRight(Super::elements - maxLength);
		}


	template <typename T>
		Template<T>&		Template<T>::EraseRight(size_t count)
		{
			if (!count)
				return *this;
			if (count >= Super::elements)
			{
				delocate(Super::data);
				Super::data = sz;
				Super::elements = 0;
				return *this;
			}
		
			resizeCopy(Super::elements-count);

		
			return *this;
		}


	template <typename T>
		Template<T>&	Template<T>::setLength(size_t newLength)
		{
			resizeCopy(newLength);
			return *this;
		}


	template <typename T>
		template <typename IndexType>
		Template<T>				Template<T>::subString(IndexType index, size_t count) const
		{
			if (index<0)
			{
				count += index;
				index=0;
			}
			if ((size_t)index >= Super::elements)
				return Template<T>();
			if (count > Super::elements)
				count = Super::elements;
			if (index+count>Super::elements)
				count = Super::elements-index;
			return Template<T>(Super::data+index,count);
		}


	
	template <typename T>
		ReferenceExpression<T>				Template<T>::subStringRef(sindex_t index, size_t count) const
		{
			if (index<0)
			{
				count += index;
				index=0;
			}
			if ((size_t)index >= Super::elements)
				return ReferenceExpression<T>(Super::data,0);
			if (count > Super::elements)
				count = Super::elements;
			if (index+count>Super::elements)
				count = Super::elements-index;
			return ReferenceExpression<T>(Super::data+index,count);
		}

	template <typename T>
		ReferenceExpression<T>				Template<T>::ref()	const
		{
			return ReferenceExpression<T>(Super::data,Super::elements);
		}




	template <typename T>
		Template<T>&				Template<T>::TrimThis()
		{
			if (!Super::elements || (!IsWhitespace(Super::data[0]) && !IsWhitespace(Super::data[Super::elements-1])))
				return *this;

			const T*left=Super::data;
			while ((*left) && IsWhitespace(*left))
				left++;
			if (!*left)
			{
				delocate(Super::data);
				Super::data = sz;
				Super::elements = 0;
				return *this;
			}
			const T*right = Super::data+Super::elements-1;
			while (IsWhitespace(*right))
				right--;
			size_t len = right-left+1;

			T*n = allocate(len);
			memcpy(n,left,len*sizeof(T));
			delocate(Super::data);
			Super::data = n;
			Super::elements = len;
			return *this;
		}


	template <typename T>
		ReferenceExpression<T>				ReferenceExpression<T>::Trim()			const
		{
			if (!Super::elements)
				return ReferenceExpression<T>();
			const T*left=Super::begin();
			const T*const end = Super::end();
			while (left < end && IsWhitespace(*left))
				left++;
			if (left == end)
				return ReferenceExpression<T>();
			const T*right = end-1;
			while (IsWhitespace(*right))
				right--;
			size_t len2 = right-left+1;
			if (!left && len2 == Super::elements)
				return *this;
			return ReferenceExpression<T>(left,len2);
		}

	template <typename T>
		ReferenceExpression<T>				ReferenceExpression<T>::SubStringRefS(sindex_t index, count_t count /*= (count_t)-1*/)	 const
		{
			if (index<0)
			{
				count += index;
				index=0;
			}
			if ((size_t)index >= Super::elements)
				return ReferenceExpression<T>(Super::data,0);
			if (count > Super::elements)
				count = Super::elements;
			if (index+count>Super::elements)
				count = Super::elements-index;
			return ReferenceExpression<T>(Super::data+index,count);
		}


	template <typename T>
		ReferenceExpression<T>				ReferenceExpression<T>::TrimLeft()		const
		{
			size_t left = 0;
	//		const T*const end = pointer + len;
			while (left < Super::elements && IsWhitespace(Super::data[left]))
				left++;
			if (!left)
				return *this;
			return ReferenceExpression<T>(Super::data+left,Super::elements-left);
		}


	template <typename T>
		ReferenceExpression<T>				ReferenceExpression<T>::TrimRight()		const
		{
			if (!Super::elements)
				return ReferenceExpression<T>();
			size_t right = Super::elements-1;
			while (right && IsWhitespace(Super::data[right]))
				right--;
			right+=!IsWhitespace(Super::data[right]);  //should increase cnt if necessary
		
			if (right == Super::elements)
				return *this;
			return ReferenceExpression<T>(Super::data,right);
		}

	

	template <typename T>
		Template<T>				Template<T>::Trim()			const
		{
			return ToRef().Trim();
		}

	template <typename T>
		ReferenceExpression<T>				Template<T>::TrimRef()			const
		{
			return ToRef().Trim();
		}

	template <typename T>
		Template<T>				Template<T>::TrimLeft()		const
		{
			return ToRef().TrimLeft();
		}

	template <typename T>
		ReferenceExpression<T>				Template<T>::TrimLeftRef()		const
		{
			return ToRef().TrimLeft();
		}

	template <typename T>
		Template<T>				Template<T>::TrimRight()		const
		{
			return ToRef().TrimRight();
		}

	template <typename T>
		ReferenceExpression<T>				Template<T>::TrimRightRef()		const
		{
			return ToRef().TrimRight();
		}


	template <typename T>
		Template<T>&				Template<T>::Insert(size_t index, const ReferenceExpression<T>&str)
		{
			if (str.IsEmpty())
				return *this;
			if (!Super::elements)
				return this->operator=(str);

			if (index >= Super::elements)
				index = Super::elements-1;
			T*n = allocate(Super::elements+str.GetLength());
		
			memcpy(n,Super::data,index*sizeof(T));
			memcpy(n+index,str.pointer(),str.GetLength()*sizeof(T));
			memcpy(n+index+str.GetLength(),Super::data+index,(Super::elements-index)*sizeof(T));
			Super::elements += str.GetLength();
			delocate(Super::data);
			Super::data = n;
			return *this;
		}

	template <typename T>
		Template<T>&				Template<T>::ReplaceSubString(size_t index, size_t count, const ReferenceExpression<T>&str)
		{
			if (str.IsEmpty())
				return Erase(index,count);
		
			if (!Super::elements)
				return this->operator=(str);
			
			if (index >= Super::elements)
				index = Super::elements-1;
			if (count > Super::elements-index)
				count = Super::elements-index;

			size_t new_len = Super::elements-count+str.GetLength();
			T*n = allocate(new_len);
			memcpy(n,Super::data,index*sizeof(T));
			memcpy(n+index,str.pointer(),str.GetLength()*sizeof(T));
			memcpy(n+index+str.GetLength(),Super::data+(index+count),(Super::elements-index-count)*sizeof(T));
			Super::elements = new_len;
			delocate(Super::data);
			Super::data = n;
			return *this;
		}

	template <typename T>
		Template<T>&				Template<T>::Insert(size_t index, T c)
		{
			if (!Super::elements)
			{
				this->operator=(c);
				return *this;
			}
			if (index >= Super::elements)
				index = Super::elements-1;
			T*n = allocate(Super::elements+1);
		
			memcpy(n,Super::data,index*sizeof(T));
			n[index] = c;
			memcpy(n+index+1,Super::data+index,(Super::elements-index)*sizeof(T));
			Super::elements ++;
			delocate(Super::data);
			Super::data = n;
			return *this;

		}


	template <typename T>
		Template<T>&				Template<T>::ConvertToLowerCase()
		{
			#if __STR_REFERENCE_COUNT__
				duplicate();
			#endif
			CharFunctions::strlwr(Super::data);
			return *this;
		}

	template <typename T>
		Template<T>&				Template<T>::ConvertToUpperCase()
		{
			#if __STR_REFERENCE_COUNT__
				duplicate();
			#endif
			CharFunctions::strupr(Super::data);
			return *this;
		}

	template <typename T>
		Template<T>				Template<T>::CopyToLowerCase()	const
		{
			return Template<T>(*this).ConvertToLowerCase();
		}

	template <typename T>
		Template<T>				Template<T>::CopyToUpperCase()	const
		{
			return Template<T>(*this).ConvertToUpperCase();
		}

	template <typename T>
		Template<T>				Template<T>::GetBetween(const Template<T>&left_delimiter, const Template<T>&right_delimiter)	const
		{
			const T*begin = CharFunctions::strstr(Super::data,left_delimiter.Super::data);
			if (!begin)
				return Template<T>();
			begin += left_delimiter.Super::elements;
			const T*end = CharFunctions::strstr(begin,right_delimiter.Super::data);
			if (!end)
				return Template<T>();
			return Template<T>(begin,end-begin);
		}

	template <typename T>
		ReferenceExpression<T>				Template<T>::GetBetweenRef(const Template<T>&left_delimiter, const Template<T>&right_delimiter)	const
		{
			const T*begin = CharFunctions::strstr(Super::data,left_delimiter.Super::data);
			if (!begin)
				return ReferenceExpression<T>(Super::data,0);
			begin += left_delimiter.Super::elements;
			const T*end = CharFunctions::strstr(begin,right_delimiter.Super::data);
			if (!end)
				return ReferenceExpression<T>(Super::data,0);
			return ReferenceExpression<T>(begin,end-begin);
		}

	template <typename T>
		const T*		Template<T>::lastCharPointer()					const
		{
			return Super::data+Super::elements-1;
		}



	template <typename T>
		Template<T>				Template<T>::GetFirstWord()							const
		{
			const T*begin = Super::data;
			while (IsWhitespace(*begin))
				begin++;
			const T*end = begin;
			while (*end && !IsWhitespace(*end))
				end++;
			return Template<T>(begin,end-begin);
		}

	template <typename T>
		ReferenceExpression<T>				Template<T>::GetFirstWordRef()							const
		{
			const T*begin = Super::data;
			while (IsWhitespace(*begin))
				begin++;
			const T*end = begin;
			while (*end && !IsWhitespace(*end))
				end++;
			return ReferenceExpression<T>(begin,end-begin);
		}

	template <typename T>
		Template<T>				Template<T>::GetLastWord()							const
		{
			return GetLastWordRef();
		}
	
	template <typename T>
		ReferenceExpression<T>				Template<T>::GetLastWordRef()							const
		{
			const T	*end = Super::data+Super::elements-1,
					*begin = Super::data;
			while (end >= begin && IsWhitespace(*end))
				end--;
			if (end < begin)
				return ReferenceExpression<T>();
			begin = end;
			while (begin >= Super::data && !IsWhitespace(*begin))
				begin--;
			begin++;
			end++;
		
			return ReferenceExpression<T>(begin,end-begin);
		}


	template <typename T>
		count_t				Template<T>::FindAndReplace(const ReferenceExpression<T>&needle, const ReferenceExpression<T>&replacement)
		{
			index_t at = 0;
			count_t rs = 0;
			while ((at = Find(needle,at))!=InvalidIndex)
			{
				rs++;
				ReplaceSubString(at,needle.GetLength(),replacement);
				at += replacement.GetLength();
			}
			return rs;
		}

	template <typename T>
		count_t				Template<T>::FindAndReplaceIgnoreCase(const ReferenceExpression<T>&needle, const ReferenceExpression<T>&replacement)
		{
			index_t at = 0;
			count_t rs = 0;
			while ((at = FindIgnoreCase(needle,at))!=InvalidIndex)
			{
				rs++;
				ReplaceSubString(at,needle.GetLength(),replacement);
				at += replacement.GetLength();
			}
			return rs;
		}

	template <typename T>
		count_t				Template<T>::FindAndReplace(T needle, const ReferenceExpression<T>&replacement)
		{
			count_t rs = 0;
			index_t at = 0;
			while ((at = Find(needle,at))!=InvalidIndex)
			{
				rs++;
				ReplaceSubString(at,1,replacement);
				at += replacement.Length();
			}
			return rs;
		}

	template <typename T>
		count_t	Template<T>::FindAndReplace(bool doReplace(T character), T replacement)
		{
			#if __STR_REFERENCE_COUNT__
				duplicate();
			#endif
			count_t rs = 0;
			for (size_t i = 0; i < Super::elements; i++)
				if (doReplace(Super::data[i]))
				{
					Super::data[i] = replacement;
					rs++;
				}
			return rs;
		}

	template <typename T>
		count_t	Template<T>::FindAndReplace(T replace_what, T replace_with)
		{
			#if __STR_REFERENCE_COUNT__
				duplicate();
			#endif
			count_t rs = 0;
			for (size_t i = 0; i < Super::elements; i++)
				if (Super::data[i] == replace_what)
				{
					Super::data[i] = replace_with;
					rs++;
				}
			return rs;
		}

	template <typename T>
		void				Template<T>::Set(size_t index, T c)
		{
			if (index >= Super::elements)
				return;
			#if __STR_REFERENCE_COUNT__
				duplicate();
			#endif
			Super::data[index] = c;
		}



	#if 0
	template <typename T>
		T				Template<T>::operator[](size_t index) const
		{
			if (index >= Super::elements)
				return 0;
			return Super::data[index];
		}

	template <typename T>
		T				Template<T>::operator[](int index)			const
		{
			if (/*index < 0 ||*/ (size_t)index >= Super::elements)
				return 0;
			return Super::data[index];
		}

	template <typename T>
		T				Template<T>::operator[](BYTE index)			const
		{
			if ((size_t)index >= Super::elements)
				return 0;
			return Super::data[index];
		}
	#endif

	template <typename T>
		void	Template<T>::operator+=(const Template<T>&other)
		{
			if (!other.Super::elements)
				return;
			STRING_METHOD_BEGIN("(const Template<T>&other)",other);
			size_t old_length = Super::elements;
			size_t new_length = Super::elements + other.Super::elements;
			resizeCopy(new_length);
			if (&other != this)
				memcpy(Super::data+old_length,other.Super::data,other.Super::elements*sizeof(T));
			else
				memcpy(Super::data+old_length,Super::data,old_length*sizeof(T));
			STRING_METHOD_END
		}

	template <typename T>	
		void	Template<T>::operator+=(const T*string)
		{
			if (!string)
				return;
			STRING_METHOD_BEGIN("(const T*string)",string);
			size_t 	len = CharFunctions::strlen(string);
		
			if (!len)
			{
				STRING_METHOD_END
				return;
			}
			size_t	old_length = Super::elements,
					new_length = Super::elements+len;
			bool self = Super::data == string;
			resizeCopy(new_length);
			if (!self)
				memcpy(Super::data+old_length,string,len*sizeof(T));
			else
				memcpy(Super::data+old_length,Super::data,old_length*sizeof(T));
			STRING_METHOD_END
		}

	template <typename T>	
		void	Template<T>::operator+=(T c)
		{
			STRING_METHOD_BEGIN("(T c)",c);
			size_t new_length = Super::elements + 1;
			resizeCopy(new_length);
			Super::data[Super::elements-1] = c;	//Super::elements increased during resizeCopy(). Trailing zero should be set
			STRING_METHOD_END
		}

	
	template <typename T>
		template <typename Expression>
			inline void	Template<T>::appendExpression(const Expression&expression)
			{
				size_t	old_length = Super::elements,
						new_length = Super::elements + expression.GetLength();
				if (new_length == Super::elements)
					return;
				if (Super::elements && expression.References(Super::data))
				{
					T*new_field = allocate(new_length);
					memcpy(new_field,Super::data,old_length*sizeof(T));
					expression.WriteTo(new_field+old_length);
					delocate(Super::data);
					Super::data = new_field;
					Super::elements = new_length;
				}
				else
				{
					resizeCopy(new_length);
					expression.WriteTo(Super::data+old_length);
				}
			}

	template <typename T>	
		template <typename T0, typename T1>
			void	Template<T>::operator+=(const ConcatExpression<T0,T1>&expression)
			{
				appendExpression(expression);
			}
		
	template <typename T>	
		template <class T0>
			void				Template<T>::operator+=(const CharacterExpression<T0>&expression)
			{
				appendExpression(expression);
			}
				
	template <typename T>	
		template <class T0>
			void				Template<T>::operator+=(const ReferenceExpression<T0>&expression)
			{
				appendExpression(expression);
			}
		
	template <typename T>
		template <class T0>
			void				Template<T>::operator+=(const StringExpression<T0>&expression)
			{
				appendExpression(expression);
			}



	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::operator<(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator<(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>0;
			}
				
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator<(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::operator<(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::operator<=(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>=0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator<=(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>=0;
			}
				
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator<=(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>=0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::operator<=(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)>=0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::operator>=(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<=0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator>=(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<=0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator>=(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<=0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::operator>=(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<=0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::operator>(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator>(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator>(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::operator>(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)<0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::operator!=(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)!=0;
			}
		
	template <typename T>	
		template <typename T0>
			bool	Template<T>::operator!=(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)!=0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator!=(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)!=0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::operator!=(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)!=0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::operator==(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator==(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}
				
	template <typename T>	
		template <class T0>
			bool				Template<T>::operator==(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::operator==(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::Equals(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}
		
	template <typename T>	
		template <class T0>
			bool				Template<T>::Equals(const ReferenceExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}
				
	template <typename T>	
		template <class T0>
			bool				Template<T>::Equals(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::Equals(const StringExpression<T0>&expression)	const
			{
				return expression.CompareTo(Super::data,Super::elements)==0;
			}

	template <typename T>	
		template <typename T0, typename T1>
			bool	Template<T>::EqualsIgnoreCase(const ConcatExpression<T0,T1>&expression)	const
			{
				return expression.CompareToIgnoreCase(Super::data,Super::elements)==0;
			}
		
				
	template <typename T>	
		template <class T0>
			bool				Template<T>::EqualsIgnoreCase(const CharacterExpression<T0>&expression)	const
			{
				return expression.CompareToIgnoreCase(Super::data,Super::elements)==0;
			}
		
	template <typename T>
		template <class T0>
			bool				Template<T>::EqualsIgnoreCase(const StringExpression<T0>&expression)	const
			{
				return expression.CompareToIgnoreCase(Super::data,Super::elements)==0;
			}

		
	#ifdef DSTRING_H
		template <typename T>
			bool				Template<T>::operator<(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())<0;
			}

		template <typename T>
			bool				Template<T>::operator<=(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())<=0;
			}

		template <typename T>
			bool				Template<T>::operator>(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())>0;
			}

		template <typename T>
			bool				Template<T>::operator>=(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())>=0;
			}

		template <typename T>
			bool				Template<T>::operator==(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())==0;
			}

		template <typename T>
			bool				Template<T>::Equals(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())==0;
			}

		template <typename T>
			bool				Template<T>::equalsCaseIgnore(const AnsiString&string)	const
			{
				return CharFunctions::strcmpi(Super::data,string.c_str())==0;
			}

		template <typename T>
			bool				Template<T>::operator!=(const AnsiString&string)	const
			{
				return CharFunctions::strcmp(Super::data,string.c_str())!=0;
			}

	#endif
	

	template <typename T>
		bool				Template<T>::operator<(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)<0;
		}
	
	template <typename T>
		bool				Template<T>::operator<=(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)<=0;
		}
	
	template <typename T>
		bool				Template<T>::operator>(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)>0;
		}
	
	template <typename T>
		bool				Template<T>::operator>=(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)>=0;
		}
	
	template <typename T>
		bool				Template<T>::operator==(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)==0;
		}
	
	template <typename T>
		bool				Template<T>::operator!=(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)!=0;
		}

	template <typename T>
		bool				Template<T>::operator<(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)<0;
		}

	template <typename T>
		bool				Template<T>::operator<=(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)<=0;
		}

	template <typename T>
		bool				Template<T>::operator>(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)>0;
		}

	template <typename T>
		bool				Template<T>::operator>=(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)>=0;
		}

	template <typename T>
		bool				Template<T>::operator==(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)==0;
		}

	template <typename T>
		bool				Template<T>::Equals(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)==0;
		}
	template <typename T>
		bool				Template<T>::Equals(T chr)		const
		{
			return Super::data[0] == chr && !Super::data[1];;
		}
	template <typename T>
		bool				Template<T>::Equals(const Template<T>&string)		const
		{
			return CharFunctions::strcmp(Super::data,string.Super::data)==0;
		}




	template <typename T>
		bool				Template<T>::operator!=(const T*string)		const
		{
			return CharFunctions::strcmp(Super::data,string)!=0;
		}


	template <typename T>
		bool				Template<T>::operator<(T c)		const
		{
			return Super::data[0] < c;
		}
	
	template <typename T>
		bool				Template<T>::operator<=(T c)		const
		{
			return Super::data[0] <= c;
		}
	
	template <typename T>
		bool				Template<T>::operator>(T c)		const
		{
			return Super::data[0] > c;
		}
	
	template <typename T>
		bool				Template<T>::operator>=(T c)		const
		{
			return Super::data[0] >= c;
		}
	
	template <typename T>
		bool				Template<T>::operator==(T c)		const
		{
			return Super::data[0] == c && Super::elements==1;
		}
	
	template <typename T>
		bool				Template<T>::operator!=(T c)		const
		{
			return Super::data[0] != c || Super::elements!=1;
		}
	

	
	#ifdef DSTRING_H
		template <typename T>
			Template<T>&	Template<T>::operator=(const AnsiString&string)
			{
				Resize(string.GetLength());
				CharFunctions::strncpy(Super::data,string.c_str(),Super::elements);
				return *this;
			}
	#endif
	

	template <typename T>
		Template<T>&	Template<T>::operator=(const ArrayRef<T>&string)
		{
			//ASSERT_NOT_NULL__(Super::data);
			STRING_METHOD_BEGIN("(const Ctr::ArrayData<T>&string)",string.pointer());
			count_t length = string.Length();
			while (length > 0 && string[length-1] == (T)0)
				length--;
			if (!length)
				Resize(0);
			else
			{
				const T	*first = string.pointer(),
						*last = first+length-1;
				while (last >= first && !*last)
					last--;
				Resize(last-first+1);
				CharFunctions::strncpy(Super::data,string.pointer(),Super::elements);
			}
			STRING_METHOD_END
			return *this;
		}

	#if __STR_RVALUE_REFERENCES__			
	template <typename T>
		Template<T>&	Template<T>::operator=(Template<T>&&string)
		{
			adoptData(string);
			return *this;
		}
	#endif


	template <typename T>
		Template<T>&	Template<T>::operator=(const Template<T>&string)
		{
			//ASSERT_NOT_NULL__(Super::data);
			STRING_METHOD_BEGIN("(const Template<T>&string)",string.c_str());
			if (&string == this)
			{
				STRING_METHOD_END
				return *this;
			}
			#if __STR_REFERENCE_COUNT__
				delocate(Super::data);
				Super::elements = string.Super::elements;
				Super::data = string.Super::data;
				if (Super::data != sz)
					count(Super::data)++;
			#else
				Resize(string.Super::elements);
				memcpy(Super::data,string.Super::data,Super::elements*sizeof(T));
			#endif
			STRING_METHOD_END
			return *this;
		}
	
	template <typename T>
		template <typename T2>
			Template<T>&	Template<T>::operator=(const T2*string)
			{
				//ASSERT_NOT_NULL__(Super::data);
				STRING_METHOD_BEGIN("(const T2*string)",string);
				if (string == (const T2*)Super::data)
				{
					STRING_METHOD_END
					return *this;
				}
				Resize(CharFunctions::strlen(string));
				for (size_t i = 0; i < Super::elements; i++)
					Super::data[i] = (T)string[i];
				STRING_METHOD_END
				return *this;
			}	
	template <typename T>
		Template<T>&	Template<T>::operator=(const T*string)
		{
			//ASSERT_NOT_NULL__(Super::data);
			STRING_METHOD_BEGIN("(const T*string)",string);
			if (string == nullptr)
			{
				free();
				STRING_METHOD_END
				return *this;
			}
			if (string == Super::data)
			{
				STRING_METHOD_END
				return *this;
			}
			Resize(CharFunctions::strlen(string));
			memcpy(Super::data,string,Super::elements*sizeof(T));
			STRING_METHOD_END
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(char c)
		{
			//ASSERT_NOT_NULL__(Super::data);
			STRING_METHOD_BEGIN("(char c)",c);
			STRING_DEBUG("resize to 1");
		
			Resize(1);
		
			STRING_DEBUG("write char");
		
			Super::data[0] = (T)c;
		
			STRING_DEBUG("done");
			STRING_METHOD_END
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(wchar_t c)
		{
			//ASSERT_NOT_NULL__(Super::data);
			STRING_METHOD_BEGIN("(wchar_t c)",(char)c);		
			Resize(1);
			Super::data[0] = (T)c;
			STRING_METHOD_END
			return *this;
		}



	template <typename T>
		template <class Type, class UType>
			void Template<T>::copySigned(Type value)
			{
				bool negative = false;
				if (value < 0)
				{
					value*=-1;
					negative = true;
				}

				T	char_buffer[256],
					*end = char_buffer+ARRAYSIZE(char_buffer),
					*c = end;
				UType uval = value;
				while (uval && c != char_buffer)
				{
					(*(--c)) = (T)('0'+(uval%10));
					uval/=10;
				}
				if (c==end)
					(*(--c)) = (T)'0';
				Resize(end-c+negative);
				if (negative)
					Super::data[0] = '-';
				memcpy(Super::data+negative,c,(end-c)*sizeof(T));
			}

	template <typename T>
		template <class Type>
			void Template<T>::copyUnsigned(Type value)
			{
				T	char_buffer[256],
					*end = char_buffer+ARRAYSIZE(char_buffer),
					*c = end;
				while (value && c != char_buffer)
				{
					(*(--c)) = (T)('0'+(value%10));
					value/=10;
				}
				if (c==end)
					(*(--c)) = (T)'0';
				Resize(end-c);
				memcpy(Super::data,c,(end-c)*sizeof(T));
			}
		

	template <typename T>
		template <class Type, class UType>
			void Template<T>::makeSigned(Type value)
			{
				bool negative = false;
				if (value < 0)
				{
					value*=-1;
					negative = true;
				}

				T	char_buffer[256],
					*end = char_buffer+ARRAYSIZE(char_buffer),
					*c = end;
				UType uval = value;
				while (uval && c != char_buffer)
				{
					(*(--c)) = (T)('0'+(uval%10));
					uval/=10;
				}
				if (c==end)
					(*(--c)) = (T)'0';
				Super::elements = end-c+negative;
				Super::data = allocate(Super::elements);
				if (negative)
					Super::data[0] = '-';
				memcpy(Super::data+negative,c,(end-c)*sizeof(T));
			}


	template <typename T>
		template <class Type>
			void Template<T>::makeUnsigned(Type value)
			{
				T	char_buffer[256],
					*end = char_buffer+ARRAYSIZE(char_buffer),
					*c = end;
				while (value && c != char_buffer)
				{
					(*(--c)) = (T)('0'+(value%10));
					value/=10;
				}
				if (c==end)
					(*(--c)) = (T)'0';
				Super::elements = end-c;
				Super::data = allocate(Super::elements);
				memcpy(Super::data,c,(end-c)*sizeof(T));
			}





	

	template <typename T>
		Template<T>&	Template<T>::operator=(short value)
		{
			copySigned<short,unsigned short>(value);
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(unsigned short value)
		{
			copyUnsigned<unsigned short>(value);
			return *this;
		}



	template <typename T>
		Template<T>&	Template<T>::operator=(int value)
		{
			copySigned<int,unsigned>(value);
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(long value)
		{
			copySigned<long,unsigned long>(value);
			return *this;
		}


	template <typename T>
		Template<T>&	Template<T>::operator=(unsigned value)
		{
			copyUnsigned<unsigned>(value);
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(unsigned long value)
		{
			copyUnsigned<unsigned long>(value);
			return *this;
		}


	template <typename T>
		Template<T>&	Template<T>::operator=(long long value)
		{
			copySigned<long long, unsigned long long>(value);
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(unsigned long long value)
		{
			copyUnsigned<unsigned long long>(value);
			return *this;
		}

	template <typename T>
		template <class Type, class UType> 
			T* Template<T>::SignedToStr(Type value, T*end, T*first)
			{
				bool negative = false;
				if (value < 0)
				{
					value*=-1;
					negative = true;
				}

				T *c = end;
				UType uval = value;
				while (uval && c != first)
				{
					(*(--c)) = '0'+(uval%10);
					uval/=10;
				}
				if (c==end)
					(*(--c)) = '0';
				if (negative && c != first)
					(*(--c)) = '-';
				return c;
			}
	
	template <typename T>
		template <typename Type>
			T* Template<T>::FloatToStr(Type value, unsigned char exactness, bool force_trailing_zeros, T*end, T*first)
			{
				ASSERT__(end > first);
				if (isnan(value))
				{
					end-=3;
					end[0] = L'N';
					end[1] = L'a';
					end[2] = L'N';
					return end;
				}
				if (isinf(value))
				{
					end -= 4;
					if (value > 0)
						(*end) = L'+';
					else
						(*end) = L'-';
					end[1] = L'I';
					end[2] = L'N';
					end[3] = L'F';
					return end;
				}
			
				bool negative = value < 0;
				if (negative)
					value*=-1;
			
				T*c = end;
				//(*c) = 0; no other possibility writes terminal zeros. so this one does neither
				if (value > INT_MAX)
				{
					int exponent = (int)log10(value);
					value /= pow(10.0,exponent);
					c = SignedToStr<int,unsigned>(exponent,c,first);
					if (c != first)
						(*(--c)) = L'e';
				}
				
				double sub_decimal = pow(10.0f,exactness)*value;
				if (sub_decimal > UINT64_MAX)
				{
					exactness = (BYTE)log10(UINT64_MAX/value);
					sub_decimal = pow(10.0f,exactness)*value;
				}
				UINT64 v = (UINT64)round(sub_decimal);

				size_t at(0);
				bool write = !exactness || force_trailing_zeros;	//start write trailing zeros immediately
				while (v && c != first)
				{
					T ch = '0'+(v%10);
					write = write || ch != L'0' || at==exactness;
					if (write)
						(*--c) = ch;
					v/=10;
					at++;
					if (at == exactness && c != first && write)
						(*--c) = globalDecimalSeparator;
				}
				if (c != end)
				{
					while (at < exactness && c != first)
					{
						(*--c) = L'0';
						at++;
					}
					if (at == exactness && c != first)
					{
						if ((*c) != globalDecimalSeparator)
							(*--c) = globalDecimalSeparator;
						if (c != first)
							(*--c) = L'0';
					}
					if (negative && c != first)
						(*--c) = L'-';
				}
				else
				{
					if (force_trailing_zeros)
					{
						while (exactness && first != c)
						{
							(*--c) = L'0';
							exactness--;
						}
						if (c != first)
							(*--c) = globalDecimalSeparator;
						if (c != first)
							(*--c) = L'0';
					}
					else
						(*--c) = L'0';
				}
			//	len = end-c;
				return c;
			}


	template <typename T>
		template <typename Float>
			inline void Template<T>::copyFloat(Float value)
			{
				T	buffer[257],
					*str = FloatToStr(value, 5, false,buffer+ARRAYSIZE(buffer)-1, buffer);

				Resize(buffer+ARRAYSIZE(buffer)-str-1);
				memcpy(Super::data,str,Super::elements*sizeof(T));
			}

	template <typename T>
		template <typename Float>
			inline void Template<T>::makeFloat(Float value,unsigned char precision, bool force_trailing_zeros)
			{
				T	buffer[257],
					*str = FloatToStr(value, precision, force_trailing_zeros, buffer+ARRAYSIZE(buffer)-1, buffer);

				Super::elements = buffer+ARRAYSIZE(buffer)-str-1;
				Super::data = allocate(Super::elements);
				memcpy(Super::data,str,Super::elements*sizeof(T));
			}
		

	template <typename T>
		Template<T>&	Template<T>::operator=(double value)
		{
			copyFloat(value);
			return *this;
		}

	template <typename T>
		Template<T>&	Template<T>::operator=(long double value)
		{
			copyFloat(value);
			return *this;
		}


	template <typename T>
		Template<T>&	Template<T>::operator=(float value)
		{
			copyFloat(value);
			return *this;
		}

	template <typename T>
		template <typename Expression>
			inline void Template<T>::copyExpression(const Expression&expression)
			{
				if (Super::elements && expression.References(Super::data))
				{
					size_t len = expression.GetLength();
					T*new_field = allocate(len);
					STRING_DEBUG("writing expression to new Super::data...");
					expression.WriteTo(new_field);
					STRING_DEBUG("new Super::data is filled with '"<<new_field<<"' now");
					STRING_DEBUG("discarding old Super::data");
					delocate(Super::data);
					Super::data = new_field;
					Super::elements = len;
				}
				else
				{
					Resize(expression.GetLength());
					expression.WriteTo(Super::data);
				}
				//CHECK_STRING(*this);
			}
	
	template <typename T>
		template <typename T0, typename T1>
			Template<T>&	Template<T>::operator=(const ConcatExpression<T0,T1>&expression)
			{
				copyExpression(expression);
				return *this;
			}
		
	template <typename T>
		template <class T0>
			Template<T>&	Template<T>::operator=(const ReferenceExpression<T0>&expression)
			{
				copyExpression(expression);
				return *this;
			}
		
	template <typename T>
		template <class T0>
			Template<T>&	Template<T>::operator=(const StringExpression<T0>&expression)
			{
				copyExpression(expression);
				return *this;
			}

	#if __STR_RVALUE_REFERENCES__		
		template <typename T>
			Template<T>&	Template<T>::operator=(StringExpression<T>&&expression)
			{
				adoptData(expression);
				return *this;
			}
	#endif

	#if __STR_EXPRESSION_TMPLATES__
		template <typename T>
			template <typename T0>
				ConcatExpression<StringExpression<T>,ReferenceExpression<T0> >
					Template<T>::operator+(const T0*string)	const
					{
						return ConcatExpression<StringExpression<T>,ReferenceExpression<T0> >(StringExpression<T>(*this),ReferenceExpression<T0>(string));
					}
			
		template <typename T>
			ConcatExpression<StringExpression<T>,CharacterExpression<T> >
				Template<T>::operator+(T c)	const
				{
					return ConcatExpression<StringExpression<T>,CharacterExpression<T> >(StringExpression<T>(*this),CharacterExpression<T>(c));
				}
			

		template <typename T>
			template <typename T0, typename T1>
				ConcatExpression<StringExpression<T>,ConcatExpression<T0,T1> >
					Template<T>::operator+(const ConcatExpression<T0,T1>&expression)	const
					{
						return ConcatExpression<StringExpression<T>,ConcatExpression<T0,T1> >(StringExpression<T>(*this),expression);
					}
					
		template <typename T>
			template <class T0>
				ConcatExpression<StringExpression<T>,ReferenceExpression<T0> >
					Template<T>::operator+(const ReferenceExpression<T0>&expression)	const
					{
						return ConcatExpression<StringExpression<T>,ReferenceExpression<T0> >(StringExpression<T>(*this),expression);
					}
				
		template <typename T>
			template <class T0>
				ConcatExpression<StringExpression<T>,StringExpression<T0> >
					Template<T>::operator+(const StringExpression<T0>&expression)	const
					{
						return ConcatExpression<StringExpression<T>,StringExpression<T0> >(StringExpression<T>(*this),expression);
					}


		template <typename T0, typename T1>
			ConcatExpression<ReferenceExpression<T0>,StringExpression<T1> > operator+(const T0*c, const Template<T1>& string)
			{
				return ConcatExpression<ReferenceExpression<T0>,StringExpression<T1> >(ReferenceExpression<T0>(c),StringExpression<T1>(string));
			}

		template <typename T>
			ConcatExpression<CharacterExpression<char>,StringExpression<T> > operator+(char c, const Template<T>& string)
			{
				return ConcatExpression<CharacterExpression<char>,StringExpression<T> >(CharacterExpression<char>(c),StringExpression<T>(string));
			}


		template <typename T>
			ConcatExpression<CharacterExpression<wchar_t>,StringExpression<T> > operator+(wchar_t c, const Template<T>& string)
			{
				return ConcatExpression<CharacterExpression<wchar_t>,StringExpression<T> >(CharacterExpression<wchar_t>(c),StringExpression<T>(string));
			}
	#else
		template <typename T>
			Template<T>
				Template<T>::operator+(const T*string)	const
				{
					size_t len = CharFunctions::strlen(string);
					if (!len)
						return *this;
					Template<T> result = Template<T>(TStringLength(Super::elements+(size_t)len));
					memcpy(result.Super::data,Super::data,Super::elements*sizeof(T));
					memcpy(result.Super::data+Super::elements,string,len*sizeof(T));
					return result;
				}
	
		template <typename T>
			template <typename T1>
				Template<T>
					Template<T>::operator+(const T1*string)	const
					{
						size_t len = CharFunctions::strlen(string);
						if (!len)
							return *this;
						Template<T> result = Template<T>(TStringLength(Super::elements+len));
						T*out = result.Super::data;
						for (size_t i = 0; i < Super::elements; i++)
							(*out++) = (T)Super::data[i];
						for (size_t i = 0; i < len; i++)
							(*out++) = (T)string[i];
						return result;
					}
			
		template <typename T>
			Template<T>
				Template<T>::operator+(T c)	const
				{
					Template<T> result = Template<T>(TStringLength(Super::elements+1));
					memcpy(result.Super::data,Super::data,Super::elements*sizeof(T));
					result.Super::data[Super::elements] = c;
					return result;
				}
			

		template <typename T>
			template <typename T0, typename T1>
				Template<T>
					Template<T>::operator+(const ConcatExpression<T0,T1>&expression)	const
					{
						size_t len = expression.GetLength();
						if (!len)
							return *this;
						Template<T> result = Template<T>(TStringLength(Super::elements+len));
						memcpy(result.Super::data,Super::data,Super::elements*sizeof(T));
						expression.WriteTo(result.Super::data+Super::elements);
						return result;
					}
					
		template <typename T>
			template <class T0>
				Template<T>
					Template<T>::operator+(const ReferenceExpression<T0>&expression)	const
					{
						size_t len = expression.GetLength();
						if (!len)
							return *this;
						Template<T> result = Template<T>(TStringLength(Super::elements+len));
						memcpy(result.Super::data,Super::data,Super::elements*sizeof(T));
						expression.WriteTo(result.Super::data+Super::elements);
						return result;
					}
				
		template <typename T>
			template <class T0>
				Template<T>
					Template<T>::operator+(const StringExpression<T0>&expression)	const
					{
						size_t len = expression.GetLength();
						if (!len)
							return *this;
						Template<T> result = Template<T>(TStringLength(Super::elements+len));
						memcpy(result.Super::data,Super::data,Super::elements*sizeof(T));
						expression.WriteTo(result.Super::data+Super::elements);
						return result;
					}
				
		template <typename T>
			template <class T0>
				Template<T>
					Template<T>::operator+(const Template<T0>&expression)	const
					{
						size_t len = expression.GetLength();
						if (!len)
							return *this;
						Template<T> result = Template<T>(TStringLength(Super::elements+len));
						memcpy(result.Super::data,Super::data,Super::elements*sizeof(T));
						expression.WriteTo(result.Super::data+Super::elements);
						return result;
					}


		template <typename T0, typename T1>
			Template<T1> inline operator+(const T0*c, const Template<T1>& string)
			{
				size_t len = CharFunctions::strlen(c);
				if (!len)
					return string;
				Template<T1> result = Template<T1>(TStringLength(string.Super::elements+len));
				T1*out = result.Super::data;
				for (size_t i = 0; i < len; i++)
					(*out++) = c[i];
				for (size_t i = 0; i < string.Super::elements; i++)
					(*out++) = string.Super::data[i];
				return result;
			}
	/*		
		template <>
			Template<char> inline operator+(const char*c, const Template<char>& string)
			{
				size_t len = CharFunctions::strlen(c);
				if (!len)
					return string;
				Template<char> result = Template<char>(TStringLength(string.Super::elements+len));
				memcpy(result.Super::data,c,len);
				memcpy(result.Super::data+len,string.Super::data,string.Super::elements);
				return result;
			}
		
		template <>
			Template<wchar_t> inline operator+(const wchar_t*c, const Template<wchar_t>& string)
			{
				size_t len = CharFunctions::strlen(c);
				if (!len)
					return string;
				Template<wchar_t> result = Template<wchar_t>(TStringLength(string.Super::elements+len));
				memcpy(result.Super::data,c,len*sizeof(wchar_t));
				memcpy(result.Super::data+len,string.Super::data,string.Super::elements*sizeof(wchar_t));
				return result;
			}
	*/
		template <typename T>
			Template<T> inline operator+(char c, const Template<T>& string)
			{
				Template<T> result = Template<T>(TStringLength(string.Super::elements+1));
				memcpy(result.Super::data+1,string.Super::data,string.Super::elements*sizeof(T));
				result.Super::data[0] = (T)c;
				return result;
			}


		template <typename T>
			Template<T> inline operator+(wchar_t c, const Template<T>& string)
			{
				Template<T> result = Template<T>(TStringLength(string.Super::elements+1));
				memcpy(result.Super::data+1,string.Super::data,string.Super::elements*sizeof(T));
				result.Super::data[0] = (T)c;
				return result;
			}







	template <typename T0, typename T1>
		template <typename T2>
			inline ConcatExpression<ConcatExpression<T0,T1>,StringExpression<T2> >
							ConcatExpression<T0,T1>::operator+(const StringExpression<T2>&expression)	const
							{
								return ConcatExpression<ConcatExpression<T0,T1>,StringExpression<T2> >(*this,expression);
							}



	template <typename T>
		template <typename T2>
			inline ConcatExpression<ReferenceExpression<T>,ReferenceExpression<T2> >
							ReferenceExpression<T>::operator+(const ReferenceExpression<T2>&expression)	const
							{
								return ConcatExpression<ReferenceExpression<T>,ReferenceExpression<T2> >(*this,expression);
							}
	template <typename T>
		template <typename T2>
			inline ConcatExpression<ReferenceExpression<T>,StringExpression<T2> >
							ReferenceExpression<T>::operator+(const StringExpression<T2>&expression)	const
							{
								return ConcatExpression<ReferenceExpression<T>,StringExpression<T2> >(*this,expression);
							}
	template <typename T>
		template <typename T2, typename T3>
			inline ConcatExpression<ReferenceExpression<T>,ConcatExpression<T2,T3> >
							ReferenceExpression<T>::operator+(const ConcatExpression<T2,T3>&expression)	const
							{
								return ConcatExpression<ReferenceExpression<T>,ConcatExpression<T2,T3> >(*this,expression);
							}
	template <typename T>
		template <typename T2>
			inline ConcatExpression<ReferenceExpression<T>,CharacterExpression<T2> >
							ReferenceExpression<T>::operator+(const CharacterExpression<T2>&expression)	const
							{
								return ConcatExpression<ReferenceExpression<T>,CharacterExpression<T2> >(*this,expression);
							}		


	template <typename T>
		template <typename T2>
			inline ConcatExpression<StringExpression<T>,ReferenceExpression<T2> >
							StringExpression<T>::operator+(const ReferenceExpression<T2>&expression)	const
							{
								return ConcatExpression<StringExpression<T>,ReferenceExpression<T2> >(*this,expression);
							}
	template <typename T>
		template <typename T2>
			inline ConcatExpression<StringExpression<T>,CharacterExpression<T2> >
							StringExpression<T>::operator+(const CharacterExpression<T2>&expression)	const
							{
								return ConcatExpression<StringExpression<T>,CharacterExpression<T2> >(*this,expression);
							}
	template <typename T>
		template <typename T2>
			inline ConcatExpression<StringExpression<T>,StringExpression<T2> >
							StringExpression<T>::operator+(const StringExpression<T2>&expression)	const
							{
								return ConcatExpression<StringExpression<T>,StringExpression<T2> >(*this,expression);
							}
	template <typename T>
		template <typename T2, typename T3>
			inline ConcatExpression<StringExpression<T>,ConcatExpression<T2,T3> >
							StringExpression<T>::operator+(const ConcatExpression<T2,T3>&expression)	const
							{
								return ConcatExpression<StringExpression<T>,ConcatExpression<T2,T3> >(*this,expression);
							}		


	

	template <typename T0, typename T1>
		inline ConcatExpression<ReferenceExpression<T0>,ReferenceExpression<T1> > operator+(const T0*a, const ReferenceExpression<T1>&b)
		{
			return ConcatExpression<ReferenceExpression<T0>,ReferenceExpression<T1> >(a,b);
		}
	
	template <typename T>
		inline ConcatExpression<CharacterExpression<char>,ReferenceExpression<T> > operator+(char a, const ReferenceExpression<T>&b)
		{
			return ConcatExpression<CharacterExpression<char>,ReferenceExpression<T> >(a,b);
		}
	
	template <typename T>
		inline ConcatExpression<CharacterExpression<wchar_t>,ReferenceExpression<T> > operator+(wchar_t a, const ReferenceExpression<T>&b)
		{
			return ConcatExpression<CharacterExpression<wchar_t>,ReferenceExpression<T> >(a,b);
		}
	
	template <typename T0, typename T1>
		inline ConcatExpression<ReferenceExpression<T0>,CharacterExpression<T1> > operator+(const T0*a, const CharacterExpression<T1>&b)
		{
			return ConcatExpression<ReferenceExpression<T0>,CharacterExpression<T1> >(a,b);
		}
	
	template <typename T>
		inline ConcatExpression<CharacterExpression<char>,CharacterExpression<T> > operator+(char a, const CharacterExpression<T>&b)
		{
			return ConcatExpression<CharacterExpression<char>,CharacterExpression<T> >(a,b);
		}
	
	template <typename T>
		inline ConcatExpression<CharacterExpression<wchar_t>,CharacterExpression<T> > operator+(wchar_t a, const CharacterExpression<T>&b)
		{
			return ConcatExpression<CharacterExpression<wchar_t>,CharacterExpression<T> >(a,b);
		}
	

	template <typename T0, typename T1, typename T2>
		inline ConcatExpression<ReferenceExpression<T0>,ConcatExpression<T1, T2> > operator+(const T0*a, const ConcatExpression<T1, T2>&b)
		{
			return ConcatExpression<ReferenceExpression<T0>,ConcatExpression<T1, T2> >(a,b);
		}
	
	template <typename T0, typename T1>
		inline ConcatExpression<CharacterExpression<char>,ConcatExpression<T0, T1> > operator+(char a, const ConcatExpression<T0, T1>&b)
		{
			return ConcatExpression<CharacterExpression<char>,ConcatExpression<T0, T1> >(a,b);
		}
	
	template <typename T0, typename T1>
		inline ConcatExpression<CharacterExpression<wchar_t>,ConcatExpression<T0, T1> > operator+(wchar_t a, const ConcatExpression<T0, T1>&b)
		{
			return ConcatExpression<CharacterExpression<wchar_t>,ConcatExpression<T0, T1> >(a,b);
		}
	
	template <typename T0, typename T1>
		inline ConcatExpression<ReferenceExpression<T0>,StringExpression<T1> > operator+(const T0*a, const StringExpression<T1>&b)
		{
			return ConcatExpression<ReferenceExpression<T0>,StringExpression<T1> >(a,b);
		}
	
	template <typename T>
		inline ConcatExpression<CharacterExpression<char>,StringExpression<T> > operator+(char a, const StringExpression<T>&b)
		{
			return ConcatExpression<CharacterExpression<char>,StringExpression<T> >(a,b);
		}
	
	template <typename T>
		inline ConcatExpression<CharacterExpression<wchar_t>,StringExpression<T> > operator+(wchar_t a, const StringExpression<T>&b)
		{
			return ConcatExpression<CharacterExpression<wchar_t>,StringExpression<T> >(a,b);
		}


	template <typename E0, typename E1>
		template <typename T2>
			bool			ConcatExpression<E0, E1>::operator<(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())<0;
			}
		
	template <typename E0, typename E1>
		template <typename T2>
			bool			ConcatExpression<E0, E1>::operator<=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())<=0;
			}
		
	template <typename E0, typename E1>
		template <typename T2>
			bool			ConcatExpression<E0, E1>::operator>(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())>0;
			}
		
	template <typename E0, typename E1>
		template <typename T2>
			bool			ConcatExpression<E0, E1>::operator>=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())>=0;
			}
		
	template <typename E0, typename E1>
		template <typename T2>
			bool			ConcatExpression<E0, E1>::operator!=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())!=0;
			}
	
		
	template <typename E0, typename E1>
		template <typename T2>
			bool			ConcatExpression<E0, E1>::operator==(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())==0;
			}
/*
	template <typename T>
		template <typename T2>
			bool			ReferenceExpression<T>::operator<(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())<0;
			}
		
	template <typename T>
		template <typename T2>
			bool			ReferenceExpression<T>::operator<=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())<=0;
			}
		
	template <typename T>
		template <typename T2>
			bool			ReferenceExpression<T>::operator>(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())>0;
			}
		
	template <typename T>
		template <typename T2>
			bool			ReferenceExpression<T>::operator>=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())>=0;
			}
		
	template <typename T>
		template <typename T2>
			bool			ReferenceExpression<T>::operator!=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())!=0;
			}
	
		
	template <typename T>
		template <typename T2>
			bool			ReferenceExpression<T>::operator==(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())==0;
			}

	
	template <typename T>
		template <typename T2>
			bool			CharacterExpression<T>::operator<(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())<0;
			}
		
	template <typename T>
		template <typename T2>
			bool			CharacterExpression<T>::operator<=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())<=0;
			}
		
	template <typename T>
		template <typename T2>
			bool			CharacterExpression<T>::operator>(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())>0;
			}
		
	template <typename T>
		template <typename T2>
			bool			CharacterExpression<T>::operator>=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())>=0;
			}
		
	template <typename T>
		template <typename T2>
			bool			CharacterExpression<T>::operator!=(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())!=0;
			}
	
		
	template <typename T>
		template <typename T2>
			bool			CharacterExpression<T>::operator==(const StringType::Template<T2>&other)	const
			{
				return CompareTo(other.c_str(),other.GetLength())==0;
			}*/
}
	

#define DEFINE_COMPARE_OPERATORS(_OPERATOR_,_REVERSE_OPERATOR_)\
	template <typename T0, typename T1>\
		inline bool operator _OPERATOR_(const T0*a, const StringType::Template<T1>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_(a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(char a, const StringType::Template<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(wchar_t a, const StringType::Template<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T0, typename T1>\
		inline bool operator _OPERATOR_(const T0*a, const StringType::ReferenceExpression<T1>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_(a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(char a, const StringType::ReferenceExpression<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(wchar_t a, const StringType::ReferenceExpression<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T0, typename T1>\
		inline bool operator _OPERATOR_(const T0*a, const StringType::CharacterExpression<T1>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(char a, const StringType::CharacterExpression<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_(a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(wchar_t a, const StringType::CharacterExpression<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_(a));\
		}\
	template <typename T0, typename T1>\
		inline bool operator _OPERATOR_(const T0*a, const StringType::StringExpression<T1>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(char a, const StringType::StringExpression<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T>\
		inline bool operator _OPERATOR_(wchar_t a, const StringType::StringExpression<T>& b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T0, typename T1, typename T2>\
		inline bool operator _OPERATOR_(const T0*a, const StringType::ConcatExpression<T1, T2>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T0, typename T1>\
		inline bool operator _OPERATOR_(char a, const StringType::ConcatExpression<T0, T1>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}\
	template <typename T0, typename T1>\
		inline bool operator _OPERATOR_(wchar_t a, const StringType::ConcatExpression<T0, T1>&b)\
		{\
			return !(b.operator _REVERSE_OPERATOR_ (a));\
		}
	


DEFINE_COMPARE_OPERATORS(<,>=)
DEFINE_COMPARE_OPERATORS(>,<=)
DEFINE_COMPARE_OPERATORS(<=,>)
DEFINE_COMPARE_OPERATORS(>=,<)
DEFINE_COMPARE_OPERATORS(==,!=)
DEFINE_COMPARE_OPERATORS(!=,==)

	

#endif
