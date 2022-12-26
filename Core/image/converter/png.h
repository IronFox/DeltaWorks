#ifndef pngH
#define pngH


#define PNG_FILE_IO_ERROR   1
#define PNG_OK              0
#define PNG_NOT_SUPPORTED   (-1)
#define PNG_INTERNAL        (-2)
#define PNG_ERROR           (-3)


#include "img_format.h"
#ifdef __BORLANDC__
	#undef FAR
	#define FAR
#endif
#include "../../container/vectorList.h"

namespace DeltaWorks
{


	class PNG:public ImageFormat
	{
	public:
		PNG()				:ImageFormat("PNG",IEXT("png")) {magic_bytes.set4(0x89,0x50,0x4e,0x47);}
		virtual	void		SaveToFilePointer(const Image&image, FILE*file)			override;
		virtual	void		LoadFromFilePointer(Image&image, FILE*file)				override;
		void				SaveToFileQ(const Image&image, const PathString&filename);	//!< Identical to saveToFile() using weaker compression
		static	void		CompressToArray(const Image&image, Ctr::Array<BYTE>&data);
		static	void		CompressToBuffer(const Image&image, Ctr::BasicVector<BYTE>&data);
		static	void		CompressToStream(const Image&image, IWriteStream&stream);
		static	void		DecompressArray(Image&image, const Ctr::ArrayRef<const BYTE>&data);
		static	void		DecompressData(Image&image, const void*data, size_t data_size);
		static	void		DecompressStream(Image&out_image, IReadStream&stream);

		void				LoadFromFile16(Image16&, const PathString&filename);
		void				SaveToFile16(const Image16&, const PathString&filename, int compressionLevel);
	};

	extern PNG		png;
}

#endif
