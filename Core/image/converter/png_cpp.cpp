#include "../../global_root.h"
#include "png.h"
#include "../../container/buffer.h"

extern "C"
{
	#include "../../third_party/png/png.h"
}

namespace DeltaWorks
{

	PNG	png;

	namespace Detail
	{

		int	GetColorType(BYTE numChannels)
		{
			switch (numChannels)
			{
				case 1:
					return PNG_COLOR_TYPE_GRAY;
				break;
				case 2:
					return PNG_COLOR_TYPE_GRAY_ALPHA;
				break;
				case 3:
					return PNG_COLOR_TYPE_RGB;
				break;
				case 4:
					return PNG_COLOR_TYPE_RGB_ALPHA;
				break;
				default:
					throw Except::IO::DriveAccess::FileFormatFault(CLOCATION,Except::globalString("Unexpected number of channels"));
				break;
			}
		}

		void OnError()
		{}

		void PNGAPI OnReadError(png_structp png, png_const_charp err_str)
		{
			throw Except::IO::DriveAccess::DataReadFault(CLOCATION,err_str);
		}

		void PNGAPI OnWriteError(png_structp png, png_const_charp err_str)
		{
			throw Except::IO::DriveAccess::DataWriteFault(CLOCATION,err_str);
		}

		void PNGAPI OnWarning(png_structp, png_const_charp)
		{}


		class WriteIO
		{
		public:
			virtual void Write(const png_byte* data, png_size_t size) = 0;
			virtual void Flush() = 0;
		};

		class ReadIO
		{
		public:
			virtual void Read(png_bytep outData, png_size_t size) = 0;
		};


		class BufferOut : public WriteIO, public Ctr::Buffer<png_byte>
		{
		public:

			virtual void Write(const png_byte* data, png_size_t size) override
			{
				Ctr::Buffer<BYTE>::Append(data,size);
			}

			virtual void Flush() override {};
		};

		class BufferRefOut : public WriteIO
		{
		public:
			BufferRefOut(Ctr::BasicBuffer<BYTE>	*buffer):buffer(buffer)
			{}

			virtual void Write(const png_byte* data, png_size_t size) override
			{
				buffer->Append(data,size);
			}

			virtual void Flush() override {};
		private:
			Ctr::BasicBuffer<BYTE>	*const buffer;
		};

		class StreamOut : public WriteIO
		{
		public:
			/**/			StreamOut(IWriteStream*stream):stream(stream)
			{}

			virtual void Write(const png_byte* data, png_size_t size) override
			{
				stream->Write(data,(serial_size_t)size);
			}

			virtual void Flush() override {};

		private:
			IWriteStream	*const stream;
		};

		class StreamIn : public ReadIO
		{
		public:
			/**/			StreamIn(IReadStream*stream):stream(stream)
			{}

			virtual void Read(png_bytep outData, png_size_t size) override
			{
				stream->Read(outData,(serial_size_t)size);
			}

		private:
			IReadStream	*const stream;
		};
		


		void WriteFunc(png_structp png, png_bytep data, png_size_t size)
		{
			((WriteIO*)png_get_io_ptr(png))->Write(data,size);
		}
		
		void ReadFunc(png_structp png, png_bytep data, png_size_t size)
		{
			((ReadIO*)png_get_io_ptr(png))->Read(data,size);
		}

		void FlushFunc(png_structp png)
		{
			((WriteIO*)png_get_io_ptr(png))->Flush();
		}


		png_structp NewReadStruct()
		{
			return png_create_read_struct(PNG_LIBPNG_VER_STRING,(void*)OnError,OnReadError,OnWarning);
		}

		png_structp NewWriteStruct()
		{
			return png_create_write_struct(PNG_LIBPNG_VER_STRING,(void*)OnError,OnWriteError,OnWarning);
		}


		void ReadPng(Image&target, png_structp png)
		{

			png_infop info= png_create_info_struct(png);
			if (!info)
				throw Except::IO::GeneralFault(CLOCATION,"PNG API failed to create info structure");

			try
			{
				png_read_info(png, info);

				png_uint_32 width,height;
				int bitDepth,colorType;
				png_get_IHDR(png, info, &width, &height, &bitDepth,
						&colorType, NULL, NULL, NULL);

				if (colorType == PNG_COLOR_TYPE_PALETTE)
					png_set_expand(png);
				if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
					png_set_expand(png);
				if (png_get_valid(png, info, PNG_INFO_tRNS))
					png_set_expand(png);
				if (bitDepth == 16)
					png_set_strip_16(png);

				auto numChannels = png_get_channels(png, info);

				target.SetContentType(PixelType::Color);

				target.SetSize(width,height,numChannels);


				const size_t  rowbytes = png_get_rowbytes(png, info);
				Array<BYTE>		rawData(rowbytes * height);
				Array<png_bytep>	rowPointers(height);

  
				for (png_uint_32 i = 0;  i < height;  ++i)
					rowPointers[i] = rawData + i*rowbytes;
				png_read_image(png, rowPointers.GetPointer());
				png_read_end(png, NULL);

				target.read(rawData.GetPointer());
				png_destroy_info_struct(png,&info);
				target.FlipVertically();
			}
			catch (...)
			{
				png_destroy_info_struct(png,&info);
				throw;
			}
		}


