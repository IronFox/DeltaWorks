#pragma once

#include "charFunctions.h"

#include "../container/array.h"



namespace DeltaWorks::StringType
{
			/**
	General purpose character markers, used internally by some methods of Template<>.
	They must provide a ()-operator that accepts a character, and returns true it the character is marked.
	Typically, this operator is const, but is not required to be.
	Some applications may fail if the operator is not const.
	*/
	namespace Marker
	{

		template <typename T>
			struct UnEscapeMarker
			{
				bool	escaped = false;
				const T	escapeChar;

				/**/	UnEscapeMarker(T escapeChar = T('\\')):escapeChar(escapeChar) {}
				bool	operator()(T c) throw()
				{
					if (c == escapeChar)
						escaped = !escaped;
					else
						escaped = false;
					return escaped;
				}
			};

		template <typename T>
			struct FieldMarker
			{
				const T	*begin,
						*end;

				/**/	FieldMarker(const T*begin_, const T*end_):begin(begin_),end(end_) {}
				/**/	FieldMarker(const T*begin_, size_t length):begin(begin_),end(begin_+length) {}

				bool	operator()(T c) const throw()
				{
					for (const T*it = begin; it < end; it++)
						if ((*it) == c)
							return true;
					return false;
				}
			};

		template <typename T>
			struct CaseInsensitiveFieldMarker
			{
				const T	*begin,
						*end;

				/**/	CaseInsensitiveFieldMarker(const T*begin_, const T*end_):begin(begin_),end(end_) {}
				/**/	CaseInsensitiveFieldMarker(const T*begin_, size_t length):begin(begin_),end(begin_+length){}

				bool	operator()(T c) const throw()
				{
					c = CharFunctions::tolower(c);
					for (const T*it = begin; it < end; it++)
						if (CharFunctions::tolower(*it) == c)
							return true;
					return false;
				}
			};

		template <typename T>
			struct CharacterMarker
			{
				T		chr;
						CharacterMarker(T chr_):chr(chr_) {}

				bool	operator()(T c) const throw()
				{
					return c == chr;
				}
			};
	}


	template <typename T>
		class Sequence : public ArrayRef<T>
		{
		protected:
			/**/			Sequence() {}
			/**/			Sequence(T*str, size_t length):Super(str,length) {}
			template <class Marker>
				count_t		GenericCountCharacters(const Marker&marked, bool countMatches) const;
		public:
			typedef const T*const_iterator;
			typedef Sequence<T>		Self;
			typedef ArrayRef<T>	Super;

			using Super::GetLength;
			using Super::begin;
			using Super::end;

			/**/			Sequence(T*str):Super(str,CharFunctions::strlen(str)) {}
			/**/			Sequence(const T*str):Super(const_cast<T*>(str),CharFunctions::strlen(str)) {}

			friend hash_t	Hash(const Self&ref)
			{
				return StdCharHash(ref.Super::data,ref.Super::elements);
			}


			/**
			Returns true if the local string ends with the specified string, false otherwise
			*/
			bool			EndsWith(const T*str) const {return EndsWith(Self(str));}
			bool			EndsWith(const ArrayRef<T>&string)	const;		//!< Returns true if the local string ends with the specified string, false otherwise
			bool			EndsWith(T c) const
							{
								if (!Super::elements)
									return false;
								return Super::data[Super::elements-1] == c;
							}
			/**
			Returns true if the local string begins with the specified string, false otherwise
			*/
			bool			BeginsWith(const T*str) const {return BeginsWith(Self(str));}
			bool			BeginsWith(const ArrayRef<T>&string)	const;		//!< Returns true if the local string begins with the specified string, false otherwise
			bool			BeginsWith(T c) const
							{
								if (!Super::elements)
									return false;
								return Super::data[0] == c;
							}
			/**
			Returns true if the local string begins with the specified string, false otherwise
			*/
			bool			BeginsWithIgnoreCase(const T*string)		const	{return BeginsWithIgnoreCase(Self(string));}
			bool			BeginsWithIgnoreCase(const ArrayRef<T>&string)	const;		//!< Returns true if the local string begins with the specified string, false otherwise
			/**
			Returns true if the local string ends with the specified string, false otherwise
			*/
			bool			EndsWithIgnoreCase(const T*string)				const	{return EndsWithIgnoreCase(Self(string));}
			bool			EndsWithIgnoreCase(const ArrayRef<T>&string)		const;		//!< Returns true if the local string ends with the specified string, false otherwise


