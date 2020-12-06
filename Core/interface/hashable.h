#ifndef hashableH
#define hashableH

#include "common.h"
//#include "../container/array.h"

namespace DeltaWorks
{
	namespace GlobalHashFunctions
	{}

	/**
	Combines two hash values into a new one
	@param currentHash Continuous (current) hash value
	@param additionalHash Hash to include into the combined hash value
	@return Combined hash value, to use as currentHash in future combinations
	*/
	hash_t	CombineHashes(hash_t currentHash, hash_t additionalHash);



	/**
		@brief Computes a binary hash of the specified memory section

		@param memory Memory to calculate the hash value of. May be NULL if size is 0
		@param size Memory size in bytes

		@return Hash value
	*/
	inline hash_t StdMemHash(const void*memory, size_t size)
	{
		const char*ident = (const char*)memory;
		hash_t	hash = 0;
		for (size_t i = 0; i < size; i++)
		{
			hash += ident[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;	
	}

	/**
		@copydoc stdMemHash()

		Template parameter version for size component, if you're hoping for the compiler to unroll this
	*/
	template <size_t size>
		inline hash_t StdMemHashT(const void*memory)
		{
			const char*ident = (const char*)memory;
			hash_t	hash = 0;
			for (size_t i = 0; i < size; i++)
			{
				hash += ident[i];
				hash += (hash << 10);
				hash ^= (hash >> 6);
			}
			hash += (hash << 3);
			hash ^= (hash >> 11);
			hash += (hash << 15);
			return hash;	
		}

	/**
		@brief Calculates a character based hash value

		@param begin Pointer to the first character to calculate the hash value of
		@param end Pointer one past the last character to calculate the hash value of
		@return Hash value
	*/

	template <typename CharType>
		inline hash_t StdCharHash(const CharType*begin, const CharType*end)
		{
			hash_t	result = 0;
			while (begin != end)
			{
				result += *begin++;
				result += (result << 10);
				result ^= (result >> 6);
			}
			result += (result << 3);
			result ^= (result >> 11);
			result += (result << 15);
			return result;	
		}

	template <typename CharType>
		inline hash_t StdCharHash(const CharType*begin, size_t len)	//! @overload
		{
			return StdCharHash(begin,begin+len);
		}
		

	class HashValue
	{
	private:
		hash_t			current_value;
	public:

		/**/			HashValue():current_value(0) {}
		/**/			HashValue(hash_t initial_value):current_value(initial_value) {}

		HashValue&		Add(hash_t hash_value)
						{
							current_value *= 17;
							current_value += hash_value;
							return *this;
						};

		HashValue&		AddFloat(float value)
						{
							return Add(*(const UINT32*)&value);
						};

		HashValue&		AddFloat(double value)
						{
							return Add(*(const UINT64*)&value);
						};

		template <typename T>
			HashValue&	AddGeneric(const T&value);
		template <typename T>
			HashValue&	operator<<(const T&value);
		template <typename T, count_t ArrayLength>
			HashValue&	operator<<(const T(&value)[ArrayLength]);

		operator hash_t() const
		{
			return current_value;
		}
								
	};



	/**
		@brief Interface used to classify objects that can generate a hash value of their current data

		Ideally hashable objects should also implement == and != operators

	*/
	interface IHashable
	{
	protected:
	virtual							~IHashable()	{};
	public:
	virtual	hash_t					ToHash()	const=0;
	};

	/**
	Extendable list of hash functions
	*/
	namespace GlobalHashFunctions
	{
		hash_t			MemHash(const void*memory, size_t size);

		hash_t 			Hash(const std::string&ident);
		hash_t 			Hash(const std::wstring&ident);
		hash_t 			Hash(const char*ident);
		hash_t 			Hash(const wchar_t*ident);
		hash_t 			Hash(const void*ident);
		hash_t 			Hash(char 	ident);
		hash_t 			Hash(short 	ident);
		hash_t 			Hash(int 	ident);
		hash_t 			Hash(long	ident);
		hash_t 			Hash(long long	ident);
		hash_t 			Hash(unsigned char 	ident);
		hash_t 			Hash(unsigned short 	ident);
		hash_t 			Hash(unsigned int 	ident);
		hash_t 			Hash(unsigned long	ident);
		hash_t 			Hash(unsigned long long	ident);
		hash_t 			Hash(float);
		hash_t 			Hash(double);
		hash_t 			Hash(long double);
		hash_t			Hash(const IHashable&hashable);
		hash_t			Hash(const IHashable*hashable);


		template <typename T0, typename T1>
			inline hash_t 		Hash(const std::pair<T0,T1>&ident);
	};
	template <typename T>
		inline hash_t		HashField(const T*field, count_t length);


	#include "hashable.tpl.h"
}

#endif