		void ProcessExport(BYTE*data, count_t cnt)
		{}

		void ProcessExport(UINT16*data, count_t cnt)
		{
			for (index_t i = 0; i < cnt; i++)
				data[i] = htons(data[i]);
		}

		void ProcessImport(BYTE*data, count_t cnt)
		{}

		void ProcessImport(UINT16*data, count_t cnt)
		{
			for (index_t i = 0; i < cnt; i++)
				data[i] = ntohs(data[i]);
		}
			
		template <typename T>
		void WritePng(const GenericImage<TIntImageNature<T> >&resource, png_structp png, int level = 9)
		{
			png_infop info = png_create_info_struct(png);
			if (!info)
				throw Except::IO::GeneralFault(CLOCATION,"PNG API failed to create info structure");
			try
			{
				png_set_compression_level(png,level);
				//png_set_write_fn(png,NULL,writeSection,flushSection);
				png_set_IHDR(png,info,resource.GetWidth(),resource.GetHeight(),sizeof(T)*8,Detail::GetColorType(resource.GetChannels()),PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
				png_set_sig_bytes(png,0);
				//log_file << "configuring info...\r\n";
				//memcpy(info->signature,"PNG     ",8);

				//log_file << "creating map...\r\n";
				const BYTE numChannels = resource.GetChannels();
				Array<png_bytep> rows(resource.GetHeight());
				Array<png_byte> dataCopy  = ArrayRef<const png_byte>((const png_byte*)resource.GetData(), resource.size());
		
				ProcessExport((T*)dataCopy.GetPointer(), resource.CountPixels() * resource.GetChannels());
				
				const count_t rowStride = resource.GetWidth() * resource.GetChannels() * sizeof(T);

				for (unsigned i = 0; i < resource.GetHeight(); i++)
				{
					rows[i] = dataCopy + (resource.GetHeight() - i - 1) * rowStride;
				}

				//log_file << "sending map...\r\n";

				png_set_rows(png,info,rows.GetPointer());
				//log_file << "writing png...\r\n";

				png_write_png(png,info,PNG_TRANSFORM_IDENTITY,NULL);
				//log_file << "cleaning up...\r\n";
				png_destroy_info_struct(png,&info);
				//log_file << "cleaning up...\r\n";
			}
			catch (...)
			{
				png_destroy_info_struct(png,&info);
				throw;
			}
		}

	}

	static 	inline void  _c3(const BYTE v[3], BYTE w[3])
		{
			w[0] = v[0];
			w[1] = v[1];
			w[2] = v[2];
		}

	static 	inline void  _c3(const uint16_t v[3], uint16_t w[3])
		{
			w[0] = v[0];
			w[1] = v[1];
			w[2] = v[2];
		}
	static 	inline void  _c3(const uint8_t v[3], uint16_t w[3])
		{
			w[0] = uint16_t(v[0]) << 8;
			w[1] = uint16_t(v[1]) << 8;
			w[2] = uint16_t(v[2]) << 8;
		}


	void	PNG::LoadFromFile16(Image16&target, const PathString&filename)
	{
		FILE*f = FOPEN(filename.c_str(),"rb");
		if (!f)
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"File not found or inaccessible for reading: '"+FileSystem::PathToString(filename)+"'");

