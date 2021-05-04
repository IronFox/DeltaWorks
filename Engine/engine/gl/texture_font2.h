#ifndef engine_gl_texture_font2H
#define engine_gl_texture_font2H

/******************************************************************

OpenGL texture-font.

******************************************************************/


#include "../textout.h"
#include "../../gl/gl.h"
#include "../../gl/glu.h"
#include <image/converter/magic.h>
#include <io/xml.h>

namespace Engine
{


	class GLTextureFont2	//! OpenGL texture font object usable in combination with the Textout class
	{
	protected:
			struct TCharacter
			{
				bool			isset;
				unsigned		width,
								height,
								yoffset;
				M::Rect<float>		texcoords;
			};
			
	        GLuint              base,		//!< Display list base used to store the final characters
								texture;	//!< OpenGL texture reference
			TCharacter			chars[0x100];
			float				height,
								scale;
								
			void				clear();
			GLuint				ToIndex(char32_t c) const;
	public:
	                            GLTextureFont2();
	virtual                    ~GLTextureFont2();
			void				LoadFromFile(const PathString&filename, float scale=1.0f);	//!< Loads the local font configuration from the specified XML file.

	        float               GetWidth(const StringRef&str);	//!< Calculates the unscaled width of the specified string. For EveGLTextureFont this is identical to the length of the string. \param str String to determine the length of \param len Number of characters to stop after \return Unscaled length of the string
	        float               GetWidth(const UTF32StringRef&str);	//!< Calculates the unscaled width of the specified string. For EveGLTextureFont this is identical to the length of the string. \param str String to determine the length of \param len Number of characters to stop after \return Unscaled length of the string
	        float               GetWidth(char c);						//!< Calculates the unscaled width of a single character. For EveGLTextureFont this is always 1. \param c Character to determine the width of \return Unscaled length of the character
			float				GetHeight()	{return scale*height;}
	        void                Begin(const TFontState&state);		//!< Begins text output starting with the specified state \param state Status to begin with
	        void                AlterColor(const TFontColor&color);	//!< Changes the active color while between begin() and end(). \param color Color to use from here on
	        void                Write(const StringRef&str);	//!< Print the specified string. Newline characters are overridden  \param str String to print \param len Number of characters to print of \b str
	        void                Write(const UTF32StringRef&str);	//!< Print the specified string. Newline characters are overridden  \param str String to print \param len Number of characters to print of \b str
	        void                End();									//!< Ends text output
	};
}

#endif

