#ifndef imageTplH
#define imageTplH



#undef _TYPENAME
#ifdef _MSC_VER
	#define _TYPENAME		//visual studio insists that lambda function bodies are not templates themselves even if their parent methods are, and typename is thus misplaced
#else
	#define _TYPENAME	typename		//clang, on the other hand, insists that they are template methods, and thus typename is required
#endif


template <class Nature>
	template <typename Float>
		void		GenericImage<Nature>::sampleNormal(Float x0, Float y0, Float x1, Float y1, M::TVec3<Float>&target)		const
		{
			T	sampled[0x100];
			ImageTemplate<T>::sample(x0,y0,x1,y1,sampled);

			target.x = Nature::toNormalComponent(sampled[0]);
			target.y = Nature::toNormalComponent(sampled[1]);
			target.z = Nature::toNormalComponent(sampled[2]);

			M::Vec::normalize0(target);
		}

template <typename T>
	template <typename Float>
		void		ImageTemplate<T>::sample(Float x0, Float y0, Float x1, Float y1, T*out)		const
		{
			float   pwidth = Float(1)/Float(image_width),
					pheight = Float(1)/Float(image_height);
		/*    x0-=pwidth/2;
			y0-=pheight/2;
			x1-=pwidth/2;
			y1-=pheight/2;*/
			#ifdef COS_RESIZE
				float   xp0 = M::CubicFactor(1-fmod(x0,pwidth)/pwidth),
						yp0 = M::CubicFactor(1-fmod(y0,pheight)/pheight),
						xp1 = M::CubicFactor(fmod(x1,pwidth)/pwidth),
						yp1 = M::CubicFactor(fmod(y1,pheight)/pheight);
			#else
				float   xp0 = 1-fmod(x0,pwidth)/pwidth,
						yp0 = 1-fmod(y0,pheight)/pheight,
						xp1 = fmod(x1,pwidth)/pwidth,
						yp1 = fmod(y1,pheight)/pheight;
			#endif
			int     px0 = (int)(x0*(image_width)),
					py0 = (int)(y0*(image_height)),
					px1 = (int)(x1*(image_width)),
					py1 = (int)(y1*(image_height));

			if (px0 == px1)
			{
				px1++;
				if (xp0<GLOBAL_ERROR)
					xp0 = 1-xp1;
			}
			else
				if (x1-x0<=pwidth)
					xp1 = 1-xp0;
			if (py0 == py1)
			{
				py1++;
				if (yp0<GLOBAL_ERROR)
					yp0 = 1-yp1;
			}
			else
				if (y1-y0<=pheight)
					yp1 = 1-yp0;


			Float   count(0),target[0x100];
			M::Vec::clearD(target,image_channels);

			for (int x = px0+1; x < px1; x++)
				for (int y = py0+1; y < py1; y++)
					if (const T*pixel = GetVerified(x,y))
					{
						M::Vec::addD(target,pixel,image_channels);
						count++;
					}
			for (int x = px0+1; x < px1; x++)
			{
				const T	*p0 = GetVerified(x,py0),
						*p1 = GetVerified(x,py1);
				for (BYTE k = 0; k < image_channels; k++)
				{
					if (p0)
						target[k] += yp0*p0[k];
					if (p1)
						target[k] += yp1*p1[k];
				}
				if (p0)
					count += yp0;
				if (p1)
					count += yp1;
			}
			for (int y = py0+1; y < py1; y++)
			{
				const T	*p0 = GetVerified(px0,y),
						*p1 = GetVerified(px1,y);
				for (BYTE k = 0; k < image_channels; k++)
				{
					if (p0)
						target[k] += xp0*p0[k];
					if (p1)
						target[k] += xp1*p1[k];
				}
				if (p0)
					count += xp0;
				if (p1)
					count += xp1;
			}
			const T	*p0 = GetVerified(px0,py0),
					*p1 = GetVerified(px0,py1),
					*p2 = GetVerified(px1,py1),
					*p3 = GetVerified(px1,py0);

			for (BYTE k = 0; k < image_channels; k++)
			{
				if (p0)
					target[k] += xp0*yp0*p0[k];
				if (p1)
					target[k] += xp0*yp1*p1[k];
				if (p2)
					target[k] += xp1*yp1*p2[k];
				if (p3)
					target[k] += xp1*yp0*p3[k];
			}
			if (p0)
				count += xp0*yp0;
			if (p1)
				count += xp0*yp1;
			if (p2)
				count += xp1*yp1;
			if (p3)
				count += xp1*yp0;
			M::Vec::divideD(target,count,image_channels);
			M::Vec::copyD(target,out,image_channels);

		}


template <typename T>
	ImageTemplate<T>::ImageTemplate():image_data(NULL)
	{}

template <typename T>
	ImageTemplate<T>::ImageTemplate(const ImageTemplate<T>&other):image_data(NULL)
	{
		SetSize(other.image_width,other.image_height,other.image_channels);
		memcpy(image_data,other.image_data,other.size());
	}

template <typename T>
	ImageTemplate<T>::ImageTemplate(dimension_t width, dimension_t height, BYTE channels,PixelType type):image_data(NULL)
	{
		SetSize(width,height,channels);
		content_type = type;
	}

template <typename T>
	ImageTemplate<T>::ImageTemplate(const THeader&header):BaseImage(0x1<<header.x_exp,0x1<<header.y_exp,header.channels,GetContentType(header)),image_data(NULL)
	{
		alloc(image_data,image_width*image_height*image_channels);
	}

template <typename T>
	ImageTemplate<T>::~ImageTemplate()
	{
		dealloc(image_data);
	}

template <typename T>
	ImageTemplate<T>&		ImageTemplate<T>::operator=(const ImageTemplate<T>&other)
	{
		content_type = other.content_type;
		origin = other.origin;
		SetSize(other.image_width,other.image_height,other.image_channels);
		memcpy(image_data,other.image_data,other.size());
		return *this;
	}



template <typename T>
	void		ImageTemplate<T>::applyHeader(const THeader&header)
	{
		SetSize(0x1<<header.x_exp,0x1<<header.y_exp,header.channels);
		SetContentType(GetContentType(header));
	}


template <typename T>
	inline	T*			ImageTemplate<T>::data()
	{
		return image_data;
	}

template <typename T>
	inline	const T*	ImageTemplate<T>::data() const
	{
		return image_data;
	}

template <typename T>
	void		ImageTemplate<T>::adoptData(ImageTemplate<T>&other)
	{
		dealloc(image_data);
	
		content_type = other.content_type;
	
		origin.adoptData(other.origin);
	
		image_width = other.image_width;
		image_height = other.image_height;
		image_channels = other.image_channels;
		image_data = other.image_data;
	
		other.image_width = 0;
		other.image_height = 0;
		other.image_data = NULL;
	}
template <typename T>
	void		ImageTemplate<T>::swap(ImageTemplate<T>&other)
	{
		origin.swap(other.origin);
	
		swp(content_type,other.content_type);
		swp(image_width,other.image_width);
		swp(image_height,other.image_height);
		swp(image_channels,other.image_channels);
		swp(image_data,other.image_data);
	}
template <typename T>
	bool		ImageTemplate<T>::operator==(const Self&other) const
	{
		return Super::operator==(other) && memcmp(image_data,other.image_data,size()) == 0;
	}
	

template <typename T>
	void		ImageTemplate<T>::SetSize(dimension_t width, dimension_t height, BYTE channels)
	{
		if (width*height*channels != image_width*image_height*image_channels)
			re_alloc(image_data,width*height*channels);
		image_width = width;
		image_height = height;
		image_channels = channels;
	}

template <typename T>
	inline void		ImageTemplate<T>::SetChannel(dimension_t X, dimension_t Y, BYTE channel, T newData)
	{
		image_data[Y*image_width*image_channels+X*image_channels+channel] = newData;
	}

template <typename T>
	void		ImageTemplate<T>::SetChannel(BYTE channel, T new_value)
	{
		if (channel >= image_channels)
			return;
		Concurrency::parallel_for(dimension_t(0),image_width,[this,channel,new_value](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
				SetChannel(x,y,channel,new_value);
		});
	}

template <typename T>
	void		ImageTemplate<T>::PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r)
	{
		if (image_channels != 1)
			return;
		for (dimension_t x = left; x < left+width; x++)
			for (dimension_t y = bottom; y < bottom + height; y++)
			{
				if (x < image_width && y < image_height)
				{
					T*pixel = get(x,y);
					pixel[0] = r;
				}
			}
	}

template <typename T>
	void		ImageTemplate<T>::PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g)
	{
		switch (image_channels)
		{
			case 1:
				PaintRect(left,bottom,width,height,r);
			break;
		}
		if (image_channels != 2)
			return;
		for (dimension_t x = left; x < left+width; x++)
			for (dimension_t y = bottom; y < bottom + height; y++)
			{
				if (x < image_width && y < image_height)
				{
					T*pixel = get(x,y);
					pixel[0] = r;
					pixel[1] = g;
				}
			}
	}


template <typename T>
	void		ImageTemplate<T>::PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g, T b)
	{
		switch (image_channels)
		{
			case 1:
				PaintRect(left,bottom,width,height,r);
			break;
			case 2:
				PaintRect(left,bottom,width,height,r,g);
			break;
		}
		if (image_channels != 3)
			return;
		for (dimension_t x = left; x < left+width; x++)
			for (dimension_t y = bottom; y < bottom + height; y++)
			{
				if (x < image_width && y < image_height)
				{
					T*pixel = get(x,y);
					pixel[0] = r;
					pixel[1] = g;
					pixel[2] = b;
				}
			}

	}

template <typename T>
	void		ImageTemplate<T>::PaintRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g, T b, T a)
	{
		switch (image_channels)
		{
			case 1:
				PaintRect(left,bottom,width,height,r);
			break;
			case 2:
				PaintRect(left,bottom,width,height,r,g);
			break;
			case 3:
				PaintRect(left,bottom,width,height,r,g,b);
			break;
		}
		if (image_channels != 4)
			return;
		for (dimension_t x = left; x < left+width; x++)
			for (dimension_t y = bottom; y < bottom + height; y++)
			{
				if (x < image_width && y < image_height)
				{
					T*pixel = get(x,y);
					pixel[0] = r;
					pixel[1] = g;
					pixel[2] = b;
					pixel[3] = a;
				}
			}

	}
template <class Nature>
	void		GenericImage<Nature>::AddRect(dimension_t left, dimension_t bottom, dimension_t width, dimension_t height, T r, T g, T b)
	{
		if (image_channels != 3)
			return;
		Concurrency::parallel_for(left,left+width,[this,bottom,height,r,g,b](dimension_t x)
		{
			for (dimension_t y = bottom; y < bottom + height; y++)
			{
				if (x < image_width && y < image_height)
				{
					T*pixel = this->get(x,y);
					
					pixel[0] = GenericImage<Nature>::ImageNature::clamp((F)pixel[0] + r);
					pixel[1] = GenericImage<Nature>::ImageNature::clamp((F)pixel[1] + g);
					pixel[2] = GenericImage<Nature>::ImageNature::clamp((F)pixel[2] + b);
				}
			}
		});
	}


