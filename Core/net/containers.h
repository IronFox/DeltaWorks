#ifndef netContainersH
#define netContainersH

#include "../global_root.h"
#include "../io/bz2.h"
#include "../global_string.h"
#include "../container/vectorList.h"
//#include "tcp.h"

namespace DeltaWorks
{



	namespace Package
	{
		/**
		Checks whether a character is a valid network-character.
		Valid characters are currently all characters for which StringFunctions::IsInvalidChar() returns false
		@return true if the specified character is valid, false otherwise
		*/
		bool			IsValidChar(char32_t c);
		/**
		Checks whether or not the specified string is a valid network string.
		Each character must pass IsValidChar(), and the length of the local string must be less or equal than @a maxLength (if set)
		@param str String to test
		@param maxLength Maximum allowed string length
		@return true if the specified string is valid, false otherwise
		*/
		bool			IsValid(const StringRef&str, const count_t maxLength=std::numeric_limits<count_t>::max());
		bool			IsValid(const UTF32StringRef&str, const count_t maxLength=std::numeric_limits<count_t>::max());
		/**
		Similar to IsValid(), but throws detailed exceptions
		*/
		void			CheckValidity(const StringRef&str, const count_t maxLength=std::numeric_limits<count_t>::max());
		/**
		@copydoc IsValid(const StringRef&,const count_t maxLength)
		*/
		inline bool		IsValid(const String&str, const count_t maxLength=std::numeric_limits<count_t>::max()) {return IsValid(str.ToRef(),maxLength);}
		inline bool		IsValid(const UTF32String&str, const count_t maxLength=std::numeric_limits<count_t>::max()) {return IsValid(str.ToRef(),maxLength);}
		/**
		Similar to IsValid(), but triggers detailed fatals in debug mode, and silently fixes issues in release. Any found error in debug mode terminate the local program
		*/
		void			AssertValidity(String&str, const count_t maxLength=std::numeric_limits<count_t>::max());
		/**
		@copydoc CheckValidity(const StringRef&,const count_t maxLength)
		*/
		inline void		CheckValidity(const String&str, const count_t maxLength=std::numeric_limits<count_t>::max()){CheckValidity(str.ToRef(),maxLength);}


		template <serial_size_t max_size>
			class NetString:public String
			{
				typedef String Super;
			public:
				typedef NetString<max_size>	Self;

				static const size_t	MaxSize = max_size;
				static const size_t	MaxLength = max_size;

				/**/				NetString()
									{}
				/**/				NetString(const String&string):String(string)
									{
										AssertValidity(*this,MaxLength);
									}
				/**/				NetString(const char*string):String(string)
									{
										AssertValidity(*this,MaxLength);
									}
				/**/				NetString(const StringRef&string):String(string)
									{
										AssertValidity(*this,MaxLength);
									}

				friend void			Deserialize(IReadStream&s,Self&v)
				{
					const serial_size_t len=s.ReadSize();
					if (len > MaxLength)
						throw Except::Memory::SerializationFault(CLOCATION,"Deserialized string length "+String(len)+" exceeds maximum safe size "+String(MaxLength));
					v.SetLength(len);
					s.Read(v.mutablePointer(),len);
					CheckValidity(v,MaxLength);
				}

				void				operator=(const String&other)
				{
					String::operator=(other);
					AssertValidity(*this,MaxLength);
				}
			};


			#if 0
		template <typename T>
			class Native:public SerializableObject
			{
				typedef Native<T>				Self;
			public:
				T								value;
				
												Native():value()
												{}
												Native(T value_):value(value_)
												{}
											
				virtual	serial_size_t			GetSerialSize() const	override
				{
					return sizeof(T);
				}
				
				virtual	void					Serialize(IWriteStream&outStream) const	override
				{
					outStream.Write(&value,sizeof(value));
				}
				
				virtual	void					Deserialize(IReadStream&inStream)	override
				{
					inStream.Read(&value,sizeof(value));
				}

				virtual	bool					HasFixedSize() const	override
				{
					return true;
				}

				void							swap(Native<T>&other)		{swp(value,other.value);}
				friend void						swap(Self&a, Self&b)		{a.swap(b);}
				void							adoptData(Native<T>&other)	{value = other.value;}
			
