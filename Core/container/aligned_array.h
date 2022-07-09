#ifndef aligned_arrayH
#define aligned_arrayH

#include <wchar.h>
#include "../interface/serializable.h"
#include "array.h"


#if SYSTEM==WINDOWS
	#define wcswcs	wcsstr
#endif

namespace DeltaWorks
{

	/*!
	\brief	Generalized aligned array container
	
	AlignedArray is a generalized array management structure, implicitly performing many operations that manual array handling
	requires. The contained array content is automatically copied, allocated or deallocated where needed and shielded
	from inappropriate (and usually undesired) operations.
	*/
	template <class C, size_t A> class AlignedArray: public SerializableObject, public Arrays
	{
		protected:
			C*				data;
			BYTE*			root;
			count_t			elements;
				
				
				
			template <class C0, size_t A0>
				static inline void		allocAligned(BYTE*&root, C0*&data, count_t elements)
				{
					if (!elements)
					{
						root = NULL;
						data = NULL;
						return;
					}
					root = alloc<BYTE>(sizeof(C0)*elements+A0-1);
					BYTE*data_root = root;
					#ifndef __GNUC__
						while (((size_t)data_root)&(A0-1))
							data_root++;
					#else
						while (((long)data_root)&(A0-1))
							data_root++;
					#endif
					data = (C0*)data_root;
				}
			template <class C0, size_t A0>
				static inline void		re_allocAligned(BYTE*&root, C0*&array, count_t elements)
				{
					if (root)
						dealloc(root);
					allocAligned<C0,A0>(root,array,elements);
				}				
		
			template <class C0, size_t A0>
				static inline void 		relocAligned(BYTE*&root,C0*&data,count_t&length, count_t new_length)
				{
					if (length == new_length || new_length == Undefined)
						return;
					length = new_length;
					re_allocAligned<C0,A0>(root,data,length);
				}
				
	
				
				
		public:
			/**/	AlignedArray(count_t length=0):elements(length)	//!< Creates a new array \param length Length of the new array object
			{
				allocAligned<C,A>(root,data,elements);
			}

				
			inline AlignedArray(const C*string)
			{
				if (string)
				{
					const C*terminator(string);
					while (*terminator++);
					elements = terminator-string;
				}
				else
					elements = 0;
				allocAligned<C,A>(root,data,elements);
				for (register count_t i = 0; i < elements; i++)
					data[i] = string[i];
			}
			
			template <class T>
				inline AlignedArray(const T*field, count_t length):elements(length)	//!< Ctr::Array copy constructor \param field Ctr::Array that should be copies \param length Number of elements, field contains
				{
					allocAligned<C,A>(root,data,elements);
					for (register count_t i = 0; i < elements; i++)
						data[i] = field[i];
				}
				
			template <class T, size_t A1>
				inline AlignedArray(const AlignedArray<T,A1>&other):elements(other.Count())
				{
					allocAligned<C,A>(root,data,elements);
					for (register count_t i = 0; i < elements; i++)
						data[i] = other[i];
				}
				
				inline AlignedArray(const AlignedArray<C,A>&other):elements(other.Count())
				{
					allocAligned<C,A>(root,data,elements);
					for (register count_t i = 0; i < elements; i++)
						data[i] = other[i];
				}
				
			virtual	~AlignedArray()
			{
				dealloc(root);
			}
				
			inline AlignedArray<C,A>& operator=(const AlignedArray<C,A>&other) //! Assignment operator
			{
				if (&other == this)
					return *this;
				relocAligned<C,A>(root,data,elements,other.elements);
				for (register count_t i = 0; i < elements; i++)
					data[i] = other.data[i];
				return *this;
			}
				
			template <class T, size_t A1>
				inline AlignedArray<C,A>& operator=(const AlignedArray<T,A1>&other) //! Assignment operator. Copies each element via the = operator
				{
					if ((AlignedArray<C,A>*)&other == this)	//this should not happen but anyway
						return *this;
					relocAligned<C,A>(root,data,elements,other.elements);
					for (register count_t i = 0; i < elements; i++)
						data[i] = other.data[i];
					return *this;
				}
				
			template <class T, size_t A1>
				inline AlignedArray<C,A>&	operator+=(const AlignedArray<T,A1>&other) //! Append operator. Copies local and appended elements via the = operator
				{
					C	*new_data;
					BYTE*new_root;
					allocAligned<C,A>(new_root,new_data,elements+other.elements);
				
					for (register count_t i = 0; i < elements; i++)
						new_data[i] = data[i];
					for (register count_t i = 0; i < other.elements; i++)
						new_data[elements+i] = other.data[i];
					dealloc(root);
					root = new_root;
					data = new_data;
					elements+=other.elements;
					return *this;
				}
				
			template <class T, size_t A1>
				inline AlignedArray<C,A>	operator+(const AlignedArray<T,A1>&other) const //! Append operator. Copies local and appended elements via the = operator
				{
					AlignedArray<C,A> result(elements+other.elements);
					for (register count_t i = 0; i < elements; i++)
						result.data[i] = data[i];
					for (register count_t i = 0; i < other.elements; i++)
						result.data[elements+i] = other.data[i];
					return result;
				}
				
			inline C*			operator+(sindex_t rel)	//!< Retrieves a pointer to the nth element @param rel Relative index. 0 points to the first element in the array. 	@return Pointer to the requested element for sub array access
			{
				return data+rel;
			}
				
			inline const C*			operator+(sindex_t rel) const	//!< @overload
			{
				return data+rel;
			}
			
								
			template <class T, size_t A1>
				inline bool			operator==(const AlignedArray<T,A1>&other) const //! Equality query \return true if all elements of the local array are identical to their respective counter parts in \b other. Equality is queried via the = operator.
				{
					if (elements != other.elements)
						return false;
					for (count_t i = 0; i < elements; i++)
						if (data[i] != other.data[i])
							return false;
					return true;
				}
				
			template <class T, size_t A1>
				inline bool			operator!=(const AlignedArray<T,A1>&other) const //! Equality query
				{
					if (elements != other.elements)
						return true;
					for (count_t i = 0; i < elements; i++)
						if (data[i] != other.data[i])
							return true;
					return false;
				}

			template <class T, size_t A1>
				inline bool			operator>(const AlignedArray<T,A1>&other) const //! Dictionary comparison
				{
					count_t len = elements < other.elements?elements:other.elements;
					for (count_t i = 0; i < len; i++)
						if (data[i] > other.data[i])
							return true;
						else
							if (data[i] < other.data[i])
								return false;
					return false;
				}

			template <class T, size_t A1>
				inline bool			operator<(const AlignedArray<T,A1>&other) const //! Dictionary comparison
				{
					count_t len = elements < other.elements?elements:other.elements;
					for (count_t i = 0; i < len; i++)
						if (data[i] > other.data[i])
							return false;
						else
							if (data[i] < other.data[i])
								return true;
					return false;
				}

			template <class T, size_t A1>
				inline bool			operator>=(const AlignedArray<T,A1>&other) const //! Dictionary comparison
				{
					count_t len = elements < other.elements?elements:other.elements;
					for (count_t i = 0; i < len; i++)
						if (data[i] > other.data[i])
							return true;
						else
							if (data[i] < other.data[i])
								return false;
					return true;
				}

			template <class T, size_t A1>
				inline bool			operator<=(const AlignedArray<T,A1>&other) const //! Dictionary comparison
				{
					count_t len = elements < other.elements?elements:other.elements;
					for (count_t i = 0; i < len; i++)
						if (data[i] > other.data[i])
							return false;
						else
							if (data[i] < other.data[i])
								return true;
					return true;
				}
				
			inline	void	free()		//! Frees the contained data and resets the local array length to 0
			{
				dealloc(root);
				elements = 0;
				data = NULL;
				root = NULL;
			}
			inline	C*		pointer()			//! Explict type conversion to a native array of the contained type \return Pointer to the first contained element or NULL if the local array is empty.
			{
				return data;
			}
			inline const C* pointer() const		//! @overload
			{
				return data;
			}				
			inline	C&		operator[](index_t index)		//! Sub-element access \param index Index of the requested element (0 = first element) \return Reference to the requested element
			{
				return data[index];
			}
			inline	const C&		operator[](index_t index) const	//!< @overload
			{
				return data[index];
			}

			
			inline	C		shiftLeft()	//!< Moves all array elements down, starting with the first, finishing with one before the last element. Elements are copied via the = operator. The original first element of the array will be removed from the array, the last one duplicated. @return Original (dropped) first element
			{
				if (!elements)
					return C();
				C first = data[0];
				for (unsigned i = 0; i < elements-1; i++)
					data[i] = data[i+1];
				return first;
			}
				
			inline	C		shiftRight()	//!< Moves all array elements up, starting with the last, finishing with one past the first element. Elements are copied via the = operator. The original last element of the array will be removed from the array, the first one duplicated. @return Original (dropped) last element
			{
				if (!elements)
					return C();
				C last = data[elements-1];
				for (unsigned i = elements-1; i >0; i--)
					data[i] = data[i-1];
				return last;
			}
				
			template <typename T>
				inline	void	Fill(const T&element, count_t offset=0, count_t max=Undefined)	//! Sets up to \b max elements starting from @b offset of the local array to \b element \param element Element to repeat @param offset First index \param max If specified: Maximum number of elements to set to \b element
				{
					if (!elements)
						return;
					if (offset >= elements)
						offset = elements-1;
					if (max > elements)
						max = elements;
					for (register count_t i = offset; i < max; i++)
						data[i] = (C)element;
				}


			
			template <class T>
				inline void	copyFrom(const T*origin, count_t max=Undefined) //! Copies all elements from \b origin via the = operator \param origin Ctr::Array to copy from (may be of a different entry type) \param max Maximum number of elements to read
				{
					if (max > elements)
						max = elements;
					for (register count_t i = 0; i < max; i++)
						data[i] = (C)origin[i];
				}
				
			inline	void	adoptData(AlignedArray<C,A>&other)	//! Clears any existing local data, adopts pointer and size and sets both NULL of the specified origin array.
			{
				if (this == &other)
					return;
				dealloc(root);
				data = other.data;
				root = other.root;
				elements = other.elements;
				other.data = NULL;
				other.root = NULL;
				other.elements = 0;
			}
			
			inline	void	adoptData(const C*origin, count_t max=Undefined)	//!< Adopts up to @b max elements from origin
			{
				if (max > elements)
					max = elements;
				for (register count_t i = 0; i < max; i++)
					data[i].adoptData(origin[i]);
			}
				
			inline	void	resizeAdoptFrom(const C*origin, count_t length)	//!< Resizes the local field and adopts the specified number of elements
			{
				SetSize(length);
				for (register count_t i = 0; i < length; i++)
					data[i].adoptData(origin[i]);
			}
				
				
			template <class T, size_t A1>
				inline void	readFrom(const AlignedArray<T,A1>&origin, count_t max=Undefined) //! Copies all elements from \b origin via the = operator overwriting any existing local elements \param origin Ctr::Array to copy from (may be of a different entry type) \param max Maximum number of elements to read
				{
					if (max > origin.elements)
						max = origin.elements;
					SetSize(max);
					for (register count_t i = 0; i < max; i++)
						data[i] = origin.data[i];
				}
			
			inline C&	Append()	//!< Appends a singular element to the end of the array and returns a reference to it. Append() has to allocate a new array, copy the contents of the old array and delete the old array. Copying is performed via the = operator @return Reference to the appended element
			{
				resizeCopy(elements+1);
				return data[elements-1];
			}
			
			inline C&	appendAdopt()	//!< Appends a singular element to the end of the array and returns a reference to it. appendAdopt() has to allocate a new array, adopt the contents of the old array and delete the old array. Copying is performed via the adoptData() method. The contained class has to provide the adoptData method for this method to work. @return Reference to the appended element
			{
				resizeAdopt(elements+1);
				return data[elements-1];
			}
			
			inline C&	appendAdoptSingular()	//!< @overload
			{
				resizeAdopt(elements+1);
				return data[elements-1];
			}
			
			template <class T, size_t A1>
				inline void append(const AlignedArray<T,A1>&origin, count_t max=Undefined)	//! Appends some or all elements of \b origin to the end of the local array. Elements are duplicated using the = operator \param origin Ctr::Array to copy from \param max Maximum number of elements to append
				{
					if (max> origin.elements)
						max = origin.elements;
					count_t old_elements = elements;
					resizeCopy(elements+max);
					for (count_t i = 0; i < max; i++)
						data[old_elements+i] = origin.data[i];
				}
			
			template <class T, size_t A1>
				inline void appendAdopt(const AlignedArray<T,A1>&origin, count_t max=Undefined)	//! Appends some or all elements of \b origin to the end of the local array using adoptData(). The contained type must provide an adoptData() implementation for this method to work \param origin Ctr::Array to copy from \param max Maximum number of elements to append
				{
					if (max> origin.elements)
						max = origin.elements;
					count_t old_elements = elements;
					resizeAdopt(elements+max);
					for (count_t i = 0; i < max; i++)
						data[old_elements+i].adoptData(origin.data[i]);
				}
			
			template <class T, size_t A1>
				inline void Insert(unsigned offset, const AlignedArray<T,A1>&origin, count_t max=Undefined)	//! Inserts some or all elements of \b origin before the specified offset (0=first element). Elements are duplicated using the = operator @param offset Offset to insert in front of (0= first element) \param origin Ctr::Array to copy from \param max Maximum number of elements to append
				{
					if (offset >= elements)
						append(origin,max);
					else
					{
						if (max> origin.elements)
							max = origin.elements;
						if (!max)
							return;
							
						C	*new_field;
						BYTE*new_root;
						allocAligned<C,A>(new_root,new_field,elements+max);
							
						C	*at = new_field;
						for (count_t i = 0; i < offset; i++)
							(*at++) = data[i];
						for (count_t i = 0; i < max; i++)
							(*at++) = origin.data[i];
						for (count_t i = offset; i < elements; i++)
							(*at++) = data[i];
						elements += max;
						//ASSERT_EQUAL__(at,new_field+elements);
						dealloc(root);
						data = new_field;
						root = new_root;
					}
				}
			
			template <class T, size_t A1>
				inline void insertAdopt(unsigned offset, const AlignedArray<T,A1>&origin, count_t max=Undefined)	//! Inserts some or all elements of \b origin before the specified offset (0=first element) using adoptData(). The contained type must provide an adoptData() implementation for this method to work @param offset Offset to insert in front of (0= first element) \param origin Ctr::Array to copy from \param max Maximum number of elements to append
				{
					if (offset >= elements)
						appendAdopt(origin,max);
					else
					{
						if (max> origin.elements)
							max = origin.elements;
						if (!max)
							return;
							
						C	*new_field;
						BYTE*new_root;
						allocAligned<C,A>(new_root,new_field,elements+max);
							
						C	*at = new_field;
						for (count_t i = 0; i < offset; i++)
							(*at++).adoptData(data[i]);
						for (count_t i = 0; i < max; i++)
							(*at++).adoptData(origin.data[i]);
						for (count_t i = offset; i < elements; i++)
							(*at++).adoptData(data[i]);
						elements += max;
						//ASSERT_EQUAL__(at,new_field+elements);
						dealloc(root);
						data = new_field;
						root = new_root;
					}
				}
			
			inline void	Erase(count_t index, count_t count=1)	//! Erases a number of elements from the array. The remaining elements (if any) are copied via the = operator. \param index Index of the first element to erase with 0 being the first entry \param count Number of elements to erase
			{
				if (index >= elements)
					return;
				if (count > elements-index)
					count = elements-index;
				if (!count)
					return;
				elements -= count;

				C	*new_field;
				BYTE*new_root;
				allocAligned<C,A>(new_root,new_field,elements);
					
				for (unsigned i = 0; i < index; i++)
					new_field[i] = data[i];
				for (unsigned i = index; i < elements; i++)
					new_field[i] = data[i+count];
				dealloc(root);
				data = new_field;
				root = new_root;
			}
			
			inline void	eraseAdopt(count_t index, count_t count=1)	//! Erases a number of elements from the array. The remaining elements (if any) are adopted via adoptData(). The contained type must provide an adoptData() method for this method to work. \param index Index of the first element to erase with 0 being the first entry \param count Number of elements to erase
			{
				if (index >= elements)
					return;
				if (count > elements-index)
					count = elements-index;
				if (!count)
					return;
				elements -= count;
					
				C	*new_field;
				BYTE*new_root;
				allocAligned<C,A>(new_root,new_field,elements);
					
				for (unsigned i = 0; i < index; i++)
					new_field[i].adoptData(data[i]);
				for (unsigned i = index; i < elements; i++)
					new_field[i].adoptData(data[i+count]);
				dealloc(root);
				data = new_field;
				root = new_root;
			}
				
			inline void revert()	//!< Reverses the order of elements in the local field
			{
				for (unsigned i = 0; i < elements/2; i++)
					swp(data[i],data[elements-1-i]);
			}
				
			inline void revertAdopt()	//!< Identical to revert() except that adoptData() is invoked on each element.  The contained type must provide an adoptData() method for this method to work. 
			{
				C	dump;
				for (unsigned i = 0; i < elements/2; i++)
				{
					dump.adoptData(data[i]);
					data[i].adoptData(data[elements-1-i]);
					data[elements-1-i].adoptData(dump);
				}
			}
				
			inline bool concludedBy(const C*element)	const	//! Queries if the specifed entry pointer points exactly one past the last element.
			{
				return element == data+elements;
			}
				
			inline bool Owns(const C*element)	const	//! Queries if the specified entry pointer was taken from the local array. Actual pointer address is checked, not what it points to.
			{
				return element >= data && element < data+elements;
			}
				
			inline count_t GetIndexOf(const C*element)	const	//! Queries the index of the specifed element with 0 being the first element. To determine whether ot not the specified element is member of this array use the Owns() method.
			{
				return element-data;
			}				
				
			inline	void	SetSize(count_t new_size)	//! Resizes the array. The new array's content is uninitialized. \param new_size New array size in elements (may be 0)
			{
				relocAligned<C,A>(root,data,elements,new_size);
			}
			inline	void	resizeCopy(count_t new_size) //! Resizes the array. The new array's content is filled with the elements of the old array as far as possible. \param new_size New array size in elements (may be 0)
			{
				if (new_size == elements || new_size == Undefined)
					return;
				C	*new_field;
				BYTE*new_root;
				allocAligned<C,A>(new_root,new_field,new_size);

				count_t copy = new_size<elements?new_size:elements;
				for (count_t i = 0; i < copy; i++)
					new_field[i] = data[i];
				dealloc(root);
				root = new_root;
				data = new_field;
				elements = new_size;
			}
			inline	void	resizeAdopt(count_t new_size)	//! Resizes the array similar to resizeCopy(). This method is only applicable if the contained type is again of type AlignedArray<...> or any other class providing the 'adoptData' method. For arrays of arrays this method is considerably faster than resizeCopy()
			{
				if (new_size == elements || new_size == Undefined)
					return;
					
				C	*new_field;
				BYTE*new_root;
				allocAligned<C,A>(new_root,new_field,new_size);

				count_t copy = new_size<elements?new_size:elements;
				for (count_t i = 0; i < copy; i++)
					new_field[i].adoptData(data[i]);
				dealloc(root);
				root = new_root;
				data = new_field;
				elements = new_size;
			}
				
			template <class T>
				inline count_t findFirst(const T&entry) const	//!< Finds the index of the first occurance of the specified entry. Entries are compared via the == operator. @param entry Entry to look for @return Index of the found match plus one or 0 if no match was found
				{
					for (count_t i = 0; i < elements; i++)
						if (data[i] == entry)
							return i+1;
					return 0;
				}
				
			template <class T>
				inline count_t	findFirst(const T*field, count_t length) const
				{
					if (length > elements)
						return 0;
					for (count_t i = 0; i <= elements-length; i++)
					{
						bool match(true);
						for (count_t j = 0; j < length; j++)
							if (data[i+j] != field[j])
							{
								match = false;
								break;
							}
						if (match)
							return i+1;
					}
					return 0;
				}
				
			template <class T>
				inline count_t findLast(const T&entry) const
				{
					for (count_t i = elements-1; i < elements; i--)
						if (data[i] == entry)
							return i+1;
					return 0;
				}
				
			template <class T>
				inline count_t	findLast(const T*field, count_t length) const
				{
					if (length > elements)
						return 0;
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
							return i+1;
					}
					return 0;
				}
				
