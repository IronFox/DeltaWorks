#include "lineWrap.h"


namespace DeltaWorks::LineWrap
{
	namespace UTF8
	{



		void 	Wrap(const ArrayRef<char>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<char> >&result)
		{
			Details::Wrap(string,result, MakeWrapper(lineCharCount,Details::ConstantCharWidthFunction));
		}


	}

	namespace UTF32
	{


		void 	Wrap(const ArrayRef<char32_t>&string, count_t lineCharCount, ArrayData<StringType::ReferenceExpression<char32_t> >&result)
		{
			Details::Wrap(string,result,MakeWrapper(lineCharCount,Details::ConstantCharWidthFunction));
		}

	}


	static void	TestWrap()	//let compiler check this
	{
		UTF32String line32 = U"lädskna kljhü ßnalksj alÄks ÿ j";
		String line8 = (const char*)u8"lädskna kljhü ßnalksj alÄks ÿ j";
		UTF16String line16 = u"lädskna kljhü ßnalksj alÄks ÿ j";


		Array<StringRef> wrapped8;
		Array<UTF16StringRef> wrapped16;
		Array<UTF32StringRef> wrapped32;

		for (count_t w = 3; w < 16; w++)
		{
			UTF8::Wrap(line8,w,wrapped8);
			UTF16::Wrap(line16,w,wrapped16);
			UTF32::Wrap(line32,w,wrapped32);


			ASSERT_EQUAL__(wrapped8.Count(),wrapped32.Count());
			ASSERT_EQUAL__(wrapped16.Count(),wrapped32.Count());

			for (index_t i = 0; i < wrapped32.Count(); i++)
			{
				auto w8 = wrapped8[i];
				auto w16 = wrapped16[i];
				auto w32 = wrapped32[i];
				UTF32String up8,up16;
				StringEncoding::UTF8::ToUTF32(w8,up8);
				StringEncoding::UTF16::ToUTF32(w16,up16);

				if (up8.GetLength() != w32.GetLength())
				{
					FATAL__("Length difference. Expected utf8 to be "+String(w32.GetLength())+" characters long but found "+String(up8.GetLength()));
				}

				ASSERT2__(up8 == w32,StringEncoding::UTF32::ToUTF8(up8.ToRef()),StringEncoding::UTF32::ToUTF8(w32));
				ASSERT2__(up16 == w32,StringEncoding::UTF32::ToUTF8(up16.ToRef()),StringEncoding::UTF32::ToUTF8(w32));
			}
		}

		bool done = true;
	}

	/*
	class Test
	{
	public:
		Test()
		{
			TestWrap();
		}



	} test;
	*/

}

