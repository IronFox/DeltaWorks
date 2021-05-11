#pragma once

#include "str_class.h"

namespace DeltaWorks
{
	namespace LineWrap
	{
		namespace UTF32
		{
			void 	Wrap(const ArrayRef<char32_t>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<char32_t> >&result);
			template <typename Width>
				void	Wrap(const ArrayRef<char32_t>&string, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<char32_t> >&result);

			//these are really bad if allowed, because the string becomes invalid the moment the function returns. thus they are deleted:
			void 	Wrap(StringType::Template<char32_t>&&, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<char32_t> >&result)=delete;
			template <typename Width>
				void	Wrap(StringType::Template<char32_t>&&, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<char32_t> >&result)=delete;
		}

		namespace UTF16
		{
			template <typename Char>
				void 	Wrap(const ArrayRef<Char>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<Char> >&result);
			template <typename Char, typename Width>
				void	Wrap(const ArrayRef<Char>&string, const Width&lineWidth, Width (*charWidthFunction)(Char),ArrayData<StringType::ReferenceExpression<Char> >&result);

			//these are really bad if allowed, because the string becomes invalid the moment the function returns. thus they are deleted:
			template <typename Char>
				void 	Wrap(StringType::Template<Char>&&, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<Char> >&result)=delete;
			template <typename Char, typename Width>
				void	Wrap(StringType::Template<Char>&&, const Width&lineWidth, Width (*charWidthFunction)(Char),ArrayData<StringType::ReferenceExpression<Char> >&result)=delete;
		}

		namespace UTF8
		{
			void 	Wrap(const ArrayRef<char>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<char> >&result);
			template <typename Width>
				void	Wrap(const ArrayRef<char>&string, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<char> >&result);

			//these are really bad if allowed, because the string becomes invalid the moment the function returns. thus they are deleted:
			void 	Wrap(String&&, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<char> >&result)=delete;
			template <typename Width>
				void	Wrap(String&&, const Width&lineWidth, Width (*charWidthFunction)(char32_t),ArrayData<StringType::ReferenceExpression<char> >&result)=delete;
		}

	}


}

#include "lineWrap.tpl.h"