				void							operator|=(const T&other)	{value |= other;}
				void							operator&=(const T&other)	{value &= other;}
				void							operator+=(const T&other)	{value += other;}
				void							operator-=(const T&other)	{value -= other;}
				void							operator*=(const T&other)	{value *= other;}
				void							operator/=(const T&other)	{value /= other;}
				template <typename T1>
				bool							operator==(const T1&other) const
				{
					return value == other;
				}
	//			T								operator&(const T&other) const	{return value&other;}
				
				Native<T>&					operator=(T id_)
				{
					value = id_;
					return *this;
				}
				
				operator T()	const
				{
					return value;
				}				
			};
			#endif /*0*/
		
		template <class C0, class C1>
			class Tuple:public SerializableObject
			{
				typedef Tuple<C0,C1>		Self;
			public:
				C0							a;
				C1							b;

				/**/						Tuple()	{}
				/**/						Tuple(const C0&a_, const C1&b_):a(a_),b(b_){}
				/**/						Tuple(C0&& a_, C1&& b_) :a(std::move(a_)), b(std::move(b_)) {}


				virtual	void						Serialize(IWriteStream&outStream) const	override
				{
					a.Serialize(outStream);
					b.Serialize(outStream);
				}
				
				virtual	void						Deserialize(IReadStream&inStream)	override
				{
					a.Deserialize(inStream);
					b.Deserialize(inStream);
				}

				void						swap(Self&other)
											{
												a.swap(other.a);
												b.swap(other.b);
											}
				friend void					swap(Self&a, Self&b)		{a.swap(b);}
				void						adoptData(Self&other)
											{
												a.adoptData(other.a);
												b.adoptData(other.b);
											}

				Self&						setA(const C0&a_)
											{
												a = a_;
												return *this;
											}

				Self&						setB(const C1&b_)
											{
												b = b_;
												return *this;
											}
			};
	
	
		
		class CompressedString:public SerializableObject
		{
			typedef CompressedString	Self;
		public:
			Ctr::Array<BYTE>		compressed;
			String			uncompressed;
			bool			is_compressed;

	
			void			swap(CompressedString&other)
							{
								compressed.swap(other.compressed);
								uncompressed.swap(other.uncompressed);
								swp(is_compressed,other.is_compressed);
							}
			friend void		swap(Self&a, Self&b)		{a.swap(b);}
			void			adoptData(CompressedString&other)
							{
								compressed.adoptData(other.compressed);
								uncompressed.adoptData(other.uncompressed);
								is_compressed = other.is_compressed;
							}


			
			friend void Serialize(IWriteStream&s, const CompressedString&v)
			{
				using Serialization::Serialize;
				s.WritePrimitive(v.is_compressed);
				if (v.is_compressed)
				{
					UINT32 decompressed_size = UINT32(v.uncompressed.GetLength());
					s.WriteSize(v.uncompressed.GetLength());
					Serialize(s,v.compressed);
				}
				else
					Serialize(s,v.uncompressed);
			}
			
			friend void Deserialize(IReadStream&s, CompressedString&v)
			{
				using Serialization::Deserialize;
				s.ReadPrimitive(v.is_compressed);
				if (v.is_compressed)
				{
					const serial_size_t	decompressed_size = s.ReadSize();
					Deserialize(s,v.compressed);
					//cout << "extracted "<<compressed.GetContentSize()<<" compressed byte(s). uncompressed="<<decompressed_size<<endl;
					v.uncompressed.setLength(decompressed_size);
					const serial_size_t result = (serial_size_t)BZ2::decompress(v.compressed.pointer(),v.compressed.GetContentSize(),v.uncompressed.mutablePointer(),decompressed_size);
					if (result != decompressed_size)
						throw Except::Memory::SerializationFault(CLOCATION,"Decompression returned "+String(result)+". Expected "+String(decompressed_size)+".ignoring package");
				}
				else
					Deserialize(s,v.uncompressed);

				if (!IsValid(v.uncompressed,std::numeric_limits<count_t>::max()))
					throw Except::Memory::SerializationFault(CLOCATION,"Decompression string contains invalid character");
			}	
			
			void			Set(const String&string)
			{
				uncompressed = string;
				update();
			}
			
