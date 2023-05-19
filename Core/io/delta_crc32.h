#ifndef crc32H
#define crc32H

/******************************************************************

crc32-hashsum calculator.

******************************************************************/


namespace DeltaWorks
{

	/**
	@brief CRC32 Checksum calculation

	Calculates a sequenced CRC32 checksum. 
	*/
	namespace CRC32
	{
		class	Sequence
		{
		private:
			UINT32		status;
		public:
			typedef UINT32	HashContainer;	//for interchangability

			/**/		Sequence();
			/**/		Sequence(const void* source, size_t size)
			{
				Reset();
				Append(source, size);
			}
			void		Reset();								//!< Restarts the checksum sequence (this method is automatically called by the constructor)
			Sequence&	Append(const void*source, size_t size);	//!< Appends data to the CRC32 volume and advances the internal status @param source Binary data to insert into the calculation @param size Number of bytes that should be inserted into the calculation. @a source must provide at least this many bytes
			template<typename T>
				Sequence&AppendPOD(const T&pod)					{return Append(&pod,sizeof(T));}

			UINT32		Finish()	const;						//!< Resolves a result based on the current internal status. The method does not actually change anything and CRC32 calculation may be continued if desired
			void		Finish(UINT32&rs) const {rs = Finish();}
		};
	}

	/**
		\brief Compares two given data fields for equality
		\param pntr0 Pointer to the first byte of the first data field
		\param pntr1 Pointer to the first byte of the second data field
		\param size Size of both data fields in bytes
		\return 0 if both fields are identical, -1 if pntr0 is less than pntr1 and 1 if pntr0 is greater than pntr1

		fastCompare compares the given data fields and returns a linear order based on the content of the specified data fields.

	*/
	char        fastCompare(const void*pntr0, const void*pntr1, size_t size);


}

#endif
