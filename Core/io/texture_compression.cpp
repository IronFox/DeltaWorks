#include "../global_root.h"
#include "texture_compression.h"

/******************************************************************

Simplified interface to extract image-date from compressed
.tex-files and texture-blocks.

******************************************************************/

#undef LOG_TEXTURE_COMPRESSION
//#define LOG_TEXTURE_COMPRESSION	//define this to write compression operations to compression.log in working directory

#include "../image/converter/png.h"


#undef LOG_TEXTURE_COMPRESSION_STAGE
#ifdef LOG_TEXTURE_COMPRESSION
	#include "../io/log.h"

	#define LOG_TEXTURE_COMPRESSION_STAGE(STAGE)	log_file << STAGE << nl;
#else
	#define LOG_TEXTURE_COMPRESSION_STAGE(STAGE)
#endif


namespace DeltaWorks
{
	static String	ext_error;








	PixelType	TextureCompression::extractContentType(const Ctr::ArrayData<BYTE>&source)
	{
		if (source.size() < sizeof(Image::THeader))
			return PixelType::None;
		return Image::GetContentType(*(const Image::THeader*)source.pointer());
	}

	Image*    TextureCompression::decompress(const BYTE*data, size_t size)
	{
		if (size <= sizeof(Image::THeader))
		{
			ext_error = "Field is empty";
			return NULL;
		}
		const Image::THeader*head = (Image::THeader*)data;
		UINT32 crc = CRC32::Sequence().Append(&head->settings,size-4).Finish();
		if (crc != head->crc)
		{
			ext_error = "CRC mismatch";
			return NULL;
		}

		Image*result = SignalNew(new Image(*head));


		switch (head->type&TEX_CODEC_MASK)
		{
			case NoCompression:
				result->read(&data[sizeof(Image::THeader)]);
			break;
			case SectorCompression:
				ext_error = ("Built-in compression is no longer supported");
				Discard(result);
				return NULL;
			break;
			case BZ2Compression:
			{
				if (!BZ2::decompress(&data[sizeof(Image::THeader)],size-sizeof(Image::THeader),result->GetData(),result->size()))
				{
					ext_error = "Decompression failed ("+String(BZ2::errorStr())+")";
					Discard(result);
					return NULL;
				}
			}
			break;
			case PNGCompression:
			{
				png.DecompressArray(*result,ArrayRef<const BYTE>(data+sizeof(Image::THeader),size-sizeof(Image::THeader)));
			}
			break;
			default:
				ext_error = "Codec not supported ("+IntToStr(head->type&TEX_CODEC_MASK)+")";
				Discard(result);
				return NULL;
		}
		return result;
	}

	Image*    TextureCompression::decompress(const Ctr::ArrayData<BYTE>&data)
	{
		return decompress(data.pointer(),data.GetLength());
	}

	bool TextureCompression::decompress(const BYTE*data, size_t size, Image&to)
	{
		if (size <= sizeof(Image::THeader))
		{
			ext_error = "Field is empty";
			return false;
		}
		Image::THeader*head = (Image::THeader*)data;
		UINT32 crc = CRC32::Sequence().Append(&head->settings,size-4).Finish();
		if (crc != head->crc)
		{
			ext_error = "CRC mismatch";
			return false;
		}

		to.applyHeader(*head);
	
		switch (head->type&TEX_CODEC_MASK)
		{
			case NoCompression:
				to.read(&data[sizeof(Image::THeader)]);
			break;
			case SectorCompression:
				ext_error = ("Built-in compression is no longer supported");
				return false;
			break;
			case BZ2Compression:
			{
				if (!BZ2::decompress(&data[sizeof(Image::THeader)],size-sizeof(Image::THeader),to.GetData(),to.size()))
				{
					ext_error = "Decompression failed ("+String(BZ2::errorStr())+")";
					return false;
				}
			}
			break;
			case PNGCompression:
			{
				png.DecompressArray(to,ArrayRef<const BYTE>(data+sizeof(Image::THeader),size-sizeof(Image::THeader)));
			}
			break;
			default:
				ext_error = "Codec not supported ("+IntToStr(head->type&TEX_CODEC_MASK)+")";
				return false;
		}
		return true;
	}


