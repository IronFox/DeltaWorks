#ifndef riff_handlerH
#define riff_handlerH
#include <stdio.h>


class File;
class Chunk;
struct TRiffInfo;
struct SRiffChunk;

#include "random_access_file.h"
#include "byte_stream.h"
#include <ctype.h>
#define F_OPEN_CHECK	//undefine to disable file-open-checking
#define RIFF_SENSITIVE	//undefine to disable MS-extended-type-checking
//#define DOUBLE_CHECK_DATA	//define to enable output-data-check

/**
standard-RIFF-format-handler

*/


/** how to use **

RIFF-format (read this if you want to know how it works):

RIFF is a tree-like data-structure containing lists and files.
while files can contain any kind of data, lists represent a tree-node
containing internal lists and files.

example:

riff
{
	file
	list
	{
		list
		{
			file
			file
		}
		file
	}
	file
	list
	{
		file
	}
}

both lists and files are stored in so called chunks containing a type-4-char-id
and an unsigned long byte-size followed by the actual data:

chunk
{
	char		type[4];
	unsigned	size;	//bytes
	[data]
}

data-size has to be multible of 2 (0,2,4,6,...) for speed-reasons.
If a file's data-size is not a multible of 2 a file-end-byte is added.
This end-byte however is not added to the size-value located in the
chunk's head.
for example:

chunk
{
	"some"
	3
	[4-byte-data]
}


type-specifier:
allowed is any 4-char-combination not containing a "\0". Note that
names shorter than 4 chars need to be finalized using
spaces (not \0).
for example:
"Main"
or
"DATA"
or
"VTX "

lists need one of the two following specifiers (both case-sensitive):
"LIST"
or
"RIFF"

Usually the main-list is called "RIFF" but any name will be accepted.
If you create a file using the File-object "RIFF" will be used.

****** USAGE ******

1) create a new File object (you can directly open a file by supplying a file-name as parameter)
2) (if you haven't already done so) create or open a file. 
3) (...)
4) call "close()" or delete the object (file will be closed automatically)


reading information from an open RIFF-file:

after opening the file-cursor will be reset.
Otherwise use "reset()" to reset the cursor to the first element of
the RIFF main context.
Since RIFF-files are tree-like data structures, reading the file is
context-orientated.
When you opened or reset the file you will be in the main context at
the first element.
Otherwise use "First()" to return the cursor to the first element of the
current context. "First()" will return false if the present context is
empty (no list elements).
Use "next()" to advance the cursor to the next element.
"next()" will return false if there is no next element in the selected
context.
Example to seek through all elements of one context:

	if (First())
		do
		{
			(...) whatever
		}
		while (next());
		
you can also directly select an element using "select(unsigned)".
"select(...)" will return false if the given unsigned is out of range.
if you use "findFirst(id)" ... "findNext(id)" instead of
"First()" ... "next()" you can find all chunks in the present context
matching the given id (UINT32 or const char*).
For example to find all lists in the present context use:

	if (findFirst("LIST"))
		do
		{
			(...) whatever
		}
		while (findNext("LIST"));

if the present element is a list-chunk you can enter it using "enter()".
This will move the cursor into the selected chunk and redefine the context.
"enter()" will return false if the present element cannot be entered.
To return from a context to a superior context use "dropBack()".
if you already are in the top-most context "dropBack()" will return false.

Retrieving information from the selected chunk:

IsID(...)
	returns true if the selected chunk's id matches the given type-id.
	You can either check for an unsigned int or a string.
	examples:
		IsID(65273)
		or
		IsID("DATA")

getIndex()
	returns selected chunk's index inside present 
	context (0 if first element / 1 if second element ...)

getID()
	returns ID-string (const char*). The string is 0-terminated at
	result[4].

getSize()
	extract data-size of the selected chunk in bytes.

get(void*out_pntr)
	writes chunk-data to "out_pntr" and returns data-size.


*** editing an open riff-file ***

Any edit-command will return false if the file was not opened in write-
mode.


insertBlock(ID, void*data, unsigned data_size)
	inserts a block between the previous and the present chunk
	(if existing). Any data given will be written to the file.
	Works too if the present context is empty or no previous element is
	available.
	
	to insert a new list use
		insertBlock("LIST",NULL,0);
	Note that lists must be called "LIST" or "RIFF" or File-object
	will not recognize it as list but a file which cannot be entered.

appendBlock(ID, void*data, unsigned data_size)
	appends a block behind the last chunk
	(if existing). Any data given will be written to the file.
	Works too if the present context is empty or no previous element is
	available.
	
	to append a new list use
		appendBlock("LIST",NULL,0);


dropBlock()
	this will drop the selected chunk.
	Note, that all data inside this chunk will be erased (including sub-
	chunks).
	
resizeBlock()	- UNTESTED!!
	this will resize a selected block to the defined new size.
	It should work but remains untested.

*/

