#pragma once

#include <global_root.h>
#include "image.h"

namespace DeltaWorks
{
	namespace FontToImage
	{
		class Font
		{
		public:
			/**/	Font(const char*fontName, int fontHeight, bool bold, bool italic);
			/**/	Font(const char*fontName, int fontWidth, int fontHeight, bool bold, bool italic);
			/**/	Font(Font&&other);
			/**/	Font();

			void	operator=(Font&& other);

			virtual ~Font();


			void	Rasterize(char32_t unicodeChar, UINT borderWidth, Image& image, int& pixelOriginX, int& pixelOriginY) const;
		private:
			HFONT	font;
			HDC		hdc;
		};



	}

}

