#include "../global_root.h"
#include "file_system.h"
#include "../string/encoding.h"
#include <algorithm>
#include "../container/buffer.h"

/******************************************************************

Plattform-independent file-browsing-module.

******************************************************************/


namespace DeltaWorks
{


	namespace FileSystem
	{
		#if SYSTEM==WINDOWS
			#define ABS_MARKER	L"\\\\?\\"
		#else
			#define ABS_MARKER "/"
		#endif
		#define ABS_MARKER_LENGTH	(ARRAYSIZE(ABS_MARKER)-1)

		static	PathString Escape(const PathString&in)
		{
			PathString rs = in;
			for (index_t i = 0; i < rs.GetLength(); i++)
				if (rs.GetChar(i) == '\\' || rs.GetChar(i) == '\'' || rs.GetChar(i) == '\"')
				{
					rs.Insert(i,'\\');
					i++;
				}
			return rs;
		}


		template <typename T>
			bool IsSlash(T chr)
			{
				return chr == (T)'/' || chr == (T)'\\';
			}

		template <typename T>
			bool IsDot(T chr)
			{
				return chr == (T)'.';
			}

		template <typename T>
			bool CastEquals(const StringType::Sequence<T>&s0, const char*s1)
			{
				for (index_t i = 0; i < s0.GetLength(); i++)
				{
					T chr0 = s0.GetChar(i);
					char chr1 = s1[i];
					if (!chr1)
						return false;
					if (chr0 != (T)chr1)
						return false;
				}
				return true;
			}

		template <typename T>
			bool CastEquals(const StringType::Sequence<T>&s0, char c1)
			{
				return s0.GetLength() == 1 && s0.GetChar(0) == (T)c1;
			}


		Ctr::Buffer0<std::pair<PathString,String> > prefixRules;

		void			SetPathPrefix(const PathString&prefix, const String&replaceWithCaption)
		{
			prefixRules << std::make_pair(prefix,replaceWithCaption);
		}

		void			ClearPathPrefixes()
		{
			prefixRules.Clear();
		}

		static bool		IsAbsPathWin(const PathString&path)
		{
			 return (isalpha(path.FirstChar()) && path.GetChar(1) == ':');
		}

		PathString		StringToPath(const StringType::Sequence<char32_t>&utf32Path)
		{
			#if SYSTEM==WINDOWS
				PathString rs;
				StringEncoding::UTF32::ToUTF16(utf32Path,rs);
				if (IsAbsPathWin(rs))
					rs = ABS_MARKER + rs;

				rs.FindAndReplace('/',FOLDER_SLASH);

				return rs;
			#else
				PathString rs;
				StringEncoding::UTF32::ToUTF8(utf32Path,rs);
				rs.FindAndReplace('\\',FOLDER_SLASH);
				return utf8Path;
			#endif
		}

		PathString		StringToPath(const UTF32String&utf32Path)
		{
			return StringToPath(utf32Path.ToRef());
		}



		PathString		StringToPath(const StringType::Sequence<char>&utf8Path)
		{
			#if SYSTEM==WINDOWS
				PathString rs;
				StringEncoding::UTF8::ToUTF16(utf8Path,rs);
				if (IsAbsPathWin(rs))
					rs = ABS_MARKER + rs;

				rs.FindAndReplace('/',FOLDER_SLASH);

				return rs;
			#else
				PathString rs = utf8Path;
				rs.FindAndReplace('\\',FOLDER_SLASH);
				return utf8Path;
			#endif
		}

		String			PathToString(const PathString::SequenceType&path, bool replaceDetectedPrefixes/*=true*/)
		{
			String*caption = nullptr;
			auto ref = path;
			String rs;
			if (replaceDetectedPrefixes)
			{
				foreach (prefixRules,r)
					if (ref.BeginsWith(r->first))
					{
						caption = &r->second;
						ref = ref.SubStringRef(r->first.GetLength());
						break;
					}
			}
			#if SYSTEM==WINDOWS
				if (ref.BeginsWith(ABS_MARKER))
					ref = ref.SubStringRef(4);
				StringEncoding::UTF16::ToUTF8(ref,rs);
			#else
				rs = ref;
			#endif
			if (caption)
				rs = "["+*caption+"]"+rs;
			return rs;
		}


		Sys::Mutex	TempFile::mutex;

		File::File()
		{}

		File::File(const Drive&drive):name(drive.root),location(drive.root)
		{}

		PathString	 File::GetName()  const
		{
			return name;
		}

		PathString	 File::GetInnerName()  const
		{
			return ExtractFileName(name);
		}
	
		const PathString&	 File::GetLocation() const
		{
			return location;
		}


		bool		File::IsDirectory() const
		{
			return FileSystem::IsDirectory(location);
		}


		PathString	 File::GetFolder()		 const
		{
			return ExtractFileDir(location);
		}

		const PathString::char_t* File::GetExtensionPointer()	const
		{
			index_t at = name.GetLength();
			const PathString::char_t*field = name.c_str();
			while (at!=InvalidIndex && !IsDot(field[at]))
				at--;
			if (at == InvalidIndex)
				return field+name.GetLength();
			return field+at+1;
		}
	
		PathString  File::GetExtension()	  const
		{
			return GetExtensionPointer();
		}
	
		bool	File::IsExtension(const PathString::ReferenceType&ext)		const
		{
			if (ext.BeginsWith('.'))
				return IsExtension(ext.SubStringRef(1));
			return ext.EqualsIgnoreCase(GetExtensionPointer());
		}

		#ifdef WIN32
		bool	File::IsExtension(const char*ext)		const
		{
			if (ext[0] == '.')
				return IsExtension(ext+1);
			return StringFunctions::strcmpi(ext,GetExtensionPointer()) == 0;
		}
		#endif
	


		bool	File::DoesExist()							const
		{
			return FileSystem::DoesExist(location);
		}


		ftime_t				File::GetModificationTime()				  const
		{
			return FileSystem::GetModificationTime(location);
		}

	

		bool		  File::operator<(const File&other)  const
		{
			return location < other.location;
		}

		bool		  File::operator>(const File&other)  const
		{
			return location > other.location;
		}

		bool		  File::operator<=(const File&other)  const
		{
			return location <= other.location;
		}

		bool		  File::operator>=(const File&other)  const
		{
			return location >= other.location;
		}

		bool		  File::operator==(const File&other)  const
		{
			return location == other.location;
		}

		bool		  File::operator!=(const File&other)  const
		{
			return location != other.location;
		}


		bool	 File::operator<(const PathString&filename)  const
		{
			return location < filename;
		}

		bool	 File::operator>(const PathString&filename)  const
		{
			return location > filename;
		}

		bool	 File::operator<=(const PathString&filename)  const
		{
			return location <= filename;
		}

		bool	 File::operator>=(const PathString&filename)  const
		{
			return location >= filename;
		}

		bool	 File::operator==(const PathString&filename)  const
		{
			return location == filename;
		}

		bool	 File::operator!=(const PathString&filename)  const
		{
 			return location != filename;
		}

