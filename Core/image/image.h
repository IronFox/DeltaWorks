#ifndef imageH
#define imageH


/******************************************************************

Image-class used to store and process textures
(or images in general).

******************************************************************/

#include "../global_root.h"
#include "../io/bit_stream.h"
#include "../math/vector.h"
#include <stdio.h>
//#include <ppl.h>
#include "../general/parallel_for.h"
#include <random>
#include "../general/undef.h"
#include "../container/array2d.h"



#define		TEX_NORMAL_BIT			0x80
#define		TEX_OBJECT_SPACE_BIT	0x40
#define		TEX_CODEC_MASK			(0xFF^(TEX_NORMAL_BIT|TEX_OBJECT_SPACE_BIT))

#define		COS_RESIZE

namespace DeltaWorks
{

	CONSTRUCT_ENUMERATION8(PixelType, None, Color, StrictColor, PureLuminance, BGRA, Depth,ObjectSpaceNormal, TangentSpaceNormal);
	//StrictColor enforces loading 1 and 2 channel images as R and RG textures, respectively. Effective only when loading images into VRAM, mostly identical to Color otherwise



	class ImageRandomSource : std::mt19937
	{
	public:
		typedef std::mt19937	Super;
		ImageRandomSource(unsigned long seed) : Super(std::max<unsigned long>(1,seed))
		{}


		float	operator()()	{return float(Super::operator()()) / float(Super::max());}


	};


	template <typename T>
		class ImageTemplate;

	/**
		@brief Basic image implementation
	*/
	class BaseImage
	{
	public:
		typedef UINT32			dimension_t,dim_t;	
			
		struct THeader	//! File header used for certain texture io-operations
		{
					UINT32		crc;	//!< Image crc32 checksum (including settings)
			union
			{
				struct
				{
					BYTE		x_exp,	//!< Size x-exponent (final image width = 2^x_exp)
								y_exp,	//!< Size y-exponent (final image height = 2^y_exp)
								channels,	//!< Image color channels
								type;		//!< Image content type
				};
				UINT32			settings;
			};
		};
		template <bool IsHorizontal>
			class Axis;

		template <bool IsHorizontal>
			class Iterator
			{
				dim_t	at;
				friend class Axis<IsHorizontal>;

				/**/			Iterator(dim_t at):at(at)	{}
			public:
				Iterator		operator++(int) {Iterator rs(*this); at++; return rs;}
				Iterator&		operator++() {at++; return *this;}
				Iterator		operator--(int) {Iterator rs(*this); at--; return rs;}
				Iterator&		operator--() {at--; return *this;}
				dim_t			operator*() const {return at;}
				bool			operator==(dim_t other) const {return at == other;}
				bool			operator!=(dim_t other) const {return at != other;}
				bool			operator<=(dim_t other) const {return at <= other;}
				bool			operator>=(dim_t other) const {return at >= other;}
				bool			operator<(dim_t other) const {return at < other;}
				bool			operator>(dim_t other) const {return at > other;}
				bool			operator==(const Iterator<IsHorizontal>&other) const {return at == other.at;}
				bool			operator!=(const Iterator<IsHorizontal>&other) const {return at != other.at;}
				bool			operator<=(const Iterator<IsHorizontal>&other) const {return at <= other.at;}
				bool			operator>=(const Iterator<IsHorizontal>&other) const {return at >= other.at;}
				bool			operator<(const Iterator<IsHorizontal>&other) const {return at < other.at;}
				bool			operator>(const Iterator<IsHorizontal>&other) const {return at > other.at;}
			};
		
		template <bool IsHorizontal>
			class Axis
			{
				const dim_t		extent;

				friend class BaseImage;

				/**/			Axis(dim_t extent):extent(extent)	{}
			public:
				typedef Iterator<IsHorizontal>	iterator,const_iterator;

				constexpr iterator	begin() const {return 0;}
				iterator		end() const {return extent;}
				dim_t			size() const {return extent;}
			};	

	protected:
		dimension_t				image_width,		//!< Width of the image in pixels.
								image_height;		//!< Height of the image in pixels.
		BYTE					image_channels;	//!< Number of color channels
		PixelType				content_type;	//!< Contained image type. Certain methods will behave differently depending on this value.


	public:
		String					origin;		//!< Custom string. Possibly a filename. This value is carried along but not used by any method of this file
		enum channel_append_t{	//! Operation to perfom if new channels are appended via the SetChannel() method.
								CA_ZERO=0,	//!< Set the content of new channels to 0 (black)
								CA_REPEAT,	//!< Repeat the content of the last defined channel
								CA_UNDEF,	//!< Leave the content of new channels undefined
								CA_HALF,	//!< Set the content of new channels to half the maximum possible value (grey)
								CA_MAX		//!< Set the content of new channels to the maximum possible value (white)
								};
							
		enum combiner_t{	//! Operation to perform when drawing images into the local image.
								IC_COPY,			//!< Overwrites the local pixel data with the provided remote pixel data
								IC_ALPHA_BLEND,		//!< Blends the provided remote pixel data above the local pixel data (using the remote 4th channel as opacity value)
								IC_MULTIPLY,		//!< Each channel of the local pixel data will be multiplied with the respective channel of the remote pixel data
								IC_MULT_NEGATIVE,	//!< Each channel of the local pixel data will be negative multiplied with the respective channel of the remote pixel data
								IC_ALPHA_ADD,		//!< The pixel data will be multipled with any available alpha channel and then added. Behaves identical to IC_ADD if no alpha channel is available
								IC_ADD,				//!< The pixel data of the remote image will be added to the pixel data of the local image (and clamped to [0,255] if necessary).
							
							
								Copy=IC_COPY,
								Replace=IC_COPY,
								AlphaBlend=IC_ALPHA_BLEND,
								Multiply=IC_MULTIPLY,
								MultNegative=IC_MULT_NEGATIVE,
								AlphaAdd=IC_ALPHA_ADD,
								Blend=IC_ALPHA_ADD,
								Add=IC_ADD
								};

