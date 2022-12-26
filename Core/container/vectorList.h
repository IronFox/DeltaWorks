#ifndef bufferH
#define bufferH

#include "common.h"
#include <exception>
#include <new>
#include <algorithm>
#include <initializer_list>

#include "strategy.h"
#include "array.h"

#define __BUFFER_RVALUE_REFERENCES__	1		//!< Allows && references in constructors and assignment operators
#define __BUFFER_DBG_RANGE_CHECK__		1		//!< Checks index ranges before access and issues a FATAL__ in case of a violation (DEBUG MODE ONLY)
#define __BUFFER_DBG_COUNT__			1		//!< Maintains a count_t variable that provides the number of elements in the buffer (DEBUG MODE ONLY)


namespace DeltaWorks
{

	namespace Container
	{
		/**
		Base vector storage type that holds all pointers and allows some operations.
		It does not know any used memory strategy and hence cannot destroy the referenced data on its own.
		NEVER USE ON ITS OWN
		*/
		template <typename T>
			class CommonVectorStorage
			{
				typedef CommonVectorStorage<T>	Self;
			protected:
				T						*storageBegin,		//!< Pointer to the first element of the storage area. May be NULL indicating an empty buffer
										*storageEnd,		//!< Pointer one past the last valid element of the storage area. May be NULL indicating an empty buffer. (storage_end-storageBegin) stays valid even if the buffer is empty
										*usageEnd;			//!< Pointer one past the last allocated element of the storage area. May be NULL indicating an empty buffer. (usage_end-storageBegin) stays valid even if the buffer is empty
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					count_t				fillLevel;			//!< Current fill level in elements. Maintained in debug mode only
				#endif


				inline static	T*		Allocate(count_t len);		//!< Allocates the specified number of elements via malloc. No constructors are called. The function returns NULL if len is 0. std::bad_alloc may be thrown if allocation fails
				inline static	T*		AllocateNotEmpty(count_t len);	//!< Identical to the above but without length check. @a len must not be 0. passing 0 causes undefined behavior
				
				explicit				CommonVectorStorage(count_t len);			//!< Constructor \param len Initial buffer size in elements.
				/**/					CommonVectorStorage(const Self&other);
				#if __BUFFER_RVALUE_REFERENCES__
					/**/				CommonVectorStorage(Self&&other);
				#endif
				/**/					CommonVectorStorage(std::initializer_list<T> items);

			public:
				typedef T*				iterator;
				typedef const T*		const_iterator;


				virtual					~CommonVectorStorage() {DBG_ASSERT_IS_NULL__(storageBegin);DBG_ASSERT_IS_NULL__(storageEnd);DBG_ASSERT_IS_NULL__(usageEnd);DBG_ASSERT__(fillLevel == 0);};
				MutableArrayRef<T>		ToRef() {return MutableArrayRef<T>(storageBegin,Count());}
				ArrayRef<T>				ToRef() const {return ArrayRef<T>(storageBegin,Count());}
				MutableArrayRef<T>		SubRef(index_t start, count_t count=InvalidIndex) {if (start >= Count()) return MutableArrayRef<T>(); return MutableArrayRef<T>(storageBegin+start,std::min(count, Count()-start));}
				ArrayRef<T>				SubRef(index_t start, count_t count=InvalidIndex) const {if (start >= Count()) return ArrayRef<T>(); return ArrayRef<T>(storageBegin+start,std::min(count, Count()-start));}
				void					Fill(const T&pattern);					//!< Copies \b pattern to each element of the local buffer

