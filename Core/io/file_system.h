#ifndef file_systemH
#define file_systemH


#include "../global_root.h"
#if SYSTEM == WINDOWS
	#include <winbase.h>
	#include <stdio.h>
#elif SYSTEM == LINUX
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <errno.h>
#endif

#include <ctype.h>
#include "../global_string.h"
#include "../general/system.h"
#include "../general/thread_system.h"

#undef CopyFile
#undef MoveFile
#undef CreateDirectory

namespace DeltaWorks
{

	/**
		\brief Plattform independent file system abstraction
	
		The FileSystem module attempts to create a more abstract view on the local file system, avoiding system specific peculiarities.
	*/
	namespace FileSystem
	{
		class File;
		class Drive;
		class Folder;
		class FolderScan;



	
		typedef unsigned long long ftime_t;	//!< File time type (unix time)
		typedef unsigned long long fsize_t;
	
	
		#if SYSTEM==WINDOWS
			static const int		MaxDrives=26;
		#else
			static const int		MaxDrives=1;
		#endif

		PathString		ExtractFileExt(const PathString&filename);										//!< Extracts the file extension of \b filename without preceding '.'
		PathString		ExtractFileDir(const PathString&filename);										//!< Extracts the file directory of \b filename without trailing slash
		PathString		ExtractFileName(const PathString&filename);										//!< Extracts the file name of \b filename without preceding folder or succeeding extension (neither preceding slashes or succeeding dots either)
		PathString		ExtractFileDirName(const PathString&filename);									//!< Extracts folders and file name of \b filename but no trailing extension (or extension dot)
		PathString		ExtractFileNameExt(const PathString&filename);									//!< Extracts the file name and extension of \b filename but no preceding folders (or slashes)
		PathString		EscapeSpaces(const PathString&path);											//!< Inserts a backslash before every space character (for LINUX systems)
	
