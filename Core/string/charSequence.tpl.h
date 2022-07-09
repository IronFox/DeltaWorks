#pragma once


namespace DeltaWorks::StringType
{

	
	template <typename T>
		index_t			Sequence<T>::Find(const ArrayRef<T>&needle, index_t offset /*=0*/)	const
		{
			if (needle.IsEmpty() || offset+needle.GetLength() > Super::elements)
				return InvalidIndex;
			const T*ptr = CharFunctions::strstr(begin()+offset,end(),needle.begin(),needle.end());
			if (ptr != nullptr)
				return ptr - Super::data;
			return InvalidIndex;
		}

	template <typename T>
		index_t			Sequence<T>::FindIgnoreCase(const ArrayRef<T>&needle, index_t offset /*=0*/)	const
		{
			if (needle.IsEmpty() || offset+needle.GetLength() > Super::elements)
				return InvalidIndex;
			const T*ptr = CharFunctions::stristr(begin()+offset,end(),needle.begin(),needle.end());
			if (ptr != nullptr)
				return ptr - Super::data;
			return InvalidIndex;
		}

	template <typename T>
		index_t			Sequence<T>::FindIgnoreCase(T c, index_t offset /*=0*/)					const
		{
			if (offset >= Super::elements)
				return InvalidIndex;
			const T*pntr = CharFunctions::strichr(Super::data+offset,c);
			return pntr?(pntr-Super::data):InvalidIndex;
		}




	template <typename T>
		index_t			Sequence<T>::FindWord(const ArrayRef<T>&needle, index_t offset /*=0*/) const
		{
			if (needle.IsEmpty() || offset + needle.GetLength() > Super::elements)
				return InvalidIndex;
	//			const T*offset = Super::data + offset_;
			const size_t nlen = needle.GetLength();
			while (true)
			{
				index_t next = Find(needle,offset);
				if (next == InvalidIndex)
					return InvalidIndex;
				if (!CharFunctions::isalnum(Super::data[next+nlen]))
					return next;
				offset = next+nlen;
				while (offset < Super::elements && CharFunctions::isalnum(Super::data[offset]))
					offset++;
				if (offset == Super::elements)
					return InvalidIndex;
			}
		}
	

	template <typename T>
		index_t			Sequence<T>::FindWordIgnoreCase(const ArrayRef<T>&needle, index_t offset /*=0*/) const
		{
			if (needle.IsEmpty() || offset + needle.GetLength() > Super::elements)
				return InvalidIndex;
	//			const T*offset = Super::data + offset_;
			const size_t nlen = needle.GetLength();
			while (true)
			{
				index_t next = FindIgnoreCase(needle,offset);
				if (next == InvalidIndex)
					return InvalidIndex;
				if (!CharFunctions::isalnum(Super::data[next+nlen]))
					return next;
				offset = next+nlen;
				while (offset < Super::elements && CharFunctions::isalnum(Super::data[offset]))
					offset++;
				if (offset == Super::elements)
					return InvalidIndex;
			}
		}
	



	template <typename T>
		index_t			Sequence<T>::Find(T c, index_t offset /*=0*/)					const
		{
			if (offset >= Super::elements)
				return InvalidIndex;
			const T*pntr = CharFunctions::strchr(Super::data+offset, Super::data+Super::elements,c);
			return pntr?(pntr-Super::data):InvalidIndex;
		}


	
	template <typename T>
		index_t			Sequence<T>::Find(bool callback(T), index_t offset /*=0*/)					const
		{
			const T *end = Super::data+Super::elements,
					*haystack = Super::data+offset;
			while (haystack < end && !callback(*haystack))
				haystack++;
			return haystack < end?haystack-Super::data:InvalidIndex;
		}






	template <typename T>
		index_t			Sequence<T>::FindLastWord(const ArrayRef<T>&needle, index_t offset_ /*=InvalidIndex*/ ) const
		{
			if (needle.GetLength() == 0 || needle.GetLength() > Super::elements)
				return InvalidIndex;
			const size_t nlen = needle.GetLength();
			const T*offset = Super::data+ std::min( Super::elements-nlen, offset_);
			while (offset >= Super::data)
			{
				while (offset > Super::data && (CharFunctions::isalnum(offset[-1]) || CharFunctions::isalnum(offset[nlen])))
					offset--;
				if (CharFunctions::isalnum(offset[nlen]))
					return InvalidIndex;
				if (!CharFunctions::strncmp(offset,needle.pointer(), nlen))
					return offset-Super::data;
				offset--;
			}
			return InvalidIndex;
		}




