#ifndef bufferTplH
#define bufferTplH



#undef min

#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
	#define CHK_FILL_LEVEL	if (fillLevel != usageEnd - storageBegin) FATAL__("Invalid buffer state");
#else
	#define CHK_FILL_LEVEL
#endif


#if defined(_DEBUG) && __BUFFER_DBG_RANGE_CHECK__
	#if __BUFFER_DBG_COUNT__
		#define BUFFER_CHECK_RANGE(_INDEX_)	if ((_INDEX_) >= fillLevel) FATAL__("Index out of range for buffer access");
	#else
		#define BUFFER_CHECK_RANGE(_INDEX_)	if ((_INDEX_) >= Count()) FATAL__("Index out of range for buffer access");
	#endif
	#define BUFFER_ASSERT_NOT_EMPTY()	if (usageEnd == storageBegin)	FATAL__("Cannot access element: buffer is empty");
#else
	#define BUFFER_CHECK_RANGE(_INDEX_)
	#define BUFFER_ASSERT_NOT_EMPTY()
#endif





template <typename T, typename MyStrategy>
	inline void		BasicVector<T, MyStrategy>::DestructAndFree(T*range_begin, T*range_end)
	{
		MyStrategy::destructRange(range_begin,range_end);
		free(range_begin);
	}


template <typename T>
	inline T*	CommonVectorStorage<T>::Allocate(count_t len)
	{
		if (!len)
			return NULL;
		T*rs = (T*)malloc(sizeof(T)*len);
		if (!rs)
			throw std::bad_alloc(/*"Unable to resize buffer instance: Insufficient memory"*/);
		return rs;
	}


template <typename T>
	inline T*	CommonVectorStorage<T>::AllocateNotEmpty(count_t len)
	{
		T*rs = (T*)malloc(sizeof(T)*len);
		if (!rs)
			throw std::bad_alloc(/*"Unable to resize buffer instance: Insufficient memory"*/);
		return rs;
	}


template <typename T, typename MyStrategy>
	inline void BasicVector<T, MyStrategy>::EnsureHasSpace(count_t elements)
	{
		const count_t	current_size = storageEnd-storageBegin,
						fill = usageEnd-storageBegin,
						want_size = fill+elements;
		count_t target_size = current_size;

		if (!target_size)
			target_size = 4;

		while (target_size < want_size)
		{
			count_t new_target_size = target_size << 1;
			if (new_target_size < target_size)	//overflow
				throw std::bad_alloc(/*"Unable to resize buffer instance: Size overflow"*/);
			target_size = new_target_size;
		}

		if (target_size != current_size)
		{
			try
			{
				T	*new_field = AllocateNotEmpty(target_size);

				MyStrategy::constructRangeFromFleetingData(new_field,new_field+current_size,storageBegin);
				DestructAndFree(storageBegin,usageEnd);

				storageBegin = new_field;
				storageEnd = storageBegin+target_size;
				usageEnd = storageBegin+fill;
				CHK_FILL_LEVEL

			}
			catch (std::bad_alloc& exception)
			{
				storageBegin = storageEnd = usageEnd = NULL;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel = 0;
					CHK_FILL_LEVEL
				#endif
				throw;
			}
		}

	}

template <typename T, typename MyStrategy>
	inline void BasicVector<T, MyStrategy>::EnsureHasSpace()
	{
		if (usageEnd == storageEnd)
		{
			index_t	current_index = usageEnd-storageBegin;
			count_t	len = (storageEnd-storageBegin)*2;

			if (len < (count_t)(storageEnd-storageBegin))
				throw std::bad_alloc(/*"Unable to resize buffer instance: Size overflow"*/);

			if (!len)
				len = 4;
			try
			{
				T	*new_field = AllocateNotEmpty(len);//alloc<T>(len);

				MyStrategy::constructRangeFromFleetingData(new_field,new_field+current_index,storageBegin);
				DestructAndFree(storageBegin,usageEnd);

				storageBegin = new_field;
				storageEnd = storageBegin+len;
				usageEnd = storageBegin+current_index;
				CHK_FILL_LEVEL
			}
			catch (std::bad_alloc& exception)
			{
			/*	unnecessary since exception would be thrown in AllocateNotEmpty(), which leaves list intact
				storageBegin = storageEnd = usageEnd = NULL;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel = 0;
					CHK_FILL_LEVEL
				#endif*/
				throw;
			}
		}
	}