		ftime_t			GetModificationTime(const PathString&name);										//!< Returns the LINUX timestamp of the specified file \param name File name \return LINUX timestamp
		fsize_t			GetFileSize(const PathString&name);												//!< Returns the size (in bytes) of the specified file
		bool			DoesExist(const PathString&path);												//!< Checks if the specified file or folder exists \param path File/folder name \return true if the specified file/folder exists, false otherwise
		bool			IsFile(const PathString::char_t*path);											//!< Checks if the specified file exists \param path Filename \return true if the entry exists in the file system and is a file, false otherwise
		bool			IsFile(const PathString&path);													//!< Checks if the specified file exists \param path Filename \return true if the entry exists in the file system and is a file, false otherwise
		bool			IsFolder(const PathString&path);												//!< Checks if the specified folder/directory exists (identical to IsDirectory()) \param path Folder name \return true if the entry exists in the file system and is a folder, false otherwise
		bool			IsDirectory(const PathString&path);												//!< Checks if the specified folder/directory exists (identical to IsFolder()) \param path Folder name \return true if the entry exists in the file system and is a folder, false otherwise
		PathString		WorkingDirectory(); 															//!< Returns the current working directory without trailing slash
		PathString		GetWorkingDirectory();	 														//!< Identical to GetWorkingDirectory()
		bool			SetWorkingDirectory(const PathString&path); 									//!< Sets the current working directory \param path new working directory
		bool			SetWorkingDirectory(const Folder&folder); 										//!< Sets the current working directory \param folder new working directory
		count_t			GetDriveList(Drive*target,count_t max);											//!< Queries system drives \param target Pointer to a drive field to write to \param max Number of entries in the drive field \return Total number of drives in the system
		void			GetDriveList(Ctr::Array<Drive>&out);													//!< Queries system drives \param target Reference to an array to write to
		bool			Copy(const PathString&source, const PathString&destination, File&outFile, bool overwrite_if_exist);		//!< Identical to copyFile(). Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		Copy(const PathString&source, const PathString&destination, bool overwrite_if_exist);		//!< Identical to copyFile(). Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		Copy(const File*source, const PathString&destination, bool overwrite_if_exist);		//!< Identical to copyFile(). Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		Copy(const File&source, const PathString&destination, bool overwrite_if_exist);		//!< Identical to copyFile(). Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		CopyFile(const PathString&source, const PathString&destination, bool overwrite_if_exist);	//!< Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		CopyFile(const File*source, const PathString&destination, bool overwrite_if_exist);	//!< Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		CopyFile(const File&source, const PathString&destination, bool overwrite_if_exist);	//!< Copies \b source to \b destination . \param source Source file \param destination Destination file \param overwrite_if_exist Specifies whether or not \b destination should be overwritten if existing \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		Move(const PathString&source, const PathString&destination);								//!< Identical to MoveFile(). Moves \b source to \b destination . \param source Source file \param destination Destination file \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		Move(const File*source, const PathString&destination);								//!< Identical to MoveFile(). Moves \b source to \b destination . \param source Source file \param destination Destination file \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		Move(const File&source, const PathString&destination);								//!< Identical to MoveFile(). Moves \b source to \b destination . \param source Source file \param destination Destination file \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		MoveFile(const PathString&source, const PathString&destination);					//!< Moves \b source to \b destination . \param source Source file \param destination Destination file \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		MoveFile(const File*source, const PathString&destination);						//!< Moves \b source to \b destination . \param source Source file \param destination Destination file \return pointer to a global file object pointing to the destination file or NULL on failure
		const File*		MoveFile(const File&source, const PathString&destination);						//!< Moves \b source to \b destination . \param source Source file \param destination Destination file \return pointer to a global file object pointing to the destination file or NULL on failure
		/**
		Recursively checks all segments of the specified path, and tries to create each as a directory.
		If the specified path is found to be relative then it is assumed relative to the current working directory.
		@param path Name(s) of the folder to create
		@return pointer to a global file object pointing to the newly created folder or NULL on failure
		*/
		const File*		CreateDirectory(const PathString&path);
		/**
		@copydoc CreateDirectory(const PathString&)
		@param[out] out Created folder
		@return true on success, false otherwise
		*/
		bool			CreateDirectory(const PathString&folder_name, File&out);
		/**
		Recursively checks all segments of the specified path, and tries to create each as a directory.
		Contrary to CreateDirectory()/CreateDirectory(), this function also checks if any files are found
		matching the specified segment, and tries to erase them.
		The function throws Except::IO::DriveAccess::GeneralFault in case of errors
		@param path Path of the folder to create
		*/
		void			ForceCreateDirectory(const PathString&path);
		/**
		@copydoc ForceCreateDirectory(const PathString&);
		@param[out] out Created folder
		*/
		void			ForceCreateDirectory(const PathString&path, File&out);
		
		void			UnlinkFile(const PathString&filename);											//!< Unlinks (erases) the specified file. Throws IO::DriveAccess::GeneralFault in case of failures
		bool			TryToUnlinkFile(const PathString&filename);										//!< Unlinks (erases) the specified file. @return true on success
		bool			TryToRemoveFolder(const PathString&foldername);										//!< Recursively erases the specified folder and all contained files \return true on success, false if any one file or directory could not be deleted
		bool			TryToRemoveFolderContents(const PathString&foldername);								//!< Recursively erases the contents of the specified folder \return true on success, false if any one file or directory could not be deleted
		void			RemoveFolder(const PathString&foldername);										//!< Recursively erases the specified folder and all contained files. Throws IO::DriveAccess::GeneralFault in case of failures
		void			RemoveFolderContents(const PathString&foldername);								//!< Recursively erases the contents of the specified folder. Throws IO::DriveAccess::GeneralFault in case of failures
		PathString		GetRelativePath(const PathString&origin, const PathString&destination);	//!< Creates a relative path from \b origin to \b destination \return relative path
		PathString		GetAbsolutePath(const PathString&relative_path);	//!< Converts the specified relative path to an absolute path based on the current working directory.

		const File*		LocateExecutable(const char*name);				//!< Identifies a program executable depending on the global path variables. Also follows symlinks. Windows .lnk files not supported yet. \param name Name of the executable to locate \return Pointer to a global file object pointing to the executable or NULL if the executable could not be located
		bool			LocateExecutable(const char*name, File&target);	//!< Identifies a program executable depending on the global path variables. Also follows symlinks. Windows .lnk files not supported yet. \param name Name of the executable to locate \param target Target File to store the file target in. \return true on if the executable could be located, false otherwise


