#ifndef bitmapH
#define bitmapH
#include <stdio.h>


/******************************************************************

Bitmap-Format handler.

******************************************************************/

#include "img_format.h"

namespace DeltaWorks
{

	class Bitmap:public ImageFormat
	{
	public:
						Bitmap():ImageFormat("Windows Bitmap",IEXT("bmp"))
						{
							magic_bytes.set2(0x42,0x4d);
						}
					
		virtual	void	SaveToFilePointer(const Image&image, FILE*file) override;
		virtual	void	LoadFromFilePointer(Image&image, FILE*file) override;
	};

	extern Bitmap	bitmap;
}

#endif