template <typename T>
	void		ImageTemplate<T>::NoiseFillChannel(BYTE channel, T min_value, T max_value)
	{
		NoiseFillChannel(channel,min_value,max_value,rand());
	}
template <typename T>
	void		ImageTemplate<T>::NoiseFillChannel(BYTE channel, T min_value, T max_value, unsigned seed)
	{
		if (channel >= image_channels)
			return;
		Concurrency::parallel_for(dimension_t(0),image_width,[this,channel,seed,min_value,max_value](dimension_t x)
		{
			ImageRandomSource rnd((unsigned long)(seed+x*29));
			for (dimension_t y = 0; y < image_height; y++)
			{
				T value = min_value+T(rnd()*(max_value-min_value));
				SetChannel(x,y,channel,value);
			}
		});
	}



template <typename T>
	void		ImageTemplate<T>::BlurChannel(BYTE channel, float radius, bool loop)
	{
		if (channel >= image_channels || radius <= 0)
			return;
		Array2D<T>	channels(image_width,image_height);
	
		int steps = (int)ceil(radius);
		Concurrency::parallel_for(INT32(0),INT32(image_width),[this,steps,loop,radius,channel,&channels](INT32 x)
		{
			for (INT32 y = 0; y < (INT32)image_height; y++)
			{
				float sum = 0;
				float counter = 0;
				for (INT32 ix = -steps; ix <= steps; ix++)
					for (INT32 iy = -steps; iy <= steps; iy++)
					{
						INT32 ax = x+ix;
						INT32 ay = y+iy;
						if (!loop)
						{
							if (ax < 0 || ay < 0 || ax >= (INT32)image_width || ay >= (INT32)image_height)
								continue;
						}
						else
						{
							while (ax < 0)
								ax += image_width;
							while (ay < 0)
								ay += image_height;
							while (ax >= (INT32)image_width)
								ax -= image_width;
							while (ay >= (INT32)image_height)
								ay -= image_height;
						}
					
				
				
						float intensity = 1.0f-(float)((float)ix*ix+(float)iy*iy)/(radius*radius);
					
						if (intensity <= 0)
							continue;
					
						counter += intensity;
						sum += intensity*(float)GetPixel(ax,ay)[channel];
					}
				ASSERT__(counter > 0);
				channels.Get(x,y) = T(sum/counter);
			}
		});
		Concurrency::parallel_for(dimension_t(0),image_width,[this,&channels,channel](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
				GetPixel(x,y)[channel] = channels.Get(x,y);
		});
	}


template <typename T>
	void		ImageTemplate<T>::LinearBlurChannel(BYTE channel, float radius, bool loop)
	{
		if (channel >= image_channels || radius <= 0)
			return;
		Array2D<T>	channels(image_width,image_height);
	
		int steps = (int)ceil(radius);
		Concurrency::parallel_for(INT32(0),INT32(image_width),[this,steps,loop,radius,channel,&channels](INT32 x)
		{
			for (INT32 y = 0; y < (INT32)image_height; y++)
			{
				float sum = 0;
				float counter = 0;
				for (INT32 ix = -steps; ix <= steps; ix++)
					{
						INT32 ax = x+ix;
						if (!loop)
						{
							if (ax < 0 || ax >= (INT32)image_width)
								continue;
						}
						else
						{
							while (ax < 0)
								ax += image_width;
							ax %= (INT32)image_width;
						}
					
						float intensity = 1.0f-(float)((float)ix)/(radius);
					
						if (intensity <= 0)
							continue;
					
						counter += intensity;
						sum += intensity*(float)GetPixel(ax,y)[channel];
					}
				ASSERT__(counter > 0);
				channels.get(x,y) = T(sum/counter);
			}
		});
		Concurrency::parallel_for(INT32(0),INT32(image_width),[this,steps,loop,radius,channel,&channels](INT32 x)
		{
			for (INT32 y = 0; y < (INT32)image_height; y++)
			{
				float sum = 0;
				float counter = 0;
				for (INT32 iy = -steps; iy <= steps; iy++)
					{
						INT32 ay = y+iy;
						if (!loop)
						{
							if (ay < 0 || ay >= (INT32)image_height)
								continue;
						}
						else
						{
							while (ay < 0)
								ay += image_height;
							ay %= (INT32)image_height;
						}
					
						float intensity = 1.0f-(float)((float)iy)/(radius);
					
						if (intensity <= 0)
							continue;
					
						counter += intensity;
						sum += intensity*(float)channels.get(x,ay);
					}
				ASSERT__(counter > 0);
				GetPixel(x,y)[channel] = T(sum/counter);
			}
		});
	}


template <typename T>
	void		ImageTemplate<T>::Fill(T red, T green, T blue, T alpha)
	{
		size_t pixels = size_t(image_width)*size_t(image_height);
		switch (image_channels)
		{
			case 0:
			break;
			case 1:
				Concurrency::parallel_for(index_t(0),pixels,[this,red](index_t i)
				{
					image_data[i] = red;
				});
			break;
			case 2:
				Concurrency::parallel_for(index_t(0),pixels,[this,red,green](index_t i)
				{
					image_data[i*2] = red;
					image_data[i*2+1] = green;
				});
			break;
			case 3:
				Concurrency::parallel_for(index_t(0),pixels,[this,red,green,blue](index_t i)
				{
					image_data[i*3] = red;
					image_data[i*3+1] = green;
					image_data[i*3+2] = blue;
				});
			break;
			case 4:
				Concurrency::parallel_for(index_t(0),pixels,[this,red,green,blue,alpha](index_t i)
				{
					image_data[i*4] = red;
					image_data[i*4+1] = green;
					image_data[i*4+2] = blue;
					image_data[i*4+3] = alpha;
				});
			break;
			default:
				Concurrency::parallel_for(index_t(0),pixels,[this,red,green,blue,alpha](index_t i)
				{
					image_data[i*image_channels] = red;
					image_data[i*image_channels+1] = green;
					image_data[i*image_channels+2] = blue;
					image_data[i*image_channels+3] = alpha;
				});
			break;
		}
	}

template <class Nature>
	void		GenericImage<Nature>::SetChannels(BYTE new_channels,BaseImage::channel_append_t append)
	{
		if (!new_channels || new_channels==image_channels)
			return;
		if (!image_channels && append == BaseImage::CA_REPEAT)
			append = BaseImage::CA_UNDEF;
		T*buffer = alloc<T>(size_t(image_width)*size_t(image_height)*size_t(new_channels));
		Concurrency::parallel_for(dimension_t(0),image_width,[this,buffer,new_channels](dimension_t x)
		{
			typedef GenericImage<Nature>::Size Size;
			using std::min;
			for (dimension_t y = 0; y < image_height; y++)
			{
			 //   BYTE*old = GetPixel(x,y);
				T*target = &buffer[Size(y)*Size(image_width)*Size(new_channels)+Size(x)*Size(new_channels)];
				M::Vec::copyD(GetPixel(x,y),target,min(image_channels,new_channels));
			};
		});
		T fixed_value = 0;
		switch (append)
		{
				case BaseImage::CA_ZERO:
					fixed_value = 0;
				break;
				case BaseImage::CA_MAX:
					fixed_value = max;
				break;
				case BaseImage::CA_HALF:
					fixed_value = max/2;
				break;
		}


		if (new_channels > image_channels)
			switch (append)
			{
				case BaseImage::CA_REPEAT:
				{
					Concurrency::parallel_for(dimension_t(0),image_width,[this,buffer,new_channels](dimension_t x)
					{
						typedef GenericImage<Nature>::Size Size;
						for (dimension_t y = 0; y < image_height; y++)
							M::Vec::setD(&buffer[(Size(y)*Size(image_width)+Size(x))*Size(new_channels)+Size(image_channels)],this->GetPixel(x,y)[image_channels-1],new_channels-image_channels);
					});
				}
				break;
				case BaseImage::CA_ZERO:
				case BaseImage::CA_MAX:
				case BaseImage::CA_HALF:
					Concurrency::parallel_for(dimension_t(0),image_width,[this,buffer,new_channels,fixed_value](dimension_t x)
					{
						typedef GenericImage<Nature>::Size Size;
						for (dimension_t y = 0; y < image_height; y++)
							M::Vec::setD(&buffer[(Size(y)*Size(image_width)+Size(x))*Size(new_channels)+Size(image_channels)],fixed_value,new_channels-image_channels);
					});
				break;
				default:
				break;
			}
		dealloc(image_data);
		image_data = buffer;
		image_channels = new_channels;
	}

template <typename T>
	inline void		ImageTemplate<T>::set3(dimension_t X, dimension_t Y, const T*data)
	{
		T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
		pixel[0] = data[0];
		pixel[1] = data[1];
		pixel[2] = data[2];
	}

template <typename T>
	inline void		ImageTemplate<T>::Set(dimension_t X, dimension_t Y, T red, T green, T blue)
	{
		T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
		switch (image_channels)
		{
			case 0:
			break;
			case 1:
				pixel[0] = red;
			break;
			case 2:
				pixel[0] = red;
				pixel[1] = green;
			break;
			default:
				pixel[0] = red;
				pixel[1] = green;
				pixel[2] = blue;
			break;
		}
	}

template <typename T>
	inline void		ImageTemplate<T>::Set(dimension_t X, dimension_t Y, T red, T green, T blue, T alpha)
	{
		T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
		switch (image_channels)
		{
			case 0:
			break;
			case 1:
				pixel[0] = red;
			break;
			case 2:
				pixel[0] = red;
				pixel[1] = green;
			break;
			case 3:
				pixel[0] = red;
				pixel[1] = green;
				pixel[2] = blue;
			break;
			default:
				pixel[0] = red;
				pixel[1] = green;
				pixel[2] = blue;
				pixel[3] = alpha;
			break;

		}
	}




template <typename T>
	inline void		ImageTemplate<T>::set4(dimension_t X, dimension_t Y, const T*data)
	{
		T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
		pixel[0] = data[0];
		pixel[1] = data[1];
		pixel[2] = data[2];
		pixel[3] = data[3];
	}
	
template <typename T>
	inline void		ImageTemplate<T>::Set(dimension_t X, dimension_t Y, const T*data)
	{
		if (!image_data)
			return;
		T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
		for (BYTE i = 0; i < image_channels; i++)
			pixel[i] = data[i];
	}


