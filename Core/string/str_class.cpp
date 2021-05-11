#include "../global_root.h"
#include "str_class.h"
#include "../io/file_system.h"
#include <algorithm>





void gotoxy(int x, int y)
{
	#if SYSTEM==UNIX
		char essq[100];		// String variable to hold the escape sequence
		char xstr[100];		// Strings to hold the x and y coordinates
		char ystr[100];		// Escape sequences must be built with characters
	 
		/*
		** Convert the screen coordinates to strings
		*/
		sprintf(xstr, "%d", x);
		sprintf(ystr, "%d", y);
	 
		/*
		** Build the escape sequence (vertical move)
		*/
		essq[0] = '\0';
		strcat(essq, "\033[");
		strcat(essq, ystr);
	 
		/*
		** Described in man terminfo as vpa=\E[%p1%dd
		** Vertical position absolute
		*/
		strcat(essq, "d");
	 
		/*
		** Horizontal move
		** Horizontal position absolute
		*/
		strcat(essq, "\033[");
		strcat(essq, xstr);
		// Described in man terminfo as hpa=\E[%p1%dG
		strcat(essq, "G");
	 
		/*
		** Execute the escape sequence
		** This will move the cursor to x, y
		*/
		printf("%s", essq);
	#elif SYSTEM==WINDOWS
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE) ; 
		COORD position = { (SHORT)x, (SHORT)y } ; 
     
		SetConsoleCursorPosition( hStdout, position ) ;
	#endif
 
}


#if SYSTEM==UNIX
	#include <iconv.h>

	/*String	ToUTF8(const StringW&str)
	{
		iconv_t	iv = iconv_open("WCHAR_T", "UTF-8");
		size_t maxLen = str.GetLength() * 4;
		Ctr::Array<char>	field(maxLen);
		char*source = const_cast<char*>((const char*)str.c_str());
		char*dest = field.pointer();
		size_t sourceBytes = (str.GetLength()) * sizeof(wchar_t);
		size_t destBytes = maxLen;
		size_t resultLength = iconv(iv, &source, &sourceBytes, &dest, &destBytes);
		if (resultLength == InvalidIndex)
			return "";
		return String(field.pointer(),maxLen - destBytes);
	}*/


#endif


char*	strnchr(char*string,size_t cnt, char c)
{
	char*end = string+cnt;
	while (string != end)
	{
		if (*string == c)
			return string;
		string++;
	}
	return NULL;
}

char*	strnstr(char *big, const char *little, size_t len)
{
	size_t len2 = strlen(little);
	if (len2 > len)
		return NULL;
	char*end = big+len-len2;
	while (big != end)
		if (!strncmp(big,little,len2))
			return big;
		else
			big++;
	return NULL;
}



namespace DeltaWorks
{

	namespace StringType
	{

		template class Template<char>;
		template class Template<wchar_t>;
		template class ReferenceExpression<char>;
		template class ReferenceExpression<wchar_t>;
	
		static  char		zc(0);

		unsigned	GLOBAL_STRING_ID_COUNTER(0);
		void*	   GLOBAL_TRACED_ORIGINAL_ADDR;

	}

	#define ErrBox(msg) ErrMessage(msg)


	#if SYSTEM!=WINDOWS
		static const int MB_OK = 0;
	#endif
		





	static unsigned	genericLengthFunction(const char*begin, const char*end)
	{
		return end-begin;
	}






	#if SYSTEM==WINDOWS


	#else

		void strlwr(char*str)
		{
			while (*str)
			{
				*str = tolower(*str);
				str++;
			}
		}
		void strupr(char*str)
		{
			while (*str)
			{
				*str = toupper(*str);
				str++;
			}
		}

		void wcslwr(wchar_t*str)
		{
			while (*str)
			{
				*str = towlower(*str);
				str++;
			}
		}
		void wcsupr(wchar_t*str)
		{
			while (*str)
			{
				*str = towupper(*str);
				str++;
			}
		}


