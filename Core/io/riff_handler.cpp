#include "../global_root.h"
#include "riff_handler.h"

namespace DeltaWorks
{

	namespace Riff
	{


		#undef FOPEN
		#undef STRLEN
		#if SYSTEM==WINDOWS
			#define STRLEN(_STR_)	wcslen(_STR_)
			#define FOPEN(_FILE_,_MODE_)	_wfopen((_FILE_),L##_MODE_)
		#else
			#define STRLEN(_STR_)	strlen(_STR_)
			#define FOPEN(_FILE_,_MODE_)	fopen((_FILE_),_MODE_)
		#endif

		#ifdef DOUBLE_CHECK_DATA
		unsigned char check_buffer[1000000];
		#endif
		/*
		C_String c42ansi(const char*c4)
		{
		C_String rs;
		for (unsigned char i = 0; i < 4; i++)
			rs+=c4[i];
		return rs;
		}
		*/

		//rule: now.addr is at the beginning of its data-block/children

		char			Chunk::str_out[5] = "    ";


		static bool ischar(char c)
		{
			return isupper((BYTE)c) || isdigit((BYTE)c) || c == ' ';
		}


		static RIFF_SIZE	castSize(size_t size)
		{
			if (size > 0xFFFFFFFFULL)
				FATAL__("Parameter size exceeds maximum allowed RIFF size");
			return (RIFF_SIZE)size;
		}

		RIFF_ADDR SRiffChunk::blockEnd()
		{
			return addr+info.size;
		}

		RIFF_ADDR SRiffChunk::root()
		{
			return addr-RIFF_CHUNK_HEAD_SIZE;
		}

		void File::mark()
		{
				mark_stack[mark_stacked++] = now;
		}

		void File::recall()
		{
				if (!mark_stacked)
					return;
				now = mark_stack[--mark_stacked];
		}

		void File::forget()
		{
			mark_stacked--;
		}


		const char*File::MainID()
		{
			memcpy(str_out,&history[0].info.sid,4);
			return str_out;
		}

		#ifdef RIFF_SENSITIVE
			const char*File::SubID()
			{
				memcpy(str_out,&sub_id,sizeof(RIFF_INDEX_ID));
				return str_out;
			}
		#endif

		RIFF_SIZE File::mainSize()
		{
			return history[0].info.size;
		}

		bool File::IsMultipleOf(size_t size)
		{
			return now.info.size && !(now.info.size%size);
		}

		RIFF_SIZE File::GetSize()
		{
			return now.info.size;
		}

		TID File::GetID()
		{
			return now.info.sid;
		}

		const SRiffChunk& File::GetChunk()
		{
			return now;
		}

		File::File():mark_stacked(0),husage(0),streaming(false),has_form_type_(false),ignore_main_size_(false)
		{
			str_out[4] = 0;
		}

		File::File(const char_t*filename):mark_stacked(0),husage(0),streaming(false),has_form_type_(false),ignore_main_size_(false)
		{
			str_out[4] = 0;
			file.Open(filename);
		}

		File::~File()
		{
			Close();
		}

		bool File::Open(const char_t*filename)
		{
			Close();
			if (!file.Open(filename,RandomAccessFile::Direct))
				return false;
			Reset();
			return true;
		}

		void File::Assign(BYTE*data, ULONG size)
		{
			Close();
			file.Assign(data,size);
			Reset();
		}

		bool File::ReOpen()
		{
			Close();
			if (!file.ReOpen(RandomAccessFile::Direct))
				return false;
			Reset();
			return true;
		}

		bool File::Create(const char_t*filename)
		{
			Close();
			if (!file.Create(filename,RandomAccessFile::Direct))
				return false;
			file.Append("RIFF",4);
			file.Append((BYTE)0,4);
			Reset();
			return true;
		}


		void File::Close()
		{
			if (streaming)
				FATAL__("closing in stream");
			file.Close();
		}

