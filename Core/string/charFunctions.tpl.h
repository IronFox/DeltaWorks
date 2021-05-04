#pragma once


namespace CharFunctions
{
	template <>
		inline size_t	__fastcall strlen<char>(const char*string) throw()
		{
			return ::strlen(string);
		}

	template <>
		inline size_t	__fastcall strlen<wchar_t>(const wchar_t*string) throw()
		{
			return ::wcslen(string);
		}

	template <typename T>
		inline size_t	__fastcall strlen(const T*string) throw()
		{
			size_t rs = 0;
			while (*string++)
				rs++;
			return rs;
		}
		
	template <>
		inline const char*	__fastcall strstr<char,char>(const char*haystack, const char*needle) throw()
		{
			return ::strstr(haystack,needle);
		}

	template <>
		inline const wchar_t*	__fastcall strstr<wchar_t,wchar_t>(const wchar_t*haystack, const wchar_t*needle) throw()
		{
			return ::wcsstr(haystack,needle);
		}
		
	template <typename T0, typename T1>
		inline const T0*	__fastcall strstr(const T0*haystack, const T1*needle) throw()
		{
			while (*haystack)
			{
				const T0	*p0 = haystack;
				const T1	*p1 = needle;
				while (*p0 && *p0 == (T0)*p1)
				{
					p0++;
					p1++;
				}
				if (!*p1)
					return haystack;
				haystack++;
			}
			return NULL;
		}

	template <typename T0, typename T1>
		inline const T0*	__fastcall strstr(const T0*haystack, const T0*const haystackEnd, const T1*needle, const T1*const needleEnd) throw()
		{
			while (haystack != haystackEnd)
			{
				const T0	*p0 = haystack;
				const T1	*p1 = needle;
				while (p0 != haystackEnd && p1 != needleEnd && *p0 == (T0)*p1)
				{
					p0++;
					p1++;
				}
				if (p1 == needleEnd)
					return haystack;
				haystack++;
			}
			return NULL;
		}

	template <typename T0, typename T1>
		inline const T0*	__fastcall stristr(const T0*haystack, const T0*const haystackEnd, const T1*needle, const T1*const needleEnd) throw()
		{
			while (haystack != haystackEnd)
			{
				const T0	*p0 = haystack;
				const T1	*p1 = needle;
				while (p0 != haystackEnd && p1 != needleEnd && tolower(*p0) == (T0)tolower(*p1))
				{
					p0++;
					p1++;
				}
				if (p1 == needleEnd)
					return haystack;
				haystack++;
			}
			return NULL;
		}
		
		
	template <typename T0, typename T1>
		inline const T0*	__fastcall stristr(const T0*haystack, const T1*needle) throw()
		{
			while (*haystack)
			{
				const T0	*p0 = haystack;
				const T1	*p1 = needle;
				while (*p0 && tolower(*p0) == (T0)tolower(*p1))
				{
					p0++;
					p1++;
				}
				if (!*p1)
					return haystack;
				haystack++;
			}
			return NULL;
		}

	template <>
		inline const char*	__fastcall strchr(const char*haystack, char needle) throw()
		{
			return ::strchr(haystack,needle);
		}
	
	template <>
		inline const wchar_t*	__fastcall strchr(const wchar_t*haystack, wchar_t needle) throw()
		{
			return ::wcschr(haystack,needle);
		}

	template <typename T>
		inline const T*	__fastcall strichr(const T*haystack, T needle) throw()
		{
			needle = tolower(needle);
			while (*haystack)
			{
				if (tolower(*haystack) == needle)
					return haystack;
				haystack++;
			}
			return NULL;
		}

	template <typename T>
		inline const T*	__fastcall strichr(const T*haystack, const T*haystackEnd, T needle) throw()
		{
			needle = tolower(needle);
			while (haystack != haystackEnd)
			{
				if (tolower(*haystack) == needle)
					return haystack;
				haystack++;
			}
			return NULL;
		}
		
	template <typename T>
		inline const T*	__fastcall strchr(const T*haystack, T needle) throw()
		{
			while (*haystack)
			{
				if (*haystack == needle)
					return haystack;
				haystack++;
			}
			return NULL;
		}

	template <typename T>
		inline const T*	__fastcall strchr(const T*haystack, const T*const haystackEnd, T needle) throw()
		{
			while (haystack != haystackEnd)
			{
				if (*haystack == needle)
					return haystack;
				haystack++;
			}
			return NULL;
		}
	
	template <typename T>
		inline const T*	__fastcall strchr(const T*haystack, bool isNeedle(T)) throw()
		{
			while (*haystack)
			{
				if (isNeedle(*haystack))
					return haystack;
				haystack++;
			}
			return NULL;
		}
	