		static File  fs_file_sibling;

		const File*	  File::GetSibling(const PathString&ext)  const
		{
			PathString sibling_location = ExtractFileName(location)+'.'+ext;
			if (!FileSystem::DoesExist(sibling_location))
				return NULL;
			fs_file_sibling = *this;
			fs_file_sibling.name = ExtractFileName(name)+'.'+ext;
			fs_file_sibling.location = sibling_location;
			return &fs_file_sibling;
		}




		String ToString(const File&f)
		{
			if (f.IsDirectory())
				return "Folder: "+String(f.location);
			if (f.DoesExist())
				return "File: "+String(f.location);
			return "Non-existent: "+String(f.location);
		}

		bool File::TryToUnlink() const
		{
			if (!this)
				return false;
			if (IsDirectory())
				return TryToRemoveFolder(location);
			return TryToUnlinkFile(location);
		}

		void File::Unlink() const
		{
			if (IsDirectory())
				RemoveFolder(location);
			else
				UnlinkFile(location);
		}





		TempFile::TempFile(bool docreate):created(docreate&&Create())
		{}


		TempFile::~TempFile()
		{
			if (created)
			{
				TryToUnlinkFile(filename);
			}

		}


		bool				TempFile::Create()
		{
			Sys::MutexLock	lock(mutex);
			PathString tmp_folder = FileSystem::StringToPath(Sys::getTempFolder());
			#if SYSTEM==WINDOWS

				static const size_t buffer_size=512;

				PathString::char_t	szTempName[buffer_size];
		/*		UINT	uRetVal;
				BOOL fSuccess;*/


			// Create a temporary file.
				if (!GetTempFileNameW(tmp_folder.c_str(), // directory for tmp files
									  L"generic",		// temp file name prefix
									  0,			// create unique name
									  szTempName))  // buffer for name
					return false;


				filename = szTempName;
				FILE*tmp = _wfopen(filename.c_str(),L"wb");
			#elif SYSTEM==LINUX
				static unsigned subsequent_call;
				srand(time(NULL)+(subsequent_call++));
				unsigned loops(0);
				do
				{
					filename = tmp_folder+"/generic."+IntToHex(rand(),4);
				}
				while (IsFile(filename) && loops++ < 512);
				if (IsFile(filename))
					return false;
				//filename = mktemp("/tmp/generic.XXXXXX");
				//filename = mkstemp("/tmp/generic.XXXXXX");
				FILE*tmp = fopen(filename.c_str(),"wb");
			#else
				#error not supported
			#endif

			if (!tmp)
				return false;
			fclose(tmp);
			created = true;

			#if SYSTEM==WINDOWS
				SetFileAttributesW(filename.c_str(),FILE_ATTRIBUTE_TEMPORARY);
			#endif

		//	cout << " + "<<filename.c_str()<<endl;
			return true;
		}

		bool	TempFile::Destroy()
		{
			if (created)
			{
				Sys::MutexLock	lock(mutex);
				TryToUnlinkFile(filename);
				created = false;
				return true;
			}
			return false;
		}


		const PathString&	  TempFile::GetLocation()
		{
			if (!created && !Create())
				FATAL__("Unable to create temporary file");
			return filename;
		}

		TempFile::operator const PathString& ()
		{
			if (!created && !Create())
				FATAL__("Unable to create temporary file");
			return filename;
		}



		PathString Drive::GetName()			const
		{
			return name;
		}

		PathString Drive::GetRoot()			const
		{
			return root;
		}

		String Drive::ToString() const
		{
			return PathToString(name)+" ("+PathToString(root)+")";
		}

		File Drive::ToFile()			const
		{
			File result;
			result.location = root;
			result.name = root;
			return result;
		}



		Folder::Folder():absolute_folder(GetWorkingDirectory())
		{}

		Folder::Folder(const PathString&folder_string) : absolute_folder(GetWorkingDirectory())
		{
			ResolvePath(folder_string,nullptr,absolute_folder);
		}

		Folder::Folder(const File&file)
		{
			locate(file.location);
		}

		Folder::Folder(const File*file)
		{
			if (file)
				locate(file->location);
		}

		Folder::Folder(const Folder&folder):absolute_folder(folder.absolute_folder)
		{}


		Folder& Folder::operator=(const Folder&other)
		{
			absolute_folder = other.absolute_folder;
			return *this;
		}

		Folder& Folder::operator=(const File&file)
		{
			locate(file.location);
			return *this;
		}

		Folder& Folder::operator=(const File*file)
		{
			if (file)
				locate(file->location);
			return *this;
		}

		Folder& Folder::operator=(const PathString&folder_string)
		{
			MoveTo(folder_string);
			return *this;
		}



		FolderScan::FolderScan(const Folder&f):path(f.GetLocation())
		{}
		

		void FolderScan::CloseScan()
		{
			if (!find_handle)
				return;
			#if SYSTEM==WINDOWS
				FindClose(find_handle);
			#elif SYSTEM==LINUX
				closedir(find_handle);
			#else
				#error not supported
			#endif
			find_handle = NULL;
		}

		bool Folder::ResolvePath(const PathString&path_string, PathString*finalParent, PathString&final) const
		{
			if (!path_string.GetLength())
				return false;
			PathString local = path_string;
			bool valid_location = false;
			#if SYSTEM==WINDOWS
			
				bool isAbs = path_string.BeginsWith(ABS_MARKER);
				if (isAbs || IsAbsPathWin(path_string))
				{
					if (isAbs)
					{
						local = path_string.subString(ABS_MARKER_LENGTH+3);
						final = path_string.subString(0,ABS_MARKER_LENGTH+3);

						if (isalpha(local.FirstChar()) && local.GetChar(1) == ':')
						{
							local = local.subString(3);
							final = ABS_MARKER + local.subString(0,3);
						}
					}
					else
					{
						local = path_string.subString(3);
						final = ABS_MARKER + path_string.subString(0,3);
					}
					valid_location = true;
				}
				else
					if (path_string.FirstChar() == '\\')
					{
						local = path_string.subString(1);
						final = GetWorkingDirectory().subString(0, 3);
						valid_location = true;
					}
					else
					{
						final = absolute_folder;
						valid_location = this->IsValidLocation();
					}
			#elif SYSTEM==LINUX
				if (path_string.FirstChar() == '/')
				{
					local = path_string.subString(1);
					final = "/";
					valid_location = true;
				}
				else
				{
					final = absolute_folder;
					valid_location = this->IsValidLocation();
				}
			#else
				#error not supported
			#endif

			if (!valid_location)
			{
				final = GetWorkingDirectory();
				valid_location = true;
			}
			if (finalParent)
				*finalParent = final;
		
			while (local.IsNotEmpty())
			{
				PathString step;
				index_t	at0 = local.Find('/'),
						at1 = local.Find('\\'),
						at = std::min(at0,at1);

			
				if (at != InvalidIndex)
				{
					step = local.subString(0,at);
					local.EraseLeft(at+1);
				}
				else
				{
					step = local;
					local = PathString();
				}
				if (CastEquals(step,'.') || !step.GetLength())
					continue;
				if (CastEquals(step,".."))
				{
					if (!ExitAbs(final,false))
						return false;
					continue;
				}
				if (local.IsEmpty())
					if (finalParent)
						*finalParent = final;
				if (!IsSlash(final.LastChar()))
					final += FOLDER_SLASH;
				final += step;
			}
			//final = absolute_folder;
			return DoesExist(final);
		}