template <typename T>
	inline void		ImageTemplate<T>::Set(dimension_t X, dimension_t Y, const T*data, BYTE channels)
	{
		if (!image_data || !channels)
			return;
		index_t offset = (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
		for (BYTE k = 0; k < channels; k++)
			image_data[offset+k] = data[k];
		for (BYTE k = channels; k < image_channels; k++)
			image_data[offset+k] = data[channels-1];
	}











template <class Nature>
	template <typename Float>
		inline void		GenericImage<Nature>::set3f(dimension_t X, dimension_t Y, const Float data[3])
		{
			T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
			pixel[0] = Nature::floatToChannel(data[0]);
			pixel[1] = Nature::floatToChannel(data[1]);
			pixel[2] = Nature::floatToChannel(data[2]);
		}

template <class Nature>
	template <typename Float>
		inline void		GenericImage<Nature>::setf(dimension_t X, dimension_t Y, Float red, Float green, Float blue)
		{
			T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
			switch (image_channels)
			{
				case 0:
				break;
				case 1:
					pixel[0] = Nature::floatToChannel(red);
				break;
				case 2:
					pixel[0] = Nature::floatToChannel(red);
					pixel[1] = Nature::floatToChannel(green);
				break;
				default:
					pixel[0] = Nature::floatToChannel(red);
					pixel[1] = Nature::floatToChannel(green);
					pixel[2] = Nature::floatToChannel(blue);
				break;
			}
		}

template <class Nature>
	template <typename Float>
		inline void		GenericImage<Nature>::setf(dimension_t X, dimension_t Y, Float red, Float green, Float blue, Float alpha)
		{
			T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
			switch (image_channels)
			{
				case 0:
				break;
				case 1:
					pixel[0] = Nature::floatToChannel(red);
				break;
				case 2:
					pixel[0] = Nature::floatToChannel(red);
					pixel[1] = Nature::floatToChannel(green);
				break;
				case 3:
					pixel[0] = Nature::floatToChannel(red);
					pixel[1] = Nature::floatToChannel(green);
					pixel[2] = Nature::floatToChannel(blue);
				break;
				default:
					pixel[0] = Nature::floatToChannel(red);
					pixel[1] = Nature::floatToChannel(green);
					pixel[2] = Nature::floatToChannel(blue);
					pixel[3] = Nature::floatToChannel(alpha);
				break;

			}
		}




template <class Nature>
	template <typename Float>
		inline void		GenericImage<Nature>::set4f(dimension_t X, dimension_t Y, const Float data[4])
		{
			T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
			pixel[0] = Nature::floatToChannel(data[0]);
			pixel[1] = Nature::floatToChannel(data[1]);
			pixel[2] = Nature::floatToChannel(data[2]);
			pixel[3] = Nature::floatToChannel(data[3]);
		}
	
template <class Nature>
	template <typename Float>
		inline void		GenericImage<Nature>::setf(dimension_t X, dimension_t Y, const Float*data)
		{
			if (!image_data)
				return;
			T*pixel = image_data + (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
			for (BYTE i = 0; i < image_channels; i++)
				pixel[i] = Nature::floatToChannel(data[i]);
		}


template <class Nature>
	template <typename Float>
		inline void		GenericImage<Nature>::setf(dimension_t X, dimension_t Y, const Float*data, BYTE channels)
		{
			if (!image_data || !channels)
				return;
			index_t offset = (size_t(Y)*size_t(image_width)+size_t(X))*size_t(image_channels);
			for (BYTE k = 0; k < channels; k++)
				image_data[offset+k] = Nature::floatToChannel(data[k]);
			for (BYTE k = channels; k < image_channels; k++)
				image_data[offset+k] = Nature::floatToChannel(data[channels-1]);
		}

template <typename T>
	void			ImageTemplate<T>::CopyChannel(const ImageTemplate<T>&source_image, BYTE source_channel, BYTE target_channel)
	{
		if (source_image.GetWidth() != GetWidth() || source_image.GetHeight() != GetHeight() || source_channel >= source_image.channels() || target_channel >= channels())
		{
			FATAL__("Copy operation cannot be performed due to image incompatibility or parameter invalidity");
			return;
		}
		Concurrency::parallel_for(dimension_t(0),image_width,[this,&source_image,source_channel,target_channel](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
			{
				this->get(x,y)[target_channel] = source_image.get(x,y)[source_channel];
			};
		});
	}

template <typename T>
	float				ImageTemplate<T>::SampleChannelAt(float x, float y, BYTE channel, bool loop)	const
	{
		if (!image_width || !image_height || channel >= image_channels)
			return 0;
		if (loop)
		{
			x = x - floor(x),
				//fmod(x,1.f);
			y = y - floor(y);
			//fmod(y,1.f);
			x*=(image_width);
			y*=(image_height);
		}
		else
		{
			x = M::clamped(x,0.f,1.f);
			y = M::clamped(y,0.f,1.f);
			x*=(image_width-1);
			y*=(image_height-1);
		}
		dimension_t x0 = ((dimension_t)floor(x)) % image_width,
					y0 = ((dimension_t)floor(y)) % image_height,
					x1,
					y1;

		if (loop)
		{
			x1 = (x0+1) % image_width;
			y1 = (y0+1) % image_height;
		}
		else
		{
			x1 = std::min(x0+1,image_width-1);
			y1 = std::min(y0+1,image_height-1);
		}
		float	xf = x - floor(x),
					//fmod(x,1.f),
				yf = y - floor(y);
				//fmod(y,1.f);
		float	s00 = (float)get(x0,y0)[channel],
				s10 = (float)get(x1,y0)[channel],
				s11 = (float)get(x1,y1)[channel],
				s01 = (float)get(x0,y1)[channel],
				i0 = s00 * (1.f - xf) + s10 * xf,
				i1 = s01 * (1.f - xf) + s11 * xf;
		//ShowMessage(String(s00)+"; "+String(s10)+"; "+String(s11)+"; "+String(s01));
		return (i0 * (1.f - yf) + i1 * yf);
	}


template <typename T>
	T				ImageTemplate<T>::SmoothSampleChannelAt(float x, float y, BYTE channel, bool loop)	const
	{
		if (!image_width || !image_height || channel >= image_channels)
			return 0;
		if (loop)
		{
			x = x - floor(x),
				//fmod(x,1.f);
			y = y - floor(y);
			//fmod(y,1.f);
			x*=(image_width);
			y*=(image_height);
		}
		else
		{
			x = M::clamped(x,0.f,1.f);
			y = M::clamped(y,0.f,1.f);
			x*=(image_width-1);
			y*=(image_height-1);
		}
		dimension_t x0 = (dimension_t)floor(x),
					y0 = (dimension_t)floor(y),
					x1,
					y1;

		if (loop)
		{
			x1 = (x0+1) % image_width;
			y1 = (y0+1) % image_height;
		}
		else
		{
			x1 = std::min(x0+1,image_width-1);
			y1 = std::min(y0+1,image_height-1);
		}
		float	xf = M::cubicFactor(x - floor(x)),
					//fmod(x,1.f),
				yf = M::cubicFactor(y - floor(y));
				//fmod(y,1.f);
		float	s00 = (float)get(x0,y0)[channel],
				s10 = (float)get(x1,y0)[channel],
				s11 = (float)get(x1,y1)[channel],
				s01 = (float)get(x0,y1)[channel],
				i0 = s00 * (1.f - xf) + s10 * xf,
				i1 = s01 * (1.f - xf) + s11 * xf;
		//ShowMessage(String(s00)+"; "+String(s10)+"; "+String(s11)+"; "+String(s01));
		return static_cast<T>(i0 * (1.f - yf) + i1 * yf);
	}



template <typename T>
	inline T*		ImageTemplate<T>::GetVerified(dimension_t X, dimension_t Y)
	{
		if (X < image_width && Y < image_height && image_channels)
			return GetPixel(X,Y);
		return NULL;
	}

template <typename T>
	inline const T*		ImageTemplate<T>::GetVerified(dimension_t X, dimension_t Y) const
	{
		if (X < image_width && Y < image_height && image_channels)
			return GetPixel(X,Y);
		return NULL;
	}


template <typename T>
	void		ImageTemplate<T>::SwapChannels(BYTE c0, BYTE c1)
	{
		if (c0 == c1 || c0 >= image_channels || c1 >= image_channels)
			return;
		typedef size_t	size_type;
		size_t pixels = size_t(image_width)*size_t(image_height);
		Concurrency::parallel_for(index_t(0),pixels,[this,c0,c1](index_t i)
		{
			swp(
				image_data[i*size_type(image_channels)+size_type(c0)],
				image_data[i*size_type(image_channels)+size_type(c1)]
			);
		});
	}


template <typename T>
	void		ImageTemplate<T>::AppendAlpha(const ImageTemplate<T>*other)
	{
		if (!other || !other->image_channels || other->image_width != image_width || other->image_height != image_height)
			return;
		typedef size_t	size_type;
		if (image_channels != 4)
		{
			T*buffer = alloc<T>(size_t(image_width)*size_t(image_height)*size_t(4));
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&buffer](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
				{
				 //   BYTE*old = GetPixel(x,y);
					using std::min;
					M::Vec::copyD(this->GetPixel(x,y),&buffer[size_type(y)*size_type(image_width)*size_type(4)+size_type(x)*size_type(4)],min(image_channels,(BYTE)4));
				};
			});

			dealloc(image_data);
			image_data = buffer;
			image_channels = 4;
		}

		Concurrency::parallel_for(dimension_t(0),image_width,[this,&other](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
				SetChannel(x,y,3,other->GetPixel(x,y)[0]);
		});
	}

template <typename T>
	void		ImageTemplate<T>::AppendAlphaAndDelete(ImageTemplate<T>*other)
	{
		AppendAlpha(other);
		if (other)
			Discard(other);
	}

template <typename T>
	inline size_t ImageTemplate<T>::size()																const
	{
		return (size_t)image_width*(size_t)image_height*(size_t)image_channels*sizeof(T);
	}

template <class Nature>
	void	GenericImage<Nature>::ChangeSaturation(float scale_by)
	{
		if (!IsColorMap() || image_channels < 3)
			return;
		Concurrency::parallel_for(dimension_t(0),image_height,[this,scale_by](dimension_t y)
		{
			for (dimension_t x = 0; x < image_width; x++)
			{
				T	*pixel = GetPixel(x,y);
				F	avg = (static_cast<F>(pixel[0]) + static_cast<F>(pixel[1]) + static_cast<F>(pixel[2]))/static_cast<F>(3);
				pixel[0] = static_cast<T>(avg + (pixel[0] - avg)*scale_by);
				pixel[1] = static_cast<T>(avg + (pixel[1] - avg)*scale_by);
				pixel[2] = static_cast<T>(avg + (pixel[2] - avg)*scale_by);
			}
		});
	}

template <class Nature>
	void	GenericImage<Nature>::Power(BYTE channel, float exponent)
	{
		if (!IsColorMap())
			return;
		Concurrency::parallel_for(dimension_t(0),image_height,[this,exponent,channel](dimension_t y)
		{
			for (dimension_t x = 0; x < image_width; x++)
			{
				F	val = pow(static_cast<F>(GetPixel(x,y)[channel]) / static_cast<F>(max),exponent);
				GetPixel(x,y)[channel] = static_cast<T>(val * static_cast<F>(max));
			}
		});

	}



template <class Nature>
	void GenericImage<Nature>::FlipHorizontally()
	{
		if (IsNormalMap() && image_channels >= 3)
		{
			Concurrency::parallel_for(dimension_t(0),image_height,[this](dimension_t y)
			{
				M::TVec3<F> n0,n1;
				for (dimension_t x = 0; x < image_width/2; x++)
				{
					GetNormal(x,y,n0);
					GetNormal(image_width-x-1,y,n1);
					n0.x *= -1;
					n1.x *= -1;
					setNormal(x,y,n1);
					setNormal(image_width-x-1,y,n0);
				}
			});
		
			if (image_channels > 3)
			{
				BYTE range = image_channels-3;
				Concurrency::parallel_for(dimension_t(0),image_height,[this,range](dimension_t y)
				{
					T buffer[0x100];
					for (dimension_t x = 0; x < image_width/2; x++)
					{
						T	*lower = GetPixel(x,y)+3,
							*upper = GetPixel(image_width-x-1,y)+3;
						M::Vec::copyD(lower,buffer,range);
						M::Vec::copyD(upper,lower,range);
						M::Vec::copyD(buffer,upper,range);
					}
				});
			}			
		}
		else
		{
			Concurrency::parallel_for(dimension_t(0),image_height,[this](dimension_t y)
			{
				T buffer[0x100];
				for (dimension_t x = 0; x < image_width/2; x++)
				{
					M::Vec::copyD(GetPixel(x,y),buffer,image_channels);
					setf(x,y,GetPixel(image_width-x-1,y));
					setf(image_width-x-1,y,buffer);
				}
			});
		}
	}



