#include "font_to_image.h"



namespace DeltaWorks
{
	namespace FontToImage
	{
		/**/	Font::Font(Font&& other)
		{
			font = other.font;
			hdc = other.hdc;
			other.font = nullptr;
			other.hdc = nullptr;
		}

		/**/	Font::Font() :hdc(nullptr), font(nullptr)
		{}


		void	Font::operator=(Font&& other)
		{
			font = other.font;
			hdc = other.hdc;
			other.font = nullptr;
			other.hdc = nullptr;
		}



		/**/	Font::Font(const char* fontName, int fontHeight, bool bold, bool italic) :Font(fontName, 0, fontHeight, bold, italic)
		{}

		/**/	Font::Font(const char* fontName, int fontWidth, int fontHeight, bool bold, bool italic)
		{
			font = ::CreateFontA(fontHeight,           // height of font
				fontWidth,            // average character width
				0,                    // angle of escapement
				0,					  // base-line orientation angle
				bold ? 700 : 400,     // font weight
				italic,               // italic attribute option
				FALSE,                // underline attribute option
				FALSE,                // strikeout attribute option
				ANSI_CHARSET,         // character set identifier
				OUT_DEFAULT_PRECIS,   // output precision
				CLIP_DEFAULT_PRECIS,  // clipping precision
				ANTIALIASED_QUALITY,  // output quality
				FF_DONTCARE,          // pitch and family
				fontName);            // typeface name
			hdc = ::GetDC(0);
		}

		/*virtual*/ Font::~Font()
		{
			if (hdc)
				::ReleaseDC(0, hdc);
			if (font)
				::DeleteObject(font);
		}


		void	Font::Rasterize(char32_t unicodeChar, UINT borderWidth, Image& image, int& pixelOriginX, int& pixelOriginY) const
		{
			if (!hdc || !font)
				throw new Except::Program::GeneralFault(CLOCATION, "Font reference disposed");
			HGDIOBJ old_font = ::SelectObject(hdc, font);

			GLYPHMETRICS metrics;
			MAT2 mat;
			memset(&mat, 0, sizeof(mat));
			mat.eM11.value = 1;
			mat.eM22.value = 1;
			bool success = false;

			auto len = GetGlyphOutlineA(hdc, unicodeChar, GGO_GRAY8_BITMAP, &metrics, 0, nullptr, &mat);
			if (len != GDI_ERROR)
			{
				//Array<BYTE> data;
				//data.SetSize(len);
				//data.Fill(0);
				auto expectedLen = metrics.gmBlackBoxX * metrics.gmBlackBoxY;
				//if (len == expectedLen)
				{
					//auto numRows = len / metrics.gmCellIncX;
					auto numCols = len / metrics.gmBlackBoxY;
					auto numRows = metrics.gmBlackBoxY;
					auto expectedLen2 = numCols * numRows;
					if (expectedLen2 == len)
					{
						image.SetSize(numCols + borderWidth * 2, numRows + borderWidth * 2, 1);
						Array<BYTE> temp;
						temp.SetSize((count_t)len);
						auto rtn = GetGlyphOutlineA(hdc, unicodeChar, GGO_GRAY8_BITMAP, &metrics, len, temp.GetPointer(), &mat);

						for (UINT x = 0; x < numCols; x++)
							for (UINT y = 0; y < numRows; y++)
								image.Set(x+borderWidth, y + borderWidth, temp[y * numCols + x] * 255 / 64, 0, 0);

						for (UINT x = 0; x < image.GetWidth(); x++)
							for (UINT y = 0; y < borderWidth; y++)
							{
								image.Set(x, y, 0, 0, 0, 0);
								image.Set(x, image.GetHeight() - y-1, 0, 0, 0, 0);
							}
						for (UINT y = borderWidth; y < image.GetHeight()- borderWidth; y++)
							for (UINT x = 0; x < borderWidth; x++)
							{
								image.Set(x, y, 0, 0, 0, 0);
								image.Set(image.GetWidth()-x-1, y, 0, 0, 0, 0);
							}


						image.FlipVertically();
						pixelOriginY = metrics.gmBlackBoxY - metrics.gmptGlyphOrigin.y;
						pixelOriginX = metrics.gmptGlyphOrigin.x;
						success = true;
					}
				}
			}

			

			::SelectObject(hdc, old_font);

			if (!success)
				throw Except::Program::GeneralFault(CLOCATION,"Font rasterization failed");
		}




	}

}