		bool				Folder::ResolvePathAndMoveTo(const PathString&path, bool moveToParent)
		{
			PathString parent,str;
			if (ResolvePath(path,&parent,str))
			{
				absolute_folder = moveToParent ? parent : str;
				bool valid_location = moveToParent || IsFolder(absolute_folder);
				if (!valid_location)
				{
					absolute_folder = parent;
					valid_location = true;
				}
				return true;
			}
			return false;
		}




		bool Folder::locate(const PathString&folder_string)
		{
			absolute_folder = folder_string;
			if (absolute_folder.GetLength() > 1 && IsSlash(absolute_folder.LastChar()))
				absolute_folder.Erase(absolute_folder.GetLength()-1);
			#if SYSTEM==WINDOWS
				if (!absolute_folder.BeginsWith(ABS_MARKER))
				{
					absolute_folder = ABS_MARKER + absolute_folder;
				}
			#endif
			return IsFolder(absolute_folder);
		}

		bool Folder::MoveTo(const PathString&folder_string)
		{
			return ResolvePathAndMoveTo(folder_string,false);
		}

		bool Folder::MoveTo(const File*file)
		{
			if (file)
				return locate(file->location);
			return false;
		}

		bool Folder::MoveTo(const File&file)
		{
			return locate(file.location);
		}

		bool Folder::Exit()
		{
			if (!IsValidLocation())
				return false;
			return ExitAbs(absolute_folder,true);
			//#if SYSTEM==LINUX
			//	if (absolute_folder == '/' || absolute_folder == '\\')
			//		return false;
			//#endif

			//PathString current = absolute_folder,
			//		dir = ExtractFileNameExt(absolute_folder),
			//		super = ExtractFileDir(absolute_folder);
			//#if SYSTEM!=LINUX
			//	if (!super.GetLength())
			//		return false;
			//#else
			//	if (!super.GetLength())
			//	{
			//		absolute_folder = FOLDER_SLASH;
			//		return true;
			//	}
			//#endif
			//if (!locate(super))
			//{
			//	absolute_folder = current;
			//	valid_location = true;
			//	return false;
			//}
			//return true;
		}

		/*static*/ bool Folder::ExitAbs(PathString&path, bool check)
		{
			if (check && !IsDirectory(path))
				return false;
			#if SYSTEM!=WINDOWS
				if (absolute_folder == '/' || absolute_folder == '\\')
					return false;
			#endif

			PathString current = path,
					dir = ExtractFileNameExt(path),
					super = ExtractFileDir(path);
			if (super.IsEmpty())
				return false;
			if (!check || IsDirectory(super))
			{
				path = super;
				return true;
			}
			return false;
		}

		bool Folder::Enter(const PathString&folder)
		{
			if (!IsValidLocation())
				return false;
		//	PathString current = absolute_folder;
				//	target;
			return ResolvePathAndMoveTo(folder,false);
	
		}

		bool Folder::Enter(const File&file)
		{
			if (!IsValidLocation())
				return false;
			PathString current = absolute_folder;
			if (!locate(file.location))
			{
				absolute_folder = current;
				return false;
			}
			return true;
		}

		bool Folder::Enter(const File*file)
		{
			return file && Enter(*file);
		}


		bool Folder::IsValidLocation() const
		{
			return IsFolder(absolute_folder);
		}

		bool FolderScan::IsValidLocation() const
		{
			return path.IsNotEmpty() && IsFolder(path);
		}

		bool		 FolderScan::Rewind()
		{
			CloseScan();
			if (!IsValidLocation())
				return false;
			#if SYSTEM==WINDOWS
				find_handle = FindFirstFileW(PathString(path+L"\\*").c_str(),&find_data);
			#elif SYSTEM==LINUX
				find_handle = opendir(path.c_str());
			#else
				#error not supported
				return false;
			#endif
			mustRewindManually = false;
			return true;
		}


		const Folder::File*  FolderScan::NextEntry()
		{
			bool dummy;
			return NextEntry(dummy);
		}

		bool  FolderScan::NextEntry(File&file)
		{
			bool dummy;
			return NextEntry(file,dummy);
		}

		const Folder::File*  FolderScan::NextEntry(bool&outIsDirectory)
		{
			if (NextEntry(file,outIsDirectory))
				return &file;
			return NULL;
		}