		//void strlwr(char*target)
		//{
		//	if (!target) return;
		//	while (*target)
		//	{
		//		if (*target >= 'A' && *target <= 'Z')
		//			(*target) += 'a' - 'A';
		//		target++;
		//	}
		//}
		//void strupr(char*target)
		//{
		//	if (!target) return;
		//	while (*target)
		//	{
		//		if (*target >= 'a' && *target <= 'z')
		//			(*target) -= 'a' - 'A';
		//		target++;
		//	}
		//}
	#endif


	namespace StringType
	{


		char*	strAllocate(unsigned size)
		{
			if (size <= 1)
				return &zc;
			#if (STRING_ALLOCATION_MODE == 0)
				return (char*)malloc(size);
			#elif (STRING_ALLOCATION_MODE == 1)
				return SHIELDED_ARRAY(new char[size],size);
			#else
				return sys::allocate(size);
			#endif
		}

		char*	strRelocate(char*current, unsigned current_size, unsigned new_size)
		{
	
			char*next = strAllocate(new_size);
			if (new_size>current_size)
				memcpy(next,current,current_size);
			else
				memcpy(next,current,new_size);
			strFree(current);
			return next;
		}


		void	 strFree(char*current)
		{
			if (current == &zc || !current)
				return;
			#if (STRING_ALLOCATION_MODE == 0)
				free(current);
			#elif (STRING_ALLOCATION_MODE == 1)
				DISCARD_ARRAY(current);
			#else
				sys::delocate(current);
			#endif
		}
	}

	void ShowMessage(const char*line)
	{
		DisplayMessage("message",line);
	}


	static void _msgBox(const char*body, const char*head, UINT type)
	{
		#if SYSTEM == WINDOWS
			MessageBoxA (NULL,body,head,type);
		#else
			std::cout << "["<<head<<"]:"<<body<<std::endl;
		#endif
	}

	static void _msgBox(const wchar_t*body, const wchar_t*head, UINT type)
	{
		#if SYSTEM == WINDOWS
			MessageBoxW (NULL,body,head,type);
		#else
			std::cout << "["<<head<<"]:"<<body<<std::endl;
		#endif
	}

	template <typename T>
		const T*	_empty()
		{
			return NULL;
		}
	template <>
		const char*	_empty<char>()
		{
			return "<empty string>";
		}
	template <>
		const wchar_t*	_empty<wchar_t>()
		{
			return L"<empty string>";
		}

	
	#if SYSTEM==WINDOWS
		void	GetProgramFileName(wchar_t*out)
		{
			GetModuleFileNameW( NULL, out, MAX_PATH );
		}
	#elif SYSTEM==LINUX
		extern char * program_invocation_name;

		void	GetProgramFileName(char*out)
		{
	//		char real[PATH_MAX] = {0};
			realpath(program_invocation_name, out);
		}
	#elif SYSTEM==UNIX
		#include <stdlib.h>
		void	GetProgramFileName(char*out)
		{
	//		char real[PATH_MAX] = {0};
			realpath(getprogname(), out);
		}
	#else
		#error find some solution
	#endif
	

	template <typename T>
		StringType::Template<T>	_getApplicationName()
		{
			PathString::char_t szFileName[MAX_PATH];

			GetProgramFileName(  szFileName);

			return StringType::Template<T>(FileSystem::ExtractFileName(PathString(szFileName))+StringType::Cast(": "));
		}


	template <typename T0, typename T1>
		static void _displayMessage(const T0*head_, const StringType::Sequence<T1>&line, bool prefixProgramName)
		{
			Ctr::Array<StringType::Template<T1> >	lines;
			Explode((T1)'\n',line,lines);
			StringType::Template<T0> head = prefixProgramName ? _getApplicationName<T0>() + head_ : head_;
			if (!lines.Count())
				_msgBox(_empty<T0>(),head.c_str(),MB_OK);
			StringType::Template<T1> sum;
			head += (T0)' ';
			count_t rounds = lines.Count() / 40;
			if (lines.Count() % 40)
				rounds ++;
			for (index_t i = 0; i < lines.Count(); i+= 40)
			{
				sum = implode((T1)'\n',lines + i,std::min<count_t>(40,lines.Count() - i));
				if (lines.Count() > 40)
				{
					_msgBox(sum.c_str(),(head+ StringType::Template<T0>(i/40 +1)+(T0)'/'+ StringType::Template<T0>(rounds)).c_str(),MB_OK);
				}
				else
					_msgBox(sum.c_str(),head.c_str(),MB_OK);
			}
		}