			template <class T>
				inline count_t	findFirst(const T*string) const
				{
					const T*terminator(string);
					while (*terminator++);
					return findFirst(string,terminator-string-1);
				}
				
			template <class T>
				inline count_t	findLast(const T*string) const
				{
					const T*terminator(string);
					while (*terminator++);
					return findLast(string,terminator-string-1);
				}
			
			template <class T, size_t A1>
				inline count_t	findFirst(const AlignedArray<T,A1>&other) const
				{
					return findFirst(other.data,other.elements);
				}
				
			template <class T, size_t A1>
				inline count_t	findLast(const AlignedArray<T,A1>&other) const
				{
					return findLast(other.data,other.elements);
				}
				
			inline C&	First()	//!< Retrieves a reference to the first element in the field. The method will return an undefined result if the local array is empty
			{
				return data[0];
			}
				
			inline const C& First()	const
			{
				return data[0];
			}
				
			inline C&	Last()
			{
				return data[elements-1];
			}
				
			inline const C& Last()	const
			{
				return data[elements-1];
			}
				
			inline count_t GetLength()	const		//! Queries the current array size in elements \return Number of elements 
			{
				return elements;
			}
			inline count_t Count()	const		//! Queries the current array size in elements \return Number of elements 
			{
				return elements;
			}
			inline count_t size()	const		//! Queries the current array size in elements \return Number of elements 
			{
				return elements;
			}
			inline size_t GetContentSize()	const		//! Returns the summarized size of all contained elements in bytes \return Size of all elements
			{
				return (size_t)elements*sizeof(C);
			}
			inline bool isTerminated()	const		//! Queries whether or not the local data ends with an object resolvable to bool that returns false (i.e. a char array with the last character being zero).
			{
				return elements && !data[elements-1];
			}
				
		
			virtual	void			Serialize(IWriteStream&outStream) const	override
			{
				outStream.WriteSize(elements);

				if (!IsISerializable(data))
				{
					outStream.Write(data,(serial_size_t)GetContentSize());
					return;
				}

				for (index_t i = 0; i < elements; i++)
					SerializeObject(data+i,sizeof(C),outStream,true);
			}
		