		png_structp png = Detail::NewReadStruct();
		png_infop info = nullptr;
		try
		{
			if (!png)
				FATAL__("PNG read struct creation failed");

			info= png_create_info_struct(png);
			if (!info)
			{
				FATAL__("PNG info struct creation failed");
				png_destroy_read_struct(&png, NULL, NULL);
				return;
			}
	
			png_init_io(png,f);
			png_set_sig_bytes(png, 0);
			png_read_info(png, info);

			png_uint_32 width,height;
			int bitDepth,colorType;
			png_get_IHDR(png, info, &width, &height, &bitDepth,
					&colorType, NULL, NULL, NULL);

			 if (colorType == PNG_COLOR_TYPE_PALETTE)
				png_set_expand(png);
			if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
				png_set_expand(png);
			if (png_get_valid(png, info, PNG_INFO_tRNS))
				png_set_expand(png);

			auto numChannels = png_get_channels(png, info);

			target.SetContentType(PixelType::Color);

			target.SetSize(width,height,numChannels);


			const size_t  rowbytes = png_get_rowbytes(png, info);
			Array<BYTE>		rawData(rowbytes * height);
			Array<png_bytep>	rowPointers(height);

  
			for (png_uint_32 i = 0;  i < height;  ++i)
				rowPointers[i] = rawData + i*rowbytes;
			png_read_image(png, rowPointers.GetPointer());
			png_read_end(png, NULL);
			png_destroy_read_struct(&png, &info, NULL);

			//uint16_t*pixelBuffer = new uint16_t[info->channels];
			switch (bitDepth)
			{
				case 16:
					target.read((const UINT16*)rawData.GetPointer());
					Detail::ProcessImport(target.GetData(), target.CountPixels() * target.GetChannels());
				break;
				case 8:
					for (unsigned x = 0; x < width; x++)
						for (unsigned y = 0; y < height; y++)
						{
							uint16_t*pixel = target.Get(x,y);
							for (BYTE k = 0; k < numChannels; k++)
								pixel[k] = uint16_t( Math::Clamp(double(rowPointers[y][x * numChannels + k]) / 255.0 * 65535.0,0,65535));
						}
				break;
			}
			png_destroy_read_struct(&png,&info,NULL);
			target.FlipVertically();
			fclose(f);
		}
		catch (...)
		{
			png_destroy_read_struct(&png,&info,NULL);
			fclose(f);
			throw;
		}
	}

	void	PNG::SaveToFile16(const Image16&resource, const PathString&filename, int compressionLevel)
	{
		FILE*f = FOPEN(filename.c_str(),"wb");
		if (!f)
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"File is inaccessible for writing: '"+FileSystem::PathToString(filename)+"'");


		png_structp png = Detail::NewWriteStruct();
		try
		{
			png_init_io(png,f);
			Detail::WritePng<UINT16>(resource,png, compressionLevel);
			png_destroy_write_struct(&png,NULL);
			fclose(f);
		}
		catch (...)
		{
			png_destroy_write_struct(&png,NULL);
			fclose(f);
			throw;
		}