	void			DisplayMessage(const char*head, const char*line, bool prefixProgramName/*=true*/)
	{
		#if SYSTEM==WINDOWS
			_displayMessage(head, StringRef(line),prefixProgramName);
			//MessageBoxA (NULL,line,head?head:"message",MB_OK);
		#elif SYSTEM==UNIX
			if (head)
				std::cout << head << ":"<<std::endl<<"  ";
			std::cout << line<<std::endl;
		#endif
	}

	void			DisplayMessage(const char*head, const String&line, bool prefixProgramName/*=true*/)
	{
		#if SYSTEM==WINDOWS
			_displayMessage(head,line.ToRef(),prefixProgramName);
			//MessageBoxA (NULL,line.c_str(),head?head:"message",MB_OK);
		#elif SYSTEM==UNIX
			if (head)
				std::cout << head << ":"<<std::endl<<"  ";
			std::cout << line<<std::endl;
		#endif

	}

	void			DisplayMessage(const String&head, const String&line, bool prefixProgramName/*=true*/)
	{
		DisplayMessage(head.c_str(),line,prefixProgramName);
	}

	void ShowMessage(const String&line)
	{
		DisplayMessage("message",line);
	}


	void ErrMessage(const char*line)
	{
		DisplayMessage("error",line);
	}

	void ErrMessage(const String&line)
	{
		DisplayMessage("error",line);
	}

	void			DisplayMessageW(const wchar_t*head, const wchar_t*line, bool prefixProgramName/*=true*/)
	{
		#if SYSTEM==WINDOWS
			_displayMessage(head,StringRefW(line),prefixProgramName);
		#elif SYSTEM==UNIX
			if (head)
				std::wcout << head << ":"<<std::endl<<"  ";
			std::wcout << line<<std::endl;
		#endif
	}

	void			DisplayMessageW(const wchar_t*head, const StringW&line, bool prefixProgramName/*=true*/)
	{
		#if SYSTEM==WINDOWS
			_displayMessage(head,line.ToRef(),prefixProgramName);
		#elif SYSTEM==UNIX
			if (head)
				std::wcout << head << ":"<<std::endl<<"  ";
			std::wcout << line<<std::endl;
		#endif

	}

	void			DisplayMessageW(const StringW&head, const StringW&line, bool prefixProgramName/*=true*/)
	{
		DisplayMessageW(head.c_str(),line,prefixProgramName);
	}

	void ShowMessageW(const StringW&line)
	{
		DisplayMessageW(L"message",line);
	}

	void ShowMessageW(const wchar_t*line)
	{
		DisplayMessageW(L"message",line);
	}


	void ErrMessageW(const wchar_t*line)
	{
		DisplayMessageW(L"error",line);
	}

	void ErrMessageW(const StringW&line)
	{
		DisplayMessageW(L"error",line);
	}





	static char* pointerToHex(const void*pointer, int min_len, char*end, char*first)
	{
		size_t value = (size_t)pointer;
		while (value && end != first)
		{
			BYTE c = value & (0xF);
			if (c < 10)
				(*--end) = '0'+c;
			else
				(*--end) = 'A'+c-10;
			value>>=4;
			min_len--;
		}
		while (min_len-->0 && end!=first)
			(*--end) = '0';
		return end;
	}

	static char* intToHex(unsigned value, int min_len, char*end, char*first)
	{
		while (value && end != first)
		{
			BYTE c = value & (0xF);
			if (c < 10)
				(*--end) = '0'+c;
			else
				(*--end) = 'A'+c-10;
			value>>=4;
			min_len--;
		}
		while (min_len-->0 && end!=first)
			(*--end) = '0';
		return end;
	}

