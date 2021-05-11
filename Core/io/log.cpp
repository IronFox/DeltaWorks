#include "../global_root.h"
#include "log.h"


namespace DeltaWorks
{

	TLogIndent  indent(BYTE characters)
	{
		TLogIndent  rs;
		rs.length = characters;
		return rs;
	}

	char LogFile::space_buffer[0x100];


	LogFile::LogFile():f(NULL),format("[%Y %B %d. %H:%M:%S] "),indentation(0),indent_(false),closeWhenIdle(false)
	{}

	LogFile::LogFile(const PathString&filename, bool makeclear, bool closeWhenIdle):f(NULL),do_open(true),last(filename),format("[%Y %B %d. %H:%M:%S] "),indentation(0),indent_(false),make_clear(makeclear),closeWhenIdle(closeWhenIdle)
	{
		memset(space_buffer,' ',sizeof(space_buffer));
	}

	LogFile::~LogFile()
	{
		close();
	}

	bool LogFile::begin()
	{
		if (f)
			return true;
		if (!do_open)
			return false;
		if (make_clear)
			f = FOPEN(last.c_str(),"wb");
		else
			f = FOPEN(last.c_str(),"ab");
		do_open = false;
		return f != NULL;
	}

	bool LogFile::Open(const PathString&filename, bool makeclear, bool closeWhenIdle/*=true*/)
	{
		Close();
		this->closeWhenIdle = closeWhenIdle;
		last = filename;
		do_open = true;
		make_clear = makeclear;
		return true;
	}

	bool LogFile::Create(const PathString&filename, bool closeWhenIdle/*=true*/)
	{
		return Open(filename,true,closeWhenIdle);
	}

	bool LogFile::isActive()
	{
		return f!=NULL || do_open;
	}

	void LogFile::close()
	{
		if (f)
			fclose(f);
		f = NULL;
		do_open = false;
	}
    


	void LogFile::clear()
	{
		close();
		Open(last,true,false);
	}

	void LogFile::timeStamp()
	{
		if (!begin())
			return;
		time_t tt = time(NULL);
		const tm*t = localtime(&tt);
		char output[256];
		size_t len = strftime(output,sizeof(output),format.c_str(),t);
		ASSERT__(fwrite(output,1,len,f)==len);
	}



	void LogFile::end()
	{
		if (closeWhenIdle)
		{
			fclose(f);
			f = nullptr;
			do_open = true;
			make_clear = false;
		}
		else
			fflush(f);
	}

	bool LogFile::WriteIndent()
	{
		if (indent_)
		{
			for (unsigned i = 0; i < indentation; i++)
			{
				static const char c = '\t';
				if (fwrite(&c,1,1,f)!=1)
				{
					end();
					return false;
				}
			}
			indent_ = false;
		}
		return true;
	}

	bool LogFile::Log(const char*line, bool time_)
	{
		if (!begin() || !WriteIndent())
			return false;

		if (time_)
			timeStamp();
		const char*nl = strchr(line,'\n');
		if (nl)
		{
			if (fwrite(line,1,nl-line+1,f)!=nl-line+1)
			{
				end();
				return false;
			}
			indent_ = true;
			if (!strlen(nl+1))
			{
				end();
				return true;
			}
			return Log(nl+1,false);
		}
		size_t len = strlen(line);
		if (fwrite(line,1,len,f)!=len)
		{
			end();
			return false;
		}
		char nl_ = '\n';
		if (fwrite(&nl_,1,1,f)!=1)
		{
			end();
			return false;
		}
		end();
		return true;
	}


	bool LogFile::Log(const String&line, bool time_)
	{
		if (!begin())
			return false;
		if (indent_)
		{
			for (unsigned i = 0; i < indentation; i++)
			{
				static const char c = '\t';
				if (fwrite(&c,1,1,f)!=1)
				{
					end();
					return false;
				}
			}
			indent_ = false;
		}
		if (time_)
			timeStamp();
		index_t at;
		if ((at = line.Find('\n'))!= InvalidIndex)
		{
			at++;
			if (fwrite(line.c_str(),1,at,f)!=at)
			{
				end();
				return false;
			}
			indent_ = true;
			if (at == line.GetLength())
			{
				end();
				return true;
			}
			return Log(line.c_str()+at,false);
		}
		if (fwrite(line.c_str(),1,line.GetLength(),f)!=line.GetLength())
		{
			end();
			return false;
		}
		char nl_ = '\n';
		if (fwrite(&nl_,1,1,f)!=1)
		{
			end();
			return false;
		}
		end();
		return true;
	}

	LogFile& LogFile::operator<<(const TEndLine&)
	{
		return operator<<(nl);
	}

	LogFile& LogFile::operator<<(const TNewLine&)
	{
		if (!begin())
			return *this;
		static const char c = '\n';
		size_t written = fwrite(&c,1,1,f);
	
		end();
		indent_ = true;
	
		return *this;
	}

	LogFile& LogFile::operator<<(const char*line)
	{
		Log(line);
		return *this;
	}

	LogFile& LogFile::operator<<(const String&line)
	{
		Log(line);
		return *this;
	}

	LogFile& LogFile::operator<<(const TLogIndent&indent)
	{
		if (!begin())
			return *this;
		size_t written = fwrite(space_buffer,1,indent.length,f);
		end();
		indent_ = false;
		return *this;
	}

	void	LogFile::Indent()
	{
		indentation++;
	}

	void	LogFile::Outdent()
	{
		if (indentation)
			indentation--;
	}

	LogFile&		LogFile::operator<<(short v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%d",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(unsigned short v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%u",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(int v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%d",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(unsigned int v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%u",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(long long v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%lld",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(unsigned long long v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%llu",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(long v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%ld",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(unsigned long v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%lu",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(float v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%f.3",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(double v)
	{
		if (!begin() || !WriteIndent())
			return *this;
		Indent();
		fprintf(f,"%f.8",v);
		end();
		return *this;
	}

	LogFile&		LogFile::operator<<(bool v)
	{
		return operator<<(v ? "true":"false");
	}

        
}