template <typename T>
	CommonVectorStorage<T>::CommonVectorStorage(count_t len)
	{
		try
		{
			usageEnd = storageBegin = Allocate(len);
			storageEnd = storageBegin+len;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = 0;
				CHK_FILL_LEVEL
			#endif
		}
		catch (std::bad_alloc& exception)
		{
			usageEnd = storageBegin = storageEnd = NULL;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = 0;
				CHK_FILL_LEVEL
			#endif
			throw;
		}
	}


template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>::BasicVector(count_t len):Super(len)
	{}



template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>::~BasicVector()
	{
		DestructAndFree(storageBegin,usageEnd);
		#ifdef _DEBUG
			/*
			In debug mode the destructor of CommonBufferStorage checks that these are properly unset,
			such that CommonBufferStorage never has to delete things on its own, or is left with 
			non-freed pointers.
			The basic problem is that the buffer releases memory by invoking the used strategy
			which CommonBufferStorage cannot do.
			In release we don't check anything because it shouldn't ever happen
			*/
			storageBegin = usageEnd = storageEnd = nullptr;
			fillLevel = 0;
		#endif
	}

template <typename T, typename MyStrategy>
	void	BasicVector<T, MyStrategy>::clear(count_t len)
	{
		//deloc(storageBegin);
		MyStrategy::destructRange(storageBegin,usageEnd);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = 0;
		#endif

		if (storageEnd-storageBegin != len)
		{
			free(storageBegin);
			try
			{
				usageEnd = storageBegin = Allocate(len);
				storageEnd = storageBegin+len;
			}
			catch (std::bad_alloc& exception)
			{
				usageEnd = storageBegin = storageEnd = NULL;
				throw;
			}
		}
		else
			usageEnd = storageBegin;
		CHK_FILL_LEVEL
			//alloc<T>(len);
	}

