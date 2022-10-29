#include "glyph_font.h"
#include <io/file_system.h>
#include <string/encoding.h>

/******************************************************************

OpenGL texture-font.

******************************************************************/

namespace Engine
{

	void GLGlyphFont::SetFont(float scale, FontToImage::Font&& font)
	{
		this->font = std::move(font);
		this->scale = scale;
		ClearCache();
		Init();
	}

	void GLGlyphFont::Init()
	{
		const auto& x = Resolve('X');
		height = x.height * 1.2f;
		spaceWidth = Resolve('I').width;
	}

	GLGlyphFont::GLGlyphFont() :scale(1.0f)
	{}


	GLGlyphFont::GLGlyphFont(float scale, FontToImage::Font&& font) :scale(scale), font(std::move(font))
	{
		Init();
	}

	GLGlyphFont::~GLGlyphFont()
	{
		ClearCache();
	}
	
	void				GLGlyphFont::ClearCache()
	{
		cached.VisitAllValues([](auto& v)
			{
				if (v.texture)
					glDeleteTextures(1,&v.texture);
				glDeleteLists(v.renderList, 1);
			});
		cached.Clear();
	}

	
	
	const GLGlyphFont::TCharacter&				GLGlyphFont::Resolve(char32_t c)
	{
		auto ptr = cached.QueryPointer(c);
		if (ptr)
			return *ptr;
		auto&nchr = cached.Set(c);

		if (c == ' ')
		{
			nchr.width = spaceWidth;
			nchr.height = height;
			nchr.yoffset = 0;
			nchr.texture = 0;


			nchr.renderList = glGenLists(1);


			glNewList(nchr.renderList, GL_COMPILE);
				glTranslatef(scale * nchr.width, 0, 0);
			glEndList();

			return nchr;
		}



		int x,y;
		Image image;
		font.Rasterize(c, 1,image, x, y);
		nchr.yoffset = -y;

		nchr.width = image.GetWidth();
		nchr.height = image.GetHeight();

		glGenTextures(1, &nchr.texture);

		if (!nchr.texture)
			throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL refuses to create texture handle (glGetError()='" + String(glGetError()) + ")");

		glBindTexture(GL_TEXTURE_2D, nchr.texture);
		//M::TVec4<> black{ 0,0,0,0 };
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black.v);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA8, image.GetWidth(), image.GetHeight(), GL_ALPHA, GL_UNSIGNED_BYTE, image.GetData());
		
		//glTexImage2D(GL_TEXTURE_2D, 0, 1, image.GetWidth(), image.GetHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, image.GetData());
		//image.ScaleHalf();
		//glTexImage2D(GL_TEXTURE_2D, 1, 1, image.GetWidth(), image.GetHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, image.GetData());
		//image.ScaleHalf();
		//glTexImage2D(GL_TEXTURE_2D, 2, 1, image.GetWidth(), image.GetHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, image.GetData());
		//glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);


		nchr.renderList = glGenLists(1);


		glNewList(nchr.renderList, GL_COMPILE);
			glBindTexture(GL_TEXTURE_2D, nchr.texture);
			glBegin(GL_QUADS);
				glTexCoord2f(0, 0);   	glVertex2f(0, scale * nchr.yoffset);
				glTexCoord2f(1, 0);		glVertex2f(scale * nchr.width, scale * nchr.yoffset);
				glTexCoord2f(1, 1);   	glVertex2f(scale * nchr.width, scale * (nchr.yoffset + nchr.height));
				glTexCoord2f(0, 1);   	glVertex2f(0, scale * (nchr.yoffset + nchr.height));
			glEnd();
			glTranslatef(scale * nchr.width, 0, 0);
		glEndList();

		return nchr;
	}

	float GLGlyphFont::GetWidth(const StringRef&str)
	{
		float rs = 0;
		const char*at = str.pointer(),
				*const end = at + str.GetLength();
		while (at < end)
		{
			char32_t c;
			if (!StringEncoding::UTF8::ToUTF32(at,end,c))
				break;
			const auto&ch = Resolve(c);
			rs += ch.width;
		}
	    return rs*scale;
	}

	float GLGlyphFont::GetWidth(const UTF32StringRef&str)
	{
		float rs = 0;
		const auto*at = str.pointer(),
				*const end = at + str.GetLength();
		for (;at < end; at++)
		{
			const auto& ch = Resolve(*at);
			rs += ch.width;
		}
	    return rs*scale;
	}

	float GLGlyphFont::GetWidth(char c)
	{
		const auto& ch = Resolve(c);
		return ch.width * scale;
	}

	void GLGlyphFont::Begin(const TFontState&state)
	{
	    glPushMatrix();
			glPushAttrib(GL_LIST_BIT | GL_TEXTURE_BIT);
			glTranslatef(state.left+state.indent,state.top,state.depth);
	        glTranslatef(0,-state.lineOffset,0);
	        glScalef(state.x_scale,state.y_scale,1);
	        glTranslatef(0,-1,0);
	        glColor4fv(state.v);
	    	glEnable(GL_TEXTURE_2D);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void GLGlyphFont::AlterColor(const TFontColor&color)
	{
	        glColor4fv(color.v);
	}


	void GLGlyphFont::Write(const StringRef&str)
	{
		const char*c = str.pointer(),
				*const end = c + str.GetLength();
		while (c < end)
		{
			char32_t c32;
			if (!StringEncoding::UTF8::ToUTF32(c,end,c32))
				break;
			const auto& chr = Resolve(c32);
			glCallList(chr.renderList);

		}
	        //glCallLists(GLuint(len),GL_UNSIGNED_BYTE,str);
	}
	
	void GLGlyphFont::Write(const UTF32StringRef&str)
	{
		const auto*c = str.pointer(),
				*const end = c + str.GetLength();
		for (;c != end; ++c)
		{
			const auto& chr = Resolve(*c);
			glCallList(chr.renderList);
		}
	}

	void GLGlyphFont::End()
	{
	        glPopAttrib();
	    glPopMatrix();

	}
}