		void File::Reset()
		{
			streaming = false;
			if (!file.IsActive())
				return;

			file.Read(0,now.info);
			if (ignore_main_size_)
				now.info.size = file.GetSize()-RIFF_CHUNK_HEAD_SIZE;
			now.addr = RIFF_CHUNK_HEAD_SIZE;
			now.index = 0;
			has_form_type_ = false;
			#ifdef  RIFF_SENSITIVE
				if (TID(now.info.sid) == RIFF_RIFF && now.info.size >= 8)
				{
					RIFF_INDEX_ID test;
					char*ctest = (char*)&test;
					file.Read(RIFF_CHUNK_HEAD_SIZE+4,test);
					if (ischar(ctest[0]) && ischar(ctest[1]) && ischar(ctest[2]) && ischar(ctest[3]))
					{
						file.Read(RIFF_CHUNK_HEAD_SIZE,sub_id);
						now.info.size-=4;
						now.addr+=4;
						has_form_type_ = true;
					}
					else
						sub_id = RIFF_RIFF;
				}
			#endif
			history[0] = now;
			husage = 0;
			mark_stacked = 0;
			Enter();
		}

		void File::WriteContextHeader(BYTE i)
		{
			if (has_form_type_)
			{
				//contexts start behind their form or list type, so their header is 4 bytes further back than root() says, and their size counts the type
				TRiffInfo info = history[i].info;
				info.size += 4;
				file.Overwrite(history[i].root()-4,info);
			}
			else
				file.Overwrite(history[i].root(),history[i].info);
		}

		bool File::FindFirst(TID ID)
		{
			mark();
			if (First())
				do
					if (IsID(ID))
					{
						forget();
						return true;
					}
				while (Next());
			recall();
			return false;
		}



		bool File::FindNext(TID ID)
		{
			mark();
			while (Next())
				if (IsID(ID))
				{
					forget();
					return true;
				}
			recall();
			return false;
		}

		bool File::Select(unsigned index)
		{
			if (index < now.index)
				First();
			while (now.index != index && Next());
			return now.index == index;
		}

		unsigned File::GetIndex()
		{
			return now.index;
		}


		bool File::IsID(TID ID)
		{
			return ID == now.info.sid;
		}


		RIFF_ADDR File::Tell()
		{
			return now.addr+streaming?streaming_at:0;
		}


		bool File::Next()
		{
			RIFF_SIZE size = now.info.size+now.info.size%2;
			if (now.addr+size+RIFF_CHUNK_HEAD_SIZE > history[husage-1].blockEnd())
				return false;
			now.addr+=size+RIFF_CHUNK_HEAD_SIZE;
			file.Read(now.addr-RIFF_CHUNK_HEAD_SIZE,now.info);
			now.index++;
			return true;
		}

		bool File::First()
		{
			if (streaming)
				FATAL__("seeking first while streaming");
			if (!history[husage-1].info.size || !file.IsActive())
				return false;
			file.Read(history[husage-1].addr,now.info);
			now.addr = history[husage-1].addr+RIFF_CHUNK_HEAD_SIZE;
			now.index = 0;
			return true;
		}

		bool File::Enter()
		{
			if (streaming)
				FATAL__("trying to enter while streaming");
			if ((!IsID("LIST") && !IsID("RIFF") && husage)) return false;
			if (husage && has_form_type_)
			{
				//skip the list type. The root context was already moved behind the form type by Reset()
				if (now.info.size < 4)
					return false;
				now.addr += 4;
				now.info.size -= 4;
			}
			history[husage++] = now;
			file.Read(now.addr,now.info);
			now.addr+=RIFF_CHUNK_HEAD_SIZE;
			now.index = 0;
			return true;
		}

		bool File::DropBack()
		{
			if (streaming)
				FATAL__("trying to drop back while streaming");
			if (husage <= 1)
				return false;
			now = history[--husage];
		//    ShowMessage("dropping back "+IntToStr(now.addr));
			return true;
		}

		RIFF_SIZE File::Get(void*out)
		{
			if (streaming)
				FATAL__("get does not work during streaming-operation");
			file.Extract(now.addr,out,now.info.size);
			return now.info.size;
		}

		RIFF_SIZE File::Get(void*out, size_t max)
		{
			if (streaming)
				FATAL__("get does not work during streaming-operation");
			RIFF_SIZE reading = now.info.size<max?now.info.size:castSize(max);
			file.Extract(now.addr,out,reading);
			return now.info.size;
		}

		RIFF_ADDR File::GetAddr()
		{
			return now.addr;
		}

		const char_t* File::GetFileName()
		{
			return file.GetFilename();
		}


		void File::OpenStream()
		{
			if (streaming)
				FATAL__("already streaming");
			streaming = true;
			streaming_at = 0;
			mark();
		}

		void File::CloseStream()
		{
			if (!streaming)
				FATAL__("trying to close non-existing stream");
			streaming = false;
			recall();
		}