							BaseImage(dimension_t width=0, dimension_t height=0, BYTE channels=3, PixelType type=PixelType::Color):image_width(width),image_height(height),image_channels(channels),content_type(type)
							{}
		virtual				~BaseImage()	{};
		Axis<true>			Horizontal() const {return Axis<true>(GetWidth());}
		Axis<false>			Vertical() const {return Axis<false>(GetHeight());}
		dimension_t			GetWidth()															const;	//!< Queries current image width \return Image width in pixels
		dimension_t			GetHeight()															const;	//!< Queries current image height \return Image height in pixels
		count_t				CountPixels()														const;	//!< Queries the number of pixelsd
		float				GetPixelAspect()													const;	//!< Queries the pixel aspect (width/height) of the local image
		dimension_t			dimension(BYTE axis)												const; 	//!< Queries current image width/height \param axis Coordinate axis to retrieve (0=width, 1=height) \return Image width or height in pixels
		dimension_t			GetDimension(BYTE axis)												const; 	//!< Queries current image width/height \param axis Coordinate axis to retrieve (0=width, 1=height) \return Image width or height in pixels
		bool				IsEmpty()															const;	//!< Checks if the local image contains at least one pixel and one channel, false otherwise
		bool				IsNotEmpty()														const;	//!< identical to !IsEmpty()		
		BYTE				channels()															const;	//!< Queries current image color channels \return Number of color channels
		BYTE				GetChannels()														const;	//!< Queries current image color channels \return Number of color channels
		PixelType			contentType()														const;	//!< Queries content type \return content type
		PixelType			GetContentType()													const;	//!< Queries content type \return content type
		static PixelType	contentType(const THeader&header);											//!< Extracts the content type from the provided image header
		static PixelType	GetContentType(const THeader&header);										//!< Extracts the content type from the provided image header
		String				contentTypeString()													const;	//!< Generates a short string describing the current content type \return content type string
		String				GetContentTypeString()												const;	//!< Generates a short string describing the current content type \return content type string
		void				SetContentType(PixelType type);												//!< Sets content type. Does not change the actual data.
		bool				writeHeader(THeader&header)											const;	//!< Generates a header from the local image data \return true if the local image coordinates could be transformed into exponents, false otherwise. The resulting header will contain approximated exponents if false is returned
		THeader				header()															const;	//!< Generates a header from the local image data
		THeader				GetHeader()															const;	//!< Generates a header from the local image data
		String				ToString()															const;		//!< Generates a string representation of the local image (width, height, channels, type, etc)
		bool				IsColorMap()														const;	//!< Checks if the local map contains color pixels
		bool				IsNormalMap()														const;	//!< Checks if the local map contains normal pixels

		bool				operator==(const BaseImage&other) const;
		bool				operator!=(const BaseImage&other) const {return !operator==(other);}
	};


	/**
		@brief General image template

		This object may hold integer and floating point channel values and provides only methods
		that may seamlessly be applied to either.
	*/
	template <typename T>
		class ImageTemplate:public BaseImage
		{
			typedef BaseImage			Super;
		protected:
			using Super::dimension_t;
			using Super::Iterator;
	
				T						*image_data;		//!< Pointer to the actual image data.

			template <typename Float>
				void					sample(Float x0, Float y0, Float x1, Float y1, T*target)		const;

			bool						ColumnIsOpaque(dimension_t x, bool IsOpaque(const T*))	const;
			bool						RowIsOpaque(dimension_t y, bool IsOpaque(const T*))		const;
			static bool					RGBAIsOpaque(const T*);
			static bool					GrayAIsOpaque(const T*);
		public:
			typedef Iterator<true>		X;
			typedef Iterator<false>		Y;
			typedef ImageTemplate<T>	Self;

