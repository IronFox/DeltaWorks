#ifndef jpegH
#define jpegH

/******************************************************************

JPEG-Format handler.

******************************************************************/

//typedef unsigned char    boolean;
//typedef int boolean;
#undef main

/*
#if defined(__BORLANDC__) || defined(_MSC_VER)
	#define boolean int
	#undef FAR
	#define FAR
#else
	#if SYSTEM == WINDOWS
		#undef boolean
		#define boolean int
	#else
		typedef int	boolean;
	#endif
#endif
*/

#include "img_format.h"

#include "../../general/root.h"	//redefine "main"

namespace DeltaWorks
{

	class JPEG:public ImageFormat
	{
	public:
						JPEG():ImageFormat("JPEG","jpg jpeg")
						{
							magic_bytes.set4(0xff,0xd8,0xff,0xe0);
						}
					
		virtual	void	SaveToFilePointer(const Image&image, FILE*file)	override;
		virtual	void	LoadFromFilePointer(Image&image, FILE*file)	override;
	};

	extern JPEG		jpeg;
}

#endif