				inline count_t			GetLength()					const;	//!< Returns the number of elements currently constructed in the local container. Reserved (non-constructed) objects are not counted
				inline count_t			GetCount()					const	/**@copydoc GetLength()*/ {return GetLength();}
				inline count_t			size()						const	/**@copydoc GetLength()*/ {return GetLength();}
				inline count_t			Count()						const	/**@copydoc GetLength()*/ {return GetLength();}
				inline count_t			operator()()				const	/**@copydoc GetLength()*/ {return GetLength();}
				inline count_t			GetStorageSize()			const;	//!< Queries the number of elements held in total. This also includes unconstructed incremental storage
				inline size_t			GetContentSize()			const {return Count() * sizeof(T);}
				inline bool				empty()						const;	//!< @copydoc IsEmpty()
				inline bool				IsEmpty()					const;	//!< Returns true if the buffer holds no elements
				inline bool				IsNotEmpty()				const;	//!< Returns true if the buffer holds at least one element
				inline T&				First();							//!< Retrieves a reference to the first element in the buffer. The behavior of this method is undefined if the local buffer is empty
				inline const T&			First()	const;						//!< @overload
				inline T&				Last();								//!< Retrieves a reference to the last element in the buffer. The behavior of this method is undefined if the local buffer is empty
				inline const T&			Last()	const;						//!< @overload
				template <typename T2>
					inline bool			Contains(const T2&element)	const;	//!< Determines whether or not an equivalent to the specified element is currently stored in the active region of the buffer. Comparison is done via the ==operator.
				template <typename T2>
					inline index_t		GetIndexOf(const T2&element)	const;	//!< Determines the index of the specified element via == operator. If no match was found then index_t(-1)/InvalidIndex is returned
				inline iterator			begin()	{return storageBegin;}
				inline const_iterator	begin()	const {return storageBegin;}
				inline iterator			end()	{return usageEnd;}
				inline const_iterator	end() const {return usageEnd;}
				inline T*				pointer();							//!< Returns a pointer to the beginning of the buffer
				inline const T*			pointer()					const;	//!< Returns a pointer to the beginning of the buffer
				inline T*				GetPointer() {return pointer();}
				inline const T*			GetPointer() const {return pointer();}
				inline T&				operator[](index_t);
				inline const T&			operator[](index_t)			const;
				inline T&				at(index_t);
				inline const T&			at(index_t)			const;

				inline bool				Owns(const T*element)	const;	//! Queries if the specified entry pointer was taken from the local buffer. Actual pointer address is checked, not what it points to.

				inline bool				operator==(const CommonVectorStorage<T>&other) const;
				inline bool				operator!=(const CommonVectorStorage<T>&other) const;

				inline T&				GetFromEnd(index_t);					//!< Retrieves the nth element from the end of the consumed buffer space. GetFromEnd(0) is identical to Last()
				inline const T&			GetFromEnd(index_t)			const;	//!< @copydoc GetFromEnd()

				void					swap(CommonVectorStorage<T>&other);			//!< Swaps data with the other buffer
				template <typename IndexType>
					inline T*			operator+(IndexType delta);				//!< Returns a pointer to the beginning of the buffer plus the specified delta
				template <typename IndexType>
					inline const T*		operator+(IndexType delta)		const;	//!< Returns a pointer to the beginning of the buffer plus the specified delta


				friend void swap(Self& a, Self& b)
				{
					a.swap(b);
				}
			};


		/**
		Basic dynamic list structure using the vector allocation principle (double allocated memory each time allocated space is full).
		@param T Contained type. Must support empty construction, operator=, and operator==. Depending on the used memory strategy, other methods may be required
		@param MyStrategy Memory strategy plugin. May accelerate certain operations if the contained object supports them.
			E.g. using the Swap or Adopt strategies makes memory relocations faster for objects with dynamic content, where swap() or adoptData() methods exist.
			Except for special circumstances, it is suggested to not change the default strategy but instead define rvalue constructors and operator=(),
			as the default (Copy) strategy will use them where appropriate. Only heavily dynamic objects may benefit from explicitly using the Swap
			strategy due to the otherwise added overhead of needlessly destroying local dynamic content in rvalue operator=() implementations.
		*/
		template <typename T, typename MyStrategy=typename StrategySelector<T>::Default>
			class BasicVector : public CommonVectorStorage<T>
			{
			protected:
				typedef CommonVectorStorage<T>	Super;


				inline void				EnsureHasSpace(count_t elements);	//!< Ensures that at least the specified number of non-allocated elements is available from @a usage_end to @a storage_end . The local storage may be resized/allocated if necessary. Element movement behavior is described by the passed @a Strategy struct. The requested elements are not constructed. std::bad_alloc may be thrown
				inline void				EnsureHasSpace();					//!< Similar to the above. Ensures that at least one more non-allocated element is available.

	
				inline void				GenericCompactify();				//!< Reduces storage size to the number of currently allocated elements. If the storage must be reallocated then elements are moved as described in the specified @a Strategy struct. std::bad_alloc may be thrown
				static inline void		DestructAndFree(T*range_begin, T*range_end);	//!< Invokes destructors for all elements from @a range_begin to @a range_end excluding @a range_end . Then invokes free(@a range_begin ). @a range_begin may be NULL if @a range_end is too.

			public:
				typedef BasicVector<T,MyStrategy>		Self;
				using Super::iterator;