template <class Nature>
	void GenericImage<Nature>::FlipVertically()
	{
		if (IsNormalMap() && image_channels >= 3)
		{
			Concurrency::parallel_for(dimension_t(0),image_width,[this](dimension_t x)
			{
				M::TVec3<F> n0,n1;
				for (dimension_t y = 0; y < image_height/2; y++)
				{
					GetNormal(x,y,n0);
					GetNormal(x,image_height-y-1,n1);
					n0.y *= -1;
					n1.y *= -1;
					setNormal(x,y,n1);
					setNormal(x,image_height-y-1,n0);
				}
			});
			if (image_channels > 3)
			{
				BYTE range = image_channels-3;
				Concurrency::parallel_for(dimension_t(0),image_width,[this,range](dimension_t x)
				{
					T buffer[0x100];
					for (dimension_t y = 0; y < image_height/2; y++)
					{
						T	*lower = GetPixel(x,y)+3,
							*upper = GetPixel(x,image_height-y-1)+3;
						M::Vec::copyD(lower,buffer,range);
						M::Vec::copyD(upper,lower,range);
						M::Vec::copyD(buffer,upper,range);
					}
				});
			}
		}
		else
		{
			Concurrency::parallel_for(dimension_t(0),image_width,[this](dimension_t x)
			{
				T buffer[0x100];
				for (dimension_t y = 0; y < image_height/2; y++)
				{
					M::Vec::copyD(GetPixel(x,y),buffer,image_channels);
					setf(x,y,GetPixel(x,image_height-y-1));
					setf(x,image_height-y-1,buffer);
				}
			});
		}
	}


template <class Nature>
	void GenericImage<Nature>::Rotate90CW()
	{
		GenericImage<Nature>	temp(image_height,image_width,image_channels);
		temp.content_type = content_type;
		temp.origin.adoptData(origin);



		if (IsNormalMap())
		{
			Concurrency::parallel_for(dimension_t(0),image_height,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_width; y++)
				{
					M::TVec3<F> n;
					GetNormal(image_width-y-1,x,n);
					swp(n.x,n.y);
					n.y *= -1;
					temp.setNormal(x,y,n);
				}
			});
		}
		else
			Concurrency::parallel_for(dimension_t(0),image_height,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_width; y++)
					temp.Set(x,y,GetPixel(image_width-y-1,x));
			});

		adoptData(temp);
	}

template <class Nature>
	void GenericImage<Nature>::Rotate90CCW()
	{
		GenericImage<Nature>	temp(image_height,image_width,image_channels);
		temp.content_type = content_type;
		temp.origin.adoptData(origin);

		if (IsNormalMap())
		{
			Concurrency::parallel_for(dimension_t(0),image_height,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_width; y++)
				{
					M::TVec3<F> n;
					GetNormal(image_width-y-1,x,n);
					swp(n.x,n.y);
					n.x *= -1;
					temp.setNormal(x,y,n);
				}
			});
		}
		else
			Concurrency::parallel_for(dimension_t(0),image_height,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_width; y++)
					temp.Set(x,y,GetPixel(y,image_height-x-1));
			});

		adoptData(temp);
	}








template <class Nature>
	bool GenericImage<Nature>::ScaleHalf()
	{
		if (image_width<2 || image_height<2)
			return false;
		GenericImage<Nature>	temp(image_width/2,image_height/2,image_channels);
		temp.content_type = content_type;
		temp.origin.adoptData(origin);
		if (! temp.ScaleHalf(*this))
			return false;
		adoptData(temp);
		return true;
	}

template <class Nature>
	bool GenericImage<Nature>::ScaleHalf(const Self&source)
	{
		Super::SetSize(source.GetWidth()/2,source.GetHeight()/2,source.GetChannels());
		if (image_channels == 4)
		{
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&source](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
					if (IsNormalMap())
					{
						M::TVec3<F> norm0,norm1,norm2,norm3;
						source.GetNormal(x*2,y*2,norm0);
						source.GetNormal(x*2+1,y*2,norm1);
						source.GetNormal(x*2+1,y*2+1,norm2);
						source.GetNormal(x*2,y*2+1,norm3);
						F a0 = Nature::toFloat(source.GetPixel(x*2,y*2)[3]);
						F a1 = Nature::toFloat(source.GetPixel(x*2+1,y*2)[3]);
						F a2 = Nature::toFloat(source.GetPixel(x*2+1,y*2+1)[3]);
						F a3 = Nature::toFloat(source.GetPixel(x*2,y*2+1)[3]);
						F aSum = a0 + a1 + a2 + a3;
						if (aSum > 0)
						{
							M::Vec::mult(norm0,a0);
							M::Vec::mad(norm0,norm1,a1);
							M::Vec::mad(norm0,norm2,a2);
							M::Vec::mad(norm0,norm3,a3);
							M::Vec::div(norm0,aSum);
							M::Vec::normalize0(norm0);
							this->setNormal(x,y,norm0);
							this->GetPixel(x,y)[3] = Nature::floatToChannel(aSum/(F)4);
						}
						else
						{
							M::Vec::add(norm0,norm1);
							M::Vec::add(norm0,norm2);
							M::Vec::add(norm0,norm3);
							M::Vec::normalize0(norm0);
							this->setNormal(x,y,norm0);
							this->GetPixel(x,y)[3] = 0;
						}
					}
					else
					{
						const T	*p0 = source.GetPixel(x*2,y*2),
								*p1 = source.GetPixel(x*2+1,y*2),
								*p2 = source.GetPixel(x*2+1,y*2+1),
								*p3 = source.GetPixel(x*2,y*2+1);
						F a0 = Nature::toFloat(p0[3]);
						F a1 = Nature::toFloat(p1[3]);
						F a2 = Nature::toFloat(p2[3]);
						F a3 = Nature::toFloat(p3[3]);
						F aSum = a0 + a1 + a2 + a3;
						T*outPixel = this->get(x,y);
						if (aSum > 0)
						{
							F avg0 = ((F)p0[0] * a0 + (F)p1[0] * a1 + (F)p2[0] * a2 + (F)p3[0] * a3) / aSum;
							F avg1 = ((F)p0[1] * a0 + (F)p1[1] * a1 + (F)p2[1] * a2 + (F)p3[1] * a3) / aSum;
							F avg2 = ((F)p0[2] * a0 + (F)p1[2] * a1 + (F)p2[2] * a2 + (F)p3[2] * a3) / aSum;
							outPixel[0] = (T)avg0;
							outPixel[1] = (T)avg1;
							outPixel[2] = (T)avg2;
							outPixel[3] = Nature::floatToChannel(aSum / (F)4);
						}
						else
						{
							F avg0 = ((F)p0[0] + (F)p1[0] + (F)p2[0] + (F)p3[0]) / (F)4;
							F avg1 = ((F)p0[1] + (F)p1[1] + (F)p2[1] + (F)p3[1]) / (F)4;
							F avg2 = ((F)p0[2] + (F)p1[2] + (F)p2[2] + (F)p3[2]) / (F)4;
							outPixel[0] = (T)avg0;
							outPixel[1] = (T)avg1;
							outPixel[2] = (T)avg2;
							outPixel[3] = 0;
						}
					}
			});
		}
		else
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&source](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
					if (IsNormalMap() && image_channels >= 3)
					{
						M::TVec3<F> norm0,norm1,norm2,norm3;
						source.GetNormal(x*2,y*2,norm0);
						source.GetNormal(x*2+1,y*2,norm1);
						source.GetNormal(x*2+1,y*2+1,norm2);
						source.GetNormal(x*2,y*2+1,norm3);
						M::Vec::add(norm0,norm1);
						M::Vec::add(norm0,norm2);
						M::Vec::add(norm0,norm3);
						M::Vec::normalize0(norm0);
						this->setNormal(x,y,norm0);
						if (image_channels > 3)
						{
							for (BYTE c = 3; c < image_channels; c++)
							{
								F col = source.GetPixel(x*2,y*2)[c];
								col += source.GetPixel(x*2+1,y*2)[c];
								col += source.GetPixel(x*2+1,y*2+1)[c];
								col += source.GetPixel(x*2,y*2+1)[c];
								this->SetChannel(x,y,c,(T)(col/4));
							}
						}
					}
					else
					{
						for (BYTE c = 0; c < image_channels; c++)
						{
							F col = source.GetPixel(x*2,y*2)[c];
							col += source.GetPixel(x*2+1,y*2)[c];
							col += source.GetPixel(x*2+1,y*2+1)[c];
							col += source.GetPixel(x*2,y*2+1)[c];
							this->SetChannel(x,y,c,(T)(col/4));
						}
		/*                if (!x && !y)
							ShowMessage(_toString(temp.GetPixel(x,y),image_channels));*/
					}
			});

		return true;
	}

template <class Nature>
	bool GenericImage<Nature>::ScaleXHalf()
	{
		if (image_width<2)
			return false;
		GenericImage<Nature>	temp(image_width/2,image_height,image_channels);
		temp.content_type = content_type;
		temp.origin.adoptData(origin);

		Concurrency::parallel_for(dimension_t(0),image_height,[this,&temp](dimension_t y)
		{
			for (dimension_t x = 0; x < image_width/2; x++)
				if (IsNormalMap() && image_channels == 3)
				{
					M::TVec3<F> norm0,norm1;
					GetNormal(x*2,y,norm0);
					GetNormal(x*2+1,y,norm1);
					M::Vec::add(norm0,norm1);
					M::Vec::normalize0(norm0);
					temp.setNormal(x,y,norm0);
				}
				else
					for (BYTE c = 0; c < image_channels; c++)
						temp.SetChannel(x,y,c,(T)(((F)this->GetPixel(x*2,y)[c]+this->GetPixel(x*2+1,y)[c])/2));
		});
		adoptData(temp);
		return true;
	}


template <class Nature>
	bool GenericImage<Nature>::ScaleYHalf()
	{
		if (image_height<2)
			return false;
		GenericImage<Nature>	temp(image_width,image_height/2,image_channels);
		temp.SetContentType(content_type);
		temp.origin.adoptData(origin);

		Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height/2; y++)
				if (IsNormalMap() && image_channels == 3)
				{
					M::TVec3<F> norm0,norm1;
					GetNormal(x,y*2,norm0);
					GetNormal(x,y*2+1,norm1);
					M::Vec::add(norm0,norm1);
					M::Vec::normalize0(norm0);
					temp.setNormal(x,y,norm0);
				}
				else
					for (BYTE c = 0; c < image_channels; c++)
						temp.SetChannel(x,y,c,(T)(((F)this->GetPixel(x,y*2)[c]+this->GetPixel(x,y*2+1)[c])/2));
		});
		adoptData(temp);
		return true;
	}