		bool File::Skip(size_t data_size)
		{
			if (!streaming)
				FATAL__("no stream open");
			if (streaming_at+data_size>now.info.size)
				return false;
			streaming_at+=castSize(data_size);
			return true;
		}

		bool	File::CanStreamBytes(size_t bytes)	const
		{
			if (!streaming)
				return false;
			return ((size_t)streaming_at+bytes <= (size_t)now.info.size);
		}

		bool File::StreamPointer(void*target, size_t data_size)
		{
			if (!streaming)
				FATAL__("no stream open");
			if ((size_t)streaming_at+data_size > (size_t)now.info.size)
				return false;
			file.Extract(now.addr+streaming_at,target,castSize(data_size));
			streaming_at+=castSize(data_size);
			return true;
		}

		bool File::AppendBlock(TID id, const void*data, size_t size)
		{
			const UINT32 defaultListType = RIFF_DEFAULT_LIST_TYPE;
			if (has_form_type_ && id == RIFF_LIST && !size)
			{
				//lists in files with a form type need a list type
				data = &defaultListType;
				size = 4;
			}
		//    bool catch_newone = !history[husage-1].info.size;
			RIFF_SIZE isize = RIFF_CHUNK_HEAD_SIZE+castSize(size+size%2);
			BYTE*insert = alloc<BYTE>(isize);
			char c[4];
			id.CopyTo(c);
			memcpy(insert,c,4);
			const RIFF_SIZE size32 = castSize(size);
			memcpy(&insert[4],&size32,sizeof(size32));
			if (data && size)
				memcpy(&insert[RIFF_CHUNK_HEAD_SIZE],data,size);
			if (size%2)
				insert[RIFF_CHUNK_HEAD_SIZE+size] = 0;	//pad byte
			RIFF_ADDR offset = history[husage-1].addr+history[husage-1].info.size+history[husage-1].info.size%2;
			memcpy(str_out,c,4);
		//    ShowMessage(C_String(str_out)+" "+IntToStr(offset));
			if (!file.Insert(offset,insert,isize))
			{
				dealloc(insert);
				return false;
			}
			for (BYTE i = 0; i < husage; i++)
			{
		//        ShowMessage("adding: "+IntToStr(isize)+" to "+IntToStr(i));
				history[i].info.size+=isize;
				WriteContextHeader(i);
			}
		/*    if (catch_newone)
			{*/
				id.CopyTo(now.info.sid);
				now.info.size = castSize(size);
				now.addr = offset+RIFF_CHUNK_HEAD_SIZE;
				now.index = UNSIGNED_UNDEF;
		//    }
			dealloc(insert);
    
			return true;
		}

		bool File::InsertBlock(TID id, const void*data, size_t size)
		{
			const UINT32 defaultListType = RIFF_DEFAULT_LIST_TYPE;
			if (has_form_type_ && id == RIFF_LIST && !size)
			{
				//lists in files with a form type need a list type
				data = &defaultListType;
				size = 4;
			}
			if (!history[husage-1].info.size)
				return AppendBlock(id,data,size);
			RIFF_SIZE isize = RIFF_CHUNK_HEAD_SIZE+castSize(size+size%2);
			BYTE*insert = alloc<BYTE>(isize);
			char c[4];
			id.CopyTo(c);
			memcpy(insert,c,4);
			const RIFF_SIZE size32 = castSize(size);
			memcpy(&insert[4],&size32,sizeof(size32));
			if (data && size)
				memcpy(&insert[RIFF_CHUNK_HEAD_SIZE],data,size);
			if (size%2)
				insert[RIFF_CHUNK_HEAD_SIZE+size] = 0;	//pad byte
			if (!file.Insert(now.addr-RIFF_CHUNK_HEAD_SIZE,insert,isize))
			{
				dealloc(insert);
				return false;
			}
			id.CopyTo(now.info.sid);
			now.info.size = castSize(size);
			for (BYTE i = 0; i < husage; i++)
			{
				history[i].info.size+=isize;
				WriteContextHeader(i);
			}
			dealloc(insert);
			return true;
		}

		bool File::DropBlock()
		{
			if (!history[husage-1].info.size || !file.IsActive() || streaming)
				return false;
			RIFF_SIZE total_size = now.info.size+now.info.size%2+RIFF_CHUNK_HEAD_SIZE;
			if (!file.Erase(now.addr-RIFF_CHUNK_HEAD_SIZE,total_size))
				return false;
			for (BYTE i = 0; i < husage; i++)
			{
				history[i].info.size-=total_size;
				WriteContextHeader(i);
			}
			First();    
			return true;
		}

