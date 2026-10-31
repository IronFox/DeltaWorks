#include "../../global_root.h"

#include "jpeg.h"

extern "C"
{
	#undef boolean
	#include <jpeglib.h>
}


/******************************************************************

JPEG-Format handler.

******************************************************************/

namespace DeltaWorks
{

	JPEG	jpeg;

	int jpeg_error;
	char jpeg_detail[JMSG_LENGTH_MAX];

	struct my_error_mgr {
	  struct jpeg_error_mgr pub;	/* "public" fields */

	  int setjmp_buffer;	/* for return to caller */
	};

	typedef struct my_error_mgr * my_error_ptr;



	void my_error_exit (j_common_ptr cinfo_ptr)
	{
		jpeg_decompress_struct* cinfo_tmp = reinterpret_cast<jpeg_decompress_struct*>(cinfo_ptr);
		jpeg_decompress_struct& cinfo = *cinfo_tmp;
		const char* msg1 = cinfo.err->jpeg_message_table[cinfo.err->msg_code - cinfo.err->first_addon_message];
		const char* msg2 = "";
		if (cinfo.err->addon_message_table != NULL)
		{
			msg2 = cinfo.err->addon_message_table[cinfo.err->msg_code - cinfo.err->first_addon_message];
			//MessageBox(NULL,msg2,NULL,MB_OK);
		}
		throw Except::IO::DriveAccess::DataReadFault(String(msg1) + '\n' + String(msg2));
	//	
	//	//const char* msg2 = "";
	//	MessageBox(NULL,msg1,NULL,MB_OK);
	//	if (cinfo.err->addon_message_table != NULL)
	//	{
	//	msg2 = cinfo.err->addon_message_table[cinfo.err->msg_code - cinfo.err->first_addon_message];
	//	MessageBox(NULL,msg2,NULL,MB_OK);
	//	}
	//	jpeg_destroy_decompress(&cinfo);
	//
	//
	////	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	//	//char buffer[JMSG_LENGTH_MAX];
	//	//(*cinfo->err->format_message) (cinfo, jpeg_detail);
	//	//longjmp(myerr->setjmp_buffer, 1);
	//	
	//	throw IO::DriveAccess::DataReadFault();
	}



	void j_putRGBScanline(BYTE *jpegline, 
						 int widthPix,
						 BYTE *outBuf,
						 int row)
	{
		int offset = row * widthPix * 3;
		int count;
		for (count=0;count<widthPix;count++) 
		{
			*(outBuf + offset + count * 3 + 0) = *(jpegline + count * 3 + 0);
			*(outBuf + offset + count * 3 + 1) = *(jpegline + count * 3 + 1);
			*(outBuf + offset + count * 3 + 2) = *(jpegline + count * 3 + 2);
		}
	}

	//
	//	stash a gray scanline
	//

	void j_putGrayScanlineToRGB(BYTE *jpegline, 
								 int widthPix,
								 BYTE *outBuf,
								 int row)
	{
		int offset = row * widthPix * 3;
		int count;
		for (count=0;count<widthPix;count++) {

			BYTE iGray;

			// get our grayscale value
			iGray = *(jpegline + count);

			*(outBuf + offset + count * 3 + 0) = iGray;
			*(outBuf + offset + count * 3 + 1) = iGray;
			*(outBuf + offset + count * 3 + 2) = iGray;
		}
	}

	void	JPEG::LoadFromFilePointer(Image&result, FILE*f)
	{
		struct jpeg_decompress_struct cinfo;
		struct my_error_mgr jerr;
		JSAMPARRAY buffer;		/* Output row buffer */
		int row_stride;		/* physical row width in output buffer */

		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = my_error_exit;
	/*	if (setjmp(jerr.setjmp_buffer))
		{
			jpeg_error = JPEG_DATA_ERROR;
			jpeg_destroy_decompress(&cinfo);
			fclose(f);
			return NULL;
		}*/
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, f);
		(void) jpeg_read_header(&cinfo, TRUE);
		(void) jpeg_start_decompress(&cinfo);
		result.SetContentType(PixelType::Color);
		result.SetSize(cinfo.output_width,cinfo.output_height,3);
	
		/* JSAMPLEs per row in output buffer */
		row_stride = cinfo.output_width * cinfo.output_components;

		/* Make a one-row-high sample array that will go away when done with image */
		buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			(void) jpeg_read_scanlines(&cinfo, buffer, 1);
			if (cinfo.out_color_components==3)
			{
				j_putRGBScanline(buffer[0],
									result.GetWidth(),
									result.GetData(),
									cinfo.output_scanline-1);

			}
			else
				if (cinfo.out_color_components==1)
				{
    				j_putGrayScanlineToRGB(buffer[0],
	    								result.GetWidth(),
		    							result.GetData(),
			    						cinfo.output_scanline-1);
    			}
		}
		result.FlipVertically();
		(void) jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	}


	void	JPEG::SaveToFilePointer(const Image&img, FILE*f)
	{
		if (img.GetChannels() > 3)
		{
			Image copy = img;
			copy.SetChannels(3);
			SaveToFilePointer(copy,f);
			return;
		}


		ASSERT__(img.GetChannels() == 1 || img.GetChannels() == 3);



		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
	
		JSAMPROW row_pointer[1];
		cinfo.err = jpeg_std_error( &jerr );
		jpeg_create_compress(&cinfo);
		jpeg_stdio_dest(&cinfo, f);


		cinfo.image_width = img.GetWidth();	
		cinfo.image_height = img.GetHeight();
		cinfo.input_components = img.GetChannels();
		cinfo.in_color_space = img.GetChannels() == 3 ? JCS_RGB : JCS_GRAYSCALE;

		jpeg_set_defaults( &cinfo );

		jpeg_set_quality( &cinfo, 90, FALSE);

		jpeg_start_compress( &cinfo, TRUE );

		Array<BYTE> lineCopy(img.GetWidth() * img.GetChannels());

		while( cinfo.next_scanline < cinfo.image_height )
		{
			lineCopy.copyFrom(img.GetPixel(0,img.GetHeight() - cinfo.next_scanline - 1),lineCopy.Count());
			row_pointer[0] = lineCopy.GetPointer();
				//&raw_image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
			jpeg_write_scanlines( &cinfo, row_pointer, 1 );
		}

		jpeg_finish_compress( &cinfo );
		jpeg_destroy_compress( &cinfo );










	}
}

