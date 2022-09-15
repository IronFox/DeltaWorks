#include "texture_font2.h"
#include <io/file_system.h>
#include <string/encoding.h>

/******************************************************************

OpenGL texture-font.

******************************************************************/

namespace Engine
{

	GLTextureFont2::GLTextureFont2():base(0),texture(0),scale(1.0f)
	{
		clear();
	}

	GLTextureFont2::~GLTextureFont2()
	{
		clear();
	}
	
	void				GLTextureFont2::clear()
	{
		if (base)
	        glDeleteLists(base,0x100);
		if (texture)
			glDeleteTextures(1,&texture);
		for (unsigned i = 0; i < ARRAYSIZE(chars); i++)
			chars[i].isset = false;
	}

	template <typename T>
		static bool			get(const XML::Node&node, const String&name, T&out)
		{
			String val;
			return node.Query(name,val) && convert(val.c_str(),out);
		}

	
	
	void				GLTextureFont2::LoadFromFile(const PathString&filename, float scale_)
	{
		XML::Container	container;
		container.LoadFromFile(filename);

		const XML::Node*xfont = container.Find("font");
		if (!xfont)
			throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML font file lacks <font> tag"));

		if (!get(*xfont,"scale",scale))
			throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML <font> attribute lacks 'scale' parameter"));
		scale*=scale_;
		String imagefile;
		if (!xfont->Query("image",imagefile))
			throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML <font> attribute lacks 'image' parameter"));
		FileSystem::Folder folder(FileSystem::ExtractFileDir(filename));
		Image image;
		FileSystem::File f;
		if (!folder.FindFile(FileSystem::StringToPath(imagefile),f,true))
		{
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"Font image file not found: "+imagefile);
		}
		Magic::LoadFromFile(image,  f.GetLocation());


		if (image.channels() != 4)
			throw Except::IO::DriveAccess::FileFormatFault(CLOCATION,"XML font file specifies unusable font image. Image must have 4 channels (RGBA) but has "+String(image.channels()));

		clear();

		for (UINT32 x = 0; x < image.GetWidth(); x++)
			for (UINT32 y = 0; y < image.GetHeight(); y++)
			{
				BYTE*pixel = image.get(x,y);
				if (pixel[3] == 0)
					pixel[0] = pixel[1] = pixel[2] = 0;
			}

	
		height = 0;
		unsigned lowest = UNSIGNED_UNDEF;
		
		for (index_t i = 0; i < xfont->children.Count(); i++)
		{
			const XML::Node&xchar = xfont->children[i];
			if (xchar.name != "char")
				continue;
			
			BYTE id;
			if (!get(xchar,"id",id))
				continue;
			unsigned left, right,bottom,top;
			if (!get(xchar,"left",left))
				continue;
			if (!get(xchar,"right",right))
				continue;
			if (!get(xchar,"bottom",bottom))
				continue;
			if (!get(xchar,"top",top))
				continue;
			chars[id].isset = true;
			chars[id].width = right-left+1;
			chars[id].height = top-bottom+1;
			chars[id].yoffset = bottom;
			if (bottom < lowest)
				lowest = bottom;
			if (chars[id].height > height)
				height = chars[id].height;
			chars[id].texcoords.Set(left,bottom,right,top);
			chars[id].texcoords.x /= (float)(image.GetWidth()-1);
			chars[id].texcoords.y /= (float)(image.GetHeight()-1);
		}
		for (unsigned i = 0; i < ARRAYSIZE(chars); i++)
			chars[i].yoffset -= lowest;
			
		//ShowMessage(String(lower_edge)+" . "+String(upper_edge));
	
		glGenTextures(1,&texture);