				explicit				BasicVector(count_t len);			//!< Constructor \param len Initial buffer size in elements.
				/**/					BasicVector(const Self&other);
				#if __BUFFER_RVALUE_REFERENCES__
					/**/				BasicVector(Self&&other);
					/**/				BasicVector(Super&&other):Super(std::forward<Super>(other)){}
				#endif
				/**/					BasicVector(std::initializer_list<T> items);
				virtual				   ~BasicVector();
				Self&					operator=(const ArrayRef<T>&array);
				Self&					operator=(const Self&other);
				Self&					operator=(std::initializer_list<T> items);
				#if __BUFFER_RVALUE_REFERENCES__
					Self&				operator=(Self&&other);
					Self&				operator=(Super&&other) { Super::adoptData(other); return *this; }
				#endif

				inline Self&			operator<<(const T&element);			//!< Appends \b element to the end of the buffer, advancing the buffer cursor by one. The buffer will automatically be doubled in size if it is full at the time this operator is invoked.
				#if __BUFFER_RVALUE_REFERENCES__
					inline Self&		operator<<(T&&element);			//!< Appends \b element to the end of the buffer, advancing the buffer cursor by one. The buffer will automatically be doubled in size if it is full at the time this operator is invoked.
				#endif
				inline index_t			push_back(const T&element);				//!< @copydoc operator<<() @return Index of the appended element (buffer fill level prior to appending)
				inline void				clear(count_t len);						//!< Reallocates the buffer to the specified size and resets the buffer cursor. Any stored data is destructed.
				inline void				clear();								//!< @copydoc reset()
				inline void				Clear(count_t len)	/**@copydoc clear(count_t len)*/ {clear(len);}
				inline void				Clear()				/**@copydoc clear()*/ {clear();}
				void					ResizePreserveContent(count_t len);		//!< Resizes the local buffer size but preserves the old content and fill level where possible. If the old fill level exceeds the new size then all new elements will be occupied and initialized
				template <typename Strategy2>
					Self&				MoveAppendAll(BasicVector<T,Strategy2>&buffer, bool clearSourceOnCompletion=true);	//!< Appends all elements in the specified other buffer to the end of the local buffer. The elements will be moved, leaving the parameter buffer empty upon completion.
				Self&					MoveAppendAll(Ctr::ArrayData<T>&array, bool clearSourceOnCompletion=true);					//!< Appends all elements in the specified array to the end of the local buffer. The elements will be moved, leaving the parameter array empty upon completion.	
				Self&					moveAppend(T*data, count_t elements);			//!< Appends all elements in the specified range to the end of the local buffer. The elements will be moved, leaving the individual objects of the parameter field empty upon completion.
				inline Self&			MoveAppend(T*data, count_t elements)	/**@copydoc moveAppend()*/ {return moveAppend(data,elements);}
				inline Self&			MoveAppend(std::initializer_list<T> items);
				template <typename T2>
					Self&				Append(const T2*data, count_t elements);	//!< Appends a number of elements to the end of the buffer, advancing the buffer cursor by the specified number of elements. The buffer will automatically be resized if necessary.
				template <typename T2>
					Self&				AppendAll(std::initializer_list<T2> items);
				Self&					AppendAll(const ArrayRef<T>&array);				//!< Appends a number of elements to the end of the buffer, advancing the buffer cursor by the specified array's contained of elements. The buffer will automatically be resized if necessary.
				template <typename T2, typename Strategy2>
					Self&				AppendAll(const BasicVector<T2,Strategy2>&buffer);	//!< Appends a number of elements to the end of the buffer, advancing the buffer cursor by the specified array's contained of elements. The buffer will automatically be resized if necessary.
				template <typename T2>
					Self&				appendAddresses(T2*data, count_t elements);	//!< Appends the addresses of the specified elements to the end of the buffer (assuming T is a pointer type fo T2). The buffer will automatically be resized if necessary.
				template <typename T2>
					inline Self&		AppendAddresses(T2*data, count_t elements)	/**@copydoc appendAddresses()*/ {return appendAddresses(data,elements);}
				template <typename T2>
					Self&				appendAddresses(MutableArrayRef<T2>&array);	//!< Appends the addresses of the specified array's elements to the end of the buffer (assuming T is a pointer type fo T2). The buffer will automatically be resized if necessary.
				template <typename T2>
					inline Self&		AppendAddresses(MutableArrayRef<T2>&array)	/**@copydoc appendAddresses()*/ {return appendAddresses(array);}
				template <typename T2>
					Self&				appendVA(count_t elements, ...);		//!< Appends a number of elements to the buffer
				inline void				reset();							//!< Resets the buffer cursor to the beginning. Does \b not resize the local buffer.
				inline void				Reset()								/**@copydoc reset()*/	{reset();}
				bool					Truncate(count_t count);			//!< Decrements the local buffer counter to the specified count. The method fails if the local buffer count is less or equal the specified count. The actually allocated buffer size remains unchanged.
				inline T*				appendRow(count_t length);			//!< Appends a number of elements and returns a pointer to the first element. The method returns NULL, if length is 0 @param length Number of elements to append. Must be greater 0 @return Pointer to the first of the appended elements or NULL, if an error occured
				inline T*				AppendRow(count_t length)			/**@copydoc appendRow()*/ {return appendRow(length);}
				inline T*				appendRow(count_t length, const T&pattern);			//!< Appends a number of elements and returns a pointer to the first element. The method returns NULL, if length is 0 @param length Number of elements to append. Must be greater 0 @param pattern Data to Fill newly appended elements with @return Pointer to the first of the appended elements or NULL, if an error occured
				inline T*				AppendRow(count_t length, const T&pattern)			/**@copydoc appendRow()*/ {return appendRow(length,pattern);}
				inline T&				Append();							//!< Appends a single element and returns a reference to it
				inline T&				append(const T&);					//!< Appends a single element via copy, and returns a reference to it in buffer storage
				inline T&				Append(const T&element)				/**@copydoc Append()*/	{return append(element);}
				inline index_t			appendIfNotFound(const T&);			//!< Appends a single element via copy, assuming an equal element does not already exist in the local buffer (compared via == operator) @return Index of the first found or the newly appended element
				inline index_t			AppendIfNotFound(const T&element)	/**@copydoc appendIfNotFound()*/	{return appendIfNotFound(element);}
				inline T&				moveAppend(T&);						//!< Appends a single element via move, and returns a reference to it in buffer storage
				inline T&				MoveAppend(T&element)				/**@copydoc moveAppend()*/	{return moveAppend(element);}
				#if __BUFFER_RVALUE_REFERENCES__
					inline T&			append(T&&);						//!< Appends a single element via move, and returns a reference to it in buffer storage
					inline T&			Append(T&&element)					/**@copydoc Append()*/ {return append(std::forward<T>(element));}
					inline T&			MoveAppend(T&&element)				/**@copydoc moveAppend()*/ { return append(std::forward<T>(element)); }
				#endif