template <typename T>
	CommonVectorStorage<T>::CommonVectorStorage(std::initializer_list<T> items)
	{
		count_t	Fill = items.size();
		//alloc(storageBegin,Fill);
		try
		{
			storageBegin = Allocate(Fill);
		}
		catch (std::bad_alloc& exception)
		{
			storageBegin = storageEnd = usageEnd = NULL;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = 0;
				CHK_FILL_LEVEL
			#endif

			throw;
		}

		storageEnd = usageEnd = storageBegin+Fill;
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = Fill;
			CHK_FILL_LEVEL
		#endif


		typename std::initializer_list<T>::const_iterator it = items.begin();
		for (index_t i = 0; i < Fill; i++, ++it)
		{
			new ((storageBegin+i)) T(*it);
		}
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>::BasicVector(std::initializer_list<T> items):Super(items)
	{}



template <typename T>
	CommonVectorStorage<T>::CommonVectorStorage(const Self&other)
	{
		const count_t	fill = other.usageEnd-other.storageBegin;
		//alloc(storageBegin,Fill);
		try
		{
			storageBegin = Allocate(fill);
		}
		catch (std::bad_alloc& exception)
		{
			storageBegin = storageEnd = usageEnd = NULL;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = 0;
				CHK_FILL_LEVEL
			#endif

			throw;
		}

		storageEnd = usageEnd = storageBegin+fill;
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = fill;
			CHK_FILL_LEVEL
		#endif

		for (index_t i = 0; i < fill; i++)
		{
			new ((storageBegin+i)) T(other.storageBegin[i]);
			//storageBegin[i] = other.storageBegin[i];
		}
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>::BasicVector(const Self&other):Super(other)
	{}


#if __BUFFER_RVALUE_REFERENCES__
template <typename T>
	CommonVectorStorage<T>::CommonVectorStorage(Self&&other):storageBegin(other.storageBegin),storageEnd(other.storageEnd),usageEnd(other.usageEnd)
	{
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = other.fillLevel;
			other.fillLevel = 0;
			CHK_FILL_LEVEL
		#endif

		other.storageBegin = other.storageEnd = other.usageEnd = NULL;
	}


template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>::BasicVector(Self&&other):Super(std::move(other))
	{}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::operator=(Self&&other)
	{
		adoptData(other);
		return *this;
	}
#endif

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::operator=(const BasicVector<T, MyStrategy>&other)
	{
		MyStrategy::destructRange(storageBegin,usageEnd);
		const count_t	fill = other.usageEnd-other.storageBegin;
		if (count_t(storageEnd-storageBegin) < fill)
		{
			free(storageBegin);
			try
			{
				storageBegin = Allocate(fill);
			}
			catch (std::bad_alloc& exception)
			{
				storageBegin = storageEnd = usageEnd = NULL;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel = 0;
					CHK_FILL_LEVEL
				#endif
				throw;
			}
			storageEnd = usageEnd = storageBegin+fill;
		}
		else
			usageEnd = storageBegin+fill;

		//	deloc(storageBegin);
			//alloc(storageBegin,Fill);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = fill;
			CHK_FILL_LEVEL
		#endif

		for (index_t i = 0; i < fill; i++)
			new ((storageBegin+i)) T(other.storageBegin[i]);
		return *this;
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::operator=(const ArrayRef<T>&other)
	{
		Clear();
		T*field = AppendRow(other.Count());
		MyStrategy::template copyRange<const T,T>(other.begin(),other.end(),field);
		return *this;
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::operator=(std::initializer_list<T> items)
	{
		Clear();
		T*field = AppendRow(items.size());
		MyStrategy::copyRange(items.begin(),items.end(),field);
		return *this;
	}

	
template <typename T, typename MyStrategy>
	void	BasicVector<T, MyStrategy>::Revert()
	{
		MyStrategy::revert(storageBegin,usageEnd);
	}


template <typename T, typename MyStrategy>
	void	BasicVector<T, MyStrategy>::ResizePreserveContent(count_t new_len)
	{
		if ((count_t)(storageEnd-storageBegin) == new_len)
			return;

		count_t preserve = std::min<count_t>(new_len,usageEnd-storageBegin);

		try
		{
			//count_t len = usageEnd-storageBegin;
			T	*new_field = Allocate(new_len);

			MyStrategy::constructRangeFromFleetingData(new_field,new_field+preserve,storageBegin);
			DestructAndFree(storageBegin,usageEnd);

			storageBegin = new_field;
			storageEnd = storageBegin+new_len;
			usageEnd = storageBegin+preserve;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = preserve;
				CHK_FILL_LEVEL
			#endif

		}
		catch (std::bad_alloc& exception)
		{
			storageBegin = usageEnd = storageEnd = NULL;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = 0;
				CHK_FILL_LEVEL
			#endif
			throw;
		}
	}


template <typename T>
	void	CommonVectorStorage<T>::Fill(const T&pattern)
	{
		for (T*c = storageBegin; c < usageEnd; c++)
			(*c) = pattern;
	}

template <typename T, typename MyStrategy>
	inline T*				BasicVector<T, MyStrategy>::appendRow(count_t elements, const T&init_pattern)
	{
		EnsureHasSpace(elements);

		T*result = usageEnd;
		usageEnd += elements;
		MyStrategy::constructRange(result,usageEnd,init_pattern);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel+=elements;
			CHK_FILL_LEVEL
		#endif
		return result;
	}

	
template <typename T, typename MyStrategy>
	inline T*				BasicVector<T, MyStrategy>::appendRow(count_t elements)
	{
		EnsureHasSpace(elements);

		T*result = usageEnd;
		usageEnd += elements;
		MyStrategy::constructRange(result,usageEnd);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel+=elements;
			CHK_FILL_LEVEL
		#endif
		return result;
	}
	
template <typename T, typename MyStrategy>
	template <typename T2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::appendVA(count_t elements, ...)
	{
		if (!elements)
			return *this;

		EnsureHasSpace(elements);
		va_list vl;
		va_start(vl,elements);
		for (index_t i=0; i<elements; i++)
		{
			new (usageEnd++) T(va_arg(vl,T2));
		}
		va_end(vl);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel+=elements;
			CHK_FILL_LEVEL
		#endif

		return *this;
	}


template <typename T, typename MyStrategy>
	inline T&				BasicVector<T, MyStrategy>::Append()
	{
		EnsureHasSpace();

		new (usageEnd) T;
		T&rs = (*usageEnd++);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel++;
			CHK_FILL_LEVEL
		#endif
		return rs;
	}


	
template <typename T, typename MyStrategy>
	inline T&				BasicVector<T, MyStrategy>::Insert(index_t before)
	{
		if (usageEnd == storageEnd)
		{
			index_t	current_index = usageEnd-storageBegin;
			count_t	len = (storageEnd-storageBegin)*2;
			if (!len)
				len = 4;
			try
			{
				T	*new_field = AllocateNotEmpty(len),//alloc<T>(len),
					*write = new_field,
					*read = storageBegin;

				count_t construct_before = std::min(before,current_index),	//elements to construct before the inserted element
						construct_after = current_index - construct_before;	//elements to construct after the inserted element
					//*barrier = vmin(storageBegin+before,usageEnd);

				MyStrategy::constructRangeFromFleetingData(write, write+construct_before, read); write+= construct_before; read+= construct_before;
				T*result = write;
				new (write++) T;
				MyStrategy::constructRangeFromFleetingData(write, write+construct_after, read);

					//(*write++) = (*read++);
				//deloc(storageBegin);
				DestructAndFree(storageBegin,usageEnd);

				storageBegin = new_field;
				storageEnd = storageBegin+len;
				usageEnd = storageBegin+current_index+1;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel++;
					CHK_FILL_LEVEL
				#endif

				return *result;
				//return std::min(storageBegin+before,usageEnd-1);	//logically correct, but 'min' may fail for very large 'before' values (e.g. -1)
					//*(storageBegin+before);
			}
			catch (std::bad_alloc& exception)
			{
				storageBegin = storageEnd = usageEnd = NULL;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel = 0;
					CHK_FILL_LEVEL
				#endif
				throw;
			}
		}
		else
		{
			new (usageEnd) T;
			//T*el = std::min(storageBegin+before,usageEnd);	//logically correct, but 'min' may fail for very large 'before' values (e.g. -1)
			T*el = storageBegin+std::min(before,Count());
			for (T*c = usageEnd; c > el; c--)
				MyStrategy::move(*(c-1),*c);
			usageEnd++;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel++;
				CHK_FILL_LEVEL
			#endif

			return *el;
		}
	}

template <typename T, typename MyStrategy>
	inline T*				BasicVector<T, MyStrategy>::insertRow(index_t before, count_t length)
	{
		if (usageEnd+length > storageEnd)
		{
			count_t	current_fill_level = usageEnd-storageBegin;
			count_t	len = (storageEnd-storageBegin);
			if (!len)
				len = 4;
			while (len < current_fill_level + length)
			{
				len <<= 1;
				if (len < current_fill_level)
					throw new std::bad_alloc();
			}

			try
			{
				T	*new_field = AllocateNotEmpty(len),//alloc<T>(len),
					*write = new_field,
					*read = storageBegin;

				count_t construct_before = std::min(before,current_fill_level),	//elements to construct before the inserted elements
						construct_after = current_fill_level - construct_before;	//elements to construct after the inserted elements
					//*barrier = vmin(storageBegin+before,usageEnd);

				MyStrategy::constructRangeFromFleetingData(write, write+construct_before, read); write+= construct_before; read+= construct_before;
				MyStrategy::constructRange(write,write+length);	 write+= length;
				MyStrategy::constructRangeFromFleetingData(write, write+construct_after, read);

				DestructAndFree(storageBegin,usageEnd);

				storageBegin = new_field;
				storageEnd = storageBegin+len;
				usageEnd = storageBegin+current_fill_level+length;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel+=length;
					CHK_FILL_LEVEL
				#endif

				return std::min(storageBegin+before,usageEnd-length);
					//*(storageBegin+before);
			}
			catch (std::bad_alloc& exception)
			{
				storageBegin = storageEnd = usageEnd = NULL;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel = 0;
					CHK_FILL_LEVEL
				#endif
				throw;
			}
		}
		else
		{
			MyStrategy::constructRange(usageEnd,usageEnd + length);
			T*el = std::min(storageBegin+before,usageEnd);
			for (T*c = usageEnd+length-1; c >= el+length; c--)
				MyStrategy::move(*(c-length),*c);
			usageEnd+=length;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel+=length;
				CHK_FILL_LEVEL
			#endif

			return el;
		}

	}



template <typename T, typename MyStrategy>
template <typename T2>
	inline	T&				BasicVector<T, MyStrategy>::Insert(index_t before, const T2&data)
	{
		if (usageEnd == storageEnd)
		{
			index_t	current_index = usageEnd-storageBegin;
			count_t	len = (storageEnd-storageBegin)*2;
			if (!len)
				len = 4;
			try
			{
				T	*new_field = AllocateNotEmpty(len),//alloc<T>(len),
					*write = new_field,
					*read = storageBegin;

				count_t construct_before = std::min(before,current_index),	//elements to construct before the inserted element
						construct_after = current_index - construct_before;	//elements to construct after the inserted element
					//*barrier = vmin(storageBegin+before,usageEnd);

				MyStrategy::constructRangeFromFleetingData(write, write+construct_before, read); write+= construct_before; read+= construct_before;
				new (write++) T(data);
				MyStrategy::constructRangeFromFleetingData(write, write+construct_after, read);

					//(*write++) = (*read++);
				//deloc(storageBegin);
				DestructAndFree(storageBegin,usageEnd);

				storageBegin = new_field;
				storageEnd = storageBegin+len;
				usageEnd = storageBegin+current_index+1;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel++;
					CHK_FILL_LEVEL
				#endif


				return *std::min(storageBegin+before,usageEnd-1);
					//*(storageBegin+before);
			}
			catch (std::bad_alloc& exception)
			{
				storageBegin = storageEnd = usageEnd = NULL;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					fillLevel = 0;
					CHK_FILL_LEVEL
				#endif
				throw;
			}
		}
		else
		{
			new (usageEnd) T;
			T*el = std::min(storageBegin+before,usageEnd);
			for (T*c = usageEnd; c > el; c--)
				MyStrategy::move(*(c-1),*c);
			(*el) = data;
			//el->operator=(data);
			usageEnd++;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel++;
				CHK_FILL_LEVEL
			#endif
			return *el;
		}
	}

template <typename T, typename MyStrategy>
	inline void				BasicVector<T, MyStrategy>::eraseLast()
	{
		BUFFER_ASSERT_NOT_EMPTY();
		usageEnd--;
		usageEnd->~T();
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel--;
			CHK_FILL_LEVEL
		#endif
	}

template <typename T, typename MyStrategy>
	inline T				BasicVector<T, MyStrategy>::pop()
	{
		BUFFER_ASSERT_NOT_EMPTY();
		usageEnd--;
		T data;
		MyStrategy::move(*usageEnd,data);
		usageEnd->~T();
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel--;
			CHK_FILL_LEVEL
		#endif
		return data;
	}

	/*	Not working ... laaaame

template <typename T>
	inline T				BasicVector<T, CopyStrategy>::pop()
	{
		usageEnd--;
		T data(*usageEnd);
		usageEnd->~T();
		return data;
	}

template <typename T>
	inline T				BasicVector<T, PrimitiveStrategy>::pop()
	{
		usageEnd--;
		return *usageEnd;
	}
	*/

template <typename T>
	inline	T&				CommonVectorStorage<T>::Last()
	{
		BUFFER_ASSERT_NOT_EMPTY();
		return *(usageEnd-1);
	}
	
template <typename T>
	inline	const T&		CommonVectorStorage<T>::Last()	const
	{
		BUFFER_ASSERT_NOT_EMPTY();
		return *(usageEnd-1);
	}

template <typename T>
	inline	T&				CommonVectorStorage<T>::First()
	{
		BUFFER_ASSERT_NOT_EMPTY();
		return *storageBegin;
	}
	
template <typename T>
	inline	const T&		CommonVectorStorage<T>::First()	const
	{
		BUFFER_ASSERT_NOT_EMPTY();
		return *storageBegin;
	}

template <typename T, typename MyStrategy>
	inline BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::operator<<(const T&el)
	{
		EnsureHasSpace();

		new (usageEnd++) T(el);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel++;
			CHK_FILL_LEVEL
		#endif

		return *this;
	}

	#if __BUFFER_RVALUE_REFERENCES__
		template <typename T, typename MyStrategy>
			inline BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::operator<<(T&&el)
			{
				moveAppend(el);
				return *this;
			}
	#endif

template <typename T, typename MyStrategy>
	inline T&		BasicVector<T, MyStrategy>::append(const T&el)
	{
		EnsureHasSpace();

		new (usageEnd++) T(el);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel++;
			CHK_FILL_LEVEL
		#endif

		return *(usageEnd-1);
	}

template <typename T, typename MyStrategy>
	inline	index_t			BasicVector<T,MyStrategy>::appendIfNotFound(const T&el)
	{
		index_t index = Super::GetIndexOf(el);
		if (index == InvalidIndex)
		{
			index = Count();
			append(el);
		}
		return index;
	}


template <typename T, typename MyStrategy>
	inline T&		BasicVector<T, MyStrategy>::moveAppend(T&el)
	{
		EnsureHasSpace();

		MyStrategy::constructSingleFromFleetingData(usageEnd++, el);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel++;
			CHK_FILL_LEVEL
		#endif

		return *(usageEnd-1);
	}


	#if __BUFFER_RVALUE_REFERENCES__
		template <typename T, typename MyStrategy>
			inline T&		BasicVector<T, MyStrategy>::append(T&&el)
			{
				return moveAppend(el);
			}
	#endif


template <typename T, typename MyStrategy>
	inline index_t		BasicVector<T, MyStrategy>::push_back(const T&el)
	{
		EnsureHasSpace();

		new (usageEnd++) T(el);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel++;
			CHK_FILL_LEVEL
		#endif

		return GetLength()-1;
	}

template <typename T, typename MyStrategy> template <typename T2>
	inline BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::appendAddresses(T2*data, count_t elements)
	{
		EnsureHasSpace(elements);
		for (index_t i = 0; i < elements; i++)
			new (usageEnd++) T(data+i);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel+=elements;
			CHK_FILL_LEVEL
		#endif

		return *this;
	}

template <typename T, typename MyStrategy> template <typename Strategy2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::MoveAppendAll(BasicVector<T,Strategy2>&buffer, bool clearSourceOnCompletion/*=true*/)
	{
		if (IsEmpty() && clearSourceOnCompletion)
		{
			swap(buffer);
			return *this;
		}

		moveAppend(buffer.pointer(),buffer.GetLength());
		if (clearSourceOnCompletion)
			buffer.reset();
		return *this;
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::MoveAppendAll(Ctr::ArrayData<T>&array, bool clearSourceOnCompletion/*=true*/)
	{
		moveAppend(array.pointer(),array.GetLength());
		if (clearSourceOnCompletion)
			array.free();
		return *this;
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::moveAppend(T*data, count_t elements)
	{
		EnsureHasSpace(elements);
		MyStrategy::constructRangeFromFleetingData(usageEnd,usageEnd+elements,data);
		usageEnd += elements;
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel+=elements;
			CHK_FILL_LEVEL
		#endif

		return *this;
	}

template <typename T, typename MyStrategy>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::MoveAppend(std::initializer_list<T> items)
	{
		return moveAppend(items.begin(),items.size());
	}

template <typename T, typename MyStrategy> template <typename T2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::AppendAll(std::initializer_list<T2> items)
	{
		return append(items.begin(),items.size());
	}


template <typename T, typename MyStrategy> template <typename T2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::Append(const T2*data, count_t elements)
	{
		EnsureHasSpace(elements);
		for (index_t i = 0; i < elements; i++)
			new (usageEnd++) T(data[i]);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel+=elements;
			CHK_FILL_LEVEL
		#endif

		return *this;
	}

//template <typename T, typename MyStrategy> template <typename T2, typename Strategy2>
//	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::AppendSubList(const BasicVector<T2,Strategy2>&other, index_t offset, count_t maxElements)
//	{
//		if (offset >= other.Count())
//			return *this;
//		maxElements = std::min(maxElements,other.Count() - offset);
//		if (!maxElements)
//			return *this;
//		return append(other.pointer()+offset,maxElements);
//	}
//

template <typename T, typename MyStrategy> //template <typename T2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::AppendAll(const ArrayRef<T>&data)
	{
		return Append(data.pointer(),data.Count());
	}

template <typename T, typename MyStrategy> template <typename T2, typename Strategy2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::AppendAll(const BasicVector<T2,Strategy2>&data)
	{
		return Append(data.pointer(),data.Count());
	}


template <typename T, typename MyStrategy> template <typename T2>
	BasicVector<T, MyStrategy>&		BasicVector<T, MyStrategy>::appendAddresses(MutableArrayRef<T2>&data)
	{
		return appendAddresses(data.pointer(),data.Count());
	}


template <typename T, typename MyStrategy>
	void		BasicVector<T, MyStrategy>::compact()
	{
		if (usageEnd == storageEnd)	// already compacted
			return;
		try
		{
			count_t len = usageEnd-storageBegin;
			T	*new_field = Allocate(len);

			MyStrategy::constructRangeFromFleetingData(new_field, new_field+len, storageBegin);
			DestructAndFree(storageBegin,usageEnd);

			storageBegin = new_field;
			storageEnd = usageEnd = storageBegin+len;
			CHK_FILL_LEVEL
		}
		catch (std::bad_alloc& exception)
		{
			usageEnd = storageBegin = storageEnd = NULL;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				fillLevel = 0;
				CHK_FILL_LEVEL
			#endif
			throw;
		}
	}




template <typename T, typename MyStrategy>
	inline void		BasicVector<T, MyStrategy>::reset()
	{
		MyStrategy::destructRange(storageBegin,usageEnd);
		usageEnd = storageBegin;
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = 0;
			CHK_FILL_LEVEL
		#endif
	}

template <typename T, typename MyStrategy>
	inline void		BasicVector<T, MyStrategy>::clear()
	{
		reset();
	}

template <typename T>
	inline	bool			CommonVectorStorage<T>::empty()						const
	{
		return usageEnd==storageBegin;
	}
template <typename T>
	inline bool	CommonVectorStorage<T>::IsEmpty()					const
	{
		return usageEnd==storageBegin;
	}

template <typename T>
	inline bool	CommonVectorStorage<T>::IsNotEmpty()					const
	{
		return usageEnd!=storageBegin;
	}

template <typename T>
	inline count_t	CommonVectorStorage<T>::GetLength()					const
	{
		return usageEnd-storageBegin;
	}

template <typename T>
	inline	count_t		CommonVectorStorage<T>::GetStorageSize()				const
	{
		return storageEnd-storageBegin;
	}


	

template <typename T>
	inline T&			CommonVectorStorage<T>::operator[](index_t index)
	{
		BUFFER_CHECK_RANGE(index);
		return storageBegin[index];
	}

template <typename T>
	inline const T&	CommonVectorStorage<T>::operator[](index_t index)	const
	{
		BUFFER_CHECK_RANGE(index);
		return storageBegin[index];
	}

template <typename T>
	inline bool	CommonVectorStorage<T>::Owns(const T*element)	const
	{
		return element >= storageBegin && element < usageEnd;
	}

template <typename T>
	inline T&			CommonVectorStorage<T>::at(index_t index)
	{
		BUFFER_CHECK_RANGE(index);
		return storageBegin[index];
	}

template <typename T>
	inline const T&	CommonVectorStorage<T>::at(index_t index)	const
	{
		BUFFER_CHECK_RANGE(index);
		return storageBegin[index];
	}

template <typename T>
	inline T&			CommonVectorStorage<T>::GetFromEnd(index_t index)
	{
		BUFFER_CHECK_RANGE(index);
		return *(usageEnd-index-1);
	}

template <typename T>
	inline const T&	CommonVectorStorage<T>::GetFromEnd(index_t index)	const
	{
		BUFFER_CHECK_RANGE(index);
		return *(usageEnd-index-1);
	}

template <typename T>
	inline T*				CommonVectorStorage<T>::pointer()
	{
		return storageBegin;
	}

template <typename T>
	inline const T*		CommonVectorStorage<T>::pointer()			const
	{
		return storageBegin;
	}

template <typename T>
	template <typename IndexType>
		inline T*				CommonVectorStorage<T>::operator+(IndexType delta)
		{
			BUFFER_CHECK_RANGE((index_t)delta);
			return storageBegin+delta;
		}

template <typename T>
	template <typename IndexType>
		inline const T*		CommonVectorStorage<T>::operator+(IndexType delta)		const
		{
			BUFFER_CHECK_RANGE((index_t)delta);
			return storageBegin+delta;
		}

template <typename T, typename MyStrategy>
	bool			BasicVector<T, MyStrategy>::Truncate(count_t fillLevel)
	{
		count_t cfill = usageEnd-storageBegin;
		if (fillLevel < cfill)
		{
			MyStrategy::destructRange(storageBegin+fillLevel,usageEnd);
			usageEnd = storageBegin+fillLevel;
			#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
				this->fillLevel = fillLevel;
				CHK_FILL_LEVEL
			#endif

			return true;
		}
		return false;
	}

template <typename T, typename MyStrategy>
	void			BasicVector<T, MyStrategy>::adoptData(BasicVector<T,MyStrategy>&other)
	{
		if (&other == this)
			return;
		DestructAndFree(storageBegin,usageEnd);

		storageBegin = other.storageBegin;
		usageEnd = other.usageEnd;
		storageEnd = other.storageEnd;
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel = other.fillLevel;
			other.fillLevel = 0;
			CHK_FILL_LEVEL
		#endif

		other.storageBegin = other.usageEnd = other.storageEnd = NULL;
	}

template <typename T>
	void			CommonVectorStorage<T>::swap(Self&other)
	{
		swp(storageBegin,other.storageBegin);
		swp(usageEnd,other.usageEnd);
		swp(storageEnd,other.storageEnd);
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			swp(fillLevel,other.fillLevel);
			CHK_FILL_LEVEL
		#endif
	}

template <typename T, typename MyStrategy>
	void			BasicVector<T, MyStrategy>::copyToArray(Ctr::ArrayData<T>&target)	const
	{
		target.SetSize(usageEnd-storageBegin);
		const T*from = storageBegin;
		T*to = target.pointer();
		MyStrategy::template copyRange<const T,T>(from,usageEnd,to);
		/*while (from != usageEnd)
			(*to++) = (*from++);
		ASSERT_CONCLUSION(target,to);*/
	}

template <typename T, typename MyStrategy>
	Ctr::Array<T,MyStrategy>			BasicVector<T, MyStrategy>::copyToArray()	const
	{
		return Ctr::Array<T,MyStrategy>(storageBegin,usageEnd-storageBegin);
	}


template <typename T, typename MyStrategy>
	void			BasicVector<T, MyStrategy>::moveToArray(Ctr::ArrayData<T>&target, bool reset_buffer)
	{
		target.SetSize(usageEnd-storageBegin);
		T*from = storageBegin;
		T*to = target.pointer();
		MyStrategy::moveRange(from,usageEnd,to);
		/*while (from != usageEnd)
			Strategy::move(*from++,*to++);
		ASSERT_CONCLUSION(target,to);*/
		if (reset_buffer)
			reset();
	}

template <typename T>
	inline bool				CommonVectorStorage<T>::operator==(const Self&other) const
	{
		count_t len = usageEnd - storageBegin;
		if (len != other.usageEnd - other.storageBegin)
			return false;
		for (index_t i = 0; i < len; i++)
			if (storageBegin[i] != other.storageBegin[i])
				return false;
		return true;
	}

template <typename T>
	inline bool				CommonVectorStorage<T>::operator!=(const Self&other) const
	{
		return !operator==(other);
	}

	
template <typename T>
	template <typename T2>
		inline	bool		CommonVectorStorage<T>::Contains(const T2&element)	const
		{
			const T*at = storageBegin;
			while (at != usageEnd)
			{
				if (*at == element)
					return true;
				at++;
			}
			return false;
		}

template <typename T>
	template <typename T2>
		inline	index_t		CommonVectorStorage<T>::GetIndexOf(const T2&element)	const
		{
			const T*at = storageBegin;
			while (at != usageEnd)
			{
				if (*at == element)
					return at-storageBegin;
				at++;
			}
			return InvalidIndex;
		}

template <typename T, typename MyStrategy>
	template <typename T2>
			bool			BasicVector<T, MyStrategy>::findAndErase(const T2&element)
			{
				index_t index = Super::GetIndexOf(element);
				if (index == InvalidIndex)
					return false;
				Erase(index);
				return true;
			}

template <typename T, typename MyStrategy>
	inline	typename BasicVector<T,MyStrategy>::iterator		BasicVector<T, MyStrategy>::erase(iterator it)
	{
		index_t index = it - storageBegin;
		if (index >= (count_t)(usageEnd-storageBegin))
			return end();
		Erase(index);
		return storageBegin + index;
	}


template <typename T, typename MyStrategy>
	inline	void			BasicVector<T, MyStrategy>::Erase(index_t index)
	{
		if (index >= (count_t)(usageEnd-storageBegin))
			return;
		T*c = storageBegin+index;
		while (c+1 != usageEnd)
		{
			MyStrategy::move(*(c+1),*c);
			c++;
		}
		usageEnd--;
		usageEnd->~T();
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel--;
			CHK_FILL_LEVEL
		#endif

	}
	

template <typename T, typename MyStrategy>
	inline	void			BasicVector<T, MyStrategy>::Erase(index_t index, index_t elements)
	{
		if (index >= (count_t)(usageEnd-storageBegin) || !elements)
			return;
		if (index+elements > (count_t)(usageEnd-storageBegin))
			elements = (count_t)(usageEnd-storageBegin) - index;

		T*c = storageBegin+index;
		while (c+elements != usageEnd)
		{
			MyStrategy::move(*(c+elements),*c);
			c++;
		}

		MyStrategy::destructRange(c,usageEnd);
		usageEnd-=elements;
		#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
			fillLevel-=elements;
			CHK_FILL_LEVEL
		#endif

	}



#endif