namespace DeltaWorks
{

	namespace Riff
	{

		#if SYSTEM==WINDOWS
			typedef wchar_t	char_t;
		#else
			typedef char char_t;
		#endif

		typedef UINT32				RIFF_INDEX_ID;
		typedef const char*			RIFF_STRING_ID;
		typedef UINT32				RIFF_ADDR;
		typedef UINT32				RIFF_SIZE;

	
		struct TID
		{
			RIFF_INDEX_ID			value;

			/**/					TID():value(0)	{}
			/**/					TID(RIFF_INDEX_ID in):value(in)	{}
			/**/					TID(RIFF_STRING_ID in);
			bool					operator==(const TID&other)	const	{return value == other.value;}
			bool					operator!=(const TID&other)	const	{return value != other.value;}
			const char*				ToString() const;
			RIFF_INDEX_ID			Numeric() const {return value;}
			void					CopyTo(char out[4])	const;
		};


		static const TID			RIFF_NAME_CHUNK("NAME"),
									RIFF_DATA_CHUNK("DATA");


		#define RIFF_CHUNK_HEAD_SIZE	(sizeof(RIFF_INDEX_ID)+sizeof(RIFF_SIZE))
		#define RIFF_LIST				0x5453494C
		#define RIFF_RIFF				0x46464952

		#ifdef DOUBLE_CHECK_DATA
		extern unsigned char check_buffer[1000000];
		#endif


		struct TRiffInfo
		{
				char			sid[4];
				RIFF_SIZE		size;
		};

		struct SRiffChunk
		{
				TRiffInfo		info;
				RIFF_ADDR		addr;
				unsigned		index;
		
				RIFF_ADDR		blockEnd();
				RIFF_ADDR		root();
		};

		class File
		{
		private:
			SRiffChunk			mark_stack[256];
			BYTE				mark_stacked;

			SRiffChunk			history[256];
			BYTE				husage;

			SRiffChunk			now;
			char				str_out[5];
			bool				streaming;
			RIFF_SIZE			streaming_at;
		
			RandomAccessFile	file;

		#ifdef	RIFF_SENSITIVE
			RIFF_INDEX_ID		sub_id;
		#endif

		protected:
			void				mark();
			void				recall();
			void				forget();

		public:
			bool				ignore_main_size_;
		
			/**/				File();
			/**/				File(const char_t*filename);
			virtual				~File();

			bool				Open(const char_t*filename);	//opens an existing archive - closes old if existing
			void				Assign(BYTE*data, ULONG size); //retrieves data from external data-field
			bool				ReOpen();
			const char*			MainID();
		#ifdef	RIFF_SENSITIVE
			const char*			SubID();
		#endif
			RIFF_SIZE			mainSize();
			bool				Create(const char_t*filename);//creates a new archive
			void				Close();					//closes open archive
			void				Reset();					//resets cursor to file-start
			bool				FindFirst(TID ID);			//select riff-chunk via ID
			bool				FindNext(TID ID);			//find next riff-block with this type-ID
			bool				Select(unsigned index);		//select riff-chunk via context-index
			bool				IsID(TID ID);				//checks wether or not selected chunk's id is equal to given one
			bool				Next();						//increase cursor
			bool				First();					//reset cursor in present context
			bool				Enter();					//enters an element if its type is "LIST" or "RIFF"
			bool				DropBack();					//returns to the governing context
			inline bool			Exit();						//identical to dropBack
			RIFF_SIZE			Get(void*out);				//extrudes data
			RIFF_SIZE			Get(void*out,size_t max);//extrudes data but not more than max - returns FULL size
			template <class C>
				count_t			Get(ArrayData<C>&out);			//extracts data into an array
			const char_t*		GetFileName();
			TID					GetID();					//returns block's type-id
			unsigned			GetIndex();					//returns block's index (in present context)
			RIFF_ADDR			GetAddr();					//returns block's address
			const SRiffChunk&	GetChunk();
			RIFF_SIZE			GetSize();					//returns size of block
			bool				IsMultipleOf(size_t size); //returns true if getSize() is multiple of size
			RIFF_ADDR			Tell();						//returns present file-cursor-position
			RIFF_SIZE			StreamTell()	const	{return streaming?streaming_at:0;};			//!< Returns the number of bytes that have been read via stream so far
			bool				CanStreamBytes(size_t bytes)	const;								//!< Checks whether or not the specified number of bytes can be streamed by the next stream call, or if no such amount is left in the remaining block space
			void				OpenStream();
			template <class C>
				bool			Stream(C&obj);
			template <class C>
				bool			Stream(C*obj, count_t num);
			bool				StreamPointer(void*target, size_t data_size);
			bool				Skip(size_t data_size);
			void				CloseStream();
		
