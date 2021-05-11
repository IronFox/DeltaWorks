#ifndef global_rootH
#define global_rootH

/******************************************************************

Global super-include. This file is included by all local .h-files.

******************************************************************/

#include "general/undef.h"
#include <stdlib.h>

#pragma warning (error: 4715)	//functions with a declared return value must return something
#pragma warning (error: 4716)	//variance of 4715

#undef DEBUG_LEVEL

#define DEBUG_LEVEL 0

/** debug-level **

0: no checking
1: new/delete-checking
2: list-checking

*/


#ifdef _MSC_VER
	#pragma warning(disable:4355 4305 4244 4101 4996)

	#include <stdio.h>
	static inline FILE*	_fopen(const char*filename, const char*open_mode)
	{
		FILE* result;
		if (fopen_s(&result,filename,open_mode) != 0)
			return NULL;
		return result;
	}

	#define fopen(FILENAME,OPENMODE)	_fopen(FILENAME,OPENMODE)

#endif


#ifdef __BORLANDC__
	#include <vcl.h>
	#pragma hdrstop
	#include <windef.h>
	#include <algorith.h>
#else
	#ifdef _WIN32
		#undef _WIN32_WINNT //override
		#define _WIN32_WINNT 0x1000
		//#define WIN32_LEAN_AND_MEAN	removes some headers we would like to have
		#include <WinSock2.h>
		#include <windows.h>
	#else
		#include <string.h>
	#endif
	#include <stdio.h>
	#undef ARRAYSIZE
	#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))


#endif






/* SYSTEMs and SYSTEM_TYPEs  */

#undef LINUX

#define WINDOWS 0
#define UNIX    1
#define LINUX   2
#define MACOS   3
#define OTHER   4
#define UNKNOWN 5


/*SYSTEM specifies the general system (eg. UNIX, LINUX)*/

#if defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(_Windows)
	#define SYSTEM              WINDOWS
	#define SYSTEM_NAME         "Windows"
#elif defined(__linux) || defined(linux) || defined(unix) || defined(__unix__) || defined(__unix) || defined(__FreeBSD__)
	#define SYSTEM              UNIX
	#if defined(__linux) || defined(linux)
		#define SYSTEM			LINUX
		#define SYSTEM_NAME     "Linux"
	#elif defined(__FreeBSD__)
		#define SYSTEM			UNIX
		#define SYSTEM_NAME		"FreeBSD"
	#elif defined(__SUNPRO_C) || defined(__sun) || defined(sun)
		#define SYSTEM			UNIX
		#define SYSTEM_NAME     "Solaris"
	#else
		#define SYSTEM			UNIX
		#define SYSTEM_NAME     "Unix"
	#endif
#elif defined(__MACOS__) || defined (__MacOS__)
	#define SYSTEM				MACOS
	#define SYSTEM_NAME			"MacOS"
#elif defined(__MACOSX__) || defined(__MacOSX__)
	#define SYSTEM              UNIX
	#define SYSTEM_NAME			"MacOSX"
#else
	#define SYSTEM              OTHER
	#define SYSTEM_NAME         "Unspecified System"
#endif

//http://stackoverflow.com/questions/1505582/determining-32-vs-64-bit-in-c
#if _WIN32 || _WIN64
	#if _WIN64
		#define X64
	#else
		#undef X64
	#endif
#endif

// Check GCC
#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define X64
	#else
		#undef X64
	#endif
#endif



namespace DeltaWorks
{

	namespace GlobalTest
	{
		static const bool IsDefined = true;
	}

	namespace GlobalTest2
	{
		static const bool IsGlobal = ::DeltaWorks::GlobalTest::IsDefined;
	}


	/**
		@brief Type that indicates, well, nothing. Constructors that accept this type should behave just like parameter-less constructors
	*/
	struct Nothing
	{
		inline bool operator==(const Nothing&)	const	{return true;}
		inline bool operator!=(const Nothing&)	const	{return false;}
	};







	/**
		@brief True type to use with template evaluations
	 */
	struct TTrue
	{
		static const bool is_true=true;
	};

	/**
		@brief False type to use with template evaluations
	 */
	struct TFalse
	{
		static const bool is_false=true;
	};



	inline bool IsLittleEndian()
	{
		static const int	one = 1;
		return (((unsigned char*)(&one))[0]==1);
	}





	template <class C> inline void  swp(C&e0, C&e1)
										{
											C temp = e0;
											e0 = e1;
											e1 = temp;
										}


	#define UNSIGNED_UNDEF      ((unsigned)-1)
	#define UINT32_UNDEF      	(0xFFFFFFFF)
	#define MAX_TEMPLATE_DEPTH  300



	//#define INLINE inline __attribute__((always_inline))
	#define INLINE  inline

	/*
	These are memory error tracking functions.
	In case ever needed again, they allow to track which items were allocated, or freed.
	Modify in conjunction with the alloc-templates in general/alloc_templates.h
	*/
	template <typename T>
		inline T*		SignalNew(T*item) {return item;}
	template <typename T>
		inline void		Discard(T*item) {delete item;}



	#if SYSTEM!=WINDOWS

		struct RECT
		{
			short   left,
					top,
					right,
					bottom;
		};
    
		struct POINT
		{
			int             x,y;
		};

		#define FOLDER_SLASH        '/'
		#define FOLDER_SLASH_STR    "/"
		#define ENV_PATH_SEPARATOR  ':'
		#define FS_CASE_IGNORE      false
		#define FS_SHARED_LIB_EXT   "so"

	//    #include <general/xportal.h>

	#else
		#ifndef STILL_RUNNING
			#define STILL_RUNNING   0x0103
		#endif
		#define FOLDER_SLASH        L'\\'
		#define FOLDER_SLASH_STR    L"\\"
		#define ENV_PATH_SEPARATOR  L';'
		#define FS_CASE_IGNORE      true
		#define FS_SHARED_LIB_EXT   L"dll"
	
		#define popen				_popen
		#define pclose				_pclose

	
	
	
	#endif

	#define elif    else if







	/* addresses:

	bad:
	0x00000000 (:P)
	0xabababab
	0xbaadf00d


	gcc-error-messages and what they mean:

	* request for member 'x' in 'y' which is of non-class type 'z'
		:exchange '.' with '->' or the other way around.

	* passing `const x' as `this' argument of `y' discards qualifiers
		:make function y non const


	*/

	#define foreach(LIST,ITERATOR)	for (auto ITERATOR = (LIST).begin(); ITERATOR != (LIST).end(); ++ITERATOR)
}
	
#include "general/typedefs.h"
#include "general/alloc_templates.h"
#include "general/root.h"
#include "general/exception.h"
#include "general/enumeration.h"
	
#endif
