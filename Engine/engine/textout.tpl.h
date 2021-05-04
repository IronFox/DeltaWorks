#ifndef engine_textoutTplH
#define engine_textoutTplH

/******************************************************************

Font-dependent textout-manager. The appearance of the
resulting render depends on the used font.

******************************************************************/


namespace Engine
{

	template <class Font>
		void		Textout<Font>::BeginOutput()
		{
			activeFont->Begin(state);
		}
		
	template <class Font>
		void		Textout<Font>::WriteSegment(const char*field, size_t len)
		{
			activeFont->Write(StringRef(field,len));
		}
		
	template <class Font>
		void		Textout<Font>::WriteSegment(const char32_t*field, size_t len)
		{
			activeFont->Write(UTF32StringRef(field,len));
		}
		
	template <class Font>
		void		Textout<Font>::EndOutput()
		{
			activeFont->End();
		}
		
	template <class Font>
		void		Textout<Font>::AlterColor(const TFontColor&color)
		{
			activeFont->AlterColor(color);
		}
		

	template <class Font> Textout<Font>::Textout():activeFont(&localFont)
	{}


	
	
	
	
	

	template <class Font> void Textout<Font>::SetFont(Font*font)
	{
	    if (font)
	        activeFont = font;
	    else
	        activeFont = &localFont;
	}
	
	
	

	template <class Font> Font& Textout<Font>::GetFont()
	{
	    return localFont;
	}

	template <class Font> Font* Textout<Font>::GetActiveFont()
	{
	    return activeFont;
	}

	template <class Font> float Textout<Font>::GetScaledWidth(const StringRef&line) const
	{
	    return activeFont->GetWidth(line)*state.x_scale;
	}

	template <class Font> float Textout<Font>::GetScaledWidth(const UTF32StringRef&line) const
	{
	    return activeFont->GetWidth(line)*state.x_scale;
	}
		
	template <class Font> /*virtual override*/ float Textout<Font>::QueryUnscaledWidth(const StringRef&line) const
	{
	    return activeFont->GetWidth(line);
	}
		
	template <class Font> /*virtual override*/ float Textout<Font>::QueryUnscaledWidth(const UTF32StringRef&line) const
	{
	    return activeFont->GetWidth(line);
	}

	template <class Font> /*virtual override*/ float Textout<Font>::QueryUnscaledHeight() const
	{
		return activeFont->GetHeight();
	}

	template <class Font> float Textout<Font>::GetUnscaledWidth(const StringRef&line) const
	{
	    return activeFont->GetWidth(line);
	}

	template <class Font> float Textout<Font>::GetUnscaledWidth(const UTF32StringRef&line) const
	{
	    return activeFont->GetWidth(line);
	}

	template <class Font>
		void Textout<Font>::BeginNewLine()
		{
			state.indent = 0;
			state.lineOffset += GetScaledHeight();
		}

	
	
	template <class Font>
		void	Textout<Font>::Print(const char*str, size_t len)
		{
			Stream(str,len);
		}

	template <class Font> void Textout<Font>::Print(const char*str)
	{
		Stream(str,strlen(str));
	}

	template <class Font>
		template <typename Char>
		void	Textout<Font>::Stream(const Char*str, size_t len)
		{
			if (!len)
				return;
		    activeFont->Begin(state);
		    const Char*at = str,*end=str+len;
		    while (at<end)
		    {
		        size_t len(0);
		        while (at+len<end && at[len] != '\n')
		            len++;
				const StringType::ReferenceExpression<Char> toWrite(at,len);
		        activeFont->Write(toWrite);
				if (at+len >= end)
				{
					state.indent += GetScaledWidth(toWrite);
				}
		        at+=len;
		        if (at<end)
		        {
		            activeFont->End();
					BeginNewLine();
		            activeFont->Begin(state);
		            at++;
		        }
		    }
		    activeFont->End();
		}

	template <class Font> void Textout<Font>::Print(const String&str)
	{
	    Print(str.c_str(),str.GetLength());
	}
	template <class Font> void Textout<Font>::Print(const StringRef&str)
	{
	    Print(str.pointer(),str.GetLength());
	}
	template <class Font> void Textout<Font>::Print(const UTF32StringRef&str)
	{
		Print(str.pointer(),str.GetLength());
	}