		bool FolderScan::NextEntry(File&file,bool&outIsDirectory)
		{
			if (!find_handle && (mustRewindManually || !Rewind()))
				return false;
			bool retry=false;
			#if SYSTEM==WINDOWS
				file.name = find_data.cFileName;
				file.location = path;
				if (IsSlash(file.location.LastChar()))
					file.location+=FOLDER_SLASH;
				file.location+= find_data.cFileName;
				outIsDirectory = (find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
				if (!FindNextFileW(find_handle,&find_data))
				{
					CloseScan();
					mustRewindManually = true;
					//we still have one valid result. do NOT return false
				}
			#elif SYSTEM==LINUX
				dirent*entry = readdir(find_handle);
				if (!entry)
				{
					CloseScan();
					mustRewindManually = true;
					return false;
				}
				file.name = entry->d_name;

				file.location = absolute_folder;
				if (file.location.LastChar() != '/' && file.location.LastChar() != '\\')
					file.location+=FOLDER_SLASH;
				file.location += entry->d_name;
				struct stat s;
				if (stat(file.location.c_str(),&s))
					retry = true;
				outIsDirectory = s.st_mode&S_IFDIR;
			#else
				#error not supported
			#endif
			if (retry || !file.name.GetLength() || IsDot(file.name.FirstChar()))
				return NextEntry(file,outIsDirectory);
			return true;
		}


		const Folder::File*	 FolderScan::NextFolder()
		{
			const File*file;
			bool wasDir;
			do
			{
				file = NextEntry(wasDir);
			}
			while (file && !wasDir);
			return file;
		}

		bool	FolderScan::NextFolder(File&target)
		{
			bool wasDir;
			while (NextEntry(target,wasDir))
				if (wasDir)
					return true;
			return false;
		}

		bool	FolderScan::NextFolder(Folder&out)
		{
			if (NextFolder())
			{
				out = file;
				return true;
			}
			return false;
		}


		const Folder::File*	 FolderScan::NextFile()
		{
			const File*file;
			bool wasDir;
			do
			{
				file = NextEntry(wasDir);
			}
			while (file && wasDir);
			return file;
		}

		bool	FolderScan::NextFile(File&target)
		{
			bool wasDir;
			while (NextEntry(target,wasDir))
				if (!wasDir)
					return true;
			return false;
		}


		const Folder::File*	 FolderScan::NextFile(const PathString&extension)
		{
//			const PathString::char_t*ext = extension.c_str();
			const File*file;
			bool wasDir;
			do
			{
				file = NextEntry(wasDir);
			}
			while (file && (wasDir || !file->IsExtension(extension)));
			//while (file && (file->is_folder || strcmpi(file->GetExtensionPointer(),ext)));
			return file;
		}

		bool	FolderScan::NextFile(const PathString&extension, File&target)
		{
			const PathString::char_t*ext = extension.c_str();
			bool wasDir;
			while (NextEntry(target,wasDir))
				if (!wasDir && target.IsExtension(extension))
					return true;
			return false;
		}

		FolderScan		Folder::Scan() const
		{
			return FolderScan(*this);
		}


		const File*			Folder::Find(const PathString&path, UINT32 findFlags)						const
		{
			bool dummy,dummy2;
			return FindEntry(path,dummy,dummy2,findFlags);
		}

		bool				Folder::Find(const PathString&path, File&out, UINT32 findFlags)			const
		{
			bool dummy,dummy2;
			return FindEntry(path,out,dummy,dummy2,findFlags);
		}


		const Folder::File*	 Folder::FindEntry(const PathString&folder_str, bool&outIsDirectory,bool&outExists,UINT32 findFlags)	const
		{
			if (FindEntry(folder_str,findResult,outIsDirectory,outExists,findFlags))
				return &findResult;
			return NULL;
		}

		bool	 Folder::FindEntry(const PathString&folder_str, File&out, bool&outIsDirectory,bool&outExists,UINT32 findFlags)	const
		{
			outExists = false;
			outIsDirectory = false;
			if (!IsValidLocation())
				return false;
			if (!ResolvePath(folder_str,nullptr,out.location) && (findFlags & FindFlags::MustExists) != 0)
			{
				return false;
			}
			out.name = ExtractFileNameExt(out.location);
			#if SYSTEM==WINDOWS
				if (out.name.EndsWith(':'))
				{
					PathString driveName = out.name+L'\\';
					UINT type = GetDriveTypeW(driveName.c_str());
					bool valid = type == DRIVE_FIXED || type == DRIVE_REMOTE || type == DRIVE_CDROM || type == DRIVE_RAMDISK;
					outIsDirectory = true;
					outExists = valid;
					if (findFlags & FindFlags::MustBeFile)
						return false;
					return valid || (findFlags & FindFlags::MustExists) == 0;
				}

				DWORD attributes = GetFileAttributesW(out.location.c_str());
				if (attributes == INVALID_FILE_ATTRIBUTES)
				{
					return (findFlags & FindFlags::MustExists) == 0;
				}
				bool isDir = outIsDirectory = (attributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
				outExists = true;
			#elif SYSTEM==LINUX
				struct stat s;
				if (stat(out.location.c_str(),&s))
					return (findFlags & FindFlags::MustExists) == 0;
				bool isDir = outIsDirectory = s.st_mode&S_IFDIR;
				outExists = true;
			#else
				#error not supported
			#endif
			if (isDir && (findFlags & FindFlags::MustBeFile))
				return false;
			if (!isDir && (findFlags & FindFlags::MustBeDirectory))
				return false;
			return true;
		}

		const Folder::File*	 Folder::FindFile(const PathString&folder_str, bool mustExist/*=true*/)	const
		{
			UINT32 flags = FindFlags::MustBeFile;
			if (mustExist)
				flags |= FindFlags::MustExists;
			return Find(folder_str,flags);
		}

		bool	 Folder::FindFile(const PathString&folder_str, File&out, bool mustExist/*=true*/)	const
		{
			UINT32 flags = FindFlags::MustBeFile;
			if (mustExist)
				flags |= FindFlags::MustExists;
			return Find(folder_str,out,flags);
		}

		bool	 Folder::FindDirectory(const PathString&folder_str, File&out, bool mustExist/*=true*/)	const
		{
			bool wasDir,exists;
			UINT32 flags = FindFlags::MustBeDirectory;
			if (mustExist)
				flags |= FindFlags::MustExists;
			return FindEntry(folder_str,out,wasDir, exists,flags);
		}

		const Folder::File*	 Folder::FindDirectory(const PathString&folder_str, bool mustExist/*=true*/)	const
		{
			bool wasDir,exists;
			UINT32 flags = FindFlags::MustBeDirectory;
			if (mustExist)
				flags |= FindFlags::MustExists;
			return FindEntry(folder_str,wasDir, exists,flags);
		}

		const Folder::File*	 Folder::CreateDirectory(const PathString&name)	const
		{
			if (!IsValidLocation())
				return NULL;
			PathString location = absolute_folder;
			if (!IsSlash(location.LastChar()))
				location += FOLDER_SLASH;
			location += name;
			//locate(
			PathString final;
			ResolvePath(location,nullptr,final);
			return FileSystem::CreateDirectory(final);
		}

		bool	 Folder::CreateDirectory(const PathString&name, File&out)	const
		{
			if (!IsValidLocation())
				return false;
			PathString location = absolute_folder;
			if (!IsSlash(location.LastChar()))
				location += FOLDER_SLASH;
			location += name;
			ResolvePath(location,nullptr,location);
			return FileSystem::CreateDirectory(location, out);
		}





		String		 Folder::ToString() const
		{
			if (!IsValidLocation())
				return "Folder: "+String(absolute_folder)+"(invalid)";
			return "Folder: "+String(absolute_folder);
		}

		const PathString&  Folder::LocationStr() const
		{
			return absolute_folder;
		}

		const PathString&  Folder::GetLocation() const
		{
			return absolute_folder;
		}


		const Folder::File* Folder::Location() const
		{
			if (!IsValidLocation())
				return NULL;
			locationFile.name = ExtractFileNameExt(absolute_folder);
			locationFile.location = absolute_folder;
			return &locationFile;
		}

		const Folder::File* Folder::ParentLocation()	  const
		{
			if (!IsValidLocation())
				return NULL;
			#if SYSTEM==LINUX
				if (absolute_folder == '/' || absolute_folder == '\\')
					return nullptr;
			#endif

			PathString	 super = ExtractFileDir(absolute_folder);
			#if SYSTEM==WINDOWS
				if (!super.GetLength())
					return NULL;
			#elif SYSTEM==LINUX
				if (!super.GetLength())
				{
					file.location = FOLDER_SLASH;
					file.name = FOLDER_SLASH;
					file.is_folder = true;
					return &file;
				}
			#else
				#error not supported
			#endif
			locationFile.location = super;
			locationFile.name = ExtractFileNameExt(super);
			return &locationFile;
		}


		bool				Folder::operator<(const Folder&other)  const
		{
			return absolute_folder < other.absolute_folder;
		}

		bool				Folder::operator>(const Folder&other)  const
		{
			return absolute_folder > other.absolute_folder;
		}

		bool				Folder::operator<=(const Folder&other)  const
		{
			return absolute_folder <= other.absolute_folder;
		}

		bool				Folder::operator>=(const Folder&other)  const
		{
			return absolute_folder >= other.absolute_folder;
		}

		bool				Folder::operator==(const Folder&other)  const
		{
			return absolute_folder == other.absolute_folder;
		}

		bool				Folder::operator!=(const Folder&other)  const
		{
			return absolute_folder != other.absolute_folder;
		}



		PathString WorkingDirectory()
		{
			return GetWorkingDirectory();
		}

		PathString GetWorkingDirectory()
		{
			#if SYSTEM==WINDOWS
				//static const size_t len = 0x2000;
				wchar_t buffer[0x2000];
				if (!GetCurrentDirectoryW(ARRAYSIZE(buffer)-1,buffer))
					return L"";
				PathString rs = buffer;
				if (!rs.BeginsWith(ABS_MARKER))
					rs = ABS_MARKER + rs;
	//			size_t len = ABS_MARKER_LENGTH;
				return rs;
			#elif SYSTEM==LINUX
				char buffer[0x1000];
				if (!getcwd(buffer,ARRAYSIZE(buffer)))
					return "";
				return buffer;
			#else
				#error not supported
			#endif
		}


		bool  SetWorkingDirectory(const PathString&path)
		{
			#if SYSTEM==WINDOWS
				return !!SetCurrentDirectoryW(path.c_str());
			#elif SYSTEM==LINUX
				return !chdir(path.c_str());
			#else
				#error not supported
			#endif
		}

		bool  SetWorkingDirectory(const Folder&folder)
		{
			return SetWorkingDirectory(folder.GetLocation());
		}

		void		GetDriveList(Ctr::Array<Drive>&out)
		{
    		count_t cnt = GetDriveList(NULL,0);
			out.SetSize(cnt);
			GetDriveList(out.pointer(),out.GetLength());
		}
		count_t GetDriveList(Drive*target, count_t max)
		{
			#if SYSTEM==LINUX
				if (max)
				{
					target->root="/";
					target->name="LINUX root-folder";
				}
				return 1;
			#elif SYSTEM==WINDOWS
				count_t cnt(0);
				wchar_t	volume_name[0x100],drive[] = L"a:\\";
				for (char d = 'a'; d <= 'z'; d++)
				{
					drive[0] = (wchar_t)d;
					UINT type = GetDriveTypeW(drive);
					bool valid = type == DRIVE_FIXED || type == DRIVE_REMOTE || type == DRIVE_CDROM || type == DRIVE_RAMDISK;
					if (!valid || !GetVolumeInformationW(drive, volume_name, ARRAYSIZE(volume_name), NULL, NULL, NULL, NULL, 0))
						continue;
					if (cnt < max)
					{
						target[cnt].root = StringType::Cast("a:");
						target[cnt].root.Set(0,d);
						target[cnt].name = volume_name;
					}
					cnt++;
				}
				return cnt;
			#else
				#error not supported
			#endif
		}

		static File  fs_result;


		template<typename T>
			static StringType::Template<T>	_extractFileName(const StringType::Template<T>&filename)
			{
				index_t last_dot(filename.GetLength()),last_slash(0);
				for (index_t i = 0; i < filename.GetLength(); i++)
				{
					if (IsSlash(filename.GetChar(i)))
						last_slash = i+1;
					if (IsDot(filename.GetChar(i)))
						last_dot = i;
				}
				if (last_dot < last_slash)
					last_dot = filename.GetLength();
				return filename.subString(last_slash,last_dot-last_slash);
			}

		PathString	ExtractFileName(const PathString&filename)
		{
			return _extractFileName(filename);
		}

		template <typename T>
			static StringType::Template<T>	_extractFileExt(const StringType::Template<T>&filename)
			{
				index_t at(filename.GetLength()-1);
				while (!IsSlash(filename.GetChar(at)) && !IsDot(filename.GetChar(at)) && --at < filename.GetLength());
				if (at >= filename.GetLength() || !IsDot(filename.GetChar(at)))
					return StringType::Template<T>();
				return filename.subString(at+1);
			}

		PathString	ExtractFileExt(const PathString&filename)
		{
			return _extractFileExt(filename);
		}

		template <typename T>
			static StringType::Template<T>	_extractFileDir(const StringType::Template<T>&filename)
			{
				if (!filename.GetLength())
					return StringType::Template<T>();
				index_t at = filename.GetLength()-1;
				while (!IsSlash(filename.GetChar(at)) && --at < filename.GetLength());
				if (at >= filename.GetLength())
					return StringType::Template<T>((T)'.');
				/*if (at)
					at--;*/
				return filename.subString(0,at);
			}

		PathString	ExtractFileDir(const PathString&filename)	{return _extractFileDir(filename);}

		template <typename T>
			static StringType::Template<T>	 _extractFileDirName(const StringType::Template<T>&filename)
			{
				index_t at(filename.GetLength()-1);
				if (at >= filename.GetLength())
					return StringType::Template<T>();
				while (!IsSlash(filename.GetChar(at)) && !IsDot(filename.GetChar(at)) && --at < filename.GetLength());
				if (at >= filename.GetLength() || !IsDot(filename.GetChar(at)))
					return filename;
				return filename.subString(0,at);
			}

		PathString	 ExtractFileDirName(const PathString&filename)	{return _extractFileDirName(filename);}

		template <typename T>
			static StringType::Template<T>	 _extractFileNameExt(const StringType::Template<T>&filename)
			{
				index_t at = filename.GetLength()-1;
				if (at>= filename.GetLength())
					return StringType::Template<T>();
				while (!IsSlash(filename.GetChar(at)) && --at < filename.GetLength());
				return filename.subString(at+1);
			}

		PathString	 ExtractFileNameExt(const PathString&filename)	{return _extractFileNameExt(filename);}


		template <typename T>
			static StringType::Template<T>	_escapeSpaces(StringType::Template<T> path)
			{
				for (;;)
				{
					index_t at = path.Find((T)'\\');
					if (at != InvalidIndex)
						path.Erase(at,1);
					else
						break;
				}
				StringType::Template<T> final;
				for (index_t i = 0; i < path.GetLength(); i++)
				{
					if (path.GetChar(i) == (T)' ')
						final += (T)'\\';
					final += path.GetChar(i);
				}
				return final;
			}
		PathString	 EscapeSpaces(const PathString &path)	{return _escapeSpaces(path);}


		bool IsDirectory(const PathString&name)
		{
			return IsFolder(name);
		}


		bool IsFolder(const PathString&name)
		{
			#if SYSTEM==WINDOWS
				if (name.EndsWith(':'))
				{
					PathString driveName = name+L'\\';
					UINT type = GetDriveTypeW(driveName.c_str());
					bool valid = type == DRIVE_FIXED || type == DRIVE_REMOTE || type == DRIVE_CDROM || type == DRIVE_RAMDISK;
					return valid;
				}

				if (name.Contains('/'))//this is apparently bad when using extended path strings
				{
					StringW copy(name);
					copy.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
					return IsFolder(copy);
				}
				DWORD attribs = GetFileAttributesW(name.c_str());
				return attribs != INVALID_FILE_ATTRIBUTES && (attribs&FILE_ATTRIBUTE_DIRECTORY);
			#elif SYSTEM==LINUX
				struct ::stat s;
				return !stat(name.c_str(),&s) && (s.st_mode&S_IFDIR);
			#else
				#error not supported
			#endif
		}

		bool IsFile(const PathString&name)
		{
			return IsFile(name.c_str());
		}

		bool IsFile(const PathString::char_t*name)
		{
			#if SYSTEM==WINDOWS
				const PathString::char_t*chk = name;
				while (*chk)
				{
					if (*chk == (PathString::char_t)'/')
					{
						//this is apparently bad when using extended path strings
						StringW copy(name);
						copy.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
						return IsFile(copy.c_str());
					}
					chk++;
				}
				DWORD attribs = GetFileAttributesW(name);
				return attribs != INVALID_FILE_ATTRIBUTES && !(attribs&FILE_ATTRIBUTE_DIRECTORY);
			#elif SYSTEM==LINUX
				struct ::stat s;
				return !stat(name,&s) && !(s.st_mode&S_IFDIR);
			#else
				#error not supported
			#endif
		}


		bool DoesExist(const PathString&name)
		{
			#if SYSTEM==WINDOWS
				if (name.EndsWith(':'))
				{
					PathString driveName = name+L'\\';
					UINT type = GetDriveTypeW(driveName.c_str());
					bool valid = type == DRIVE_FIXED || type == DRIVE_REMOTE || type == DRIVE_CDROM || type == DRIVE_RAMDISK;
					return valid;
				}
			
				if (name.Contains('/'))//this is apparently bad when using extended path strings
				{
					StringW copy(name);
					copy.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
					return DoesExist(copy);
				}
				return GetFileAttributesW(name.c_str()) != INVALID_FILE_ATTRIBUTES;
			#elif SYSTEM==LINUX
				struct ::stat s;
				return !stat(name.c_str(),&s);
			#else
				#error not supported
			#endif
		}


		fsize_t		GetFileSize(const PathString&name)
		{
			#if SYSTEM==WINDOWS
				if (name.Contains('/'))//this is apparently bad when using extended path strings
				{
					StringW copy(name);
					copy.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
					return GetFileSize(copy);
				}
				WIN32_FILE_ATTRIBUTE_DATA	data;
				if (!GetFileAttributesExW(name.c_str(),GetFileExInfoStandard,&data))
				{
					return 0;
				}
				return ((fsize_t)data.nFileSizeHigh) << 32 | (fsize_t)data.nFileSizeLow;
			#elif SYSTEM==LINUX
				struct ::stat s;
				if (stat(name.c_str(),&s))
					return 0;
				return s.st_size;
			#else
				#error not supported
			#endif
		}


		ftime_t GetModificationTime(const PathString&name)
		{
			#if SYSTEM==WINDOWS
				if (name.Contains('/'))//this is apparently bad when using extended path strings
				{
					StringW copy(name);
					copy.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
					return GetModificationTime(copy);
				}

				WIN32_FILE_ATTRIBUTE_DATA	data;
				if (!GetFileAttributesExW(name.c_str(),GetFileExInfoStandard,&data))
				{
					return 0;
				}
				UINT64		ctime,
							wtime;
				memcpy(&ctime,&data.ftCreationTime,sizeof(ctime));
				memcpy(&wtime,&data.ftLastWriteTime,sizeof(wtime));

				wtime = (wtime - 116444736000000000ULL) / 10000000ULL;
				ctime = (ctime - 116444736000000000ULL) / 10000000ULL;
				return std::max(wtime,ctime);
			#elif SYSTEM==LINUX
				struct ::stat s;
				if (stat(name.c_str(),&s))
					return 0;
				return s.st_ctime;
			#else
				#error not supported
			#endif
		}


		const File*  MoveFile(const PathString&source, const PathString&destination)
		{
			return Move(source, destination);
		}


		const File*  Move(const PathString&source, const PathString&destination)
		{
			#if SYSTEM==WINDOWS
				if (source.Contains('/') || destination.Contains('/'))//this is apparently bad when using extended path strings
				{
					StringW copy0(source),copy1(destination);
					copy0.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
					copy1.FindAndReplace((PathString::char_t)'/',(PathString::char_t)'\\');
					return Move(copy0,copy1);
				}

				if (!MoveFileW(source.c_str(),destination.c_str()))
					return NULL;
			#elif SYSTEM==LINUX
				if (rename(source.c_str(),destination.c_str()))
					return NULL;
			#else
				#error not supported
			#endif
			fs_result.name = ExtractFileNameExt(destination);
			fs_result.location = destination;
			return &fs_result;
		}

		const File*  Copy(const PathString&source, const PathString&destination, bool overwrite_if_exist)
		{
			if (!Copy(source,destination,fs_result,overwrite_if_exist))
				return NULL;
			return &fs_result;
		}


		bool		Copy(const PathString&source, const PathString&destination, File&outFile, bool overwrite_if_exist)
		{
			if ((IsFile(destination)) && !overwrite_if_exist)
				return false;
			#if SYSTEM==WINDOWS
				PathString my_destination = destination;
				if (IsFolder(destination) && !IsFolder(source))
					my_destination += FOLDER_SLASH+ExtractFileNameExt(source);
				if (IsFolder(source))
				{
					if (!DoesExist(destination))
					{
						File dummy;
						if (!CreateDirectory(destination,dummy))
						{
							return false;
						}
					}
					if (IsFile(destination))
						return false;
					FolderScan scan = Folder(source).Scan();
					Folder write(destination);
					File found,dest,copied;
					while (scan.NextEntry(found))
					{
						if (write.Find(found.GetName(),dest,FindFlags::None))
							Copy(found.GetLocation(),dest.GetLocation(),copied,overwrite_if_exist);
					}
				}
				else
				{
					if (!CopyFileW(source.c_str(),my_destination.c_str(),!overwrite_if_exist) && _wsystem((L"copy \""+Escape(source)+L"\" \""+Escape(destination)+L"\"").c_str()))
						return false;
				}
			#elif SYSTEM==LINUX
				/*FILE*fsource = fopen(EscapeSpaces(source),"rb");
				if (!source)
					return NULL;
				FILE*ftarget = fopen(EscapeSpaces(destination),"wb");
				if (!ftarget)
				{
					fclose(fsource);
					return NULL;
				}
			
				BYTE buffer[1024];
				int read;
				do
				{
					read = fread(buffer,1,sizeof(buffer),fsource);
					fwrite(buffer,1,read,ftarget);
				}
				while (read == sizeof(buffer));
			
				fclose(fsource);
				fclose(ftarget);*/
		
			
				PathString call = ("cp "+EscapeSpaces(source)+" "+EscapeSpaces(destination));
				if (system(call))
					return false;
				//sleep(500);
			#else
				#error not supported
			#endif
			outFile.name = ExtractFileNameExt(destination);
			outFile.location = destination;
			return true;
		}

		const File*  Copy(const File*source, const PathString&destination, bool overwrite)
		{
			if (!source)
				return NULL;
			return Copy(source->GetLocation(),destination,overwrite);
		}

		const File*  Copy(const File&source, const PathString&destination, bool overwrite)
		{
			return Copy(source.GetLocation(),destination,overwrite);
		}


		const File*  CopyFile(const PathString&source, const PathString&destination, bool overwrite_if_exist)
		{
			return Copy(source,destination,overwrite_if_exist);
		}

		const File*  CopyFile(const File*source, const PathString&destination, bool overwrite)
		{
			return Copy(source,destination,overwrite);
		}

		const File*  CopyFile(const File&source, const PathString&destination, bool overwrite)
		{
			return Copy(source,destination,overwrite);
		}


		const File*  Move(const File*source, const PathString&destination)
		{
			return Move(source->GetLocation(),destination);
		}

		const File*  Move(const File&source, const PathString&destination)
		{
			return Move(source.GetLocation(),destination);
		}


		const File*  MoveFile(const File*source, const PathString&destination)
		{
			return Move(source->GetLocation(),destination);
		}

		const File*  MoveFile(const File&source, const PathString&destination)
		{
			return Move(source.GetLocation(),destination);
		}

		const File*  CreateDirectory(const PathString&folder_name)
		{
			if (CreateDirectory(folder_name,fs_result))
				return &fs_result;
			return NULL;
		}



		void _CreateLastDirectorySegment(const PathString&path, const TCodeLocation&loc)
		{
			#if SYSTEM==WINDOWS
				if (!CreateDirectoryW(path.c_str(),NULL))
				{
					DWORD err = GetLastError();
					if (err != ERROR_ALREADY_EXISTS)
					{
						char buffer[0x1000];
						System::WindowsErrorToString(err, buffer, sizeof(buffer));

						throw Except::IO::DriveAccess::GeneralFault(loc,"Failed to create last directory segment of '"+PathToString(path)+"': "+String(buffer));
					}
				}
			#elif SYSTEM==LINUX || SYSTEM==LINUX
				if (mkdir(out.location.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) && errno != EEXIST)
					throw Except::IO::DriveAccess::GeneralFault(loc,"Failed to create last directory segment of '"+PathToString(path)+"'");
			#else
				#error not supported
			#endif
		}

		void  _CreateDirectory(const PathString&path, File&out, bool unlinkFoundFiles, const TCodeLocation&loc)
		{
			if (!path.GetLength())
				throw Except::IO::DriveAccess::GeneralFault(loc,"Path is empty");
			out.location = path;
			if (IsSlash(out.location.LastChar()))
				out.location.Erase(out.location.GetLength()-1);
			if (out.IsDirectory())
				return;

			static const PathString::char_t slash = PathString::char_t('/');
			static const PathString::char_t localSlash = FOLDER_SLASH;

			out.location.FindAndReplace(PathString::char_t( '\\'),slash);//make uniform
			Ctr::Array<PathStringRef>	segments;
			Explode(slash,out.location,segments);
			bool absoluteWide = out.location.BeginsWith(ABS_MARKER);

			for (index_t i = 1; i <= segments.Count(); i++)
			{
				PathString path = implode(localSlash,segments.pointer(),i);
				if (path.IsEmpty() 
					|| path==localSlash
					#if SYSTEM==WINDOWS
						|| path == L"\\\\?" || path.EndsWith(L':')
					#endif
					)
					continue;
				if (IsDirectory(path))
					continue;
				if (IsFile(path))
				{
					if (!unlinkFoundFiles || !TryToUnlinkFile(path))
					{
						if (unlinkFoundFiles)
						{
							#if SYSTEM==WINDOWS
								char buffer[0x1000];
								System::WindowsErrorToString(GetLastError(), buffer, sizeof(buffer));

								throw Except::IO::DriveAccess::GeneralFault(loc,"Failed to create last directory segment of '"+PathToString(path)+"': File was found with that name but could not be deleted: "+String(buffer));
							#else
								throw Except::IO::DriveAccess::GeneralFault(loc,"Failed to create last directory segment of '"+PathToString(path)+"': File was found with that name but could not be deleted");
							#endif
						}
						else
							throw Except::IO::DriveAccess::GeneralFault(loc,"Failed to create last directory segment of '"+PathToString(path)+"': File was found with that name");
					}
				}
				_CreateLastDirectorySegment(path,loc);
			}
			
			if (slash != localSlash)
				out.location.FindAndReplace(slash,localSlash);
			ASSERT__( Folder().FindDirectory(out.location,out) );
		}

		bool  CreateDirectory(const PathString&folder_name, File&out)
		{
			try
			{
				_CreateDirectory(folder_name, out,false,CLOCATION);
				return true;
			}
			catch (...)
			{
				return false;
			}
		}

		void			ForceCreateDirectory(const PathString&path)
		{
			File f;
			ForceCreateDirectory(path,f);
		}
		void			ForceCreateDirectory(const PathString&path, File&out)
		{
			_CreateDirectory(path, out,true,CLOCATION);
		}



	#if SYSTEM==WINDOWS
		void PrintLastError(const String&whileDoing)
		{
			DWORD code = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				code,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPWSTR)&lpMsgBuf,
				0,
				NULL
				);
			//unsigned len = strlen((char*)lpMsgBuf);
			ErrMessageW(StringW(whileDoing)+ StringType::Cast(": ")+ (const wchar_t*)lpMsgBuf);

			LocalFree(lpMsgBuf);
		}