template <class Nature>
	void GenericImage<Nature>::ScaleDouble(bool loop)
	{
		if (image_width * 2 < image_width || image_height * 2 < image_height)
			return;
		if (!image_width || !image_height)
		{
			image_width *= 2;
			image_height *= 2;
			return;
		}

		ImageTemplate<T>	temp(image_width*2,image_height*2,image_channels);
		temp.SetContentType(content_type);
		temp.origin.adoptData(origin);

		Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
				temp.Set(x*2,y*2,GetPixel(x,y));
		});
    
		if (!loop)
		{
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height-1; y++)
					for (BYTE k = 0; k < image_channels; k++)
						temp.SetChannel(x*2,y*2+1,k,(T)(((F)GetPixel(x,y)[k]+GetPixel(x,y+1)[k])/2));
			});
			Concurrency::parallel_for(dimension_t(0),image_width-1,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
					for (BYTE k = 0; k < image_channels; k++)
						temp.SetChannel(x*2+1,y*2,k,(T)(((F)GetPixel(x,y)[k]+GetPixel(x+1,y)[k])/2));
			});
			Concurrency::parallel_for(dimension_t(0),image_width-1,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height-1; y++)
					for (BYTE k = 0; k < image_channels; k++)
						temp.SetChannel(x*2+1,y*2+1,k,(T)(((F)GetPixel(x,y)[k]+GetPixel(x,y+1)[k]+GetPixel(x+1,y+1)[k]+GetPixel(x+1,y)[k])/4));
			});
			Concurrency::parallel_for(dimension_t(0),image_height*2,[this,&temp](dimension_t y)
			{
				temp.Set(image_width*2-1,y,temp.GetPixel(image_width*2-2,y));
			});
			Concurrency::parallel_for(dimension_t(0),image_width*2,[this,&temp](dimension_t x)
			{
				temp.Set(x,image_height*2-1,temp.GetPixel(x,image_height*2-2));
			});
		}
		else
		{
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
					for (BYTE k = 0; k < image_channels; k++)
						temp.SetChannel(x*2,y*2+1,k,(T)(((F)GetPixel(x,y)[k]+GetPixel(x,(y+1)%image_height)[k])/2));
			});
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
					for (BYTE k = 0; k < image_channels; k++)
						temp.SetChannel(x*2+1,y*2,k,(T)(((F)GetPixel(x,y)[k]+GetPixel((x+1)%image_width,y)[k])/2));
			});
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
					for (BYTE k = 0; k < image_channels; k++)
						temp.SetChannel(x*2+1,y*2+1,k,(T)(((F)GetPixel(x,y)[k]+GetPixel(x,(y+1)%image_height)[k]+GetPixel((x+1)%image_width,(y+1)%image_height)[k]+GetPixel((x+1)%image_width,y)[k])/4));
			});
		}
    
       
		adoptData(temp);
	}

template <class Nature>
	void GenericImage<Nature>::ScaleXDouble(bool loop)
	{
		if (!image_width || image_width*2 < image_width)
			return;
		GenericImage<Nature>	temp(image_width*2,image_height,image_channels);
		temp.origin.adoptData(origin);
		temp.SetContentType(content_type);

		Concurrency::parallel_for(dimension_t(0),image_height,[this,&temp](dimension_t y)
		{
			for (dimension_t x = 0; x < image_width; x++)
			{
				temp.Set(x*2,y,this->GetPixel(x,y));
				if (x == image_width-1)
					continue;
				if (IsNormalMap() && image_channels == 3)
				{
					M::TVec3<F> norm0,norm1;
					GetNormal(x,y,norm0);
					GetNormal(x+1,y,norm1);
					M::Vec::add(norm0,norm1);
					M::Vec::normalize0(norm0);
					temp.setNormal(x*2+1,y,norm0);
				}
				else
					for (BYTE c = 0; c < image_channels; c++)
						temp.SetChannel(x*2+1,y,c,(T)(((F)this->GetPixel(x,y)[c]+this->GetPixel(x+1,y)[c])/2));
			}
			temp.Set(image_width*2-1,y,this->GetPixel(image_width-1,y));
		});
		adoptData(temp);

	}

template <class Nature>
	void GenericImage<Nature>::ScaleYDouble(bool loop)
	{
		if (!image_height || image_height*2 < image_height)
			return;
		GenericImage<Nature>	temp(image_width,image_height*2,image_channels);
		temp.origin.adoptData(origin);
		temp.SetContentType(content_type);

		Concurrency::parallel_for(dimension_t(0),image_width,[this,&temp](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
			{
				temp.Set(x,y*2,this->GetPixel(x,y));
				if (y == image_height-1)
					continue;
				if (IsNormalMap() && image_channels == 3)
				{
					M::TVec3<F> norm0,norm1;
					GetNormal(x,y,norm0);
					GetNormal(x,y+1,norm1);
					M::Vec::add(norm0,norm1);
					M::Vec::normalize0(norm0);
					temp.setNormal(x,y*2+1,norm0);
				}
				else
					for (BYTE c = 0; c < image_channels; c++)
						temp.SetChannel(x,y*2+1,c,(T)(((F)this->GetPixel(x,y)[c]+this->GetPixel(x,y+1)[c])/2));
			}
			temp.Set(x,image_height*2-1,this->GetPixel(x,image_height-1));
		});
		adoptData(temp);
	}


template <class Nature>
	bool GenericImage<Nature>::ScaleTo(dimension_t width, dimension_t height)
	{
		if (!image_width || !image_height)
			return false;
		while (image_width <= width/2)
			ScaleXDouble();
		while (image_height <= height/2)
			ScaleYDouble();
		while (image_width >= width*2)
			ScaleXHalf();
		while (image_height >= height*2)
			ScaleYHalf();
		return image_width == width && image_height == height;
	}

template <class Nature>
	void GenericImage<Nature>::Resample(dimension_t w, dimension_t h)
	{
		if (ScaleTo(w,h))
			return;
		GenericImage<Nature>	temp(w,h,image_channels);
		temp.origin.adoptData(origin);
		temp.SetContentType(content_type);
		if (IsNormalMap())
		{
			Concurrency::parallel_for(dimension_t(0),w,[this,&temp,w,h](dimension_t x)
			{
				M::TVec3<F> normal;
				for (dimension_t y = 0; y < h; y++)
				{
					sampleNormal((F)x/w,(F)y/h,(F)(x+1)/w,(F)(y+1)/h,normal);
					temp.setNormal(x,y,normal);
					//_c3(pixel,temp.GetPixel(x,y));
				}
			});
		}
		else
			Concurrency::parallel_for(dimension_t(0),w,[this,&temp,w,h](dimension_t x)
			{
				T pixel[256];
				for (dimension_t y = 0; y < h; y++)
				{
					sample((F)x/w,(F)y/h,(F)(x+1)/w,(F)(y+1)/h,pixel);
					temp.Set(x,y,pixel);
				}
			});
		adoptData(temp);
	}

template <class Nature>
	void	GenericImage<Nature>::Crop(dimension_t x, dimension_t y, dimension_t width, dimension_t height)
	{
		if (x >= image_width || y >= image_height)
		{
			free();
			return;
		}

		using std::min;
		dimension_t	xext = min(width,image_width-x),
					yext = min(height,image_height-y);
             
		if (!xext || !yext)
		{
			free();
			return;
		}
		if (xext == image_width && yext == image_height)
			return;
		const BYTE c_num = image_channels;

		GenericImage<Nature>		temp(xext,yext,image_channels);
		temp.origin.adoptData(origin);
		temp.SetContentType(content_type);
		
		Concurrency::parallel_for(dimension_t(0),yext,[this,&temp,x,y,xext](dimension_t Y)
		{
			memcpy(temp.GetPixel(0,Y),GetPixel(x,y+Y),sizeof(T)*xext*image_channels);
		});

		adoptData(temp);
	}

template <class Nature>
	void GenericImage<Nature>::Truncate(dimension_t width, dimension_t height)
	{
		if (image_width <= width && image_height <= height)
			return;

		using std::min;
		GenericImage<Nature>	new_image(min(width,image_width),min(height,image_height),image_channels);
		new_image.origin.adoptData(origin);
		new_image.SetContentType(content_type);
		new_image.Paint(this,0,0,IC_COPY);
	
		adoptData(new_image);
	}
	

template <typename T>
	void ImageTemplate<T>::readFrom(const ImageTemplate<T>*other)
	{
		SetSize(other->image_width,other->image_height,other->image_channels);
		content_type = other->content_type;
		origin = other->origin;
		read(other->image_data);
	}

template <typename T>
	void	ImageTemplate<T>::read(const T* data)
	{
		memcpy(image_data,data,size());
	}

template <typename T>
	bool	ImageTemplate<T>::ColumnIsOpaque( dimension_t x, bool IsOpaque(const T*) )	const
	{
		for (dimension_t y = 0; y < image_height; y++)
			if (IsOpaque(GetPixel(x,y)))
				return true;
		return false;
	}

template <typename T>
	bool	ImageTemplate<T>::RowIsOpaque( dimension_t y, bool IsOpaque(const T*) )	const
	{
		for (dimension_t x = 0; x < image_width; x++)
			if (IsOpaque(GetPixel(x,y)))
				return true;
		return false;
	}

template <typename T>
	bool	ImageTemplate<T>::TruncateToOpaque( bool IsOpaque(const T*) )
	{
		if (image_channels != 4 && image_channels != 2)
			return false;
		if (!image_width || !image_height)
			return false;
		dimension_t	left = 0,right=image_width,
				bottom = 0,top=image_height;
		while (left < right && !ColumnIsOpaque(left,IsOpaque))
			left++;
		while (right > left && !ColumnIsOpaque(right-1,IsOpaque))
			right--;
		while (bottom < top && !RowIsOpaque(bottom,IsOpaque))
			bottom++;
		while (bottom < top && !RowIsOpaque(top-1,IsOpaque))
			top--;
		if (bottom == top || left == right)
		{
			SetSize(0,0,image_channels);
			return true;
		}
		if (bottom == 0 && top == 0 && right == image_width && top == image_height)
			return false;
		ImageTemplate<T>	new_data(right-left,top-bottom,image_channels);
		new_data.origin.adoptData(origin);
		new_data.content_type = content_type;

		if (image_channels == 4)
			Concurrency::parallel_for(dimension_t(0),new_data.image_width,[this,&new_data,left,bottom](dimension_t x)
			{
				for (dimension_t y = 0; y < new_data.image_height; y++)
					M::VecUnroll<4>::copy(GetPixel(left+x,bottom+y),new_data.GetPixel(x,y));
			});
		else
			Concurrency::parallel_for(dimension_t(0),new_data.image_width,[this,&new_data,left,bottom](dimension_t x)
			{
				for (dimension_t y = 0; y < new_data.image_height; y++)
					M::VecUnroll<2>::copy(GetPixel(left+x,bottom+y),new_data.GetPixel(x,y));
			});
		adoptData(new_data);
		return true;
	}

template <typename T>
	/*static*/ bool		ImageTemplate<T>::RGBAIsOpaque( const T*pixel )
	{
		return pixel[3] != 0;
	}
	