	template <class Font> void Textout<Font>::PrintTagged(const char*str, char tag)
	{
	    if (!str)
	        return;
	    TFontColor 	color_state = state;
	    TFontColor	color_stack[0x100];
	    BYTE        color_depth(0);

	    activeFont->Begin(state);
	    const char*at = str;
	    while (*at)
	    {
	        size_t l(0);
	        while (at[l] && at[l] != '\n' && at[l] != tag)
	            l++;
	        activeFont->Write(at,l);
	        at+=l;
	        if ((*at) == '\n')
	        {
	            activeFont->End();
				BeginNewLine();
	            activeFont->Begin(state);
	            activeFont->AlterColor(color_state);
	            at++;
	        }
	        else
	            if ((*at) == tag)
	            {
	                at++;
	                size_t remaining = strlen(at);
	                if (remaining > 4 && !strncmpi(at,"push",4))
	                {
	                    color_stack[color_depth++] = color_state;
	                    at+=4;
	                }
	                else
	                    if (remaining > 3 && !strncmpi(at,"pop",3))
	                    {
	                        color_state = color_stack[--color_depth];
	                        activeFont->AlterColor(color_state);
	                        at+=3;
	                    }
	                    else
	                        if (remaining > 8)
	                        {
	                            color_state.red = (float)hexStr(at)/255;
	                            color_state.green = (float)hexStr(at+2)/255;
	                            color_state.blue = (float)hexStr(at+4)/255;
	                            color_state.alpha = (float)hexStr(at+6)/255;
	                            at+=8;
	                            activeFont->AlterColor(color_state);
	                        }
	            }
	    }
	    activeFont->End();
	}

	template <class Font> void Textout<Font>::PrintTagged(const String&str, char tag)
	{
	    PrintTagged(str.c_str(),tag);
	}


	template <class Font> void Textout<Font>::PrintLn(const char*str)
	{
		Print(str);
		BeginNewLine();
	}

	template <class Font> void Textout<Font>::PrintLn(const String&str)
	{
		Print(str);
		BeginNewLine();
	}
	template <class Font> void Textout<Font>::PrintLn(const StringRef&str)
	{
		Print(str);
		BeginNewLine();
	}

	template <class Font> void Textout<Font>::PrintTaggedLine(const char*str, char tag)
	{
		PrintTagged(str,tag);
		BeginNewLine();
	}