	#endif

		void			UnlinkFile(const PathString&location)
		{
			if (!IsFile(location))
			{
				if (IsDirectory(location))
					throw Except::IO::DriveAccess::GeneralFault("Unable to unlink "+PathToString(location)+": Is a directory");
				return;	//all good
			}
			#if SYSTEM==LINUX
				if (!::unlink(location.c_str()))
					throw Except::IO::DriveAccess::GeneralFault("Unable to unlink "+PathToString(location));
			#elif SYSTEM==WINDOWS
				int retry = 0;
				PathString loc = location;
				loc.FindAndReplace(L'/',L'\\');
				while (!DeleteFileW(loc.c_str()))
				{
					if (retry > 10)
					{
						char buffer[0x1000];
						System::WindowsErrorToString(GetLastError(), buffer, sizeof(buffer));

						throw Except::IO::DriveAccess::GeneralFault("Unable to unlink "+PathToString(location)+": "+buffer);
					}
					if (retry == 0)
						SetFileAttributesW(loc.c_str(), FILE_ATTRIBUTE_NORMAL);
					else
						Sleep(10);
					retry++;
				}
			#else
				#error not supported
			#endif
		}

		bool			TryToUnlinkFile(const PathString&location)
		{
			if (!IsFile(location))
			{
				if (IsDirectory(location))
					return false;
				return true;	//all good
			}
			#if SYSTEM==LINUX
				return ::unlink(location.c_str());
			#elif SYSTEM==WINDOWS
				int retry = 0;
				PathString loc = location;
				loc.FindAndReplace(L'/',L'\\');
				return DeleteFileW(loc.c_str());
			#else
				#error not supported
			#endif
		}