template <typename T>
	/*static*/ bool		ImageTemplate<T>::GrayAIsOpaque( const T*pixel )
	{
		return pixel[1] != 0;
	}



template <typename T>
	bool	ImageTemplate<T>::TruncateToOpaque( )
	{
		switch (image_channels)
		{
			case 2:
				return TruncateToOpaque(GrayAIsOpaque);
			case 4:
				return TruncateToOpaque(RGBAIsOpaque);
			default:
				return false;
		}
	}


template <typename T>
	bool	ImageTemplate<T>::ExportRectangle(dimension_t xoffset, dimension_t yoffset, dimension_t w, dimension_t h, Self&target)	const
	{
		if (yoffset >= image_height || xoffset >= image_width)
		{
			target.SetSize(0,0,image_channels);
			return false;
		}
		using std::min;
		h = min(h,image_height-yoffset);
		w = min(w,image_width-xoffset);
		target.SetSize(w,h,this->image_channels);
		ASSERT__(ExportRectangle(xoffset,yoffset,w,h,target.data()));
		return true;
	}



template <typename T>
	bool	ImageTemplate<T>::ExportRectangle(dimension_t xoffset, dimension_t yoffset, dimension_t w, dimension_t h, T*target)	const
	{
		if (yoffset+h > image_height || xoffset+w > image_width)
			return false;

		Concurrency::parallel_for(dimension_t(yoffset),yoffset+h,[this,target,xoffset,yoffset,w](dimension_t y)
		{
			memcpy(target+size_t(y-yoffset)*size_t(w)*size_t(image_channels),
					image_data+size_t(y)*size_t(image_width)*size_t(image_channels)+size_t(xoffset)*size_t(image_channels),
					size_t(w)*size_t(image_channels)*sizeof(T));
		});
		return true;
	}

template <typename T>
	bool	ImageTemplate<T>::ImportRectangle(dimension_t xoffset, dimension_t yoffset, dimension_t w, dimension_t h, const T*target)
	{
		if (yoffset+h > image_height || xoffset+w > image_width)
			return false;
		Concurrency::parallel_for(dimension_t(yoffset),yoffset+h,[this,target,xoffset,yoffset,w](dimension_t y)
		{
			memcpy(image_data+size_t(y)*size_t(image_width)*size_t(image_channels)+size_t(xoffset)*size_t(image_channels),
					target+size_t(y-yoffset)*size_t(w)*size_t(image_channels),
					size_t(w)*size_t(image_channels)*sizeof(T));
		});
		return true;
	}

template <class Nature>
	template <typename Float>
		bool	GenericImage<Nature>::ImportRectangleF(dimension_t xoffset, dimension_t yoffset, dimension_t w, dimension_t h, const Float*target)
		{
			if (yoffset+h > image_height || xoffset+w > image_width)
				return false;
			Concurrency::parallel_for(dimension_t(0),h,[this,target,xoffset,yoffset,w](dimension_t y)
			{
				for (dimension_t x = 0; x < w; x++)
				{
					index_t toffset = (size_t(y+yoffset)*size_t(image_width)+size_t(x+xoffset))*size_t(image_channels),
							soffset = (size_t(y)*size_t(w)+size_t(x))*size_t(image_channels);
					for (BYTE c = 0; c < image_channels; c++)
						 image_data[toffset+c] = Nature::floatToChannel(target[soffset+c]);
				}
			});
			return true;
		}

template <class Nature>
	template <typename Float>
		bool	GenericImage<Nature>::ExportRectangleF(dimension_t xoffset, dimension_t yoffset, dimension_t w, dimension_t h, Float*target) const
		{
			if (yoffset+h > image_height || xoffset+w > image_width)
				return false;
			Concurrency::parallel_for(dimension_t(0),h,[this,target,xoffset,yoffset,w](dimension_t y)
			{
				for (dimension_t x = 0; x < w; x++)
				{
					index_t soffset = (size_t(y+yoffset)*size_t(image_width)+size_t(x+xoffset))*size_t(image_channels),
							toffset = (size_t(y)*size_t(w)+size_t(x))*size_t(image_channels);
					for (BYTE c = 0; c < image_channels; c++)
						target[toffset+c] = Nature::toFloat(image_data[soffset+c]);
				}
			});
			return true;
		}


template <typename T>
	void	ImageTemplate<T>::ExtractChannels(BYTE channel, BYTE c_num, ImageTemplate<T>&target)
	{
		if (channel+c_num > image_channels || !c_num)
		{
			target.free();
			return;
		}
		target.SetSize(image_width,image_height,c_num);
		target.SetContentType(content_type);
		target.origin = origin;

		Concurrency::parallel_for(dimension_t(0),image_width,[this,&target,channel](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
				target.Set(x,y,GetPixel(x,y)+channel);
		});
	}

template <class Nature>
	void	GenericImage<Nature>::FractalDouble(F noise_level, bool loop)
	{
		FractalDouble(noise_level,loop,rand());
	}

template <class Nature>
	void	GenericImage<Nature>::FractalDouble(F noise_level, bool loop, unsigned seed)
	{
		if (!image_width || !image_height || !image_channels || image_width*2<image_width ||image_height*2<image_height)
			return;
	
		if (!loop)
		{
			ImageTemplate<T> new_image(image_width*2-1,image_height*2-1,image_channels);

			new_image.origin.adoptData(origin);
			new_image.SetContentType(content_type);

			#if 0
				X X X X X X X X
						   
				X X X X X X X X
						   
				X X X X X X X X
						   
				X X X X X X X X
						   
				X X X X X X X X
			#endif
		
		
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&new_image](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
				{
					const T	*from = GetPixel(x,y);
					T		*to = new_image.GetPixel(x*2,y*2);
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = from[k];
				}
			});
		
			#if 0
				* * * * * * * *
				 X X X X X X X 
				* * * * * * * *
				 X X X X X X X 
				* * * * * * * *
				 X X X X X X X 
				* * * * * * * *
				 X X X X X X X 
				* * * * * * * *
			#endif
			
			Concurrency::parallel_for(dimension_t(0),image_width-1,[this,&new_image,noise_level,seed](dimension_t x)
			{
				ImageRandomSource rnd((unsigned long)(seed+x*29));

				for (dimension_t y = 0; y+1 < image_height; y++)
				{
					const T		*from0 = GetPixel(x,y),
								*from1 = GetPixel(x+1,y),
								*from2 = GetPixel(x+1,y+1),
								*from3 = GetPixel(x,y+1);
					T			*to = new_image.GetPixel(x*2+1,y*2+1);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k]+(F)from3[k])/F(4)+(F(rnd())*F(2)-F(1))*noise_level);
				}
			});
			
			#if 0
				* * * * * * * *
				 *X*X*X*X*X*X* 
				* * * * * * * *
				 *X*X*X*X*X*X* 
				* * * * * * * *
				 *X*X*X*X*X*X* 
				* * * * * * * *
				 *X*X*X*X*X*X* 
				* * * * * * * *
			
				 0
				3X1
				 2
			#endif
			
			Concurrency::parallel_for(dimension_t(1),image_width-1,[this,&new_image,noise_level,seed](dimension_t x)
			{
				ImageRandomSource rnd((unsigned long)(seed+x*29+1023));

				for (dimension_t y = 0; y+1 < image_height; y++)
				{
					const T		*from0 = GetPixel(x,y),
								*from1 = new_image.GetPixel(x*2+1,y*2+1),
								*from2 = GetPixel(x,y+1),
								*from3 = new_image.GetPixel(x*2-1,y*2+1);
					T			*to = new_image.GetPixel(x*2,y*2+1);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k]+(F)from3[k])/F(4)+(F(rnd())*F(2)-F(1))*noise_level);
				}
			});
			
			#if 0
				* * * * * * * *
				X************* 
				* * * * * * * *
				X************* 
				* * * * * * * *
				X************* 
				* * * * * * * *
				X************* 
				* * * * * * * *
			
				 0
				 X1
				 2
			#endif
			
				Concurrency::parallel_for(dimension_t(0),image_height-1,[this,&new_image,noise_level,seed](dimension_t y)
				{
					ImageRandomSource rnd((unsigned long)(seed+y*29+10234));

					const T		*from0 = GetPixel(0,y),
								*from1 = new_image.GetPixel(1,y*2+1),
								*from2 = GetPixel(0,y+1);
					T			*to = new_image.GetPixel(0,y*2+1);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k])/F(3)+(F(rnd())*F(2)-F(1))*noise_level);
				});

			
			#if 0
				* * * * * * * *
				**************X
				* * * * * * * *
				**************X
				* * * * * * * *
				**************X
				* * * * * * * *
				**************X
				* * * * * * * *
			
				 0
				2X
				 1
			#endif
			
				Concurrency::parallel_for(dimension_t(0),image_height-1,[this,&new_image,noise_level,seed](dimension_t y)
				{
					ImageRandomSource rnd((unsigned long)(seed+y*29+102349));

					const T		*from0 = GetPixel(image_width-1,y),
								*from1 = GetPixel(image_width-1,y+1),
								*from2 = new_image.GetPixel((image_width-1)*2-1,y*2+1);
					T			*to = new_image.GetPixel((image_width-1)*2,y*2+1);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k])/F(3)+(F(rnd())*F(2)-F(1))*noise_level);
				});
		
			#if 0
				* * * * * * * *
				***************
				*X*X*X*X*X*X*X*
				***************
				*X*X*X*X*X*X*X*
				***************
				*X*X*X*X*X*X*X*
				***************
				* * * * * * * *
			
				 3
				0X2
				 1
			#endif
			
			Concurrency::parallel_for(dimension_t(0),image_width-1,[this,&new_image,noise_level,seed](dimension_t x)
			{
				ImageRandomSource rnd((unsigned long)(seed+x*29+182349));

				for (dimension_t y = 1; y+1 < image_height; y++)
				{
					const T		*from0 = GetPixel(x,y),
								*from1 = new_image.GetPixel(x*2+1,y*2-1),
								*from2 = GetPixel(x+1,y),
								*from3 = new_image.GetPixel(x*2+1,y*2+1);
					T			*to = new_image.GetPixel(x*2+1,y*2);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k]+(F)from3[k])/F(4)+(F(rnd())*F(2)-F(1))*noise_level);
				}
			});
			
			#if 0
				* * * * * * * *
				***************
				***************
				***************
				***************
				***************
				***************
				***************
				*X*X*X*X*X*X*X*
			
				 2
				0X1

			#endif
			
				Concurrency::parallel_for(dimension_t(0),image_width-1,[this,&new_image,noise_level,seed](dimension_t x)
				{
					ImageRandomSource rnd((unsigned long)(seed+x*29+1827349));

					const T		*from0 = GetPixel(x,0),
								*from1 = GetPixel(x+1,0),
								*from2 = new_image.GetPixel(x*2+1,1);
					T			*to = new_image.GetPixel(x*2+1,0);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k])/F(3)+(F(rnd())*F(2)-F(1))*noise_level);
				});
			
			#if 0
				*X*X*X*X*X*X*X*
				***************
				***************
				***************
				***************
				***************
				***************
				***************
				***************
			
			 
				0X2
				 1
			#endif
			
				Concurrency::parallel_for(dimension_t(0),image_width-1,[this,&new_image,noise_level,seed](dimension_t x)
				{
					ImageRandomSource rnd((unsigned long)(seed+x*29+827349));
					const T		*from0 = GetPixel(x,image_height-1),
								*from1 = new_image.GetPixel(x*2+1,(image_height-1)*2-1),
								*from2 = GetPixel(x+1,image_height-1);
					T			*to = new_image.GetPixel(x*2+1,(image_height-1)*2);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k])/F(3)+((F)rnd()*F(2)-F(1))*noise_level);
				});

			adoptData(new_image);
		}
		else
		{
			ImageTemplate<T> new_image(image_width*2,image_height*2,image_channels);
			new_image.origin.adoptData(origin);
			new_image.SetContentType(content_type);
		
			#if 0
			                
				X X X X X X X X 
						    
				X X X X X X X X 
						    
				X X X X X X X X 
						    
				X X X X X X X X 
						    
				X X X X X X X X 
			#endif
		
		
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&new_image](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
				{
					const T		*from = GetPixel(x,y);
					T			*to = new_image.GetPixel(x*2,y*2);
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = from[k];
				}
			});
		
			#if 0
				 X X X X X X X X
				* * * * * * * * 
				 X X X X X X X X
				* * * * * * * *
				 X X X X X X X X
				* * * * * * * *
				 X X X X X X X X
				* * * * * * * *
				 X X X X X X X X
				* * * * * * * * 
			#endif
			
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&new_image,noise_level,seed](dimension_t x)
			{
				ImageRandomSource rnd((unsigned long)(seed+x*29+27349));

				for (dimension_t y = 0; y < image_height; y++)
				{
					const T		*from0 = GetPixel(x,y),
								*from1 = GetPixel((x+1)%image_width,y),
								*from2 = GetPixel((x+1)%image_width,(y+1)%image_height),
								*from3 = GetPixel(x,(y+1)%image_height);
					T			*to = new_image.GetPixel(x*2+1,y*2+1);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k]+(F)from3[k])/F(4)+(F(rnd())*F(2)-F(1))*noise_level);
				}
			});
			
			#if 0
				X*X*X*X*X*X*X*X*
				* * * * * * * * 
				X*X*X*X*X*X*X*X*
				* * * * * * * * 
				X*X*X*X*X*X*X*X*
				* * * * * * * * 
				X*X*X*X*X*X*X*X*
				* * * * * * * * 
				X*X*X*X*X*X*X*X*
				* * * * * * * * 
			
				 0
				3X1
				 2
			#endif
			
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&new_image,noise_level,seed](dimension_t x)
			{
				ImageRandomSource rnd((unsigned long)(seed+x*29+7349));

				for (dimension_t y = 0; y < image_height; y++)
				{
					const T		*from0 = GetPixel(x,y),
								*from1 = new_image.GetPixel(x*2+1,y*2+1),
								*from2 = GetPixel(x,(y+1)%image_height),
								*from3 = new_image.GetPixel(x>0?x*2-1:image_width*2-1,y*2+1);
					T			*to = new_image.GetPixel(x*2,y*2+1);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k]+(F)from3[k])/F(4)+(F(rnd())*F(2)-F(1))*noise_level);
				}
			});
	
			#if 0
				****************
				*X*X*X*X*X*X*X*X
				****************
				*X*X*X*X*X*X*X*X
				****************
				*X*X*X*X*X*X*X*X
				****************
				*X*X*X*X*X*X*X*X
				****************
				*X*X*X*X*X*X*X*X
			
				 3
				0X2
				 1
			#endif
			
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&new_image,noise_level,seed](dimension_t x)
			{
				ImageRandomSource rnd((unsigned long)(seed+x*29+349));


				for (dimension_t y = 0; y < image_height; y++)
				{
					const T		*from0 = GetPixel(x,y),
								*from1 = new_image.GetPixel(x*2+1,y>0?y*2-1:image_height*2-1),
								*from2 = GetPixel((x+1)%image_width,y),
								*from3 = new_image.GetPixel(x*2+1,y*2+1);
					T			*to = new_image.GetPixel(x*2+1,y*2);
					typedef _TYPENAME GenericImage<Nature>::ImageNature::float_type_t	F;
					for (BYTE k = 0; k < image_channels; k++)
						to[k] = GenericImage<Nature>::ImageNature::clamp(((F)from0[k]+(F)from1[k]+(F)from2[k]+(F)from3[k])/F(4)+(F(rnd())*F(2)-F(1))*noise_level);
				}
			});
		
			adoptData(new_image);
		}
	}