	bool TextureCompression::decompress(const Ctr::ArrayData<BYTE>&data, Image&to)
	{
		return decompress(data.pointer(),data.GetLength(),to);
	}

	static size_t autoCompressImage(const Image&source, Ctr::Array<BYTE>&buffer)	//this is extremely inefficient but i can't be bothered right now.
	{
		Ctr::Array<BYTE>	temp;
		size_t current = TextureCompression::compress(source,buffer,TextureCompression::NoCompression);
		for (unsigned i = 1; i < TextureCompression::RegularCodecCount; i++)
		{
			size_t s = TextureCompression::compress(source,temp,(TextureCompression::Codec)i);
			if (s > 0 && s < current)
			{
				buffer = temp;
				current = s;
			}
		}
		return current;
	}

	size_t TextureCompression::compress(const Image&source, Ctr::Array<BYTE>&buffer, Codec codec)
	{
		if (codec == SmallestCompression)
			return autoCompressImage(source,buffer);
	
		Image::THeader	header;
		bool exact = source.writeHeader(header);
	
		header.type |= codec;
	
		
		Image local;
		const Image*from;
		if (exact)
			from = &source;
		else
		{
			local = source;
			local.Resample((1<<header.x_exp),(1<<header.y_exp));
			from = &local;
		}
	
		size_t compressed_size = 0;
		#ifdef LOG_TEXTURE_COMPRESSION
			static LogFile	log_file("compression.log",true);
		#endif

		LOG_TEXTURE_COMPRESSION_STAGE("compressing "<<from->GetWidth()<<"*"<<from->GetHeight()<<"*"<<from->GetChannels());


		switch (codec)
		{
			case NoCompression:
				compressed_size = from->size();
				buffer.SetSize((unsigned)(compressed_size+sizeof(Image::THeader)));
				memcpy(buffer+sizeof(Image::THeader),from->GetData(),compressed_size);
			break;
			case SectorCompression:
			{
				ext_error = ("Built-in compression is no longer supported");
				return 0;
			}
			break;
			case BZ2Compression:
			{
				Ctr::Array<BYTE>	temp((unsigned)(from->size()));
				compressed_size = BZ2::compress(from->GetData(), from->size(), temp.pointer(), temp.GetContentSize());
				if (!compressed_size)
				{
					ext_error = "Compression failed ("+String(BZ2::errorStr())+")";
					return 0;
				}	
				buffer.SetSize((unsigned)(compressed_size+sizeof(Image::THeader)));
				memcpy(buffer+sizeof(Image::THeader),temp.pointer(),compressed_size);
			}
			break;
			case PNGCompression:
			{
				Ctr::Vector<BYTE>	out_buffer;
				png.CompressToBuffer(*from,out_buffer);

				buffer.SetSize(sizeof(Image::THeader)+out_buffer.Count());
				LOG_TEXTURE_COMPRESSION_STAGE("buffer resized to "<<buffer.GetContentSize()<<" byte(s)");
				memcpy(buffer.pointer()+sizeof(Image::THeader),out_buffer.pointer(),compressed_size);
			}
			break;
		}
		if (!compressed_size)
			return 0;
		
		(*(Image::THeader*)buffer.pointer()) = header;
		Image::THeader*head = (Image::THeader*)buffer.pointer();
		head->crc = CRC32::Sequence().Append(&head->settings,buffer.size()-4).Finish();
	
		LOG_TEXTURE_COMPRESSION_STAGE("all done");
	
		return buffer.size();
		/*}
		catch (...)
		{
			return 0;
		}*/
	}

	const String&	TextureCompression::GetError()
	{
		return ext_error;
	}

	const String&	TextureCompression::errorStr()
	{
		return ext_error;
	}
}