	template <typename T>
		index_t			Sequence<T>::FindLastWordIgnoreCase(const ArrayRef<T>&needle, index_t offset_ /*=InvalidIndex*/ ) const
		{
			if (needle.GetLength() == 0 || needle.GetLength() > Super::elements)
				return InvalidIndex;
			const size_t nlen = needle.GetLength();
			const T*offset = Super::data+ std::min( Super::elements-nlen, offset_);
			while (offset >= Super::data)
			{
				while (offset > Super::data && (CharFunctions::isalnum(offset[-1]) || CharFunctions::isalnum(offset[nlen])))
					offset--;
				if (CharFunctions::isalnum(offset[nlen]))
					return InvalidIndex;
				if (!CharFunctions::strncmpi(offset,needle.pointer(), nlen))
					return offset-Super::data;
				offset--;
			}
			return InvalidIndex;
		}

	
	
	template <typename T>
		index_t			Sequence<T>::FindLastIgnoreCase(const ArrayRef<T>&needle, index_t offset /*=InvalidIndex*/ )	const
		{
			if (Super::elements < needle.GetLength() || needle.IsEmpty())
				return InvalidIndex;
			const T	
					*haystack = Super::data+std::min(Super::elements-needle.GetLength(), offset),
					*end = haystack + needle.GetLength(),
					*needle_end = needle.end();
			while (haystack >= Super::data)
			{
				const T	*p0 = haystack;
				const T	*p1 = needle.begin();
				while (p0 < end && p1 < needle_end && CharFunctions::tolower(*p0) == CharFunctions::tolower(*p1))
				{
					p0++;
					p1++;
				}
				if (p1==needle_end)
					return haystack-Super::data;
				haystack--;
			}
			return InvalidIndex;
		}
	

	

	template <typename T>
		index_t			Sequence<T>::FindLast(const ArrayRef<T>&needle, index_t offset /*=InvalidIndex*/ )	const
		{
			if (Super::elements < needle.GetLength() || needle.IsEmpty())
				return InvalidIndex;
			const T	
					*haystack = Super::data+std::min(Super::elements-needle.GetLength(), offset),
					*end = haystack + needle.GetLength(),
					*needle_end = needle.end();
			while (haystack >= Super::data)
			{
				const T	*p0 = haystack;
				const T	*p1 = needle.begin();
				while (p0 < end && p1 < needle_end && *p0 == *p1)
				{
					p0++;
					p1++;
				}
				if (p1==needle_end)
					return haystack-Super::data;
				haystack--;
			}
			return InvalidIndex;
		}


	template <typename T>
		index_t			Sequence<T>::FindLast(T c, index_t offset /*=InvalidIndex*/ )					const
		{
			const T 
					*haystack = Super::data+std::min(Super::elements-1, offset),
					*end = haystack + 1;
			while (haystack >= Super::data && *haystack != c)
				haystack--;
			return haystack >= Super::data?haystack-Super::data:InvalidIndex;
		}




	template <typename T>
		index_t			Sequence<T>::FindLastIgnoreCase(T c, index_t offset /*=InvalidIndex*/ )					const
		{
			c = CharFunctions::tolower(c);
			const T 
					*haystack = Super::data+std::min(Super::elements-1, offset),
					*end = haystack + 1;
			while (haystack >= Super::data && CharFunctions::tolower(*haystack) != c)
				haystack--;
			return haystack >= Super::data ? haystack-Super::data : InvalidIndex;
		}



	template <typename T>
		index_t			Sequence<T>::FindLast(bool callback(T), index_t offset /*=InvalidIndex*/)					const
		{
			const T 
					*haystack = Super::data+std::min(Super::elements-1, offset),
					*end = haystack + 1;
			while (haystack >= Super::data && !callback(*haystack))
				haystack--;
			return haystack >= Super::data ? haystack-Super::data : InvalidIndex;
		}


	template <typename T>
		bool				Sequence<T>::EndsWith(const ArrayRef<T>&string)		const
		{
			if (string.GetLength() > Super::elements)
				return false;
			return !CharFunctions::strncmp(Super::end()-string.GetLength(),string.GetPointer(),string.GetLength());
		}

	template <typename T>
		bool				Sequence<T>::EndsWithIgnoreCase(const ArrayRef<T>&string)		const
		{
			if (string.GetLength() > Super::elements)
				return false;
			return !CharFunctions::strncmpi(Super::end()-string.GetLength(),string.GetPointer(),string.GetLength());
		}

	template <typename T>
		bool				Sequence<T>::BeginsWithIgnoreCase(const ArrayRef<T>&string)		const
		{
			if (string.GetLength() > Super::elements)
				return false;
			return !CharFunctions::strncmpi(Super::data,string.GetPointer(),string.GetLength());
		}