		if (!texture)
			throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL refuses to create texture handle (glGetError()='"+String(glGetError())+")");

		glBindTexture(GL_TEXTURE_2D,texture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		gluBuild2DMipmaps(GL_TEXTURE_2D,4,image.GetWidth(),image.GetHeight(),GL_RGBA,GL_UNSIGNED_BYTE,image.GetData());
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D,0);
		
		base = glGenLists(0x100);

		if (!base)
			throw Except::Renderer::GeometryTransfer::GeneralFault("OpenGL refuses to create display list (glGetError()='"+String(glGetError())+")");


		ASSERT__(base > 0);
		if (chars[(BYTE)'i'].isset && !chars[(BYTE)' '].isset)
		{
			TCharacter&space = chars[(BYTE)' '];
			space.isset = true;
			space.texcoords.SetMinAndMax(0);
			space.width = chars[(BYTE)'i'].width;
			space.height = 1;
			space.yoffset = 0;
		}
		
		for (index_t i = 0; i < ARRAYSIZE(chars); i++)
			if (chars[i].isset)
			{
				GLuint local = base+(BYTE)i;

	            glNewList(local,GL_COMPILE);
	            
	                glBegin(GL_QUADS);
	                    glTexCoord2f(chars[i].texcoords.x.min,chars[i].texcoords.y.min);   	glVertex2f(0,scale*chars[i].yoffset);
	                    glTexCoord2f(chars[i].texcoords.x.max,chars[i].texcoords.y.min);	glVertex2f(scale*chars[i].width,scale*chars[i].yoffset);
	                    glTexCoord2f(chars[i].texcoords.x.max,chars[i].texcoords.y.max);   	glVertex2f(scale*chars[i].width,scale*(chars[i].yoffset+chars[i].height));
	                    glTexCoord2f(chars[i].texcoords.x.min,chars[i].texcoords.y.max);   	glVertex2f(0,scale*(chars[i].yoffset+chars[i].height));
	                glEnd();
	                glTranslatef(scale*chars[i].width,0,0);
	            glEndList();
	        }
	}


	GLuint				GLTextureFont2::ToIndex(char32_t c) const
	{
		char c8;
		if (StringEncoding::UTF32::ToCP1252(c,c8))
		{
			GLuint rs = GLuint((BYTE)c8);
			if (chars[rs].isset)
				return rs;
		}
		return GLuint((BYTE)'?');
	}

	float GLTextureFont2::GetWidth(const StringRef&str)
	{
		float rs = 0;
		const char*at = str.pointer(),
				*const end = at + str.GetLength();
		while (at < end)
		{
			char32_t c;
			if (!StringEncoding::UTF8::ToUTF32(at,end,c))
				break;
			auto index = ToIndex(c);
			rs += chars[index].width;
		}
	    return rs*scale;
	}

	float GLTextureFont2::GetWidth(const UTF32StringRef&str)
	{
		float rs = 0;
		const auto*at = str.pointer(),
				*const end = at + str.GetLength();
		for (;at < end; at++)
		{
			auto index = ToIndex(*at);
			rs += chars[index].width;
		}
	    return rs*scale;
	}

	float GLTextureFont2::GetWidth(char c)
	{
	    return chars[(BYTE)c].isset?scale*chars[(BYTE)c].width:0;
	}

	void GLTextureFont2::Begin(const TFontState&state)
	{
	    glPushMatrix();
	        glTranslatef(state.left+state.indent,state.top,state.depth);
	        glTranslatef(0,-state.lineOffset,0);
	        glScalef(state.x_scale,state.y_scale,1);
	        glTranslatef(0,-1,0);
	        glColor4fv(state.v);
	        glPushAttrib(GL_LIST_BIT | GL_TEXTURE_BIT);
	    	glListBase(base);
	    	glEnable(GL_TEXTURE_2D);
	    	glBindTexture(GL_TEXTURE_2D,texture);
	}

	void GLTextureFont2::AlterColor(const TFontColor&color)
	{
	        glColor4fv(color.v);
	}


	void GLTextureFont2::Write(const StringRef&str)
	{
		const char*c = str.pointer(),
				*const end = c + str.GetLength();
		while (c < end)
		{
			char32_t c32;
			if (!StringEncoding::UTF8::ToUTF32(c,end,c32))
				break;
			auto idx = ToIndex(c32);
			glCallList(base + idx);
		}
	        //glCallLists(GLuint(len),GL_UNSIGNED_BYTE,str);
	}
	
	void GLTextureFont2::Write(const UTF32StringRef&str)
	{
		const auto*c = str.pointer(),
				*const end = c + str.GetLength();
		for (;c != end; ++c)
		{
			auto idx = ToIndex(*c);
			glCallList(base + idx);
		}
	}

	void GLTextureFont2::End()
	{
	        glPopAttrib();
	    glPopMatrix();

	}
}