			/**
			Retrieves the first char of the local string.
			\0 if empty
			*/
			T				GetFirstChar() const
							{
								if (!Super::elements)
									return (T)0;
								return *Super::data;
							}
			T				FirstChar() const	{return GetFirstChar();}
			/**
			Retrieves the last char of the local string.
			\0 if empty
			*/
			T				GetLastChar() const
							{
								if (!Super::elements)
									return (T)0;
								return Super::data[Super::elements-1];
							}
			T				LastChar() const	{return GetLastChar();}

			/**
			Attempts to locate the last occurance of the specified string.
			Search is case sensitive.
			@param needle String segment to look for
			@param offset Index to reference from (first character). Reduced if necessary
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLast(const ArrayRef<T>&needle, index_t offset = InvalidIndex)	const;
			index_t			FindLast(const T*needle, index_t offset = InvalidIndex)		const	/**@copydoc FindLast()*/	{return FindLast(Self(needle),offset);}
			/**
			Attempts to locate the last occurance of the specified character.
			Search is case sensitive.
			@param needle Character to look for
			@param offset Index to reference from. Reduced if necessary
			@return Index of the found character (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLast(T needle, index_t offset = InvalidIndex)					const;
			/**
			Attempts to locate the last occurance of a character for which the specified callback function returns true.
			Search is case sensitive.
			@param callback Pointer to a function to determine whether a character is a valid match. The function should return true if the character is a valid match, false otherwise
			@param offset Index to reference from. Reduced if necessary
			@return Index of the found character (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLast(bool callback(T), index_t offset = InvalidIndex) const;
			/**
			Attempts to locate the first occurance of the specified string.
			Search is case sensitive.
			@param needle String segment to look for
			@param offset Index to reference from 
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			Find(const ArrayRef<T>&needle, index_t offset = 0)	const;
			/**
			Attempts to locate the first occurance of the specified (zero-terminated) string.
			Search is case sensitive.
			@param needle Pointer to the first character of the string segment to look for
			@param offset Index to reference from 
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			Find(const T*needle, index_t offset = 0)		const	{return Find(Self(needle),offset);}
			/**
			Attempts to locate the first occurance of the specified character.
			Search is case sensitive.
			@param needle Character to look for
			@param offset Index to reference from 
			@return Index of the found character (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			Find(T needle, index_t offset = 0)					const;
			/**
			Attempts to locate the first occurance of a character for which the specified callback function returns true.
			Search is case sensitive.
			@param callback Pointer to a function to determine whether a character is a valid match. The function should return true if the character is a valid match, false otherwise
			@param offset Index to reference from 
			@return Index of the found character (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			Find(bool callback(T), index_t offset = 0) const;
			/**
			Searches the local string for the first occurance of the specified sub string with neither the succeeding or preceeding characters being alpha-numeric.
			Search is case sensitive.
			@param needle String to search for
			@param offset Index to reference from 
			@return Starting index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindWord(const T*needle, index_t offset = 0) const	{return FindWord(Self(needle),offset);}
			/**
			@copydoc FindWord()
			*/
			index_t			FindWord(const ArrayRef<T>&needle, index_t offset = 0) const;