	template <typename T>
		inline const T*	__fastcall strchr(const T*haystack, const T*const haystackEnd, bool isNeedle(T)) throw()
		{
			while (haystack != haystackEnd)
			{
				if (isNeedle(*haystack))
					return haystack;
				haystack++;
			}
			return NULL;
		}
	
	template <>
		inline void __fastcall strncpy<char,char>(char*target,const char*source,size_t length) throw()
		{
			::strncpy(target,source,length);
		}

	template <>
		inline void __fastcall strncpy<wchar_t,wchar_t>(wchar_t*target,const wchar_t*source,size_t length) throw()
		{
			::wcsncpy(target,source,length);
		}

	template <typename T0, typename T1>
		inline void __fastcall strncpy(T0*target,const T1*source,size_t length) throw()
		{
			for (size_t i = 0; i < length; i++)
				(*target++) = (T0)(*source++);
		}
		
	template <>
		inline int __fastcall strcmp<char,char>(const char*string0,const char*string1) throw()
		{
			return ::strcmp(string0,string1);
		}
		
	template <>
		inline int __fastcall strcmp<wchar_t,wchar_t>(const wchar_t*string0,const wchar_t*string1) throw()
		{
			return ::wcscmp(string0,string1);
		}
		
	template <typename T0, typename T1>
		inline int __fastcall strcmp(const T0*string0,const T1*string1) throw()
		{
			while (*string0 && *string1)
			{
				if (*string0 < *string1)
					return -1;
				if (*string0 > *string1)
					return 1;
				string0++;
				string1++;
			}
			if (*string0 < *string1)
				return -1;
			if (*string0 > *string1)
				return 1;
			return 0;
		}
		
	template <>
		inline int __fastcall strncmp<char,char>(const char*string0,const char*string1, size_t length) throw()
		{
			return ::strncmp(string0,string1,length);
		}
		
	template <>
		inline int __fastcall strncmp<wchar_t,wchar_t>(const wchar_t*string0,const wchar_t*string1, size_t length) throw()
		{
			return ::wcsncmp(string0,string1,length);
		}
		
	template <typename T0, typename T1>
		inline int __fastcall strncmp(const T0*string0,const T1*string1, size_t length) throw()
		{
			for (size_t i = 0; i < length; i++)
			{
				if (*string0 < *string1)
					return -1;
				if (*string0 > *string1)
					return 1;
				string0++;
				string1++;
			}
			return 0;
		}
	
	template <typename T>
		inline T	__fastcall chrlwr(T c)
		{
			if (c >= (T)'A' && c <= (T)'Z')
				return c + (T)'a' - (T)'A';
			return c;
		}
	
	template <>
		inline int __fastcall strncmpi<char,char>(const char*string0,const char*string1, size_t length) throw()
		{
			if (length == 0)
				return 0;
			return ::strnicmp(string0,string1,length);
		}
		
	template <>
		inline int __fastcall strncmpi<wchar_t,wchar_t>(const wchar_t*string0,const wchar_t*string1, size_t length) throw()
		{
			if (length == 0)
				return 0;
			return ::wcsnicmp(string0,string1,length);
		}


	template <typename T0, typename T1>
		inline int __fastcall strncmpi(const T0*string0,const T1*string1, size_t length) throw()
		{
			for (size_t i = 0; i < length; i++)
			{
				T0	c0 = chrlwr(*string0);
				T1	c1 = chrlwr(*string1);
				if (c0 < c1)
					return -1;
				if (c0 > c1)
					return 1;
				string0++;
				string1++;
			}
			return 0;
		}
				
		
	template <>
		inline int __fastcall strcmpi<char,char>(const char*string0,const char*string1) throw()
		{
			return ::strcmpi(string0,string1);
		}
		
	template <>
		inline int __fastcall strcmpi<wchar_t,wchar_t>(const wchar_t*string0,const wchar_t*string1) throw()
		{
			return ::wcscmpi(string0,string1);
		}


	
	template <typename T0, typename T1>
		inline int __fastcall strcmpi(const T0*string0,const T1*string1) throw()
		{
			while (*string0 && *string1)
			{
				T0	c0 = chrlwr(*string0);
				T1	c1 = chrlwr(*string1);
				if (c0 < c1)
					return -1;
				if (c0 > c1)
					return 1;
				string0++;
				string1++;
			}
			T0	c0 = chrlwr(*string0);
			T1	c1 = chrlwr(*string1);
			if (c0 < c1)
				return -1;
			if (c0 > c1)
				return 1;
			return 0;			
		}	

	template <typename T>
		inline void __fastcall strupr(T*string) throw()
		{
			while (*string != 0)
			{
				*string = toupper(*string);
				string++;
			}
		}

