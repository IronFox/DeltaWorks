#include "extensions.h"

/******************************************************************

OpenGL-Extension-manager. The listed bits specify extensions
whose functions are derived in groups.


******************************************************************/

	bool						Engine::GLShader::Instance::forgetOnDestruct=false;

	#ifdef INSTALLED_SHADER_INSTANCE
		const Engine::GLShader::Instance	*Engine::GLShader::Instance::installedInstance(NULL);
	#endif
	Engine::GLShader::Template::RenderConfiguration		Engine::GLShader::Template::globalRenderConfig;

	DeltaWorks::Ctr::Buffer<Engine::GLShader::Template::LightShadowAttachment,0,DeltaWorks::Swap>		Engine::GLShader::Template::shadowAttachments;


	#define EXT_CONTEXT(name)					{group	= #name;}
	#define EXT_GET_EXTENSION(var)				Extension::Read(var,String(#var)+group)
	#define EXT_GET_EXTENSION_NO_CHECK(var)		Extension::ReadSilent(var,String(#var)+group)

	#undef glBlendFuncSeparate
	#undef glGenBuffers
	#undef glBindBuffer
	#undef glBufferData
	#undef glDeleteBuffers


	//namespace Ctr = DeltaWorks::Ctr;

	namespace OpenGLExtPointer
	{
	#ifdef GL_VERSION_2_0
		PFNGLDRAWBUFFERSPROC			glDrawBuffers = NULL;
	#endif

	#ifdef GL_ARB_multitexture
		PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1f	= NULL;
		PFNGLMULTITEXCOORD1FVARBPROC	glMultiTexCoord1fv	= NULL;
		PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2f	= NULL;
		PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fv	= NULL;
		PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3f	= NULL;
		PFNGLMULTITEXCOORD3FVARBPROC	glMultiTexCoord3fv	= NULL;
		PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4f	= NULL;
		PFNGLMULTITEXCOORD4FVARBPROC	glMultiTexCoord4fv	= NULL;
		PFNGLMULTITEXCOORD1DARBPROC		glMultiTexCoord1d	= NULL;
		PFNGLMULTITEXCOORD1DVARBPROC	glMultiTexCoord1dv	= NULL;
		PFNGLMULTITEXCOORD2DARBPROC		glMultiTexCoord2d	= NULL;
		PFNGLMULTITEXCOORD2DVARBPROC	glMultiTexCoord2dv	= NULL;
		PFNGLMULTITEXCOORD3DARBPROC		glMultiTexCoord3d	= NULL;
		PFNGLMULTITEXCOORD3DVARBPROC	glMultiTexCoord3dv	= NULL;
		PFNGLMULTITEXCOORD4DARBPROC		glMultiTexCoord4d	= NULL;
		PFNGLMULTITEXCOORD4DVARBPROC	glMultiTexCoord4dv	= NULL;
		PFNGLMULTITEXCOORD1IARBPROC		glMultiTexCoord1i	= NULL;
		PFNGLMULTITEXCOORD1IVARBPROC	glMultiTexCoord1iv	= NULL;
		PFNGLMULTITEXCOORD2IARBPROC		glMultiTexCoord2i	= NULL;
		PFNGLMULTITEXCOORD2IVARBPROC	glMultiTexCoord2iv	= NULL;
		PFNGLMULTITEXCOORD3IARBPROC		glMultiTexCoord3i	= NULL;
		PFNGLMULTITEXCOORD3IVARBPROC	glMultiTexCoord3iv	= NULL;
		PFNGLMULTITEXCOORD4IARBPROC		glMultiTexCoord4i	= NULL;
		PFNGLMULTITEXCOORD4IVARBPROC	glMultiTexCoord4iv	= NULL;
		PFNGLACTIVETEXTUREARBPROC		glActiveTexture	= NULL;
		PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTexture	= NULL;
	#endif

	#ifdef GL_NV_register_combiners
		PFNGLCOMBINERPARAMETERFVNVPROC			glCombinerParameterfv	= NULL;
		PFNGLCOMBINERPARAMETERFNVPROC			glCombinerParameterf	= NULL;
		PFNGLCOMBINERPARAMETERIVNVPROC			glCombinerParameteriv	= NULL;
		PFNGLCOMBINERPARAMETERINVPROC			glCombinerParameteri	= NULL;
		PFNGLCOMBINERINPUTNVPROC				glCombinerInput		= NULL;
		PFNGLCOMBINEROUTPUTNVPROC				glCombinerOutput		= NULL;
		PFNGLFINALCOMBINERINPUTNVPROC			glFinalCombinerInput	= NULL;
		PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC	glGetCombinerInputParameterfv	= NULL;
		PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC	glGetCombinerInputParameteriv	= NULL;
		PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC	glGetCombinerOutputParameterfv	= NULL;
		PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC	glGetCombinerOutputParameteriv	= NULL;
		PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC	glGetFinalCombinerInputParameterfv	= NULL;
		PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC	glGetFinalCombinerInputParameteriv	= NULL;
	#endif


	#ifdef GL_ARB_vertex_program
		PFNGLVERTEXATTRIB2FARBPROC				glVertexAttrib2f		= NULL;
		PFNGLVERTEXATTRIB2DARBPROC				glVertexAttrib2d		= NULL;
		PFNGLVERTEXATTRIB3FARBPROC				glVertexAttrib3f		= NULL;
		PFNGLVERTEXATTRIB3DARBPROC				glVertexAttrib3d		= NULL;
		PFNGLVERTEXATTRIB4FARBPROC				glVertexAttrib4f		= NULL;
		PFNGLVERTEXATTRIB4DARBPROC				glVertexAttrib4d		= NULL;
		PFNGLVERTEXATTRIB2FVARBPROC				glVertexAttrib2fv		= NULL;
		PFNGLVERTEXATTRIB2DVARBPROC				glVertexAttrib2dv		= NULL;
		PFNGLVERTEXATTRIB3FVARBPROC				glVertexAttrib3fv		= NULL;
		PFNGLVERTEXATTRIB3DVARBPROC				glVertexAttrib3dv		= NULL;
		PFNGLVERTEXATTRIB4IVARBPROC				glVertexAttrib4iv		= NULL;
		PFNGLVERTEXATTRIB4UIVARBPROC			glVertexAttrib4uiv		= NULL;
		PFNGLVERTEXATTRIB4FVARBPROC				glVertexAttrib4fv		= NULL;
		PFNGLVERTEXATTRIB4DVARBPROC				glVertexAttrib4dv		= NULL;
		PFNGLVERTEXATTRIB4NIVARBPROC			glVertexAttrib4Niv		= NULL;
		PFNGLVERTEXATTRIB4NUIVARBPROC			glVertexAttrib4Nuiv		= NULL;
		PFNGLVERTEXATTRIBPOINTERARBPROC			glVertexAttribPointer		= NULL;
		PFNGLENABLEVERTEXATTRIBARRAYARBPROC		glEnableVertexAttribArray	= NULL;
		PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	glDisableVertexAttribArray	= NULL;
		PFNGLPROGRAMSTRINGARBPROC				glProgramString				= NULL;
		PFNGLBINDPROGRAMARBPROC					glBindProgram				= NULL;
		PFNGLDELETEPROGRAMSARBPROC				glDeletePrograms			= NULL;
		PFNGLGENPROGRAMSARBPROC					glGenPrograms				= NULL;
		PFNGLPROGRAMENVPARAMETER4DARBPROC		glProgramEnvParameter4d		= NULL;
		PFNGLPROGRAMENVPARAMETER4DVARBPROC		glProgramEnvParameter4dv	= NULL;
		PFNGLPROGRAMENVPARAMETER4FARBPROC		glProgramEnvParameter4f		= NULL;
		PFNGLPROGRAMENVPARAMETER4FVARBPROC		glProgramEnvParameter4fv	= NULL;
		PFNGLPROGRAMLOCALPARAMETER4DARBPROC		glProgramLocalParameter4d	= NULL;
		PFNGLPROGRAMLOCALPARAMETER4DVARBPROC	glProgramLocalParameter4dv	= NULL;
		PFNGLPROGRAMLOCALPARAMETER4FARBPROC		glProgramLocalParameter4f	= NULL;
		PFNGLPROGRAMLOCALPARAMETER4FVARBPROC	glProgramLocalParameter4fv	= NULL;
		PFNGLGETPROGRAMENVPARAMETERDVARBPROC	glGetProgramEnvParameterdv	= NULL;
		PFNGLGETPROGRAMENVPARAMETERFVARBPROC	glGetProgramEnvParameterfv	= NULL;
		PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC	glGetProgramLocalParameterdv	= NULL;
		PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC	glGetProgramLocalParameterfv	= NULL;
		PFNGLGETPROGRAMIVARBPROC				glGetProgramiv				= NULL;
		PFNGLGETPROGRAMSTRINGARBPROC			glGetProgramString			= NULL;
		PFNGLGETVERTEXATTRIBDVARBPROC			glGetVertexAttribdv			= NULL;
		PFNGLGETVERTEXATTRIBFVARBPROC			glGetVertexAttribfv			= NULL;
		PFNGLGETVERTEXATTRIBIVARBPROC			glGetVertexAttribiv			= NULL;
		PFNGLGETVERTEXATTRIBPOINTERVARBPROC		glGetVertexAttribPointerv	= NULL;
		PFNGLISPROGRAMARBPROC					glIsProgram					= NULL;
	#endif


	#ifdef GL_ARB_vertex_buffer_object
		PFNGLBINDBUFFERARBPROC					glBindBuffer			= NULL;
		PFNGLDELETEBUFFERSARBPROC				glDeleteBuffers			= NULL;
		PFNGLGENBUFFERSARBPROC					glGenBuffers			= NULL;
		PFNGLISBUFFERARBPROC					glIsBuffer				= NULL;
		PFNGLBUFFERDATAARBPROC					glBufferData			= NULL;
		PFNGLBUFFERSUBDATAARBPROC				glBufferSubData			= NULL;
		PFNGLGETBUFFERSUBDATAARBPROC			glGetBufferSubData		= NULL;
		PFNGLMAPBUFFERARBPROC					glMapBuffer				= NULL;
		PFNGLUNMAPBUFFERARBPROC					glUnmapBuffer			= NULL;
		PFNGLGETBUFFERPARAMETERIVARBPROC		glGetBufferParameteriv	= NULL;
		PFNGLGETBUFFERPOINTERVARBPROC			glGetBufferPointerv		= NULL;
	#endif

	#ifdef GL_EXT_compiled_vertex_array
		PFNGLLOCKARRAYSEXTPROC					glLockArrays			= NULL;
		PFNGLUNLOCKARRAYSEXTPROC				glUnlockArrays			= NULL;
	#endif

	#ifdef GL_ARB_shader_objects
		PFNGLDELETEOBJECTARBPROC				glDeleteObject			= NULL;
		PFNGLGETHANDLEARBPROC					glGetHandle				= NULL;
		PFNGLDETACHOBJECTARBPROC				glDetachObject			= NULL;
		PFNGLCREATESHADEROBJECTARBPROC			glCreateShaderObject	= NULL;
		PFNGLSHADERSOURCEARBPROC				glShaderSource			= NULL;
		PFNGLCOMPILESHADERARBPROC				glCompileShader			= NULL;
		PFNGLCREATEPROGRAMOBJECTARBPROC			glCreateProgramObject	= NULL;
		PFNGLATTACHOBJECTARBPROC				glAttachObject			= NULL;
		PFNGLLINKPROGRAMARBPROC					glLinkProgram			= NULL;
		PFNGLUSEPROGRAMOBJECTARBPROC			glUseProgramObject		= NULL;
		PFNGLVALIDATEPROGRAMARBPROC				glValidateProgram		= NULL;
		PFNGLUNIFORM1FARBPROC					glUniform1f			= NULL;
		PFNGLUNIFORM2FARBPROC					glUniform2f			= NULL;
		PFNGLUNIFORM3FARBPROC					glUniform3f			= NULL;
		PFNGLUNIFORM4FARBPROC					glUniform4f			= NULL;
		PFNGLUNIFORM1IARBPROC					glUniform1i			= NULL;
		PFNGLUNIFORM2IARBPROC					glUniform2i			= NULL;
		PFNGLUNIFORM3IARBPROC					glUniform3i			= NULL;
		PFNGLUNIFORM4IARBPROC					glUniform4i			= NULL;
		PFNGLUNIFORM1FVARBPROC					glUniform1fv			= NULL;
		PFNGLUNIFORM2FVARBPROC					glUniform2fv			= NULL;
		PFNGLUNIFORM3FVARBPROC					glUniform3fv			= NULL;
		PFNGLUNIFORM4FVARBPROC					glUniform4fv			= NULL;
		PFNGLUNIFORM1IVARBPROC					glUniform1iv			= NULL;
		PFNGLUNIFORM2IVARBPROC					glUniform2iv			= NULL;
		PFNGLUNIFORM3IVARBPROC					glUniform3iv			= NULL;
		PFNGLUNIFORM4IVARBPROC					glUniform4iv			= NULL;
		PFNGLUNIFORMMATRIX2FVARBPROC			glUniformMatrix2fv		= NULL;
		PFNGLUNIFORMMATRIX3FVARBPROC			glUniformMatrix3fv		= NULL;
		PFNGLUNIFORMMATRIX4FVARBPROC			glUniformMatrix4fv		= NULL;
		PFNGLGETOBJECTPARAMETERFVARBPROC		glGetObjectParameterfv	= NULL;
		PFNGLGETOBJECTPARAMETERIVARBPROC		glGetObjectParameteriv	= NULL;
		PFNGLGETINFOLOGARBPROC					glGetInfoLog			= NULL;
		PFNGLGETATTACHEDOBJECTSARBPROC			glGetAttachedObjects	= NULL;
		PFNGLGETUNIFORMLOCATIONARBPROC			glGetUniformLocation	= NULL;
		PFNGLGETACTIVEUNIFORMARBPROC			glGetActiveUniform		= NULL;
		PFNGLGETUNIFORMFVARBPROC				glGetUniformfv			= NULL;
		PFNGLGETUNIFORMIVARBPROC				glGetUniformiv			= NULL;
		PFNGLGETSHADERSOURCEARBPROC				glGetShaderSource		= NULL;
	#endif


	#ifdef WGL_EXT_swap_control
		PFNWGLSWAPINTERVALEXTPROC				wglSwapInterval			= NULL;
		PFNWGLGETSWAPINTERVALEXTPROC			wglGetSwapInterval		= NULL;
	#endif

	#ifdef WGL_ARB_pixel_format
		PFNWGLCHOOSEPIXELFORMATARBPROC			wglChoosePixelFormat		= NULL;
		PFNWGLGETPIXELFORMATATTRIBIVARBPROC		wglGetPixelFormatAttribiv	= NULL;
		PFNWGLGETPIXELFORMATATTRIBFVARBPROC		wglGetPixelFormatAttribfv	= NULL;
	#endif

	#ifdef WGL_ARB_extensions_string
		PFNWGLGETEXTENSIONSSTRINGARBPROC		wglGetExtensionsString	= NULL;
	#endif

	#ifdef WGL_ARB_pbuffer
		PFNWGLCREATEPBUFFERARBPROC				wglCreatePbuffer		= NULL;
		PFNWGLGETPBUFFERDCARBPROC				wglGetPbufferDC			= NULL;
		PFNWGLRELEASEPBUFFERDCARBPROC			wglReleasePbufferDC		= NULL;
		PFNWGLDESTROYPBUFFERARBPROC				wglDestroyPbuffer		= NULL;
		PFNWGLQUERYPBUFFERARBPROC				wglQueryPbuffer			= NULL;
	#endif

	#if defined(GLX_SGIX_fbconfig) && !defined(GLX_VERSION_1_3)
		PFNGLXGETFBCONFIGATTRIBSGIXPROC			glXGetFBConfigAttrib	= NULL;
		PFNGLXCHOOSEFBCONFIGSGIXPROC			glXChooseFBConfig		= NULL;
		PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC	glXCreateGLXPixmapWithConfig	= NULL;
		PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC	glXCreateContextWithConfig		= NULL;
		PFNGLXGETVISUALFROMFBCONFIGSGIXPROC		glXGetVisualFromFBConfig		= NULL;
		PFNGLXGETFBCONFIGFROMVISUALSGIXPROC		glXGetFBConfigFromVisual		= NULL;
	#endif


	#ifdef WGL_ARB_render_texture
		PFNWGLBINDTEXIMAGEARBPROC				wglBindTexImage			= NULL;
		PFNWGLRELEASETEXIMAGEARBPROC			wglReleaseTexImage		= NULL;
		PFNWGLSETPBUFFERATTRIBARBPROC			wglSetPbufferAttrib		= NULL;
	#endif

	#ifdef GL_ARB_occlusion_query
		PFNGLGENQUERIESARBPROC					glGenQueries			= NULL;
		PFNGLDELETEQUERIESARBPROC				glDeleteQueries			= NULL;
		PFNGLISQUERYARBPROC						glIsQuery				= NULL;
		PFNGLBEGINQUERYARBPROC					glBeginQuery			= NULL;
		PFNGLENDQUERYARBPROC					glEndQuery				= NULL;
		PFNGLGETQUERYIVARBPROC					glGetQueryiv			= NULL;
		PFNGLGETQUERYOBJECTIVARBPROC			glGetQueryObjectiv		= NULL;
		PFNGLGETQUERYOBJECTUIVARBPROC			glGetQueryObjectuiv		= NULL;
	#endif

	#ifdef WGL_ARB_make_current_read
		PFNWGLMAKECONTEXTCURRENTARBPROC			wglMakeContextCurrent	= NULL;
		PFNWGLGETCURRENTREADDCARBPROC			wglGetCurrentReadDC		= NULL;
	#endif

	#ifdef GL_ARB_window_pos
		PFNGLWINDOWPOS2DPROC					glWindowPos2d			= NULL;
		PFNGLWINDOWPOS2DVPROC					glWindowPos2dv			= NULL;
		PFNGLWINDOWPOS2FPROC					glWindowPos2f			= NULL;
		PFNGLWINDOWPOS2FVPROC					glWindowPos2fv			= NULL;
		PFNGLWINDOWPOS2IPROC					glWindowPos2i			= NULL;
		PFNGLWINDOWPOS2IVPROC					glWindowPos2iv			= NULL;
		PFNGLWINDOWPOS2SPROC					glWindowPos2s			= NULL;
		PFNGLWINDOWPOS2SVPROC					glWindowPos2sv			= NULL;
		PFNGLWINDOWPOS3DPROC					glWindowPos3d			= NULL;
		PFNGLWINDOWPOS3DVPROC					glWindowPos3dv			= NULL;
		PFNGLWINDOWPOS3FPROC					glWindowPos3f			= NULL;
		PFNGLWINDOWPOS3FVPROC					glWindowPos3fv			= NULL;
		PFNGLWINDOWPOS3IPROC					glWindowPos3i			= NULL;
		PFNGLWINDOWPOS3IVPROC					glWindowPos3iv			= NULL;
		PFNGLWINDOWPOS3SPROC					glWindowPos3s			= NULL;
		PFNGLWINDOWPOS3SVPROC					glWindowPos3sv			= NULL;
	#endif

	#ifdef GL_EXT_geometry_shader4
		PFNGLPROGRAMPARAMETERIEXTPROC 			glProgramParameteri		= NULL;
	#endif

	#ifdef GL_ARB_framebuffer_object
		PFNGLISRENDERBUFFERPROC					glIsRenderbuffer		= NULL;
		PFNGLBINDRENDERBUFFERPROC				glBindRenderbuffer		= NULL;
		PFNGLDELETERENDERBUFFERSPROC			glDeleteRenderbuffers	= NULL;
		PFNGLGENRENDERBUFFERSPROC				glGenRenderbuffers		= NULL;
		PFNGLRENDERBUFFERSTORAGEPROC			glRenderbufferStorage	= NULL;
		PFNGLGETRENDERBUFFERPARAMETERIVPROC		glGetRenderbufferParameteriv	= NULL;
		PFNGLISFRAMEBUFFERPROC					glIsFramebuffer			= NULL;
		PFNGLBINDFRAMEBUFFERPROC				glBindFramebuffer		= NULL;
		PFNGLDELETEFRAMEBUFFERSPROC				glDeleteFramebuffers	= NULL;
		PFNGLGENRENDERBUFFERSPROC				glGenFramebuffers		= NULL;
		PFNGLCHECKFRAMEBUFFERSTATUSPROC			glCheckFramebufferStatus	= NULL;
		PFNGLFRAMEBUFFERTEXTURE1DPROC			glFramebufferTexture1D	= NULL;
		PFNGLFRAMEBUFFERTEXTURE2DPROC			glFramebufferTexture2D	= NULL;
		PFNGLFRAMEBUFFERTEXTURE3DPROC			glFramebufferTexture3D	= NULL;
		PFNGLFRAMEBUFFERRENDERBUFFERPROC		glFramebufferRenderbuffer = NULL;
		PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC	glGetFramebufferAttachmentParameteriv = NULL;
		PFNGLGENERATEMIPMAPPROC					glGenerateMipmap		= NULL;
		PFNGLBLITFRAMEBUFFERPROC				glBlitFramebuffer		= NULL;
		PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC	glRenderbufferStorageMultisample = NULL;
		PFNGLFRAMEBUFFERTEXTURELAYERPROC		glFramebufferTextureLayer = NULL;
	#endif
	
	#ifdef GL_EXT_blend_func_separate
		PFNGLBLENDFUNCSEPARATEEXTPROC			glBlendFuncSeparate		= NULL;
	#endif
	
	
	}