		/**
		Replaces or adds a new prefix replacement rule to be used by PathToString().
		Example:
		SetPathPrefix("C:","Root");
		would modify all paths starting with C: such that they start with [Root] instead:
		C:\yolo.txt -> [Root]\yolo.txt

		@param prefix Path prefix to replace in PathToString()
		@param replaceWithCaption Name to put in brackets before path
		*/
		void			SetPathPrefix(const PathString&prefix, const String&replaceWithCaption);
		/**
		Clears all registered path prefixes
		*/
		void			ClearPathPrefixes();

		String			PathToString(const PathString&path, bool replaceDetectedPrefixes=true);
		PathString		StringToPath(const StringRef&utf8Path);
		PathString		StringToPath(const String&utf8Path);
		PathString		StringToPath(const char*utf8Path);
		PathString		StringToPath(const UTF32StringRef&utf32Path);
		PathString		StringToPath(const UTF32String&utf32Path);


		/**
			\brief File/folder handle.

		*/
		class File
		{
		private:
			PathString		name,
							location;
			friend class Folder;
			friend class Drive;
			friend class FolderScan;
			friend bool			FileSystem::Copy(const PathString&source, const PathString&destination, File&outFile, bool overwrite_if_exist);
			friend const File*	FileSystem::Move(const PathString&source, const PathString&destination);
			friend const File*	FileSystem::Move(const File*, const PathString&);
			friend const File*	FileSystem::Move(const File&, const PathString&);
			friend void			_CreateDirectory(const PathString&, File&, bool, const TCodeLocation&loc);
		public:
			/**/				File();
			/**/				File(const Drive&drive);

			void				swap(File&other)
			{
				name.swap(other.name);
				location.swap(other.location);
			}
			friend void swap(File& a, File& b)	{a.swap(b);}

			const File*			GetSibling(const PathString&ext)		const;
			ftime_t				GetModificationTime()				const;		//!< Returns the LINUX filetime of the local entry \return LINUX time stamp
			PathString			GetName()							const;		//!< Returns the name of the local entry (without preceding folder(s)) \return Filename
			PathString			GetInnerName()						const;		//!< Returns the name of the local entry (without preceding folder(s)) excluding any trailing extension \return Inner filename
			const PathString&	GetLocation()						const;		//!< Returns the absolute filename (including any preceding folder(s)) \return Absolute location
			PathString			GetFolder()							const;		//!< Returns the absolute name of the parent folder without trailing slash \return Absolute location of the containing folder
			/**
			Returns the file extension (without preceding dot)
			@return String containing the file extension
			*/
			PathString			GetExtension()						const;
			const PathString::char_t*			GetExtensionPointer()	const;		//!< Returns the file extension (without preceding dot) \return Pointer to the beginning of the local filename's file extension. Does not return NULL
			/**
			Checks if the specified extension equals the local file's extension.
			Comparison is case insensitive.
			@param ext Extension to compare to (with or without preceding dot).
			@return true if the specified extension matches the extension of the local file
			*/
			bool				IsExtension(const PathString::ReferenceType&extRef) const;
			bool				IsExtension(const PathString&ext)	const	{return IsExtension(ext.ToRef());}
			bool				IsExtension(const PathString::char_t*cString) const {return IsExtension(PathString::ReferenceType(cString));}
			#ifdef WIN32
			bool				IsExtension(const char*cString)		const;
			#endif
			bool				IsDirectory()						const;		//!< Returns true if the local entry is a folder/directory \return true if folder
			friend String		ToString(const File&);
			bool				DoesExist()							const;		//!< Queries existence of the local file or folder

			bool				TryToUnlink()						const;		//!< Unlinks (erases) the local file/folder (making the local handle invalid) \return true on success
			void				Unlink()							const;		//!< Unlinks (erases) the local file/folder (making the local handle invalid). Throws IO::DriveAccess::GeneralFault on failure
			
			bool				operator<(const File&other)			const;
			bool				operator>(const File&other)			const;
			bool				operator<=(const File&other)		const;
			bool				operator>=(const File&other)		const;
			bool				operator==(const File&other)		const;
			bool				operator!=(const File&other)		const;