	template <typename T>
		bool				Sequence<T>::BeginsWith(const ArrayRef<T>&string)		const
		{
			if (string.GetLength() > Super::elements)
				return false;
			return !CharFunctions::strncmp(Super::data,string.GetPointer(),string.GetLength());
		}

	template <typename T>
		bool			Sequence<T>::IsValid(bool validCharacter(T character))	const
		{
			for (index_t i = 0; i < Super::elements; i++)
				if (!validCharacter(Super::data[i]))
					return false;
			return true;
		}

	template <typename T>
		T				Sequence<T>::GetChar(size_t index) const
		{
			if (index >= Super::elements)
				return (T)0;
			return Super::data[index];
		}


	template <typename T>
		template <class Marker>
			count_t					Sequence<T>::GenericCountCharacters(const Marker&marked, bool countMatches)const
			{
				index_t count = 0;
				const T*read = Super::data;
				const T*const end = Super::end();
				while (read != end)
					count += (marked(*read++) == countMatches);
				return count;
			}

	template <typename T>
		count_t					Sequence<T>::CountCharacters(const ArrayRef<T>& characters, bool countMatches)	const
		{
			if (characters.IsEmpty())
				return 0;

			return GenericCountCharacters(Marker::FieldMarker<T>(characters.GetPointer(),characters.Count()),countMatches);
		}


	template <typename T>
		count_t					Sequence<T>::CountCharacters(const T* characters, bool countMatches)	const
		{
			if (!characters)
				return 0;
			size_t len = CharFunctions::strlen(characters);
			if (!len)
				return 0;
			return GenericCountCharacters(Marker::FieldMarker<T>(characters,len),countMatches);
		}

	template <typename T>
		count_t					Sequence<T>::CountCharacters(const T* characters, count_t characterCount, bool countMatches)	const
		{
			if (!characters || !characterCount)
				return 0;
			return GenericCountCharacters(Marker::FieldMarker<T>(characters,characterCount),countMatches);
		}

	template <typename T>
		count_t					Sequence<T>::CountCharacters(bool isMatch(T character), bool countMatches)	const
		{
			return GenericCountCharacters(isMatch,countMatches);
		}

	template <typename T>
		count_t					Sequence<T>::CountCharactersIgnoreCase(const ArrayRef<T>& characters, bool countMatches)	const
		{
			if (characters.IsEmpty())
				return 0;
			return GenericCountCharacters(Marker::CaseInsensitiveFieldMarker<T>(characters.GetPointer(),characters.GetLength()), countMatches);
		}

	template <typename T>
		count_t					Sequence<T>::CountCharactersIgnoreCase(const T* characters, bool countMatches)	const
		{
			if (!characters)
				return 0;
			size_t len = CharFunctions::strlen(characters);
			if (!len)
				return 0;

			return GenericCountCharacters(Marker::CaseInsensitiveFieldMarker<T>(characters,len),countMatches);
		}
	template <typename T>
		count_t					Sequence<T>::CountCharactersIgnoreCase(const T* characters, count_t characterCount, bool countMatches)	const
		{
			if (!characters || !characterCount)
				return 0;
			return GenericCountCharacters(Marker::CaseInsensitiveFieldMarker<T>(characters,characterCount),countMatches);
		}



	template <typename T>
		Sequence<T>				Sequence<T>::SubStringRefS(sindex_t index, count_t count /*= (count_t)-1*/)	 const
		{
			if (index<0)
			{
				count += index;
				index=0;
			}
			if ((size_t)index >= Super::elements)
				return Sequence<T>(Super::data,0);
			if (count > Super::elements)
				count = Super::elements;
			if (index+count>Super::elements)
				count = Super::elements-index;
			return Sequence<T>(Super::data+index,count);
		}


	template <>
		inline void		Sequence<char>::Print(std::ostream&stream)	const
		{
			stream.write(Super::data,Super::elements);
		}
						
	template <typename T>
		inline void		Sequence<T>::Print(std::ostream&stream)	const
		{
			for (size_t i = 0; i < Super::elements; i++)
				stream << (char)Super::data[i];
		}

	#ifdef WCOUT

		template <>
			inline	void		Sequence<wchar_t>::Print(std::wostream&stream)	const
								{
									stream.write(Super::data,Super::elements);
								}

							
		template <typename T>
			inline	void		Sequence<T>::Print(std::wostream&stream)	const
								{
									for (size_t i = 0; i < Super::elements; i++)
										stream << (wchar_t)Super::data[i];
								}

	#endif
}
