#ifndef string_bufferH
#define string_bufferH

#include "../io/str_stream.h"
#include "../container/vectorList.h"


namespace DeltaWorks
{
	template <typename T>
	class GenericStringBuffer : public Container::PreAllocVector<T>
	{
	public:
		typedef GenericStringBuffer<T>	Self;
		typedef Container::PreAllocVector<T>	Super;
		typedef StringType::ReferenceExpression<T>	StringRef;
		typedef StringType::Template<T>	String;

		using Super::GetPointer;
		using Super::GetLength;

		/**/				GenericStringBuffer(size_t len=1024);
		/**/				GenericStringBuffer(const T*string, size_t extra=100);
		/**/				GenericStringBuffer(const String&string, size_t extra=100);
		bool				operator==(const StringRef&other) const;
		bool				operator==(const String&other) const	{return operator==(other.ref());}
		bool				operator==(const T*other) const			{return operator==(StringRef(other));}
		bool				operator!=(const StringRef&other) const	{return !operator==(other);}
		bool				operator!=(const String&other) const	{return !operator==(other);}
		bool				operator!=(const T*other) const			{return !operator==(other);}
		Self&				operator=(const Self&other);
		Self&				operator=(const String&string);
		Self&				operator=(const T*string);
		Self&				operator<<(const String&);
		Self&				operator<<(const T*);
		Self&				operator<<(const Self&);
		Self&				operator<<(const TNewLine&);
		Self&				operator<<(const TSpace&);
		Self&				operator<<(const TTabSpace&);
		Self&				operator<<(const Ctr::ArrayData<T>&);
		Self&				operator<<(const StringRef&);
		Self&				operator<<(T);
		Self&				operator<<(BYTE);
		Self&				operator<<(long long);
		Self&				operator<<(unsigned long long);
		Self&				operator<<(int);
		Self&				operator<<(unsigned);
		Self&				operator<<(long);
		Self&				operator<<(unsigned long);
		Self&				operator<<(short);
		Self&				operator<<(unsigned short);
		Self&				operator<<(float);
		Self&				operator<<(double);
		
		void				Insert(index_t index, T chr);	//!< Inserts a single character before the specified position (0=first character)
		void				Insert(index_t index, const T*str, size_t len);	//!< Inserts a string segment before the specified position (0=first character)
		void				Insert(index_t index, const String&string)		{Insert(index,string.c_str(),string.GetLength());}
		void				Insert(index_t index, const StringRef&string)	{Insert(index,string.pointer(),string.GetLength());}
		
		inline T			operator[](index_t index)	const{return Super::operator[](index);}
		
		StringRef			ToStringRef()	const;		//!< Returns a string reference to the local content
		String				CopyToString()		const;		//!< Copies the local content to a string

		inline void			Write(const T*str, size_t strLen)	{memcpy(Super::AppendRow(strLen),str,strLen*sizeof(T));}

	private:
		template <typename N, typename UN>
			void			WriteSigned(N value);
		template <typename N>	
			void			WriteUnsigned(N value);
		template <typename N>	
			void			WriteFloat(N value, unsigned precision);
	};





	template <typename T>
	GenericStringBuffer<T>::GenericStringBuffer(size_t l):Super(l)
	{}

	template <typename T>
	GenericStringBuffer<T>::GenericStringBuffer(const T*string, size_t extra):Super(0)
	{
		size_t strLen = CharFunctions::strlen(string);
		memcpy(Super::AppendRow(strLen),string,strLen);
	}

	template <typename T>
	GenericStringBuffer<T>::GenericStringBuffer(const String&string, size_t extra):Super(string.GetLength()+extra)
	{
		memcpy(Super::AppendRow(string.GetLength()),string.c_str(),string.GetLength());
	}

	template <typename T>
	bool				GenericStringBuffer<T>::operator==(const StringRef&other) const
	{
		return this->GetLength() == other.GetLength() && !memcmp(GetPointer(),other.GetPointer(),GetLength());
	}