			/**/						ImageTemplate();
			/**/						ImageTemplate(const Self&other);									//!< Copy constructor
			/**/						ImageTemplate(dimension_t width, dimension_t height, BYTE channels=3,PixelType type=PixelType::Color);		//!< Direct constructor. Directly allocates a field of the specified size. \param width Image width \param height Image height, \param channels Number of color channels. \param type Image content type
			/**/						ImageTemplate(const THeader&header);															//!< Header constructor. Configures the local image based on the information stored in the provided header
			virtual						~ImageTemplate();
			Self&						operator=(const Self&other);												//!< Copy assignment
			void						free()
										{
											dealloc(image_data);
											image_data = NULL;
											image_width = 0;
											image_height = 0;
											image_channels = 3;
											content_type = PixelType::Color;
										}
			void						clear()	{free();}
			void						Clear() {free();}
			void						applyHeader(const THeader&header);											//!< Changes the size, channels, and type of the local image based on the provided header. Any content in the local image will be lost if the size of the local color array changes
			inline	T*					GetData()	/**@copydoc data()*/ {return data();}
			inline	const T*			GetData() const	/**@copydoc data()*/ {return data();}
			inline	T*					data();																	//!< Retrieves a pointer to the actual pixel data of the image \return Pointer to the image pixel data array. The returned array <b>must not be deleted</b>
			inline	const T*			data()																const;	//!< Const version of GetData().
			void						adoptData(ImageTemplate<T>&other);													//!< Clears any local data and adopts all pointers of the other image. The respective other image will be empty when the operation is executed.
			void						swap(ImageTemplate<T>&other);													//!< Swaps all pointers and attributes with the other image.
			friend void					swap(Self&a, Self&b)	{a.swap(b);}
			void						SetSize(dimension_t width, dimension_t height, BYTE channels);				//!< Alters the dimensions of the local image. The content of the pixel data field is lost if it is resized during this operation. \param width New image width in pixels \param height New image height in pixels. \param channels New number of color channels.
			inline	void				SetChannel(dimension_t x, dimension_t y, BYTE channel, T newData);	//!< Alters the content of one color channel of one specific pixel.  \param x x-coordinate of the pixel (0 = left most pixel, must be valid) \param y y-coordinate of the pixel (usually 0 = bottom most pixel, must be valid) \param channel Target channel (must be valid) \param newData New value for the specified channel of the specified pixel.
			void						SetChannel(BYTE channel, T new_value);							//!< Alters the content of one color channel of all pixels  \param channel Target channel (must be valid) \param newData New value for the specified channel of all pixels.
			void						CopyChannel(const ImageTemplate<T>&source_image, BYTE source_channel, BYTE target_channel);
			void						PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r);
			void						PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g);
			void						PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g, T b);
			void						PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g, T b, T a);
			void						NoiseFillChannel(BYTE channel, T min_value, T max_value);		//!< Alters the content of one color channel of all pixels to random values \param channel Target channel (must be valid) \param min_value Minimum channel value @param max_value maximum channel value
			void						NoiseFillChannel(BYTE channel, T min_value, T max_value, unsigned seed);		//!< Alters the content of one color channel of all pixels to random values \param channel Target channel (must be valid) \param min_value Minimum channel value @param max_value maximum channel value
			void						BlurChannel(BYTE channel, float radius, bool loop);					//!< Blurs the specified image channel (all other channels are left unchanged) @param channel Channel to blur @param radius Radius controling the blur intensity @param loop Image is seamless and blurring should loop around image borders
			void						LinearBlurChannel(BYTE channel, float radius, bool loop);					//!< Blurs the specified image channel (all other channels are left unchanged). Separates horizontal from vertical blurring @param channel Channel to blur @param radius Radius controling the blur intensity @param loop Image is seamless and blurring should loop around image borders
			void						Fill(T red, T green, T blue, T alpha=TypeInfo<T>::zero);			//!< Fills the lower 4 (or less if less) channels of the local image with the specified values.
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least 3 entries
									
											set3() overwrites the local pixel data with the specified color ( @b data). The method does not check for correctness and assumes
											the local image has at least 3 channels.
											Corrupted x/y coordinates or insufficient entries in the \b data field will lead to access violations and/or segmentation faults.
										*/
			inline	void				set3(dimension_t x, dimension_t y, const T*data);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least 4 entries
									
											set3() overwrites the local pixel data with the specified color ( @b data). The method does not check for correctness and assumes
											the local image has at least 4 channels.
											Corrupted x/y coordinates or insufficient entries in the \b data field will lead to access violations and/or segmentation faults.
										*/
			inline	void				set4(dimension_t x, dimension_t y, const T*data);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least one entry per local channel
									
											Set() overwrites the local pixel data with the specified color (\b data). The method does not check for correctness.
											Corrupted x/y coordinates or insufficient entries in the \b data field will lead to access violations and/or segmentation faults.
										*/
			inline 	void				Set(dimension_t x, dimension_t y, const T*data);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least one entry per local channel
											\param channels Number of entries in the provided \b data array.
									
											Set() overwrites the local pixel data with the specified color (\b data). If \b channels is less than the local
											number of channels then the last entry in the \b data field will be repeated for the remaining channels.
											The method does not check for correctness. Corrupted x/y coordinates or insufficient entries in the \b data
											field will lead to access violations and/or segmentation faults.
										*/
			inline 	void				Set(dimension_t x, dimension_t y, const T*data, BYTE channels);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param red Red color component
											\param green Green color component
											\param blue Blue color component
											\param alpha Opacity color component 
									
											Set() overwrites the local pixel data with the specified color (\b red, \b green, \b blue, \b alpha). Color components that are not
											provided by the local image are ignored. Channels above 4 are left unchanged.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
			inline	void				Set(dimension_t x, dimension_t y, T red, T green, T blue, T alpha); 
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param red Red color component
											\param green Green color component
											\param blue Blue color component
									
											Set() overwrites the local pixel data with the specified color (\b red, \b green, \b blue). Color components that are not
											provided by the local image are ignored. Channels above 3 are left unchanged.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
			inline	void				Set(dimension_t x, dimension_t y, T red, T green, T blue); 

			float						SampleChannelAt(float x, float y, BYTE channel,bool loop)	const;
			T							SmoothSampleChannelAt(float x, float y, BYTE channel,bool loop)	const;
			inline	T*					get(dimension_t x, dimension_t y)														//! Retrieves a pointer to the color data of the specified pixel. Warning: The method behavior is undefined for invalid pixel coordinates. \param x x-coordinate of the pixel \param y y-coordinate of the pixel  \return Pointer to the color data of the specified pixel.
										{
											return image_data+(size_t(y)*size_t(image_width)+size_t(x))*size_t(image_channels);
										}
			inline	const T*			get(dimension_t x, dimension_t y)											const		//! Retrieves a pointer to the color data of the specified pixel. Warning: The method behavior is undefined for invalid pixel coordinates. \param x x-coordinate of the pixel \param y y-coordinate of the pixel  \return Pointer to the color data of the specified pixel.
										{
											return image_data+(size_t(y)*size_t(image_width)+size_t(x))*size_t(image_channels);
										}
			inline	T*					GetPixel(dimension_t x, dimension_t y)/**@copydoc get()*/	{return get(x,y);}
			inline	const T*			GetPixel(dimension_t x, dimension_t y) const /**@copydoc get()*/	{return get(x,y);}
			inline	T*					Get(dimension_t x, dimension_t y)/**@copydoc get()*/	{return get(x,y);}
			inline	const T*			Get(dimension_t x, dimension_t y) const /**@copydoc get()*/	{return get(x,y);}
			inline 	T*					GetVerified(dimension_t x, dimension_t y);											//!< Retrieves a pointer to the color data of the specified pixel. \param x x-coordinate of the pixel \param y y-coordinate of the pixel  \return Pointer to the color data of the specified pixel or NULL if the specified pixel does not exist.
			inline 	const T*			GetVerified(dimension_t x, dimension_t y)									const;		//!< Retrieves a pointer to the color data of the specified pixel. \param x x-coordinate of the pixel \param y y-coordinate of the pixel  \return Pointer to the color data of the specified pixel or NULL if the specified pixel does not exist.
			inline	T*					GetPixel(const X&x, const Y&y)/**@copydoc get()*/	{return get(*x,*y);}
			inline	const T*			GetPixel(const X&x, const Y&y) const /**@copydoc get()*/	{return get(*x,*y);}
			inline	T*					Get(const X&x, const Y&y)/**@copydoc get()*/	{return get(*x,*y);}
			inline	const T*			Get(const X&x, const Y&y) const /**@copydoc get()*/	{return get(*x,*y);}
			inline	T*					GetLinear(index_t idx)	{return image_data + idx * image_channels;}
			inline	const T*			GetLinear(index_t idx)const {return image_data + idx * image_channels;}
			void						SwapChannels(BYTE c0, BYTE c1);													//!< Exchanges the content of two channels for all pixels. \param c0 First channel index (0 = first(red) channel) \param c1 Second channel index (0 = first(red) channel).
			void						AppendAlpha(const Self*other);												//!< Creates/overwrites the local 4th channel with the first channel of the specified other image for all pixels. \param other Pointer to another Image object. \b other is required to be of the exact same dimensions as the local image.
			void						AppendAlphaAndDelete(Self*other);												//!< Performs AppendAlpha(), then deletes \b other. \param other Pointer to another Image object. \b other is required to be of the exact same dimensions as the local image but will be deleted even if that should not be the case.
			inline	size_t				size()																const;		//!< Retrieves the size of the local pixel data. \return Size of the local pixel map in bytes (identical to GetWidth()*GetHeight()*GetChannels()).
			void						readFrom(const Self*other);																	//!< Adapts the local image data to the specified image's data. Deprecated. \param other Image to copy data from.
			void						read(const T* data);																			//!< Adopts the local image data to the specified array content. \param data Ctr::Array to copy from. Must be the exact same size as what size() returns.
			/** 
			Exports a rectangular pixel area from the local pixel data.
			@param x Pixel offset (x). The method fails if this value is greater or equal to the local image width
			@param y Pixel offset (y). The method fails if this value is greater or equal to the local image height
			@param width Pixels to export in x-direction. The width will be clamped if it exceeds the valid range
			@param height Pixels to export in y-direction. The height will be clamped if it exceeds the valid range
			@param target Image to write to. Will be resized as necessary.
			@return true if the specified parameters were valid and any pixels were exported as a result. @a target will be resized to 0x0 otherwise.
			*/
			bool						ExportRectangle(dimension_t x, dimension_t y, dimension_t width, dimension_t height, Self&target)	const;
			bool						ExportRectangle(dimension_t x, dimension_t y, dimension_t width, dimension_t height, T*target)	const;	//!< Exports a rectangular pixel area from the local pixel data. \param x Pixel offset (x) \param y Pixel offset (y) \param width Pixels in x-direction to export \param height Pixels in y-direction to export. \param target Ctr::Array to write to. Must be at least (\b width * \b height * GetChannels()) elements long.
			bool						ImportRectangle(dimension_t x, dimension_t y, dimension_t width, dimension_t height, const T*target);	//!< Overwrites a section in the local pixel data. \param x Pixel offset (x) \param y Pixel offset (y) \param width Pixels in x-direction to overwrite \param height Pixels in y-direction to overwrite. \param target Ctr::Array to read from. Must be at least (\b width * \b height * GetChannels()) elements long.
			void						ExtractChannels(BYTE channel, BYTE c_num, Self&target);								//!< Extracts the specified channel range into the specified target image

			bool						TruncateToOpaque(bool IsOpaque(const T*));										//!< Reduces the image to the minimum necessary rectangle covering all opaque pixels. @return True if the local image has been changed, false otherwise
			bool						TruncateToOpaque();																	//!< Reduces the image to the minimum necessary rectangle covering all opaque pixels. This method has no effect if the image does not have 2 (intensity+opacity) or 4 (rgb+opacity) channels @return True if the local image has been changed, false otherwise

			bool						operator==(const Self&other) const;
			bool						operator!=(const Self&other) const {return !operator==(other);}
		};



	/*!
		\brief Generalized integer image nature
	
		An image using this nature stores 2d pixel maps with a variable number of unsigned integer color channels.
		Color channels are clamped to the range [0,max_channel_value] whenever required, where max_channel_value
		is the largest number the used type can store (e.g. 255/0xFF for unsigned char, 65535/0xFFFF for unsigned short).
		The parameter type <T> may be any integer data type but should be unsigned.
	*/
	template <typename T>
		struct TIntImageNature
		{
				typedef float			float_type_t;
				typedef	T				channel_value_t;
		static const T					max_channel_value = TypeInfo<T>::max,
										normal_offset = (max_channel_value>>1)+1,
										normal_scale = normal_offset-1;

		/**
			@brief Converts a channel value to float
		*/
		static inline	float			toFloat(T v)
										{
											return float(v)/float(max_channel_value);
										}
		/**
			@brief Converts a float to a channel value
		*/
		template <typename Float>
		static inline	T				floatToChannel(Float v)
										{
											return T(v*Float(max_channel_value));
										}
		/**
			@brief Reads a normal component from a channel value
		*/
		static inline	float			toNormalComponent(T v)
										{
											return (float(v)-float(normal_offset))/float(normal_scale);
										}
		/**
			@brief Converts a normal component to a channel value
		*/
		template <typename Float>
		static inline	T				normalComponentToChannel(Float v)
										{
											return T(normal_offset+v*normal_scale);
										}
		/**
			@brief Clamps a generic (typically float) value to the allowed value range and returns a valid channel value
		*/
		template <typename Generic>
		static inline	T				clamp(Generic v)
										{
											return T(Math::clamp(v,0,max_channel_value));
										}


		};


	/*!
		\brief Generalized floating point image nature
	
		An image using this nature stores 2d pixel maps with a variable number of floating point color channels.
		Colors channels are clamped to the range [0,1] whenever required.
		The parameter type <T> may be any float data type (float, double, long double).
	*/
	template <typename T>
		struct TFloatImageNature
		{
		public:
				typedef T				float_type_t;
				typedef	T				channel_value_t;
		static const T					max_channel_value,
										normal_offset,
										normal_scale;

		static inline	T				toFloat(T v)
										{
											return v;
										}
		template <typename Float>
		static inline	T				floatToChannel(Float v)
										{
											return v;
										}
		static inline	T				toNormalComponent(T v)
										{
											return ((v)-(normal_offset))/(normal_scale);
										}
		template <typename Float>
		static inline	T				normalComponentToChannel(Float v)
										{
											return (normal_offset+v*normal_scale);
										}
		template <typename Generic>
		static inline	T				clamp(Generic v)
										{
											return T(Math::clamp(v,0,1));
										}

		};

	/*!
		\brief Generalized unclamped floating point image nature
	
		An image using this nature stores 2d pixel maps with a variable number of floating point color channels.
		This nature does not clamp values to the range [0,1] during certain operations.
	*/
	template <typename T>
		struct TUnclampedFloatImageNature:public TFloatImageNature<T>
		{
		public:
		static inline	T				toNormalComponent(T v)
										{
											return v;
										}
		template <typename Float>
		static inline	T				normalComponentToChannel(Float v)
										{
											return v;
										}
		template <typename Generic>
		static inline	T				clamp(Generic v)
										{
											return v;
										}

		};	

	template <typename T>
		const T	TFloatImageNature<T>::max_channel_value = T(1);
	template <typename T>
		const T	TFloatImageNature<T>::normal_offset = T(0.5);
	template <typename T>
		const T	TFloatImageNature<T>::normal_scale = T(0.5);

	/**
		@brief Generic image class using a nature template class to specify how to handle
		contained pixel values.

		A nature must specify the following members to be usable by GenericImage:<br>
		<br>
				typedef ...					float_type_t;							//floating point variable type that can properly represent a channel value in floating point space. e.g. float<br>
				typedef	...					channel_value_t;						//data variable type. this type is used to store channel values. e.g. unsigned char<br>
		static const T						max_channel_value = ...;				//maximum channel value. e.g. 255<br>
		<br>
		<br>
		static inline	float_type_t		toFloat(channel_value_t v);	//converts a channel value to float<br>
		template &lt;typename Float&gt;<br>
		static inline	channel_value_t		floatToChannel(Float v);	//converts a float value into a channel value<br>
		static inline	float_type_t		toNormalComponent(channel_value_t v);		//converts a channel value into a normal component (x, y, or z)<br>
		template &lt;typename Float&gt;<br>
		static inline	channel_value_t		normalComponentToChannel(Float v);	//converts a normal component into a channel value<br>
		template &lt;typename Generic&gt;<br>
		static inline	channel_value_t		clamp(Generic v);		//clamps a generic value to the allowed range and returns it as a channel value. The nature may choose not to clamp<br>
		<br>
		<br>
		Predefined natures are TIntImageNature<...>, TFloatImageNature<...>, and TUnclampedFloatImageNature<...>

	*/
	template <class Nature>
		class GenericImage:public ImageTemplate<typename Nature::channel_value_t>, public Nature
		{
		protected:
			typedef ImageTemplate<typename Nature::channel_value_t>	Super;
			typedef size_t	Size;
			typedef Nature	ImageNature;

			template <typename Float>
				void					sampleNormal(Float x0, Float y0, Float x1, Float y1, M::TVec3<Float>&target)		const;

			
		public:
			typedef ImageTemplate<typename Nature::channel_value_t>	Parent;
			typedef GenericImage<Nature>	Self;
			typedef typename Super::dimension_t	dimension_t;
		
			using Super::IsColorMap;
			using Super::IsNormalMap;
			using Super::free;
			using Super::GetPixel;
			using Super::adoptData;
			using Super::sample;
			using Super::Set;
			//#ifdef __GNUC__
				using Parent::image_channels;
				using Parent::image_width;
				using Parent::image_height;
				using Parent::image_data;
				using Parent::origin;
				using Parent::content_type;
				using Parent::get;

				using typename Parent::combiner_t;


				using Parent::IC_COPY;
				using Parent::IC_ALPHA_BLEND;
				using Parent::IC_ALPHA_ADD;
				using Parent::IC_ADD;
				using Parent::IC_MULTIPLY;
				using Parent::IC_MULT_NEGATIVE;
			
			//#endif

				typedef typename Nature::float_type_t			F;
				typedef typename Nature::channel_value_t			T;
				static const T max;


										GenericImage()	{};
										GenericImage(dimension_t width, dimension_t height, BYTE channels=3,PixelType type=PixelType::Color):ImageTemplate<T>(width,height,channels,type)
										{}
										GenericImage(const BaseImage::THeader&header):ImageTemplate<T>(header)
										{}
			
				void					ScaleChannel(BYTE channel, F factor);							//!< Magnifies or shrinks the specified channel value of all pixels
				void					ScaleChannel(BYTE channel, F offset, F factor);							//!< Magnifies or shrinks the specified channel value of all pixels
				void					FractalDouble(F noise_level, bool loop);									//!< Resizes the image to double its size (minus 1 (width/height*2-1) pixels if @b loop is not set, plain double otherwise) and fills the pixels inbetween with fractal noise @param noise_level Degree of randomness to apply to intermediate pixels @param loop Set true to interpolate edge pixels using original pixel data from the opposite edge
				void					FractalDouble(F noise_level, bool loop, unsigned seed);									//!< Resizes the image to double its size (minus 1 (width/height*2-1) pixels if @b loop is not set, plain double otherwise) and fills the pixels inbetween with fractal noise @param noise_level Degree of randomness to apply to intermediate pixels @param loop Set true to interpolate edge pixels using original pixel data from the opposite edge
				void					FlipHorizontally();																//!< Flips the local pixel data horizontally. Also flips the x-coordinate of normals if the local image is a normal map.
				void					FlipVertically();																	//!< Flips the local pixel data vertically. Also flips the y-coordinate of normals if the local image is a normal map.
				void					Rotate90CW();																	//!< Rotates the local image 90 degrees clock wise.
				void					Rotate90CCW();																	//!< Rotates the local image 90 degrees counter clock wise.
				bool					ScaleHalf();																	//!< Scales the image to half its width and height. Normals will be renormalized if necessary. \return true if the local image could be halved, false otherwise.
				bool					ScaleHalf(const Self&source);													//!< Scales the suppied image to half its width and height. Normals will be renormalized if necessary. \return true if the local image could be halved, false otherwise.
				bool					ScaleXHalf();																	//!< Scales the image to half its width. Normals will be renormalized if necessary. \return true if the local image could be halved, false otherwise.
				bool					ScaleYHalf();																	//!< Scales the image to half its height. Normals will be renormalized if necessary. \return true if the local image could be halved, false otherwise.
				void					ScaleDouble(bool loop=false);																	//!< Scales the image to double its width and height. Normals will be renormalized if necessary.
				void					ScaleXDouble(bool loop=false);																	//!< Scales the image to double its width. Normals will be renormalized if necessary.
				void					ScaleYDouble(bool loop=false);																	//!< Scales the image to double its height. Normals will be renormalized if necessary.
				bool					ScaleTo(dimension_t width, dimension_t height);										//!< Scales the image closest to the specified width and height via doubling and/or halving. Normals will be renormalized if necessary. \param width Image target width \param height Image target height \return true if the resulting image is exactly the specified size, false otherwise.
				void					Resample(dimension_t width, dimension_t height);										//!< Resizes the image using linear interpolation. Normals will be renormalized if necessary. The method in general behaves differently when resizing normal maps. \param width Image target width \param height Image target height
				void					Crop(dimension_t offset_x, dimension_t offset_y, dimension_t width, dimension_t height);	//!< Crops the local image to the specified region
				void					Truncate(dimension_t width, dimension_t height);										//!< Truncates the image to not be larger than the specified dimensions. The method has no effect if the picture is already that small or smaller
				void					ChangeSaturation(float scale_by);														//!< Alters the saturation among the first three channels of the local image
				void					AddRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g, T b);
				void					Power(BYTE channel, float exponent);																	//!< Apply the specified exponent too all pixels (only the specified channel, however)
										/*!
											\brief Draws a remote image into the pixel data of the local image
											\param other Image to draw
											\param combiner Draw rule

											Draw() draws the content of the specified other image into the local image content.
											\a other may be larger than the remaining pixels in width and height direction
											relative to \a x_offset and \a y_offset.
										*/
				void					Draw(const ImageTemplate<T>*other, dimension_t x_offset, dimension_t y_offset, BaseImage::combiner_t combiner=BaseImage::IC_ALPHA_BLEND);
				void					Paint(const ImageTemplate<T>*other, dimension_t x_offset, dimension_t y_offset, BaseImage::combiner_t combiner=BaseImage::IC_ALPHA_BLEND);	//!< Identical to Draw()
			template <class Nature1>
				void					ToNormalMap(GenericImage<Nature1>&target, F intensity, bool seamless, BYTE height_channel=0)		const;		//!< Generates a normal map from the local color map. Size and type of the specified target image will be adjusted. @param target Image container to write the resulting normal map to @param intensity Intensity of the resulting normal map @param seamless True if border pixels should loop around to determine a seamless normal map @param height_channel Color channel to use as height value to calculate the normals from
				/**
										@brief Generates a normal map from the local color map

										@param target Image to store the final normal map in
										@param x_texel_distance Geometrical distance between two neighboring texels on the x axis (that share the same y coordinate)
										@param y_texel_distance Geometrical distance between two neighboring texels on the y axis (that share the same x coordinate)
										@param height_scale Geometrical variance to be applied to channel values before calculating a texel's normal
										@param seamless True if the left image edge should connect to the right and the bottom the top one, false otherwise
										@param height_channel Channel that should be used to retrieve the 'height' of a texel
				*/
			template <class Nature1>
				void					ToNormalMap(GenericImage<Nature1>&target, F x_texel_distance, F y_texel_distance, F height_scale, bool seamless, BYTE height_channel=0)		const;


										/*!
											\brief Overwrites the color of a specific pixel with the specified floating point color
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least 3 entries
									
											set3f() overwrites the local pixel data with the specified color ( @b data). The method does not check for correctness and assumes
											the local image has at least 3 channels. Color values are expected in the range [0,1] and stretched the range of the local channel data type.
											Corrupted x/y coordinates or insufficient entries in the \b data field will lead to access violations and/or segmentation faults.
										*/
		template <typename Float>
		inline	void					set3f(dimension_t x, dimension_t y, const Float data[3]);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least 4 entries
									
											set3() overwrites the local pixel data with the specified color ( @b data). The method does not check for correctness and assumes
											the local image has at least 4 channels. Color values are expected in the range [0,1] and stretched the range of the local channel data type.
											Corrupted x/y coordinates or insufficient entries in the \b data field will lead to access violations and/or segmentation faults.
										*/
		template <typename Float>
		inline	void					set4f(dimension_t x, dimension_t y, const Float data[4]);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least one entry per local channel
									
											Set() overwrites the local pixel data with the specified color (\b data). The method does not check for correctness.
											Color values are expected in the range [0,1] and stretched the range of the local channel data type.
											Corrupted x/y coordinates or insufficient entries in the \b data field will lead to access violations and/or segmentation faults.
										*/
		template <typename Float>
		inline 	void					setf(dimension_t x, dimension_t y, const Float*data);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param data Pointer to an array providing at least one entry per local channel
											\param channels Number of entries in the provided \b data array.
									
											Set() overwrites the local pixel data with the specified color (\b data). If \b channels is less than the local
											number of channels then the last entry in the \b data field will be repeated for the remaining channels.
											Color values are expected in the range [0,1] and stretched the range of the local channel data type.
											The method does not check for correctness. Corrupted x/y coordinates or insufficient entries in the \b data
											field will lead to access violations and/or segmentation faults.
										*/
		template <typename Float>
		inline 	void					setf(dimension_t x, dimension_t y, const Float*data, BYTE channels);
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param red Red color component
											\param green Green color component
											\param blue Blue color component
											\param alpha Opacity color component 
									
											Set() overwrites the local pixel data with the specified color (\b red, \b green, \b blue, \b alpha). Color components that are not
											provided by the local image are ignored. Channels above 4 are left unchanged. Color values are expected in the range [0,1] and stretched the range of the local channel data type.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
		template <typename Float>
		inline	void					setf(dimension_t x, dimension_t y, Float red, Float green, Float blue, Float alpha); 
										/*!
											\brief Overwrites the color of a specific pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel
											\param red Red color component
											\param green Green color component
											\param blue Blue color component
									
											Set() overwrites the local pixel data with the specified color (\b red, \b green, \b blue). Color components that are not
											provided by the local image are ignored. Channels above 3 are left unchanged.
											Color values are expected in the range [0,1] and stretched the range of the local channel data type.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
		template <typename Float>
		inline	void					setf(dimension_t x, dimension_t y, Float red, Float green, Float blue); 




										/*!
											\brief Overwrites the color of a specific pixel with a normal.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel 
											\param data Pointer to a 3 element float normal vector
									
											setNormal() transforms the specified normal to a rgb color vector (-1.0f .. 1.0f will be translated to normal_offset+p*normal_scale).
											The final color is undefined if one of the normal components exceeds the range [-1,+1]. The method does not check for parameter validity and may behave
											unexpectedly or even crash if the contained data is not a 3 channel normal map or if the specified coordinates are invalid.
										*/
			template <typename Float>
				void					setNormal(dimension_t x, dimension_t y, const M::TVec3<Float>&data); //-1.0f .. 1.0f will be translated to normal_offset+p*normal_scale

										/*!
											\brief Modifies the color of the specified pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel 
											\param data Pointer to an array providing at least one entry per local channel 
											\param alpha Opacity of the specified color.
									
											blend() blends the specified color above the local pixel data.  \alpha determines
											the opacity of the specified color (0 = base pixel remains unchanged, up to 255 = base pixel will be replaced by the specified color).
											Any values between 0 and 255 will cause a crossfade between the base pixel color and the specified color.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
				void					blend(dimension_t x, dimension_t y, const T*data,T alpha);
										/*!
											\brief Modifies the color of the specified pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel 
											\param data Pointer to an array providing at least \b channels entries.
											\param alpha Opacity of the specified color.
											\param channels Number of entries in the provided \b data array.
									
											blend() blends the specified color above the local pixel data.  The first \b channels are blended,
											the remaining ignored. \alpha determines the opacity of the specified color (0 = base pixel remains
											unchanged, up to 255 = base pixel will be replaced by the specified color).
											Any values between 0 and 255 will cause a crossfade between the base pixel color and the specified color.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
				void					blend(dimension_t x, dimension_t y, const T*data,T alpha, BYTE channels);
										/*!
											\brief Modifies the color of the specified pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel 
											\param data Pointer to a float array providing at least one entry per local channel 
											\param alpha Opacity of the specified color.
									
											blend() blends the specified color above the local pixel data.  The float values in the specified float array will be
											translated from 0.0 = 0 to 1.0 = 255. \alpha determines the opacity of the specified color (0.0 = base pixel remains
											unchanged, up to 1.0 = base pixel will be replaced by the specified color).
											Any values between 0 and 1 will cause a crossfade between the base pixel color and the specified color.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
			template <typename Float>
				void					BlendF(dimension_t x, dimension_t y, const Float*data, Float alpha);
										/*!
											\brief Modifies the color of the specified pixel.
											\param x x-coordinate of the pixel
											\param y y-coordinate of the pixel 
											\param data Pointer to an array providing at least \b channels entries.
											\param alpha Opacity of the specified color.
											\param channels Number of entries in the provided \b data array.
									
											blend() blends the specified color above the local pixel data.  The float values in the specified float array will be
											translated from 0.0 = 0 to 1.0 = 255. The first \b channels are blended, the remaining ignored.
											\alpha determines the opacity of the specified color (0.0 = base pixel remains unchanged, up to 1.0 = base pixel
											will be replaced by the specified color).
											Any values between 0 and 1 will cause a crossfade between the base pixel color and the specified color.
											The method does not check for correctness. Corrupted x/y coordinates will lead to access violations and/or
											segmentation faults.
										*/
			template <typename Float>
				void					BlendF(dimension_t x, dimension_t y, const Float*data, Float alpha, BYTE channels);
			template <typename Float>
				void					GetNormal(dimension_t x, dimension_t y, M::TVec3<Float>&out)						const;		//!< Retrieves the normal content of a pixel. The method behavior is undefined for invalid pixel coordinates. \param x x-coordinate of the pixel \param y y-coordinate of the pixel  \param out Normal output field
			template <typename Float>
				bool					GetNormalVerified(dimension_t x, dimension_t y, M::TVec3<Float>&out)				const;		//!< Retrieves the normal content of a pixel. \param x x-coordinate of the pixel \param y y-coordinate of the pixel  \param out Normal output field \return true if the specified pixel exists and a normal could be extracted, false otherwise.
			template <typename Float>
				bool					ImportRectangleF(dimension_t x, dimension_t y, dimension_t w, dimension_t h, const Float*target);
			template <typename Float>
				bool					ExportRectangleF(dimension_t x, dimension_t y, dimension_t w, dimension_t h, Float*target) const;
				void					SetChannels(BYTE new_channels,BaseImage::channel_append_t append = BaseImage::CA_REPEAT);			//!< Alters the number of channels in the local image. \param new_channels New number of color channels. \param append Action to perform if the new number of channels is greater than the old one.



		};

	template <class Nature>
		const typename GenericImage<Nature>::T GenericImage<Nature>::max = Nature::max_channel_value;

	typedef GenericImage<TIntImageNature<BYTE> >				Image;					//!< Standard 8bpc integer image. Smallest, least precise image type supported
	typedef GenericImage<TIntImageNature<UINT16> >				Image16;				//!< 16bpc image. Very precise integer image type
	typedef GenericImage<TFloatImageNature<float> >				FloatImage;			//!< 32bpc floating point image.
	typedef GenericImage<TUnclampedFloatImageNature<float> >	UnclampedFloatImage;	//!< 32bpc floating point image. This image does not clamp channel values or remap for normal extraction/storage.



	template <class Nature>
		bool	alignableImages(const GenericImage<Nature>*field, unsigned cnt);	//!< checks whether or not aligning is necessary. Aligning is the process of making a number of images equal in channel count and dimensions @param field Pointer to the first image to check. @param cnt Number of images in the field @return true if aligning is required
	template <class Nature>
		bool	alignableImages(const GenericImage<Nature>**field, unsigned cnt);	//!< @overload

	template <class Nature>
		void	alignImages(GenericImage<Nature>*field, unsigned cnt);		//!< aligns images' sizes and channels to the detected field-maximum. @param field Pointer to the first image to align @param cnt Number of images in the field
	template <class Nature>
		void	alignImages(GenericImage<Nature>**field, unsigned cnt);	//!< @overload



	#include "image.tpl.h"
}

#endif