		bool File::Overwrite(const void*data, size_t check_size)
		{
			if (size_t(now.info.size) != check_size || !file.IsActive() || streaming)
				return false;
			return file.Overwrite(now.addr,data,castSize(check_size));
		}

		bool File::ResizeBlock(size_t size)
		{
			bool success;
			RIFF_ADDR end = now.addr+now.info.size+now.info.size%2;
			int difference = castSize((size+size%2)-(now.info.size+now.info.size%2));
			if (difference > 0)
				success = file.Insert(end,(BYTE)0,difference);
			else
				if (difference < 0)
					success = file.Erase(end+difference,-difference);
				else
					return true;
			if (!success)
				return false;
			now.info.size = castSize(size);
			file.Overwrite(now.root(),now.info);
			for (BYTE i = 0; i < husage; i++)
			{
				history[i].info.size+=difference;
				WriteContextHeader(i);
			}
			return true;
		}

		bool File::SwapBlocks(unsigned index0, unsigned index1)
		{
			if (index0 > index1)
			{
				unsigned save = index0;
				index0 = index1;
				index1 = save;
			}
			mark();
			if (!Select(index0))
			{
				recall();
				return false;
			}
			SRiffChunk first = now;
			BYTE*buffer = file.Extract(now.addr-RIFF_CHUNK_HEAD_SIZE,now.info.size+RIFF_CHUNK_HEAD_SIZE);
			if (!Select(index1))
			{
				recall();
				dealloc(buffer);
				return false;
			}
			SRiffChunk second = now;
			BYTE*second_buffer = file.Extract(now.addr-RIFF_CHUNK_HEAD_SIZE,now.info.size+RIFF_CHUNK_HEAD_SIZE);
			ResizeBlock(first.info.size);
			file.Overwrite(second.addr-RIFF_CHUNK_HEAD_SIZE,buffer,first.info.size+RIFF_CHUNK_HEAD_SIZE);
			Select(index0);
			ResizeBlock(second.info.size);
			file.Overwrite(first.addr-RIFF_CHUNK_HEAD_SIZE,second_buffer,second.info.size+RIFF_CHUNK_HEAD_SIZE);
			recall();
			dealloc(buffer);
			dealloc(second_buffer);
			return true;
		}


		bool File::AppendBlocks(File&other)
		{
			mark();
			other.mark();
			if (other.First())
				do
				{
					RIFF_SIZE size = other.GetSize();
					char*buffer = alloc<char>(size);
					other.Get(buffer);
					if (!AppendBlock(other.now.info.sid,buffer,size))
					{
						dealloc(buffer);
						recall();
						other.recall();
						return false;
					}
					dealloc(buffer);
				}
				while (other.Next());
			other.recall();
			recall();
			return true;
		}


		//------------------------------------------------------------------------------

		Chunk::Chunk():_data(NULL),_first(NULL),_next(NULL),_current(NULL),_previous(NULL),_parent(NULL),_index(0),_streaming(false)
		{
			TID(RIFF_RIFF).CopyTo(_info.sid);
			_info.size = 0;
		}

		Chunk::Chunk(Chunk*parent):_data(NULL),_first(NULL),_next(NULL),_current(NULL),_previous(NULL),_parent(parent),_index(0),_streaming(false)
		{
			TID(RIFF_RIFF).CopyTo(_info.sid);
			_info.size = 0;
		}

		Chunk::Chunk(Chunk*parent, TID id, const void*data, size_t size):_data(NULL),_first(NULL),_next(NULL),_current(NULL),_previous(NULL),_parent(parent),_index(0),_streaming(false)
		{
			id.CopyTo(_info.sid);
			BYTE*target = (BYTE*)SetData(size);
			if (data && size)
				memcpy(target,data,size);
		}

		void Chunk::SetData(const void*data, size_t dataSize)
		{
			memcpy(SetData(dataSize),data,dataSize);
		}
		void* Chunk::SetData(size_t dataSize)
		{
			dealloc(_data);
			RIFF_SIZE i_size = castSize(dataSize + dataSize%2);
			alloc(_data,i_size);
			_info.size = castSize(dataSize);
			if (i_size != _info.size)
				_data[_info.size] = 0;	//pad byte

			return _data;

		}