	template <typename T>
	GenericStringBuffer<T>&	GenericStringBuffer<T>::operator=(const GenericStringBuffer<T>&other)
	{
		Super::operator=(other);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&	GenericStringBuffer<T>::operator=(const T*string)
	{
		Super::clear();
		return operator<<(string);
	}


	template <typename T>
	GenericStringBuffer<T>&	GenericStringBuffer<T>::operator=(const String&other)
	{
		Super::clear();
		return operator<<(other);
	}



	template <typename T>
	template <typename N, typename UN>
		void GenericStringBuffer<T>::WriteSigned(N value)
		{
			bool negative = false;
			if (value < 0)
			{
				value*=-1;
				negative = true;
			}

			T	char_buffer[256],
					*end = char_buffer+ARRAYSIZE(char_buffer),
					*c = end;
			UN uval = value;
			while (uval && c != char_buffer)
			{
				(*(--c)) = (T)('0'+(uval%10));
				uval/=10;
			}
			if (c==end)
				(*(--c)) = (T)'0';
			if (negative)
			{
				static const T neg = (T)'-';
				Write(&neg,1);
			}
			Write(c,end-c);
		}

	template <typename T>	
	template <typename N>	
		void GenericStringBuffer<T>::WriteUnsigned(N value)
		{
			T	char_buffer[256],
				*end = char_buffer+ARRAYSIZE(char_buffer),
				*c = end;
			while (value && c != char_buffer)
			{
				(*(--c)) = (T)('0'+(value%10));
				value/=10;
			}
			if (c==end)
				(*(--c)) = (T)'0';
			Write(c,end-c);
		}

	template <typename T>
		template <typename N>	
		void GenericStringBuffer<T>::WriteFloat(N value, unsigned precision)
		{
			T	char_buffer[256],
					*str = String::FloatToStr(value, precision, false, char_buffer+ARRAYSIZE(char_buffer), char_buffer);

			Write(str,char_buffer+ARRAYSIZE(char_buffer)-str);
		}


	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(T value)
	{
		Super::Append(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(BYTE value)
	{
		WriteUnsigned(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(long long value)
	{
		WriteSigned<long long, unsigned long long>(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(unsigned long long value)
	{
		WriteUnsigned(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(int value)
	{
		WriteSigned<int,unsigned>(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(unsigned value)
	{
		WriteUnsigned(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(long value)
	{
		WriteSigned<long, unsigned long>(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(unsigned long value)
	{
		WriteUnsigned(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(short value)
	{
		WriteSigned<short,unsigned short>(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(unsigned short value)
	{
		WriteUnsigned(value);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(float value)
	{
		WriteFloat(value,5);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(double value)
	{
		WriteFloat(value,8);
		return *this;
	}


	template <typename T>
	GenericStringBuffer<T>&		GenericStringBuffer<T>::operator<<(const Ctr::ArrayData<T>&data)
	{
		if (data.IsEmpty())
			return *this;
		if (!data.Last())	//terminating zero
			Write(data.pointer(),data.GetLength()-1);
		else
			Super::AppendAll(data);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&		GenericStringBuffer<T>::operator<<(const StringRef&ref)
	{
		Write(ref.pointer(),ref.GetLength());
		return *this;
	}




	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(const GenericStringBuffer<T>&buffer)
	{
		Super::AppendAll(buffer);
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(const String&str)
	{
		Write(str.c_str(),str.GetLength());
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(const T*str)
	{
		Write(str,CharFunctions::strlen(str));
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(const TNewLine&)
	{
		Super::Append((T)'\n');
		return *this;
	}


	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(const TSpace&space)
	{
		T*out = Super::AppendRow(space.length);
		for (size_t i = 0; i < space.length; i++)
			(*out++) = (T)' ';
		return *this;
	}

	template <typename T>
	GenericStringBuffer<T>&  GenericStringBuffer<T>::operator<<(const TTabSpace&space)
	{
		T*out = Super::AppendRow(space.length);
		for (size_t i = 0; i < space.length; i++)
			(*out++) = (T)'\t';
		return *this;
	}


	template <typename T>
	StringType::Template<T>	 GenericStringBuffer<T>::CopyToString()	const
	{
		return String(Super::pointer(),Super::Count());
	}

	template <typename T>
	StringType::ReferenceExpression<T>	GenericStringBuffer<T>::ToStringRef()	const
	{
		return StringRef(Super::pointer(),Super::Count());
	}


	template <typename T>
	void			GenericStringBuffer<T>::Insert(index_t index, T chr)
	{
		Super::Insert(index,chr);
	}


	template <typename T>
	void			GenericStringBuffer<T>::Insert(index_t index, const T*str, size_t len)
	{
		memcpy(Super::InsertRow(index,len),str,len);
	}



	typedef GenericStringBuffer<char>	StringBuffer;
	typedef GenericStringBuffer<wchar_t>	StringBufferW;
	typedef GenericStringBuffer<char32_t>	UTF32StringBuffer;















}

#endif
