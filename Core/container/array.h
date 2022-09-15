#ifndef arrayH
#define arrayH

#include "common.h"
#include <wchar.h>
#include "../interface/serializable.h"
#include "strategy.h"
#include "../interface/hashable.h"
#include <cstdarg>
#include <array>

#include "../general/typedefs.h"

#define __ARRAY_RVALUE_REFERENCES__	1		//!< Allows && references in constructors and assignment operators


#ifdef _DEBUG
#define __ARRAY_DBG_RANGE_CHECK__		//!< Checks index ranges in debug mode
#endif

#if SYSTEM==WINDOWS
	#define wcswcs	wcsstr
#endif

#undef ASSERT_CONCLUSION
#ifdef _DEBUG
	#define ASSERT_CONCLUSION(array,pointer)	{if (!(array).IsConcludedBy(pointer)) FATAL__("Conclusion exception for array '" #array "', element '" #pointer "' ("+String((array).GetIndexOfPointer(pointer))+"/"+String((array).GetLength())+")"); }
#else
	#define ASSERT_CONCLUSION(array,pointer)
#endif


namespace DeltaWorks
{

	/**
		@brief Experimental property helper macro

		This macro should allow to place property like single instance classes
		into a parent class. Overhead warning. Deprecated style.
	*/
	#define PROPERTY(TYPE,NAME,PARENT_CLASS,GET,SET)\
				class {\
					public:\
					inline PARENT_CLASS*	This()\
					{\
						return (PARENT_CLASS*)((BYTE*)this-((BYTE*)&((PARENT_CLASS*)1)->NAME-1));\
					}\
					inline const PARENT_CLASS*	This()	const\
					{\
						return (const PARENT_CLASS*)((const BYTE*)this-((const BYTE*)&((const PARENT_CLASS*)1)->NAME-1));\
					}\
					inline operator TYPE() const\
					{\
						return This()->GET();\
					}\
					inline void operator=(TYPE value)\
					{\
						This()->SET(value);\
					}\
				} NAME;



	namespace Container
	{

		template <typename T>
			class MutableArrayRef;
		template <typename T>
			class ArrayRef;

		/**
			@brief Array with a fixed number of elements
			FixedArray 
		*/
		template <typename T, size_t Length, class MyStrategy=typename StrategySelector<T>::Default>
			class FixedArray:public std::array<T,Length>
			{
			public:	
				typedef FixedArray<T,Length,MyStrategy>	Self;
				typedef std::array<T,Length>		Super;
			
				/**/					FixedArray()	{}
				/**/					FixedArray(std::initializer_list<T> l)
				{
					index_t at = 0;
					for (auto i = l.begin(); i != l.end(); ++i)
					{
						if (at >= Length)
							break;
						Super::operator[](at++) = *i;
					}
				}
				/**/					FixedArray(const Self&other):Super(other)	{}
				/**/					FixedArray(Self&&other):Super(std::move(other))	{}
		
				constexpr count_t		Count() const {return Super::size();}

				/**
				Executes Strategy::swap() on all elements
				*/
				void					swap(Self&other)
				{
					for (auto i0 = Super::begin(), i1 = other.begin(); i0 != Super::end(); ++i0, ++i1)
						MyStrategy::swap(*i0,*i1);
				}

				friend void				swap(Self&a, Self&b)
				{
					a.swap(b);
				}

				T*						GetPointer()
				{
					return &*Super::begin();
				}
				
				const T*				GetPointer() const
				{
					return &*Super::begin();
				}

				T&						First()
				{
					return Super::front();
				}
				const T&				First() const
				{
					return Super::front();
				}
				T&						Last()
				{
					return Super::back();
				}
				const T&				Last() const
				{
					return Super::back();
				}

				/**
				Executes Strategy::move() on all elements
				*/
				void					adoptData(Self&other)
				{
					for (typename Super::iterator i0 = Super::begin(), i1 = other.begin(); i0 != Super::end(); ++i0, ++i1)
						MyStrategy::move(*i1,*i0);
				}

				void					Fill(const T&pattern)
				{
					Super::fill(pattern);
				}

				friend void			Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					for (typename Super::const_iterator i = v.begin(); i != v.end(); ++i)
						Serialize(s,*i);
				}
				friend void			Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					for (typename Super::iterator i = v.begin(); i != v.end(); ++i)
						Deserialize(s,*i);
				}


				constexpr size_t		GetContentSize() const {return sizeof(T)*Length;}

				void					operator=(Self&&other)
				{
					adoptData(other);
				}
				void					operator=(const Self&other)
				{
					Super::operator=(other);
				}

				void					operator=(const ArrayRef<const T>&other);

				bool					operator==(const Self&other) const
				{
					return std::equal(Super::begin(), Super::end(),other.begin());
				}

				bool					operator!=(const Self&other) const
				{
					return !operator==(other);
				}

				T&						operator[](index_t index)
				{
					return Super::operator[](index);
				}

				const T&						operator[](index_t index) const
				{
					return Super::operator[](index);
				}

				T*						operator+(index_t index)
				{
					return &*(Super::begin() + index);
				}
				const T*				operator+(index_t index) const
				{
					return &*(Super::begin() + index);
				}