				inline T&				Insert(index_t before_element);		//!< Inserts an element into the buffer and returns a reference to it
				template <typename T2>
					inline T&			Insert(index_t before_element, const T2&init_data);	//!< Inserts an element into the buffer and returns a reference to it. @a init_data is passed to the constructor or assignment operator of the new element
				inline T*				insertRow(index_t before_element, count_t length);		//!< Appends a number of elements and returns a pointer to the first element. The method returns NULL, if length is 0 @param length Number of elements to append. Must be greater 0 @return Pointer to the first of the appended elements or NULL, if an error occured
				inline T*				InsertRow(index_t before_element, count_t length)		/**@copydoc insertRow()*/ {return insertRow(before_element,length);}

				inline T				pop();								//!< Pops the last element from the buffer (the internally allocated memory section is not resized, only the local fill level decremented and the last object destructed). The method behavior is undefined if the buffer is empty.
				inline T				Pop()								/**@copydoc pop()*/ {return pop();}
				inline void				eraseLast();						//!< Simplified void-version of pop(). Can be more efficient if the contained type is complex and the returned object not used anyway. The method behavior is undefined if the buffer is empty.
				inline void				EraseLast()							/**@copydoc eraseLast()*/ {eraseLast();}
				inline void				Erase(index_t index);				//!< Removes the specified element from the buffer. The buffer's contained element count decreases by one if the specified index is valid.
				inline void				Erase(index_t index, index_t elements);		//!< Removes a range of elements from the buffer. The buffer's contained element count decreases by one if the specified index is valid. @a elements may be reduced if the range surpasses the end of the consumed buffer space

				inline iterator			erase(iterator it);

