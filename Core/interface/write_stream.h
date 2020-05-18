#ifndef iout_streamH
#define iout_streamH

#include "common.h"


namespace DeltaWorks
{
	template <bool LocalIsLittleEndian>
		inline serial_size_t	EncodeLittleEndian(serial_size_t size)
		{
			return size;
		}

	template <>
		inline serial_size_t	EncodeLittleEndian<false>(serial_size_t size)
		{
			//return (size>>56)|(((size>>48)&0xFF)<<8)|(((size>>40)&0xFF)<<16)|(((size>>32)&0xFF)<<24)|(((size>>24)&0xFF)<<32)|(((size>>16)&0xFF)<<40)|(((size>>8)&0xFF)<<48)|((size&0xFF)<<56);
			return (size>>24)|(((size>>16)&0xFF)<<8)|(((size>>8)&0xFF)<<16)|((size&0xFF)<<24);
		}

	inline serial_size_t	EncodeLittleEndianRT(serial_size_t size)
	{
		if (IsLittleEndian())
			return EncodeLittleEndian<true>(size);
		else
			return EncodeLittleEndian<false>(size);
	}

	template<class T, bool isEnum=false>
	struct GenericVTableTest
	{
		class TestClass : public T
		{
			virtual void SomeVirtualMethod()
			{}
		};
		static const bool HasVTable = (sizeof(T) == sizeof(TestClass));
	};

	template<class T>
	struct GenericVTableTest<T,true>
	{
		static const bool HasVTable = false;
	};



	#define PRIMITIVE_NO_VTABLE(TYPE)\
		template<> struct GenericVTableTest<TYPE,false> {static const bool HasVTable = false;};

	PRIMITIVE_NO_VTABLE(bool)
	PRIMITIVE_NO_VTABLE(char)
	PRIMITIVE_NO_VTABLE(unsigned char)
	PRIMITIVE_NO_VTABLE(signed char)
	PRIMITIVE_NO_VTABLE(short)
	PRIMITIVE_NO_VTABLE(unsigned short)
	PRIMITIVE_NO_VTABLE(int)
	PRIMITIVE_NO_VTABLE(unsigned int)
	PRIMITIVE_NO_VTABLE(long)
	PRIMITIVE_NO_VTABLE(unsigned long)
	PRIMITIVE_NO_VTABLE(long long)
	PRIMITIVE_NO_VTABLE(unsigned long long)
	PRIMITIVE_NO_VTABLE(float)
	PRIMITIVE_NO_VTABLE(double)
	PRIMITIVE_NO_VTABLE(long double)

	template <bool HasVTable>
		class VTableAssertion
		{
		public:
			static inline void	AssertNot()
			{
			}
		};

	template <>
		class VTableAssertion<true>
		{
		//public:	//private, so compilation will fail outright
			static inline void	AssertNot()
			{
				FATAL__("VTable detected. Cannot interpret this type as primitive.")
			}
		};


	#define OBJECT_HAS_VTABLE(type) HasVTable<type>::Value

	/*!
	@brief Generic data output stream
	
	Derivatives of IOutStream provide an interface to write blocks of data. Such a stream could (for instance) be a file, tcp stream, or pipe.
	*/
	interface IWriteStream
	{
	protected:
		virtual			~IWriteStream()	{};
		

		static UINT64	CompileFlagSet(bool a)
		{
			return !!a;
		}

		template <typename ... Bools>
			static UINT64	CompileFlagSet(bool a,	Bools ... b)
			{
				return (CompileFlagSet(a)<<sizeof...(b))|CompileFlagSet(b...);
			}

	public:
		virtual	void	Write(const void*data, serial_size_t size)=0;	//!< Writes a chunk of binary data to the stream @param data Pointer to the memory section that should be written @param size Number of bytes that should be written @return true if the specified amount of bytes could be written to the stream, false otherwise
		virtual void	Write(const volatile void*data, serial_size_t size)	{Write(const_cast<const void*>(data),size);}