			/**
			Attempts to locate the first occurance of the specified string.
			Search ignores case.
			@param needle String segment to look for
			@param offset Index to reference from 
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindIgnoreCase(const ArrayRef<T>&needle, index_t offset = 0)	const;
			/**
			Attempts to locate the first occurance of the specified (zero-terminated) string.
			Search ignores case.
			@param needle Pointer to the first character of the string segment to look for
			@param offset Index to reference from 
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindIgnoreCase(const T*needle, index_t offset = 0)		const	{return FindIgnoreCase(Self(needle),offset);}
			/**
			Attempts to locate the first occurance of the specified character.
			Search ignores case.
			@param needle Character to look for
			@param offset Index to reference from 
			@return Index of the found character (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindIgnoreCase(T needle, index_t offset = 0)					const;

			/**
			Searches the local string for the first occurance of the specified sub string with neither the succeeding or preceeding characters being alpha-numeric.
			Search ignores case.
			@param needle String to search for
			@param offset Index to reference from 
			@return Starting index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindWordIgnoreCase(const T*needle, index_t offset = 0) const	{return FindWordIgnoreCase(Self(needle),offset);}
			/**
			@copydoc FindWordIgnoreCase()
			*/
			index_t			FindWordIgnoreCase(const ArrayRef<T>&needle, index_t offset = 0) const;
			/**
			Searches the local string for the last occurance of the specified sub string with neither the succeeding or preceeding characters being alpha-numeric.
			Search is case sensitive.
			@param needle String to search for
			@param offset Index to reference from (first character). Reduced if necessary
			@return Starting index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLastWord(const T*needle, index_t offset = InvalidIndex) const	{return FindLastWord(Self(needle),offset);}
			/**
			@copydoc FindLastWord()
			*/
			index_t			FindLastWord(const ArrayRef<T>&needle, index_t offset = InvalidIndex) const;
			/**
			Searches the local string for the last occurance of the specified sub string with neither the succeeding or preceeding characters being alpha-numeric.
			Search is case sensitive.
			@param needle String to search for
			@param offset Index to reference from (first character). Reduced if necessary
			@return Starting index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLastWordIgnoreCase(const T*needle, index_t offset = InvalidIndex) const	{return FindLastWordIgnoreCase(Self(needle),offset);}
			/**
			@copydoc FindLastWordIgnoreCase()
			*/
			index_t			FindLastWordIgnoreCase(const ArrayRef<T>&needle, index_t offset = InvalidIndex) const;
			/**
			Attempts to locate the last occurance of the specified string.
			Search is case sensitive.
			@param needle String segment to look for
			@param offset Index to reference from (first character). Reduced if necessary
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLastIgnoreCase(const ArrayRef<T>&needle, index_t offset = InvalidIndex)	const;
			/**
			Attempts to locate the last occurance of the specified (zero-terminated) string.
			Search is case sensitive.
			@param needle Pointer to the first character of the string segment to look for
			@param offset Index to reference from (first character). Reduced if necessary
			@return Start index of the found word (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLastIgnoreCase(const T*needle, index_t offset = InvalidIndex)		const	{return FindLastIgnoreCase(Self(needle),offset);}
			/**
			Attempts to locate the last occurance of the specified character.
			Search is case sensitive.
			@param needle Character to look for
			@param offset Index to reference from. Reduced if necessary
			@return Index of the found character (0=first char) or InvalidIndex if no occurance was found
			*/
			index_t			FindLastIgnoreCase(T needle, index_t offset = InvalidIndex)					const;

			/**
			Checks if the specified token is part of the local string
			*/
			bool			Contains(const ArrayRef<T>&needle)				const	{return Find(needle) != InvalidIndex;}
			bool			Contains(const T*needle)						const	{return Find(needle) != InvalidIndex;}
			bool			Contains(T c)									const	{return Find(c) != InvalidIndex;}
			bool			ContainsWord(const T*word)						const	{return FindWord(word) != InvalidIndex;}
			bool			ContainsWord(const ArrayRef<T>&word)			const	{return FindWord(word) != InvalidIndex;}
			bool			ContainsIgnoreCase(const ArrayRef<T>&needle)	const	{return FindIgnoreCase(needle) != InvalidIndex;}
			bool			ContainsIgnoreCase(const T*needle)				const	{return FindIgnoreCase(needle) != InvalidIndex;}
			bool			ContainsIgnoreCase(T needle)					const	{return FindIgnoreCase(needle) != InvalidIndex;}
			bool			ContainsWordIgnoreCase(const T*word)			const	{return FindWordIgnoreCase(word) != InvalidIndex;}
			bool			ContainsWordIgnoreCase(const ArrayRef<T>&word)	const	{return FindWordIgnoreCase(word) != InvalidIndex;}