			virtual	void			Deserialize(IReadStream&inStream)	override
			{
				count_t size;
				inStream.ReadSize(size);

				
				SetSize(size);
				if (!IsISerializable(data))
				{
					inStream.Read(data,(serial_size_t)GetContentSize());
					return;
				}

				for (index_t i = 0; i < elements; i++)
					DeserializeObject(data+i,sizeof(C),inStream,0);
			}

	};



	/**
		\brief two dimensional aligned array
	
		The AlignedArray2D class maps 2d access to a single-dimensional field. Internally it behaves like a normal AlignedArray instance, but it provides some 2d access helper method.
	*/
	template <class C, size_t A>
		class AlignedArray2D:public AlignedArray<C,A>
		{
		protected:
			
				count_t	w;
			
					using AlignedArray<C,A>::resize;
					using AlignedArray<C,A>::resizeCopy;
					using AlignedArray<C,A>::resizeAdopt;
					using AlignedArray<C,A>::erase;
					using AlignedArray<C,A>::eraseAdopt;
					using AlignedArray<C,A>::append;
					using AlignedArray<C,A>::appendAdopt;
		public:
					AlignedArray2D():w(0)
					{}
				template <class T, size_t A1>
					inline AlignedArray2D(const AlignedArray2D<T,A1>&other):AlignedArray<C,A>(other),w(other.w)
					{}
				
					inline AlignedArray2D(const AlignedArray2D<C,A>&other):AlignedArray<C,A>(other),w(other.w)
					{}
				
					inline AlignedArray2D(count_t width, count_t height):AlignedArray<C,A>(width*height),w(width)
					{}
				
			
		inline	count_t	GetWidth()	const	//! Retrieves this array's width \return width
				{
					return w;
				}
		inline	count_t	GetHeight() const	//! Retrieves this array's height \return height
				{
					return w?AlignedArray<C,A>::elements/w:0;
				}
		inline	void		Resize(count_t width, count_t height)	//! Resizes the local 2d array to match the specified dimensions. The local array content is lost if the array's total size is changed
				{
					AlignedArray<C,A>::Resize(width*height);
					w = width;
				}
		template <typename T>
			inline	void		Set(count_t x, count_t y, const T&value)	//! Updates a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() @param value Value to set \return Reference to the requested element
					{
						AlignedArray<C,A>::data[y*w+x] = value;
					}
		
		inline	C&			GetChar(count_t x, count_t y)	//! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return AlignedArray<C,A>::data[y*w+x];
				}
	
		inline	const C&	GetChar(count_t x, count_t y)	const	//! Retrieves a singular element at the specified position \param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return AlignedArray<C,A>::data[y*w+x];
				}
		inline	void	adoptData(AlignedArray2D<C,A>&other)	//! Adopts pointer and size and sets both NULL of the specified origin array.
				{
					w = other.w;
					AlignedArray<C,A>::adoptData(other);
					other.w = 0;
				}			
		};







	/*!
		\brief Ctr::Array dealloc function to prevent accidental access violations.
	*/
	template <class C, size_t A> inline void dealloc(AlignedArray<C,A>&array)
	{
		array.free();
	}

	/*!
		\brief Ctr::Array re_alloc function to prevent accidental access violations.
	*/
	template <class C, size_t A> inline void re_alloc(AlignedArray<C,A>&array, count_t elements)
	{
		array.Resize(elements);
	}

	/*!
		\brief Ctr::Array reloc function to prevent accidental access violations.
	*/
	template <class FieldType, size_t A, class IndexType0, class IndexType1> inline void   reloc(AlignedArray<FieldType,A>&array, IndexType0&length, IndexType1 new_length)
	{
		array.Resize(new_length);
		length = array.GetLength();
	}

	/*!
		\brief Ctr::Array alloc function to prevent accidental access violations.
	*/
	template <class C, size_t A>  inline void alloc(AlignedArray<C,A>&target, count_t elements)
	{
		target.Resize(elements);
	}

}			


#endif