namespace Engine
{
	#ifdef GL_ARB_multitexture
		static GLuint	tangent_layer=GL_TEXTURE1;
	#endif


	void		glTangent3f(GLfloat x, GLfloat y, GLfloat z)
	{
		#ifdef GL_ARB_multitexture
			if (glMultiTexCoord3f)
				glMultiTexCoord3f(tangent_layer,x,y,z);
		#endif
	}
	
	void		glTangent3fv(const GLfloat*v)
	{
		#ifdef GL_ARB_multitexture
			if (glMultiTexCoord3fv)
				glMultiTexCoord3fv(tangent_layer,v);
		#endif
	}
	
	void		glTangentTextureLayer(GLuint layer)
	{
		#ifdef GL_ARB_multitexture
			tangent_layer = layer;
		#endif
	}



	GLTextureFactory::GLTextureFactory():frameBuffer(0),resultTexture(0),depthBuffer(0),resolution(0,0),alpha(false),mipmapping(false)
	{}
	
	GLTextureFactory::GLTextureFactory(const Resolution&res,bool alpha_,bool mipmapping_):frameBuffer(0),resultTexture(0),depthBuffer(0),resolution(0,0),alpha(false),mipmapping(false)
	{
		Create(res,alpha_,mipmapping_);
	}
	
	GLTextureFactory::~GLTextureFactory()
	{
		Clear();
	}
	
	void	GLTextureFactory::Clear()
	{
		#ifdef GL_ARB_framebuffer_object
			if (glDeleteFramebuffers)
			{
				if (frameBuffer)
					glDeleteFramebuffers(1, &frameBuffer);
				if (depthBuffer)
					glDeleteRenderbuffers(1, &depthBuffer);
			}
		#endif
		frameBuffer = 0;
		depthBuffer = 0;
	}
	
	
	
	
	
	bool				GLTextureFactory::CheckFormat(const Resolution&res, bool alpha_, bool mipmapping_)
	{
		if (resolution == res && alpha == alpha_ && mipmapping == mipmapping_)
			return true;
		return Create(res,alpha_,mipmapping_);
	}
	
	bool				GLTextureFactory::Create(const Resolution&res,bool alpha_, bool mipmapping_)
	{
		resolution = res;
		alpha = alpha_;
		mipmapping = mipmapping_;
		//samples = samples_;
		
		Clear();
		
		#ifdef GL_ARB_framebuffer_object
			if (glGenFramebuffers)
			{
					glGenFramebuffers( 1, &frameBuffer );
					glGenRenderbuffers( 1, &depthBuffer );

					glBindRenderbuffer( GL_RENDERBUFFER, depthBuffer );
					//if (!samples)
						glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution.width, resolution.height );
					/*else
						glRenderbufferStorageMultisample(GL_RENDERBUFFER,samples, GL_DEPTH_COMPONENT24, width, height);*/
/* 
					glBindRenderbuffer( GL_RENDERBUFFER, color_buffer );
					glRenderbufferStorage( GL_RENDERBUFFER, alpha?GL_RGBA8:GL_RGB8, width, height ); */


					GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
					if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
					{
						Clear();
						return false;
					}

					
				
					glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);


					glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
					//glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer );
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					
					return true;
			}
		#endif
		
		return false;
	}
	
	bool				GLTextureFactory::Begin()
	{
		#ifdef GL_ARB_framebuffer_object
			if (glBindFramebuffer && frameBuffer)
			{
				glGetError();//flush errors
				glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT,&previousFrameBuffer);		
				glGetFloatv(GL_COLOR_CLEAR_VALUE,previousClearColor.v);
				glGetIntegerv(GL_VIEWPORT, viewport); 
								
				glGenTextures( 1, &resultTexture );

				glBindTexture( GL_TEXTURE_2D, resultTexture );

				glTexImage2D( GL_TEXTURE_2D, 0, alpha?GL_RGBA:GL_RGB, 
						    resolution.width, resolution.height, 
						    0, alpha?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, 0 );

				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
				glBindTexture(GL_TEXTURE_2D,0);
				
				glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resultTexture, 0 );
				if (glGetError()!=GL_NO_ERROR)
				{
					glDeleteTextures(1,&resultTexture);
					resultTexture = 0;
					return false;
				}
				glViewport(0,0,resolution.width,resolution.height);
				glClearColor(0,0,0,0);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

				return true;
			}
		#endif
		return false;
	}
	

	
	
	GLuint				GLTextureFactory::End()
	{
		#ifdef GL_ARB_framebuffer_object
			if (glBindFramebuffer && frameBuffer)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, previousFrameBuffer);
				glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
				glClearColor(previousClearColor.r,previousClearColor.g,previousClearColor.b,previousClearColor.a);
				/*glBindTexture(GL_TEXTURE_2D,result_texture);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,0);*/
				GLuint result = resultTexture;

				if (mipmapping)
				{
					ASSERT_NOT_NULL__(glGenerateMipmap);
					glBindTexture(GL_TEXTURE_2D,resultTexture);
						glGenerateMipmap(GL_TEXTURE_2D);
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

					glBindTexture(GL_TEXTURE_2D,0);
				}

				resultTexture = 0;
				return result;
			}
			
		#endif
		return 0;
	}
	
	void				GLTextureFactory::Test()
	{
		#ifdef GL_ARB_framebuffer_object
			
			glGetError();//flush errors
			static GLuint test_texture = 0;
			if (!test_texture)
			{
				glGenTextures( 1, &test_texture );

				glBindTexture( GL_TEXTURE_2D, test_texture );

				glTexImage2D( GL_TEXTURE_2D, 0, alpha?GL_RGBA:GL_RGB, 
						    resolution.width, resolution.height, 
						    0, alpha?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, 0 );

				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			}

			
			if (glBindFramebuffer && frameBuffer)
			{
				glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT,&previousFrameBuffer);		
				glGetFloatv(GL_COLOR_CLEAR_VALUE,previousClearColor.v);
				glGetIntegerv(GL_VIEWPORT, viewport); 
								
				
				glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, test_texture, 0 );
				if (glGetError()!=GL_NO_ERROR)
					return;
				glViewport(0,0,resolution.width,resolution.height);
				glClearColor(0,0,0,0);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				
				
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
				glClearColor(previousClearColor.r,previousClearColor.g,previousClearColor.b,previousClearColor.a);
				resultTexture = 0;
			}
			
		#endif
	}
	
	void				GLTextureFactory::Abort()
	{
		#ifdef GL_ARB_framebuffer_object
			if (glBindFramebuffer && frameBuffer)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, previousFrameBuffer);
				glDeleteTextures(1,&resultTexture);
				resultTexture = 0;
				glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);				
				glClearColor(previousClearColor.r,previousClearColor.g,previousClearColor.b,previousClearColor.a);
			}
		#endif
	}

	bool				GLTextureFactory::IsActive()	const
	{
		return resultTexture != 0;
	}
	
	bool				GLTextureFactory::IsCreated()	const
	{
		return frameBuffer != 0;
	}




	void ExtCont::divide(String line)
	{
		unsigned elements	= 0;
		for (unsigned i	= 1; i < line.GetLength(); i++)
			if (line.GetChar(i) == ' ' && line.GetChar(i-1) != ' ')
				elements++;
		if (line.GetLength() && line.LastChar() != ' ')
			elements++;
		field.SetSize(elements);
		for (index_t i	= 0; i < elements; i++)
		{
			index_t p	= line.Find(' ');
			if (p == InvalidIndex)
				field[i] = line;
			else
			{
				field[i] = line.subString(0,p);
				index_t j	= i-1;
				while (j < i && field[j+1] < field[j])
				{
					swp(field[j],field[j+1]);
					j--;
				}
				line.Erase(0,p);
				line = line.TrimLeft();
			}
		}
	}

	bool ExtCont::lookup(const String&element)
	{
		index_t		lower	= 0,
					upper	= field.GetLength();
		while (lower< upper && upper <= field.GetLength())
		{
			index_t el	= (lower+upper)/2;
			if (field[el] > element)
				upper	= el;
			else
				if (field[el] < element)
					lower	= el+1;
				else
					return true;
		}
		return false;
	}

#if 0
	GLuint Extension::loadProgram(const char*source, GLenum type)
	{
		#ifdef GL_ARB_vertex_program
			GLuint	pname;
			glGenPrograms(1,&pname);
			glBindProgram(type, pname);
			glProgramString(type, GL_PROGRAM_FORMAT_ASCII_ARB,
								(GLsizei)strlen(source), source);
			int errl;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB,&errl);
			if (errl >= 0)
			{
				unsigned end	=errl+1;
				while (source[end] != ',' && source[end] != ';' && source[end] != ' ' && source[end] != '\n' && source[end])
					end++;
				String source2	= source;
				source2.insert(end,'<');
				source2.insert(errl,'>');
				String line = String(source).subString(errl-1,end-errl-1);
				compile_log	<<	(char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB)<<": \""<<line<<"\"\n---------------------------------------\n"<<source2<<nl;
				glDeletePrograms(1,&pname);
				return 0;
			}
			return pname;
		#else
			compile_log << "Error: Headers missing"<<nl;
			return 0;
		#endif
	}

	GLuint Extension::loadVertexProgram(const char*source)
	{
		return loadProgram(source,GL_VERTEX_PROGRAM_ARB);
	}

	GLuint Extension::loadFragmentProgram(const char*source)
	{
		#ifdef GL_ARB_fragment_program
			return loadProgram(source,GL_FRAGMENT_PROGRAM_ARB);
		#else
			compile_log << "Error: Headers missing"<<nl;
			return 0;
		#endif
	}

	GLuint Extension::loadGeometryProgram(const char*source)
	{
		#ifdef GL_EXT_geometry_shader4
			return loadProgram(source,GL_GEOMETRY_SHADER_EXT);
		#else
			compile_log << "Error: Headers missing"<<nl;
			return 0;
		#endif


	}
