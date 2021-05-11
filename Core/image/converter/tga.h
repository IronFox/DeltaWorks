#ifndef tgaH
#define tgaH

/******************************************************************

TGA-Format handler.

******************************************************************/

#include "img_format.h"

namespace DeltaWorks
{

	class TGA:public ImageFormat
	{
	public:
						TGA():ImageFormat("TGA",IEXT("tga"))
						{
							magic_bytes.free();//no magic bytes
						}
					
		virtual	void	SaveToFilePointer(const Image&image, FILE*file)	override;
		virtual	void	LoadFromFilePointer(Image&image, FILE*file)	override;
	};

	extern TGA		tga;
}

#endif