			bool				InsertBlock(TID ID, const void*data=NULL, size_t size=0);	//insert a new block between the previous and the present block. Works too if inside an empty list-block. DATA can be NULL.
			bool				InsertBlock(TID id, const ArrayRef<const BYTE>&data) {return InsertBlock(id,data.GetPointer(),data.GetLength());}
			bool				InsertBlock(TID ID, const ByteStreamBuffer&data)	{return InsertBlock(ID,data.GetWrittenData());}
		template <typename T>
			bool				InsertBlock(TID id, const ArrayRef<T>&data);
			bool				AppendBlock(TID ID, const void*data=NULL, size_t size=0);	//append a new block behind the last block. Works too if inside an empty list-block. DATA can be NULL.
			bool				AppendBlock(TID id, const ArrayRef<const BYTE>&data) {return AppendBlock(id,data.GetPointer(),data.GetLength());}
			bool				AppendBlock(TID ID, const ByteStreamBuffer&data)	{return AppendBlock(ID,data.GetWrittenData());}
		template <typename T>
			bool				AppendBlock(TID id, const ArrayRef<T>&data);
			bool				AppendBlocks(File&other);
			bool				DropBlock();	//erase selected block (and all subblocks)
			bool				Overwrite(const void*data, size_t check_size);
			bool				ResizeBlock(size_t size);	//resizes selected block
			bool				SwapBlocks(unsigned index0, unsigned index1);
		private:
		};



		class Chunk
		{
		protected:
				RIFF_ADDR		_offset;
				BYTE			*_data;
				Chunk		*_first,*_next,*_current,*_previous,*_parent;
				unsigned		_index;
				TRiffInfo		_info;
				bool			_streaming;
		static	char			str_out[5];

		public:

								Chunk();
								Chunk(Chunk*parent);
								Chunk(Chunk*parent, TID id, const void*data, size_t size);
		virtual					~Chunk();

	
				bool			LoadFromFile(const char_t*filename);
				RIFF_SIZE		FromFile(FILE*f, size_t size, bool force_list=false);
				bool			LoadFromData(const void*data, size_t size);
				RIFF_SIZE		FromData(const BYTE*data, size_t size, bool force_list=false);
				bool			SaveToFile(const char_t*filename);
				void			ToFile(FILE*f, bool force_list=false);
				RIFF_SIZE		SaveToData(void*out);
				void			ToData(BYTE*out, bool force_list=false);
				void			Clear();
				RIFF_SIZE		ResolveSize(bool force_list=true);

				void*			SetData(size_t dataSize);
				void			SetData(const void*data, size_t dataSize);
				void			SetData(const ByteStreamBuffer&stream)	{SetData(stream.GetWrittenData());}
			template <typename T>
				void			SetPODData(const T&podData) {SetData(&podData,sizeof(podData));}
			template <typename T>
				void			SetData(const ArrayRef<T>&ar) {SetData(ar.pointer(),ar.GetContentSize());}

				RIFF_SIZE		Get(void*out)	const;				//extracts data
				RIFF_SIZE		Get(void*out,size_t max)	const;//extracts data but not more than max - returns FULL size
				const BYTE*		pointer()	const	/** @brief Retrieves the pointer to the beginning of the data of the local chunk*/	{return _data;}
			template <typename T>
				count_t			Get(ArrayData<T>&out)	const;
				Ctr::ArrayRef<const BYTE>	ReferenceData() const {return ArrayRef<const BYTE>(pointer(),size());}
				void			SetID(TID ID);
				TID				GetID() const;
				bool			IsID(TID id)	const;				//checks wether or not this chunk's id is equal to given one
				RIFF_SIZE		size()	const;						//returns this chunk's data-size
				bool			IsMultipleOf(size_t size)	const; //returns true if size_ is multiple of size
				void			OpenStream();
			template <class C>
				bool			Stream(C&obj);
			template <class C>
				bool			Stream(C*obj, count_t num);
				bool			StreamPointer(void*target, size_t data_size);
				bool			CloseStream();
				bool			Overwrite(const void*data, size_t check_size);

				bool			DataMatch(const void*compareTo, size_t compareToSize)	const;
			template <typename T>
				bool			DataMatch(const T&pod) const {return DataMatch(&pod,sizeof(T));}