template <class Nature>
	void	GenericImage<Nature>::ScaleChannel(BYTE channel, F factor)
	{
		if (channel >= image_channels)
			return;
		Concurrency::parallel_for(dimension_t(0),image_width,[this,channel,factor](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
			{
        		T&val = GetPixel(x,y)[channel];
				val = GenericImage<Nature>::ImageNature::clamp((F)val*factor);
			}
		});
	}

template <class Nature>
	void	GenericImage<Nature>::ScaleChannel(BYTE channel, F offset, F factor)
	{
		if (channel >= image_channels)
			return;
		Concurrency::parallel_for(dimension_t(0),image_width,[this,channel,offset,factor](dimension_t x)
		{
			for (dimension_t y = 0; y < image_height; y++)
			{
        		T&val = GetPixel(x,y)[channel];
				val = GenericImage<Nature>::ImageNature::clamp(offset + (F)val*factor);
			}
		});
	}


template <class Nature>
	void	GenericImage<Nature>::Paint(const ImageTemplate<T>*other, dimension_t x, dimension_t y, BaseImage::combiner_t combiner)
	{
		Draw(other,x,y,combiner);
	}



template <class Nature0>
	void	GenericImage<Nature0>::Draw(const ImageTemplate<T>*other, dimension_t x, dimension_t y, BaseImage::combiner_t combiner)
	{
		if (x >= image_width || y >= image_height)
			return;

		using std::min;
		dimension_t	xext = min(other->GetWidth(),image_width-x),
					yext = min(other->GetHeight(),image_height-y);
             
		if (!xext || !yext)
			return;
		BYTE c_num = min(image_channels,other->channels());
		if (!c_num)
			return;

		if (other->channels() != 4)
		{
			if (combiner == IC_ALPHA_BLEND)
				combiner = IC_COPY;
			elif (combiner == IC_ALPHA_ADD)
				combiner = IC_ADD;
		}

		switch (combiner)
		{
			case IC_ALPHA_BLEND:
				switch(c_num)
				{
					case 3:
						Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext](dimension_t X)
						{
							for (dimension_t Y = 0; Y < yext; Y++)
							{
								T*p = GetPixel(x+X,y+Y);
								const T*o = other->GetPixel(X,Y);
								p[0] = (T)(((F)p[0]*(max-o[3]) + (F)o[0]*o[3])/max);
								p[1] = (T)(((F)p[1]*(max-o[3]) + (F)o[1]*o[3])/max);
								p[2] = (T)(((F)p[2]*(max-o[3]) + (F)o[2]*o[3])/max);
							}
						});
					break;
					case 4:
						Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext](dimension_t X)
						{
							for (dimension_t Y = 0; Y < yext; Y++)
							{
								T*p = GetPixel(x+X,y+Y);
								const T*o = other->GetPixel(X,Y);
								typedef _TYPENAME GenericImage<Nature0>::ImageNature::float_type_t	F;
								using std::min;
								F a = p[3]>0?min((F)o[3]/p[3],(F)1):F(1);
								p[0] = (T)(((F)p[0]*(1.0-a) + (F)o[0]*a));
								p[1] = (T)(((F)p[1]*(1.0-a) + (F)o[1]*a));
								p[2] = (T)(((F)p[2]*(1.0-a) + (F)o[2]*a));
								p[3] = (T)(p[3]+o[3]*(255-p[3])/255);
							}
						});
					break;
					default:
						Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext,c_num](dimension_t X)
						{
							for (dimension_t Y = 0; Y < yext; Y++)
								blend(x+X,y+Y,other->GetPixel(X,Y),other->GetPixel(X,Y)[3],c_num);
						});
					break;
				}
			break;
			case IC_COPY:
				{
					auto p = this;
					Concurrency::parallel_for(dimension_t(0),xext,[this,p,&other,x,y,yext,c_num](dimension_t X)
					{
						for (dimension_t Y = 0; Y < yext; Y++)
							p->Set(x+X,y+Y,other->GetPixel(X,Y),c_num);
					});
				}
			break;
			case IC_MULTIPLY:
				Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext,c_num](dimension_t X)
				{
					for (dimension_t Y = 0; Y < yext; Y++)
					{
						T		*local = GetPixel(x+X,y+Y);
						const T	*remote = other->GetPixel(X,Y);
						typedef _TYPENAME GenericImage<Nature0>::ImageNature	Nature;
						for (BYTE c = 0; c < c_num; c++)
							local[c] = Nature::floatToChannel(Nature::toFloat(local[c]) * Nature::toFloat(remote[c]));
					}
				});
			break;
			case IC_MULT_NEGATIVE:
				Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext,c_num](dimension_t X)
				{
					for (dimension_t Y = 0; Y < yext; Y++)
					{
						T		*local = GetPixel(x+X,y+Y);
						const T	*remote = other->GetPixel(X,Y);
						typedef _TYPENAME GenericImage<Nature0>::ImageNature	Nature;
						for (BYTE c = 0; c < c_num; c++)
							local[c] = max-Nature::floatToChannel((Nature::toFloat(max-local[c])) * (Nature::toFloat(max-remote[c])));
					}
				});
			break;

			case IC_ALPHA_ADD:
		
				switch(c_num)
				{
					case 3:
						Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext](dimension_t X)
						{
							for (dimension_t Y = 0; Y < yext; Y++)
							{
								T		*p = GetPixel(x+X,y+Y);
								const T	*o = other->GetPixel(X,Y);
								typedef _TYPENAME GenericImage<Nature0>::ImageNature::float_type_t	F;
								typedef _TYPENAME GenericImage<Nature0>::ImageNature	Nature;
								F weight = Nature::toFloat(o[3]);
								p[0] = Nature::clamp((F)p[0] + (F)o[0]*weight);
								p[1] = Nature::clamp((F)p[1] + (F)o[1]*weight);
								p[2] = Nature::clamp((F)p[2] + (F)o[2]*weight);
							}
						});
					break;
					case 4:
						Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext](dimension_t X)
						{
							for (dimension_t Y = 0; Y < yext; Y++)
							{
								T		*p = GetPixel(x+X,y+Y);
								const T	*o = other->GetPixel(X,Y);
								typedef _TYPENAME GenericImage<Nature0>::ImageNature::float_type_t	F;
								typedef _TYPENAME GenericImage<Nature0>::ImageNature	Nature;
								F weight = Nature::toFloat(o[3]);
								p[0] = Nature::clamp((F)p[0] + (F)o[0]*weight);
								p[1] = Nature::clamp((F)p[1] + (F)o[1]*weight);
								p[2] = Nature::clamp((F)p[2] + (F)o[2]*weight);
								p[3] = Nature::clamp((F)p[3] + (F)o[3]*weight);
							}
						});
					break;
					default:
						Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext,c_num](dimension_t X)
						{
							for (dimension_t Y = 0; Y < yext; Y++)
							{
								T		*local = GetPixel(x+X,y+Y);
								const T	*remote = other->GetPixel(X,Y);
								typedef _TYPENAME GenericImage<Nature0>::ImageNature::float_type_t	F;
								typedef _TYPENAME GenericImage<Nature0>::ImageNature	Nature;
								F weight = Nature::toFloat(remote[3]);
								for (BYTE c = 0; c < c_num; c++)
									local[c] = Nature::clamp((F)local[c]+(F)((F)remote[c]*weight));
							}
						});
					break;
				}		

			break;
			case IC_ADD:
				Concurrency::parallel_for(dimension_t(0),xext,[this,&other,x,y,yext,c_num](dimension_t X)
				{
					for (dimension_t Y = 0; Y < yext; Y++)
					{
						T		*local = GetPixel(x+X,y+Y);
						const T	*remote = other->GetPixel(X,Y);
						typedef _TYPENAME GenericImage<Nature0>::ImageNature::float_type_t	F;
						typedef _TYPENAME GenericImage<Nature0>::ImageNature	Nature;
						for (BYTE c = 0; c < c_num; c++)
							local[c] = Nature::clamp((F)local[c]+(F)remote[c]);
					}
				});
			break;
		}

	}


