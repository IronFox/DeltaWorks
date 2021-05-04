#include "containers.h"
#include <string/encoding.h>

namespace DeltaWorks
{



	namespace Package
	{
		template class NetString<256>;

		bool			IsValidChar(char32_t c)
		{
			return !StringFunctions::IsInvalidChar(c) && c != '\t';
		}

		bool			IsValid(const StringRef&str, const count_t maxLength)
		{
			if (str.GetLength() > maxLength)
				return false;
			StringEncoding::UTF8::Scanner scanner(str);
			while (scanner.HasNext())
				if (!IsValidChar(scanner.Next()))
					return false;
			return true;
		}
		bool			IsValid(const UTF32StringRef&str, const count_t maxLength)
		{
			if (str.GetLength() > maxLength)
				return false;
			for (index_t i = 0; i < str.GetLength(); i++)
				if (!IsValidChar(str.GetChar(i)))
					return false;
			return true;
		}


		void			AssertValidity(String&str, const count_t maxLength)
		{
			#ifndef _DEBUG
				str.Truncate(maxLength);

				for (index_t i = 0; i < str.GetLength(); i++)
				{
					const char c = str[i];
					if (!IsValidChar(c))
						str.Set(i,'?');
				}
			#else
				try
				{
					CheckValidity(str,maxLength);
				}
				catch (const std::exception&ex)
				{
					Except::TriggerFatal(CLOCATION,ex.what());
				}
				catch (...)
				{
					Except::TriggerFatal(CLOCATION,"Unknown exception");
				}
			#endif
		}

		void			CheckValidity(const StringRef&str, const count_t maxLength)
		{
			if (str.GetLength() > maxLength)
				throw Except::Program::ParameterFault(CLOCATION,String("NetString: '"+str+"' (length "+String(str.GetLength())+") exceeds maximum allowed length "+String(maxLength)));

			for (index_t i = 0; i < str.GetLength(); i++)
			{
				const char c = str[i];
				if (!IsValidChar(c))
					throw Except::Program::ParameterFault(CLOCATION,"NetString: Character '"+String(c)+"' at position "+String(i)+" of '"+str+"' is not allowed");
			}
		}
	}
}