				inline void				Revert();							//!< Flips the order of local elements. Uses local strategy to swap elements
				void					compact();							//!< Reduces the local buffer size to the exact fill level and copies all contained elements. Any succeeding push operation will automatically increase buffer size again. The method returns if the stack is already of compact size
				inline void				Compact()							/**@copydoc compact()*/	{compact();}
				Ctr::Array<T,MyStrategy>		copyToArray()						const;	//!< Exports the local data up to the current fill level to the returned array.
				inline Ctr::Array<T,MyStrategy>CopyToArray()						const	/**@copydoc copyToArray()*/	{return copyToArray();}
				void					copyToArray(Ctr::ArrayData<T>&target)	const;	//!< Exports the local data up to the current fill level to the specified array. The target array will be resized if necessary
				inline void				CopyToArray(Ctr::ArrayData<T>&target)	const	/**@copydoc copyToArray()*/	{copyToArray(target);}
				void					moveToArray(Ctr::ArrayData<T>&target, bool clearBuffer=true);		//!< Moves local data up to the current fill level to the specified array. Move behavior is defined by the used Strategy class. @param reset_buffer Set true to automatically reset the buffer once element movement is completed
				inline void				MoveToArray(Ctr::ArrayData<T>&target, bool clearBuffer=true)	/**@copydoc moveToArray()*/	{moveToArray(target,clearBuffer);}
				template <typename T2>
					bool				findAndErase(const T2&element);		//!< Attempts to find and erase the specified element @return true if the specified element could be found (via ==operator), false otherwise
				template <typename T2>
					inline bool			FindAndErase(const T2&element)		/**@copydoc findAndErase()*/	{return findAndErase(element);}

				void					adoptData(BasicVector<T,MyStrategy>&other);	//!< Adopts all attributes of the specified other buffer leaving it empty. Any existing local data is deleted.

				using Super::Count;
				using Super::GetLength;
				using Super::IsEmpty;
				using Super::begin;
				using Super::end;
				using Super::swap;


			protected:
				using Super::storageEnd;
				using Super::storageBegin;
				using Super::usageEnd;
				#if defined(_DEBUG) && __BUFFER_DBG_COUNT__
					using Super::fillLevel;
				#endif

				using Super::Allocate;
				using Super::AllocateNotEmpty;

			};



		/**
		General dynamic list structure using the vector allocation principle (double allocated memory each time allocated space is full).
		@param T Contained type. Must support empty construction, operator=, and operator==. Depending on the used memory strategy, other methods may be required
		@param InitialLength Number of elements allocated in the local buffer prior to adding the first element. Only the space for these objects is allocated. No actual object allocation is performed
		@param MyStrategy Memory strategy plugin. May accelerate certain operations if the contained object supports them.
			E.g. using the Swap or Adopt strategies makes memory relocations faster for objects with dynamic content, where swap() or adoptData() methods exist.
			Except for special circumstances, it is suggested to not change the default strategy but instead define rvalue constructors and operator=(),
			as the default (Copy) strategy will use them where appropriate. Only heavily dynamic objects may benefit from explicitly using the Swap
			strategy due to the otherwise added overhead of needlessly destroying local dynamic content in rvalue operator=() implementations.
		*/
		template <typename T, count_t InitialLength=16, typename MyStrategy=typename StrategySelector<T>::Default>
			class PreAllocVector:public BasicVector<T,MyStrategy>
			{
			public:
				typedef BasicVector<T,MyStrategy>				Super;
				typedef PreAllocVector<T,InitialLength,MyStrategy>	Self;
				typedef	MyStrategy							AppliedStrategy;


				explicit			PreAllocVector(count_t len=InitialLength):Super(len)
									{}
									PreAllocVector(const Super&other):Super(other)
									{}
									PreAllocVector(const Self&other):Super(other)
									{}
				#if __BUFFER_RVALUE_REFERENCES__
					template<count_t Len>
									PreAllocVector(PreAllocVector<T,Len,MyStrategy>&&other):Super(std::forward<Super>(other))
									{}
									PreAllocVector(Self && other) :Super(std::forward<Super>(other))
									{}
									PreAllocVector(Super&&other) :Super(std::forward<Super>(other))
									{}
									PreAllocVector(typename Super::Super&&other):Super(std::forward<typename Super::Super>(other))
									{}
					Self&			operator=(Self&&other) { Super::adoptData(other); return *this; }
					Self&			operator=(Super&&other) { Super::adoptData(other); return *this; }
				#endif
				Self&				operator=(const ArrayRef<T>&array){Super::operator=(array); return *this;}
				Self&				operator=(const Self&other) { Super::operator=(other); return *this; }
				Self&				operator=(const Super&other) { Super::operator=(other); return *this; }

