#ifndef iin_streamH
#define iin_streamH

#include "common.h"

#ifndef typedefsH
	#error Missing include
#endif

namespace DeltaWorks
{
	
	template <bool LocalIsLittleEndian>
		inline serial_size_t	DecodeLittleEndian(serial_size_t size)
		{
			return size;
		}

	template <>
		inline serial_size_t	DecodeLittleEndian<false>(serial_size_t size)
		{
			//return (size>>56)|(((size>>48)&0xFF)<<8)|(((size>>40)&0xFF)<<16)|(((size>>32)&0xFF)<<24)|(((size>>24)&0xFF)<<32)|(((size>>16)&0xFF)<<40)|(((size>>8)&0xFF)<<48)|((size&0xFF)<<56);

			return (size>>24)|(((size>>16)&0xFF)<<8)|(((size>>8)&0xFF)<<16)|((size&0xFF)<<24);
		}

	inline serial_size_t	DecodeLittleEndianRT(serial_size_t size)
	{
		if (IsLittleEndian())
			return DecodeLittleEndian<true>(size);
		else
			return DecodeLittleEndian<false>(size);
	}


	/*!
		@brief Generic stream data source
	
		Derivatives of IReadStream provide an interface to load blocks of data. Such a stream could (for instance) be a file, tcp stream, or pipe.
	*/
	interface IReadStream
	{
	protected:
		virtual						~IReadStream()	{};
		
		static void					ParseFlagSet(UINT64 val, bool& a)
		{
			a = (val & 1) != 0;
		}	

		template <typename ... Bools>
			static void				ParseFlagSet(UINT64 val, bool& a, Bools& ... b)
			{
				ParseFlagSet(val>>sizeof...(b),a);
				ParseFlagSet(val,b...);
			}
	public:
		virtual	void				Read(void*target_data, serial_size_t size)=0;	//!< Loads a chunk of binary data from the stream @param target_data Pointer to the memory section that read data should be written to @param size Number of bytes that should be read @return true if the requested amount of bytes could be read from the stream, false otherwise
		virtual void				Read(volatile void*data, serial_size_t size)	{Read(const_cast<void*>(data),size);}
		virtual serial_size_t		GetRemainingBytes()	const = 0;


		template <typename T>
			void					ReadPrimitive(T&element)
			{
				Read(&element,(serial_size_t)sizeof(element));
			}
		template <typename T>
			void					ReadPrimitives(T*elements, count_t num_elements)
			{
				#ifdef _M_X64
					ASSERT__(sizeof(elements) * num_elements < 0x100000000ULL);
				#endif
				Read(elements, (serial_size_t)(sizeof(T)*num_elements));
			}

			/**
			Reads the next big-endian encoded variable-length size value from the local stream.
			The method may throw an exception if the stream fails.
			1,2,3,4, or 8 bytes may be read from the stream
			@param[out] sizeOut Size destination
			*/
			void					ReadSize(serial_size64_t&sizeOut);

			/**
			Reads the next big-endian encoded variable-length size value from the local stream.
			The method may throw an exception if the read value exceeds the capacities of the given target, or if the stream fails.
			1,2,3,4, or 8 bytes may be read from the stream
			@param[out] sizeOut Size destination
			*/
			template <typename T>
				void				ReadSize(T&sizeOut)
				{
					serial_size64_t v;
					ReadSize(v);
					if (v > std::numeric_limits<T>::max())
						throw Except::Memory::SerializationFault(CLOCATION,"Given variable type cannot contain read value");
					sizeOut = static_cast<T>(v);
				}

			serial_size_t			ReadSize()	{serial_size_t rs; ReadSize(rs); return rs;}


			template <typename ... Bools>
				void				ReadFlagSet8(Bools& ... b)
				{
					UINT8 val;
					ReadPrimitive(val);
					if (sizeof...(b) > 8)
						throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 8 parameters");
					ParseFlagSet(val,b...);
				}

			template <typename ... Bools>
				void				ReadFlagSet16(Bools& ... b)
				{
					UINT16 val;
					ReadPrimitive(val);
					if (sizeof...(b) > 16)
						throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 16 parameters");
					ParseFlagSet(val,b...);
				}

			template <typename ... Bools>
				void				ReadFlagSet32(Bools& ... b)
				{
					UINT32 val;
					ReadPrimitive(val);
					if (sizeof...(b) > 32)
						throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 32 parameters");
					ParseFlagSet(val,b...);
				}

			template <typename ... Bools>
				void				ReadFlagSet64(Bools& ... b)
				{
					UINT64 val;
					ReadPrimitive(val);
					if (sizeof...(b) > 64)
						throw Except::Program::ParameterFault(CLOCATION,"This method supports only up to 64 parameters");
					ParseFlagSet(val,b...);
				}
	};




	/*!
	@brief Binary memory section in-stream
	
	MemReadStream allows stream reading from a memory section. MemReadStream does not delete any assigned data on destruction.
	*/
	class MemReadStream: public IReadStream
	{
	protected:
		const BYTE		*field,	//!< Pointer to the current memory location that is next read from. Auto incremented as needed
						*end;	//!< Pointer one past the last available byte. Data can be read up to but not past this pointer.
	public:

		/**/			MemReadStream(const void*data=NULL,size_t size = 0)
						{
							Assign(data,size);
						}
		void			Assign(const void*data, size_t size)	//!< Assigns a new field to the local memory stream
						{
							field = (const BYTE*)data;
							end = field+size;
							DBG_ASSERT__(size == 0 || data != nullptr);
						}
		const BYTE*		GetCurrent()	const
						{
							return field;
						}

		serial_size_t	GetRemainingBytes() const override {return end - field;}
					
		void			Read(void*target_data, serial_size_t size)	override
						{
							if (field+size > end)
								throw Except::Memory::SerializationFault(CLOCATION, "Stream reached end");
							memcpy(target_data,field,size);
							field+=size;
						}
	};
}

#endif