		template <typename T>
			void		WritePrimitive(const T&element)
			{
				VTableAssertion<GenericVTableTest<T,std::is_enum<T>::value>::HasVTable>::AssertNot();
				Write(&element, (serial_size_t)sizeof(element));
			}
		template <typename T>
			void		WritePrimitives(const T*elements, count_t num_elements)
			{
				VTableAssertion<GenericVTableTest<T,std::is_enum<T>::value>::HasVTable>::AssertNot();
				Write(elements, (serial_size_t)(sizeof(T)*num_elements));
			}

		void		WriteSize(serial_size64_t size);	//! Writes the content of a 64bit size variable in bit endian. This operation may throw an exception of @a size exceeds 2^61-1

		template <typename Numeric>
			void	WriteSize(Numeric size)
			{
				WriteSize((serial_size64_t)size);
			}


		template <typename ... Bools>
			void	WriteFlagSet8(Bools ... a)
			{
				UINT64 val = CompileFlagSet(a...);
				if (sizeof...(a) > 8)
					throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 8 parameters");
				WritePrimitive((BYTE)val);
			}

		template <typename ... Bools>
			void	WriteFlagSet16(Bools ... a)
			{
				UINT64 val = CompileFlagSet(a...);
				if (sizeof...(a) > 16)
					throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 16 parameters");
				WritePrimitive((UINT16)val);
			}

		template <typename ... Bools>
			void	WriteFlagSet32(Bools ... a)
			{
				UINT64 val = CompileFlagSet(a...);
				if (sizeof...(a) > 32)
					throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 32 parameters");
				WritePrimitive((UINT32)val);
			}

		template <typename ... Bools>
			void	WriteFlagSet64(Bools ... a)
			{
				UINT64 val = CompileFlagSet(a...);
				if (sizeof...(a) > 64)
					throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 64 parameters");
				WritePrimitive(val);
			}
	};


	/**
	Helper class to detect the serial size of an object. No data is actually stored or written anywhere.
	This replaces the use of GetSerialSize() as was common before. <br />
	Example:<br />
		SerialSizeScanner scanner;<br />
		object.Serialize(scanner);<br />
		byteSize = scanner.GetTotalSize();
	*/
	class SerialSizeScanner : public IWriteStream
	{
	public:
		/**
		Retrieves the byte-size of a given non-negative size variable that should be serialized using IWriteStream::WriteSize().
		@param size Size variable to determine the byte size of. Must be non-negative
		@return Number of bytes occupied by a given size variable when written using IWriteStream::WriteSize(). May be 1,2,3,4, or 8
		*/
		template <typename T>
			static	serial_size_t		GetSerialSizeOfSize(T size)
			{
				if (size <= 0x3F)
					return 1;
				if (size <= 0x3FFF)
					return 2;
				if (size <= 0x3FFFFF)
					return 3;
				if (size < (1U<<29))
					return 4;
				return 8;
			}

		virtual void	Write(const void*, serial_size_t size)	override
		{
			this->size += size;
		}

		serial_size_t	GetTotalSize() const {return size;}

	private:
		serial_size_t	size = 0;

	};


	/*!
		@brief Binary memory section write-stream
	
		MemWriteStream allows stream writing to a memory section. MemWriteStream does not delete any assigned data on destruction.
	*/
	class MemWriteStream: public IWriteStream
	{
	protected:
		BYTE			*field,	//!< Pointer to the current memory location that is next written to. Auto incremented as needed
						*end;	//!< Pointer one past the last available byte of the out-buffer. Data can be written up to but not past this pointer.

	public:
		/**/			MemWriteStream(void*data=NULL,size_t size = 0)
						{
							Assign(data,size);
						}
		void			Assign(void*data, size_t size)	//!< Assigns a new field to the local memory stream
						{
							field = (BYTE*)data;
							end = field+size;
						}
		
		BYTE*			GetCurrent()	const
						{
							return field;
						}

		size_t			GetRemainingSpace() const {return end - field;}
				
		virtual void	Write(const void*data, serial_size_t size)	override
						{
							if (field+size > end)
								throw Except::Memory::SerializationFault(CLOCATION, "Stream reached end");
							memcpy(field,data,size);
							field+=size;
						}
	};

}

#endif