			bool			IsValid(bool validCharacter(T character))	const;					//!< Runs each character of the local string by the specified validation function. Returns true if all characters passed the validation, false otherwise.

			/**	
			Const character access method
			@param Index of the character to retrieve in the range [0,Length()-1]
									
			If @b index is not in the range [0,Length()-1] (0=first character), then the result will be 0.
			*/
			T				GetChar(index_t index) const;
			T				at(index_t index) const	{return GetChar(index);}
			T				operator[](index_t index) const {return GetChar(index);}


			count_t			CountCharacters(const ArrayRef<T>& characters, bool count_matches=true)	const;							//!< Counts how often each of the characters in the specified string is contained in the local string;	@param count_matches Set true to count matching characters, false to count non-matching characters
			count_t			CountCharacters(const T* characters, bool count_matches=true)	const;											//!< Counts how often each of the characters in the specified string is contained in the local string;	@param count_matches Set true to count matching characters, false to count non-matching characters
			count_t			CountCharacters(const T* characters, count_t character_count, bool count_matches=true)	const;							//!< Counts how often each of the characters in the specified string is contained in the local string;	@param count_matches Set true to count matching characters, false to count non-matching characters
			count_t			CountCharacters(bool isMatch(T character), bool count_matches=true)	const;							//!< Counts how often characters are contained by the local string for which isMatch(c) returns true (or false if @a count_matches is false).	@param count_matches Set true to count matching characters, false to count non-matching characters
			count_t			CountCharactersIgnoreCase(const ArrayRef<T>& characters, bool count_matches=true)	const;							//!< Counts how often each of the characters in the specified string is contained in the local string; Case insensitive version	@param count_matches Set true to count matching characters, false to count non-matching characters
			count_t			CountCharactersIgnoreCase(const T* characters, bool count_matches=true)	const;											//!< Counts how often each of the characters in the specified string is contained in the local string; Case insensitive version	@param count_matches Set true to count matching characters, false to count non-matching characters
			count_t			CountCharactersIgnoreCase(const T* characters, count_t character_count, bool count_matches=true)	const;							//!< Counts how often each of the characters in the specified string is contained in the local string; Case insensitive version	@param count_matches Set true to count matching characters, false to count non-matching characters

			Self			SubStringRefS(sindex_t index, count_t count = (count_t)-1)	 const;
			Self			SubStringRef(index_t index, count_t count = (count_t)-1) const	{return SubStringRefS((sindex_t)index,count);}


			inline void		Print(std::ostream&stream)			const;	//!< Prints the local expression content to the specified stream @param stream Stream to print to
			#ifdef WCOUT
				inline void	Print(std::wostream&stream)			const;	//!< Prints the local expression content to the specified wide character stream @param stream Stream to print to
			#endif
			template <class Stream>
				inline void	PrintArchitecture(Stream&stream)			const
							{
								stream << "sequence<\"";
								Print(stream);
								stream << "\", "<<GetLength()<<">";
							}
	
			inline T*		WriteTo(T*target)	const				//! Writes the local expression content to a character array. The array must be sufficiently large since the operation does not perform any length checks. @param target Character array to write to @return Ctr::Array position once the operation is completed. The returned pointer points one character past the last written character
							{
								memcpy(target,Super::data,Super::elements*sizeof(T));
								return target+Super::elements;
							}
						
			inline T*		WriteTo(T*target, T*end)	const		//! Writes the local expression content to a character array of constrained size. @param target Character array to write to @param end Pointer to one past the last writable character. @return Ctr::Array position once the operation is completed. The returned pointer points one character past the last written character. The returned pointer will not point further than @a end
							{
								size_t cpy = end-target;
								if (Super::elements < cpy)
									cpy = Super::elements;
								memcpy(target,Super::data,cpy*sizeof(T));
								return target+cpy;
							}
					