		bool			TryToRemoveFolderContents(const PathString&location)
		{
			FolderScan scan = Folder(location).Scan();
			if (!scan.IsValidLocation())
				return false;
			FileSystem::File file;
			bool dir;
			while (scan.NextEntry(file,dir))
			{
				if (dir)
				{
					if (!TryToRemoveFolder(file.GetLocation()))
						return false;
				}
				else
					if (!file.TryToUnlink())
						return false;
			}
			return true;
		}

		void			RemoveFolderContents(const PathString&location)
		{
			FolderScan scan = Folder(location).Scan();
			if (!scan.IsValidLocation())
				throw Except::IO::DriveAccess::GeneralFault("Trying to erase invalid directory "+PathToString(location));
			FileSystem::File file;
			bool dir;
			while (scan.NextEntry(file,dir))
			{
				if (dir)
				{
					RemoveFolder(file.GetLocation());
				}
				else
					file.Unlink();
			}
		}

		bool			TryToRemoveFolder(const PathString&location)
		{
			if (!IsFolder(location))
			{
				if (IsFile(location))
					return false;
				return true;
			}
			if (!TryToRemoveFolderContents(location))
				return false;
			#if SYSTEM==LINUX
				return !rmdir(location.c_str());
			#elif SYSTEM==WINDOWS
				bool rs = !!RemoveDirectoryW(location.c_str());
				//const char* err = getLastError();
				return rs;
			#else
				#error not supported
			#endif
		}