#endif /*0*/

	#ifdef GL_ARB_shader_objects
	
	namespace GLShader
	{
	
		Variable::Variable(Instance*parent,GLint handle_, const String&name_):instance(parent),handle(handle_),name(name_)
		{}
	
		Variable::Variable():instance(NULL),handle(-1)
		{}

		#define INIT_VARIABLE_UPDATE \
			if (handle == -1)\
				return false;\
			bool wasInstalled = instance->IsInstalled();\
			if (!wasInstalled && assertIsInstalled)\
				FATAL__("trying to update variable '"+name+"' while shader is NOT installed");\
			if (!wasInstalled && (lockUninstalled || !instance->Install()))\
				return false;\
			glGetError();//flush errors

		#define FINISH_VARIABLE_UPDATE	\
			if (!wasInstalled)\
				instance->Uninstall();\
			return glGetError() == GL_NO_ERROR;

		bool			Variable::setf(float value)
		{
			INIT_VARIABLE_UPDATE
			glUniform1f(handle, value);
			FINISH_VARIABLE_UPDATE
		}

		bool					Variable::Set(const M::TVec3<>&vector)
		{
			INIT_VARIABLE_UPDATE
			glUniform3f(handle, vector.x,vector.y,vector.z);
			FINISH_VARIABLE_UPDATE
		}
	
		bool					Variable::Set(const M::TVec2<>&vector)
		{
			INIT_VARIABLE_UPDATE
			glUniform2f(handle, vector.x,vector.y);
			FINISH_VARIABLE_UPDATE
		}
	

		bool			Variable::set4f(float x, float y, float z, float w)
		{
			INIT_VARIABLE_UPDATE
			glUniform4f(handle, x,y,z,w);
			FINISH_VARIABLE_UPDATE
		}


		bool			Variable::Set(float x, float y, float z)
		{
			INIT_VARIABLE_UPDATE
			glUniform3f(handle, x,y,z);
			FINISH_VARIABLE_UPDATE
		}

		bool			Variable::Set(float x, float y)
		{
			INIT_VARIABLE_UPDATE
			glUniform2f(handle, x,y);
			FINISH_VARIABLE_UPDATE
		}


		bool			Variable::Set(const M::TVec4<>&vector)
		{
			INIT_VARIABLE_UPDATE
			glUniform4f(handle, vector.x,vector.y,vector.z,vector.w);
			FINISH_VARIABLE_UPDATE
		}
	
		bool					Variable::Set(const M::TMatrix3<>&matrix)
		{
			INIT_VARIABLE_UPDATE
			glUniformMatrix3fv(handle,1,false,matrix.v);
			FINISH_VARIABLE_UPDATE
		}
	
		bool					Variable::Set(const M::TMatrix4<>&matrix)
		{
			INIT_VARIABLE_UPDATE
			glUniformMatrix4fv(handle,1,false,matrix.v);
			FINISH_VARIABLE_UPDATE
		}


		bool			Variable::seti(int value)
		{
			INIT_VARIABLE_UPDATE
			glUniform1i(handle, value);
			FINISH_VARIABLE_UPDATE
		}
		bool			Variable::SetBool(bool value)
		{
			INIT_VARIABLE_UPDATE
			glUniform1i(handle, value);	//lame, no bool set operation
			FINISH_VARIABLE_UPDATE
		}
		bool			Variable::set2i(int x, int y)
		{
			INIT_VARIABLE_UPDATE
			glUniform2i(handle, x,y);
			FINISH_VARIABLE_UPDATE
		}
	
		bool Variable::lockUninstalled(false);
		bool Variable::assertIsInstalled(false);
		bool Instance::warnOnError(true);
	
		Instance::Instance():programHandle(0),vertexShader(0),fragmentShader(0),geometryShader(0)
		{}

		Instance::~Instance()
		{
			if (!application_shutting_down && !forgetOnDestruct)
				Clear();
		}

		bool	Instance::IsLoaded()	const
		{
			return this != NULL && programHandle != 0;
		}


		void	Instance::Clear()
		{
			if (programHandle && glGetHandle(GL_PROGRAM_OBJECT_ARB) == programHandle)
				Uninstall();
			if (vertexShader)
				glDeleteObject(vertexShader);
			if (fragmentShader)
				glDeleteObject(fragmentShader);
			if (geometryShader)
				glDeleteObject(geometryShader);
			if (programHandle)
				glDeleteObject(programHandle);
			vertexShader	= 0;
			fragmentShader	= 0;
			geometryShader = 0;
			programHandle	= 0;
			composition.Clear();
			//log.reset();
		}
	
		void	Instance::adoptData(Instance&other)
		{
			bool reinstall = other.IsInstalled();
			if (reinstall)
				other.Uninstall();
			elif (IsInstalled())
				Uninstall();

			Clear();
			composition.adoptData(other.composition);
			vertexShader = other.vertexShader;
			fragmentShader = other.fragmentShader;
			geometryShader = other.geometryShader;
			programHandle = other.programHandle;
			log.adoptData(other.log);
			other.vertexShader = 0;
			other.fragmentShader = 0;
			other.geometryShader = 0;
			other.programHandle = 0;

			if (reinstall)
				Install();
		}
	


		bool		_ExtractFileContent(const String&filename, String&target, StringBuffer&logOut)
		{
			FileStream	file(filename.c_str(),FileStream::StandardRead);
			if (!file.isOpen())
			{
				logOut << "Unable to read from '"<<filename<<"'\n";
				target = "";
				return false;
			}
			uint64_t size = file.size();
			target.Resize(static_cast<size_t>(size));
			try
			{
				file.Read(target.mutablePointer(),size);
			}
			catch (const std::exception&ex)
			{
				logOut << "Unable to read from '"<<filename<<"':"<<ex.what()<<"\n";
				target = "";
				return false;
			}
			return true;
		}

		bool		_ExtractFileContent(const String&filename, String&target)
		{
			if (filename.IsEmpty())
			{
				target = "";
				return true;
			}
			FileStream	file(filename.c_str(),FileStream::StandardRead);
			if (!file.isOpen())
			{
				target = "";
				return false;
			}
			uint64_t size = file.size();
			target.Resize(static_cast<size_t>(size));
			try
			{
				file.Read(target.mutablePointer(),size);
			}
			catch (const std::exception&ex)
			{
				target = "";
				return false;
			}
			return true;
		}



		void				Composition::Clear()
		{
			sharedSource = "";
			vertexSource = "";
			fragmentSource = "";
			geometrySource = "";
		}

		void				Composition::adoptData(Composition&other)
		{
			sharedSource.adoptData(other.sharedSource);
			vertexSource.adoptData(other.vertexSource);
			fragmentSource.adoptData(other.fragmentSource);
			geometrySource.adoptData(other.geometrySource);
		}

		void				Composition::Append(const Composition&other)
		{
			sharedSource += '\n' + other.sharedSource;
			vertexSource += '\n' + other.vertexSource;
			fragmentSource += '\n' + other.fragmentSource;
			geometrySource += '\n' + other.geometrySource;
		}
		void				Composition::Prefix(const Composition&other)
		{
			sharedSource = other.sharedSource + '\n' + sharedSource;
			vertexSource = other.vertexSource + '\n' + vertexSource;
			fragmentSource = other.fragmentSource + '\n' + fragmentSource;
			geometrySource = other.geometrySource + '\n' + geometrySource;
		}



		Composition&			Composition::Load(const String&source)
		{
			if (source.Contains("[vertex]"))
			{
				static const String type[5]={"[object]","[shared]","[fragment]","[vertex]","[geometry]"};

				const String		*current(NULL);
					

				const char	*at = source.c_str(),
							*end = at+source.GetLength();
				while (at < end)
				{
					const char*	min = NULL;
					const String*item	= NULL;
					
					for (unsigned k	= 0; k < ARRAYSIZE(type); k++)
					{
						const char*myAt = DeltaWorks::CharFunctions::strstr(at,type[k].c_str());
						if (!myAt)
							continue;
						if (!min || myAt < min)
						{
							min	= myAt;
							item = type+k;
						}
					}
					StringRef	segment = item?StringRef(at,min-at):StringRef(at,end-at);
					if (item)
						at = min+item->GetLength();
					else
						at = end;
					switch (current-type)
					{
						case 1:
							sharedSource = segment;
						break;
						case 2:
							fragmentSource	= segment;
						break;
						case 3:
							vertexSource = segment;
						break;
						case 4:
							geometrySource = segment;
						break;
					}
					current	= item;
				}
			}
			else
			{
				static Tokenizer::Configuration config;
				if (!config.recursion_break)
				{
					config.recursion_up = "{";
					config.recursion_down = "}";
					config.recursion_break = true;
					config.trim_characters.Set('\n');
					config.trim_characters.Set('\r');
				}
				static DeltaWorks::Ctr::StringList tokens;
				Tokenizer::tokenize(source, config, tokens);	//!< @overload

				for (index_t i = 0; i+1 < tokens.Count();)
				{
					String*sourceTarget = NULL;
					const String&group = tokens[i];
					{
						if (group == "shared")
							sourceTarget = &sharedSource;
						elif (group == "vertex")
							sourceTarget = &vertexSource;
						elif (group == "fragment")
							sourceTarget = &fragmentSource;
						elif (group == "geometry")
							sourceTarget = &geometrySource;
					}
					i++;
					if (!sourceTarget)
					{
						if (group.TrimRef().GetLength() > 0)
							ErrMessage("Warning: unexpected code token(s) encountered: '"+group+"'");
						continue;
					}
					for (; i < tokens.Count(); )
					{
						StringRef	source = tokens[i++].TrimRef();
						if (source.GetLength() == 0)
							continue;
						
						if (source.pointer()[0] != '{' || source.pointer()[source.GetLength()-1] != '}')
							continue;
						
						*sourceTarget = StringRef(source.pointer() + 1, source.GetLength()-2);
						sourceTarget = NULL;
						break;
					}
					if (sourceTarget != NULL)
						ErrMessage("Warning: Unable to find body for group '"+group+"'");
				}

			}
			return *this;
		}

		bool				Composition::LoadFromFiles(const String&sharedFile, const String&vertexFile, const String&fragmentFile)
		{
			geometrySource = "";
			return	Engine::GLShader::_ExtractFileContent(sharedFile,sharedSource)
					&&
					Engine::GLShader::_ExtractFileContent(vertexFile,vertexSource)
					&&
					Engine::GLShader::_ExtractFileContent(fragmentFile,fragmentSource);
		}

		bool				Composition::LoadFromFiles(const String&sharedFile, const String&vertexFile, const String&fragmentFile, const String&geometryFile)
		{
			return	Engine::GLShader::_ExtractFileContent(sharedFile,sharedSource)
					&&
					Engine::GLShader::_ExtractFileContent(vertexFile,vertexSource)
					&&
					Engine::GLShader::_ExtractFileContent(fragmentFile,fragmentSource)
					&&
					Engine::GLShader::_ExtractFileContent(geometryFile,geometrySource);
		}

		bool				Composition::LoadFromFile(const String&objectFile)
		{
			String content;
			if (!Engine::GLShader::_ExtractFileContent(objectFile,content))
				return false;
			Load(content);
			return true;
		}

	
		bool	Instance::_ExtractFileContent(const String&filename, String&target)
		{
			return Engine::GLShader::_ExtractFileContent(filename,target,log);

		}

		static String	glProgramTypeName(GLenum programType)
		{
			#undef ecase
			#define ecase(type)	case type:	return #type;
		
			switch (programType)
			{
				ecase(GL_VERTEX_SHADER_ARB)
				ecase(GL_FRAGMENT_SHADER_ARB)
				ecase(GL_GEOMETRY_SHADER_EXT)
			}
			return "Unkown shader type ("+String(programType)+")";
		}

		GLhandleARB Instance::_LoadShader(const String&source, GLenum programType)
		{
			if (!glCreateShaderObject)
			{
				log<<"extension not loaded!\n";
				return 0;
			}
			glGetError();//flush previous errors
			const char*str	= source.c_str();
			GLhandleARB shader	= glCreateShaderObject(programType);
			glShaderSource(shader,1,&str, NULL);
			glCompileShader(shader);
			GLenum glErr	= glGetError();
			while (glErr != GL_NO_ERROR)
			{
				log<<"error while compiling shader("<<glProgramTypeName(programType)<<"): \""<<glGetErrorName(glErr)<<"\"\n";
				glDeleteObject(shader);
				return 0;
			}
			int compiled;
			glGetObjectParameteriv(shader,GL_OBJECT_COMPILE_STATUS_ARB,&compiled);

			int logLen	= 0;
			glGetObjectParameteriv(shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
			if (logLen > 1)
			{
				Array<char,Primitive>	logStr(logLen);
				int		written(0);
				glGetInfoLog(shader, logLen, &written, logStr.pointer());
				if (strstr(logStr.pointer(),"was successfully compiled to run on hardware") && logLen < 64)//maybe some spaces and newlines and stuff
				{
					logLen = 0;
				}
				else
				{
					log<<"\n-- shader("<<glProgramTypeName(programType)<<") construction --\n";
					log<<logStr.pointer();
				}
			}
			if (!compiled)
			{
				log<<"internal error: shader did not compile\n";

				glDeleteObject(shader);
				shader = 0;
			}
			if (logLen > 1)
			{
				log<<"\noriginal source was:\n--------------------------------";
				String copy	= source;
				unsigned line(1);
				index_t at;
				while (copy.GetLength())
				{
					log<<"\n("+IntToStr(line++)+") ";
					if ((at = copy.Find('\n'))!=InvalidIndex)
					{
						log<<copy.subString(0,at);
						copy.Erase(0,at+1);
					}
					else
					{
						log<<copy;
						copy	= "";
					}
				}
				log<<"\n--------------------------------\n";
			}
		
			return shader;
		}

		void		Instance::LoadRequired(const Composition&composition, GLenum geometryType, GLenum outputType, unsigned maxVertices)
		{
			if (!Load(composition,geometryType,outputType,maxVertices))
				FATAL__(Report());
		}

		void				Instance::LoadFromFile(const String&filename, GLenum geometryType/*=GL_TRIANGLES*/, GLenum outputType/*=GL_TRIANGLE_STRIP*/, unsigned maxVertices/*=12*/)
		{
			Composition composition;
			if (!composition.LoadFromFile(filename))
				throw Except::IO::DriveAccess::FileOpenFault("Unable to open file \""+filename+"\"");
			if (!Load(composition,geometryType,outputType,maxVertices))
				throw Except::Renderer::ShaderRejected(Report());
		}

		/*static*/ void				Instance::_ParseUniformVariableInitializers(String&source,Ctr::BasicBuffer<Initializer,Strategy::Swap>& initializers)
		{
			const char*str = source.c_str(),*begin = NULL;
			index_t offset = initializers.Count();
			while ((begin = CharFunctions::strstr(str,"<:=")))
			{
				const char*valueBegin = begin+3;
				const char*foundAt = begin;
				begin--;
				while (begin > str && IsWhitespace( *begin ) )
					begin--;
				const char*nameEnd = begin+1;
				while (begin > str && (CharFunctions::isalnum(*begin) || *begin == '_'))
					begin--;
				begin++;
				Initializer&init = initializers.Append();
				init.variableName = String(begin,nameEnd-begin);

				while (*valueBegin && IsWhitespace( *valueBegin ))
					valueBegin++;
				const char*valueEnd = valueBegin;
				while (*valueEnd && *valueEnd != '>' && !IsWhitespace(*valueEnd))
					valueEnd++;
				ASSERT__(convert(valueBegin,valueEnd-valueBegin,init.intValue));
				str = strchr(valueEnd,'>');
				if (!str)
					return;
				str++;
				init.start = foundAt - source.c_str();
				init.length = str - foundAt;
			}
			if (offset == initializers.Count())
				return;
			StringBuffer	buffer;
			const char*from = source.c_str();
			index_t current = 0;
			foreach (initializers,init)
			{
				buffer.Write(from+current,init->start-current);
				current += init->length + (init->start-current);
			}
			buffer.Write(from+current,source.GetLength()-current);
			source = buffer.ToStringRef();

		}


		bool		Instance::Load(const Composition&composition_, GLenum geometryType, GLenum outputType, unsigned maxVertices)
		{
			Clear();
			this->composition = composition_;
			glGetError();//flush errors
			Ctr::Buffer<Initializer,0,Strategy::Swap>	initializers;
			_ParseUniformVariableInitializers(composition.sharedSource,initializers);
			_ParseUniformVariableInitializers(composition.vertexSource,initializers);
			_ParseUniformVariableInitializers(composition.fragmentSource,initializers);
			_ParseUniformVariableInitializers(composition.geometrySource,initializers);


			vertexShader	= _LoadShader(composition.sharedSource+composition.vertexSource,GL_VERTEX_SHADER_ARB);
			fragmentShader	= _LoadShader(composition.sharedSource+composition.fragmentSource,GL_FRAGMENT_SHADER_ARB);
			#ifdef GL_GEOMETRY_SHADER_EXT
				bool hasGeometryShader = composition.geometrySource.Contains("main");
				if (hasGeometryShader)
				{
					String fullSource;
					index_t p = composition.geometrySource.Find('#');
					if (p==InvalidIndex)
						fullSource = 
						"#version 120 \n"
						"#extension GL_EXT_geometry_shader4 : enable	\n"
						+composition.sharedSource+composition.geometrySource;
					else
					{
						const char*c = composition.geometrySource.c_str()+p+1;
						while (*c && *c != '\n')
							c++;
						fullSource = composition.geometrySource.subString(0,c-composition.geometrySource.c_str()+1)+composition.sharedSource+(c+1);
					}
				
				
					geometryShader = _LoadShader(fullSource,GL_GEOMETRY_SHADER_EXT);
					if (geometryShader && !glProgramParameteri)
					{
						String group;
						EXT_CONTEXT(EXT);
						EXT_GET_EXTENSION_NO_CHECK(glProgramParameteri);
						if (!glProgramParameteri)
						{
							log << "(glProgramParameteri not available)\n";
							Clear();
							return false;
						}
					}
					if (hasGeometryShader && !geometryShader)
					{
						log<<"(geometry shader not set)\n";
						Clear();
						return false;
					}
				}
				else
					geometryShader = 0;
			#endif
			if (!vertexShader || !fragmentShader)
			{
				log<<"(shader-fragment(s) not set)\n";
				Clear();
				return false;
			}
			programHandle	= glCreateProgramObject();
			glAttachObject(programHandle, vertexShader);
			glAttachObject(programHandle, fragmentShader);
			if (geometryShader)
			{
				glAttachObject(programHandle, geometryShader);

				glProgramParameteri(programHandle, GL_GEOMETRY_INPUT_TYPE_EXT, geometryType);
				glProgramParameteri(programHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT, outputType);
				glProgramParameteri(programHandle, GL_GEOMETRY_VERTICES_OUT_EXT, maxVertices);
			}


			glLinkProgram(programHandle);

			GLenum glErr	= glGetError();
			while (glErr != GL_NO_ERROR)
			{
				log<<"error while linking program: \""<<glGetErrorName(glErr)<<"\"\n";
				Clear();
				return false;
			}
			int linked;
			glGetObjectParameteriv(programHandle,GL_OBJECT_LINK_STATUS_ARB, &linked);
			int logLen	= 0;
			glGetObjectParameteriv(programHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
			if (logLen > 0)
			{
				Array<char>	logStr(logLen);
				int		written(0);
				glGetInfoLog(programHandle, logLen, &written, logStr.pointer());
				log<<"\n-- program construction --\n";
				log<<logStr;
				log<<nl;
			}
			if (!linked)
			{
				log<<"program Link failed\n";
				Clear();
				return false;
			}

			if (initializers.IsNotEmpty())
			{
				ASSERT__(Install());
				foreach (initializers,initializer)
				{
					Variable v= FindVariable(initializer->variableName,false);
					v.SetInt(initializer->intValue);
				}
				Uninstall();
			}

#if 0
			glValidateProgram(programHandle);
			logLen	= 0;
			glGetObjectParameteriv(programHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
			if (logLen > 0)
			{
				char	*logStr	= SHIELDED_ARRAY(new char[logLen],logLen);
				int		written(0);
				glGetInfoLog(programHandle, logLen, &written, logStr);
				log<<"\n-- program validation --\n";
				log<<logStr;
				log<<nl;
				DISCARD_ARRAY(logStr);
			}

			int validated;
			glGetObjectParameteriv(programHandle, GL_OBJECT_VALIDATE_STATUS_ARB,&validated);
			if (!validated)
			{
				log << "validation failed\n";
				Clear();
				return false;
			}
#endif /*9*/
			return true;
		}

		bool			Instance::Validate()
		{
			log	<< "validating program...\n";
			if (!programHandle)
			{
				log << "no program present\n";
				return false;
			}
			glGetError();//flush errors
			glValidateProgram(programHandle);
			if (GLenum err	= glGetError())
				if (err != GL_NO_ERROR)
				{
					log << "GL: "<<glGetErrorName(err);
					return false;
				}
			int logLen	= 0;
			glGetObjectParameteriv(programHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
			if (logLen > 0)
			{
				char	*logStr	= alloc<char>(logLen);
				int		written(0);
				glGetInfoLog(programHandle, logLen, &written, logStr);
				log<<"\n-- program validation --\n";
				log<<logStr;
				log<<nl;
				dealloc(logStr);
			}

			int validated;
			glGetObjectParameteriv(programHandle, GL_OBJECT_VALIDATE_STATUS_ARB,&validated);
			if (!validated)
			{
				log << "validation failed\n";
				return false;
			}
			return true;
		}

		void		Instance::LoadRequiredComposition(const String&objectSource, GLenum inputType, GLenum outputType, unsigned maxVertices)
		{
			if (!LoadComposition(objectSource,inputType,outputType,maxVertices))
				FATAL__(Report());
		}

		bool		Instance::LoadComposition(const String&objectSource, GLenum inputType, GLenum outputType, unsigned maxVertices)
		{
			return Load(Composition().Load(objectSource),inputType,outputType,maxVertices);
		}



		String	Instance::Report()
		{
			String rs	= log.ToStringRef();
			log.reset();
			return rs;
		}

		void			Instance::EnableErrorWarning()
		{
			warnOnError	= true;
		}

		void			Instance::DisableErrorWarning()
		{
			warnOnError	= false;
		}

		void			Instance::SuppressFindVariableFailureWarning()
		{
			warnOnError	= false;
		}
	
		void			Instance::SuppressWarnings()
		{
			warnOnError	= false;
		}

		bool			Instance::GetContent(GLint name, float*outField)
		{
			if (!programHandle)
			{
				log	<< "Program not created"<<nl;
				return false;
			}
			if (name	== -1)
			{
				log	<< "The requested variable does not exist"<<nl;
				return false;
			}
			glGetUniformfv(programHandle,name,outField);
			return true;
		}


		Variable Instance::FindVariable(const String&name, bool warnOnFail)
		{
			if (!programHandle)
			{
				log	<< "Program not created"<<nl;
				if (warnOnError && warnOnFail)
				{
					std::cout << "unable to locate uniform variable '"<<name<<"' - no object present."<<std::endl;
					#ifdef _DEBUG
						__debugbreak();
					#endif
					ErrMessage("unable to locate uniform variable '"+name+"' - no object present.");
				}
				return Variable();
			}
			GLint result	= glGetUniformLocation(programHandle,name.c_str());
			if (result	== -1)
				if (warnOnError && warnOnFail)
				{
					std::cout << "unable to locate uniform variable '"<<name<<"'."<<std::endl;
					#ifdef _DEBUG
						__debugbreak();
					#endif
					//ErrMessage("unable to locate uniform variable '"+name+"'.");
				}
				else
					log << nl<<"Unable to locate uniform variable '"<<name<<"'"<<nl;
			return Variable(this,result,name);
		}

		bool			Instance::Install(/*const TCodeLocation&location*/)
		{
			if (!IsLoaded())
				return false;

			glGetError();	//flush previous errors
			#ifdef INSTALLED_SHADER_INSTANCE
				if (installedInstance)
				{
					FATAL__("overriding previously installed shader object 0x"+PointerToHex(installedInstance)/*+". Remote shader was installed in:\n"+String(installed_object->installed_in.ToString())*/);
					installedInstance->Uninstall();
				}
			#else
				if (glGetHandle(GL_PROGRAM_OBJECT_ARB)	!= 0)
				{
					FATAL__("overriding previously installed shader object "+String(glGetHandle(GL_PROGRAM_OBJECT_ARB)));
				}
			#endif
			//installed_in = location;
			glUseProgramObject(programHandle);
			GLenum error	= glGetError();
			bool isInstalled = error	== GL_NO_ERROR;
			if (!isInstalled)
			{
				log.reset();
				log << "Shader installation of object #"<<programHandle<<" failed"<<nl;
				log	<< glGetErrorName(error)<<nl;

				glValidateProgram(programHandle);
				GLint logLen	= 0;
				glGetObjectParameteriv(programHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
				if (logLen > 0)
				{
					char	*logStr	= alloc<char>(logLen);
					int		written(0);
					glGetInfoLog(programHandle, logLen, &written, logStr);
					log<<"\n-- program validation --\n";
					log<<logStr;
					log<<nl;
					dealloc(logStr);
				}

				int validated;
				glGetObjectParameteriv(programHandle, GL_OBJECT_VALIDATE_STATUS_ARB,&validated);
				if (!validated)
				{
					log << "validation failed\n";
					Clear();
					return false;
				}



				if (glGetHandle(GL_PROGRAM_OBJECT_ARB) != 0)
				{
					log << "OpenGL reports error but shader is installed. Uninstalling shader and returning false."<<nl;
					glUseProgramObject(0);
				}
			}

 			#ifdef INSTALLED_SHADER_INSTANCE
				if (isInstalled)
					installedInstance = this;
				else
					installedInstance = NULL;
			#endif

			return isInstalled;
		}

		bool			Instance::Install(/*const TCodeLocation&location*/)	const
		{
			if (!this)
				return true;
			if (!programHandle)
				return false;
			glGetError();	//flush previous errors
			#ifdef INSTALLED_SHADER_INSTANCE
				if (installedInstance)
				{
					FATAL__("overriding previously installed shader object 0x"+PointerToHex(installedInstance)/*+". Remote shader was installed in:\n"+String(installed_object->installed_in.ToString())*/);
					installedInstance->Uninstall();
				}
			#else
				if (glGetHandle(GL_PROGRAM_OBJECT_ARB)	!= 0)
				{
					FATAL__("overriding previously installed shader object "+String(glGetHandle(GL_PROGRAM_OBJECT_ARB)));
				}
			#endif
			//installed_in = location;
			glUseProgramObject(programHandle);
			GLenum error	= glGetError();
			bool isInstalled = error	== GL_NO_ERROR;
			if (!isInstalled)
				glUseProgramObject(0);

			#ifdef INSTALLED_SHADER_INSTANCE
				if (isInstalled)
					installedInstance = this;
				else
					installedInstance = NULL;
			#endif
			return isInstalled;
		}



		bool			Instance::PermissiveInstall(/*const TCodeLocation&location*/)
		{
			if (!IsLoaded())
			{
				#ifdef INSTALLED_SHADER_INSTANCE
					if (installedInstance)
					{
						installedInstance->Uninstall();
						installedInstance = NULL;
					}
				#else
					glUseProgramObject(0);
				#endif

				return true;
			}
			#ifdef INSTALLED_SHADER_INSTANCE
				if (installedInstance == this)
					return true;


				if (installedInstance)
					installedInstance->Uninstall();
			#endif

			glGetError();	//flush previous errors
			//installed_in = location;

			glUseProgramObject(programHandle);
			GLenum error	= glGetError();

			bool isInstalled = (error == GL_NO_ERROR);
			if (!isInstalled)
			{
				log.reset();
				log << "Shader installation of object #"<<programHandle<<" failed"<<nl;
				log	<< glGetErrorName(error)<<nl;
				if (glGetHandle(GL_PROGRAM_OBJECT_ARB) != 0)
				{
					log << "OpenGL reports error but shader is installed. Uninstalling shader and returning false."<<nl;
					glUseProgramObject(0);
				}
			}
			#ifdef INSTALLED_SHADER_INSTANCE
				if (isInstalled)
					installedInstance = this;
				else
					installedInstance = NULL;
			#endif
			return isInstalled;
		}



		bool			Instance::PermissiveInstall(/*const TCodeLocation&location*/)	const
		{
			if (!IsLoaded())
			{
				#ifdef INSTALLED_SHADER_INSTANCE
					if (installedInstance)
					{
						installedInstance->Uninstall();
						installedInstance = NULL;
					}
				#else
					glUseProgramObject(0);
				#endif
				return true;
			}
			#ifdef INSTALLED_SHADER_INSTANCE
				if (installedInstance == this)
					return true;


				if (installedInstance)
					installedInstance->Uninstall();
			#endif
			glGetError();	//flush previous errors
			//installed_in = location;

			glUseProgramObject(programHandle);
			GLenum error	= glGetError();

			bool isInstalled = (error == GL_NO_ERROR);
			if (!isInstalled)
				glUseProgramObject(0);

			#ifdef INSTALLED_SHADER_INSTANCE
				if (isInstalled)
					installedInstance = this;
				else
					installedInstance = NULL;
			#endif
			return isInstalled;
		}







		void			Instance::Uninstall()	const
		{
			if (!this)
				return;
			#ifdef INSTALLED_SHADER_INSTANCE
				if (installedInstance != this)
					FATAL__("Uninstalling but installed pointer is not this");
			#endif
			if (glGetHandle(GL_PROGRAM_OBJECT_ARB) != programHandle)
				FATAL__("UnInstalling but installed handle is not mine");
			#ifdef INSTALLED_SHADER_INSTANCE
				installedInstance = NULL;
			#endif
			if (glUseProgramObject)
				glUseProgramObject(0);
		}

		/*static*/	void	Instance::PermissiveUninstall()
		{
			#ifdef INSTALLED_SHADER_INSTANCE
				installedInstance = NULL;
			#endif
			if (glUseProgramObject)
				glUseProgramObject(0);
		}

	
	
			bool							Instance::IsInstalled()	const
			{
				#ifdef INSTALLED_SHADER_INSTANCE
					return installedInstance == this;
				#else
					return glGetHandle(GL_PROGRAM_OBJECT_ARB) == programHandle;
				#endif
			}
			/*static*/ bool						Instance::AnyIsInstalled()
			{
				#ifdef INSTALLED_SHADER_INSTANCE
					return installedInstance != NULL;
				#else
					return glGetHandle(GL_PROGRAM_OBJECT_ARB) != 0;
				#endif
			}

	
	
	
	
			int Template::VariableExpression::Evaluate(const UserConfiguration&status, Light::Type)
			{
				return status.GetValue(index);
			}

			bool	Template::Block::UsesLighting() const
			{
				if (type == LightLoop)
					return true;
				for (index_t i = 0; i < children.Count(); i++)
					if (children[i]->UsesLighting())
						return true;
				return false;
			}

			void	Template::Block::Assemble(StringBuffer&target, const UserConfiguration&userConfig, const RenderConfiguration&renderConfig, Light::Type type, index_t lightIndex)
			{
				for (index_t i = 0; i < innerLines.Count(); i++)
					if (innerLines[i].segments.Count())
					{
						const Array<String,Adopt>&segments = innerLines[i].segments;
						for (index_t j = 0; j < segments.Count()-1; j++)
						{
							target << segments[j] << lightIndex;
						}
						target << segments.Last()<<nl;
						/*if (innerLines[i].includes)
							innerLines[i].includes->Block::Assemble(target,userConfig,renderConfig,type,lightIndex);*/
					}
				bool isElse = false;
				for (index_t i = 0; i < children.Count(); i++)
				{
					PBlock child = children[i];
					bool doAssemble=false;
					switch (child->type)
					{
						case Any:
							doAssemble = true;
							isElse = false;
						break;
						case Conditional:
							doAssemble = child->condition && child->condition->Evaluate(userConfig,type);
							isElse = !doAssemble;
						break;
						case ElseConditional:
							doAssemble = isElse && child->condition && child->condition->Evaluate(userConfig,type);
							isElse = isElse && !doAssemble;
						break;
						case Else:
							doAssemble = isElse;
							isElse = false;
						break;
						case LightLoop:
							doAssemble = true;
							isElse = false;
						break;
					}
					if (doAssemble)
					{
						if (child->type == LightLoop)
						{
							for (index_t j = 0; j < renderConfig.lights.Count(); j++)
								if (renderConfig.lights[j] != Light::None)
									child->Assemble(target,userConfig,renderConfig,renderConfig.lights[j],j);
						}
						else
							child->Assemble(target,userConfig,renderConfig,type,lightIndex);
					}
					for (index_t k = 0; k < child->trailingLines.Count(); k++)
						if (child->trailingLines[k].segments.Count())
						{
							const Array<String,Adopt>&segments = child->trailingLines[k].segments;
							for (index_t j = 0; j < segments.Count()-1; j++)
							{
								target << segments[j] << lightIndex;
							}
							target << segments.Last()<<nl;
						}
				}
			}
	
	
			Template::PExpression	Template::RootBlock::_ProcessLayer(TokenList&tokens,VariableMap&map, index_t begin, index_t end, String&error)
			{
				index_t level = 0,
						blockBegin;

		/* 		cout << "symbol chain is";
				for (unsigned i = begin; i < end; i++)
					cout << ' '<<tokens[i]->content<<"("<<tokens[i]->ident<<")";
				cout << endl;
		 */		
		
				Ctr::Vector0<PExpression>	expressions;
				for (index_t i = begin; i < end; i++)
				{
					const TToken&token = tokens[i];
					PExpression expression;
			
					switch (token.ident&Token::Mask)
					{
						case Token::PUp:
							if (!level)
								blockBegin = i+1;
							level++;
						break;
						case Token::PDown:
							level--;
							if (!level)
							{
								expression = _ProcessLayer(tokens,map,blockBegin,i,error);
								if (!expression)
									return NULL;
							}
						break;
					}
					if (!level && !expression)
						switch (token.ident&Token::Mask)
						{
							case Token::And:
								expression.reset(new AndExpression());
							break;
							case Token::Or:
								expression.reset(new OrExpression());
							break;
							case Token::Not:
								expression.reset(new NegationExpression());
							break;
							case Token::String:
							{
								int val;
								bool bval;
								bool convertable = true;
								if (convert(token.content.c_str(),bval))
									val = bval;
								else
									if (!convert(token.content.c_str(),val))
										convertable = false;
								if (convertable)
								{
									expression.reset(new ConstantExpression(val));
								}
								else
								{
									if (token.content == "omni")
									{
										expression.reset(new LightTypeExpression(Light::Omni));
									}
									elif (token.content == "direct")
									{
										expression.reset(new LightTypeExpression(Light::Direct));
									}
									elif (token.content == "spot")
									{
										expression .reset(new LightTypeExpression(Light::Spot));
									}
									else
									{
										expression.reset(new VariableExpression(token.content,map.Define(token.content)));
									}
								}
							}
							break;
							case Token::Plus:
								expression.reset(new SumExpression());
							break;
							case Token::Minus:
								expression.reset(new DifferenceExpression());
							break;
							case Token::Times:
								expression.reset(new ProductExpression());
							break;
							case Token::Divide:
								expression .reset(new QuotientExpression());
							break;
							case Token::Equals:
								expression .reset(new EqualExpression());
							break;
							case Token::Differs:
								expression .reset(new NotEqualExpression());
							break;
							case Token::Greater:
								expression .reset(new GreaterExpression());
							break;
							case Token::Less:
								expression .reset(new LessExpression());
							break;
							case Token::GreaterOrEqual:
								expression .reset(new GreaterOrEqualExpression());
							break;
							case Token::LessOrEqual:
								expression .reset(new LessOrEqualExpression());
							break;
						}
					if (expression)
					{
						expression->level = token.ident&Level::Mask;
						expressions.append(expression);
					}
				}
				for (unsigned level = 1; level <= 5; level++)
				{
					for (index_t i = 0; i < expressions.Count(); i++)
					{
						PExpression expression = expressions[i];
						if (expression->level == level*0x1000)
						{
							if (expression->AdoptRight(expressions[i+1]))
								expressions.Erase(i+1);
							if (i > 0 && expression->AdoptLeft(expressions[i-1]))
							{
								expressions.Erase(i-1);
								i--;
							}
						}
					}
				}
				if (expressions.Count() != 1)
				{
					error = "Expression parsing failed";
					return PExpression();
				}
				return expressions.First();
			}
	
			Template::PExpression	Template::RootBlock::_ParseCondition(const char*condition, VariableMap&map, String&error)
			{
				static StringTokenizer	tokenizer;
				static bool				initialized = false;
		

				if (!initialized)
				{
		
					tokenizer.setStringIdent(Token::String);
					tokenizer.reg("(",Token::PUp);
					tokenizer.reg(")",Token::PDown);
					tokenizer.reg("+",Token::Plus|Level::Five);
					tokenizer.reg("-",Token::Minus|Level::Five);
					tokenizer.reg("*",Token::Times|Level::Four);
					tokenizer.reg("/",Token::Divide|Level::Four);
					tokenizer.reg("&&",Token::And|Level::One);
					tokenizer.reg("||",Token::Or|Level::One);
					tokenizer.reg("==",Token::Equals|Level::Two);
					tokenizer.reg("!=",Token::Differs|Level::Two);
					tokenizer.reg(">",Token::Greater|Level::Three);
					tokenizer.reg("<",Token::Less|Level::Three);
					tokenizer.reg(">=",Token::GreaterOrEqual|Level::Three);
					tokenizer.reg("<=",Token::LessOrEqual|Level::Three);
					tokenizer.reg("!",Token::Not|Level::Zero);

					tokenizer.reg(" ",Token::Space);
					tokenizer.reg("  ",Token::Space);
					tokenizer.reg("   ",Token::Space);
					tokenizer.reg("\t",Token::Space);
					tokenizer.finalizeInitialization();

			
			
					initialized = true;
				}
		
				TokenList	tokens;
		
				tokenizer.parse(condition,tokens);
		
				PExpression result = _ProcessLayer(tokens,map,0,tokens.Count(),error);
				if (result && !result->Validate())
				{
					error = "Expression '"+result->ConvertToString()+"' failed to validate";
					return PExpression();
				}
				return result;
			}
	
			static void StripComments(String&line, bool&inComment)
			{
				const char	*c = line.c_str(),
							*commentStart = line.c_str();

				unsigned count = 0;
				while (*c)
				{
					if (inComment)
					{
						c = CharFunctions::strstr(c,"*/");
						if (c)
						{
							unsigned index = commentStart-line.c_str();
							line.Erase(commentStart-line.c_str(),c-commentStart+2);
							c = line.c_str();
							c+=index;
							inComment = false;
						}
						else
						{
							line.Erase(commentStart-line.c_str());
							return;
						}
					}
			
			
					c = CharFunctions::strchr(c,'/');
					if (!c)
						return;
					c++;
					if (*c == '/')	//line comment
					{
						line.Erase(c-line.c_str()-1);
						return;
					}
					if (*c == '*')
					{
						commentStart = c-1;
						inComment = true;
					}
				}
			}
	
			static void		LogLine(const Array<String,Adopt>&lines,index_t index, StringBuffer&logOut)
			{
				index_t begin = index >= 2?index-2:0,
						end = index+3 < lines.Count()?index+3:lines.Count();
				//logOut << "-------- context --------"<<nl;
				logOut << nl;
				if (begin)
					logOut << "     ..."<<nl;
				for (index_t i = begin; i < end; i++)
				{
					if (i == index)
						logOut << " >>> ";
					else
						logOut << "     ";
					logOut << lines[i];
			
					logOut<<nl;
				}
				if (end < lines.Count())
					logOut << "     ..."<<nl;
			}
	
			bool			Template::RootBlock::Scan(const String&source, VariableMap&map, StringBuffer&logOut, index_t&line)
			{
				shadeInvoked = source.ContainsWord("shade");
				shade2Invoked = source.ContainsWord("shade2");
				customShadeInvoked = source.ContainsWord("customShade");
				spotlightInvoked = source.ContainsWord("spotLight");
				omnilightInvoked = source.ContainsWord("omniLight");
				directlightInvoked = source.ContainsWord("directLight");

				root.reset(new Block());
				PBlock current = root;
				index_t blockBegin = 0;
				Array<String,Adopt>	lines;
		
				bool inComment = false;
				Explode('\n',source,lines);
		
		
		
				for (index_t i = 0; i < lines.Count(); i++)
				{
					StripComments(lines[i],inComment);
					lines[i].TrimThis();

					if (lines[i].BeginsWith("#include"))
					{
						String file = lines[i].subString(8).TrimThis();
						if (!file.GetLength())
						{
							logOut << "#include directive broken in line "<<(i+1)<<nl;
							LogLine(lines,i,logOut);					
							return false;
						}
						if ((file.FirstChar() == '"' && file.LastChar() == '"')
							||
							(file.FirstChar() == '<' && file.LastChar() == '>'))
							file = file.subString(1,file.GetLength()-2).TrimThis();
						String*include = FindShaderIncludable(file);
						if (!include)
						{
							logOut << "Unable to find '"<<file<<"' for inclusion in line "<<(i+1)<<nl;
							LogLine(lines,i,logOut);					
							return false;
						}
						Array<String,AdoptStrategy>	temp;
						Explode('\n',*include,temp);
						lines.Erase(i);
						lines.insertImport(i,temp);
					}
				}
				for (index_t i = 0; i < lines.Count(); i++)
				{
					const char*c = lines[i].c_str();
					while (*c && IsWhitespace(*c))
						c++;
					if (*c != '#')
						continue;
			

					c++;
					while (*c && IsWhitespace(*c))
						c++;
					const char*w = c;
					while (*c && !IsWhitespace(*c))
						c++;
					String	word(w,c-w);
			
					if (word == "version" || word == "extension" || word == "include" || word == "define")
						continue;
			
			
			
					Array<Line,Adopt>&target = current->children.IsNotEmpty()
												? current->children.Last()->trailingLines
												: current->innerLines;
			
		/* 			if (current->children)
						cout << "appending "<<(i-blockBegin)<<" line(s):"<<endl; */
					target.SetSize(i-blockBegin);
					for (index_t j = 0; j < target.size(); j++)
					{
						Explode(current->lightLoopConstant,lines[blockBegin+j],target[j].segments);
		/* 				if (current->children)
							cout << "  "<<lines[blockBegin+j]<<endl; */
					}
			
					blockBegin = i+1;
			

					if (word=="endlight")
					{
						if (current->parent && (current->type == Block::LightLoop))
							current = current->parent;
						else
						{
							logOut << "misplaced #"<<word<<" in line "<<(line+i+1)<<nl;
							if (current->parent && (current->type == Block::Conditional || current->type == Block::ElseConditional || current->type == Block::Else))
								logOut << "    (expected #endif)"<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
					}
					elif (word=="endif")
					{
						if (current->parent && (current->type == Block::Conditional || current->type == Block::ElseConditional || current->type == Block::Else))
							current = current->parent;
						else
						{
							logOut << "misplaced #"<<word<<" in line "<<(line+i+1)<<nl;
							if (current->parent && (current->type == Block::LightLoop))
								logOut << "    (expected #endlight)"<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
					}
					elif (word == "if")
					{
						if (*c)
							c++;
						/*...*/
						PBlock parent = current;
						current.reset(new Block());
						parent->children.append(current);
						current->parent = parent;
						current->lightLoopConstant = parent->lightLoopConstant;
						current->type = Block::Conditional;
						String error;
						current->condition = _ParseCondition(c,map,error);
						if (!current->condition)
						{
							logOut << "Condition parse error '"<<error <<"' in line "<< (line+i+1)<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
			
					}
					elif (word == "elif")
					{
						if (*c)
							c++;
						if (!current->parent)
						{
							logOut << "misplaced #"<<word<<" in line "<<(line+i+1)<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
				
						{
							PBlock predecessor = current->parent->children.Last();
							if (!predecessor || (predecessor->type != Block::Conditional && predecessor->type != Block::ElseConditional))
							{
								logOut << "misplaced #"<<word<<" in line "<<(line+i+1)<<nl;
								LogLine(lines,i,logOut);
								return false;
							}
						}
						PBlock parent = current->parent;
						current.reset(new Block());
						parent->children.append(current);
						current->parent = parent;
						current->lightLoopConstant = parent->lightLoopConstant;
						current->type = Block::ElseConditional;
						String  error;
						current->condition = _ParseCondition(c,map,error);
						if (!current->condition)
						{
							logOut << "Condition parse error '"<<error <<"' in line "<< (line+i+1)<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
							
					}
					elif (word == "else")
					{
						if (!current->parent)
						{
							logOut << "misplaced #"<<word<<" in line "<<(line+i+1)<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
				
						{
							PBlock predecessor = current->parent->children.Last();
							if (!predecessor || (predecessor->type != Block::Conditional && predecessor->type != Block::ElseConditional))
							{
								logOut << "misplaced #"<<word<<" in line "<<(line+i+1)<<nl;
								LogLine(lines,i,logOut);
								return false;
							}
						}
						PBlock parent = current->parent;
						current.reset(new Block());
						parent->children.Append(current);
						current->parent = parent;
						current->lightLoopConstant = parent->lightLoopConstant;
						current->type = Block::Else;
					}
					elif (word.BeginsWith("light"))
					{
						if (*c)
							c++;
						PBlock parent = current;
						current.reset(new Block());
						parent->children.Append(current);
						current->parent = parent;
						current->type = Block::LightLoop;
						current->lightLoopConstant = c;
						current->lightLoopConstant.TrimThis();
						if (current->lightLoopConstant.FirstChar() != '<' || current->lightLoopConstant.LastChar() != '>')
						{
							logOut << "illformatted lightloop variable '"<<current->lightLoopConstant<<"' found in line "<<(line+i+1)<<". Variables must be formatted '<NAME>'."<<nl;
							LogLine(lines,i,logOut);
							return false;
						}
					}
					else
					{
						logOut << "unsupported precompiler directive '#"<<word<<"' found in line "<<(line+i+1)<<nl;
						LogLine(lines,i,logOut);
						return false;
					}
				}
				if (current != root)
				{
					logOut << "missing #endif directive at line "<<lines.Count()<<nl;
					LogLine(lines,lines.Count()-1,logOut);
					return false;
				}
		
				{
					Array<Line,Adopt>&target = current->children.IsNotEmpty()
												? current->children.Last()->trailingLines
												: current->innerLines;
			
					target.SetSize(lines.Count()-blockBegin);
					for (index_t j = 0; j < target.size(); j++)
					{
						Explode(current->lightLoopConstant,lines[blockBegin+j],target[j].segments);
					}
				}
		
				line += lines.Count()-1;
				return true;
			}
	
	
	
	
			String			Template::RootBlock::Assemble(const RenderConfiguration&renderConfig,const UserConfiguration&userConfig, bool isShared)
			{
				static StringBuffer	buffer;
				buffer.reset();
				Assemble(renderConfig, userConfig,buffer,isShared);
				return buffer.ToStringRef();
			}
	

			bool			Template::RootBlock::UsesLighting()	const
			{
				if (shadeInvoked || shade2Invoked || customShadeInvoked)
					return true;
				return root && root->UsesLighting();
			}
		
			/*static*/ void		Template::RootBlock::_ShadowFunction(index_t level,StringBuffer&buffer)
			{
				if (level >= shadowAttachments.size() || shadowAttachments[level].IsEmpty())
					buffer << "1.0";
				else
					buffer << "fragmentShadow"<<level<<"(position)";
			}

			void			Template::RootBlock::Assemble(const RenderConfiguration&renderConfig, const UserConfiguration&userConfig,StringBuffer&buffer, bool isShared/*=false*/)
			{
				if (isShared)
				{
					for (index_t i = 0; i < shadowAttachments.Count(); i++)
						if (!shadowAttachments[i].IsEmpty())
						{
							buffer << nl;
							if (shadowAttachments[i].sharedAttachment.IsNotEmpty())
								buffer << shadowAttachments[i].sharedAttachment<<nl;
							buffer << "float fragmentShadow"<<i<<"(vec3 position)"<<nl
									<< '{'<<nl
									<< shadowAttachments[i].fragmentShadowCode
									<< '}'<<nl;
							FATAL__("Unsupported");
						}
					for (index_t i = shadowAttachments.Count(); i < renderConfig.lights.Count(); i++)
							buffer << "float fragmentShadow"<<i<<"(vec3 position)"<<nl
									<< '{'<<nl
									<< "return 1.0;"<<nl
									<< '}'<<nl;

					//buffer << nl << "void vertexShadow()\n{\n";
					//for (index_t i = 0; i < shadowAttachments.size(); i++)
					//	if (!shadowAttachments[i].IsEmpty())
					//	{
					//		if (shadowAttachments[i].vertex_shader_attachment.IsNotEmpty())
					//			buffer << shadowAttachments[i].vertex_shader_attachment<<nl;
					//	}
					//buffer << "}\n";
				}
				if (shadeInvoked)
				{
					buffer << nl <<
					"vec4 shade(vec3 position, vec3 normal, vec3 reflected)\n"
					"{\n";
					if (renderConfig.lightingEnabled)
					{
						buffer <<
						"	vec3 result = vec3(0.0);\n"
						"	float fresnel = 1.0-abs(normal.z)*0.5;\n";
						for (index_t i = 0; i < renderConfig.lights.Count(); i++)
							switch (renderConfig.lights[i])
							{
								case Light::Omni:
									buffer << 
									"	{\n"
									"		vec3 ldir = (gl_LightSource["<<i<<"].position.xyz-position);\n"
									"		float distance = length(ldir);\n"
									"		ldir /= distance;\n"
									"		float shadow = ";
									_ShadowFunction(i,buffer);
									buffer <<";\n"
									"		float attenuation = 1.0 / (gl_LightSource["<<i<<"].constantAttenuation + gl_LightSource["<<i<<"].linearAttenuation * distance + gl_LightSource["<<i<<"].quadraticAttenuation * distance*distance);\n"
									"		result += max(dot(normal,ldir),0.0)*gl_FrontLightProduct["<<i<<"].diffuse.rgb*attenuation*shadow;\n"
									"		if (gl_FrontMaterial.shininess > 0.0)\n"
									"			result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_FrontLightProduct["<<i<<"].specular.rgb*attenuation*fresnel*shadow;\n"
									"		result += gl_FrontLightProduct["<<i<<"].ambient.rgb*attenuation;\n"
									"	}\n";
								break;
								case Light::Direct:
									buffer << 
									"	{\n"
									"		vec3 ldir = gl_LightSource["<<i<<"].position.xyz;\n"
									"		float shadow = ";
													_ShadowFunction(i,buffer);
													buffer <<";\n"
									"		result += max(dot(normal,ldir),0.0)*gl_FrontLightProduct["<<i<<"].diffuse.rgb*shadow;\n"
									"		if (gl_FrontMaterial.shininess > 0.0)\n"
									"			result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_FrontLightProduct["<<i<<"].specular.rgb*fresnel*shadow;\n"
									"		result += gl_FrontLightProduct["<<i<<"].ambient.rgb;\n"
									"	}\n";
								break;
								case Light::Spot:
									buffer <<
									"	{\n"
									"		vec3 ldir = (gl_LightSource["<<i<<"].position.xyz-position);\n"
									"		float general_intensity = -dot(gl_LightSource["<<i<<"].spotDirection,ldir);\n"
									"		if (general_intensity > 0.0)\n"
									"		{\n"
									"			float diffuse = dot(normal,ldir);\n"
									"			if (diffuse > 0.0)\n"
									"			{\n"
									"				float distance = length(ldir);\n"
									"				ldir /= distance;\n"
									"				general_intensity /= distance;\n"
									"				diffuse /= distance;\n"
									"				float shadow = ";
														_ShadowFunction(i,buffer);
														buffer <<";\n"
									"				float attenuation = 1.0 / (gl_LightSource["<<i<<"].constantAttenuation + gl_LightSource["<<i<<"].linearAttenuation * distance + gl_LightSource["<<i<<"].quadraticAttenuation * distance*distance);\n"
									"				float cone_intensity = pow(general_intensity,gl_LightSource["<<i<<"].spotExponent)*attenuation;\n"
									"				diffuse *= cone_intensity;\n"
									"				if (gl_FrontMaterial.shininess > 0.0)\n"
									"				{\n"
									"					float specular = pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*cone_intensity*fresnel*shadow;\n"
									"					result += specular * gl_FrontLightProduct["<<i<<"].specular.rgb;\n"
									"				}\n"
									"				result += diffuse * gl_FrontLightProduct["<<i<<"].diffuse.rgb*shadow;\n"
									"				result += attenuation * gl_FrontLightProduct["<<i<<"].ambient.rgb;\n"
									"			}\n"
									"		}\n"
									"	}\n";				
								break;
							}
						buffer << "	return vec4(result,1.0);\n";
					}
					else
						buffer << " return vec4(1.0);\n";
					buffer << "}\n\n";
				}


				if (shade2Invoked)
				{
					buffer << nl <<
					"vec4 shade2(vec3 position, vec3 eye_direction, vec3 normal, vec3 reflected)\n"
					"{\n";
					if (renderConfig.lightingEnabled)
					{
						buffer <<
						"	vec3 result = vec3(0.0);\n"
						"	float fresnel = 1.0-abs(dot(normal,eye_direction))*0.5;\n";
						for (index_t i = 0; i < renderConfig.lights.Count(); i++)
							switch (renderConfig.lights[i])
							{
								case Light::Omni:
									buffer << 
									"	{\n"
									"		vec3 ldir = (gl_LightSource["<<i<<"].position.xyz-position);\n"
									"		float distance = length(ldir);\n"
									"		ldir /= distance;\n"
									"		float shadow = ";
									_ShadowFunction(i,buffer);
									buffer <<";\n"
									"		float attenuation = 1.0 / (gl_LightSource["<<i<<"].constantAttenuation + gl_LightSource["<<i<<"].linearAttenuation * distance + gl_LightSource["<<i<<"].quadraticAttenuation * distance*distance);\n"
									"		result += max(dot(normal,ldir),0.0)*gl_FrontLightProduct["<<i<<"].diffuse.rgb*attenuation*shadow;\n"
									"		if (gl_FrontMaterial.shininess > 0.0)\n"
									"			result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_FrontLightProduct["<<i<<"].specular.rgb*attenuation*fresnel*shadow;\n"
									"		result += gl_FrontLightProduct["<<i<<"].ambient.rgb*attenuation;\n"
									"	}\n";
								break;
								case Light::Direct:
									buffer << 
									"	{\n"
									"		vec3 ldir = gl_LightSource["<<i<<"].position.xyz;\n"
									"		float shadow = ";
													_ShadowFunction(i,buffer);
													buffer <<";\n"
									"		result += max(dot(normal,ldir),0.0)*gl_FrontLightProduct["<<i<<"].diffuse.rgb*shadow;\n"
									"		if (gl_FrontMaterial.shininess > 0.0)\n"
									"			result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_FrontLightProduct["<<i<<"].specular.rgb*fresnel*shadow;\n"
									"		result += gl_FrontLightProduct["<<i<<"].ambient.rgb;\n"
									"	}\n";
								break;
								case Light::Spot:
									buffer <<
									"	{\n"
									"		vec3 ldir = (gl_LightSource["<<i<<"].position.xyz-position);\n"
									"		float general_intensity = -dot(gl_LightSource["<<i<<"].spotDirection,ldir);\n"
									"		if (general_intensity > 0.0)\n"
									"		{\n"
									"			float diffuse = dot(normal,ldir);\n"
									"			if (diffuse > 0.0)\n"
									"			{\n"
									"				float distance = length(ldir);\n"
									"				ldir /= distance;\n"
									"				general_intensity /= distance;\n"
									"				diffuse /= distance;\n"
									"				float shadow = ";
														_ShadowFunction(i,buffer);
														buffer <<";\n"
									"				float attenuation = 1.0 / (gl_LightSource["<<i<<"].constantAttenuation + gl_LightSource["<<i<<"].linearAttenuation * distance + gl_LightSource["<<i<<"].quadraticAttenuation * distance*distance);\n"
									"				float cone_intensity = pow(general_intensity,gl_LightSource["<<i<<"].spotExponent)*attenuation;\n"
									"				diffuse *= cone_intensity;\n"
									"				if (gl_FrontMaterial.shininess > 0.0)\n"
									"				{\n"
									"					float specular = pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*cone_intensity*fresnel*shadow;\n"
									"					result += specular * gl_FrontLightProduct["<<i<<"].specular.rgb;\n"
									"				}\n"
									"				result += diffuse * gl_FrontLightProduct["<<i<<"].diffuse.rgb*shadow;\n"
									"				result += attenuation * gl_FrontLightProduct["<<i<<"].ambient.rgb;\n"
									"			}\n"
									"		}\n"
									"	}\n";				
								break;
							}
						buffer << "	return vec4(result,1.0);\n";
					}
					else
						buffer << " return vec4(1.0);\n";
					buffer << "}\n\n";
				}
		
		
				if (customShadeInvoked)
				{
					buffer << nl <<
					"vec4 customShade(vec3 position, vec3 normal, vec3 reflected, vec3 ambient, vec3 diffuse, vec3 specular)\n"
					"{\n";
					if (renderConfig.lightingEnabled)
					{
						buffer <<
						"	vec3 result = vec3(0.0);\n"
						"	float fresnel = 1.0-abs(normal.z)*0.5;\n";
						for (index_t i = 0; i < renderConfig.lights.Count(); i++)
							switch (renderConfig.lights[i])
							{
								case Light::Omni:
									buffer << 
									"	{\n"
									"		vec3 ldir = (gl_LightSource["<<i<<"].position.xyz-position);\n"
									"		float distance = length(ldir);\n"
									"		ldir /= distance;\n"
									"		float shadow = ";
													_ShadowFunction(i,buffer);
													buffer <<";\n"
									"		float attenuation = 1.0 / (gl_LightSource["<<i<<"].constantAttenuation + gl_LightSource["<<i<<"].linearAttenuation * distance + gl_LightSource["<<i<<"].quadraticAttenuation * distance*distance);\n"
									"		result += max(dot(normal,ldir),0.0)*gl_LightSource["<<i<<"].diffuse.rgb*diffuse*attenuation*shadow;\n"
									"		if (gl_FrontMaterial.shininess > 0.0)\n"
									"			result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_LightSource["<<i<<"].specular.rgb*specular*attenuation*fresnel*shadow;\n"
									"		result += gl_LightSource["<<i<<"].ambient.rgb*ambient*attenuation;\n"
									"	}\n";
								break;
								case Light::Direct:
									buffer << 
									"	{\n"
									"		vec3 ldir = gl_LightSource["<<i<<"].position.xyz;\n"
									"		float shadow = ";
													_ShadowFunction(i,buffer);
													buffer <<";\n"
									"		result += max(dot(normal,ldir),0.0)*gl_LightSource["<<i<<"].diffuse.rgb*diffuse*shadow;\n"
									"		if (gl_FrontMaterial.shininess > 0.0)\n"
									"			result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_LightSource["<<i<<"].specular.rgb*specular*fresnel*shadow;\n"
									"		result += gl_LightSource["<<i<<"].ambient.rgb*ambient;\n"
									"	}\n";
								break;
								case Light::Spot:
									buffer <<
									"	{\n"
									"		vec3 ldir = (gl_LightSource["<<i<<"].position.xyz-position);\n"
									"		float general_intensity = -dot(gl_LightSource["<<i<<"].spotDirection,ldir);\n"
									"		if (general_intensity > 0.0)\n"
									"		{\n"
									"			float shadow = ";
													_ShadowFunction(i,buffer);
													buffer <<";\n"
									"			float diff = dot(normal,ldir);\n"
									"			if (diff > 0.0)\n"
									"			{\n"
									"				float distance = length(ldir);\n"
									"				ldir /= distance;\n"
									"				general_intensity /= distance;\n"
									"				diff /= distance;\n"
									"				float attenuation = 1.0 / (gl_LightSource["<<i<<"].constantAttenuation + gl_LightSource["<<i<<"].linearAttenuation * distance + gl_LightSource["<<i<<"].quadraticAttenuation * distance*distance);\n"
									"				float cone_intensity = pow(general_intensity,gl_LightSource["<<i<<"].spotExponent)*attenuation;\n"
									"				diff *= cone_intensity;\n"
									"				if (gl_FrontMaterial.shininess > 0.0)\n"
									"				{\n"
									"					float spec = pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*cone_intensity*fresnel*shadow;\n"
									"					result += spec * gl_LightSource["<<i<<"].specular.rgb*specular;\n"
									"				}\n"
									"				result += diff * gl_LightSource["<<i<<"].diffuse.rgb*diffuse*shadow;\n"
									"				result += attenuation * gl_LightSource["<<i<<"].ambient.rgb*ambient;\n"
									"			}\n"
									"		}\n"
									"	}\n";				
								break;
							}
						buffer << "	return vec4(result,1.0);\n";
					}
					else
						buffer << " return vec4(1.0);\n";
					buffer << "}\n\n";
				}
		
		
				if (spotlightInvoked)
				buffer <<
					"vec4 spotLight(int index, vec3 position, vec3 normal, vec3 reflected, float shadow)\n"
					"{\n"
					"	float fresnel = 1.0-abs(normal.z)*0.5;\n"
					"	vec4 result = vec4(0.0);\n"
					"	vec3 ldir = (gl_LightSource[index].position.xyz-position);\n"
					"	float general_intensity = -dot(gl_LightSource[index].spotDirection,ldir);\n"
					"	if (general_intensity > 0.0)\n"
					"	{\n"
					"		float diffuse = dot(normal,ldir);\n"
					"		if (diffuse > 0.0)\n"
					"		{\n"
					"			float distance = length(ldir);\n"
					"			ldir /= distance;\n"
					"			general_intensity /= distance;\n"
					"			diffuse /= distance;\n"
					"			float attenuation = 1.0 / (gl_LightSource[index].constantAttenuation + gl_LightSource[index].linearAttenuation * distance + gl_LightSource[index].quadraticAttenuation * distance*distance);\n"
					"			float cone_intensity = pow(general_intensity,gl_LightSource[index].spotExponent)*attenuation;\n"
					"			diffuse *= cone_intensity;\n"
					"			if (gl_FrontMaterial.shininess > 0.0)\n"
					"			{\n"
					"				float specular = pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*cone_intensity*fresnel*shadow;\n"
					"				result += specular * gl_FrontLightProduct[index].specular;\n"
					"			}\n"
					"			result += diffuse * gl_FrontLightProduct[index].diffuse*shadow;\n"
					"			result += attenuation * gl_FrontLightProduct[index].ambient;\n"
					"		}\n"
					"	}\n"
					"	return result;\n"
					"}\n\n";				

				if (directlightInvoked)
				buffer <<
					"vec4 directLight(int index, vec3 position, vec3 normal, vec3 reflected, float shadow)\n"
					"{\n"
					"	float fresnel = 1.0-abs(normal.z)*0.5;\n"
					"	vec4 result = vec4(0.0);\n"
					"	vec3 ldir = gl_LightSource[index].position.xyz;\n"
					"	result += max(dot(normal,ldir),0.0)*gl_FrontLightProduct[index].diffuse*shadow;\n"
					"	if (gl_FrontMaterial.shininess > 0.0)\n"
					"		result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_FrontLightProduct[index].specular*fresnel*shadow;\n"
					"	result += gl_FrontLightProduct[index].ambient;\n"
					"	return result;\n"
					"}\n\n";				

				if (omnilightInvoked)
				buffer <<
					"vec4 omniLight(int index, vec3 position, vec3 normal, vec3 reflected, float shadow)\n"
					"{\n"
					"	float fresnel = 1.0-abs(normal.z)*0.5;\n"
					"	vec4 result = vec4(0.0);\n"
					"	vec3 ldir = (gl_LightSource[index].position.xyz-position);\n"
					"	float distance = length(ldir);\n"
					"	ldir /= distance;\n"
					"	float attenuation = 1.0 / (gl_LightSource[index].constantAttenuation + gl_LightSource[index].linearAttenuation * distance + gl_LightSource[index].quadraticAttenuation * distance*distance);\n"
					"	result += max(dot(normal,ldir),0.0)*gl_FrontLightProduct[index].diffuse*attenuation*shadow;\n"
					"	if (gl_FrontMaterial.shininess > 0.0)\n"
					"		result += pow(max(dot(reflected,ldir),0.0),gl_FrontMaterial.shininess*fresnel)*gl_FrontLightProduct[index].specular*attenuation*fresnel*shadow;\n"
					"	result += gl_FrontLightProduct[index].ambient*attenuation;\n"
					"	return result;\n"
					"}\n\n";				

	
	
				root->Assemble(buffer,userConfig,renderConfig,Light::None,0);
			}
	
	
	
	
			Template::VariableMap::VariableMap():changed(true)
			{
				Clear();
			}
			
			void					Template::VariableMap::Clear()
			{
				variableMap.Clear();
				variableMap.Set("fog",FogVariableIndex);
				variableMap.Set("lighting",LightingVariableIndex);
				changed = true;
			}

			Template::VariableMap::~VariableMap()
			{
				if (!application_shutting_down)
				{
					for (index_t i = 0; i < attachedConfigurations.Count(); i++)
						attachedConfigurations[i]->SignalMapDestruction();
				}

			}


	
	
	
	/*
			void		Configuration::toArray(Array<int>&target)	const
			{
				target.Resize(lights.Count()+values.Count());
				for (unsigned i = 0; i < lights.Count(); i++)
					target[i] = lights[i];
				for (unsigned i = 0; i < values.Count(); i++)
					target[i+lights.Count()] = values[i];
			}*/
	
			void		Template::RenderConfiguration::SetLights(count_t count,...)
			{
				va_list vl;
				va_start(vl,count);
				lights.reset();
				for (index_t i = 0; i < count; i++)
				{
					int val=va_arg(vl,int);
					lights << (Light::Type)val;
				}
				va_end(vl);
				SignalHasChanged();
			}
	
			void		Template::RenderConfiguration::SetLighting(bool lighting)
			{
				if (lightingEnabled != lighting)
				{
					lightingEnabled = lighting;
					SignalHasChanged();
				}
			}


			Template::RenderConfiguration::RenderConfiguration():lightingEnabled(false),fogEnabled(false)
			{}
	
			Template::UserConfiguration::UserConfiguration():map(NULL)
			{
				ResetAllValues();
			}

			void Template::RenderConfiguration::Clear()
			{
				if (lights.Count()!=0 || lightingEnabled || fogEnabled)
				{
					lights.reset();
					lightingEnabled = false;
					fogEnabled = false;
					SignalHasChanged();
				}
			}

	
			void	Template::UserConfiguration::Clear()
			{
				if (map)
					map->Unreg(this);
				map = NULL;
				if (values.Count() != 2 || values[0] != 0 || values[1] != 0)
				{
					values.SetSize(2);
					values[0] = 0;
					values[1] = 0;
					SignalHasChanged();
				}
			}

			void		Template::UserConfiguration::SignalMapDestruction()
			{
				map = NULL;
			}

	
			void		Template::RenderConfiguration::ReDetect()
			{
				lights.reset();
				lightingEnabled = glIsEnabled(GL_LIGHTING)!=0;
				fogEnabled = glIsEnabled(GL_FOG)!=0;
				if (lightingEnabled)
					for (GLint i = 0; i < glExtensions.maxLights; i++)
					{
						GLint index = GL_LIGHT0+i;
						if (glIsEnabled(index))
						{
							float position[4];
							glGetLightfv(index,GL_POSITION,position);
							if (M::nearingZero(position[3]))
								lights << Light::Direct;
							else
							{
								float cutoff;
								glGetLightfv(index,GL_SPOT_CUTOFF,&cutoff);
								if (cutoff < 180-M::GetError<float>())
									lights << Light::Spot;
								else
									lights << Light::Omni;
							}
						}
						else
							lights << Light::None;
					}
				SignalHasChanged();
			}

			void	Template::UserConfiguration::UpdateRenderVariables(const RenderConfiguration&config)
			{
				if (values.IsEmpty())
					return;
				if (values[0] != (int)config.fogEnabled || (values.Count() > 1 && values[1] != (int)config.lightingEnabled))
				{
					values[FogVariableIndex-1] = config.fogEnabled;
					if (values.Count() > 1)
						values[LightingVariableIndex-1] = config.lightingEnabled;
					SignalHasChanged();
				}
			}

			void	Template::UserConfiguration::UpdateRenderVariables()
			{
				UpdateRenderVariables(globalRenderConfig);
			}

			void	Template::VariableMap::SubmitChanges()
			{
				if (changed)
				{
					for (index_t i = 0; i < attachedConfigurations.Count(); i++)
						attachedConfigurations[i]->Adapt();
					changed = false;
				}
			}
	
			index_t	Template::VariableMap::Lookup(const String&varName)	const
			{
				index_t result;
				if (variableMap.Query(varName,result))
					return result;
				return 0;
			}
	
			index_t	Template::VariableMap::Define(const String&varName)
			{
				index_t result;
				if (variableMap.Query(varName,result))
					return result;
				result = variableMap.Count()+1;
				variableMap.Set(varName,result);
				changed = true;
				return result;
			}

	
			bool		Template::UserConfiguration::Set(index_t variable, int value)
			{
				index_t index = variable-1;
				if (index >= values.Count())
					return false;
				if (values[index] != value)
				{
					values[index] = value;
					SignalHasChanged();
				}
				return true;
			}
		
			bool		Template::UserConfiguration::SetByName(const String&varName, int value)
			{
				if (!map)
					return false;
				index_t index = map->Lookup(varName)-1;
				if (index >= values.Count())
					return false;
				if (values[index] != value)
				{
					values[index] = value;
					SignalHasChanged();
				}
				return true;
			}
	
			int			Template::UserConfiguration::GetValue(index_t variable)	const
			{
				index_t index = variable-1;
				if (index >= values.Count())
					return 0;
				return values[index];
			}

			int			Template::UserConfiguration::GetValueByName(const String&varName)	const
			{
				if (!map)
					return 0;
				index_t index = map->Lookup(varName)-1;
				if (index >= values.Count())
					return 0;
				return values[index];
			}
	

	
			void		Template::UserConfiguration::ResetAllValues()
			{
				values.Fill(0);
			}



			Template::UserConfiguration::~UserConfiguration()
			{
				if (!application_shutting_down && map)
					map->Unreg(this);
			}


			Template::VariableMap*	Template::UserConfiguration::GetVariableMap()
			{
				return map;
			}
		
			const Template::VariableMap*		Template::UserConfiguration::GetVariableMap() const								//!< Retrieves the map currently associated with this configuration, if any @return variable map or NULL if no such exists
			{
				return map;
			}

			void					Template::UserConfiguration::Adapt()
			{
				if (!map)
				{
					Clear();
					return;
				}

				count_t old = values.Count();
				values.ResizePreserveContent(map->variableMap.Count());
				for (index_t i = old; i < values.Count(); i++)
					values[i] = 0;
				if (old != values.Count())
					SignalHasChanged();
			}

			void					Template::UserConfiguration::Link(VariableMap*newMap, bool Adapt)
			{
				if (!this)
					return;
				if (!newMap)
				{
					Clear();
					return;
				}
				if (map == newMap)
				{
					if (Adapt)
					{
						count_t old = values.Count();
						values.ResizePreserveContent(map->variableMap.Count());
						for (index_t i = old; i < values.Count(); i++)
							values[i] = 0;
						if (old != values.Count())
							SignalHasChanged();
					}

					return;
				}
				if (map)
					map->Unreg(this);
				map = newMap;
				if (map)
					map->Reg(this);
				values.SetSize(map->variableMap.Count());
				values.Fill(0);

				SignalHasChanged();
			}

	
			static Ctr::HashContainer<String>	globalShaderIncludables;



			Template::ConfigurationComponent::~ConfigurationComponent()
			{
				if (!application_shutting_down)
					for (index_t i = 0; i < linkedConfigs.Count(); i++)
						linkedConfigs[i]->SignalComponentDestroyed(this);
			}

			void	Template::ConfigurationComponent::Unreg(Configuration*config)
			{
				linkedConfigs.FindAndErase(config);
			}

			void	Template::ConfigurationComponent::Reg(Configuration*config)
			{
				linkedConfigs.Append(config);
			}

			void	Template::ConfigurationComponent::SignalHasChanged()
			{
				version++;
			}


			bool	Template::Configuration::RequiresUpdate()	const
			{
				return structureChanged || (userConfig!=NULL && userConfigVersion != userConfig->version) || (renderConfig!=NULL && renderConfigVersion != renderConfig->version);
			}

			bool	Template::Configuration::Update()
			{

				if (valid && RequiresUpdate())
				{
					if (!registered)
					{
						userConfig->Reg(this);
						renderConfig->Reg(this);
						registered = true;
					}
					SetSize(renderConfig->lights.Count()+userConfig->values.Count());
					for (unsigned i = 0; i < renderConfig->lights.Count(); i++)
						data[i] = renderConfig->lights[i];
					for (unsigned i = 0; i < userConfig->values.Count(); i++)
						data[i+renderConfig->lights.Count()] = userConfig->values[i];
					structureChanged = false;
					userConfigVersion = userConfig->version;
					renderConfigVersion = renderConfig->version;
					return true;
				}
				return false;

			}

			void	Template::Configuration::SignalComponentDestroyed(ConfigurationComponent*component)
			{
				if (component == userConfig)
					userConfig = NULL;
				elif (component == renderConfig)
					renderConfig = NULL;
				Unlink();
			}

			Template::Configuration::~Configuration()
			{
				if (!application_shutting_down)
					Unlink();
			}

			void Template::Configuration::Unlink()
			{
				if (userConfig)
				{
					if (registered)
						userConfig->Unreg(this);
					userConfig = NULL;
				}
				if (renderConfig)
				{
					if (registered)
						renderConfig->Unreg(this);
					renderConfig = NULL;
				}
				valid = false;
				structureChanged = true;
			}

			void	Template::Configuration::Link(RenderConfiguration&newRenderConfig, UserConfiguration&newUserConfig)
			{
				if (&newRenderConfig == renderConfig && &newUserConfig == userConfig)
					return;
				Unlink();
				userConfig = &newUserConfig;
				renderConfig = &newRenderConfig;
				userConfig->Reg(this);
				renderConfig->Reg(this);
				valid = true;
				structureChanged = true;
				userConfigVersion = userConfig->version;
				renderConfigVersion = renderConfig->version;
				registered = true;
			}

			void	Template::Configuration::Link(RenderConfiguration&newRenderConfig)
			{
				if (renderConfig == &newRenderConfig)
					return;
				if (registered && renderConfig!=NULL)
					renderConfig->Unreg(this);
				renderConfig = &newRenderConfig;
				if (registered)
					renderConfig->Reg(this);
				valid = userConfig != NULL;
				renderConfigVersion = renderConfig->version;
				structureChanged = true;
			}

			void	Template::Configuration::Link(UserConfiguration&newUserConfig)
			{
				if (userConfig == &newUserConfig)
					return;
				if (registered && userConfig!=NULL)
					userConfig->Unreg(this);
				userConfig = &newUserConfig;
				if (registered)
					userConfig->Reg(this);
				valid = renderConfig != NULL;
				userConfigVersion = userConfig->version;
				structureChanged = true;
			}

				
			/*static*/ String*		Template::FindShaderIncludable(const String&filename)
			{
				String*result = globalShaderIncludables.lookup(filename);
				if (result)
					return result;
				static StringBuffer	dummy;
				dummy.reset();
				String content;
				if (!_ExtractFileContent(filename,content,dummy))
					return NULL;
				result = globalShaderIncludables.define(filename);
				(*result) = content;
				return result;
			}
	
			/*static*/ void		Template::DefineShaderIncludable(const String&filename, const String&blockCode)
			{
				(*globalShaderIncludables.define(filename)) = blockCode;
			}
	
			void				Template::Clear()
			{
				localMap.Clear();
				localUserConfig.Link(currentMap);
				usesLighting = false;
				

				sharedTemplate.Clear();
				vertexTemplate.Clear();
				fragmentTemplate.Clear();
				geometryTemplate.Clear();
				container.Clear();
				isLoaded = false;
				uniformInit.reset();
			}
	

	
			Template&				Template::PredefineUniformInt(const String&name, int value)
			{
				TUniformInit&init = uniformInit.Append();
				init.name = name;
				init.type = TUniformInit::Int;
				init.ival = value;
				return *this;
			}
	
			Template&				Template::PredefineUniformFloat(const String&name, float value)
			{
				TUniformInit&init = uniformInit.Append();
				init.name = name;
				init.type = TUniformInit::Float;
				init.fval = value;
				return *this;
			}

			Template&				Template::PredefineUniformVec(const String&name, const M::TVec3<>&value)
			{
				TUniformInit&init = uniformInit.Append();
				init.name = name;
				init.type = TUniformInit::Float3;
				init.f3val = value;
				return *this;
			}

			void				Template::LoadRequired(const Composition&composition, GLenum geometryType, GLenum outputType, unsigned maxVertices)
			{
				if (!Load(composition,geometryType,outputType,maxVertices))
					FATAL__(Report());
			}

			void				Template::LoadFromFile(const String&filename, GLenum geometryType/*=GL_TRIANGLES*/, GLenum outputType/*=GL_TRIANGLE_STRIP*/, unsigned maxVertices/*=12*/)
			{
				Composition composition;
				if (!composition.LoadFromFile(filename))
					throw Except::IO::DriveAccess::FileOpenFault("Unable to open file \""+filename+"\"");
				if (!Load(composition,geometryType,outputType,maxVertices))
					throw Except::Renderer::ShaderRejected(Report());
			}
	
			bool				Template::Load(const Composition&composition, GLenum geometryType, GLenum outputType, unsigned maxVertices)
			{
				Clear();
				isLoaded = true;
				index_t line = 0;
				if (isLoaded)
				{
					log << "-------- scanning shared source --------"<<nl;
					isLoaded = sharedTemplate.Scan(composition.sharedSource,*currentMap,log,line);
					if (!isLoaded)
						log << "  failed.\n";
				}
				if (isLoaded)
				{
					log << "-------- scanning vertex shader --------"<<nl;
					isLoaded = vertexTemplate.Scan(composition.vertexSource,*currentMap,log,line);
					if (!isLoaded)
						log << "  failed.\n";
				}
				if (isLoaded)
				{
					log << "-------- scanning fragment shader --------"<<nl;
					isLoaded = fragmentTemplate.Scan(composition.fragmentSource,*currentMap,log,line);
					if (!isLoaded)
						log << "  failed.\n";
				}
				if (isLoaded)
				{
					log << "-------- scanning geometry shader --------"<<nl;
					isLoaded = geometryTemplate.Scan(composition.geometrySource,*currentMap,log,line);
					if (!isLoaded)
						log << "  failed.\n";
				}
				currentConfig->GetUserConfig()->Link(currentMap,false);
				currentMap->SubmitChanges();	//this indirectly triggers an Adapt() call. since the above Link() call makes sure that our user config is linked to currentMap, this will update our user config as well (just as any other linked user config)
				this->geometryType = geometryType;
				this->outputType = outputType;
				this->maxVertices = maxVertices;

				usesLighting = sharedTemplate.UsesLighting() || vertexTemplate.UsesLighting() || fragmentTemplate.UsesLighting() || geometryTemplate.UsesLighting();
				if (isLoaded)
					log << "  loading process finished gracefully.\n";
				return isLoaded;
			}
	
			void				Template::LoadRequiredComposition(const String&objectSource, GLenum geometryType, GLenum outputType, unsigned maxVertices)
			{
				if (!LoadComposition(objectSource,geometryType,outputType,maxVertices))
					FATAL__(Report());
			}

			bool				Template::LoadComposition(const String&objectSource, GLenum geometryType, GLenum outputType, unsigned maxVertices)
			{
				return Load(Composition().Load(objectSource),geometryType,outputType,maxVertices);
			}

			void		Template::SetConfig(UserConfig*config, bool update)
			{
				if (config)
					currentConfig->Link(*config);
				else
					currentConfig->Link(localUserConfig);
				if (update)
					currentConfig->GetUserConfig()->Link(currentMap);
			}

			void		Template::SetConfig(RenderConfig*config)
			{
				if (config)
					currentConfig->Link(*config);
				else
					currentConfig->Link(localUserConfig);
			}

			void		Template::SetConfig(Configuration*config)
			{
				if (config)
					currentConfig = config;
				else
					currentConfig = &localConfig;
			}


			void		Template::SetVariableMap(VariableMap*newMap, bool adjust)
			{
				if (newMap)
					currentMap = newMap;
				else
					currentMap = &localMap;
				if (adjust)
					currentConfig->GetUserConfig()->Link(currentMap);
			}


	
	
	
			String		Template::Report()
			{
				String rs	= log.ToStringRef();
				log.reset();
				return rs;
			}
	
			bool		Template::IsLoaded()	const
			{
				return this!=NULL && isLoaded;
			}
	
	
			GLShader::Instance*	Template::BuildShader(bool autoUpdateRenderVariables,bool*isNew)
			{
				if (!this)
					return NULL;
				if (autoUpdateRenderVariables /*&& usesLighting*/)
					currentConfig->UpdateRenderVariables();
				log.reset();
				if (isNew)
					*isNew = false;
				GLShader::Instance*result;

				const Array<int,Primitive>*key;
				if (usesLighting)
				{
					currentConfig->Update();
					key = currentConfig;
				}
				else
					key = &currentConfig->GetUserConfig()->values;

				if (container.Query(*key,result))
				{
					//cout << result->composition_code<<endl;
					return result;
				}
				GLShader::Instance	local;
				Composition	composition;

				const RenderConfig&renderConfig = *currentConfig->GetRenderConfig();
				const UserConfig&userConfig = *currentConfig->GetUserConfig();

				composition.sharedSource = sharedTemplate.Assemble(renderConfig, userConfig,true);
				composition.vertexSource = vertexTemplate.Assemble(renderConfig, userConfig,false);
				composition.fragmentSource = fragmentTemplate.Assemble(renderConfig, userConfig,false);
				composition.geometrySource = geometryTemplate.Assemble(renderConfig, userConfig,false);
				if (local.Load(composition,geometryType,outputType,maxVertices))
				{
					if (isNew)
						*isNew = true;
					result = container.define(*key);
					result->adoptData(local);
					log << local.Report();
			
					GLuint currentProgram = glGetHandle(GL_PROGRAM_OBJECT_ARB);
					glUseProgramObject(result->programHandle);
			
					for (index_t i = 0; i < uniformInit.Count(); i++)
					{
			
						GLint var	= glGetUniformLocation(result->programHandle,uniformInit[i].name.c_str());
						if (var == -1)
							continue;

						switch (uniformInit[i].type)
						{
							case TUniformInit::Int:
								glUniform1i(var,uniformInit[i].ival);
							break;
							case TUniformInit::Float:
								glUniform1f(var,uniformInit[i].fval);
							break;
							case TUniformInit::Float3:
								glUniform3fv(var,1,uniformInit[i].f3val.v);
							break;
						}
					}
					foreach (shadowAttachments,attachment)
						foreach (attachment->samplerAssignments, a)
						{
							GLint var	= glGetUniformLocation(result->programHandle,a->samplerName.c_str());
							glUniform1i(var,a->samplerLevel);
						}
					glUseProgramObject(currentProgram);
					return result;
				}
				log << local.Report();
				return NULL;
			}
	
			String				Template::AssembleSource(bool autoUpdateRenderVariables)
			{
				if (autoUpdateRenderVariables && usesLighting)
					currentConfig->UpdateRenderVariables();

				const RenderConfig&renderConfig = *currentConfig->GetRenderConfig();
				const UserConfig&userConfig = *currentConfig->GetUserConfig();

				static StringBuffer	outBuffer;
				outBuffer.reset();
				outBuffer << "[shared]";
				sharedTemplate.Assemble(renderConfig, userConfig,outBuffer,true);
				outBuffer << "[vertex]";
				vertexTemplate.Assemble(renderConfig, userConfig,outBuffer);
				outBuffer << nl<<"[geometry]";
				geometryTemplate.Assemble(renderConfig, userConfig,outBuffer);
				outBuffer << nl<<"[fragment]";
				fragmentTemplate.Assemble(renderConfig, userConfig,outBuffer);
				return outBuffer.ToStringRef();
			}

		
	}
	
	
	
	
	/*static*/	Extension::ResolutionTable		Extension::depthBufferTable;
	/*static*/ Ctr::Buffer<Extension::DepthBuffer,0>	Extension::depthBufferList;
	/*static*/ Ctr::IndexTable<index_t>				Extension::depthBufferMap;

	/*static*/ GLuint		Extension::AllocateDepthBuffer(const Resolution&res)
	{
		index_t	index;
		if (depthBufferTable.Query(res,index))
		{
			DepthBuffer&buffer = depthBufferList[index];
			buffer.referenceCount ++;
			return buffer.handle;
		}
		{
			index = depthBufferList.GetLength();
			DepthBuffer&buffer = depthBufferList.Append();
			buffer.resolution = res;
			buffer.referenceCount = 1;
			glGenRenderbuffers( 1, &buffer.handle );
			glBindRenderbuffer( GL_RENDERBUFFER, buffer.handle );
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, res.width, res.height );
			depthBufferTable.Set(res,index);
			depthBufferMap.Set(buffer.handle,index);
			return buffer.handle;
		}
	}

	/*static*/ void 				Extension::ReleaseSharedDepthBuffer(GLuint handle)
	{
		if (!handle || application_shutting_down)
			return;
		index_t index;
		if (!depthBufferMap.Query(handle,index))
		{
			FATAL__("Depth buffer "+String(handle)+" is unknown");
			return;
		}

		DepthBuffer&buffer = depthBufferList[index];

		buffer.referenceCount --;
		if (!buffer.referenceCount)
		{
			depthBufferMap.Unset(handle);
			depthBufferMap.visitAllValues([index](index_t&value)
			{
				if (value > index)
					value--;
			});

			depthBufferTable.Unset(buffer.resolution);
			depthBufferTable.visitAllValues([index](index_t&value)
			{
				if (value > index)
					value --;
			});

			glThrowError();
			glDeleteRenderbuffers(1, &handle);
			glThrowError();
			depthBufferList.Erase(index);
		}
	}

	
	
	
	
	
	
	
	
	
	
	#endif


	/*static*/ bool			Extension::TestCurrentFrameBuffer()
	{
		GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
			return true;
		const char* error = "Undescribed and/or unsupported error";
		switch (status)
		{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				error = "Not all framebuffer attachment points are framebuffer attachment complete.";
			break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				error = "Not all attached images have the same width and height.";
			break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				error = "Draw buffer is incomplete.";
			break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				error = "Read buffer is incomplete.";
			break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				error = "No images are attached to the framebuffer.";
			break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				error = "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
			break;
                    
			default:
			break;
		}
		FATAL__("Frame buffer construction failed: OpenGL reports: \""+String(error)+"\"");
		return false;
					
	}

	

	/*static*/	TFrameBuffer	Extension::CreateFrameBuffer(const Resolution&res, DepthStorage depthStorage, BYTE numColorTargets, const GLenum*format)
	{
		glGetError();
		TFrameBuffer result;
		result.depthTarget.storageType = depthStorage;
		result.numColorTargets = numColorTargets;
		ASSERT_LESS__(numColorTargets,ARRAYSIZE(result.colorTarget));
		if (depthStorage != DepthStorage::Texture)
			ASSERT_GREATER__(numColorTargets,0);
		for (BYTE k = 0; k < numColorTargets; k++)
			result.colorTarget[k].textureHandle = 0;
		result.resolution = res;
		#ifdef GL_ARB_framebuffer_object
			GLenum status;
			
			if (glGenFramebuffers)
			{
					glGenFramebuffers( 1, &result.frameBuffer );

					// Initialize the render-buffer for usage as a depth buffer.
					// We don't really need this to render things into the frame-buffer object,
					// but without it the geometry will not be sorted properly.

					switch (depthStorage)
					{
						case DepthStorage::SharedBuffer:
							result.depthTarget.handle = AllocateDepthBuffer(res);
						break;
						case DepthStorage::PrivateBuffer:
							glGenRenderbuffers( 1, &result.depthTarget.handle );
							glBindRenderbuffer( GL_RENDERBUFFER, result.depthTarget.handle );
							glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, res.width, res.height );
						break;
						case DepthStorage::Texture:
						{
							glGenTextures( 1, &result.depthTarget.handle );

							glBindTexture( GL_TEXTURE_2D, result.depthTarget.handle );
							glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
							glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );

							glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
											  res.width, res.height, 
											  0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );

							glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
							glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
							glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
							glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
							glBindTexture( GL_TEXTURE_2D, 0);
						}
						break;
						case DepthStorage::NoDepthStencil:
						break;
					}
					
					ASSERT__(depthStorage == DepthStorage::NoDepthStencil || result.depthTarget.handle != 0);

					glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);
					GLenum	draw_buffers[GL_MAX_COLOR_ATTACHMENTS];
					for (BYTE k = 0; k < numColorTargets; k++)
					{
						draw_buffers[k] = GL_COLOR_ATTACHMENT0+k;
					}
					if (!numColorTargets)
						glDrawBuffers(0,draw_buffers);
					else
						glDrawBuffers(numColorTargets,draw_buffers);
					//glReadBuffer(GL_NONE);
					
					if (depthStorage != DepthStorage::NoDepthStencil)
						if (depthStorage != DepthStorage::Texture)
							glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.depthTarget.handle );
						else
							glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, result.depthTarget.handle, 0 );						


					for (BYTE k = 0; k < numColorTargets; k++)
					{
						glGenTextures( 1, &result.colorTarget[k].textureHandle );

						glBindTexture( GL_TEXTURE_2D, result.colorTarget[k].textureHandle );
						result.colorTarget[k].textureFormat = format[k];

						glTexImage2D( GL_TEXTURE_2D, 0, format[k], 
									  res.width, res.height, 
									  0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
					
						glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+k, GL_TEXTURE_2D, result.colorTarget[k].textureHandle, 0 );
						glBindTexture( GL_TEXTURE_2D, 0);
	
					}

					if (!TestCurrentFrameBuffer())
					{
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						DestroyFrameBuffer(result);
						result.frameBuffer = 0;
						return result;
					}					
					glBindFramebuffer(GL_FRAMEBUFFER, 0);

			}
		#endif
		return result;
	}
	
	/*static*/	bool			Extension::CopyFrameBuffer(const TFrameBuffer&from, const TFrameBuffer&to, const Resolution&res, bool copyColor /*= true*/, bool copyDepth/*=true*/)
	{
		if (!copyColor && !copyDepth)
			return true;
		GLenum mask = 0;
		if (copyColor)
			mask |= GL_COLOR_BUFFER_BIT;
		if (copyDepth)
			mask |= GL_DEPTH_BUFFER_BIT;
		if (!from.frameBuffer || !to.frameBuffer)
			return false;
		glGetError();
        glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, from.frameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, to.frameBuffer);
        glBlitFramebuffer(0, 0, res.width, res.height,
                          0, 0, res.width, res.height,
                          mask,
                          GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
		return glGetError() == GL_NO_ERROR;
	}

	/*static*/	bool			Extension::CopyFrameBuffer(const TFrameBuffer&from, const TFrameBuffer&to, bool copyColor, bool copyDepth)
	{
		if (from.resolution != to.resolution)
			return false;
		return CopyFrameBuffer(from,to,from.resolution,copyColor,copyDepth);
	}
	
	
	
	bool			Extension::ResizeFrameBuffer(TFrameBuffer&buffer,const Resolution&res)
	{
		if (buffer.resolution == res)
			return true;
		glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0 );
			
		for (BYTE k = 0; k < buffer.numColorTargets; k++)
		{
			if (!buffer.colorTarget[k].textureHandle)
				glGenTextures( 1, &buffer.colorTarget[k].textureHandle );
		
			glBindTexture(GL_TEXTURE_2D,buffer.colorTarget[k].textureHandle);
			glTexImage2D( GL_TEXTURE_2D, 0, buffer.colorTarget[k].textureFormat,
				res.width, res.height, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
			glBindTexture(GL_TEXTURE_2D,0);
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+k, GL_TEXTURE_2D, buffer.colorTarget[k].textureHandle, 0 );
		}

		switch (buffer.depthTarget.storageType)
		{
			case DepthStorage::SharedBuffer:
				ReleaseSharedDepthBuffer(buffer.depthTarget.handle);
				buffer.depthTarget.handle = AllocateDepthBuffer(res);
				glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depthTarget.handle );
			break;
			case DepthStorage::PrivateBuffer:
				if (!buffer.depthTarget.handle)
					glGenRenderbuffers( 1, &buffer.depthTarget.handle );
				
				glBindRenderbuffer( GL_RENDERBUFFER, buffer.depthTarget.handle );
				glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, res.width, res.height );
			
				glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depthTarget.handle );
			break;
			case DepthStorage::Texture:
				glBindTexture(GL_TEXTURE_2D,buffer.depthTarget.handle);
				glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
					res.width, res.height, 
					0, GL_LUMINANCE, GL_FLOAT, 0 );
				glBindTexture(GL_TEXTURE_2D,0);
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthTarget.handle, 0 );
			break;
			case DepthStorage::NoDepthStencil:
			break;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		buffer.resolution = res;
		
		
		//destroyFrameBuffer(buffer);
		//buffer = createFrameBuffer(width,height,buffer.alpha);
		return true;
	}


	/*static*/	bool			Extension::FormatHasAlpha(GLenum format)
	{
		switch (format)
		{
			case GL_ALPHA8:
			case GL_LUMINANCE8_ALPHA8:
			case GL_RGBA8:
			case GL_ALPHA16:
			case GL_LUMINANCE16_ALPHA16:
			case GL_RGBA16:
			case GL_ALPHA32I_EXT:
			case GL_RGBA32I:
			case GL_ALPHA16F_ARB:
			case GL_ALPHA32F_ARB:
			case GL_LUMINANCE_ALPHA16F_ARB:
			case GL_LUMINANCE_ALPHA32F_ARB:
			case GL_RGBA16F_ARB:
			case GL_RGBA32F_ARB:
				return true;
			default:
				return false;
		};
	}

	
	
	bool			Extension::BindFrameBuffer(const TFrameBuffer&buffer)
	{
		if (!buffer.frameBuffer)
			return false;
		#ifdef GL_ARB_framebuffer_object
			if (glBindFramebuffer)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
				glViewport(0,0,buffer.resolution.width,buffer.resolution.height);
				//glBindRenderbufferEXT( GL_RENDERBUFFER, g_depthRenderBuffer );
				//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.texture, 0 );
				//glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depth_buffer );
				
				
				return true;
			}
			else
				return false;
		#endif
		return false;
	}
	
	/*static*/ void			Extension::UnbindFrameBuffer()
	{
		#ifdef GL_ARB_framebuffer_object
			if (glBindFramebuffer)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		#endif
	}
	
	/*static*/ void			Extension::DestroyFrameBuffer(const TFrameBuffer&buffer)
	{
		#ifdef GL_ARB_framebuffer_object
			if (glDeleteFramebuffers)
			{
				if (buffer.frameBuffer)
					glDeleteFramebuffers(1, &buffer.frameBuffer);

				if (buffer.depthTarget.storageType == DepthStorage::PrivateBuffer && buffer.depthTarget.handle)
					glDeleteRenderbuffers(1, &buffer.depthTarget.handle);
			}
		#endif
		for (BYTE k = 0; k < buffer.numColorTargets; k++)
			if (buffer.colorTarget[k].textureHandle)
				glDeleteTextures(1,&buffer.colorTarget[k].textureHandle);
		if (buffer.depthTarget.storageType == DepthStorage::SharedBuffer && buffer.depthTarget.handle)
			ReleaseSharedDepthBuffer(buffer.depthTarget.handle);

		if (buffer.depthTarget.storageType == DepthStorage::Texture && buffer.depthTarget.handle)
			glDeleteTextures(1,&buffer.depthTarget.handle);
	}

	
	
	#ifdef WGL_ARB_pbuffer