			inline int		CompareSegment(const T*string, size_t string_length)	const	//! Nested compare method
							{
								int result = CharFunctions::strncmp(Super::data,string,string_length<Super::elements?string_length:Super::elements);
								if (result)
									return result;
								if (string_length < Super::elements)
									return 1;
								return 0;
							}
			inline int		CompareTo(const T*string, size_t string_length)	const	//! Expression comparison. Allows to orthographically compare the local string to the specified character segment @param string String to compare to @param string_length Number of characters that the specified string is in length @return -1 if the local expression is orthographically smaller than the specified string, 0 if it is equal, +1 if it is orthographically larger
							{
								int result = CompareSegment(string,string_length);
								if (!result && string_length > Super::elements)
									result = -1;
								return result;
							}
			inline int		CompareTo(const ArrayRef<T>&string)	const	//! Orthographic comparison
							{
								return CompareTo(string.GetPointer(),string.Length());
							}
			inline int		CompareSegmentIgnoreCase(const T*string, size_t string_length)	const	//! Nested compare method (case ignore)
							{
								int result = CharFunctions::strncmpi(Super::data,string,string_length<Super::elements?string_length:Super::elements);
								if (result)
									return result;
								if (string_length < Super::elements)
									return 1;
								return 0;
							}
			inline int		CompareToIgnoreCase(const T*string, size_t string_length)	const	//! Expression comparison. Allows to orthographically compare the local string to the specified character segment @param string String to compare to @param string_length Number of characters that the specified string is in length @return -1 if the local expression is orthographically smaller than the specified string, 0 if it is equal, +1 if it is orthographically larger
							{
								int result = CompareSegmentIgnoreCase(string,string_length);
								if (!result && string_length > Super::elements)
									result = -1;
								return result;
							}
			inline int		CompareToIgnoreCase(const T*string)	const	//! Expression comparison. Allows to orthographically compare the local string to the specified character segment @param string String to compare to @param string_length Number of characters that the specified string is in length @return -1 if the local expression is orthographically smaller than the specified string, 0 if it is equal, +1 if it is orthographically larger
							{
								size_t slen = CharFunctions::strlen(string);
								int result = CompareSegmentIgnoreCase(string,slen);
								if (!result && slen > Super::elements)
									result = -1;
								return result;
							}
			inline bool		References(const T*field)	const	//! Checks whether this expression references the specified field @param field Pointer to the field to look for @return true if the pointer was found, false otherwise
							{
								return field == Super::data;
							}
			bool			operator==(const ArrayRef<T>&other)	const	{return Super::elements == other.Length() && CharFunctions::strncmp(Super::data,other.pointer(),Super::elements) == 0;}
			bool			operator!=(const ArrayRef<T>&other)	const	{return !operator==(other);}
			bool			operator==(const T*str)	const	{size_t l = CharFunctions::strlen(str); return l == Super::elements && CharFunctions::strncmp(str,Super::data,Super::elements) == 0;}
			bool			operator!=(const T*str)	const	{return !operator==(str);}
			bool			operator==(const T&ch)	const	{return 1 == Super::elements && Super::data[0] == ch;}
			bool			operator!=(const T&ch)	const	{return !operator==(ch);}

			/**
			Tests for case insensitive equality to another String object.
			@return true if the local and the remote string content are identical, false otherwise.
			*/
			bool			EqualsIgnoreCase(const ArrayRef<T>&other)		const	{return Super::elements == other.Length() && CharFunctions::strncmpi(Super::data,other.pointer(),Super::elements) == 0;}
			bool			EqualsIgnoreCase(const T*str)			const	/**@copydoc EqualsIgnoreCase()*/ {return EqualsIgnoreCase(Self(str));}
			/**
			Tests for case insensitive equality to a character.
			@return true if the local string content matches the specified character, false otherwise.
			*/
			bool			EqualsIgnoreCase(T chr)					const
			{
				return Super::elements == 1 && CharFunctions::tolower(chr) == CharFunctions::tolower(Super::data[0]);
			}

		};



}

namespace DeltaWorks
{
	typedef StringType::Sequence<char>	CharSequence;

}

#include "charSequence.tpl.h"