		void			RemoveFolder(const PathString&location)
		{
			if (!IsFolder(location))
			{
				if (IsFile(location))
					throw Except::IO::DriveAccess("Cannot remove folder "+PathToString(location)+": is file");
				return;
			}
			RemoveFolderContents(location);
			#if SYSTEM==LINUX
				if (!!rmdir(location.c_str()))
					throw Except::IO::DriveAccess("Failed to remove folder "+PathToString(location));
			#elif SYSTEM==WINDOWS
				bool rs = !!RemoveDirectoryW(location.c_str());
				if (!rs)
				{
					char buffer[0x1000];
					System::WindowsErrorToString(GetLastError(), buffer, sizeof(buffer));
					throw Except::IO::DriveAccess("Failed to remove folder "+PathToString(location)+": "+buffer);
				}
			#else
				#error not supported
			#endif
		}

		static const File* ResolveLink(const File*file)
		{
			#if SYSTEM==LINUX
				if (!file)
					return NULL;

				char buffer[0x2000];
				int len = readlink(file->GetLocation().c_str(),buffer,ARRAYSIZE(buffer));
				if (len == -1 || len ==ARRAYSIZE(buffer))
					return file;
				buffer[len] = 0;	//appearingly readlink does not terminate the string by itself
		//		ShowMessage("symlink pointing to "+PathString(buffer));
				Folder folder(file->GetFolder());
				return folder.FindFile(buffer);
			#else

				return file;
			#endif
		}