			bool				operator<(const PathString&filename)	const;
			bool				operator>(const PathString&filename)	const;
			bool				operator<=(const PathString&filename)	const;
			bool				operator>=(const PathString&filename)	const;
			bool				operator==(const PathString&filename)	const;
			bool				operator!=(const PathString&filename)	const;
			
			operator const PathString&()								const
			{
				return location;
			}
		};

		/**
			\brief	Handle to a temporary file

		The temporary file is implicitly created when the handle is first queried as a string and is again erased if the handle object is erased.
		Alternatively it may be created explicitly by providing true as constructor parameter or calling create().
		Do not copy the object!
		*/
		class TempFile:public File
		{
		private:
			PathString				filename;
			bool					created;
			static	System::Mutex	mutex;
		public:
			/**/					TempFile(bool create=false);
			virtual					~TempFile();
			bool					Create();						//!< Creates the associated temporary file if not already created \return true on success
			bool					Destroy();						//!< Destroys the associated temporary file if created \return true on success
			const PathString&		GetLocation();					//!< Retrieves the absolute location of the temporary file. Also creates the associated file if not aready created \return Absolute file location
			operator const 			PathString& ();					//!< Implicit string conversation. Also creates the associated file if not aready created \return Absolute file location
		};



		/**
			\brief Drive handle

		*/
		class Drive
		{
		private:
				PathString		root,
								name;
				friend class	Folder;
				friend class	File;
				friend count_t	FileSystem::GetDriveList(Drive*target,count_t max);

		public:
				PathString		GetName()			const;
				PathString		GetRoot()			const;
				String			ToString()			const;
				File			ToFile()			const;
		};


		namespace FindFlags
		{
			enum flag_t
			{
				None = 0x0,
				MustExists = 0x1,
				MustBeFile = 0x2,
				MustBeDirectory = 0x4,
			};
		};

		/**
		Folder entry scanner.
		Allows to scan for files or folders in a given parent folder
		*/
		class FolderScan
		{
			PathString			path;
			File				file;
			bool				mustRewindManually = false;
			#if SYSTEM == WINDOWS
				HANDLE			find_handle = nullptr;
				WIN32_FIND_DATAW find_data;
			#elif SYSTEM == LINUX
				DIR				*find_handle = nullptr;
			#endif
			void				CloseScan();
		public:
			/**/				FolderScan() {};
			/**/				FolderScan(const File&f):path(f.GetLocation()){}
			/**/				FolderScan(const File*f):path(f ? f->GetLocation() : PathString()){}
			/**/				FolderScan(const Folder&f);
			/**/				FolderScan(const FolderScan&other):path(other.path){}
			virtual				~FolderScan() {CloseScan();}
			void				operator=(const FolderScan&other) {CloseScan(); path = other.path; mustRewindManually = false;}

			const PathString&	GetLocation()		const				/** Queries location. @return Local path */	{return path;}
			bool				IsValidLocation()	const;				//!< Queries location validity. @return true if the local FolderScan resides in a valid folder


			/**
			Checks if a rewind is necessary in order to retrieve more (the same) entries.
			@return true if Rewind() must be called before the next Next*() call can return anything
			*/
			bool				HasReachedEnd()	const	{return mustRewindManually;}