template <class Nature>
	template <class Nature1>
		void	GenericImage<Nature>::ToNormalMap(GenericImage<Nature1>&target, F x_texel_distance, F y_texel_distance, F height_scale, bool seamless, BYTE height_channel)		const
		{
			if (!image_channels)
				return;
			if (height_channel >= image_channels)
				height_channel = image_channels-1;
			target.SetSize(image_width,image_height,3);
			target.SetContentType(PixelType::TangentSpaceNormal);
	
	
			F		x_bump_scale = height_scale/x_texel_distance,
					y_bump_scale = height_scale/y_texel_distance;
				//Image normal(bump.GetWidth(),bump.GetHeight(),3);
		
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&target,seamless,x_bump_scale,y_bump_scale,height_channel](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
				{
					dimension_t	prevx = x?x-1:(seamless?image_width-1:0),
							prevy = y?y-1:(seamless?image_height-1:0),
							nextx = x+1<image_width?x+1:(seamless?0:image_width-1),
							nexty = y+1<image_height?y+1:(seamless?0:image_height-1);
		
					const T		*px0 = GetPixel(prevx,y),
								*px1 = GetPixel(nextx,y),
								*py0 = GetPixel(x,prevy),
								*py1 = GetPixel(x,nexty);
					F		dx = ((F)px1[height_channel]-(F)px0[height_channel])*x_bump_scale,
							dy = ((F)py1[height_channel]-(F)py0[height_channel])*y_bump_scale;
					M::TVec3<F>	n = {-dx,-dy,1};
					M::Vec::normalize(n);
					//cout << _toString(n);
					target.setNormal(x,y,n);
					//cout << " => "<<_toString(target.GetPixel(x,y))<<" @"<<x_bump_scale << "x"<<y_bump_scale<< endl;
				}
			});
			//cout << x_bump_scale << "x"<<y_bump_scale<<endl;

		}


template <class Nature>
	template <class Nature1>
		void	GenericImage<Nature>::ToNormalMap(GenericImage<Nature1>&target, F intensity, bool seamless, BYTE height_channel)		const
		{
			if (!image_channels)
				return;
			if (height_channel >= image_channels)
				height_channel = image_channels-1;
			target.SetSize(image_width,image_height,3);
			target.SetContentType(PixelType::TangentSpaceNormal);
	
	
			F		scale = (F(512)*F(max)),
					x_bump_scale = F(image_width)/scale*intensity,
					y_bump_scale = F(image_height)/scale*intensity;
				//Image normal(bump.GetWidth(),bump.GetHeight(),3);
		
			Concurrency::parallel_for(dimension_t(0),image_width,[this,&target,seamless,x_bump_scale,y_bump_scale,height_channel](dimension_t x)
			{
				for (dimension_t y = 0; y < image_height; y++)
				{
					dimension_t	prevx = x?x-1:(seamless?image_width-1:0),
							prevy = y?y-1:(seamless?image_height-1:0),
							nextx = x+1<image_width?x+1:(seamless?0:image_width-1),
							nexty = y+1<image_height?y+1:(seamless?0:image_height-1);
		
					const T		*px0 = GetPixel(prevx,y),
								*px1 = GetPixel(nextx,y),
								*py0 = GetPixel(x,prevy),
								*py1 = GetPixel(x,nexty);
					F		dx = ((F)px1[height_channel]-(F)px0[height_channel])*x_bump_scale,
							dy = ((F)py1[height_channel]-(F)py0[height_channel])*y_bump_scale;
					M::TVec3<F>	n = {-dx,-dy,1};
					M::Vec::normalize(n);
					//cout << _toString(n);
					target.setNormal(x,y,n);
					//cout << " => "<<_toString(target.GetPixel(x,y))<<" @"<<x_bump_scale << "x"<<y_bump_scale<< endl;
				}
			});
			//cout << x_bump_scale << "x"<<y_bump_scale<<endl;
		}

template <class Nature>
	template <typename Float>
		void	GenericImage<Nature>::GetNormal(dimension_t X, dimension_t Y, M::TVec3<Float>&out) const
		{
			const T*pixel = GetPixel(X,Y);
			out.x = Nature::toNormalComponent(pixel[0]);
			out.y = Nature::toNormalComponent(pixel[1]);
			out.z = Nature::toNormalComponent(pixel[2]);
		}

template <class Nature>
	template <typename Float>
		bool GenericImage<Nature>::GetNormalVerified(dimension_t X, dimension_t Y, M::TVec3<Float>&out)             const
		{
			if (X >= image_width || Y >= image_height || image_channels<3)
				return false;

			const T*pixel = GetPixel(X,Y);
			out.x = Nature::toNormalComponent(pixel[0]);
			out.y = Nature::toNormalComponent(pixel[1]);
			out.z = Nature::toNormalComponent(pixel[2]);
			return true;
		}


template <class Nature>
	template <typename Float>
		void	GenericImage<Nature>::setNormal(dimension_t X, dimension_t Y, const M::TVec3<Float>&data)
		{
			T*p = GetPixel(X,Y);
			//for (BYTE i = 0; i < 3; i++)
			p[0] = Nature::normalComponentToChannel(data.x);
			p[1] = Nature::normalComponentToChannel(data.y);
			p[2] = Nature::normalComponentToChannel(data.z);
		}

template <class Nature>
	void GenericImage<Nature>::blend(dimension_t X, dimension_t Y, const T*data,T alpha)
	{
		typedef typename TypeInfo<T>::GreaterType::Type	GT;	//next larger type (floats map to themselves)
		T*pixel = GetPixel(X,Y);
		for (BYTE k = 0; k < image_channels; k++)
			pixel[k] = (GT(pixel[k])*(max-alpha) + GT(data[k])*alpha)/max;
	}

template <class Nature>
	template <typename Float>
		void GenericImage<Nature>::BlendF(dimension_t X, dimension_t Y, const Float*data, Float alpha)
		{
			T*pixel = GetPixel(X,Y);
			for (BYTE k = 0; k < image_channels; k++)
				pixel[k] = (T)((F)pixel[k]*(1-alpha) + (F)(data[k]*alpha*F(max)));
		}

template <class Nature>
	void GenericImage<Nature>::blend(dimension_t X, dimension_t Y, const T*data,T alpha, BYTE channels)
	{
		typedef typename TypeInfo<T>::GreaterType::Type	GT;	//next larger type
		if (channels > image_channels)
			channels = image_channels;
		T*pixel = GetPixel(X,Y);
		for (BYTE k = 0; k < channels; k++)
			pixel[k] = (GT(pixel[k])*(max-alpha) + GT(data[k])*alpha)/max;
	}

template <class Nature>
	template <typename Float>
		void GenericImage<Nature>::BlendF(dimension_t X, dimension_t Y, const Float*data, Float alpha, BYTE channels)
		{
			if (channels > image_channels)
				channels = image_channels;
			T*pixel = GetPixel(X,Y);
			for (BYTE k = 0; k < channels; k++)
				pixel[k] = (T)((F)pixel[k]*(1-alpha) + (F)(data[k]*alpha*F(max)));
		}



template <class Nature>
	bool    alignableImages(const GenericImage<Nature>*field, unsigned cnt)
	{
		if (!cnt || !field)
			return false;
		unsigned w(field[0].GetWidth()),h(field[0].GetHeight());
		BYTE channels_(field[0].GetChannels());
		for (unsigned i = 1; i < cnt; i++)
			if (field[i].GetWidth() != w || field[i].GetHeight() != h || field[i].GetChannels() != channels_)
				return true;

		return false;
	}

template <class Nature>
	bool    alignableImages(const GenericImage<Nature>**field, unsigned cnt)
	{
		if (!cnt || !field)
			return false;
		unsigned w(field[0]->GetWidth()),h(field[0]->GetHeight());
		BYTE channels_(field[0]->GetChannels());
		for (unsigned i = 1; i < cnt; i++)
			if (field[i]->GetWidth() != w || field[i]->GetHeight() != h || field[i]->GetChannels() != channels_)
				return true;
		return false;
	}




template <class Nature>
	void    alignImages(GenericImage<Nature>*field, unsigned cnt)
	{
		if (!cnt || !field)
			return;
		unsigned w(field->GetWidth()),h(field->GetHeight());
		BYTE channels_(field->GetChannels());
		for (unsigned i = 1; i < cnt; i++)
		{
			if (field[i].GetWidth() > w)
				w = field[i].GetWidth();
			if (field[i].GetHeight() > h)
				h = field[i].GetHeight();
			if (field[i].GetChannels() > channels_)
				channels_ = field[i].GetChannels();
		}
		for (unsigned i = 0; i < cnt; i++)
		{
			if (!field[i].ScaleTo(w,h))
				field[i].Resample(w,h);
			BYTE difference = channels_-field[i].GetChannels();
			field[i].SetChannels(channels_);
			for (BYTE k = channels_-difference; k < channels_; k++)
				field[i].SetChannel(k,(BYTE)0x100);
		}
	}

template <class Nature>
	void    alignImages(GenericImage<Nature>**field, unsigned cnt)
	{
		if (!cnt || !field)
			return;
		unsigned w(0),h(0);
		BYTE channels_(0);
		for (unsigned i = 0; i < cnt; i++)
			if (field[i])
			{
				if (field[i]->GetWidth() > w)
					w = field[i]->GetWidth();
				if (field[i]->GetHeight() > h)
					h = field[i]->GetHeight();
				if (field[i]->GetChannels() > channels_)
					channels_ = field[i]->GetChannels();
			}
		for (unsigned i = 0; i < cnt; i++)
			if (field[i])
			{
				if (!field[i]->ScaleTo(w,h))
					field[i]->Resample(w,h);
				BYTE difference = channels_-field[i]->GetChannels();
				field[i]->SetChannels(channels_);
				for (BYTE k = channels_-difference; k < channels_; k++)
					field[i]->SetChannel(k,(BYTE)0x100);
			}
	}



#endif