		/*
		FILE*f = FOPEN(filename.c_str(),"wb");
		if (!f)
			throw Except::IO::DriveAccess::FileOpenFault("File not found or inaccessible for writing: '"+filename+"'");
	
		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,(void*)pngError,writeError,handleWarning);
		png_init_io(png,f);
		png_infop info= png_create_info_struct(png);
		png_set_compression_level(png,compressionLevel);
		//png_set_write_fn(png,NULL,writeSection,flushSection);

		//log_file << "configuring info...\r\n";
		info->width = resource.GetWidth();
		info->height = resource.GetHeight();
		info->valid = 0;
		info->rowbytes = resource.GetWidth()*resource.GetChannels()*2;
		info->palette = NULL;
		info->num_palette = 0;
		info->num_trans = 0;
		info->bit_depth = 16;
		info->color_type = _GetColorType(resource.GetChannels());
		info->compression_type = PNG_COMPRESSION_TYPE_BASE;
		info->filter_type = PNG_FILTER_TYPE_BASE;
		info->interlace_type = PNG_INTERLACE_NONE;
		info->channels = resource.GetChannels();
		info->pixel_depth = resource.GetChannels()*16;
		info->spare_byte = 0;
		memcpy(info->signature,"PNG     ",8);

		//log_file << "creating map...\r\n";

		png_bytepp image = new png_bytep[resource.GetHeight()];
		for (unsigned i = 0; i < resource.GetHeight(); i++)
		{
			image[i] = new png_byte[resource.GetWidth()*resource.GetChannels()*2];
			uint16_t*row = (uint16_t*)image[i];
			for (unsigned j = 0; j < resource.GetWidth(); j++)
			{
				const uint16_t*pixel = resource.Get(j,resource.GetHeight()-1-i);
				uint16_t*outPixel = row + j*info->channels;
				for (BYTE k = 0; k < info->channels; k++)
					outPixel[k] = htons(pixel[k]);
			}
		}

		//log_file << "sending map...\r\n";

		png_set_rows(png,info,image);
		//log_file << "writing png...\r\n";

		png_write_png(png,info,PNG_TRANSFORM_IDENTITY,NULL);
		//log_file << "cleaning up...\r\n";
		png_destroy_info_struct(png,&info);
		//log_file << "cleaning up...\r\n";
		png_destroy_write_struct(&png,NULL);


		for (unsigned i = 0; i < resource.GetHeight(); i++)
		{
			delete[] image[i];
		}
		delete[] image;
		fclose(f);
		*/
	}


	void PNG::SaveToFileQ(const Image&resource, const PathString&filename)
	{
		FILE*f = FOPEN(filename.c_str(),"wb");
		if (!f)
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"File is inaccessible for writing: '"+FileSystem::PathToString(filename)+"'");


		png_structp png = Detail::NewWriteStruct();
		try
		{
			png_init_io(png,f);
			Detail::WritePng<BYTE>(resource,png,2);	//fast compression
			png_destroy_write_struct(&png,NULL);
			fclose(f);
		}
		catch (...)
		{
			png_destroy_write_struct(&png,NULL);
			fclose(f);
			throw;
		}
	}







	void PNG::SaveToFilePointer(const Image&resource, FILE*f)
	{
		png_structp png = Detail::NewWriteStruct();
		try
		{
			png_init_io(png,f);
			Detail::WritePng<BYTE>(resource,png);
			png_destroy_write_struct(&png,NULL);
		}
		catch (...)
		{
			png_destroy_write_struct(&png,NULL);
			throw;
		}
	}


	void PNG::LoadFromFilePointer(Image&target, FILE*f)
	{
		png_structp png = Detail::NewReadStruct();
		if (!png)
			FATAL__("PNG read struct creation failed");
		try
		{
			png_init_io(png,f);
			png_set_sig_bytes(png, 0);
			Detail::ReadPng(target,png);
			png_destroy_read_struct(&png,nullptr,nullptr);
		}
		catch (...)
		{
			png_destroy_read_struct(&png,nullptr,nullptr);
			throw;
		}
	}




	/*static*/	void		PNG::CompressToStream(const Image&source_image, IWriteStream&stream)
	{
		Detail::StreamOut str(&stream);

		png_structp png = Detail::NewWriteStruct();
		try
		{
			png_set_write_fn(png,&str,Detail::WriteFunc,Detail::FlushFunc);
			Detail::WritePng(source_image, png);
			png_destroy_write_struct(&png,NULL);
		}
		catch (...)
		{
			png_destroy_write_struct(&png,NULL);
			throw;
		}
	}

	
	/*static*/	void		PNG::CompressToBuffer(const Image&image, Ctr::BasicBuffer<BYTE>&data)
	{
		Detail::BufferRefOut buf(&data);
		
		png_structp png = Detail::NewWriteStruct();
		try
		{
			png_set_write_fn(png,&buf,Detail::WriteFunc,Detail::FlushFunc);
			Detail::WritePng(image, png);
			png_destroy_write_struct(&png,NULL);
		}
		catch (...)
		{
			png_destroy_write_struct(&png,NULL);
			throw;
		}
	}


	/*static*/	void		PNG::CompressToArray(const Image&source_image, Ctr::Array<BYTE>&data)
	{
		Detail::BufferOut buf;
		
		png_structp png = Detail::NewWriteStruct();
		try
		{
			png_set_write_fn(png,&buf,Detail::WriteFunc,Detail::FlushFunc);
			Detail::WritePng(source_image, png);
			png_destroy_write_struct(&png,NULL);
			data = buf.ToRef();
		}
		catch (...)
		{
			png_destroy_write_struct(&png,NULL);
			throw;
		}
	}


	/*static*/	void		PNG::DecompressData(Image&out_image, const void*data, size_t data_size)
	{
		MemReadStream source(data,data_size);
		Detail::StreamIn str(&source);
		
		png_structp png = Detail::NewReadStruct();
		try
		{
			png_set_read_fn(png,&str,Detail::ReadFunc);
			Detail::ReadPng(out_image, png);
			png_destroy_read_struct(&png,nullptr,nullptr);
		}
		catch (...)
		{
			png_destroy_read_struct(&png,nullptr,nullptr);
			throw;
		}
	}

	/*static*/	void		PNG::DecompressArray(Image&out_image, const Ctr::ArrayRef<const BYTE>&data)
	{
		DecompressData(out_image,data.pointer(),data.size());

	}


	/*static*/	void		PNG::DecompressStream(Image&out_image, IReadStream&source)
	{
		Detail::StreamIn str(&source);
		
		png_structp png = Detail::NewReadStruct();
		try
		{
			png_set_read_fn(png,&str,Detail::ReadFunc);
			Detail::ReadPng(out_image, png);
			png_destroy_read_struct(&png,nullptr,nullptr);
		}
		catch (...)
		{
			png_destroy_read_struct(&png,nullptr,nullptr);
			throw;
		}
	}
}