				MutableArrayRef<T>				ToRef();
			};


		namespace Compare
		{
			template <typename T0, typename T1=T0>
				inline int		Primitives(const T0&a, const T1&b)
				{
					if (a < b)
						return -1;
					if (a > b)
						return 1;
					return 0;
				}
		}


		class Arrays	//! Defines a number of array specific constants and types
		{
		public:
					typedef size_t			count_t,index_t;				//!< Array element count type
					static const count_t	Undefined=count_t(-1);	//!< Undefined length value
		};


		template <typename T>
			class FixedSizeSerializable
			{
			public:
				static size_t		GetSize()
				{
					static T		testElement;	//!< Used to test serializability
					return GetFixedSize(&testElement);
				}
			};


		template <typename T>
			class ArrayRef
			{
			protected:
				T*				data;
				count_t			elements;

				friend class ArrayRef<const T>;

			public:
				typedef ArrayRef<T>	Self;
				typedef const T*	const_iterator;


				/**/				ArrayRef():data(nullptr),elements(0)	{}
				/**/				ArrayRef(T*data, count_t elements):data(data),elements(elements)	{}
				/**/				ArrayRef(T&element):data(&element),elements(1)	{}
				/**/				ArrayRef(const ArrayRef<typename std::remove_const<T>::type>&nonConst):data(nonConst.data),elements(nonConst.elements)	{}
				virtual				~ArrayRef()	{}

				ArrayRef<const T>	Const() const
				{
					return ArrayRef<const T>(*this);
				}


				operator ArrayRef<const T>() const {return ArrayRef<const T>(data,elements);}

				template <typename I>
					const T*		operator+(I rel) const	//! Retrieves a pointer to the nth element @param rel Relative index. 0 points to the first element in the array. 	@return Pointer to the requested element for sub array access
									{
										return data+rel;
									}
				
				template <typename I>
					const T*		operator-(I rel) const	//! Retrieves a pointer to the nth element @param rel Relative index. 0 points to the first element in the array. 	@return Pointer to the requested element for sub array access
									{
										return data-rel;
									}
				
				inline const T*		pointer() const			//! Explicit type conversion to a native array of the contained type \return Pointer to the first contained element or NULL if the local array is empty.
									{
										return data;
									}
				/**
				Explicit type conversion to a native array of the contained type \return Pointer to the first contained element or NULL if the local array is empty.
				*/
				inline const T*		GetPointer() const
									{
										return data;
									}

				/**
				Sub-element access 
				@param index Index of the requested element (0 = first element)
				@return Reference to the requested element
				*/
				const T&			operator[](index_t index) const
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				const T&			at(index_t index) const		//! @copydoc operator[]()
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				/**
				Retrieves the nth element from the end of the array. GetFromEnd(0) is identical to Last()
				*/
				const T&			GetFromEnd(index_t index) const
									{
										index = elements - index - 1;
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}

				template <typename T1>
					inline int		CompareTo(const ArrayRef<T1>&other, int comparer(const T&, const T1&) ) const	//! Compares the local array with the remote array. The objects of the local array must implement a compareTo method that accepts objects of the remote array
									{
										count_t len = elements < other.GetLength()?elements:other.GetLength();
										for (count_t i = 0; i < len; i++)
										{
											char val = comparer(data[i],other[i]);
											if (val != 0)
												return val;
										}

										if (elements > other.GetLength())
											return 1;
										if (elements < other.GetLength())
											return -1;
										return 0;
									}

				template <typename T1>
					inline int		CompareTo(const ArrayRef<T1>&other) const	//! Compares the local array with the remote array. The objects of the local array must implement a compareTo method that accepts objects of the remote array
									{
										count_t len = elements < other.GetLength()?elements:other.GetLength();
										for (count_t i = 0; i < len; i++)
										{
											char val = data[i].CompareTo(other[i]);
											if (val != 0)
												return val;
										}

										if (elements > other.GetLength())
											return 1;
										if (elements < other.GetLength())
											return -1;
										return 0;
									}

				template <typename T1>
					inline bool		operator==(const ArrayRef<T1>&other) const //! Equality query \return true if all elements of the local array are identical to their respective counter parts in \b other. Equality is queried via the = operator.
									{
										if (elements != other.GetLength())
											return false;
										for (count_t i = 0; i < elements; i++)
											if (data[i] != other[i])
												return false;
										return true;
									}
				
				template <typename T1>
					inline bool		operator!=(const ArrayRef<T1>&other) const //! Equality query
									{
										if (elements != other.GetLength())
											return true;
										for (count_t i = 0; i < elements; i++)
											if (data[i] != other[i])
												return true;
										return false;
									}

				template <typename T1>
					inline bool		operator>(const ArrayRef<T1>&other) const //! Dictionary comparison
									{
										count_t len = elements < other.GetLength()?elements:other.GetLength();
										for (count_t i = 0; i < len; i++)
											if (data[i] > other[i])
												return true;
											else
												if (data[i] < other[i])
													return false;
										return elements > other.GetLength();
									}

				template <typename T1>
					inline bool		operator<(const ArrayRef<T1>&other) const //! Dictionary comparison
									{
										count_t len = elements < other.GetLength()?elements:other.GetLength();
										for (count_t i = 0; i < len; i++)
											if (data[i] > other[i])
												return false;
											else
												if (data[i] < other[i])
													return true;
										return elements < other.GetLength();
									}

				template <typename T1>
					inline bool		operator>=(const ArrayRef<T1>&other) const //! Dictionary comparison
									{
										count_t len = elements < other.GetLength()?elements:other.GetLength();
										for (count_t i = 0; i < len; i++)
											if (data[i] > other[i])
												return true;
											else
												if (data[i] < other[i])
													return false;
										return true;
									}

				template <typename T1>
					inline bool		operator<=(const ArrayRef<T1>&other) const //! Dictionary comparison
									{
										count_t len = elements < other.GetLength()?elements:other.GetLength();
										for (count_t i = 0; i < len; i++)
											if (data[i] > other[i])
												return false;
											else
												if (data[i] < other[i])
													return true;
										return true;
									}

				inline	bool		IsEmpty()	const	//! Checks if the local array is empty
									{
										return elements == 0;
									}
				inline	bool		IsNotEmpty()	const	//! Checks if the local array is not empty (contains at least one element)
									{
										return elements != 0;
									}

				inline bool			IsConcludedBy(const T*element)	const	//! Queries if the specified entry pointer points exactly one past the last element.
									{
										return element == data+elements;
									}

				inline bool			Owns(const T*element)	const	//! Queries if the specified entry pointer was taken from the local array. Actual pointer address is checked, not what it points to.
									{
										return element >= data && element < data+elements;
									}
				inline index_t		GetIndexOfPointer(const T*element)	const	//! Queries the index of the specifed element with 0 being the first element. To determine whether ot not the specified element is member of this array use the Owns() method.
									{
										return element-data;
									}				
				/**
				Finds the first matching entry and returns its index
				@param entry Element to look for (compared via ==)
				@return Index of the found entry of InvalidIndex if no match was found
				*/
				inline index_t		GetIndexOf(const T&entry) const
									{
										return GetNextIndexOf(entry,0);
									}
				inline index_t		GetNextIndexOf(const T&entry, index_t offset) const
									{
										for (count_t i = offset; i < elements; i++)
											if (data[i] == entry)
												return i;
										return InvalidIndex;
									}

				inline bool			Contains(const T&entry) const	{	return GetIndexOf(entry) != InvalidIndex;	}

				template <class T1>
					inline index_t	GetNextIndexOfSegment(const T1*field, count_t length, index_t offset) const
									{
										if (length > elements)
											return InvalidIndex;
										for (count_t i = offset; i+length <= elements; i++)
										{
											bool match(true);
											for (count_t j = 0; j < length; j++)
												if (data[i+j] != field[j])
												{
													match = false;
													break;
												}
												if (match)
													return i;
										}
										return InvalidIndex;
									}
				template <class T1>
					inline index_t	GetIndexOfSegment(const T1*field, count_t length) const
									{
										return GetNextIndexOfSegment(field,length,0);
									}

				inline index_t		GetLastIndexOf(const T&entry) const
									{
										for (count_t i = elements-1; i < elements; i--)
											if (data[i] == entry)
												return i;
										return InvalidIndex;
									}

				template <class T1>
					inline index_t	GetLastIndexOfSegment(const T1*field, count_t length) const
									{
										if (length > elements)
											return InvalidIndex;
										for (count_t i = elements-length; i<elements; i--)
										{
											bool match(true);
											for (count_t j = 0; j < length; j++)
												if (data[i+j] != field[j])
												{
													match = false;
													break;
												}
												if (match)
													return i;
										}
										return InvalidIndex;
									}

				inline index_t		GetIndexOf(const ArrayRef<T>&other) const
									{
										return GetIndexOfSegment(other.data,other.elements);
									}

				inline const_iterator begin() const	{return data;}
				inline const_iterator end() const	{return data+elements;}

				/**
				Retrieves a const reference to the first element in the field.
				The method return an undefined result, possibly causing access violations if the local array is empty
				*/
				inline const T&		First()	const
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (!elements)
												FATAL__("Array is empty - cannot retrieve First()");
										#endif

										return data[0];
									}

				/**
				Retrieves a const reference to the first element in the field.
				The method return an undefined result, possibly causing access violations if the local array is empty
				*/
				inline const T&		Last()	const
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (!elements)
												FATAL__("Array is empty - cannot retrieve Last()");
										#endif
										return data[elements-1];
									}

				inline count_t		Length()	const		//! Queries the current array size in elements \return Number of elements 
									{
										return elements;
									}
				inline count_t		GetLength()	const		/**@copydoc GetLength()*/ {return elements;}
				inline count_t		Count()	const		//! Queries the current array size in elements \return Number of elements 
									{
										return elements;
									}
				inline count_t		size()	const		//! Queries the current array size in elements \return Number of elements 
									{
										return elements;
									}
				inline size_t		GetContentSize()	const		//! Returns the summarized size of all contained elements in bytes \return Size of all elements
									{
										return elements*sizeof(T);
									}

				inline bool			IsTerminatedBy(const T&element)	const
									{
										return elements && data[elements-1] == element;
									}


				inline Self			SubRef(index_t start, count_t count) const
				{
					if (IsEmpty())
						return Self();
					start = std::min(start,elements-1);
					count = std::min(count,elements-start);
					return Self(data+start,count);
				}

				inline Self			SubRef(index_t start) const
				{
					if (IsEmpty())
						return Self();
					start = std::min(start,elements-1);
					count_t count = elements-start;
					return Self(data+start,count);
				}


				friend hash_t		Hash(const Self&data)
				{
					return HashField(data.data,data.elements);
				}

				friend void			Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					s.WriteSize(v.elements);
					Serialization::SerializeArray(s,v.data,v.elements);
				}

				/**
				Checks whether the given pointer lies within the locally referenced array segment
				*/
				bool				Addresses(const T*ptr) const {return ptr >= this->begin() && ptr < this->end();}

			};


	template <typename T>
			class MutableArrayRef : public ArrayRef<T>
			{
			public:
				typedef ArrayRef<T>	Super;
				typedef MutableArrayRef<T>	Self;

				typedef T*			iterator;
				typedef const T*	const_iterator;

				MutableArrayRef()	{}
				MutableArrayRef(T*data, count_t elements):Super(data,elements)	{}
				MutableArrayRef(T&element):Super(element)	{}

				using Super::IsEmpty;

				operator MutableArrayRef<const T>() const {return MutableArrayRef<const T>(Super::data,elements);}

				template <typename I>
					inline	T*		operator+(I rel)	//! Retrieves a pointer to the nth element @param rel Relative index. 0 points to the first element in the array. 	@return Pointer to the requested element for sub array access
									{
										return data+rel;
									}
				
				template <typename I>
					inline const T*	operator+(I rel) const	//! @copydoc operator+()
									{
										return data+rel;
									}

				template <typename I>
					inline	T*		operator-(I rel)	//! Retrieves a pointer to the nth element @param rel Relative index. 0 points to the first element in the array. 	@return Pointer to the requested element for sub array access
									{
										return data-rel;
									}
				
				template <typename I>
					inline const T*	operator-(I rel) const	//! @copydoc operator-()
									{
										return data-rel;
									}
				inline	T*			pointer()			//! Explicit type conversion to a native array of the contained type \return Pointer to the first contained element or NULL if the local array is empty.
									{
										return data;
									}
				inline const T*		pointer() const		//! @copydoc pointer()
									{
										return data;
									}
				inline	T*			GetPointer()			//! Explicit type conversion to a native array of the contained type \return Pointer to the first contained element or NULL if the local array is empty.
									{
										return data;
									}
				inline const T*		GetPointer() const		//! @copydoc pointer()
									{
										return data;
									}

				inline	T&			operator[](index_t index)		//! Sub-element access \param index Index of the requested element (0 = first element) \return Reference to the requested element
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				inline	const T&	operator[](index_t index) const	//! @copydoc operator[]()
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				inline	T&			at(index_t index)		//! @copydoc operator[]()
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				inline	const T&	at(index_t index) const	//! @copydoc at()
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				inline	T&			GetFromEnd(index_t index)					//! Retrieves the nth element from the end of the array. GetFromEnd(0) is identical to Last()
									{
										index = elements - index - 1;
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}
				inline	const T&	GetFromEnd(index_t index)			const	//! @copydoc GetFromEnd()
									{
										index = elements - index - 1;
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (index >= elements)
												FATAL__("Index out of bounds");
										#endif
										return data[index];
									}


				template <typename T1>
					inline	void	Fill(const T1&element, count_t offset=0, count_t max=InvalidIndex)	//! Sets up to \b max elements starting from @b offset of the local array to \b element \param element Element to repeat @param offset First index \param max If specified: Maximum number of elements to set to \b element
									{
										if (!elements)
											return;
										if (offset >= elements)
											offset = elements-1;
										if (max > elements)
											max = elements;
										for (count_t i = offset; i < max; i++)
											data[i] = (T)element;
									}

				inline iterator		begin()			{return data;}
				inline const_iterator begin() const	{return data;}
				inline iterator		end()			{return data+elements;}
				inline const_iterator end() const	{return data+elements;}

				inline T&			First()	//!< Retrieves a reference to the first element in the field. The method will return an undefined result if the local array is empty
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (!elements)
												FATAL__("Array is empty - cannot retrieve First()");
										#endif
										return data[0];
									}

				inline const T&		First()	const
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (!elements)
												FATAL__("Array is empty - cannot retrieve First()");
										#endif

										return data[0];
									}

				inline T&			Last()
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (!elements)
												FATAL__("Array is empty - cannot retrieve Last()");
										#endif

										return data[elements-1];
									}

				inline const T&		Last()	const
									{
										#ifdef __ARRAY_DBG_RANGE_CHECK__
											if (!elements)
												FATAL__("Array is empty - cannot retrieve Last()");
										#endif
										return data[elements-1];
									}


				inline Self			SubRef(index_t start, count_t count) const
				{
					if (IsEmpty())
						return Self();
					start = std::min(start,elements);
					count = std::min(count,elements-start);
					return Self(data+start,count);
				}

				inline Self			SubRef(index_t start) const
				{
					if (IsEmpty())
						return Self();
					start = std::min(start,elements);
					count_t count = elements-start;
					return Self(data+start,count);
				}

			protected:
				using Super::data;
				using Super::elements;

			};


		template <typename T, size_t Length, class MyStrategy>
			void FixedArray<T,Length,MyStrategy>::operator=(const ArrayRef<const T>&other) 
			{
				ASSERT__(Length == other.GetLength());
				for (index_t i = 0; i < Length; i++)
					Super::operator[](i) = other[i];
			}

		template <typename T, size_t Length, class MyStrategy>
			MutableArrayRef<T>				FixedArray<T,Length,MyStrategy>::ToRef()
			{
				return MutableArrayRef<T>(Super::data(),Length);
			}



		/**
		@brief Array data containment

		ArrayData provides all non-strategy-dependent functionality that array usage requires. Methods or functions that do not require strategy-dependent functionality may accept ArrayData instead.
		*/
		template <class C>
			class ArrayData: public MutableArrayRef<C>, public Arrays
			{
				typedef MutableArrayRef<C>	Super;
				typedef ArrayData<C>	Self;
			protected:
				using Super::data;
				using Super::elements;

				/**/				ArrayData(count_t length=0):Super(alloc<C>(length),length)	{}
				/**/				ArrayData(const C&e0, const C&e1):Super(alloc<C>(2),2)
									{
										data[0] = e0;
										data[1] = e1;
									}
				/**/				ArrayData(const C&e0, const C&e1, const C&e2):Super(alloc<C>(3),3)
									{
										data[0] = e0;
										data[1] = e1;
										data[2] = e2;
									}
				/**/				ArrayData(const C&e0, const C&e1, const C&e2, const C&e3):Super(alloc<C>(4),4)
									{
										data[0] = e0;
										data[1] = e1;
										data[2] = e2;
										data[3] = e3;
									}

			public:
				static const count_t	Undefined=Arrays::Undefined;
			private:

				/**/				ArrayData(const ArrayData<C>&){}
				/**/				ArrayData(const MutableArrayRef<C>&){}
				void				operator=(const ArrayData<C>&){}
				void				operator=(const MutableArrayRef<C>&){}
			public:
				#if __ARRAY_RVALUE_REFERENCES__
					/**/			ArrayData(ArrayData<C>&&rvalue):Super(rvalue.data,rvalue.elements)
									{
										rvalue.data = NULL;
										rvalue.elements = 0;
									}
					ArrayData<C>&	operator=(ArrayData<C>&&rvalue)
									{
										adoptData(rvalue);
										return *this;
									}
				#endif
				virtual				~ArrayData() override
									{
										if (!application_shutting_down)
											dealloc(data);
									}



				
				inline	void		free()		//! Frees the contained data and resets the local array length to 0
									{
										dealloc(data);
										elements = 0;
										data = NULL;
									}
				inline	void		clear()		/** @copydoc free() */{free();}
				inline	void		Clear()		/** @copydoc free() */{free();}
				inline	void		Free()		/** @copydoc free() */{free();}
			





				inline	void		Set(count_t num_values, ...)	//! Resizes the local array and fills it with the specified elements. The elements must be specified in the type of the local array's data
									{
										SetSize(num_values);
										va_list vl;
										va_start( vl, num_values );
										for (index_t i = 0; i < num_values; i++)
											data[i] = va_arg( vl, C );
									}

				template <typename T>
					inline	void	set2(const T&x, const T&y)	//! Resizes the local array and fills it with the specified elements. The elements must be specified in the type of the local array's data
									{
										SetSize(2);
										data[0] = C(x);
										data[1] = C(y);
									}

				template <typename T>
					inline	void	set3(const T&x, const T&y, const T&z)	//! Resizes the local array and fills it with the specified elements. The elements must be specified in the type of the local array's data
									{
										SetSize(3);
										data[0] = C(x);
										data[1] = C(y);
										data[2] = C(z);
									}

				template <typename T>
					inline	void	set4(const T&x, const T&y, const T&z, const T&w)	//! Resizes the local array and fills it with the specified elements. The elements must be specified in the type of the local array's data
									{
										SetSize(4);
										data[0] = C(x);
										data[1] = C(y);
										data[2] = C(z);
										data[3] = C(w);
									}


				inline	void		adoptData(ArrayData<C>&other)	//! Clears any existing local data, adopts pointer and size and sets both NULL of the specified origin array.
									{
										if (this == &other)
											return;
										free();
										data = other.data;
										elements = other.elements;
										other.data = NULL;
										other.elements = 0;
									}

				inline	void		swap(Self&other)
									{
										swp(data,other.data);
										swp(elements,other.elements);
									}
				friend void			swap(Self&a, Self&b)
				{
					a.swap(b);
				}


				inline	void		SetSize(count_t new_size)	/** @brief Resizes the array. The new array's content is constructed but uninitialized. \param new_size New array size in elements (may be 0) */
									{
										reloc(data,elements,new_size);
									}

				//template <class T>



				friend void			Deserialize(IReadStream&s, Self&v)
				{
					v.SetSize(s.ReadSize());
					Serialization::DeserializeArray(s,v.data,v.elements);
				}
			};




			/*
				Verification array<T> (Array<T,Strategy>)

				Notes:
					for (a,b:T): a = b is defined as true if both a and b are undefined/uninitialized

				Abstract array: Field = Sequence 0...N of elements of type T
				Concrete array: state St = (count_t L, T*A)
				Invariants:
					* if L > 0: A is not NULL, and field of type T and length L (in elements)
					* if L = 0: A is NULL
					conversely:
						*if A is NULL: L is 0
						*if A is not NULL: L > 0

		
				for all states St whoes invariants are satisfied
					F: state St -> Sequence of elements of type T
						St.L > 0: F(St) = Sequence St.A[0]...St.A[St.L-1] of type T
						St.L = 0: F(St) = Empty sequence


				create: count_t -> array<T>
					abstract:
						create<array<T>>(count_t parameter) = 
							parameter > 0: Sequence A[0]...A[parameter-1] of type T; A[i] is of value create<T> (parameterless, undefined value for native types)
							parameter = 0: Empty sequence
					concrete:
						f_create<array<T>>(count_t parameter) -> state S:
							St.L <- parameter
							St.A <- alloc<T>(parameter)	(alloc<T> is verified to generate an array of length 'parameter' elements, and NULL if 'parameter' is 0. Elements are initialized without parameter)

							F(St) = 
								St.L > 0: Sequence St.A[0]...St.A[elements-1] of type T; St.A[i] is of value create<T> (parameterless, undefined value for native types)
								St.L = 0: Empty sequence
			
					-> correct

				append: state x const T* x count_t -> state
					abstract:
						append(Sequence<T> A = F(state St), Sequence<T> B = F(state(count_t L,const T*A))) =
							|A| = 0 and |B| = 0: Empty sequence
							|A| = 0 and |B| > 0: B
							|A| > 0 and |B| = 0: A
							otherwise: Sequence A[0]...A[|A|-1], B[0]...B[|B|-1] of type T
					concrete:
						f_append(state St, const T*A, count_t L) -> state St':
							St'.A <- alloc<T>(St.L+L);
							St'.L <- St.L+L;
							Strategy::moveElements(St.A,St'.A,St.L);	//move elements from old array over to new array (simply copy in most basic cases)
							dealloc<T>(St.A);							//deallocates a dynamic array. Doesn't do anything if St.A is NULL
							Strategy::copyElements(A,St'.A+St.L,L);		//explicitly copy elements from import array over to appropriate location in new field

						F(St') =
							St.L = 0 and L = 0:
								{
									implied by invariants:
										St.A = NULL
									St'.A <- alloc<T>(0+0); (=NULL)
									St'.L <- 0+0;							//invariant satisfaction (St'.L is 0 as required by St'.A = NULL)
									Strategy::moveElements(NULL,NULL,0);	//no effect
									dealloc<T>(NULL);						//no effect
									Strategy::copyElements(A,NULL+0,0);		//no effect
								}
								= F(state(0,NULL)) = Empty sequence

							St.L = 0 and L > 0:
								{
									implied by invariants:
										St.A = NULL
									St'.A <- alloc<T>(0+L); (=field of type T and length L)
									St'.L <- 0+L; (=L)						//invariant satisfaction (St'.A is field of type T and length St'.L)
									Strategy::moveElements(NULL,St'.A,0);	//no effect
									dealloc<T>(NULL);						//no effect
									Strategy::copyElements(A,St'.A+0,L);	//=Strategy::copyElements(A,St'.A,L) => copy L elements from A to St'.A. St'.L is L, so all elements of St' are defined by (L,A)
								}
								= Sequence A[0]...A[L-1] of type T (= Sequence<T> B where B = F(state(L,A)))

							St.L > 0 and L = 0:
								{
									implied by invariants:
										St.A != NULL
										St.A is field of type T and length St.L
									St'.A <- alloc<T>(St.L+0); (=field of type T and length St.L)
									St'.L <- St.L+0; (=St.L)					//invariant satisfaction (St'.A is field of type T and length St'.L)
									Strategy::moveElements(St.A,St'.A,St.L);	//Move L elements from St.A to St'.A. Both fields are of length St.L (=St'.L). St' is now completely defined by St
									dealloc<T>(St.A);							//discard old array
									Strategy::copyElements(A,St'.A+St.L,0);		//no effect
								}
								= Sequence St.A[0]...St.A[St.L-1] of type T (= Sequence<T> A where A = F(St))

							St.L > 0 and L > 0:
								{
									implied by invariants:
										St.A != NULL
										St.A is field of type T and length St.L
									St'.A <- alloc<T>(St.L+L);					//St'.A is now a field exactly long enough to contain all elements of both the previous state array St.A and append array A
									St'.L <- St.L+L;							//invariant satisfaction (St'.A is field of type T and length St'.L)
									Strategy::moveElements(St.A,St'.A,St.L);	//Move elements St.A[0]...St.A[St.L-1] of the previous state array St.A over to the new array St'.A. Side-note: At least one element is moved (St.L >= 1), and at least one element of St'.A is still uninitialized (L >= 1)
									dealloc<T>(St.A);							//deallocate old array
									Strategy::copyElements(A,St'.A+St.L,L);		//Copy A[0]...A[L-1] to the memory section immediately succeeding the section required for the elements of the old state St
								}
								= Sequence St.A[0]...St.A[St.L-1], A[0]...A[L-1] (= Sequence A[0]...A[|A|-1], B[0]...B[|B|-1] where A = F(St) and B = F(state(L,A)))
					-> correct


				The verification assumes that invoked functions are correct as well, which cannot be predicted in all cases
			*/

		/*!

			\brief	Generalized array container
	
			Array is a generalized array management structure, implicitly performing many operations that manual array handling
			requires. The contained array content is automatically copied, allocated or deallocated where needed and shielded
			from inappropriate (and usually undesired) operations.
		*/
		template <class C, class MyStrategy=typename StrategySelector<C>::Default>
			class Array: public ArrayData<C>
			{
				typedef ArrayData<C>	Super;
				typedef Array<C,MyStrategy>	Self;
				protected:
					#ifdef __GNUC__
						using ArrayData<C>::data;
						using ArrayData<C>::elements;
					#endif
				public:
					typedef typename Super::iterator iterator;
					typedef typename Super::const_iterator const_iterator;
					typedef count_t			Size;
					static const count_t	Undefined=Arrays::Undefined;

					typedef	MyStrategy	AppliedStrategy;

					typedef ArrayData<C>	Data;

					explicit	Array(count_t length=0):Data(length)	/** Creates a new array \param length Length of the new array object */ {}
					explicit	Array(const C&e0, const C&e1):Data(e0,e1)	/** Creates a new 2-element array @param e0 First element to Fill into the array @param e1 Second element to Fill into the array*/ {}
					explicit	Array(const C&e0, const C&e1, const C&e2):Data(e0,e1,e2)	/** Creates a new 3-element array @param e0 First element to Fill into the array @param e1 Second element to Fill into the array @param e2 Third element to Fill into the array*/ {}
					explicit	Array(const C&e0, const C&e1, const C&e2, const C&e3):Data(e0,e1,e2,e3)	/** Creates a new 4-element array @param e0 First element to Fill into the array @param e1 Second element to Fill into the array @param e2 Third element to Fill into the array @param e3 Fourth element to Fill into the array*/ {}
				
					inline		Array(const C*string)
					{
						if (string)
						{
							const C*terminator(string);
							while (*terminator++);
							elements = terminator-string;
						}
						else
							elements = 0;
						alloc(data,elements);
						MyStrategy::copyElements(string,data,elements);
					}
			
					template <class T>
						inline	Array(const T*field, count_t length):Data(length)	//!< Array copy constructor \param field Array that should be copies \param length Number of elements, field contains
						{
							MyStrategy::copyElements(field,data,elements);
						}

						inline	Array(const std::initializer_list<C>&other):Data(other.size())
						{
							typename std::initializer_list<C>::const_iterator it = other.begin();
							index_t at = 0;
							for (;it != other.end(); ++it)
							{
								Super::data[at++] = *it;
							}
						}
				
						inline	Array(const Array<C,MyStrategy>&other):Data(other.Count())
						{
							MyStrategy::copyElements(other.data,data,elements);
						}

					template <class T>
						inline	Array(const ArrayRef<T>&other):Data(other.Count())
						{
							MyStrategy::copyElements(other.GetPointer(),data,elements);
						}
				
						inline	Array(const ArrayRef<C>&other):Data(other.Count())
						{
							MyStrategy::copyElements(other.GetPointer(),data,elements);
						}

				#if __ARRAY_RVALUE_REFERENCES__
						template <class OtherStrategy>
							Array(Array<C,OtherStrategy>&&rvalue):Data(0)
							{
								this->adoptData(rvalue);
							}


						template <class OtherStrategy>
							Array<C,MyStrategy>&	operator=(Array<C,OtherStrategy>&&rvalue)
							{
								this->adoptData(rvalue);
								return *this;
							}
				#endif



							inline Self& operator=(const Self&other) //! Assignment operator. Copies each element via the = operator
							{
								return Copy(other);
							}
				
				
						template <class T>
							inline Self& operator=(const ArrayRef<T>&other) //! Assignment operator. Copies each element via the = operator
							{
								return Copy(other);
							}
						template <class T>
							inline Self& Copy(const ArrayRef<T>&other) //! Assignment operator. Copies each element via the = operator
							{
								if (((const ArrayRef<T>*)this) == &other)	//this should not happen but anyway
									return *this;
								reloc(data,elements,other.Count());
								MyStrategy::copyElements(other.GetPointer(),data,elements);
								return *this;
							}
							
							inline	C		shiftDown()	//!< Moves all array elements down, starting with the first, finishing with one before the last element. Elements are copied via the = operator. The original first element of the array will be removed from the array, the last one duplicated. @return Original (dropped) first element
							{
								if (!elements)
									return C();
								C first = data[0];
								for (index_t i = 0; i < elements-1; i++)
									MyStrategy::move(data[i+1],data[i]);
								return first;
							}
				
							inline	C		shiftUp()	//!< Moves all array elements up, starting with the last, finishing with one past the first element. Elements are copied via the = operator. The original last element of the array will be removed from the array, the first one duplicated. @return Original (dropped) last element
							{
								if (!elements)
									return C();
								C last = data[elements-1];
								for (index_t i = elements-1; i >0; i--)
									MyStrategy::move(data[i-1],data[i]);
								return last;
							}
				
			
						template <class T>
							inline void	copyFrom(const T*origin, count_t max=Undefined) //! Copies all elements from \b origin via the = operator \param origin Array to copy from (may be of a different entry type) \param max Maximum number of elements to read
							{
								if (max > elements)
									max = elements;

								MyStrategy::copyElements(origin,data,max);
							}
						template <class T>
							inline void	copyFrom(index_t target_offset, const T*origin, count_t max=Undefined) //! Copies all elements from \b origin via the = operator \param origin Array to copy from (may be of a different entry type) \param max Maximum number of elements to read
							{
								if (target_offset >= elements)
									return;
								if (max > elements - target_offset)
									max = elements - target_offset;

								MyStrategy::copyElements(origin,data+target_offset,max);
							}

							inline	void	importFrom(C*origin, count_t max=Undefined)	//! Moves up to @b max elements from @b origin to the local array. The type of movement depends on the used strategy
							{
								if (max > elements)
									max = elements;
								MyStrategy::moveElements(origin,data,max);
							}
				

			

				
							inline	void	resizeAndImport(C*origin, count_t length)	//!< Resizes the local field and imports the specified number of elements (see Import())
							{
								Super::SetSize(length);
								MyStrategy::moveElements(origin,data,length);
							}
				
						template <class T>
							inline void	ResizeAndCopy(const T*origin, count_t length) //! Copies all elements from \b origin via the = operator overwriting any existing local elements \param origin Array to copy from (may be of a different entry type) \param count Number of elements to copy
							{
								Super::SetSize(length);
								MyStrategy::copyElements(origin,data,length);
							}

				
						template <class T>
							inline void	ResizeAndCopy(const ArrayRef<T>&origin, count_t max=Undefined) //! Copies all elements from \b origin via the = operator overwriting any existing local elements \param origin Array to copy from (may be of a different entry type) \param max Maximum number of elements to read
							{
								if (max > origin.Count())
									max = origin.Count();
								Super::SetSize(max);
								MyStrategy::copyElements(origin.pointer(),data,max);
							}
			
							inline C&	Append()	//!< Appends a singular element to the end of the array and returns a reference to it. Append() has to allocate a new array, copy the contents of the old array and delete the old array. Copying is performed via the = operator @return Reference to the appended element
							{
								ResizePreserveContent(elements+1);
								return data[elements-1];
							}


							inline C&	Insert(index_t beforeIndex)
							{
								if (beforeIndex >= elements)
									return Append();

								C	*new_field = alloc<C>(elements+1),
									*at = new_field;

								MyStrategy::moveElements(data,		new_field,			beforeIndex);
								MyStrategy::moveRange(data+beforeIndex,	data+elements,	new_field+beforeIndex+1);

								/*
								for (count_t i = 0; i < offset; i++)
									Strategy::move(data[i],*at++);
								for (count_t i = 0; i < max; i++)
									Strategy::move(origin.data[i],*at++);
								for (count_t i = offset; i < elements; i++)
									Strategy::move(data[i],*at++);*/
								elements ++;
								//ASSERT_EQUAL__(at,new_field+elements);
								dealloc(data);
								data = new_field;
								return data[beforeIndex];
							}

							inline void	Insert(index_t offset, const C&element)
							{
								Insert(offset) = element;
							}
			
							inline void appendImport(Array<C,MyStrategy>&origin, count_t max=Undefined)	//! Appends some or all elements of \b origin to the end of the local array. Movement behavior is defined by the used strategy \param origin Array to copy from \param max Maximum number of elements to append
							{
								if (max > origin.elements)
									max = origin.elements;
								count_t old_elements = elements;
								ResizePreserveContent(elements+max);
								MyStrategy::moveElements(origin.data, data+old_elements, max);
							}

							inline void appendImport(C*import_data, count_t import_length)
							{
								count_t old_elements = elements;
								ResizePreserveContent(elements+import_length);
								MyStrategy::moveElements(import_data, data+old_elements, import_length);
							}
			
						template <class T, class OtherStrategy>
							inline void appendCopy(const Array<T,OtherStrategy>&origin, count_t max=Undefined)	//! Appends some or all elements of \b origin to the end of the local array. Elements are duplicated explicitly using the = operator (strategy is ignored) \param origin Array to copy from \param max Maximum number of elements to append
							{
								if (max > origin.Count())
									max = origin.Count();
								count_t old_elements = elements;
								ResizePreserveContent(elements+max);
								HybridStrategy<MyStrategy,OtherStrategy>::copyElements(origin.pointer(),data+old_elements,max);
							}

							inline void appendCopy(const ArrayData<C>&origin, count_t max=Undefined)	//! Appends some or all elements of \b origin to the end of the local array. Elements are duplicated explicitly using the = operator (strategy is ignored) \param origin Array to copy from \param max Maximum number of elements to append
							{
								if (max > origin.Count())
									max = origin.Count();
								count_t old_elements = elements;
								ResizePreserveContent(elements+max);
								MyStrategy::copyElements(origin.pointer(),data+old_elements,max);
							}

						template <class T>
							inline void appendCopy(const T*copy_data, count_t copy_length)
							{
								count_t old_elements = elements;
								ResizePreserveContent(elements+copy_length);
								MyStrategy::copyElements(copy_data,data+old_elements,copy_length);
							}
			

							inline void insertImport(index_t offset, Array<C,MyStrategy>&origin, count_t max=Undefined)	//! Inserts some or all elements of \b origin before the specified offset (0=first element). Elements are moved depending on the applied strategy @param offset Offset to insert in front of (0= first element) \param origin Array to copy from \param max Maximum number of elements to append
							{
								if (offset >= elements)
									appendImport(origin,max);
								else
								{
									if (max> origin.elements)
										max = origin.elements;
									if (!max)
										return;
									C	*new_field = alloc<C>(elements+max),
										*at = new_field;

									MyStrategy::moveElements(data,		new_field,			offset);
									MyStrategy::moveElements(origin.data,	new_field+offset,	max);

									MyStrategy::moveRange(data+offset,	data+elements,	new_field+offset+max);

									/*
									for (count_t i = 0; i < offset; i++)
										Strategy::move(data[i],*at++);
									for (count_t i = 0; i < max; i++)
										Strategy::move(origin.data[i],*at++);
									for (count_t i = offset; i < elements; i++)
										Strategy::move(data[i],*at++);*/
									elements += max;
									//ASSERT_EQUAL__(at,new_field+elements);
									dealloc(data);
									data = new_field;
								}
							}

						template <class T, class OtherStrategy>
							inline void insertCopy(index_t offset, const Array<T,OtherStrategy>&origin, count_t max=Undefined)	//! Inserts some or all elements of \b origin before the specified offset (0=first element). Elements are duplicated using the = operator @param offset Offset to insert in front of (0= first element) \param origin Array to copy from \param max Maximum number of elements to append
							{
								if (offset >= elements)
									appendCopy(origin,max);
								else
								{
									if (max > origin.Count())
										max = origin.Count();
									if (!max)
										return;
									C	*new_field = alloc<C>(elements+max);

									HybridStrategy<MyStrategy,OtherStrategy>::moveElements(data,		new_field,		offset);
									HybridStrategy<MyStrategy,OtherStrategy>::copyElements(origin.pointer(), new_field+offset,	max);
									HybridStrategy<MyStrategy,OtherStrategy>::moveRange(data+offset,	data+elements,	new_field+offset+max);

									elements += max;
									//ASSERT_EQUAL__(at,new_field+elements);
									dealloc(data);
									data = new_field;
								}
							}
			

							inline void	Erase(index_t index, count_t count=1)	//! Erases a number of elements from the array. The remaining elements (if any) are moved depending on the used strategy. \param index Index of the first element to erase with 0 being the first entry \param count Number of elements to erase
							{
								if (index >= elements)
									return;
								if (count > elements-index)
									count = elements-index;
								if (!count)
									return;

								C*end = data+elements;

								elements -= count;
								C*new_field = alloc<C>(elements);
								MyStrategy::moveElements(data, new_field, index);
								MyStrategy::moveRange(data+index+count, end, new_field+index);

								dealloc(data);
								data = new_field;
							}
							inline iterator	erase(iterator position)
							{
								index_t index = position - Super::begin();
								Erase(index);
								return Super::begin() + index;
							}
							inline iterator	erase(iterator first, iterator end)
							{
								index_t index = first - Super::begin();
								Erase(index, end - first);
								return Super::begin() + index;
							}
						template <class T>
							inline bool	 FindAndErase (const T &element)
							{
								index_t index = Super::findFirst (element);
								if (index)
								{
									Erase(index-1);
									return true;
								}
								return false;		
							}
				
							inline void revert()	//!< Reverses the order of elements in the local field
							{
								MyStrategy::revert(data,data+elements);
							}
			
		
				
							inline	void	ResizePreserveContent(count_t new_size) //! Resizes the array. The new array's content is filled with the elements of the old array as far as possible. Element movement is defined by the used strategy \param new_size New array size in elements (may be 0)
							{
								if (new_size == elements || new_size == Undefined)
									return;
								C*	new_field = alloc<C>(new_size);
								count_t copy = new_size<elements?new_size:elements;
								MyStrategy::moveElements(data,new_field,copy);
								dealloc(data);
								data = new_field;
								elements = new_size;
							}
				
							/**
							Resizes the local array to match the new size.
							The operation may delete elements from the end, or add new ones.
							@param pattern Element to use to fill any added elements
							*/
							inline	void	ResizePreserveContent(count_t newSize, const C&pattern)
							{
								if (newSize == elements || newSize == Undefined)
									return;
								C*	new_field = alloc<C>(newSize);
								count_t copy = newSize<elements?newSize:elements;
								MyStrategy::moveElements(data,new_field,copy);
								for (index_t i = elements; i < newSize; i++)
									new_field[i] = pattern;
								dealloc(data);
								data = new_field;
								elements = newSize;
							}
			protected:
				using Super::data;
				using Super::elements;
			};


		template <>
			inline Array<char>::Array(const char*field):Data((count_t)strlen(field)+1)
			{
				alloc(data,elements);
				strncpy(data,field,elements);
			}

		template <>
			inline Array<wchar_t>::Array(const wchar_t*field):Data((count_t)wcslen(field)+1)
			{
				alloc(data,elements);
				wcsncpy(data,field,elements);
			}
			
			


		template <typename T>
			inline index_t	operator-(const T*ptr, const ArrayData<T>&field)
			{
				return static_cast<index_t>(ptr - field.pointer());
			}

	}




	/*!
		\brief Array dealloc function to prevent accidental access violations.
	*/
	template <class C>
		inline void dealloc(Ctr::ArrayData<C>&array)
		{
			array.free();
		}

	/*!
		\brief Array re_alloc function to prevent accidental access violations.
	*/
	template <class C>
		inline void re_alloc(Ctr::ArrayData<C>&array, count_t elements)
		{
			array.SetSize(elements);
		}

	/*!
		\brief Array reloc function to prevent accidental access violations.
	*/
	template <class FieldType, class IndexType0, class IndexType1>
		inline void   reloc(Ctr::ArrayData<FieldType>&array, IndexType0&length, IndexType1 new_length)
		{
			array.SetSize(new_length);
			length = array.GetLength();
		}

	/*!
		\brief Array alloc function to prevent accidental access violations.
	*/
	template <class C>
		inline void alloc(Ctr::ArrayData<C>&target, count_t elements)
		{
			target.SetSize(elements);
		}

			
	template <typename Serializable>
		void SerializeToArray(const Serializable&s,Array<BYTE>&target)
		{
			using Serialization::Serialize;
			SerialSizeScanner scanner;
			Serialize(scanner,s);
			target.SetSize(scanner.GetTotalSize());
			if (scanner.GetTotalSize())
			{
				MemWriteStream	writer(target.GetPointer(),scanner.GetTotalSize());
				Serialize(writer,s);
			}
		}





}

DECLARE_T__(Container::ArrayData,SwapStrategy);
DECLARE_T2__(Container::Array,class,class,SwapStrategy);


#endif
