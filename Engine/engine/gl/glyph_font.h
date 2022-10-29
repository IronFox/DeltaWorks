#pragma once
/******************************************************************

OpenGL glyph-font.

******************************************************************/


#include "../textout.h"
#include "../../gl/gl.h"
#include "../../gl/glu.h"
#include <image/font_to_image.h>
#include <io/xml.h>

#include <container/hashtable.h>

namespace Engine
{


	class GLGlyphFont
	{
	protected:
		struct TCharacter
		{
			unsigned			width,
								height;
			int					yoffset;
			GLuint				texture;
			GLuint				renderList;
		};
		HashTable<char32_t, TCharacter> cached;
		float					height,
								scale,
								spaceWidth;
								
		FontToImage::Font		font;

		void					Init();
		void					ClearCache();
		const TCharacter&		Resolve(char32_t c);
	public:
		/**/					GLGlyphFont();
		/**/					GLGlyphFont(float scale, FontToImage::Font&&font);

		void					SetFont(float scale, FontToImage::Font&& font);

		virtual                 ~GLGlyphFont();

	    float					GetWidth(const StringRef&str);	//!< Calculates the unscaled width of the specified string. For EveGLTextureFont this is identical to the length of the string. \param str String to determine the length of \param len Number of characters to stop after \return Unscaled length of the string
	    float					GetWidth(const UTF32StringRef&str);	//!< Calculates the unscaled width of the specified string. For EveGLTextureFont this is identical to the length of the string. \param str String to determine the length of \param len Number of characters to stop after \return Unscaled length of the string
	    float					GetWidth(char c);						//!< Calculates the unscaled width of a single character. For EveGLTextureFont this is always 1. \param c Character to determine the width of \return Unscaled length of the character
		float					GetHeight()	{return scale*height;}
	    void					Begin(const TFontState&state);		//!< Begins text output starting with the specified state \param state Status to begin with
	    void					AlterColor(const TFontColor&color);	//!< Changes the active color while between begin() and end(). \param color Color to use from here on
	    void					Write(const StringRef&str);	//!< Print the specified string. Newline characters are overridden  \param str String to print \param len Number of characters to print of \b str
	    void					Write(const UTF32StringRef&str);	//!< Print the specified string. Newline characters are overridden  \param str String to print \param len Number of characters to print of \b str
	    void					End();									//!< Ends text output
	};
}