		Chunk::~Chunk()
		{
			dealloc(_data);
			if (_first)
				Discard(_first);
			if (_next)
				Discard(_next);
		}

		/**
		Checks whether the data of a root RIFF chunk starts with a form type.
		DeltaWorks used to write no form type, so the first child chunk may start where the form type would be.
		A form type is assumed if the four bytes after it (the first child's id, or the first child's size if there is no form type) look like a chunk id.
		@param id Id of the root chunk
		@param size Data size of the root chunk
		@param bytes8to15 The first 8 data bytes of the root chunk
		*/
		static bool DetectFormType(TID id, RIFF_SIZE size, const BYTE bytes8to15[8])
		{
			#ifdef RIFF_SENSITIVE
				if (id != RIFF_RIFF || size < 8)
					return false;
				const char*next = (const char*)bytes8to15 + 4;
				return ischar(next[0]) && ischar(next[1]) && ischar(next[2]) && ischar(next[3]);
			#else
				return false;
			#endif
		}

    
		bool Chunk::LoadFromFile(const char_t*filename)
		{
			Clear();
			FILE*f = FOPEN(filename,"rb");
			if (!f)
				return false;
			Clear();
			fseek(f,0,SEEK_END);
			RIFF_SIZE max_len = ftell(f);
			fseek(f,0,SEEK_SET); 
			FromFile(f,max_len,true);
			fclose(f);
			return true;
		}

		RIFF_SIZE Chunk::FromFile(FILE*f, size_t size, bool force_list, bool typed)
		{
			Clear();
			_type = TID();
			if (size < sizeof(TRiffInfo))
				return 0;
			if (!fread(&_info,sizeof(_info),1,f))
				return 0;
			if (size-sizeof(TRiffInfo) < _info.size)
			{
				throw Except::IO::DriveAccess::FileFormatFault("RIFF size violation");
				return sizeof(TRiffInfo);
			}
			RIFF_SIZE local = _info.size;
			_previous = NULL;
			_index = 0;
			_current = NULL;
			const bool isList = TID(_info.sid) == RIFF_LIST || force_list;
			if (force_list)
			{
				typed = false;
				if (local >= 8)
				{
					BYTE probe[8];
					if (fread(probe,1,8,f) != 8)
						return 0;
					fseek(f,-8,SEEK_CUR);
					typed = DetectFormType(TID(_info.sid),local,probe);
				}
			}
			if (isList && typed && local >= 4)
			{
				if (!fread(&_type.value,4,1,f))
					return 0;
				local -= 4;
			}
			if (isList)
				while (local >= sizeof(TRiffInfo))
				{
					if (!_current)
					{
						_current = SignalNew(new Chunk(this));
						_first = _current;
					}
					else
					{
						_current->_next = SignalNew(new Chunk(this));
						_current = _current->_next;
					}
					RIFF_SIZE read = _current->FromFile(f,local,false,typed);
					if (!read)
						break;
					local -= read < local ? read : local;
				}
			else
			{
				RIFF_SIZE i_size = local+local%2;
				re_alloc(_data,i_size);
				if (fread(_data,1,local,f)!=local)
					return 0;
				if (i_size != local)
				{
					BYTE pad;
					fread(&pad,1,1,f);	//the pad byte may be missing at the very end of the file
					_data[local] = 0;	//written as zero, whatever the source contained
				}
			}
			_current = NULL;
			return _info.size+_info.size%2+sizeof(TRiffInfo);
		}

		bool Chunk::LoadFromData(const void*data, size_t size)
		{
			Clear();
			return FromData((const BYTE*)data,size, true) > 0;
		}