			void			update()
			{
				static Ctr::Array<BYTE>	buffer;
				if (buffer.GetLength() < uncompressed.GetLength())
					buffer.SetSize(uncompressed.GetLength());
				size_t compressed_size = BZ2::compress(uncompressed.c_str(),uncompressed.GetLength(),buffer.pointer(),buffer.GetLength());
				is_compressed = compressed_size>0 && compressed_size+SerialSizeScanner::GetSerialSizeOfSize((serial_size_t)uncompressed.GetLength()) < uncompressed.GetLength();
				if (is_compressed)
					compressed.copyFrom(buffer.pointer(),(count_t)compressed_size);
				else
					compressed.free();
			}
	
		};
	
		template <class C>
			class CompressedArray:public SerializableObject
			{
				typedef CompressedArray<C>		Self;
			public:
				Ctr::Array<BYTE>		compressed,
								serialized;
				Ctr::Array<C>		uncompressed;
				bool			is_compressed;

				void			swap(CompressedArray<C>&other)
								{
									compressed.swap(other.compressed);
									serialized.swap(other.serialized);
									uncompressed.swap(other.uncompressed);
									swp(is_compressed,other.is_compressed);
								}
				friend void		swap(Self&a, Self&b)		{a.swap(b);}
				void			adoptData(CompressedArray<C>&other)
								{
									compressed.adoptData(other.compressed);
									serialized.adoptData(other.serialized);
									uncompressed.adoptData(other.uncompressed);
									is_compressed = other.is_compressed;
								}
		
				
				virtual	bool	Serialize(IWriteStream&outStream) const	override
								{
									if (!outStream.Write(&is_compressed,sizeof(is_compressed)))
										return false;
									if (is_compressed)
									{
										size_t decompressed_size = serialized.GetLength();
										return outStream.WriteSize(decompressed_size) && compressed.Serialize(outStream);
									}
									//cout << "serialize count="<<uncompressed.Count()<<endl;
									return serialized.Serialize(outStream);
								}
				
				virtual	bool	Deserialize(IReadStream&inStream)	override
								{
									if (!inStream.Read(&is_compressed,sizeof(is_compressed)))
										return false;
									//cout << "fixed size remaining: "<<fixed_size<<endl;
									if (is_compressed)
									{
										//cout << "compressed"<<endl;
										serial_size_t	serial_size;
										if (!inStream.ReadSize(serial_size))
											return false;
										if (!compressed.Deserialize(inStream))
											return false;
										serialized.SetSize(serial_size);
										if (BZ2::decompress(compressed.pointer(),compressed.GetContentSize(),serialized.pointer(),serial_size) != serial_size)
											return false;
										MemReadStream stream(serialized.pointer(),(serial_size_t)serialized.GetContentSize());
										return uncompressed.Deserialize(stream);
									}
									//cout << "deserialized array is not compressed"<<endl;
									if (!serialized.Deserialize(inStream))
										return false;
									//cout << "primary deserialization succeeded"<<endl;
									/*for (index_t i = 0; i < serialized.size(); i++)
									{
										if (Template::isalnum(serialized[i]))
											cout << ' '<<serialized[i];
										else
											cout << ' '<<String((UINT)serialized[i]);
									}
									cout << endl;*/
									MemReadStream	inner_read(serialized.pointer(),(serial_size_t)serialized.GetContentSize());
									return uncompressed.Deserialize(inner_read);
								}	
				
				void			serializeAndCompress()
				{
					WriteBuffer<>	buffer;
					uncompressed.serialize(buffer,true);
					buffer.moveToArray(serialized);
					size_t compressed_size = serialized.GetLength() > 256 ? BZ2::compress(serialized.pointer(),serialized.GetLength(),buffer.pointer(),buffer.GetLength()) : 0;
					is_compressed = compressed_size>0 && compressed_size < serialized.GetLength();	//the latter comparison should be redundant...
					if (is_compressed)
					{
						compressed.SetSize(compressed_size);
						compressed.copyFrom(buffer.pointer());
					}
					else
						compressed.free();
				}
		
			};		
		}




	namespace Strategy
	{
	template <serial_size_t max_size>
		class StrategySelector<Package::NetString<max_size> >
		{
		public:
		typedef	Strategy::Swappable	Default;
		};
	}
}
DECLARE__(Package::CompressedString,Swappable);
DECLARE_T__(Package::CompressedArray,Swappable);
//DECLARE_T__(Package::Native,Swappable);
DECLARE_T2__(Package::Tuple,typename,typename,Swappable);

#endif