	static char* intToHex(UINT64 value, int min_len, char*end, char*first)
	{
		while (value && end != first)
		{
			BYTE c = value & (0xF);
			if (c < 10)
				(*--end) = '0'+c;
			else
				(*--end) = 'A'+c-10;
			value>>=4;
			min_len--;
		}
		while (min_len-->0 && end!=first)
			(*--end) = '0';
		return end;
	}


	template <class Type, class UType> 
		static char* signedToStr(Type value, char*end, char*first)
		{
			bool negative = false;
			if (value < 0)
			{
				value*=-1;
				negative = true;
			}

			char *c = end;
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

	//
	//template <class Type>
	//	static char* floatToStr(Type value, unsigned char exactness, char*end, char*first)
	//	{
	//		/*if (value > INT_MAX||value < INT_MIN)
	//		{
	//			end-=8;
	//			strcpy(end,"exceeded");
	//			return end;
	//		}*/
	//
	//		if (isinf(value))
	//		{
	//			end -= 4;
	//			if (value > 0)
	//				strcpy(end,"+INF");
	//			else
	//				strcpy(end,"-INF");
	//			return end;
	//		}
	//		
	//		if (isnan(value))
	//		{
	//			end-=3;
	//			strcpy(end,"NAN");
	//			return end;
	//		}
	//		bool negative = value < 0;
	//		if (negative)
	//			value*=-1;
	//		
	//		char*c = end;
	//		(*c) = 0;
	//		if (value > INT_MAX)
	//		{
	//			int exponent = (int)log10(value);
	//			//cout << "exponent of "<<value<<" is "<<exponent << endl;
	//			value /= pow(10.0,exponent);
	//			c = signedToStr<int,unsigned>(exponent,c,first);
	//			if (c != first)
	//				(*(--c)) = 'e';
	//		}
	//			
	//			
	//		if (pow(10.0f,exactness)*value > INT_MAX)
	//			exactness = (BYTE)log10(INT_MAX/value);
	//		UINT64 v = (UINT64)(value*pow(10.0f,exactness));
	//		unsigned at(0);
	//		bool Write = !exactness;
	//		while (v && c != first)
	//		{
	//			char ch = '0'+(v%10);
	//			Write = Write || ch != '0' || at==exactness;
	//			if (Write)
	//				(*--c) = ch;
	//			v/=10;
	//			at++;
	//			if (at == exactness && c != first && Write)
	//				(*--c) = DecimalSeparator;
	//		}
	//		if (c != end)
	//		{
	//			while (at < exactness && c != first)
	//			{
	//				(*--c) = '0';
	//				at++;
	//			}
	//			if (at == exactness && c != first)
	//			{
	//				if ((*c) != DecimalSeparator)
	//					(*--c) = DecimalSeparator;
	//				if (c != first)
	//					(*--c) = '0';
	//			}
	//			if (negative && c != first)
	//				(*--c) = '-';
	//		}
	//		else
	//			(*--c) = '0';
	//	//	len = end-c;
	//		return c;
	//	}

	char*			writeToCharField(unsigned value, char*offset, char*end)
	{
		char	*c = offset;
		while (value && c != end)
		{
			(*(c++)) = '0'+(value%10);
			value/=10;
		}
		if (c==offset)
			(*(c++)) = '0';
	
		for (char*first = offset, *last=c; first < last; first++, last--)
			swp(*first,*last);
		return c;
	}

	char*			writeFloatToCharField(float value, char*offset, char*end)
	{
		char	*c = offset;


		if (value > INT_MAX||value < INT_MIN)
		{
			if (end-c>=8)
			{
				strncpy(c,"exceeded",8);
				return c+8;
			}
			return c;
		}
		if (isnan(value))
		{
			if (end-c>=3)
			{
				strncpy(c,"NAN",3);
				return c+3;
			}
			return c;
		}
		bool negative = value < 0;
		if (negative)
			value*=-1;
		float exactness = 5;
		if (pow(10.0f,exactness)*value > INT_MAX)
			exactness = (BYTE)log10(INT_MAX/value);
		UINT64 v = (UINT64)(value*pow(10.0f,exactness));
		unsigned at(0);
		bool write = !exactness;
		while (v && c != end)
		{
			char ch = '0'+(v%10);
			write = write || ch != '0' || at==exactness;
			if (write)
				(*c++) = ch;
			v/=10;
			at++;
			if (at == exactness && c != end && write)
				(*c++) = globalDecimalSeparator;
		}
		if (c != offset)
		{
			while (at < exactness && c != end)
			{
				(*c++) = '0';
				at++;
			}
			if (at == exactness && c != end)
			{
				if ((*c) != globalDecimalSeparator)
					(*c++) = globalDecimalSeparator;
				if (c != end)
					(*c++) = '0';
			}
			if (negative && c != end)
				(*c++) = '-';
		}
	
		for (char*first = offset, *last=c; first < last; first++, last--)
			swp(*first,*last);
		return c;
	}

	String PointerToHex(const void*pointer)
	{
		char buffer[257];
		buffer[sizeof(buffer)-1] = 0;
		return pointerToHex(pointer,sizeof(pointer)*2,buffer+sizeof(buffer)-1,buffer);
	}

	String			DataToBinary(const void*data, size_t byteSize)
	{
		String result(TStringLength(byteSize*8));
		const BYTE*const raw = (const BYTE*)data;
		for (index_t i = 0; i < byteSize; i++)
		{
			const BYTE val = raw[i];
			for (int k = 0; k < 8; k++)
			{
				BYTE bit = (val >> (7-k)) & 1;
				result.Set(i*8+k,bit ? '1': '0');
			}
		}
		return result;
	}


	static char		hexChar(BYTE value)
	{
		if (value < 10)
			return '0'+value;
		return 'A'+(value-10);
	}

	void		binaryToHex(const void*data, size_t data_size, String&target)
	{
		target.Resize((unsigned)data_size<<1);
		const BYTE*pntr = (const BYTE*)data;
		for (unsigned i = 0; i < data_size; i++)
		{
			target.Set((i<<1),hexChar(((pntr[i]>>4)&0xF)));
			target.Set((i<<1)+1, hexChar((pntr[i])&0xF));
		}
	}

	String		binaryToHex(const void*data, size_t data_size)
	{
		String rs;
		binaryToHex(data,data_size,rs);
		return rs;
	}



	#ifndef __BORLANDC__
	String IntToHex(int value, BYTE max_len)
	{
		char buffer[257];
		buffer[sizeof(buffer)-1] = 0;
		return intToHex((unsigned)value,max_len,buffer+sizeof(buffer)-1,buffer);
	}

	String IntToHex(__int64 value, BYTE max_len)
	{
		char buffer[257];
		buffer[sizeof(buffer)-1] = 0;
		return intToHex((UINT64)value,max_len,buffer+sizeof(buffer)-1,buffer);
	}

	String IntToStr(int value)
	{
		return String(value);
	}

	String FloatToStr(float value)
	{
		return String(value);
	}
	#else
	String intToHex(int value, BYTE max_len)
	{
		char buffer[257];
		buffer[sizeof(buffer)-1] = 0;
		return intToHex((unsigned)value,max_len,buffer+sizeof(buffer)-1,buffer);
	}

	String intToHex(__int64 value, BYTE max_len)
	{
		char buffer[257];
		buffer[sizeof(buffer)-1] = 0;
		return intToHex((UINT64)value,max_len,buffer+sizeof(buffer)-1,buffer);
	}

	String intToStr(int value)
	{
		return String(value);
	}

	String FloatToStr(float value)
	{
		return value;
	}


	#endif



	String operator "" _s(const char*v, size_t len)
	{
		return String(v,len);
	}

	StringW operator "" _s(const wchar_t*v, size_t len)
	{
		return StringW(v,len);
	}


	UTF32String operator "" _s(const char32_t*v, size_t len)
	{
		return UTF32String(v,len);
	}

	UTF16String operator "" _s(const char16_t*v, size_t len)
	{
		return UTF16String(v,len);
	}


}
