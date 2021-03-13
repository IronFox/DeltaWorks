#ifndef byte_streamH
#define byte_streamH

#include "../interface/read_stream.h"
#include "../interface/write_stream.h"
#include "../container/array.h"
#include "../container/buffer.h"

namespace DeltaWorks
{
	class ByteStreamBuffer : public IWriteStream
	{
	public:
		typedef ByteStreamBuffer	 Self;

		virtual	void	Write(const void*data, serial_size_t size)	override {if (!size) return; memcpy(this->data.AppendRow(size),data,size);}

		ArrayRef<const BYTE>	GetWrittenData() const {return data.ToRef().Const();}
		void			Clear()	{data.Clear();}

		template <typename T>
			Self&		Append(const T*field, count_t numElements)
			{
				using Serialization::Serialize;
				for (index_t i = 0; i < numElements; i++)
					Serialize(*this,field[i]);
				return *this;
			}

		template <typename T>
			Self&		operator<<(const T&data)
			{
				using Serialization::Serialize;
				Serialize(*this,data);
				return *this;
			}


		template <typename T>
			Self&		Append(const T&data)
			{
				using Serialization::Serialize;
				Serialize(*this,data);
				return *this;
			}

	private:
		Buffer<BYTE>	data;
	};


	class ByteReadStream : public IReadStream
	{
	private:
		const char		*begin,
						*current,
						*end;
	
		void			_GetData(void*pntr, size_t size);
		bool			_TryGetData(void*pntr, size_t size);
	public:

		/**/			ByteReadStream() :begin(NULL), current(NULL), end(NULL)	{}
		/**/			ByteReadStream(const void*begin, const void*end) :begin((const char*)begin), current((const char*)begin), end((const char*)end)	{}
		/**/			ByteReadStream(const ArrayRef<const BYTE>&data) : begin((const char*)data.begin()),current((const char*)data.begin()),end((const char*)data.end()){}
	
		void			Rewind()	{ current = begin; }
		bool			ReachedEnd() const { return current == end; }
		template <class C>
			void		Stream(C&out)	{ _GetData(&out, sizeof(C)); }
		template <class C>
			void		Stream(C*array, count_t count)	{ _GetData(array, sizeof(C)*count); }
		template <class C>
			bool		TryStream(C&out)	{ return _TryGetData(&out, sizeof(C)); }
		template <class C>
			bool		TryStream(C*array, count_t count)	{ return _TryGetData(array, sizeof(C)*count); }
		template <class C>
			ByteReadStream&	operator>>(C&object)	{ Stream(object); return *this; }

		virtual	void	Read(void*target_data, serial_size_t size)	override { _GetData(target_data, size); }
		virtual serial_size_t		GetRemainingBytes() const override { return end - current; }
	};


	#include "byte_stream.tpl.h"
}

#endif