		static bool ResolveLink(File&file)
		{
			#if SYSTEM==LINUX
				char buffer[0x2000];
				int len = readlink(file.GetLocation().c_str(),buffer,ARRAYSIZE(buffer));
				if (len == -1 || len >= ARRAYSIZE(buffer))
					return true;
				buffer[len] = 0;	//appearingly readlink does not terminate the string by itself
		//		ShowMessage("symlink pointing to "+PathString(buffer));
				Folder folder(file.GetFolder());
				return folder.FindFile(buffer,file);
			#else

				return true;
			#endif
		}

		const File*  LocateExecutable(const char*name)
		{
			if (LocateExecutable(name,fs_result))
				return &fs_result;
			return NULL;
		}

		bool  LocateExecutable(const char*name, File&target)
		{
			PathString wd = GetWorkingDirectory();
			Folder folder(wd);

			PathString filename= StringToPath(name);

			#if SYSTEM==LINUX
				if (filename.GetIndexOf('/'))
			#endif
			{
				#if SYSTEM==WINDOWS
					if (folder.FindFile(filename,target))
						return ResolveLink(target);
					if (folder.FindFile(filename+StringType::Cast(".exe"),target))
						return ResolveLink(target);
					if (folder.FindFile(wd+FOLDER_SLASH_STR+filename,target))
						return ResolveLink(target);
					if (folder.FindFile(wd+FOLDER_SLASH_STR+filename+StringType::Cast(".exe"),target))
						return ResolveLink(target);
				#else
					if (folder.FindFile(filename,target))
						return ResolveLink(target);
					if (folder.FindFile(wd+FOLDER_SLASH_STR+filename,target))
						return ResolveLink(target);
				#endif
			}

			{
				Ctr::Array<char> field(getenv("PATH"));
				char*segment_begin = field.pointer();
				for (index_t i = 1; i < field.GetLength()-1; i++)
					if (field[i] == ENV_PATH_SEPARATOR)
					{
						field[i] = 0;
						if (IsSlash(field[i-1]))
							field[i-1] = 0;
						filename = FileSystem::StringToPath(String(segment_begin)+FOLDER_SLASH+name);
						if (folder.FindFile(filename,target))
							return ResolveLink(target);
						if (folder.FindFile(filename+StringType::Cast(".exe"),target))
							return ResolveLink(target);
						segment_begin = field+i+1;
					}
				{
					filename = FileSystem::StringToPath(String(segment_begin)+FOLDER_SLASH+name);
					if (folder.FindFile(filename,target))
						return ResolveLink(target);
					if (folder.FindFile(filename+StringType::Cast(".exe"),target))
						return ResolveLink(target);
				}
			}
			return false;
		}

		PathString			GetAbsolutePath(const PathString&relative_path)
		{
			Folder	folder(GetWorkingDirectory());
			File file;
			if (folder.Find(relative_path,file,FindFlags::None))
				return file.GetLocation();
			return relative_path;
		}

		PathString			GetRelativePath(const PathString&origin, const PathString&destination)
		{
			const PathString::char_t	*o = origin.c_str(),
										*d = destination.c_str(),
										*of = origin.c_str(),
										*df = destination.c_str();
			while (*o && (*o == *d || (IsSlash(*o) && IsSlash(*d))))
			{
				if (IsSlash(*o))
				{
					of = o+1;
					df = d+1;
				}
				o++;
				d++;
			}
			if (!*o && (IsSlash(*d)))
			{
				of = o;
				df = d+1;
			}
			if (!*d && (IsSlash(*o)))
			{
				df = d;
				of = o+1;
			}
		
			if (of == origin.c_str())
				return destination; //no relative path found
		//	unsigned down(0);
			PathString rs;
			o = of;
			while (*o)
			{
				if (IsSlash(*o))
					rs  += StringToPath("../");
				o++;
			}
			return rs + df;
		}

	}
}