			/**
			Resets the folder cursor to the beginning of the folder.
			Must be called once any Next*() call has returned false, and another scan should be started
			Called automatically by the first Next*() call, but not at the end the current pass
			@return true if rewind was successful, false if the local folder doesn't exist (anymore)
			*/
			bool				Rewind();
			/**
			Returns the next entry in the local folder
			@param[out] outIsDirectory Set to true if the returned entry is a folder, false otherwise. Undefined if the method returns false
			@return Pointer to a file container pointing to the next entry in the local folder or NULL if no further entries exist.
				The returned object is owned by the local object, and updated by the next local pointer-returning Next*() call. Must not be deleted or otherwise kept
			*/
			const File*			NextEntry(bool&outIsDirectory);
			/**
			Describes the next entry in the local folder
			@param[out] out Target file structure to hold the next entry. Unchanged if the method returns false
			@param[out] outIsDirectory Set to true if the returned entry is a folder, false otherwise. Undefined if the method returns false
			@return true if a next entry exists, false if no further entries exist
			*/
			bool				NextEntry(File&out,bool&outIsDirectory);
			/**
			Returns the next entry in the local folder
			@return Pointer to a file container pointing to the next entry in the local folder, or NULL if no further entries exist.
				The returned object is owned by the local object, and updated by the next local pointer-returning Next*() call. Must not be deleted or otherwise kept
			*/
			const File*			NextEntry();
			/**
			Describes the next entry in the local folder
			@param[out] out Target file structure to hold the next entry. Unchanged if the method returns false
			@return true if a next entry exists, false if no further entries exist
			*/
			bool				NextEntry(File&out);
			bool				operator>>(File&out) /** @copydoc NextEntry(File&)*/ {return NextEntry(out);}
			/**
			Returns the next sub-folder of the local folder
			@return Pointer to a file container pointing to the next folder in the local folder, or NULL if no further sub-folders exist.
				The returned object is owned by the local object, and updated by the next local pointer-returning Next*() call. Must not be deleted or otherwise kept
			*/
			const File*			NextFolder();
			/**
			Describes the next sub-folder of the local folder
			@param[out] out Target to hold the next folder. Unchanged if the method returns false
			Wreturn true if a next folder exists, false if no further sub-folders exist
			*/
			bool				NextFolder(File&out);
			bool				NextFolder(Folder&out);						//!< @copydoc NextFolder(File&)
			bool				operator>>(Folder&out) /** @copydoc NextFolder(File&)*/ {return NextFolder(out);}
			/**
			Returns the next file in the local folder
			@return Pointer to a file container pointing to the next file in the local folder, or NULL if no further files exist.
				The returned object is owned by the local object, and updated by the next local pointer-returning Next*() call. Must not be deleted or otherwise kept
			*/
			const File*			NextFile();			
			/**
			Describes the next file in the local folder
			@param[out] out Target file structure to hold the next file. Unchanged if the method returns false
			@return true if a next file exists, false if no further files exist
			*/
			bool				NextFile(File&out);
			/**
			Returns the next file in the local folder matching the specified extension.
			Comparison is case insensitive. 
			@param extension Extension to match (without preceding dot)
			@return Pointer to a file container pointing to the next file matching the specified extension in the local folder, or NULL if no further files exist matching the extension.
				The returned object is owned by the local object, and updated by the next local pointer-returning Next*() call. Must not be deleted or otherwise kept
			*/
			const File*			NextFile(const PathString&extension);
			/**
			Describes the next file in the local folder matching the specified extension.
			Comparison is case insensitive. 
			@param extension Extension to match (without preceding dot)
			@param[out] out Target to hold the next matching file matching the given extension. Unchanged if the method returns false
			@return true if a next file of the specified extension exists, false if no further files exist
			*/
			bool				NextFile(const PathString&extension, File&out);	
		};

		/**
		\brief Folder handle
		*/
		class Folder 
		{
		public:
			typedef File	File;			//!< File entry
			typedef Drive	Drive;			//!< Drive entry
		private:
			PathString			absolute_folder;
			mutable File		findResult,locationFile;

			bool				locate(const PathString&path);
			bool				ResolvePathAndMoveTo(const PathString&path, bool moveToParent);
			bool				ResolvePath(const PathString&path, PathString*finalParent, PathString&final) const;
			static bool			ExitAbs(PathString&, bool check);
		public:



								Folder();
								Folder(const PathString&folder_string);
								Folder(const File&file);
								Folder(const File*file);
								Folder(const Folder&other);
			Folder&				operator=(const Folder&other);
			Folder&				operator=(const File&file);
			Folder&				operator=(const File*file);
			Folder&				operator=(const PathString&folder_string);
			bool				MoveTo(const PathString&folder_string);	//!< Leaves the current location and moves to the specified folder. If goTo fails then Folder is in an invalid location. \param folder_string Absolute or relative folder to move to \return true on success
			bool				MoveTo(const File*file);				//!< Leaves current location and moves to the specified folder. If goTo fails then Folder is in an invalid location. \param file Handle to the folder to move to \return true on success
			bool				MoveTo(const File&file);				//!< Leaves current location and moves to the specified folder. If goTo fails then Folder is in an invalid location. \param file Handle to the folder to move to \return true on success
			bool				Exit();									//!< Attempts to drop back to the next superior folder. If no such exists or it cannot be entered then Folder will return to the original state. \return true on success
			bool				Enter(const File&file);					//!< Attempts to enter the specified sub folder. If no such exists or it cannot be entered then Folder will restore the original state. \param file Handle of the respective sub folder \return true on success
			bool				Enter(const File*file);					//!< Attempts to enter the specified sub folder. If no such exists or it cannot be entered then Folder will restore the original state. \param file Handle of the respective sub folder \return true on success
			bool				Enter(const PathString&folder_name);		//!< Attempts to enter the specified sub folder. If no such exists or it cannot be entered then Folder will restore the original state. \param folder_name Name of the respective sub folder \return true on success
			const File*			ParentLocation()	const;				//!< Queries parent location. Returns NULL if folder is top-most folder \return Pointer to a handle pointing to the respective parent folder or NULL if no such exists
			const File*			Location()			const;				//!< Queries location. Returns NULL if the current location is invalid \return Pointer to a handle pointing to the current location or NULL if the current location is invalid
			const PathString&	LocationStr()		const;				//!< Queries location. \return Absolute location
			const PathString&	GetLocation()		const;				//!< Queries location. \return Absolute location
			bool				IsValidLocation()	const;				//!< Queries location validity. \return true if Folder resides in a valid folder

			FolderScan		Scan() const;

			const File*			FindEntry(const PathString&path, bool&outIsDirectory,bool&outExists,UINT32 findFlags)						const;
			bool				FindEntry(const PathString&path, File&out, bool&outIsDirectory,bool&outExists,UINT32 findFlags)			const;
			const File*			Find(const PathString&path,UINT32 findFlags = FindFlags::MustExists)						const;	//!< Locates the specified file or folder starting from the current folder location. \param path Path to the file or folder that should be located \return Pointer to a file container pointing to the specified entry or NULL if the entry could not be located
			bool				Find(const PathString&path, File&out,UINT32 findFlags = FindFlags::MustExists)			const;	//!< Locates the specified file or folder starting from the current folder location. \param path Path to the file or folder that should be located \param out Out File structure reference. \return true if the entry could be located
			const File*			FindFile(const PathString&path, bool mustExist=true)					const;	//!< Locates the specified file starting from the current folder location \param path Path to the file that should be located \return Pointer to a file container pointing to the specified file or NULL if the file could not be located
			bool				FindFile(const PathString&path, File&out, bool mustExist=true)		const;	//!< Locates the specified file starting from the current folder location. \param path Path to the file or folder that should be located \param out Out File structure reference. \return true if the entry could be located
			const File*			FindDirectory(const PathString&path, bool mustExist=true)				const;	//!< Locates the specified folder starting from the current folder location \param path Path to the folder that should be located \return Pointer to a file container pointing to the specified folder or NULL if the folder could not be located
			bool				FindDirectory(const PathString&path, File&out, bool mustExist=true)		const;	//!< Locates the specified folder starting from the current folder location. \param path Path to the file or folder that should be located \param out Out File structure reference. \return true if the entry could be located
			const File*			CreateDirectory(const PathString&name)				const;	//!< Creates the specified sub folder \param name Name of the sub folder to create \return Pointer to a file container pointing to the newly created folder or NULL if the folder could not be created
			bool				CreateDirectory(const PathString&name, File&out)	const;	//!< Creates the specified sub folder \param name Name of the sub folder to create \param out Out File structure reference. \return true if the new folder could be created, false otherwise.
			
			String				ToString()		const;						//!< Creates a string representation of the local folder
			
			bool				operator<(const Folder&other)	const;
			bool				operator>(const Folder&other)	const;
			bool				operator<=(const Folder&other)	const;
			bool				operator>=(const Folder&other)	const;
			bool				operator==(const Folder&other)	const;
			bool				operator!=(const Folder&other)	const;

		};


		typedef Folder	Directory;

	}

	typedef FileSystem::File		CFSFile;
	typedef FileSystem::Folder		CFSFolder;
	typedef FileSystem::Drive		CFSDrive;
	typedef FileSystem::TempFile	CFSTempFile;
}

#endif