		RIFF_SIZE Chunk::FromData(const BYTE*data, size_t size, bool force_list, bool typed)
		{
			Clear();
			_type = TID();
			if (size < sizeof(TRiffInfo))
				return 0;
			memcpy(&_info,data,sizeof(_info));
			if (size-sizeof(TRiffInfo) < _info.size)
				return sizeof(TRiffInfo);
			RIFF_SIZE local = _info.size;
			_previous = NULL;
			_index = 0;
			_current = NULL;
			RIFF_ADDR offset = sizeof(TRiffInfo);
			const RIFF_ADDR end = offset + local;
			const bool isList = TID(_info.sid) == RIFF_LIST || force_list;
			if (force_list)
				typed = local >= 8 && DetectFormType(TID(_info.sid),local,data + offset);
			if (isList && typed && local >= 4)
			{
				memcpy(&_type.value,data + offset,4);
				offset += 4;
			}
			if (isList)
				while (offset < end && end - offset >= sizeof(TRiffInfo))
				{
					if (!_current)
					{
						_current = SignalNew(new Chunk(this));
						_first = _current;
					}
					else
					{
						_current->_next = SignalNew(new Chunk(this));
						_current = _current->_next;
					}
					RIFF_SIZE read = _current->FromData(&data[offset],end-offset,false,typed);
					if (!read)
						break;
					offset+=read;
				}
			else
			{
				RIFF_SIZE i_size = local+local%2;
				re_alloc(_data,i_size);
				memcpy(_data,&data[offset],local);
				if (i_size != local)	//the pad byte may be missing at the very end of the data, and is written as zero
					_data[local] = 0;
			}
			_current = NULL;
			return _info.size+_info.size%2+sizeof(TRiffInfo);
		}


		bool Chunk::SaveToFile(const char_t*filename)
		{
			FILE*f = FOPEN(filename,"wb");
			if (!f)
				return false;
			ResolveSize(true);
			ToFile(f, true);
			fclose(f);
			return true;
		}

		TID Chunk::WrittenType() const
		{
			return _type.value != 0 ? _type : TID(RIFF_DEFAULT_LIST_TYPE);
		}

		void Chunk::ToFile(FILE*f, bool force_list, bool typed)
		{
			if (force_list)
				typed = HasFormType();
			if (!fwrite(&_info,sizeof(_info),1,f))
				return;
			if (TID(_info.sid) == RIFF_LIST || force_list)
			{
				if (typed)
				{
					const TID type = WrittenType();
					fwrite(&type.value,4,1,f);
				}
				Chunk*chunk = _first;
				while (chunk)
				{
					chunk->ToFile(f,false,typed);
					chunk = chunk->_next;
				}
			}
			else
				ASSERT__(fwrite(_data,1,_info.size+_info.size%2,f)==_info.size+_info.size%2);
		}

		RIFF_SIZE Chunk::SaveToData(void*out)
		{
			ResolveSize(true);
			ToData((BYTE*)out,true);
			return sizeof(_info)+_info.size+_info.size%2;
		}

		void Chunk::ToData(BYTE*out,bool force_list, bool typed)
		{
			if (force_list)
				typed = HasFormType();
			memcpy(out,&_info,sizeof(_info));
			RIFF_ADDR offset = sizeof(TRiffInfo);
			if (TID(_info.sid) == RIFF_LIST || force_list)
			{
				if (typed)
				{
					const TID type = WrittenType();
					memcpy(&out[offset],&type.value,4);
					offset += 4;
				}
				Chunk*chunk = _first;
				while (chunk)
				{
					chunk->ToData(&out[offset],false,typed);
					offset+=chunk->_info.size+chunk->_info.size%2+sizeof(TRiffInfo);
					chunk = chunk->_next;
				}
			}
			else
				memcpy(&out[offset],_data,_info.size + _info.size%2);
		}

		RIFF_SIZE Chunk::ResolveSize(bool force_list, bool typed)
		{
			if (force_list)
				typed = HasFormType();
			if (TID(_info.sid) == RIFF_LIST || force_list)
			{
				_info.size = typed ? 4 : 0;
				Chunk*chunk = _first;
				while (chunk)
				{
					_info.size += chunk->ResolveSize(false,typed);
					chunk = chunk->_next;
				}
				return sizeof(_info) + _info.size;
			}
			else
				return sizeof(_info) + _info.size + _info.size%2;
		}


		void Chunk::Clear()
		{
			if (_first)
				Discard(_first);
			_first = nullptr;
			_current = nullptr;
			_previous = nullptr;
			_index = 0;
			dealloc(_data);
			_data = nullptr;
			_info.size = 0;
		}

		Chunk*Chunk::FindFirst(TID id)
		{
			if (Chunk*chunk = First())
				do
				{
					if (TID(chunk->_info.sid) == id)
						return chunk;
				}
				while (chunk = Next());
			return nullptr;
		}


		Chunk*Chunk::FindNext(TID ID)
		{
			while (Chunk*chunk = Next())
				if (TID(chunk->_info.sid) == ID)
					return chunk;
			return nullptr;
		}


		Chunk*Chunk::Select(unsigned index)
		{
			if (index < _index)
				First();
			while (_index < index && Next());

			if (_index == index)
				return _current;
			return nullptr;
		}