#if 0
	TPBuffer Extension::createPBuffer(unsigned width, unsigned height, ePBufferType type, BYTE fsaa)
	{

		TPBuffer	result;
		result.enabled	= false;
		if (!wglChoosePixelFormat||!wglQueryPbuffer)
			return result;
		int bind_type(0),
			alpha_bits(0);

		switch (type)
		{
			#ifdef WGL_DEPTH_TEXTURE_FORMAT_NV
			case PBT_DEPTH:
				bind_type	= WGL_BIND_TO_TEXTURE_DEPTH_NV;
				alpha_bits	= 0;
			break;
			#endif
			case PBT_RGB_CUBE:
			case PBT_RGB:
				bind_type	= WGL_BIND_TO_TEXTURE_RGB_ARB;
				alpha_bits	= 0;
			break;
			case PBT_RGBA_CUBE:
			case PBT_RGBA:
				bind_type	= WGL_BIND_TO_TEXTURE_RGBA_ARB;
				alpha_bits	= 8;
			break;
		}


		int attribs[] = {
				WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
				bind_type, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_FALSE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_STEREO_ARB, GL_FALSE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_RED_BITS_ARB, 8,
				WGL_GREEN_BITS_ARB, 8,
				WGL_BLUE_BITS_ARB, 8,
				WGL_ALPHA_BITS_ARB, alpha_bits,
				WGL_DEPTH_BITS_ARB, 16,
				WGL_STENCIL_BITS_ARB, 8,
				fsaa?WGL_SAMPLES_ARB:0, fsaa, //zero terminate here if no samples set
				WGL_SAMPLE_BUFFERS_ARB,fsaa?GL_TRUE:GL_FALSE,
				0,0
			};
		GLuint PixelFormat;
		unsigned nof;
		result.enabled	= wglChoosePixelFormat(hDC,attribs,NULL,1,(int*)&PixelFormat,&nof) && nof;	//			<<<<<<<<<<<<<<<<<<<<<
		if (!result.enabled && fsaa)
		{
			attribs[23] = 0;
			result.enabled	= wglChoosePixelFormat(hDC,attribs,NULL,1,(int*)&PixelFormat,&nof) && nof;
		}
		if (!result.enabled)
			return result;

		int pattribs[] = {
				WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB,
				WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
				0,0
			};
		switch (type)
		{
			case PBT_RGBA_CUBE:
				pattribs[3] = WGL_TEXTURE_CUBE_MAP_ARB;
			break;
			case PBT_RGB_CUBE:
				pattribs[3] = WGL_TEXTURE_CUBE_MAP_ARB;
			case PBT_RGB:
				pattribs[1] = WGL_TEXTURE_RGB_ARB;
			break;
			#ifdef WGL_DEPTH_TEXTURE_FORMAT_NV
			case PBT_DEPTH:
				pattribs[1] = WGL_DEPTH_TEXTURE_FORMAT_NV;
			break;
			#endif
			default:
			break;
		}
		result.buffer	= wglCreatePbuffer(hDC, PixelFormat, width, height, pattribs);
		result.device_context	= wglGetPbufferDC(result.buffer);
		result.buffer_context	= wglCreateContext(result.device_context);
		wglQueryPbuffer(result.buffer,WGL_PBUFFER_WIDTH_ARB,&result.width);
		wglQueryPbuffer(result.buffer,WGL_PBUFFER_HEIGHT_ARB,&result.height);
		
		wglShareLists(wglGetCurrentContext(),result.buffer_context);

		glGenTextures(1, &result.texture);
		glBindTexture(GL_TEXTURE_2D, result.texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		return result;
	}

	void	Extension::destroyPBuffer(const TPBuffer&buffer)
	{
		if (!buffer.enabled)
			return;
		wglDeleteContext(buffer.buffer_context);
		wglReleasePbufferDC(buffer.buffer,buffer.device_context);
		wglDestroyPbuffer(buffer.buffer);
		glDeleteTextures(1,&buffer.texture);
	}
#endif /*0*/

	#elif defined(GLX_VERSION_1_3)
#if 0

	TPBuffer	Extension::createPBuffer(unsigned width, unsigned height, ePBufferType type, BYTE fsaa)
	{


		TPBuffer	result;
		result.enabled	= false;
		int bind_type,
			alpha_bits;

		switch (type)
		{
			#ifdef WGL_DEPTH_TEXTURE_FORMAT_NV
			case PBT_DEPTH:
				//bind_type	= WGL_BIND_TO_TEXTURE_DEPTH_NV;
				alpha_bits	= 0;
			break;
			#endif
			case PBT_RGB_CUBE:
			case PBT_RGB:
				//bind_type	= WGL_BIND_TO_TEXTURE_RGB_ARB;
				alpha_bits	= 0;
			break;
			case PBT_RGBA_CUBE:
			case PBT_RGBA:
				//bind_type	= WGL_BIND_TO_TEXTURE_RGBA_ARB;
				alpha_bits	= 8;
			break;
		}


		int attribs[] = {
			GLX_DOUBLEBUFFER,	False,
			GLX_STEREO,			False,
			GLX_RED_SIZE,		8,
			GLX_GREEN_SIZE,		8,
			GLX_BLUE_SIZE,		8,
			GLX_ALPHA_SIZE,		alpha_bits,
			GLX_DEPTH_SIZE,		16,
			GLX_STENCIL_SIZE,	8,
			GLX_RENDER_TYPE,	GLX_RGBA_BIT,
			0,					0};


		int pattribs[] = {
				GLX_PBUFFER_WIDTH, width,
				GLX_PBUFFER_HEIGHT, height,
				0,0
			};
		int nof;
		GLXFBConfig*config	= glXChooseFBConfig(display, screen,attribs,&nof);
		result.enabled	= config && nof;
		if (!result.enabled)
			return result;
		result.buffer	= glXCreatePbuffer(display, *config, pattribs);
		result.buffer_context	= glXCreateNewContext(display, *config, GLX_RGBA, glXGetCurrentContext(), True);
		result.display	= display;

		glGenTextures(1, &result.texture);
		glBindTexture(GL_TEXTURE_2D, result.texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		

		
		return result;
	}

	void		Extension::destroyPBuffer(const TPBuffer&buffer)
	{
		if (!buffer.enabled)
			return;
		glXDestroyContext(display,buffer.buffer_context);
		glXDestroyPbuffer(display,buffer.buffer);

	}
#endif /*0*/
	#endif



	static bool _ResolveArray(GLenum main_, GLenum size_, GLenum stride_, GLenum type_, GLenum pntr_, String&result)
	{
		if (!glIsEnabled(main_))
		{
			result = "disabled";
			return false;
		}
		GLint	components,
				stride,
				type;
		GLvoid* pntr;

		if (size_)
			glGetIntegerv(size_,&components);
		else
			components	= 3;
		glGetIntegerv(stride_,&stride);
		glGetIntegerv(type_,&type);
		glGetPointerv(pntr_,&pntr);

		result = "(num: "+IntToStr(components)+"; stride: "+IntToStr(stride)+"; type: "+getTypeName(type)+"; pntr: 0x"+PointerToHex(pntr)+")";
		return true;
	}
	static String _ResolveArray(GLenum main_, GLenum size_, GLenum stride_, GLenum type_, GLenum pntr_)
	{
		String rs;
		_ResolveArray(main_,size_,stride_,type_,pntr_,rs);
		return rs;
	}
	
	static const char*	_FilterToString(GLint filter)
	{
		#undef ECASE
		#define ECASE(_CASE_)	case _CASE_: return #_CASE_;
		switch (filter)
		{
			ECASE(GL_LINEAR)
			ECASE(GL_NEAREST)
			ECASE(GL_NEAREST_MIPMAP_NEAREST)
			ECASE(GL_LINEAR_MIPMAP_NEAREST)
			ECASE(GL_NEAREST_MIPMAP_LINEAR)
			ECASE(GL_LINEAR_MIPMAP_LINEAR)
		}
		return "unknown";
	}
		
	static const char*	depthTextureModeToString(GLint filter)
	{
		#undef ECASE
		#define ECASE(_CASE_)	case _CASE_: return #_CASE_;
		switch (filter)
		{
			ECASE(GL_INTENSITY)
			ECASE(GL_ALPHA)
			ECASE(GL_LUMINANCE)
		}
		return "unknown";
	}		
	static const char*	textureCompareModeToString(GLint filter)
	{
		#undef ECASE
		#define ECASE(_CASE_)	case _CASE_: return #_CASE_;
		switch (filter)
		{
			ECASE(GL_COMPARE_R_TO_TEXTURE)
			ECASE(GL_NONE)
		}
		return "unknown";
	}
	
	static String	formatToString(GLint filter)
	{
		#undef ECASE
		#define ECASE(_CASE_)	case _CASE_: return #_CASE_;
		switch (filter)
		{
			case 1: return "GL_LUMINANCE8(1)";
			case 2: return "GL_LUMINANCE8_ALPHA8(2)";
			case 3: return "GL_RGB8(3)";
			case 4: return "GL_RGBA8(4)";
			ECASE(GL_R3_G3_B2)
			ECASE(GL_ALPHA4)
			ECASE(GL_ALPHA8)
			ECASE(GL_ALPHA12)
			ECASE(GL_ALPHA16)
			ECASE(GL_LUMINANCE4)
			ECASE(GL_LUMINANCE8)
			ECASE(GL_LUMINANCE12)
			ECASE(GL_LUMINANCE16)
			ECASE(GL_LUMINANCE4_ALPHA4)
			ECASE(GL_LUMINANCE6_ALPHA2)
			ECASE(GL_LUMINANCE8_ALPHA8)
			ECASE(GL_LUMINANCE12_ALPHA4)
			ECASE(GL_LUMINANCE12_ALPHA12)
			ECASE(GL_LUMINANCE16_ALPHA16)
			ECASE(GL_INTENSITY)
			ECASE(GL_INTENSITY4)
			ECASE(GL_INTENSITY8)
			ECASE(GL_INTENSITY12)
			ECASE(GL_INTENSITY16)
			ECASE(GL_RGB)
			ECASE(GL_RGB4)
			ECASE(GL_RGB5)
			ECASE(GL_RGB8)
			ECASE(GL_RGB10)
			ECASE(GL_RGB12)
			ECASE(GL_RGB16)
			ECASE(GL_RGBA)
			ECASE(GL_RGBA2)
			ECASE(GL_RGBA4)
			ECASE(GL_RGB5_A1)
			ECASE(GL_RGBA8)
			ECASE(GL_RGB10_A2)
			ECASE(GL_RGBA12)
			ECASE(GL_RGBA16)

			ECASE(GL_DEPTH_COMPONENT)
			ECASE(GL_DEPTH_COMPONENT16)
			ECASE(GL_DEPTH_COMPONENT24)
			ECASE(GL_DEPTH_COMPONENT32)

			ECASE(GL_RGBA16F)
			ECASE(GL_RGB16F)
			ECASE(GL_RGBA32F)
			ECASE(GL_RGB32F)
			ECASE(GL_R16F)
			ECASE(GL_R32F)
			ECASE(GL_RG16F)
			ECASE(GL_RG32F)
			ECASE(GL_ALPHA16F_ARB)
			ECASE(GL_INTENSITY16F_ARB)
			ECASE(GL_LUMINANCE16F_ARB)
			ECASE(GL_LUMINANCE_ALPHA16F_ARB)

			
		}
		return "unknown ("+String(filter)+")";
	}

	String	Extension::QueryRenderState()
	{
		#undef BSTR
		#define BSTR(b)		String(b?"enabled":"disabled")
		GLfloat	projection[16], modelview[16], raster_distance;
		M::TVec4<GLfloat>	raster, raster_color, raster_texcoords,color,coords;
		M::TVec3<GLfloat>	normal;

		GLuint		program=0;
		GLint		array_buffer_binding = 0,
					element_array_buffer_binding = 0,
					pixel_pack_buffer_binding = 0,
					pixel_unpack_buffer_binding = 0,
					active_texture = -1,
					client_active_texture = -1,
					fbo = 0,
					viewport[4] = {0,0,0,0};
		GLboolean	raster_valid;
			glGetFloatv(GL_PROJECTION_MATRIX,projection);
			glGetFloatv(GL_MODELVIEW_MATRIX,modelview);
			glGetFloatv(GL_CURRENT_RASTER_POSITION,raster.v);
			glGetFloatv(GL_CURRENT_RASTER_COLOR,raster_color.v);
			glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&raster_valid);
			glGetFloatv(GL_CURRENT_RASTER_DISTANCE,&raster_distance);
			glGetFloatv(GL_CURRENT_RASTER_TEXTURE_COORDS,raster_texcoords.v);
			glGetFloatv(GL_CURRENT_COLOR,color.v);
			glGetFloatv(GL_CURRENT_NORMAL,normal.v);
			glGetIntegerv(GL_VIEWPORT,viewport);
		#ifdef GL_ACTIVE_TEXTURE_ARB
			glGetIntegerv(GL_ACTIVE_TEXTURE_ARB,&active_texture);
		#endif
		
		#ifdef GL_CLIENT_ACTIVE_TEXTURE_ARB
			glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE_ARB,&client_active_texture);
		#endif
			
			
		#ifdef GL_ARB_shader_objects
			if (glGetHandle)
				program = (GLuint)glGetHandle(GL_PROGRAM_OBJECT_ARB);
		#endif
		#ifdef GL_FRAMEBUFFER_BINDING
			glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
		#endif
		#ifdef GL_ARB_vertex_buffer_object
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&array_buffer_binding);
			glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&element_array_buffer_binding);
			glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&pixel_pack_buffer_binding);
			glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&pixel_unpack_buffer_binding);
		#endif

		

		String rs	= "OpenGL state:\n\
  Projection:\n"+M::Mat::Raw::ToString4x4(projection)+"\n\
  Modelview:\n"+M::Mat::Raw::ToString4x4(modelview)+"\n\
  Program: "+String(program)+"\n\
  FBO: "+String(fbo)+"\n\
  Viewport: "+(M::Vec::toString(M::Vec::ref4(viewport)))+"\n\
  Active texture layer: "+String(active_texture!=-1?active_texture-GL_TEXTURE0:-1)+"\n\
  Client active texture layer: "+String(client_active_texture!=-1?client_active_texture-GL_TEXTURE0:-1)+"\n\
  Color: "+M::Vec::toString(color)+"\n\
  Normal: "+M::Vec::toString(normal)+"\n\
  RasterPos: "+M::Vec::toString(raster)+"\n\
  RasterColor: "+M::Vec::toString(raster_color)+"\n\
  RasterDistance: "+FloatToStr(raster_distance)+"\n\
  RasterCoords: "+M::Vec::toString(raster_texcoords)+"\n\
  RasterValid: "+(raster_valid?"true":"false")+"\n\
  AlphaTest: ";
  
  if (glIsEnabled(GL_ALPHA_TEST))
  {
	rs += "enabled (";
	GLfloat   alpha_ref;
    GLint     alpha_func;

    glGetFloatv(GL_ALPHA_TEST_REF,&alpha_ref);
    glGetIntegerv(GL_ALPHA_TEST_FUNC,&alpha_func);
	switch (alpha_func)
	{
		case GL_LESS:
			rs += "<";
		break;
		case GL_LEQUAL:
			rs += "<=";
		break;
		case GL_EQUAL:
			rs += "=";
		break;
		case GL_GREATER:
			rs += ">";
		break;
		case GL_GEQUAL:
			rs += ">=";
		break;
		case GL_NOTEQUAL:
			rs += "!=";
		break;
	}
	rs += " "+String(alpha_ref)+")";
  
  }
  else
	rs += "disabled";
  rs += "\n\
  DepthTest: ";
  if (glIsEnabled(GL_DEPTH_TEST))
  {
	  GLint val;
	  glGetIntegerv(GL_DEPTH_FUNC,&val);
	  switch (val)
	  {
		case GL_LESS:
			rs += "GL_LESS";
		break;
		case GL_LEQUAL:
			rs += "GL_LEQUAL";
		break;
		case GL_EQUAL:
			rs += "! GL_EQUAL !";
		break;
		case GL_GREATER:
			rs += "GL_GREATER";
		break;
		case GL_GEQUAL:
			rs += "GL_GEQUAL";
		break;
		case GL_NOTEQUAL:
			rs += "! GL_NOTEQUAL !";
		break;
	  }
  }
  else
	  rs += "Disabled\n";

  rs += "\n\
  FaceCull: "+BSTR(glIsEnabled(GL_CULL_FACE))+"\n\
  Lighting: "+BSTR(glIsEnabled(GL_LIGHTING))+"\n\
  Blend: "+BSTR(glIsEnabled(GL_BLEND))+"\n\
  Array Buffer Binding: "+String(array_buffer_binding)+"\n\
  Element Array Buffer Binding: "+String(element_array_buffer_binding)+"\n\
  Pixel Pack Buffer Binding: "+String(pixel_pack_buffer_binding)+"\n\
  Pixel Unpack Buffer Binding: "+String(pixel_unpack_buffer_binding)+"\n\
  VertexArray: "+_ResolveArray(GL_VERTEX_ARRAY,GL_VERTEX_ARRAY_SIZE,GL_VERTEX_ARRAY_STRIDE,GL_VERTEX_ARRAY_TYPE,GL_VERTEX_ARRAY_POINTER)+"\n\
  ColorArray: "+_ResolveArray(GL_COLOR_ARRAY,GL_COLOR_ARRAY_SIZE,GL_COLOR_ARRAY_STRIDE,GL_COLOR_ARRAY_TYPE,GL_COLOR_ARRAY_POINTER)+"\n\
  NormalArray: "+_ResolveArray(GL_NORMAL_ARRAY,0,GL_NORMAL_ARRAY_STRIDE,GL_NORMAL_ARRAY_TYPE,GL_NORMAL_ARRAY_POINTER)+"\n";
		if (glActiveTexture)
		{
			for (unsigned i	= 0; i < (unsigned)maxTextureLayers; i++)
			{
				glActiveTexture(GL_TEXTURE0+i);
				
				String texture,array,texture_info;
				GLint	value, handle;
				GLenum target,face_target;
				String dimension;
				glGetIntegerv(GL_TEXTURE_BINDING_1D,&handle);
				face_target = target = GL_TEXTURE_1D;
				if (handle == 0)
				{
					glGetIntegerv(GL_TEXTURE_BINDING_2D,&handle);
					face_target = target = GL_TEXTURE_2D;
					if (handle == 0)
					{
						glGetIntegerv(GL_TEXTURE_BINDING_3D,&handle);
						face_target = target = GL_TEXTURE_3D;
						if (handle == 0)
						{
							glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP,&handle);
							face_target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
							target = GL_TEXTURE_BINDING_CUBE_MAP;
							if (handle == 0)
							{
								texture = "disabled";
								continue;
							}
						}
					}
				}
				switch (target)
				{
					case GL_TEXTURE_1D:
						texture = "1D";
					break;
					case GL_TEXTURE_2D:
						texture = "2D";
					break;
					case GL_TEXTURE_3D:
						texture = "3D";
					break;
					case GL_TEXTURE_CUBE_MAP:
						texture = "CUBE";
					break;
				}
				{
					texture += " ("+String(handle);
					if (glIsEnabled(target))
						texture += ",enabled";
					texture += ")";
				}

				{
					{
						glGetTexParameteriv(target,GL_TEXTURE_MIN_FILTER,&value);
						texture_info += "   Min-Filter: "+String(_FilterToString(value))+"\n";
						glGetTexParameteriv(target,GL_TEXTURE_MAG_FILTER,&value);
						texture_info += "   Mag-Filter: "+String(_FilterToString(value))+"\n";
						glGetError();
						glGetTexLevelParameteriv(face_target,0,GL_TEXTURE_INTERNAL_FORMAT,&value);
						glThrowError();
						texture_info += "   Format: "+formatToString(value)+"\n";
						glGetTexLevelParameteriv(face_target,0,GL_TEXTURE_WIDTH,&value);
						texture_info += "   Width: "+String(value)+"\n";
						glGetTexLevelParameteriv(face_target,0,GL_TEXTURE_HEIGHT,&value);
						texture_info += "   Height: "+String(value)+"\n";
						glGetTexParameteriv(target,GL_TEXTURE_COMPARE_MODE,&value);
						texture_info += "   Compare mode: "+String(textureCompareModeToString(value))+"\n";
						glGetTexParameteriv(target,GL_DEPTH_TEXTURE_MODE,&value);
						texture_info += "   Depth texture mode: "+String(depthTextureModeToString(value))+"\n";
					}
					
				}
				
				
				rs += "TexLayer "+IntToStr(i)+"/"+String(maxTextureLayers) +"\n";
				rs += "  Texture: "+texture+"\n"+texture_info;
			}
			glActiveTexture(active_texture);
		}
		if (glClientActiveTexture)
		{
			for (unsigned i	= 0; i < (unsigned)maxTexcoordLayers; i++)
			{
				glClientActiveTexture(GL_TEXTURE0+i);
				glGetFloatv(GL_TEXTURE_MATRIX,modelview);
				glGetFloatv(GL_CURRENT_TEXTURE_COORDS,coords.v);
				
				String texture,array,texture_info;
				bool has_array = _ResolveArray(GL_TEXTURE_COORD_ARRAY,GL_TEXTURE_COORD_ARRAY_SIZE,GL_TEXTURE_COORD_ARRAY_STRIDE,GL_TEXTURE_COORD_ARRAY_TYPE,GL_TEXTURE_COORD_ARRAY_POINTER,array);
				GLint	value, handle;
				GLenum target,face_target;
				
				
				rs += "CoordLayer "+IntToStr(i)+"/"+String(maxTexcoordLayers) +"\n";
				rs += "  CoordArray: "+array+"\n";
				rs += "  Coords: "+M::Vec::toString(coords)+"\n";
				rs += "  Matrix "+M::Vec::toString(M::Vec::ref4(modelview))+"; "+M::Vec::toString(M::Vec::ref4(modelview+4))+"; "+M::Vec::toString(M::Vec::ref4(modelview+8))+"; "+M::Vec::toString(M::Vec::ref4(modelview+12))+"\n";
			}
			glClientActiveTexture(client_active_texture);
		}
		// glMatrixMode(GL_PROJECTION);
			// glLoadMatrixf(projection);
		// glMatrixMode(GL_MODELVIEW);
			// glLoadMatrixf(modelview);

		return rs;
		#undef BSTR
	}

	bool Extension::Init(GLuint index)
	{
		#define Set(group_name)	EXT_CONTEXT(group_name)
		#define get(var)			EXT_GET_EXTENSION(var)
		#define silent(var)		EXT_GET_EXTENSION_NO_CHECK(var)

		bool one_failed	= false;

		#ifdef GL_VERSION_2_0
			if (!glDrawBuffers)
			{
				group = "";
				get(glDrawBuffers);
			}
		#endif

		if (index&EXT_MULTITEXTURE_BIT)
		{
			#ifdef GL_ARB_multitexture
				if (IsAvailable("GL_ARB_multitexture"))
				{
					Set(ARB);
					get(glMultiTexCoord1f);
					get(glMultiTexCoord1fv);
					get(glMultiTexCoord2f);
					get(glMultiTexCoord2fv);
					get(glMultiTexCoord3f);
					get(glMultiTexCoord3fv);
					get(glMultiTexCoord4f);
					get(glMultiTexCoord4fv);
					get(glMultiTexCoord1d);
					get(glMultiTexCoord1dv);
					get(glMultiTexCoord2d);
					get(glMultiTexCoord2dv);
					get(glMultiTexCoord3d);
					get(glMultiTexCoord3dv);
					get(glMultiTexCoord4d);
					get(glMultiTexCoord4dv);
					get(glMultiTexCoord1i);
					get(glMultiTexCoord1iv);
					get(glMultiTexCoord2i);
					get(glMultiTexCoord2iv);
					get(glMultiTexCoord3i);
					get(glMultiTexCoord3iv);
					get(glMultiTexCoord4i);
					get(glMultiTexCoord4iv);
					get(glActiveTexture);
					get(glClientActiveTexture);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_REGISTER_COMBINERS_BIT)
		{
			#ifdef GL_NV_register_combiners
				if (IsAvailable("GL_NV_register_combiners"))
				{
					Set(NV);
					get(glCombinerParameterfv);
					get(glCombinerParameterf);
					get(glCombinerParameteriv);
					get(glCombinerParameteri);
					get(glCombinerInput);
					get(glCombinerOutput);
					get(glFinalCombinerInput);
					get(glGetCombinerInputParameterfv);
					get(glGetCombinerInputParameteriv);
					get(glGetCombinerOutputParameterfv);
					get(glGetCombinerOutputParameteriv);
					get(glGetFinalCombinerInputParameterfv);
					get(glGetFinalCombinerInputParameteriv);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_VERTEX_PROGRAM_BIT)
		{
			#ifdef GL_ARB_vertex_program
				if (IsAvailable("GL_ARB_vertex_program"))
				{
					Set(ARB);
					get(glVertexAttrib2f);
					get(glVertexAttrib2d);
					get(glVertexAttrib3f);
					get(glVertexAttrib3d);
					get(glVertexAttrib4f);
					get(glVertexAttrib4d);
					get(glVertexAttrib2fv);
					get(glVertexAttrib2dv);
					get(glVertexAttrib3fv);
					get(glVertexAttrib3dv);
					get(glVertexAttrib4iv);
					get(glVertexAttrib4uiv);
					get(glVertexAttrib4fv);
					get(glVertexAttrib4dv);
					get(glVertexAttrib4Niv);
					get(glVertexAttrib4Nuiv);
					get(glVertexAttribPointer);
					get(glEnableVertexAttribArray);
					get(glDisableVertexAttribArray);
					get(glProgramString);
					get(glBindProgram);
					get(glDeletePrograms);
					get(glGenPrograms);
					get(glProgramEnvParameter4d);
					get(glProgramEnvParameter4dv);
					get(glProgramEnvParameter4f);
					get(glProgramEnvParameter4fv);
					get(glProgramLocalParameter4d);
					get(glProgramLocalParameter4dv);
					get(glProgramLocalParameter4f);
					get(glProgramLocalParameter4fv);
					get(glGetProgramEnvParameterdv);
					get(glGetProgramEnvParameterfv);
					get(glGetProgramLocalParameterdv);
					get(glGetProgramLocalParameterfv);
					get(glGetProgramiv);
					get(glGetProgramString);
					get(glGetVertexAttribdv);
					get(glGetVertexAttribfv);
					get(glGetVertexAttribiv);
					get(glGetVertexAttribPointerv);
					get(glIsProgram);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_VERTEX_BUFFER_OBJECT_BIT)
		{
			#ifdef GL_ARB_vertex_buffer_object
				if (IsAvailable("GL_ARB_vertex_buffer_object"))
				{
					Set(ARB);
					get(glBindBuffer);
					get(glDeleteBuffers);
					get(glGenBuffers);
					get(glIsBuffer);
					get(glBufferData);
					get(glBufferSubData);
					get(glGetBufferSubData);
					get(glMapBuffer);
					get(glUnmapBuffer);
					get(glGetBufferParameteriv);
					get(glGetBufferPointerv);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_COMPILED_ARRAYS_BIT)
		{
			#ifdef GL_EXT_compiled_vertex_array
				if (IsAvailable("GL_EXT_compiled_vertex_array"))
				{
					Set(EXT);
					get(glLockArrays);
					get(glUnlockArrays);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_SHADER_OBJECTS_BIT)
		{
			#ifdef GL_ARB_shader_objects
				if (IsAvailable("GL_ARB_shader_objects"))
				{
					Set(ARB);
					get(glDeleteObject);
					get(glGetHandle);
					get(glDetachObject);
					get(glCreateShaderObject);
					get(glShaderSource);
					get(glCompileShader);
					get(glCreateProgramObject);
					get(glAttachObject);
					get(glLinkProgram);
					get(glUseProgramObject);
					get(glValidateProgram);
					get(glUniform1f);
					get(glUniform2f);
					get(glUniform3f);
					get(glUniform4f);
					get(glUniform1i);
					get(glUniform2i);
					get(glUniform3i);
					get(glUniform4i);
					get(glUniform1fv);
					get(glUniform2fv);
					get(glUniform3fv);
					get(glUniform4fv);
					get(glUniform1iv);
					get(glUniform2iv);
					get(glUniform3iv);
					get(glUniform4iv);
					get(glUniformMatrix2fv);
					get(glUniformMatrix3fv);
					get(glUniformMatrix4fv);
					get(glGetObjectParameterfv);
					get(glGetObjectParameteriv);
					get(glGetInfoLog);
					get(glGetAttachedObjects);
					get(glGetUniformLocation);
					get(glGetActiveUniform);
					get(glGetUniformfv);
					get(glGetUniformiv);
					get(glGetShaderSource);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_WIN_CONTROL_BIT)
		{
			#ifdef WGL_EXT_swap_control
				Set(EXT);
				silent(wglSwapInterval);
				silent(wglGetSwapInterval);
			#endif
			#ifdef WGL_ARB_pixel_format
				Set(ARB);
				silent(wglChoosePixelFormat);
				silent(wglGetPixelFormatAttribiv);
				silent(wglGetPixelFormatAttribfv);
			#endif
			#if defined(GLX_SGIX_fbconfig) && !defined(GLX_VERSION_1_3)
				Set(SGIX);
				silent(glXGetFBConfigAttrib);
				silent(glXChooseFBConfig);
				silent(glXCreateGLXPixmapWithConfig);
				silent(glXCreateContextWithConfig);
				silent(glXGetVisualFromFBConfig);
				silent(glXGetFBConfigFromVisual);
			#endif
		}
		if (index&EXT_PBUFFER_BIT)
		{
			#if SYSTEM	==WINDOWS
				#ifdef WGL_ARB_pbuffer
					Set(ARB);
					if (IsAvailable("WGL_ARB_pbuffer"))
					{
						get(wglCreatePbuffer);
						get(wglGetPbufferDC);
						get(wglReleasePbufferDC);
						get(wglDestroyPbuffer);
						get(wglQueryPbuffer);
					}
					else
						one_failed	= true;
				#else
					one_failed	= true;
				#endif
			#endif
		}
		if (index&EXT_RENDER_TEXTURE_BIT)
		{
			#ifdef WGL_ARB_render_texture
				if (IsAvailable("WGL_ARB_render_texture"))
				{
					Set(ARB);
					get(wglBindTexImage);
					get(wglReleaseTexImage);
					get(wglSetPbufferAttrib);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}

		if (index&EXT_OCCLUSION_QUERY_BIT)
		{
			#ifdef GL_ARB_occlusion_query
				if (IsAvailable("GL_ARB_occlusion_query"))
				{
					Set(ARB);
					get(glGenQueries);
					get(glDeleteQueries);
					get(glIsQuery);
					get(glBeginQuery);
					get(glEndQuery);
					get(glGetQueryiv);
					get(glGetQueryObjectiv);
					get(glGetQueryObjectuiv);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}

		if (index&EXT_MAKE_CURRENT_READ_BIT)
		{
			#ifdef WGL_ARB_make_current_read
				if (IsAvailable("WGL_ARB_make_current_read"))
				{
					Set(ARB);
					get(wglMakeContextCurrent);
					get(wglGetCurrentReadDC);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}

		if (index&EXT_WINDOW_POS_BIT)
		{
			#ifdef GL_ARB_window_pos
				if (IsAvailable("GL_ARB_window_pos"))
				{
					Set(ARB);
					get(glWindowPos2d);
					get(glWindowPos2dv);
					get(glWindowPos2f);
					get(glWindowPos2fv);
					get(glWindowPos2i);
					get(glWindowPos2iv);
					get(glWindowPos2s);
					get(glWindowPos2sv);
					get(glWindowPos3d);
					get(glWindowPos3dv);
					get(glWindowPos3f);
					get(glWindowPos3fv);
					get(glWindowPos3i);
					get(glWindowPos3iv);
					get(glWindowPos3s);
					get(glWindowPos3sv);
				}
				else
					one_failed	= true;
			#else
				one_failed	= true;
			#endif
		}
		if (index&EXT_FRAME_BUFFER_OBJECT_BIT)
		{
			#ifdef GL_ARB_framebuffer_object
				if (IsAvailable("GL_ARB_framebuffer_object"))
				{
					//Set(ARB);
					group = "";
					get(glIsRenderbuffer);
					get(glBindRenderbuffer);
					get(glDeleteRenderbuffers);
					get(glGenRenderbuffers);
					get(glRenderbufferStorage);
					get(glGetRenderbufferParameteriv);
					get(glIsFramebuffer);
					get(glBindFramebuffer);
					get(glDeleteFramebuffers);
					get(glGenFramebuffers);
					get(glCheckFramebufferStatus);
					get(glFramebufferTexture1D);
					get(glFramebufferTexture2D);
					get(glFramebufferTexture3D);
					get(glFramebufferRenderbuffer);
					get(glGetFramebufferAttachmentParameteriv);
					get(glGenerateMipmap);
					get(glBlitFramebuffer);
					get(glRenderbufferStorageMultisample);
					get(glFramebufferTextureLayer);
				}
				else
					one_failed = true;
			#else
				one_failed = true;
			#endif
		}
		
		if (index&EXT_BLEND_FUNC_SEPARATE_BIT)
		{
			#ifdef GL_EXT_blend_func_separate
				if (IsAvailable("GL_EXT_blend_func_separate"))
				{
					Set(EXT);
					get(glBlendFuncSeparate);
				}
				else
					one_failed = true;
			#else
				one_failed = true;
			#endif
		}
		
	
	




		return !one_failed;
		#undef silent
		#undef get
		#undef set
	}

	bool Extension::IsAvailable(const String&extension_name)
	{
		return glExt.lookup(extension_name) || sysglExt.lookup(extension_name);
	}


	String ExtCont::listColumns(const String&plank, BYTE columns)
	{
		String rs;
		size_t height	= field.GetLength() / columns;
		if (field.GetLength() % columns)
			height++;
		BYTE	width[256];
		for (BYTE k	= 0; k < columns; k++)
		{
			width[k] = 0;
			for (index_t i	= 0; i < height; i++)
				if (height*k+i < field.GetLength() && field[height*k+i].GetLength() > width[k])
					width[k] = BYTE(field[height*k+i].GetLength());
			width[k]+= 2;
		}
		for (index_t i	= 0; i < height; i++)
		{
			rs += plank;
			for (BYTE k	= 0; k < columns; k++)
				if (height*k+i < field.GetLength())
				{
					rs+= field[height*k+i];
					for (BYTE j	= 0; j < width[k] - field[height*k+i].GetLength(); j++)
						rs+=' ';
				}
			rs+='\n';
		}
		return rs;
	}


	String ExtCont::listExtensions(const String&plank, count_t max_chars_per_line)
	{
		count_t c	= 0;
		String rs(plank);
		for (index_t i	= 0; i < field.GetLength(); i++)
		{
			if (max_chars_per_line && c + field[i].GetLength()+2 > max_chars_per_line)
			{
				rs+="\n"+plank;
				c	= 0;
			}
			rs+=field[i];
			if (i < field.GetLength()-1)
				rs+= ", ";
			c+= field[i].GetLength()+2;
		}
		return rs;
	}

	void Extension::Initialize(
	#if SYSTEM	==WINDOWS
		HDC hDC
	#elif SYSTEM	==UNIX
		Display*display,int screen
	#endif
		)
	{
		String sysgl_str;
		#if SYSTEM	==WINDOWS && defined(WGL_ARB_extensions_string)
			this->hDC	= hDC;
			EXT_CONTEXT(ARB);
			EXT_GET_EXTENSION_NO_CHECK(wglGetExtensionsString);
			if (wglGetExtensionsString)
				sysgl_str	= wglGetExtensionsString(hDC);
		#elif SYSTEM	==UNIX
			this->display	= display;
			this->screen	= screen;
			#ifdef __glx_h__
				sysgl_str	=	glXQueryExtensionsString(display,screen);
			#endif
		#endif

		sysglExt.divide(sysgl_str);
		String line	= (char*)glGetString(GL_EXTENSIONS);
		glExt.divide(line);


		maxLights = 8;
		glGetIntegerv(GL_MAX_LIGHTS,&maxLights);
		maxTextureMaxAnisotropy = 1.0f;
		#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&maxTextureMaxAnisotropy);
		#endif
		maxTextureLayers	= 0;
		maxTexcoordLayers	= 0;
		#ifdef GL_MAX_TEXTURE_UNITS_ARB
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB,&maxTextureLayers);
			glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB,&maxTexcoordLayers);
		#endif
		maxRegisterCombiners	= 0;
		#ifdef GL_MAX_GENERAL_COMBINERS_NV
			glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV,&maxRegisterCombiners);
		#endif
		maxCubeTextureSize	= 0;
		#ifdef GL_MAX_CUBE_MAP_TEXTURE_SIZE
			glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE,&maxCubeTextureSize);
		#endif

#if 0
		initial_total = 1;
		if (available("GL_NVX_gpu_memory_info"))
		{
			cout<<"NVIDIA VRAM info available"<<endl;
			query_method = VRAMQueryMethod::NVIDIA;
		}
		elif (available("GL_ATI_meminfo"))
		{
			cout<<"ATI VRAM info available"<<endl;
			query_method = VRAMQueryMethod::ATI;
			{
				GLint	val[4];
				glGetError();
				glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI,val);
				initial_total = size_t(val[0])*1024;
				glThrowError();
			}
		}
		else
		{
			cout << "No supported VRAM info available!"<<endl;
			query_method = VRAMQueryMethod::None;
		}
#endif /*0*/

		

	}

#if 0
	bool			Extension::deviceMemoryState(size_t&free,size_t&total)	const
	{
		switch (query_method)
		{
			case VRAMQueryMethod::NVIDIA:
			{
				#ifndef GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX 
					#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX 0x9047
					#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
					#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
					#define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX 0x904A
					#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX 0x904B
				#endif
				GLint	val;
				glGetError();
				glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX,&val);
				total = size_t(val)*1024;
				glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX,&val);
				free = size_t(val)*1024;
				return glGetError() == GL_NO_ERROR;
			}
			break;
			case VRAMQueryMethod::ATI:
			{
				GLint	val[4];
				glGetError();
				total = initial_total;
				glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI,val);
				free = size_t(val[0])*1024;
				return glGetError() != GL_NO_ERROR;
			}
			break;
		}
		return false;
	}
#endif /*0*/


	Extension glExtensions;
}
