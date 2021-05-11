#include "../../global_root.h"
#include "magic.h"


/******************************************************************

Auto-Format handler.

******************************************************************/

namespace DeltaWorks
{

	ImageFormat*	Magic::format[] = {&bitmap,&jpeg,&png,&tga};

	void			Magic::GetExtensions(Ctr::ArrayData<PathString>&extensions)
	{
		count_t cnt = 0;
		for (index_t i = 0; i < ARRAYSIZE(format); i++)
			cnt += format[i]->extensions.Count();
		extensions.SetSize(cnt);
		PathString*to = extensions.pointer();
		for (index_t i = 0; i < ARRAYSIZE(format); i++)
		{
			for (index_t j = 0; j < format[i]->extensions.Count(); j++)
				(*to++) = format[i]->extensions[j];
		}
		ASSERT_CONCLUSION(extensions,to);
	}

	void			Magic::GetExtensions(Ctr::ArrayData<PathString>&extensions, Ctr::ArrayData<ImageFormat*>&formats)
	{
		count_t cnt = 0;
		for (index_t i = 0; i < ARRAYSIZE(format); i++)
			cnt += format[i]->extensions.Count();
		extensions.SetSize(cnt);
		formats.SetSize(cnt);
		PathString*sto = extensions.pointer();
		ImageFormat**fto = formats.pointer();
	
		for (index_t i = 0; i < ARRAYSIZE(format); i++)
		{
			for (index_t j = 0; j < format[i]->extensions.Count(); j++)
			{
				(*sto++) = format[i]->extensions[j];
				(*fto++) = format[i];
			}
		}
		ASSERT_CONCLUSION(extensions,sto);
		ASSERT_CONCLUSION(formats,fto);
	}

	static const PathString::char_t* extractExt(const PathString&name)
	{
		index_t offset = name.GetLength();
		const PathString::char_t*cstr = name.c_str();
		while (--offset && cstr[offset] != '.');
		if (offset)
			return cstr + (offset+1);
		return NULL;
	}


	void		Magic::LoadFromFile(Image&target, const PathString&filename)
	{
		LoadFromFile(target,filename,NULL);
	}

	void        Magic::LoadFromFile(Image&target, const PathString&filename, const PathString::char_t*file_ext)
	{
		if (!file_ext)
			file_ext = extractExt(filename);

		BYTE	magic_bytes[10];
		FILE*file = FOPEN(filename.c_str(),"rb");
		if (!file)
		{
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"File not found or inaccessible: '"+FileSystem::PathToString(filename)+"'");
			return;
		}


		size_t actual_bytes = fread(magic_bytes,1,10,file);
		FILE_READ_ASSERT_ZERO__(fseek(file,0,SEEK_SET));

		/*cout << "interpreting image bytes";
		for (index_t i = 0; i < actual_bytes; i++)
			cout << " 0x"<<std::hex<<(int)magic_bytes[i];
		cout << "..."<<endl;*/

		for (index_t i = 0; i < ARRAYSIZE(format); i++)
		{
			ImageFormat*format = Magic::format[i];
			if (format->magic_bytes.IsEmpty() || format->magic_bytes.Count()>actual_bytes)
				continue;	//process later or ignore
			if (memcmp(format->magic_bytes.pointer(),magic_bytes,format->magic_bytes.Count())!=0)
				continue;
			//cout << " found direct match #"<<i<<" '"<<format->name<<"'"<<endl;
			try
			{
				format->LoadFromFilePointer(target,file);
				fclose(file);
				return;
			}
			catch (...)
			{
				fclose(file);
				throw;
			}
		}
		//cout << " no direct match found. comparing extensions"<<endl;

		for (index_t i = 0; i < ARRAYSIZE(format); i++)
		{
			ImageFormat*format = Magic::format[i];
			if (file_ext)
			{
				if (!format->Supports(file_ext))
					continue;
				//cout << " found extension match #"<<i<<" '"<<format->name<<"'"<<endl;
			}
			else
			{
				if (!format->magic_bytes.IsEmpty())
					continue;
				//cout << " trying format #"<<i<<" '"<<format->name<<"'"<<endl;
			}
			try
			{
				format->LoadFromFilePointer(target,file);
				fclose(file);
				return;
			}
			catch (...)
			{
				fclose(file);
				throw;
			}
		}

		fclose(file);
		throw Except::Program::UnsupportedRequest(CLOCATION,"'"+ FileSystem::PathToString(filename)+"': Unable to find matching image format");
	}

	void			Magic::SaveToFile(const Image&image, const PathString&filename)
	{
		return SaveToFile(image,filename,NULL);
	}

	void        	Magic::SaveToFile(const Image&image, const PathString&filename, const PathString::char_t*file_ext)
	{
		if (!file_ext)
			file_ext = extractExt(filename);

		if (file_ext)
		{
			for (index_t i = 0; i < ARRAYSIZE(format); i++)
				if (format[i]->Supports(file_ext))
				{
					format[i]->SaveToFile(image,filename);
					return;
				}
		}
		throw Except::Program::UnsupportedRequest(CLOCATION,"Extension not supported: '"+String(file_ext)+"'");
	}
}