		Chunk*Chunk::First()
		{
			_current = _first;
			_previous = nullptr;
			_index = 0;
			return _current;    
		}

		Chunk*Chunk::Next()
		{
			if (!_current)
				return nullptr;
			_previous = _current;
			_current = _current->_next;
			_index++;
			return _current;    
		}

		void Chunk::SetID(TID id)
		{
			id.CopyTo(_info.sid);
		}


		TID Chunk::GetID()	const
		{
			return _info.sid;
		}


		TID::TID(RIFF_STRING_ID sid):value(0)
		{
			if (!sid)
				return;
			for (int i = 0; i < 4; i++)
			{
				if (!sid[i])
					return;
				value |= ((UINT32)(BYTE)sid[i])<<(i*8);
			}
		}

		const char* TID::ToString() const
		{
			static char field[5] = "    ";
			CopyTo(field);
			return field;
		}

		void		TID::CopyTo(char field[4])	const
		{
			for (int i = 0; i < 4; i++)
			{
				field[i] = (char)(((value)>>(i*8)) & 0xFF);
			}
		}




		RIFF_SIZE Chunk::Get(void*out)	const
		{
			memcpy(out,_data,_info.size);
			return _info.size;
		}

		RIFF_SIZE Chunk::Get(void*out,size_t max)	const
		{
			RIFF_SIZE size = _info.size;
			if (size > max)
				size = castSize(max);
			memcpy(out,_data,size);
			return size;        
		}

		unsigned Chunk::CurrentIndex()	const
		{
			return _index;
		}

		Chunk*Chunk::Current()
		{
			return _current;
		}


		bool Chunk::IsID(TID id) const
		{
			return id == _info.sid;
		}


		RIFF_SIZE Chunk::size() const
		{
			return _info.size;
		}

		bool Chunk::IsMultipleOf(size_t size) const
		{
			return _info.size && !(_info.size%size);
		}

		void Chunk::OpenStream()
		{
			_offset = 0;
			_streaming = true;
		}

		bool Chunk::StreamPointer(void*target, size_t data_size)
		{
			if (data_size+_offset > _info.size)
				return false;
			memcpy(target,&_data[_offset],data_size);
			_offset += castSize(data_size);
			return true;
		}

		bool Chunk::CloseStream()
		{
			_streaming = false;
			return _offset == _info.size;
		}
        
		bool Chunk::EraseCurrent()
		{
			if (!_current)
				return false;
			if (_previous)
				_previous->_next = _current->_next;
			else
				_first = _current->_next;
			_current->_next = nullptr;
			Discard(_current);
			_current = _previous->_next;
			return true;
		}

		bool Chunk::ResizeCurrent(size_t size)
		{
			if (!_current)
				return false;
			if (size == _current->_info.size)
				return true;
			BYTE*temp = alloc<BYTE>(size+size%2);
			RIFF_SIZE move = castSize(size);
			if (move > _current->_info.size)
				move = _current->_info.size;
			memcpy(temp,_current->_data,move);
			dealloc(_current->_data);
			_current->_data = temp;
			_current->_info.size = castSize(size);
			return true;
		}

		Chunk*Chunk::InsertBlock(TID ID, const void*data, size_t size)
		{
			if (!_current)
				return AppendBlock(ID,data,size);
			Chunk*chunk = SignalNew(new Chunk(this,ID,data,size));
			if (!_previous)
			{
				chunk->_next = _first;
				_first = chunk;
			}
			else
			{
				chunk->_next = _previous->_next;
				_previous->_next = chunk;
			}
			_current = chunk;
			return chunk;
		}

		Chunk*Chunk::AppendBlock(TID ID, const void*data, size_t size)
		{
			if (!_current)
				First();
			while (Next());
			Chunk*chunk = SignalNew(new Chunk(this,ID,data,size));
			if (_previous)
				_previous->_next = chunk;
			else
				_first = chunk;
			_current = chunk;
			return chunk;
		}

		bool Chunk::Overwrite(const void*data, size_t check_size)
		{
			if (_info.size != check_size)
				return false;
			memcpy(_data,data,check_size);
			return true;
		}


		bool Chunk::DataMatch(const void*compareTo, size_t compareToSize)	const
		{
			if (compareToSize != _info.size)
				return false;
			return !memcmp(_data,compareTo,_info.size);
		}

	}
}
