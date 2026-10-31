#ifndef image_compressionH
#define image_compressionH

/******************************************************************

Simplified interface to extract image-date from compressed
.tex-files and texture-blocks.

******************************************************************/



#include "../image/image.h"
#include "bz2.h"
#include "delta_crc32.h"
#include "../container/vectorList.h"

namespace DeltaWorks
{
	/**
	@brief Simplified texture decompression interface for images of size 2^n * 2^m
	*/
	namespace TextureCompression
	{

		enum Codec	//! At this point supported compression codecs
		{
			NoCompression,		//!< Texture is not compressed
			SectorCompression,	//!< Texture is compressed using Image sector compression (deprecated)
			BZ2Compression,		//!< Texture uses raw BZ2 compression
			PNGCompression,		//!< Texture uses PNG image compression
			RegularCodecCount,	//!< Number of supported regular compression codecs
		
			SmallestCompression=TEX_CODEC_MASK-1	//!< Sequentially tries all available compression codecs and chooses the smallest. Applicable for compressImage() only
		};


		PixelType	extractContentType(const Ctr::ArrayData<BYTE>&source);	//!< Extracts the content type of a compressed image
		/**
			\brief  Extracts the specified compressed binary image and writes the result to a new Image object
			\param source Pointer to the first byte of the source data field
			\param size Size of the source data field in bytes
			\return Pointer to a new Image object which contains the extracted data or NULL if the provided data is not extractable.

			The client application is responsible for discarding the returned object.
		*/
		Image*     decompress(const BYTE*source, size_t size);
  
		/**
			\brief  Extracts the specified compressed binary image and writes the result to a new Image object
			\param source Reference to an array containing the compressed data
			\return Pointer to a new Image object which contains the extracted data or NULL if the provided data is not extractable.

			The client application is responsible for discarding the returned object.
		*/
		Image*     decompress(const Ctr::ArrayData<BYTE>&source);
    
		/**
			\brief  Extracts the specified compressed binary image and writes the result to the provided Image object
			\param source Pointer to the first byte of the source data field
			\param size Size of the source data field in bytes
			\return true on success
		*/
		bool        decompress(const BYTE*source, size_t size, Image&to);
	
		/**
			\brief  Extracts the specified compressed binary image and writes the result to the provided Image object
			\param source Reference to an array containing the compressed data
			\return true on success
		*/
		bool        decompress(const Ctr::ArrayData<BYTE>&source, Image&to);
	
		/*!
		\brief Compresses an image
	
		The image will be aligned to the next greater image size matching 2^n * 2^m and compressed using the specified codec.
	
		\param source	Image to compress
		\param buffer_out	Buffer to store the compressed data in. It will automatically be resized to sufficient size.
		\param codec	Compression codec to use
	
		*/
		size_t		compress(const Image&source, Ctr::Array<BYTE>&buffer_out, Codec codec=PNGCompression);
	
		const String&	GetError();
		const String&	errorStr();

	}

}

#endif