				bool			FindFirstNamedList(Chunk*&outNameChunk, Chunk*&outDataChunk)
				{
					outNameChunk = FindFirst(RIFF_NAME_CHUNK);
					for (;;)
					{
						if (!outNameChunk)
							return false;
						outDataChunk = Next();
						if (!outDataChunk)
							return false;
						if (outDataChunk->IsID(RIFF_LIST))
							return true;
						outNameChunk = FindNext(RIFF_NAME_CHUNK);
					}
				}
				bool			FindNextNamedList(Chunk*&outNameChunk, Chunk*&outDataChunk)
				{
					for (;;)
					{
						outNameChunk = FindNext(RIFF_NAME_CHUNK);
						if (!outNameChunk)
							return false;
						outDataChunk = Next();
						if (!outDataChunk)
							return false;
						if (outDataChunk->IsID(RIFF_LIST))
							return true;
					}
				}

				Chunk*			FindFirstNamedListMatch(const void*namePtr, size_t nameSize)
				{
					
					Chunk*outNameChunk, *outDataChunk;
					if (FindFirstNamedList(outNameChunk,outDataChunk))
						do
						{
							if (outNameChunk->size() == nameSize && memcmp(outNameChunk->pointer(),namePtr,nameSize)==0)
								return outDataChunk;
						}
						while (FindNextNamedList(outNameChunk,outDataChunk));
					return nullptr;
				}
				Chunk*			FindNextNamedListMatch(const void*namePtr, size_t nameSize)
				{
					Chunk*outNameChunk, *outDataChunk;
					while (FindNextNamedList(outNameChunk,outDataChunk));
					{
						if (outNameChunk->size() == nameSize && memcmp(outNameChunk->pointer(),namePtr,nameSize)==0)
							return outDataChunk;
					}
					return nullptr;
				}
			template <typename T>
				Chunk*			FindFirstNamedListMatch(const T&podName)
				{
					return FindFirstNamedListMatch(&podName,sizeof(podName));
				}
			template <typename T>
				Chunk*			FindNextNamedListMatch(const T&podName)
				{
					return FindNextNamedListMatch(&podName,sizeof(podName));
				}


				Chunk*			FindFirst(TID ID);			//select riff-chunk via ID
				Chunk*			FindNext(TID ID);			//find next riff-block with this type-ID
				Chunk*			Select(unsigned index);		//select riff-chunk via context-index
				Chunk*			Next();						//increase cursor
				Chunk*			First();					//reset cursor in present context
				Chunk*			GetNextSibling()		/** Retrieves the sibling of this chunk in its parent context*/ {return _next;}
				const Chunk*	GetNextSibling() const	/** @copydoc GetNextSibling() */ {return _next;}
				Chunk*			GetFirstChild()			/** Retrieves the first child of the local chunk */ {return _first;}
				const Chunk*	GetFirstChild() const	/** @copydoc GetFirstChild() */ {return _first;}
				unsigned		CurrentIndex()	const;					//returns block's index
				Chunk*			Current();
				bool			EraseCurrent();
				bool			ResizeCurrent(size_t size);
				Chunk*			InsertBlock(TID ID, const void*data=NULL, size_t size=0);	//insert a new block between the previous and the present block. Works too if inside an empty list-block. DATA can be NULL.
				Chunk*			InsertBlock(TID ID, const ByteStreamBuffer&data)	{return InsertBlock(ID,data.GetWrittenData());}
			template<typename T>
				Chunk*			InsertBlock(TID id, const ArrayRef<T>&data);
				Chunk*			AppendBlock(TID ID, const void*data=NULL, size_t size=0);	//insert a new block between the previous and the present block. Works too if inside an empty list-block. DATA can be NULL.
				Chunk*			AppendBlock(TID ID, const ByteStreamBuffer&data)	{return AppendBlock(ID,data.GetWrittenData());}
			template<typename T>
				Chunk*			AppendBlock(TID id, const ArrayRef<T>&data);

				Chunk*			AppendNamedList(const void*namePtr, size_t nameSize)
				{
					AppendBlock(RIFF_NAME_CHUNK,namePtr,nameSize);
					return AppendBlock(RIFF_LIST);
				}

			template <typename T>
				Chunk*			AppendNamedList(const T&podIdentifier)
				{
					return AppendNamedList(&podIdentifier,sizeof(podIdentifier));
				}

				Chunk*			AppendNamedBlock(const void*namePtr, size_t nameSize)
				{
					AppendBlock(RIFF_NAME_CHUNK,namePtr,nameSize);
					return AppendBlock(RIFF_DATA_CHUNK);
				}

			template <typename T>
				Chunk*			AppendNamedBlock(const T&podIdentifier)
				{
					return AppendNamedBlock(&podIdentifier,sizeof(podIdentifier));
				}
		};


		typedef Chunk	Container;


		#include "riff_handler.tpl.h"

	}
}

#endif

