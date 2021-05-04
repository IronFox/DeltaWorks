#pragma once

#include "../global_root.h"

#ifdef _MSC_VER
	#define strcmpi _strcmpi
	#define strnicmp _strnicmp
	#define wcscmpi _wcsicmp
#endif


namespace DeltaWorks
{

	template <typename T>
		inline bool IsWhitespace(T c)
		{
			return c == (T)' ' || c == (T)'\t' || c == (T)'\r' || c == (T)'\n';
		}

	template <typename T>
		inline bool IsNewline(T c)
		{
			return c == (T)'\r' || c == (T)'\n';
		}



	namespace CharFunctions
	{

		template <typename T>
			inline size_t	__fastcall strlen(const T*string) throw();
		template <typename T0, typename T1>
			inline const T0*	__fastcall strstr(const T0*haystack, const T1*needle) throw();
		template <typename T0, typename T1>
			inline const T0*	__fastcall strstr(const T0*haystack, const T0*const haystackEnd, const T1*needle, const T1*const needleEnd) throw();
		template <typename T0, typename T1>
			inline const T0*	__fastcall stristr(const T0*haystack, const T0*const haystackEnd, const T1*needle, const T1*const needleEnd) throw();
		template <typename T0, typename T1>
			inline const T0*	__fastcall stristr(const T0*haystack, const T1*needle) throw();
		template <typename T>
			inline const T*	__fastcall strchr(const T*haystack, T needle) throw();
		template <typename T>
			inline const T*	__fastcall strchr(const T*haystack, const T*const haystackEnd, T needle) throw();
		template <typename T>
			inline const T*	__fastcall strchr(const T*haystack, bool isNeedle(T)) throw();
		template <typename T>
			inline const T*	__fastcall strchr(const T*haystack, const T*const haystackEnd, bool isNeedle(T)) throw();
		template <typename T>
			inline const T*	__fastcall strichr(const T*haystack, T needle) throw();
		template <typename T>
			inline const T*	__fastcall strichr(const T*haystack, const T*haystackEnd, T needle) throw();
		template <typename T0, typename T1>
			inline void __fastcall strncpy(T0*target,const T1*source,size_t length) throw();
		template <typename T0, typename T1>
			inline int __fastcall strncmp(const T0*string0,const T1*string1, size_t length) throw();
		template <typename T0, typename T1>
			inline int __fastcall strncmpi(const T0*string0,const T1*string1, size_t length) throw();
		template <typename T0, typename T1>
			inline int __fastcall strcmp(const T0*string0,const T1*string1) throw();
		template <typename T0, typename T1>
			inline int __fastcall strcmpi(const T0*string0,const T1*string1) throw();
		template <typename T>
			inline void __fastcall strupr(T*string) throw();
		template <typename T>
			inline void __fastcall strlwr(T*string) throw();
		template <typename T>
			inline T	__fastcall toupper(T chr) throw();
		template <typename T>
			inline T	__fastcall tolower(T chr) throw();
		template <typename T>
			inline bool	__fastcall isalnum(T chr) throw();
		template <typename T>
			inline bool	__fastcall isdigit(T chr) throw();
		template <typename T>
			inline bool	__fastcall isalpha(T chr) throw();
		template <typename T>
			inline bool __fastcall isEscapable(T chr) throw();	//!< Returns true for characters that can be escaped
		template <typename T0, typename T1>
			inline void	__fastcall Cast(const T0*from, T1*to, count_t numChars);

		/**
		Checks whether the given character is a control character that should not be present in a regular string.
		The tab and newline (\n) characters are permitted although they are technically control characters.
		Carriage return (\r) is considered invalid, although sometimes due to OS compatibility, it may be appropriate
		*/
		template <typename T>
			inline bool __fastcall IsInvalidChar(T chr) throw();
	}
	namespace StringFunctions = CharFunctions;
	#include "charFunctions.tpl.h"

}