	template <>
		inline void __fastcall strupr<char>(char*string) throw()
		{
			::strupr(string);
		}
	template <>
		inline void __fastcall strupr<wchar_t>(wchar_t*string) throw()
		{
			::wcsupr(string);
		}

	template <typename T>
		inline void __fastcall strlwr(T*string) throw()
		{
			while (*string != 0)
			{
				*string = tolower(*string);
				string++;
			}
		}

	template <>
		inline void __fastcall strlwr<char>(char*string) throw()
		{
			::strlwr(string);
		}

	template <>
		inline void __fastcall strlwr<wchar_t>(wchar_t*string) throw()
		{
			::wcslwr(string);
		}

	template <typename T>
		inline T	__fastcall toupper(T chr) throw()
		{
			std::locale::global(std::locale(""));  // (*)
			auto& f = std::use_facet<std::ctype<T>>(std::locale());
			f.toupper(&chr, &chr + 1);
			return chr;

			/*
			if (chr >= T('a') && chr <= T('z'))
				return chr + (T('A') - T('a'));
			//if (chr >= T('à') && chr <= T('ö'))
			//	return chr + (T('À') - T('à'));
			return chr;*/
		}

	template <>
		inline char	__fastcall toupper<char>(char chr) throw()
		{
			return ::toupper(chr);
		}

	template <>
		inline wchar_t	__fastcall toupper<wchar_t>(wchar_t chr) throw()
		{
			return ::towupper(chr);
		}


	template <typename T>
		inline T	__fastcall tolower(T chr) throw()
		{
			std::locale::global(std::locale(""));  // (*)
			auto& f = std::use_facet<std::ctype<T>>(std::locale());
			f.tolower(&chr, &chr + 1);
			return chr;
			/*
			if (chr >= T('A') && chr <= T('Z'))
				return chr + (T('a') - T('A'));

			//if (chr >= T('À') && chr <= T('Ö'))
			//	return chr + (T('à') - T('À'));
			return chr;
			*/
		}

	template <>
		inline char	__fastcall tolower<char>(char chr) throw()
		{
			return ::tolower(chr);
		}

	template <>
		inline wchar_t	__fastcall tolower<wchar_t>(wchar_t chr) throw()
		{
			return ::towlower(chr);
		}	

	template <typename T>
		inline bool __fastcall isalpha(T chr) throw()
		{
			return ::iswalpha((wchar_t)chr) != 0;
			//return (chr >= T('a') && chr <= T('z'))
			//	||
			//	(chr >= T('A') && chr <= T('Z'))
			//	||
			//	(chr >= T('À') && chr <= T('ÿ') && chr != T('÷') && chr != T('×'));
		}

	template <>
		inline bool __fastcall isalpha(char chr) throw()
		{
			return ::isalpha((BYTE)chr)!=0;
		}

	template <>
		inline bool __fastcall isalpha(wchar_t chr) throw()
		{
			return ::iswalpha(chr)!=0;
		}


	template <typename T>
		inline bool __fastcall isdigit(T chr) throw()
		{
			return (chr >= T('0') && chr <= T('9'));
		}

	template <>
		inline bool __fastcall isdigit(char chr) throw()
		{
			return ::isdigit((BYTE)chr)!=0;
		}

	template <>
		inline bool __fastcall isdigit(wchar_t chr) throw()
		{
			return ::iswdigit(chr)!=0;
		}
		
	template <typename T>
		inline bool	__fastcall isalnum(T chr) throw()
		{
			return std::isalpha(chr) || std::isdigit(chr);
		}

	template <>
		inline bool	__fastcall isalnum<char>(char chr) throw()
		{
			return ::isalnum(chr)!=0;
		}

	template <>
		inline bool	__fastcall isalnum<wchar_t>(wchar_t chr) throw()
		{
			return ::iswalnum(chr)!=0;
		}

	template <typename T>
		inline bool __fastcall isEscapable(T chr) throw()
		{
			return chr == T('\\') || chr == T('\'') || chr == T('\"');
		}


	template <typename T0, typename T1>
		inline void	__fastcall Cast(const T0*from, T1*to, count_t numChars)
		{
			for (index_t i = 0; i < numChars; i++)
				to[i] =  (T1)(index_t)from[i];
		}

	template <>
		inline void	__fastcall Cast<char,wchar_t>(const char*from, wchar_t*to, count_t numChars)
		{
			for (index_t i = 0; i < numChars; i++)
				to[i] =  (unsigned char)from[i];
		}

	template <typename T>
		inline bool __fastcall IsInvalidChar(T chr) throw()
		{
			return (chr >= 0 && chr < 32 && chr != 9 && chr != 10) || (chr >= 127 && chr <= 159);
		}

	template <>
		inline bool __fastcall IsInvalidChar<char>(char chr) throw()
		{
			return (chr >= 0 && chr < 32 && chr != 9 && chr != 10);
		}

}