	template <class Font> void Textout<Font>::PrintTaggedLine(const String&str, char tag)
	{
		PrintTagged(str,tag);
		BeginNewLine();
	}



	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const String&str)
		{
			Stream(str.c_str(),str.GetLength());
			return *this;
		}
	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const StringRef&str)
		{
			Stream(str.pointer(),str.GetLength());
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const UTF32String&str)
		{
			Stream(str.c_str(),str.GetLength());
			return *this;
		}
	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const UTF32StringRef&str)
		{
			Stream(str.pointer(),str.GetLength());
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const char*str)
		{
			Stream(str,strlen(str));
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const TNewLine&)
		{
			BeginNewLine();
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const TSpace&space)
		{
			if (!space.length)
				return *this;
			static const char blank = ' ';
			state.indent += GetScaledWidth(blank)*space.length;
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(const TTabSpace&space)
		{
			if (!space.length)
				return *this;
			static const char blank = ' ';
			float tab_width = GetScaledWidth(blank)*4;
			state.indent = (ceil(state.indent/tab_width)+space.length-1)*tab_width;
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(char c)
		{
			if (c == '\n')
			{
				BeginNewLine();
				return *this;
			}
		    activeFont->Begin(state);
		        activeFont->Write(StringRef(&c,1));
		    activeFont->End();
			state.indent += GetScaledWidth(c);
			return *this;
		}
	

	template <class Font>
		template <typename T>	
			void Textout<Font>::StreamUnsigned(T value)
			{
				char	char_buffer[256],
						*End = char_buffer+ARRAYSIZE(char_buffer),
						*c = End;
				while (value && c != char_buffer)
				{
					(*(--c)) = (char)('0'+(value%10));
					value/=10;
				}
				if (c==End)
					(*(--c)) = (char)'0';

				const StringRef toWrite(c,End-c);
				activeFont->Begin(state);
					activeFont->Write(toWrite);
				activeFont->End();
				state.indent += GetScaledWidth(toWrite);
			}

	template <class Font>
		template <typename T, typename UT>
			void Textout<Font>::StreamSigned(T value)
			{
				bool negative = false;
				if (value < 0)
				{
					value*=-1;
					negative = true;
				}

				char	char_buffer[256],
						*End = char_buffer+ARRAYSIZE(char_buffer),
						*c = End;
				UT uval = value;
				while (uval && c != char_buffer)
				{
					(*(--c)) = (char)('0'+(uval%10));
					uval/=10;
				}
				if (c==End)
					(*(--c)) = (char)'0';

				const StringRef toWrite(c,End-c);
				activeFont->Begin(state);
					if (negative)
						activeFont->Write(StringRef("-",1));
					activeFont->Write(toWrite);
				activeFont->End();
				state.indent += GetScaledWidth(toWrite);
				if (negative)
					state.indent += GetScaledWidth('-');
			}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(BYTE b)
		{
			StreamUnsigned(b);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(long long ll)
		{
			StreamSigned<long long, unsigned long long>(ll);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(unsigned long long ll)
		{
			StreamUnsigned(ll);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(int i)
		{
			StreamSigned<int,unsigned>(i);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(unsigned u)
		{
			StreamUnsigned(u);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(long l)
		{
			StreamSigned<long, unsigned long>(l);
			return *this;
		}
		
	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(unsigned long l)
		{
			StreamUnsigned(l);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(short s)
		{
			StreamSigned<short,unsigned short>(s);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(unsigned short s)
		{
			StreamUnsigned(s);
			return *this;
		}

	template <class Font>
		template <typename T>
			void Textout<Font>::StreamFloat(T value, unsigned precision)
			{
				char	char_buffer[256],
						*str = String::FloatToStr(value, precision, false, char_buffer+ARRAYSIZE(char_buffer), char_buffer);
				const StringRef toWrite(str,char_buffer+ARRAYSIZE(char_buffer)-str);
				activeFont->Begin(state);
					activeFont->Write(toWrite);
				activeFont->End();
				state.indent += GetScaledWidth(toWrite);
			}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(float f)
		{
			StreamFloat(f,5);
			return *this;
		}

	template <class Font>
		Textout<Font>&	Textout<Font>::operator<<(double d)
		{
			StreamFloat(d,8);
			return *this;
		}




	template <typename Char>
		void		VirtualTextout::Write(const StringType::ReferenceExpression<Char>&str)
		{
			if (str.GetLength()==0)
				return;
			BeginOutput();
			const Char*at = str.pointer(),
					*const end=at+str.GetLength();
			while (at<end)
			{
				size_t len(0);
				while (at+len<end && at[len] != '\n')
					len++;
				WriteSegment(at,len);
				at+=len;
				if (at<end)
				{
					state.lineOffset += QueryScaledHeight();
					EndOutput();
					BeginOutput();
					at++;
				}
			}
			EndOutput();
		}

	template <typename Char>
		void		VirtualTextout::WriteTagged(const StringType::ReferenceExpression<Char>&str, Char tag)
		{
			if (!str)
				return;
			TFontColor 	color_state = state;
			TFontColor	color_stack[0x100];
			BYTE        color_depth(0);

			BeginOutput();
			const Char*at = str;
			while (*at)
			{
				size_t l(0);
				while (at[l] && at[l] != '\n' && at[l] != tag)
					l++;
				WriteSegment(at,l);
				at+=l;
				if ((*at) == '\n')
				{
					state.lineOffset += QueryScaledHeight();
					EndOutput();
					BeginOutput();
					AlterColor(color_state);
					at++;
				}
				else
					if ((*at) == tag)
					{
						at++;
						size_t remaining = strlen(at);
						if (remaining > 4 && !strncmpi(at,"push",4))
						{
							color_stack[color_depth++] = color_state;
							at+=4;
						}
						else
							if (remaining > 3 && !strncmpi(at,"pop",3))
							{
								color_state = color_stack[--color_depth];
								AlterColor(color_state);
								at+=3;
							}
							else
								if (remaining > 8)
								{
									color_state.red = (float)hexStr(at)/255;
									color_state.green = (float)hexStr(at+2)/255;
									color_state.blue = (float)hexStr(at+4)/255;
									color_state.alpha = (float)hexStr(at+6)/255;
									at+=8;
									AlterColor(color_state);
								}
					}
			}
			EndOutput();
		}


}

#endif