				friend void swap(Self& a, Self& b)
				{
					a.swap(b);
				}

				using Super::operator<<;
				using Super::operator[];
				using Super::operator+;
			};

		/**
		General dynamic list structure using the vector allocation principle (double allocated memory each time allocated space is full).
		Allocates NO elements in advance, making the constructor fast, and the memory footprint low. Good for large data types, or lists in lists.
		@param T Contained type. Must support empty construction, operator=, and operator==. Depending on the used memory strategy, other methods may be required
		@param MyStrategy Memory strategy plugin. May accelerate certain operations if the contained object supports them.
			E.g. using the Swap or Adopt strategies makes memory relocations faster for objects with dynamic content, where swap() or adoptData() methods exist.
			Except for special circumstances, it is suggested to not change the default strategy but instead define rvalue constructors and operator=(),
			as the default (Copy) strategy will use them where appropriate. Only heavily dynamic objects may benefit from explicitly using the Swap
			strategy due to the otherwise added overhead of needlessly destroying local dynamic content in rvalue operator=() implementations.
		*/
		template <typename T, typename MyStrategy=typename StrategySelector<T>::Default>
			class Vector:public PreAllocVector<T,0,MyStrategy>
			{
			public:
				typedef PreAllocVector<T, 0, MyStrategy>				Super;
				typedef Vector<T,MyStrategy>					Self;
				typedef	MyStrategy							AppliedStrategy;


				explicit			Vector() :Super(0)
									{}
									Vector(const Super&other):Super(other)
									{}
									Vector(const Self&other):Super(other)
									{}
				#if __BUFFER_RVALUE_REFERENCES__
					template<count_t Len>
									Vector(PreAllocVector<T,Len,MyStrategy>&&other):Super(std::forward<PreAllocVector<T,Len,MyStrategy> >(other))
									{}
									Vector(Self&&other):Super(std::forward<Super>(other))
									{}
									Vector(Super&&other):Super(std::forward<Super>(other))
									{}
									Vector(typename Super::Super&&other):Super(std::forward<typename Super::Super>(other))
									{}
									Vector(typename Super::Super::Super&&other):Super(std::forward<typename Super::Super::Super>(other))
									{}
					Self&			operator=(Self&&other){Super::adoptData(other); return *this;}
					Self&			operator=(Super&&other){Super::adoptData(other); return *this;}
					Self&			operator=(typename Super::Super&&other) { Super::adoptData(other); return *this; }
				#endif
				Self&				operator=(const ArrayRef<T>&array){Super::operator=(array); return *this;}
				Self&				operator=(const Self&other) { Super::operator=(other); return *this; }
				Self&				operator=(const Super&other) { Super::operator=(other); return *this; }

				friend void swap(Self& a, Self& b)
				{
					a.swap(b);
				}

				using Super::operator<<;
				using Super::operator[];
				using Super::operator+;
			};
		template <count_t InitialLength=1024>
			class WriteBuffer:public PreAllocVector<BYTE,InitialLength,PrimitiveStrategy>, public IWriteStream
			{
			public:
				typedef PreAllocVector<BYTE,InitialLength,PrimitiveStrategy>	Super;

				virtual	bool		write(const void*data, serial_size_t size)
									{
										Super::Append((const BYTE*)data,static_cast<size_t>(size));
										return true;
									}		
			};


		#include "vectorList.tpl.h"


		//template <typename T, typename MyStrategy=typename StrategySelector<T>::Default>
		//	using Vector = Vector<T,MyStrategy>;
		//template <typename T, count_t InitialLength=16, typename MyStrategy=typename StrategySelector<T>::Default>
		//	using Vector = PreAllocVector<T,InitialLength,MyStrategy>;

	}

	template <typename T, typename MyStrategy>
		class StrategySelector<Container::BasicVector<T, MyStrategy> >
		{
		public:
			typedef	SwapStrategy		Default;
		};

	template <typename T, count_t InitialLength, typename MyStrategy>
		class StrategySelector<Container::PreAllocVector<T, InitialLength, MyStrategy> >
		{
		public:
			typedef	SwapStrategy		Default;
		};

	template <typename T, typename MyStrategy>
		class StrategySelector<Container::Vector<T, MyStrategy> >
		{
		public:
			typedef	SwapStrategy		Default;
		};


	template <count_t InitialLength>
		class StrategySelector<Container::WriteBuffer<InitialLength> >
		{
		public:
			typedef	SwapStrategy		Default;
		};


	
}

#endif
