#include "opengl.h"
#include <string/encoding.h>

namespace Engine
{
	#if SYSTEM==WINDOWS
		#ifndef PFD_SUPPORT_COMPOSITION
			#define PFD_SUPPORT_COMPOSITION	0x00008000
		#endif
	/*static*/		bool GL::Shader::localShaderIsBound = false;


		static bool myPickPixelFormat(HDC context, int*attribs, GLint&pixel_format_out, BYTE&sample_out)
		{
			/*
			it seems some implementations of opengl do not support the WGL_SAMPLES_ARB / WGL_SAMPLE_BUFFERS_ARB - indices
			and fail when assigned.
			strange.
			*/
			GLint	formats[1024];
			UINT	nof;
			if (!wglChoosePixelFormat(context,attribs,NULL,ARRAYSIZE(formats),formats,&nof) || !nof)
			{
				attribs[13] = 24;
				attribs[17] = 24;
				attribs[19] = 8;
				attribs[20] = 0; //zero terminate here
				attribs[21] = 0;
				sample_out = 0;
				if (!wglChoosePixelFormat(context,attribs,NULL,ARRAYSIZE(formats),formats,&nof) || !nof)
					return false;
			}
			
			PIXELFORMATDESCRIPTOR	desc;
			//find one that supports aero
			for (UINT i = 0; i < nof; i++)
			{
				if (!DescribePixelFormat(context,formats[i],sizeof(desc),&desc))
				{
					std::cout << "Warning: Unable to retrieve pixel format descriptor at offset "<<i<<" ("<<formats[i]<<")"<<std::endl;
					continue;
				}
				if (desc.dwFlags & PFD_SUPPORT_COMPOSITION)
				{
					//cout << "Found appropriate pixel format descriptor at offset "<<i<<" ("<<formats[i]<<")"<<endl;
					pixel_format_out = formats[i];
					return true;
				}
			}
			if (nof)	//it's better to return something than nothing at all. might still be that this operating system simply does not support the PFD_SUPPORT_COMPOSITION flag
			{
				pixel_format_out = formats[0];
				return true;
			}
			return false;
		}
	#endif

	LogFile	OpenGL::log_file(StringType::Cast("opengl.log"),true);

	GL::RenderState	OpenGL::state;

	
	
		



	
	bool				OpenGL::hasMoved(const PLight&light)
	{
		return light->moved;
	}

	bool				OpenGL::wasModified(const PLight&light)
	{
		return light->modified;
	}

	void				OpenGL::setHasMoved(const PLight&light,bool b)
	{
		light->moved = b;
	}

	void				OpenGL::setWasModified(const PLight&light,bool b)
	{
		light->modified = b;

	}


	index_t			OpenGL::getIndexOf(const PLight&light)
	{
		return light->index;
	}

	void				OpenGL::setIndexOf(const PLight&light,index_t index)
	{
		light->index = index;
	}

	LightScene*		OpenGL::getLightSceneOf(const PLight&light)
	{
		return light->scene;
	}



	PLight	OpenGL::createLight(bool enable)
	{
		PLight light(new Light(active_scene,this));
		active_scene->Append(light);
		
		if (enable)
			light->enable();
		return light;
	}

	void		OpenGL::discardLight(const PLight&light)
	{
		LightScene*scene = light->scene;
		
	    if (scene->at(light->origin) != light)
	        FATAL__("bad light");
			
	    light->disable();
	    index_t index = light->origin;
	    scene->Erase(light->origin);
	    for (index_t i = index; i < scene->Count(); i++)
	        scene->at(i)->origin = i;
	}

	void		OpenGL::clearLights()
	{
		foreach (*active_scene,light)
			(*light)->disable();
		active_scene->clear();
	}

	void		OpenGL::resetLighting()
	{
		if (application_shutting_down)
			return;
		Array<LightScene*>	exported;
		scenes.ExportTo(exported);
		for (index_t i = 0; i < exported.Count(); i++)
		{
			foreach (*exported[i],light)
				(*light)->disable();
		}
		scenes.clear();
		active_scene_index = 0;
		scene_index_counter = 0;
		active_scene = scenes.define(scene_index_counter++);
		pickLightScene(0);
	}

	index_t  OpenGL::createLightScene()
	{
		index_t index = scene_index_counter++;
		scenes.define(index);
	    pickLightScene(index);
	    return index;
	}

	bool			OpenGL::isLightScene(index_t scenario)	const
	{
		return scenes.IsSet(scenario);
	}

	void            OpenGL::discardLightScene(index_t index)
	{
	    if (!index)
			return;
		LightScene*scene = scenes.drop(index);
		if (!scene)
			return;
		if (scene == active_scene)
			pickLightScene(0);
		foreach (*scene,light)
			(*light)->disable();
		Discard(scene);
	}
	
	index_t		OpenGL::getLightScene()	const
	{
		return active_scene_index;
	}
	
	void			OpenGL::getSceneLights(ArrayData<PLight>&array, bool enabled_only)
	{
		size_t	count = 0;
		if (!enabled_only)
			count = active_scene->Count();
		else
			for (unsigned i = 0; i < active_scene->Count(); i++)
				if (active_scene->at(i)->isEnabled())
					count++;
		array.SetSize(count);
		count = 0;
		for (unsigned i = 0; i < active_scene->Count(); i++)
			if (!enabled_only || active_scene->at(i)->isEnabled())
				array[count++] = active_scene->at(i);
	}


	
	
	namespace GL
	{
		void Decode(GLenum format, BYTE&channels, PixelType&pt);
	
		GLShader::Template::VariableMap		Shader::globalMap;
		GLShader::Template::UserConfig		Shader::globalUserConfig;
		GLShader::Template::Configuration	Shader::globalConfig(GLShader::Template::globalRenderConfig,globalUserConfig,false);
		Texture::Reference					Shader::globalSkyTexture;

		



		V2::Detail::TTextureInfo		V2::Detail::Describe(const Texture&t)	{return std::make_pair(t.GetHandle(),t.GetDimension());}
		V2::Detail::TTextureInfo		V2::Detail::Describe(const Texture*t)	{return t ? std::make_pair(t->GetHandle(),t->GetDimension()) : std::make_pair<GLuint,TextureDimension>(0,TextureDimension::None);}
		V2::Detail::TTextureInfo		V2::Detail::Describe(const Texture::Reference&ref) {return std::make_pair(ref.GetHandle(),ref.GetDimension());}
		V2::Detail::TTextureInfo		V2::Detail::Describe(const FBO&object) {return Describe(object.Refer(0));}
		void							V2::Detail::BindTexture(index_t layer, const TTextureInfo&handle)
		{
			//glActiveTexture((GLuint)(GL_TEXTURE0 + _texturesBound)); _texturesBound++;
			if (!handle.first)
			{
				FATAL__("Tyring to bind empty texture");
				return;
			}
			glActiveTexture((GLuint)(GL_TEXTURE0 + layer));
			glBindTexture(Translate(handle.second),handle.first);
			glActiveTexture(GL_TEXTURE0);
		}
		void			V2::Detail::Configure(const TTextureInfo&info, bool clamp)
		{
			if (!info.first)
				return;
			glPushAttrib(GL_TEXTURE_BIT);
			GLenum value = clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			GLenum type = Translate(info.second);
			glBindTexture(type,info.first);
				glTexParameteri( type, GL_TEXTURE_WRAP_S, value);
				glTexParameteri( type, GL_TEXTURE_WRAP_T, value);
				glTexParameteri( type, GL_TEXTURE_WRAP_R, value);
			glPopAttrib();
		}

		void	V2::UnbindTexture(index_t layer)
		{
			glActiveTexture((GLuint)(GL_TEXTURE0 + layer));
			glBindTexture(GL_TEXTURE_1D,0);
			glBindTexture(GL_TEXTURE_2D,0);
			glBindTexture(GL_TEXTURE_3D,0);
			glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			glActiveTexture(GL_TEXTURE0);
		}

			
		void			V2::TextureState::_Reset()
		{
			for (index_t i = _temporaryFallBackTo; i < _texturesBound; i++)
			{
				glActiveTexture((GLuint)(GL_TEXTURE0 + i));
				glBindTexture(GL_TEXTURE_1D,0);
				glBindTexture(GL_TEXTURE_2D,0);
				glBindTexture(GL_TEXTURE_3D,0);
				glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			}
			_Done();
			_texturesBound = _temporaryFallBackTo;
		}

		/**
		@brief Defines that subsequent texture bind operations should be appended to any currently bound textures.

		These temporarily bound textures will be reset each time new textures are bound. Textures that have been bound before
		LockCurrentBinding() was called will be preserved until UnlockBinding() is called.
		*/
		void			V2::TextureState::LockCurrentBinding()
		{
			_temporaryFallBackTo = _texturesBound;
		}
		/**
		@brief Unlocks any texture layers that may have been locked during any previous LockCurrentBinding() call.

		This method does not actually unbind any textures. To clear/redefine the current binding, invoke BindTextures().
		*/
		void			V2::TextureState::UnlockBinding()
		{
			_temporaryFallBackTo = 0;
		}















		bool	Shader::Install()	const
		{
			if (!handle)
			{
				Uninstall();
				return false;
			}
			
			GLShader::Template::globalRenderConfig.ReDetect();
			localShaderIsBound = handle->BuildShader()->PermissiveInstall();
			return localShaderIsBound;
		}

		/*static*/ bool Shader::_Install(const GLShader::Instance*instance)
		{
			localShaderIsBound = instance->PermissiveInstall();
			return localShaderIsBound;
		}
		
		/*static*/ void	Shader::Uninstall()
		{
			if (localShaderIsBound)	//we didn't install it, we don't uninstall it
				GLShader::Instance::PermissiveUninstall();
			localShaderIsBound= false;
		}
		
		String			Shader::Report()	const
		{
			if (!handle)
				return "No Handle set";
			return handle->Report();
		}
		
		bool	Shader::create(const SourceCode&code,bool use_global_status)
		{
			
			if (!handle)
				handle = SignalNew(new GLShader::Template());
		
			if (use_global_status)
			{
				handle->SetVariableMap(&globalMap);
				handle->SetConfig(&globalConfig);
			}
			else
			{
				handle->ResetVariableMap();
				handle->ResetConfig();	//private status
			}
			
			((TExtShaderConfiguration&)*this) = code;
			bool success = handle->LoadComposition(code.code);

			if (!success)
				return false;
			for (index_t i = 0; i < code.num_samplers; i++)
				handle->PredefineUniformInt("sampler"+String(i),(int)i);
			if (code.sky_lighting)
				handle->PredefineUniformInt("sky",(int)code.num_samplers);

			return true;
		}
		
		
		static String sampleTexture(index_t index, TextureDimension type, bool has_normal, const MaterialLayer&layer)
		{
			String coords = "gl_TexCoord["+String(index)+"]";
			/*if (layer.reflect)
			{
				if (!has_normal)
					FATAL__("Trying to reference mirrored texture coordinates during normal calculation. Normals are unavailable at this point");
				//coords = "reflect(frag_direction,"+coords+".xyz)";
				coords = "reflected";
			}*/
			switch (type)
			{
				case TextureDimension::Cube:
					return "textureCube(sampler"+String(index)+","+coords+".xyz)";
				case TextureDimension::Linear:
					return "texture1D(sampler"+String(index)+","+coords+".x)";
				case TextureDimension::Planar:
					return "texture2D(sampler"+String(index)+","+coords+".xy)";
				case TextureDimension::Volume:
					return "texture3D(sampler"+String(index)+","+coords+".xyz)";
			}
			FATAL__("Unexpected texture type encountered: #"+String(type));
			return "";
		}






		bool	Shader::compose(const MaterialComposition<Texture::Reference>&config, SourceCode&code_out)
		{
			StringBuffer	code,samplers;

			code_out.requires_tangents = false;

			bool	declared = false,need_refraction=false;	//true if normal/reflected/refracted have been declared before the fragment normal has been declared

			for (index_t i = 0; i < config.layers.Count(); i++)
			{
				switch (config.textures[i].GetDimension())
				{
					case TextureDimension::Linear:
						samplers << "uniform sampler1D sampler"<<i<<";\n";
					break;
					case TextureDimension::Planar:
						samplers << "uniform sampler2D sampler"<<i<<";\n";
					break;
					case TextureDimension::Volume:
						samplers << "uniform sampler3D sampler"<<i<<";\n";
					break;
					case TextureDimension::Cube:
						samplers << "uniform samplerCube sampler"<<i<<";\n";
					break;
				}
			}
			if (globalSkyTexture.IsNotEmpty())
			{
				samplers << "uniform samplerCube sky;\n";
				code_out.sky_lighting = true;
			}
			else
				code_out.sky_lighting = false;

			code_out.num_samplers = config.layers.Count();

			String	uniform_variables = need_refraction?"uniform float eta;\n":"",
				varying_variables = "varying vec3 frag_normal;\n"
				"varying vec3 frag_position, frag_direction;\n",
				fragment_variables;

			StringBuffer
				vertex_transformations = "frag_normal = gl_NormalMatrix*gl_Normal;\n"
				"frag_position =(gl_ModelViewMatrix*gl_Vertex).xyz;\n"
				"frag_direction = /*normalize*/(frag_position);\n"
				,
				fragment_transformations,
				normal_retrieval;

			count_t num_normal_maps = 0;
			for (index_t i = 0; i < config.layers.Count(); i++)
			{
				const MaterialLayer&layer = config.layers[i];
				if (config.textures[i].GetDimension() == TextureDimension::None)
					continue;

				if (layer.system_type == SystemType::Custom)
					vertex_transformations << "gl_TexCoord["<<i<<"] = gl_TextureMatrix["<<i<<"] * gl_MultiTexCoord"<<i<<";\n";
				else
					vertex_transformations << "gl_TexCoord["<<i<<"] = gl_MultiTexCoord"<<i<<";\n";
				if (layer.IsNormalMap())
				{
					if (num_normal_maps++)
						fragment_transformations << "normal += ";
					else
						fragment_transformations << "vec3 normal = ";
					if (layer.content_type == PixelType::TangentSpaceNormal)
					{
						code_out.requires_tangents = true;
						fragment_transformations << "tangent_space * (";
					}
					else
						fragment_transformations << "gl_NormalMatrix * (";

					String sampled = sampleTexture(i,config.textures[i].GetDimension(),false,layer);
					fragment_transformations << sampled<<".rgb*2.0-1.0";
					fragment_transformations << ");\n";
				}
			}
			if (code_out.requires_tangents)
			{
				varying_variables += "varying mat3 tangent_space;\n";
				varying_variables += "varying vec3 tangent,input_normal;\n";
				vertex_transformations << 	"tangent_space[0] = gl_NormalMatrix*gl_MultiTexCoord"<<config.layers.Count()<<".xyz;\n"
					"tangent_space[1] = normalize(cross(frag_normal,tangent_space[0]));\n"
					"tangent_space[2] = normalize(cross(tangent_space[0],tangent_space[1]));\n"
					"tangent = gl_MultiTexCoord"<<config.layers.Count()<<".xyz;\n"
					//"input_normal = gl_Normal;\n"
					;

			}
			if (num_normal_maps >= 1)
			{
				if (num_normal_maps > 1)
					fragment_transformations << "normal = normalize(normal);\n";
			}
			else
				fragment_variables += "vec3 normal = normalize(frag_normal);\n";


			StringBuffer	fragment_code;

			fragment_code << fragment_variables;
			fragment_code << fragment_transformations;
			fragment_code << "vec3 reflected = normalize(reflect(frag_direction,normal));\n";

			if (config.fully_reflective)
				fragment_code << "	float fresnel = 1.0;\n";
			else
				fragment_code <<"	float fresnel = 1.0-abs(dot(normal,frag_direction)/length(frag_direction));\n"
								"	fresnel = fresnel*fresnel;\n"
								"	fresnel = fresnel*fresnel;\n";

			if (code_out.sky_lighting)
			{
				fragment_code <<
						"	vec4 color = vec4(0.0,0.0,0.0,1.0);\n"
						"	{\n"
						//"		vec3 diffuse_color = textureCube(sky,(gl_TextureMatrix["<<config.layers.Count()<<"]*vec4(-normal,0.0)).xyz).rgb;\n"
						//"		if (gl_FrontMaterial.shininess > 0.0)\n"
						"		{\n"
						"			vec3 specular_color = textureCube(sky, (gl_TextureMatrix["<<config.layers.Count()<<"]*vec4(-reflected,0.0)).xyz).rgb;\n"
						//"			specular_color = pow(specular_color,gl_FrontMaterial.shininess*fresnel);\n"
						"			color.rgb += specular_color*fresnel*gl_FrontMaterial.specular.rgb;\n"
						"		}\n"

						//"		color.rgb += diffuse_color*gl_FrontMaterial.diffuse.rgb*(1.0-fresnel*0.5)*0.5;\n"
						//"		color.rgb += gl_FrontMaterial.ambient.rgb;\n"
						"	}\n";
				fragment_code << "color.rgb += shade(frag_position, normal, reflected).rgb*(1.0-fresnel*gl_FrontMaterial.specular.rgb);\n";
			}
			else
				fragment_code << "vec4 color = shade(frag_position, normal, reflected);\n";
			//fragment_code << "color.rgb += gl_FrontMaterial.emission.rgb*fresnel;\n";
			//fragment_code << "vec4 texture_color = vec4(1.0);\n";

			count_t colors_sampled=0;
				
			for (index_t i = 0; i < config.layers.Count(); i++)
			{
				const MaterialLayer&layer = config.layers[i];
				if (config.textures[i].GetDimension() == TextureDimension::None)
					continue;
				if (layer.IsNormalMap())
					continue;
				if (layer.combiner == GL_DECAL)
					continue;
				String sampled = sampleTexture(i,config.textures[i].GetDimension(),true,layer);
				if (!colors_sampled++)
				{
					fragment_code << "vec4 texture_color = "<<sampled<<";\n";
					continue;
				}

				switch (layer.combiner)
				{
					case GL_MODULATE:
						fragment_code << "texture_color *= "<<sampled<<";\n";
					break;
					case GL_BLEND:
						fragment_code << "{\nvec4 sampled = "<<sampled<<";\n"
						"texture_color = vec4(1.0)-((vec4(1.0)-texture_color)*(vec4(1.0)-sampled));\n}\n";
					break;
				}
			}

			if (colors_sampled)
				fragment_code << "color *= texture_color;\n";
			for (index_t i = 0; i < config.layers.Count(); i++)
			{
				const MaterialLayer&layer = config.layers[i];
				if (config.textures[i].GetDimension() == TextureDimension::None)
					continue;
				if (layer.IsNormalMap())
					continue;
				if (layer.combiner != GL_DECAL)
					continue;
				String sampled = sampleTexture(i,config.textures[i].GetDimension(),true,layer);
				fragment_code << "{\nvec4 sampled = "<<sampled<<";\n"
						"color = mix(color,sampled,sampled.w);\n}\n";
			}

			vertex_transformations << 
				"#if fog\n"
					"gl_FogFragCoord = clamp((gl_Fog.end - abs(frag_position.z))*gl_Fog.scale,0.0,1.0);\n"
				"#endif\n";


			fragment_code << 
				"#if fog\n"
					"color.rgb = mix(gl_Fog.color.rgb,color.rgb,gl_FogFragCoord);\n"
				"#endif\n";

			fragment_code << "gl_FragColor = color;\n";
			//fragment_code << "gl_FragColor = vec4(gl_TexCoord[0].xy,0.0,1.0);\n";

			/*if (code_out.requires_tangents)
				fragment_code << "gl_FragColor.rgb = (tangent.xyz)*0.5+0.5;\n";	//input_normal*/

			code << "[shared]\n"
				<< samplers
				<< uniform_variables
				<< varying_variables
				<< "[vertex]\n"
				<< "void main()\n"
				<< "{\n"
				<< "gl_Position = ftransform();\n"
				<< vertex_transformations
				//<< "vertexShadow();\n"
				<< "}\n"
				<< "[fragment]\n"
				<< "void main()\n"
				<< "{\n"
				<< fragment_code
				<< "}\n";

			code_out.code = code.ToStringRef();
			return true;
		}






	
		bool	Query::createQuery()
		{
			ContextLock	context_lock;

			glGetError();
			if (!glGenQueries)
				return false;
			if (!handle)
				glGenQueries(1,&handle);
			return glGetError() == GL_NO_ERROR;
		}
		
		void	Query::adoptData(Query&other)
		{
			Handle<GLuint>::adoptData(other);
			geometry_handle = other.geometry_handle;
			other.geometry_handle = 0;
		}

		void	Query::swap(Query&other)
		{
			Handle<GLuint>::swap(other);
			swp(geometry_handle,other.geometry_handle);
		}
	
		void	Query::clear()
		{
			ContextLock	context_lock;

			if (handle && glDeleteQueries)
				glDeleteQueries(1,&handle);
			if (geometry_handle)
				glDeleteLists(geometry_handle,1);
			flush();
		}
		
		void	Query::flush()
		{
			Handle<GLuint>::flush();
			geometry_handle = 0;
		}
		
		
	
		bool	Query::createPoint(const float point[3])
		{
			ContextLock	context_lock;

			if (!createQuery())
				return false;
			if (!geometry_handle)
				geometry_handle = glGenLists(1);
			if (!geometry_handle)
				return false;
			glNewList(geometry_handle,GL_COMPILE);
				glBegin(GL_POINTS);
					glVertex3fv(point); //overkill, ain't it?
				glEnd();
			glEndList();
			return true;
		}
		
		bool	Query::create()
		{
			ContextLock	context_lock;

			if (!createQuery())
				return false;
			if (geometry_handle)
			{
				glDeleteLists(geometry_handle,1);
				geometry_handle = 0;
			}
			return true;
		}
		
		bool	Query::createBox(const float lower[3],const float upper[3])
		{
			ContextLock	context_lock;

			if (!createQuery())
				return false;
			if (!geometry_handle)
				geometry_handle = glGenLists(1);
			if (!geometry_handle)
				return false;
			glNewList(geometry_handle,GL_COMPILE);
				glBegin(GL_QUADS);
					glVertex3fv(lower);
					glVertex3f(lower[0],lower[1],upper[2]);
					glVertex3f(lower[0],upper[1],upper[2]);
					glVertex3f(lower[0],upper[1],lower[2]);

					glVertex3f(upper[0],lower[1],upper[2]);
					glVertex3f(upper[0],lower[1],lower[2]);
					glVertex3f(upper[0],upper[1],lower[2]);
					glVertex3f(upper[0],upper[1],upper[2]);

					glVertex3f(lower[0],upper[1],lower[2]);
					glVertex3f(lower[0],upper[1],upper[2]);
					glVertex3f(upper[0],upper[1],upper[2]);
					glVertex3f(upper[0],upper[1],lower[2]);

					glVertex3f(lower[0],lower[1],upper[2]);
					glVertex3f(lower[0],lower[1],lower[2]);
					glVertex3f(upper[0],lower[1],lower[2]);
					glVertex3f(upper[0],lower[1],upper[2]);

					glVertex3f(lower[0],lower[1],upper[2]);
					glVertex3f(upper[0],lower[1],upper[2]);
					glVertex3f(upper[0],upper[1],upper[2]);
					glVertex3f(lower[0],upper[1],upper[2]);

					glVertex3f(upper[0],lower[1],lower[2]);
					glVertex3f(lower[0],lower[1],lower[2]);
					glVertex3f(lower[0],upper[1],lower[2]);
					glVertex3f(upper[0],upper[1],lower[2]);
				glEnd();
			glEndList();
			return true;
		}	


	
		void		Shader::flush()
		{
			Handle<GLShader::Template*>::flush();
			requires_tangents = false;
		}


		void		Shader::clear()
		{
			if (handle)
				Discard(handle);
			flush();
		}
		
		
		
		bool			Texture::configureFilter(GLenum target, TextureFilter filter, float anisotropy)
		{
			switch (filter)
			{
				case TextureFilter::None:
					glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				return false;
				case TextureFilter::Linear:
					glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				return false;
				case TextureFilter::Bilinear:
					glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
					glTexParameterf( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
					glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				return true;
				case TextureFilter::Trilinear:
					glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameterf( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
					glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				return true;
			}
			FATAL__("Unexpected filter type #"+String(filter));
			return false;
		}


		void			Texture::clear()
		{
			if (handle)
			{
				ContextLock	context_lock;
				glDeleteTextures(1,&handle);
			}
			flush();
		}
		
		void			Texture::flush()
		{
			GenericTexture::flush();
			Handle<GLuint>::flush();
		}
		
		void			Texture::overrideSetHandle(GLuint h, unsigned width, unsigned height, BYTE num_channels, PixelType type, bool do_clear)
		{
			if (do_clear)
				clear();
			handle = h;
			texture_width = width;
			texture_height = height;
			texture_channels = num_channels;
			texture_dimension = TextureDimension::Planar;
			texture_type = type;
		}
		
		
		void			Texture::adoptData(Texture&other)
		{
			Handle<GLuint>::adoptData(other);
			GenericTexture::adoptData(other);
		}
		void			Texture::swap(Texture&other)
		{
			Handle<GLuint>::swap(other);
			GenericTexture::swap(other);
		}
		
		bool			Texture::Resize(GLuint width_, GLuint height_, BYTE channels_)
		{
			if (!width_ || !height_ || !channels_ || channels_ > 4)
				return false;
			ContextLock	context_lock;

			if (!handle)
			{
				glGetError();
				glGenTextures(1,&handle);
				ASSERT__(handle!=0);
				glBindTexture(GL_TEXTURE_2D,handle);
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_2D,0);
				if (glGetError() != GL_NO_ERROR)
					return false;
			}
			if (texture_width != width_ || texture_height != height_ || (texture_width != 0 && texture_height != 0 && texture_channels != channels_) || texture_dimension == TextureDimension::Cube)
			{
				texture_width = width_;
				texture_height = height_;
				texture_channels = channels_;
				texture_dimension = TextureDimension::Planar;
				glGetError();
				glBindTexture(GL_TEXTURE_2D,handle);
					GLenum internal_format, import_format;
					formatAt<GLType<GLbyte> >(texture_channels,texture_type,false, internal_format, import_format);
					glTexImage2D( GL_TEXTURE_2D, 0, internal_format, texture_width, texture_height, 0, import_format, GL_UNSIGNED_BYTE, NULL);
				glBindTexture(GL_TEXTURE_2D,0);
				return glGetError() == GL_NO_ERROR;
			}
			return true;
		}
	
		bool	Texture::ExportTo(Image&target)	const
		{
			if (!handle)
				return false;
			target.SetSize(texture_width,texture_height,texture_channels);
			ContextLock	context_lock;

			glPushAttrib(GL_TEXTURE_BIT);
				glBindTexture(GL_TEXTURE_2D,handle);
				glGetTexImage(GL_TEXTURE_2D,0,texture_channels==3?GL_RGB:GL_RGBA,GL_UNSIGNED_BYTE,target.GetData());
			glPopAttrib();
			return true;
		}

		bool			Texture::load(const Buffer&object, GLuint width_, GLuint height_, BYTE channels_)
		{
			if (object.IsEmpty())
				return false;
			ContextLock	context_lock;

			if (!Resize(width_,height_,channels_))
				return false;


			#ifndef GL_PIXEL_UNPACK_BUFFER_ARB
				#define GL_PIXEL_UNPACK_BUFFER_ARB	0x88EC
			#endif
			//cout << "read "<<width<<"*"<<height<<"*3"<<endl;
			
			glGetError();
			glBindTexture(GL_TEXTURE_2D,handle);

			glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, object.getHandle());

			GLenum internal_format, import_format;
			formatAt<GLType<GLbyte> >(texture_channels,texture_type,false, internal_format, import_format);
			
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0,
							 texture_width, texture_height,
							 import_format, GL_UNSIGNED_BYTE, NULL);
			
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);

			glBindTexture(GL_TEXTURE_2D,0);
			return (glGetError() == GL_NO_ERROR);
		}

		
	
		void	Buffer::clear()
		{

			if (handle)
			{
				ContextLock	context_lock;
				glDeleteBuffers(1,&handle);
			}
			flush();
		}
		
		void	Buffer::flush()
		{
			Handle<GLuint>::flush();
			data_size = 0;
		}

		void	Buffer::adoptData(Buffer&other)
		{
			Handle<GLuint>::adoptData(other);
			data_size = other.data_size;
			other.data_size = 0;
		}
	
		void	Buffer::swap(Buffer&other)
		{
			Handle<GLuint>::swap(other);
			swp(data_size,other.data_size);
		}
		
		void	Buffer::Resize(size_t size_, GLenum type)
		{
			ContextLock	context_lock;

			if (!handle)
			{
				ASSERT_NOT_NULL__(glGenBuffers);
				glGenBuffers(1,&handle);
				ASSERT__(handle != 0);
			}
			if (data_size == size_)
				return;
			
			
			glGetError();
			glBindBuffer(type,handle);
			glBufferData(type,size_,NULL,GL_STATIC_DRAW_ARB);
			glBindBuffer(type,0);
			ASSERT__(glGetError()==GL_NO_ERROR);
			data_size = size_;
		}
		
		bool	Buffer::loadData(const void*data, size_t size_, GLenum type)
		{
			if (!glBindBuffer)
				return false;
			ContextLock	context_lock;

			if (!handle)
			{
				ASSERT_NOT_NULL__(glGenBuffers);
				glGenBuffers(1,&handle);
				ASSERT__(handle != 0);
			}
			glGetError();
			glBindBuffer(type,handle);
			glBufferData(type,size_,data,GL_STATIC_DRAW_ARB);
			glBindBuffer(type,0);
			data_size = size_;
			return glGetError()==GL_NO_ERROR;
		}

		bool		Buffer::loadData(const void*data, GLenum type)
		{
			if (!glBindBuffer)
				return false;
			ContextLock	context_lock;

			glGetError();
			glBindBuffer(type,handle);
			glBufferSubData(type,0,data_size,data);
			//glBufferData(type,data_size,data,GL_STATIC_DRAW_ARB);
			glBindBuffer(type,0);
			return glGetError()==GL_NO_ERROR;
		}





		void	SmartBuffer::clear()
		{
			if (on_device)
			{
				if (!application_shutting_down)
				{
					ContextLock	context_lock;

					if (device_handle)
						glDeleteBuffers(1,&device_handle);
				}
			}
			else
				dealloc(host_data);

			on_device = false;	//implies device_handle=0
			host_data = NULL;
			data_size = 0;
		}
		
		void	SmartBuffer::Resize(size_t size_, GLenum type)
		{
			if (size_ == data_size)
				return;
			ContextLock	context_lock;

			if (!host_data)	//clear
			{
				if (glGenBuffers != NULL && OpenGL::queryGeometryUpload())
				{
					glGenBuffers(1,&device_handle);
					on_device = true;
				}
				else
					on_device = false;
			}
			if (!on_device)
				re_alloc(host_data, size_);
			else
			{
				glGetError();
				glBindBuffer(type,device_handle);
				glBufferData(type,size_,NULL,GL_STATIC_DRAW_ARB);
				glBindBuffer(type,0);
				ASSERT__(glGetError()==GL_NO_ERROR);
			}
			data_size = size_;
		}
		
		void	SmartBuffer::loadData(const void*data, size_t size_, GLenum type)
		{
			ContextLock	context_lock;

			if (!host_data)	//clear
			{
				if (glGenBuffers != NULL && OpenGL::queryGeometryUpload())
				{
					glGenBuffers(1,&device_handle);
					on_device = true;
				}
				else
					on_device = false;
			}
			if (!on_device)
			{
				reloc(host_data, data_size, size_);
				memcpy(host_data,data,size_);
			}
			else
			{
				glGetError();
				glBindBuffer(type,device_handle);
				glBufferData(type,size_,data,GL_STATIC_DRAW_ARB);
				glBindBuffer(type,0);
				ASSERT__(glGetError()==GL_NO_ERROR);
				data_size = size_;
			}
		}

		void		SmartBuffer::streamData(const void*data, size_t size_, GLenum type)
		{
			ASSERT_LESS_OR_EQUAL__(size_,data_size);
			if (!on_device)
			{
				memcpy(host_data,data,size_);
			}
			else
			{
				ContextLock	context_lock;

				glGetError();
				glBindBuffer(type,device_handle);
				glBufferSubData(type,0,size_,data);
				glBindBuffer(type,0);
				ASSERT__(glGetError()==GL_NO_ERROR);
			}
		}


		bool	FBO::exportColorTo(FloatImage&target,BYTE target_index/*=0*/)	const
		{
			if (target_index >= config.numColorTargets || !config.colorTarget[target_index].textureHandle)
				return false;
			BYTE channels;
			PixelType pt;
			Decode(config.colorTarget[target_index].textureFormat, channels,pt);
			target.SetSize(config.resolution.width,config.resolution.height,channels);
			target.SetContentType(pt);

			ContextLock	context_lock;

			glPushAttrib(GL_TEXTURE_BIT);
				glBindTexture(GL_TEXTURE_2D,config.colorTarget[target_index].textureHandle);
				glGetTexImage(GL_TEXTURE_2D,0,formatToOrder(config.colorTarget[target_index].textureFormat),GL_FLOAT,target.GetData());
			glPopAttrib();
			return true;
		}

		bool	FBO::exportColorTo(UnclampedFloatImage&target,BYTE target_index/*=0*/)	const
		{
			if (target_index >= config.numColorTargets || !config.colorTarget[target_index].textureHandle)
				return false;
			BYTE channels;
			PixelType pt;
			Decode(config.colorTarget[target_index].textureFormat, channels,pt);
			target.SetSize(config.resolution.width,config.resolution.height,channels);
			target.SetContentType(pt);

			ContextLock	context_lock;

			glPushAttrib(GL_TEXTURE_BIT);
				glBindTexture(GL_TEXTURE_2D,config.colorTarget[target_index].textureHandle);
				glGetTexImage(GL_TEXTURE_2D,0,formatToOrder(config.colorTarget[target_index].textureFormat),GL_FLOAT,target.GetData());
			glPopAttrib();
			return true;
		}

		bool	FBO::exportColorTo(Image&target,BYTE target_index/*=0*/)	const
		{
			if (target_index >= config.numColorTargets || !config.colorTarget[target_index].textureHandle)
				return false;
			BYTE channels;
			PixelType pt;
			Decode(config.colorTarget[target_index].textureFormat, channels,pt);
			target.SetSize(config.resolution.width,config.resolution.height,channels);
			target.SetContentType(pt);

			ContextLock	context_lock;

			glPushAttrib(GL_TEXTURE_BIT);
				glBindTexture(GL_TEXTURE_2D,config.colorTarget[target_index].textureHandle);
				glGetTexImage(GL_TEXTURE_2D,0,formatToOrder(config.colorTarget[target_index].textureFormat),GL_UNSIGNED_BYTE,target.GetData());
			glPopAttrib();
			return true;
		}

		
		void		FBO::flush()
		{
			Handle<GLuint>::flush();
			config = TFBOConfig();
		}
		
		void		FBO::clear()
		{
			if (handle)
			{
				ContextLock	context_lock;

				Extension::DestroyFrameBuffer(TFrameBuffer(handle,config));
			}
			flush();
		}
		
		void		FBO::adoptData(FBO&other)
		{
			Handle<GLuint>::adoptData(other);
			config = other.config;
			other.config = TFBOConfig();
		}
		void		FBO::swap(FBO&other)
		{
			Handle<GLuint>::swap(other);
			swp(config,other.config);
		}

		void		FBO::Resize(const Resolution&res)
		{
			if (res == config.resolution)
				return;
			ContextLock	context_lock;
			TFrameBuffer	buffer;
			((TFBOConfig&)buffer) = config;
			buffer.frameBuffer = handle;
			Extension::ResizeFrameBuffer(buffer,res);
			config = buffer; //handle should stay the same
		}
		

		bool		FBO::create(const Configuration&config)
		{
			ContextLock	context_lock;

			clear();
			TFrameBuffer	buffer = Extension::CreateFrameBuffer(config.resolution, config.depthStorage,config.numColorTargets, config.format);
			if (!buffer.frameBuffer)
				return false;
			handle = buffer.frameBuffer;
			this->config = buffer;
			if (!config.filtered)	//defaults to filtered
			{
				for (int i = 0; i < 4; i++)
					if (this->config.colorTarget[i].textureHandle)
					{
						glBindTexture(GL_TEXTURE_2D,this->config.colorTarget[i].textureHandle);
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
					}
				glBindTexture(GL_TEXTURE_2D,0);
			}
			return true;
		}
	
		GLenum formatToOrder(GLenum format)
		{
			switch (format)
			{
				case GL_RGBA:
				case GL_RGBA8:
				case GL_RGBA16F:
				case GL_RGBA32F:
				case GL_RGBA16:
				case GL_RGBA32I:
				case GL_RGBA32UI:
					return GL_RGBA;
				case GL_RGB:
				case GL_RGB8:
				case GL_RGB16:
				case GL_RGB32I:
				case GL_RGB32UI:
				case GL_RGB16F:
				case GL_RGB32F:
					return GL_RGB;
				case GL_LUMINANCE_ALPHA:
				case GL_LUMINANCE8_ALPHA8:
					return GL_LUMINANCE_ALPHA;
				case GL_RG:
				case GL_RG8:
				case GL_RG16:
				case GL_RG32UI:
				case GL_RG32I:
				case GL_RG16F:
				case GL_RG32F:
					return GL_RG;
				case GL_ALPHA:
				case GL_ALPHA8:
				case GL_ALPHA16:
				case GL_ALPHA16F_ARB:
					return GL_ALPHA;
				case GL_LUMINANCE:
				case GL_LUMINANCE8:
				case GL_LUMINANCE16:
				case GL_LUMINANCE16F_ARB:
					return GL_LUMINANCE;
				case GL_R:
				case GL_R8:
				case GL_R16:
				case GL_R32I:
				case GL_R32UI:
				case GL_R16F:
				case GL_R32F:
					return GL_RED;
				default:
					FATAL__("Unexpected opengl format enumeration value");
					return 0;
			}

		}

		void Decode(GLenum format, BYTE&channels, PixelType&pt)
		{
			switch (format)
			{
			case GL_RGBA:
			case GL_RGBA8:
			case GL_RGBA16F:
			case GL_RGBA32F:
			case GL_RGBA16:
			case GL_RGBA32I:
			case GL_RGBA32UI:
				channels = 4;
				pt = PixelType::Color;
			break;
			case GL_RGB:
			case GL_RGB8:
			case GL_RGB16:
			case GL_RGB32I:
			case GL_RGB32UI:
			case GL_RGB16F:
			case GL_RGB32F:
				channels = 3;
				pt = PixelType::Color;
			break;
			case GL_LUMINANCE_ALPHA:
			case GL_LUMINANCE8_ALPHA8:
				channels = 2;
				pt = PixelType::Color;
			break;
			case GL_RG:
			case GL_RG8:
			case GL_RG16:
			case GL_RG32UI:
			case GL_RG32I:
			case GL_RG16F:
			case GL_RG32F:
				channels = 2;
				pt = PixelType::StrictColor;
			break;
			case GL_ALPHA:
			case GL_ALPHA8:
			case GL_ALPHA16:
			case GL_ALPHA16F_ARB:
				channels = 1;
				pt = PixelType::Color;
			break;
			case GL_LUMINANCE:
			case GL_LUMINANCE8:
			case GL_LUMINANCE16:
			case GL_LUMINANCE16F_ARB:
				channels = 1;
				pt = PixelType::PureLuminance;
			break;
			case GL_R:
			case GL_R8:
			case GL_R16:
			case GL_R32I:
			case GL_R32UI:
			case GL_R16F:
			case GL_R32F:
				channels = 1;
				pt = PixelType::StrictColor;
			break;
			default:
				FATAL__("Unexpected opengl format enumeration value");
				channels = 0;
				pt = PixelType::None;
			break;
			}

		}
		Texture::Reference		FBO::reference(UINT target/*=0*/)	const
		{
			Texture t;
			BYTE channels;
			PixelType pt;
			Decode(config.colorTarget[target].textureFormat,channels,pt);
			t.overrideSetHandle(config.colorTarget[target].textureHandle,config.resolution.width,config.resolution.height,channels,pt,false);
			Texture::Reference result(t.reference());
			t.flush();
			return result;

		}

		Texture::Reference		FBO::ReferDepth()	const
		{
			ASSERT_EQUAL__(config.depthTarget.storageType,DepthStorage::Texture);
			Texture t;
			BYTE channels = 1;
			PixelType pt = PixelType::Depth;
			t.overrideSetHandle(config.depthTarget.handle,config.resolution.width,config.resolution.height,channels,pt,false);
			Texture::Reference result(t.reference());
			t.flush();
			return result;

		}


		void		FBO::generateMIPLayers(UINT target/*=0*/)
		{
			if (IsEmpty() || !glGenerateMipmap)
				return;
			ContextLock	context_lock;

			glBindTexture(GL_TEXTURE_2D,config.colorTarget[target].textureHandle);
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glBindTexture(GL_TEXTURE_2D,0);
		}
	
	
	
		RenderSetup::RenderSetup():bound_texture_layers(0),bound_texcoord_layers(0),expect_tangent_normals(false),changed(false),material(NULL),enabled_light_field(8),num_enabled_lights(0)
		{
			memset(matrix_type,0,sizeof(matrix_type));
		}

		void		RenderSetup::updateSpecs(StringBuffer&target)	const
		{
			target <<  '&' << (expect_tangent_normals?"1":"0");
			target <<  '&' << (lighting_enabled?"1":"0");

			//target <<  '&' << material.flags;
			target <<  '&' << bound_texture_layers;
			target <<  '&' << bound_texcoord_layers;

			target << '&';
			/*
			for (unsigned i = 0; i < material.layers(); i++)
			{
				target << '&' << material.layer(i).flags;
				target << '&' << material.layer(i).combiner;
				target << '&' << (unsigned)texture_type[i];
			}*/

			target << '&';

			if (lighting_enabled)
				for (unsigned i = 0; i < num_enabled_lights; i++)
				{
					PLight light = enabled_light_field[i];
					target << '&' << (unsigned)light->GetType();
				}
		}



		RenderState::RenderState():	indices_bound(false),
										matrix_changed(false),in_query(false),single_texture_bound(false),
										reconstruct_shader(true),upload_geometry(true),
										geometry_lock(false)
		{

		}
	
	
	
	
	
	
	
	}
	
	
	
	
	
	
	//CGLRenderState				OpenGL::state;
	//ModularDataTable<GLuint>		OpenGL::texture_table;
	int								OpenGL::error_code;					//error-code from the last operation
	OpenGL							*OpenGL::globalInstance(NULL);
	/*static*/		Ctr::PreAllocVector<OpenGL::GLBinding,4>	OpenGL::created_contexts;		//!< All created contexts
	
	//HashContainer<CGLShader>		OpenGL::shader_cache;

	
	
	/*static*/			GL::ContextLock::ContextLock()
	{
		if (OpenGL::hasCurrentContext())
		{
			is_bound = false;
			return;
		}
		if (OpenGL::created_contexts.IsEmpty())
			throw Except::Renderer::GeneralFault(Except::globalString("Cannot bind OpenGL context: none created"));
		is_bound = true;
		OpenGL::setCurrentContext(OpenGL::created_contexts.First());
	}

	/*static*/			GL::ContextLock::~ContextLock()
	{
		if (!is_bound)
			return;
		is_bound = false;
		if (OpenGL::created_contexts.IsEmpty())
			FATAL__("Cannot unbind OpenGL context: no reference context");
		#if SYSTEM==WINDOWS
			wglMakeCurrent(OpenGL::created_contexts.First().device_context, NULL);
		#elif SYSTEM_VARIANCE==LINUX
			glXMakeCurrent(OpenGL::created_contexts.First().display,None,NULL);
		#endif
	}


	/*static*/ void	OpenGL::BuildMipMaps(const GL::Texture::Reference&ref, const GLShader::Instance&mipMapShader, GLenum format)
	{
		const GLuint texHandle = ref.GetHandle();
		const GLenum texFormat = format;
		ASSERT__(texHandle != 0);
		
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glColorMask(true,true,true,true);

		glGetError();
		static GLuint	subFBO = 0;
		if (!subFBO)
		{
			GLenum status;
			
			ASSERT__(glGenFramebuffers);

			glGenFramebuffers( 1, &subFBO );
		}

		glBindFramebuffer(GL_FRAMEBUFFER, subFBO);
		GLenum	draw_buffers[GL_MAX_COLOR_ATTACHMENTS] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1,draw_buffers);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,texHandle);
		ASSERT__(mipMapShader.Install());

		Resolution res = Resolution(ref.GetWidth(),ref.GetHeight());
		unsigned layer = 0;
		do
		{
			float sx = 1.f / float(res.width);
			float sy = 1.f / float(res.height);
			
			res/= 2;
			layer++;
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,layer-1);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,layer-1);
			glTexImage2D(GL_TEXTURE_2D,layer,texFormat,res.width,res.height,0,GL_RGBA,GL_BYTE,NULL);
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHandle, layer );
			glViewport(0,0,res.width,res.height);
			//glClearColor(0,1,0,1);
			//glClear(GL_COLOR_BUFFER_BIT);

			glExtensions.TestCurrentFrameBuffer();

			glBegin(GL_QUADS);
				glTexCoord4f(0,0, sx, sy);	glVertex2f(-1,-1);
				glTexCoord4f(1,0, sx, sy);	glVertex2f(1,-1);
				glTexCoord4f(1,1, sx, sy);	glVertex2f(1,1);
				glTexCoord4f(0,1, sx, sy);	glVertex2f(-1,1);
			glEnd();
			glThrowError();
		}
		while (res.width > 1 || res.height > 1);
		
		mipMapShader.Uninstall();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,1000);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_2D);
		glThrowError();

	}

	/*static*/ void	OpenGL::BuildMipMaps(const GL::FBO&fbo, unsigned target, const GLShader::Instance&mipMapShader)
	{
		ASSERT_LESS__(target, 4);
		BuildMipMaps(fbo.Refer(target),mipMapShader, fbo.GetConfig().colorTarget[target].textureFormat);
	}
	

	void	OpenGL::Capture(Image&target)
	{
		DBG_ASSERT__(hasCurrentContext());
		if (target.GetChannels() != 3 && target.GetChannels() != 4)
			target.SetChannels(3);
		//set pixel read 
		glGetError();
		glReadPixels(0,0,target.GetWidth(),target.GetHeight(),  target.GetChannels()==4?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,target.GetData());
		ASSERT_EQUAL__(glGetError(), GL_NO_ERROR);
	}

	void	OpenGL::Capture(FloatImage&target)
	{
		DBG_ASSERT__(hasCurrentContext());
		if (target.GetChannels() != 3 && target.GetChannels() != 4)
			target.SetChannels(3);
		glGetError();
		glReadPixels(0,0,target.GetWidth(),target.GetHeight(),  target.GetChannels()==4?GL_RGBA:GL_RGB,GL_FLOAT,target.GetData());
		ASSERT_EQUAL__(glGetError(), GL_NO_ERROR);
	}
	
	void	OpenGL::Capture(Texture&target, unsigned width, unsigned height)
	{
		if (width == 0 || height == 0)
			return;
		DBG_ASSERT__(hasCurrentContext());
		if (target.GetWidth() != width || target.GetHeight() != height || target.channels() != 3)
			target.loadPixels<GLbyte>(NULL, width, height, 3, PixelType::Color,1.0f, true, TextureFilter::Linear);
		glGetError();
		glBindTexture(GL_TEXTURE_2D,target.getHandle());
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,  width, height);
		glBindTexture(GL_TEXTURE_2D,0);
		GLenum err = glGetError();
		ASSERT_EQUAL__(err, GL_NO_ERROR);
	}
	
	void	OpenGL::CaptureDepth(Texture&target, unsigned width, unsigned height)
	{
		if (width == 0 || height == 0)
			return;
		DBG_ASSERT__(hasCurrentContext());
		if (target.GetWidth() != width || target.GetHeight() != height || target.channels() != 3)
			target.loadPixels<GLDepthComponent>(NULL, width, height, 1, PixelType::Depth,1.0f, true, TextureFilter::Linear);
		glGetError();
		glBindTexture(GL_TEXTURE_2D,target.getHandle());
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
			glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,  width, height);
		glBindTexture(GL_TEXTURE_2D,0);
		GLenum err = glGetError();
		ASSERT_EQUAL__(err, GL_NO_ERROR);
	}
	
	
	
	
	//OpenGL::ShaderObject		OpenGL::shaderObject(const MaterialConfiguration&configuration, const Array<TextureObject>&textures, bool strict);




	bool	OpenGL::enableGeometryUpload()
	{
		GL_BEGIN
		if (state.geometry_lock)
		{
			GL_END
			return queryGeometryUpload();
		}
		state.upload_geometry = true;
		GL_END
		return glGenBuffers!=NULL;
	}

	void	OpenGL::disableGeometryUpload()
	{
		if (state.geometry_lock)
			return;
		state.upload_geometry = false;
	}

	bool	OpenGL::queryGeometryUpload()
	{
		return state.upload_geometry && glGenBuffers!=NULL;
	}

	void	OpenGL::lockGeometryUpload()
	{
		state.geometry_lock = true;
	}

	void	OpenGL::unlockGeometryUpload()
	{
		state.geometry_lock = false;
	}

	bool	OpenGL::queryGeometryUploadLock()
	{
		return state.geometry_lock;
	}



	void OpenGL::enableLight(const PLight&light)
	{
		GL_BEGIN
		if (getLightSceneOf(light) != active_scene || state.render_setup.num_enabled_lights >= state.render_setup.enabled_light_field.GetLength())	//can't enable
		{
			GL_END
			return;
		}
		setIndexOf(light,state.render_setup.num_enabled_lights);
		state.render_setup.enabled_light_field[state.render_setup.num_enabled_lights++] = light;
		updateLight(light);
		state.render_setup.changed = true;
		GL_END
	}

	void OpenGL::disableLight(const PLight&light)
	{
		GL_BEGIN
		if (getLightSceneOf(light) != active_scene)
			setIndexOf(light,InvalidIndex);

		index_t index = getIndexOf(light);

		if (index == InvalidIndex)
		{
			GL_END
			return;
		}
		if (index >= state.render_setup.num_enabled_lights)
			FATAL__("invalid state");
		state.render_setup.num_enabled_lights --;
		for (index_t i = index; i < state.render_setup.num_enabled_lights; i++)
		{
			state.render_setup.enabled_light_field[i] = state.render_setup.enabled_light_field[i+1];
			setIndexOf(state.render_setup.enabled_light_field[i], i);
			updateLight(state.render_setup.enabled_light_field[i]);
		}
		glDisable(GL_LIGHT0+GLenum(state.render_setup.num_enabled_lights));
		setIndexOf(light,InvalidIndex);
		state.render_setup.changed = true;
		GL_END
	}


	void OpenGL::updateLight(const PLight&light)
	{
		GL_BEGIN
		index_t index = getIndexOf(light);
		if (verbose)
			log_file << "updateLight("<<index<<"): updating light."<<nl;

		if (index == InvalidIndex || getLightSceneOf(light) != active_scene)
		{
			GL_END
			return;
		}
		if (verbose)
			log_file << "updateLight("<<index<<"): is active."<<nl;

		GLenum	l = GL_LIGHT0+GLenum(index);
		glEnable(l);
		glLightfv(l,GL_DIFFUSE,light->GetDiffuse().v);
		glLightfv(l,GL_AMBIENT,light->GetAmbient().v);
		glLightfv(l,GL_SPECULAR,light->GetSpecular().v);
		if (verbose)
			log_file << "updateLight("<<index<<"): color set to diffuse: "<<M::Vec::toString(light->GetDiffuse())<<" ambient: "<<M::Vec::toString(light->GetAmbient())<<" specular: "<<M::Vec::toString(light->GetSpecular())<<nl;
		if (light->GetType() == Light::Spot)
		{
			glLightf(l,GL_SPOT_CUTOFF,light->GetSpotCutoff());
			glLighti(l,GL_SPOT_EXPONENT,light->GetSpotExponent());
			if (verbose)
				log_file << "updateLight("<<index<<"): light type is spot (cutoff: "<<light->GetSpotCutoff()<<"; exponent: "<<light->GetSpotExponent()<<")"<<nl;
		}
		else
		{
			glLightf(l,GL_SPOT_CUTOFF,180);
			glLighti(l,GL_SPOT_EXPONENT,0);
			if (verbose)
				log_file << "updateLight("<<index<<"): light type is direct or omni (no cutoff or exponent)"<<nl;
		}
		if (verbose)
			log_file << "updateLight("<<index<<"): update attenuation to: const: "<<light->GetConstantAttenuation()<<" linear: "<<light->GetLinearAttenuation()<<" quad: "<<light->GetQuadraticAttenuation()<<nl;
		glLightf(l,GL_LINEAR_ATTENUATION,light->GetLinearAttenuation());
		glLightf(l,GL_QUADRATIC_ATTENUATION,light->GetQuadraticAttenuation());
		glLightf(l, GL_CONSTANT_ATTENUATION,light->GetConstantAttenuation());

		if (lighting_enabled)
		{
			if (verbose)
				log_file<<"updateLight("<<index<<"): lighting is enabled. updating light position ("<<index<<")"<<nl;
			updateLightPosition(light);
		}
		setWasModified(light,false);
		state.render_setup.changed = true;
		GL_END
	}

	void OpenGL::updateLightPosition(const PLight&light)
	{
		GL_BEGIN
		index_t index = getIndexOf(light);
		M::TVec4<> p;
		p.xyz = light->GetPosition();
		p.w = light->GetType() == Light::Direct?0.0f:1.0f;
		glEnable(GL_LIGHT0+GLenum(index));	//just in case
		glLightfv(GL_LIGHT0+GLenum(index),GL_POSITION,p.v);
			if (verbose)
				log_file<<"updateLightPosition(): light position ("<<index<<") updated to "<<M::Vec::toString(p)<<nl;
		if (light->GetType() == Light::Spot)
			glLightfv(GL_LIGHT0+GLenum(index),GL_SPOT_DIRECTION,light->GetSpotDirection().v);
		setHasMoved(light,false);
		GL_END
	}


	void	OpenGL::updateLighting(bool force)
	{
		GL_BEGIN
		if (verbose)
			log_file << "updateLighting(): updating all lights ("<<state.render_setup.num_enabled_lights<<"). matrix_changed is "<<(state.matrix_changed?"true":"false")<<nl;
		for (index_t i = 0; i < state.render_setup.num_enabled_lights; i++)
			if (force || wasModified(state.render_setup.enabled_light_field[i]))
				updateLight(state.render_setup.enabled_light_field[i]);
			else
				if (state.matrix_changed || hasMoved(state.render_setup.enabled_light_field[i]))
					updateLightPosition(state.render_setup.enabled_light_field[i]);
		GL_END
	}

	void	OpenGL::setVerbose(bool v)
	{
		verbose = v;
	}
	
	bool	OpenGL::lightingEnabled()	const
	{
		return lighting_enabled;
	}

	void	OpenGL::enableLighting()
	{
		GL_BEGIN
		if (verbose)
			log_file << "enableLighting(): "<<state.render_setup.num_enabled_lights<<" light(s)"<<nl;
		glEnable(GL_LIGHTING);

		lighting_enabled = true;	//how old is this code? and STILL there was a hidden bug. if 'lighting_enabled' is set to true after updating the lights then positions are not updated
		updateLighting();
		state.matrix_changed = false;

		GL_END
	}

	void	OpenGL::disableLighting()
	{
		glDisable(GL_LIGHTING);
		lighting_enabled = false;
	}



	bool			OpenGL::pickLightScene(index_t index)
	{
		GL_BEGIN
		LightScene*scene = scenes.lookup(index);
		if (!scene)
		{
			GL_END
			return false;
		}
		if (active_scene == scene)
		{
			GL_END
			return true;
		}

		index_t last_assigned = state.render_setup.num_enabled_lights;
		state.render_setup.num_enabled_lights = 0;
		scene->reset();
		for (LightScene::iterator light = scene->begin(); light != scene->end(); ++light)
			if ((*light)->isEnabled())
			{
				setIndexOf(*light,state.render_setup.num_enabled_lights);
				state.render_setup.enabled_light_field[state.render_setup.num_enabled_lights++] = *light;
				updateLight(*light);
			}
		for (index_t i = state.render_setup.num_enabled_lights; i < last_assigned; i++)
			glDisable(GL_LIGHT0+GLenum(i));

		active_scene = scene;
		active_scene_index = index;
		state.render_setup.changed = true;
		GL_END
		return true;
	}




	GLuint OpenGL::resolveQuery(const Query&query)
	{
		GL_BEGIN
		if (!query.handle)
		{
			GL_END
			return 0;
		}
		GLuint count;
		glGetQueryObjectuiv(query.handle,GL_QUERY_RESULT_ARB,&count);
		GL_END
		return count;
	}

	#if 0
	void OpenGL::loadFloatTexture(const FloatImage*img, bool use_mipmap, GLuint format, GLuint internal_format)
	{
		GL_BEGIN
			cout << format << "/"<<internal_format<<endl;
		if (img->GetContentType() == Image::ColorMap)
		{
			if (glGenerateMipmap && img->GetHeight() > 1)
			{
				glTexImage2D(GL_TEXTURE_2D,0,internal_format,img->GetWidth(),img->GetHeight(),0,format,GL_FLOAT,img->GetData());
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				if (img->GetHeight() > 1)
					gluBuild2DMipmaps(GL_TEXTURE_2D,internal_format,img->GetWidth(),img->GetHeight(),format,GL_FLOAT,img->GetData());
				else
					gluBuild1DMipmaps(GL_TEXTURE_1D,internal_format,img->GetWidth(),format, GL_FLOAT, img->GetData());
			}
			GL_END
			return;
		}

		FloatImage clone (*img);
		if (clone.GetChannels() != 4)
		{
			clone.SetChannels(4);
			clone.SetChannel(3,1.0f);
		}
		GLenum target = img->GetHeight()>1?GL_TEXTURE_2D:GL_TEXTURE_1D;


		glTexImage2D(target,0,GL_RGBA32F_ARB,clone.GetWidth(),clone.GetHeight(),0,GL_RGBA,GL_FLOAT,clone.data());
		if (use_mipmap)
		{
			UINT32 layer(1);
			while (true)
			{
				bool	x = clone.ScaleXHalf(),
						y = clone.ScaleYHalf();
				if (!x && !y)
				{
					GL_END
					return;
				}
				glTexImage2D(target,layer++,GL_RGBA32F_ARB,clone.GetWidth(), clone.GetHeight(),0,GL_RGBA,GL_FLOAT,clone.data());
			}
		}
		GL_END
	}


	void OpenGL::loadTexture(const Image&img, bool use_mipmap, GLuint type, BYTE face)
	{
		GL_BEGIN
		if (img.GetContentType() == Image::ColorMap)
		{
			if (glGenerateMipmap)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,0,img.GetChannels(),img.GetWidth(),img.GetHeight(),0,type,GL_UNSIGNED_BYTE,img.GetData());
			else
				gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,img.GetChannels(),img.GetWidth(),img.GetHeight(),type,GL_UNSIGNED_BYTE,img.GetData());
			GL_END
			return;
		}

		Image clone(img);
		if (clone.GetChannels() != 4)
		{
			clone.SetChannels(4);
			clone.SetChannel(3,255);
		}


		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,0,GL_RGBA,clone.GetWidth(),clone.GetHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,clone.GetData());
		if (use_mipmap)
		{
			UINT32 layer(1);
			while (true)
			{
				bool x = clone.ScaleXHalf(),
						y = clone.ScaleYHalf();
				if (!x && !y)
				{
					GL_END
					return;
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,layer++,GL_RGBA,clone.GetWidth(), clone.GetHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,clone.GetData());
			}
		}
		GL_END
	}


	
	void OpenGL::loadFloatTexture(const FloatImage&img, bool use_mipmap, GLuint format, GLuint internal_format, BYTE face)
	{
			cout << format << "/"<<internal_format<<endl;
		GL_BEGIN
		if (img.GetContentType() == Image::ColorMap)
		{
			if (glGenerateMipmap)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,0,internal_format,img.GetWidth(),img.GetHeight(),0,format,GL_FLOAT,img.GetData());
			else
				gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,internal_format,img.GetWidth(),img.GetHeight(),format,GL_FLOAT,img.GetData());
			GL_END
			return;
		}

		FloatImage clone(img);
		if (clone.GetChannels() != 4)
		{
			clone.SetChannels(4);
			clone.SetChannel(3,1.0f);
		}


		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,0,GL_RGBA32F_ARB,clone.GetWidth(),clone.GetHeight(),0,GL_RGBA,GL_FLOAT,clone.GetData());
		if (use_mipmap)
		{
			UINT32 layer(1);
			while (true)
			{
				bool x = clone.ScaleXHalf(),
						y = clone.ScaleYHalf();
				if (!x && !y)
				{
					GL_END
					return;
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,layer++,GL_RGBA32F_ARB,clone.GetWidth(), clone.GetHeight(),0,GL_RGBA,GL_FLOAT,clone.GetData());
			}
		}
		GL_END
	}
	
	
	#endif
	
	bool OpenGL::queryBegin(bool depthTest)
	{
		GL_BEGIN
		state.in_query = glBeginQuery!=NULL;
		if (!state.in_query)
		{
			GL_END
			return false;
		}

		glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glDisable(GL_BLEND);
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		glDepthMask(GL_FALSE);
		if (depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		GL_END
		return true;
	}

	void OpenGL::queryEnd()
	{
		if (!state.in_query)
			return;
		glPopAttrib();
		state.in_query = false;
	}



	void OpenGL::castQuery(const Query&query)
	{
		GL_BEGIN
		if (!state.in_query)
			FATAL__("trying to cast query outside query-mode");
		glBeginQuery(GL_SAMPLES_PASSED_ARB,query.handle);
			glCallList(query.geometry_handle);
		glEndQuery(GL_SAMPLES_PASSED_ARB);
		GL_END
	}

	void OpenGL::castPointQuery(const Query&query, const M::TVec3<>&point)
	{
		GL_BEGIN
		if (!state.in_query)
			FATAL__("trying to cast query outside query-mode");
		glBeginQuery(GL_SAMPLES_PASSED_ARB,query.handle);
			glBegin(GL_POINTS);
				glVertex3fv(point.v);
			glEnd();
		glEndQuery(GL_SAMPLES_PASSED_ARB);
		GL_END
	}




	String OpenGL::renderState()
	{
		return glExtensions.QueryRenderState();
	}

	/*static*/			OpenGL::GLBinding			OpenGL::getCurrentContext()
	{
		GLBinding result;
		#if SYSTEM_VARIANCE==LINUX
			result.wnd = glXGetCurrentDrawable();
			result.gl_context = glXGetCurrentContext();
			result.display = glXGetCurrentDisplay();
		#elif SYSTEM==WINDOWS
			result.device_context = wglGetCurrentDC();
			result.gl_context = wglGetCurrentContext();
		#endif
		return result;
	}
	/*static*/			bool			OpenGL::hasCurrentContext()
	{
		#if SYSTEM_VARIANCE==LINUX
			GLContext gl_context = glXGetCurrentContext();
		#elif SYSTEM==WINDOWS
			GLContext gl_context = wglGetCurrentContext();
		#endif
		return gl_context != 0;
	}
	/*static*/			void						OpenGL::setCurrentContext(const GLBinding&current)
	{
		#if SYSTEM_VARIANCE==LINUX
			ASSERT__(!glXMakeCurrent(current.display,current.wnd,current.gl_context))
		#elif SYSTEM==WINDOWS
			ASSERT__(wglMakeCurrent(current.device_context,current.gl_context));
		#endif
	}

	void OpenGL::adoptCurrentContext()
	{
		#if SYSTEM_VARIANCE==LINUX
			wnd = glXGetCurrentDrawable();
			gl_context = glXGetCurrentContext();
			display = glXGetCurrentDisplay();
			glExtensions.Initialize(display,DefaultScreen(display));
		#elif SYSTEM==WINDOWS
			device_context = wglGetCurrentDC();
			gl_context = wglGetCurrentContext();
			glExtensions.Initialize(device_context);
		#endif
	}

	bool OpenGL::TargetFBO(const FBO&pobj)
	{
		if (!glBindFramebuffer || pobj.IsEmpty())
			return false;
		GL_BEGIN
			glBindFramebuffer(GL_FRAMEBUFFER, pobj.handle);
			glViewport(0,0,pobj.config.resolution.width,pobj.config.resolution.height);
		GL_END
		return true;
	}

	void OpenGL::TargetBackbuffer()
	{
		GL_BEGIN
			glExtensions.UnbindFrameBuffer();
		GL_END
	}


	void OpenGL::bindMaterial(const MaterialConfiguration&config, const Texture *const * list, const Shader&shader)
	{
		GL_BEGIN
			Shader::Instance*instance = shader.construct();
			ASSERT1__(shader.IsEmpty() || instance!=NULL,shader.Report());
			genericBindMaterial(config,list,instance != NULL);
			Shader::_Install(instance);
		GL_END
	}

	void OpenGL::bindMaterial(const MaterialConfiguration&config, const Texture::Reference*list, const Shader&shader)
	{
		GL_BEGIN
			Shader::Instance*instance = shader.construct();
			ASSERT1__(shader.IsEmpty() || instance!=NULL,shader.Report());

			genericBindMaterial(config,list,instance != NULL);
			Shader::_Install(instance);
		GL_END
	}

	void OpenGL::bindMaterial(const MaterialConfiguration&config, const Texture *const * list, const Shader::Instance*shader)
	{
		GL_BEGIN
			genericBindMaterial(config,list,shader->IsLoaded());
			Shader::_Install(shader);
			
		GL_END
	}
	void OpenGL::bindMaterial(const MaterialConfiguration&config, const Texture::Reference*list, const Shader::Instance*shader)
	{
		GL_BEGIN
			genericBindMaterial(config,list,shader->IsLoaded());
			Shader::_Install(shader);
		GL_END
	}


	void OpenGL::bindMaterialIgnoreShader(const MaterialConfiguration&config, const Texture *const * list)
	{
		GL_BEGIN
			genericBindMaterial(config,list,false);
		GL_END
	}

	void OpenGL::bindMaterialIgnoreShader(const MaterialConfiguration&config, const Texture::Reference*list)
	{
		GL_BEGIN
			genericBindMaterial(config,list,false);
		GL_END
	}





	void OpenGL::unbindAll()
	{
		GL_BEGIN

		Shader::Uninstall();

		state.render_setup.material = NULL;
		state.indices_bound = false;
		
		if (glBindBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}

		if (glActiveTexture)
		{
			for (index_t i = 0; i < state.render_setup.bound_texture_layers; i++)
			{
				glActiveTexture(GL_TEXTURE0+static_cast<GLenum>(i));
				glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
				glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
				glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
				glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
				if (state.render_setup.matrix_type[i] != SystemType::Identity)
				{
					glMatrixMode(GL_TEXTURE);
						glLoadIdentity();
					glMatrixMode(GL_MODELVIEW);
					state.render_setup.matrix_type[i] = SystemType::Identity;
				}
			}

			for (index_t i = 0; i < state.render_setup.bound_texcoord_layers; i++)
			{
				glActiveTexture(GL_TEXTURE0+static_cast<GLenum>(i));
				glClientActiveTexture(GL_TEXTURE0+static_cast<GLenum>(i));
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_R);
			}

		}
		else
		{
			glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
			glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
		}
		glDisable(GL_ALPHA_TEST);
		//glEnable(GL_BLEND);
		state.render_setup.bound_texture_layers = 0;
		state.render_setup.bound_texcoord_layers = 0;
		if (glActiveTexture)
		{
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
		}
		glMaterial(GL_FRONT_AND_BACK,GL_AMBIENT,CGS::ambient_default.v);
		glMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE,CGS::diffuse_default.v);
		glMaterial(GL_FRONT_AND_BACK,GL_SPECULAR,CGS::specular_default.v);
		glMaterial(GL_FRONT_AND_BACK,GL_EMISSION,CGS::emission_default.v);
		glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,1+(int)(CGS::shininess_default*127));

		GL_END
		//unbindPBufferObject();
	}
		
		OpenGL::OpenGL():verbose(false),active_scene_index(0),scene_index_counter(0),lighting_enabled(false)
		{
		    active_scene = scenes.define(scene_index_counter++);
			#if SYSTEM_VARIANCE==LINUX
				visual = NULL;
			#endif
		}

		void	 OpenGL::initDefaultExtensions()
		{
			GL_BEGIN
		    glExtensions.Init(EXT_WIN_CONTROL_BIT
								|EXT_MULTITEXTURE_BIT
								|EXT_OCCLUSION_QUERY_BIT
								|EXT_VERTEX_BUFFER_OBJECT_BIT
								|EXT_VERTEX_PROGRAM_BIT
								|EXT_SHADER_OBJECTS_BIT
								|EXT_FRAME_BUFFER_OBJECT_BIT
								|EXT_BLEND_FUNC_SEPARATE_BIT);
			//state.dot3_available = glExtensions.available("GL_ARB_texture_env_dot3");
			state.render_setup.enabled_light_field.SetSize(glExtensions.maxLights);

			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glPixelStorei(GL_PACK_ALIGNMENT,1);

			GL_END
		}



	#if SYSTEM==WINDOWS


		HWND	OpenGL::createWindow(const String&class_name, const String&window_name,int x, int y, unsigned width, unsigned height, bool hide_border, TVisualConfig&config)
		{


			while (true)
		    {
		        bool retry = false;


	            DWORD style = WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
				if (!hide_border)
					style |=   WS_OVERLAPPEDWINDOW  | WS_SIZEBOX;

	            window = CreateWindowExW(0L,
	                StringEncoding::UTF8::ToUTF16(class_name).c_str(),StringEncoding::UTF8::ToUTF16(window_name).c_str(),
	                style,
	                x,y, //Position
	                width,height, //Dimensionen
	                HWND_DESKTOP,
	                NULL,
	                GetInstance(), //Application
	                NULL);

	            if (!window)
	            {
	                error_code = ERR_GL_WINDOW_CREATION_FAILED;
	                return NULL;
	            }
	            ShowWindow(window, SW_SHOW);
	            UpdateWindow(window);

	            SetForegroundWindow(window);
	            SetFocus(window);


	            //RECT Screen;
	            PIXELFORMATDESCRIPTOR pfd=
	            {
	                sizeof(PIXELFORMATDESCRIPTOR), //own size for internal copy-functions i presume
	                1,  //version
	                PFD_DRAW_TO_WINDOW |        //draw to window rather than to bitmap (have to look up later)
	                PFD_SUPPORT_OPENGL |        //use openGL (could use "PFD_SUPPORT_DIRECTDRAW" additional maybe?)
					PFD_SUPPORT_COMPOSITION |	//aero
	                PFD_DOUBLEBUFFER,           //swap-buffer-system
	                PFD_TYPE_RGBA,              //color-type supporting alpha-channel
	                config.color_buffer_bits,   //by default use 32 bits here
	                0,0,0,0,0,0,
	                config.alpha_buffer_bits,   //by default use 8 bits here
	                0,
	                config.accumulation_buffer_bits,    //should be zero by default
	                0,0,0,0,                            //ignored bits of accumulation_buffer
	                config.depth_buffer_bits,           //depth-buffer-bits 16 by default
	                config.stencil_buffer_bits,         //bits for stencil-buffer to use
	                config.auxiliary_buffer_bits,       //auxiliary_buffer_bits (whatever that buffer can be used for)
	                0/*PFD_MAIN_PLANE*/,                     //draw to first layer only (would need to enable PFD_SWAP_LAYER_BUFFERS to use multible layers)
	                0,                                  //reserved (for whatever)
	                0,0,0                               //"Layer-Masks ignored" - maybe this is to reduce the ammount of colors displayed... would UpdateProjection sense somehow - each bit ignored would half the number
	            };
	            GLint PixelFormat;
	            device_context = GetDC(window);
	            if (!device_context)
	            {
	                DestroyWindow(window);
					window = NULL;
	                error_code = ERR_CONTEXT_CREATION_FAILED;
	                return NULL;
	            }
				
				
				

	            if (!wglChoosePixelFormat)
	            {
	                PixelFormat = ChoosePixelFormat(device_context, &pfd);
	                if (!PixelFormat)
	                {
	                    ReleaseDC(window,device_context);
	                    DestroyWindow(window);
						window = NULL;
	                    error_code = ERR_CONFIG_REJECTED;
	                    return NULL;
	                }
	                if (config.fsaa_samples)
	                    retry = true;
	            }
	            else
	            {
	                unsigned nof;
	                int attribs[] =
	                {
	                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	                    WGL_STEREO_ARB, GL_FALSE,
	                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	                    WGL_COLOR_BITS_ARB, config.color_buffer_bits,
	                    WGL_ALPHA_BITS_ARB, config.alpha_buffer_bits,
	                    WGL_DEPTH_BITS_ARB, config.depth_buffer_bits,
	                    WGL_STENCIL_BITS_ARB, config.stencil_buffer_bits,
	                    config.fsaa_samples?WGL_SAMPLES_ARB:0, config.fsaa_samples, //zero terminate here if no samples set
	                    WGL_SAMPLE_BUFFERS_ARB,config.fsaa_samples?GL_TRUE:GL_FALSE,
	                    0,0
	                };

	                /*
	                it seems some implementations of opengl do not support the WGL_SAMPLES_ARB / WGL_SAMPLE_BUFFERS_ARB - indices
	                and fail when assigned.
	                strange.
	                */
					if (!myPickPixelFormat(device_context,attribs,PixelFormat,config.fsaa_samples))
					{
						ReleaseDC(window,device_context);
						DestroyWindow(window);
						error_code = ERR_CONFIG_REJECTED;
						window = NULL;
						return NULL;
					}
	                
	                DescribePixelFormat(device_context,PixelFormat,sizeof(pfd),&pfd);
					ASSERT__(!!(pfd.dwFlags & PFD_SUPPORT_COMPOSITION));
					
	            }

	            if (!SetPixelFormat(device_context,PixelFormat,&pfd))
	            {
	                ReleaseDC(window,device_context);
	                DestroyWindow(window);
	                error_code = ERR_PFD_SET_FAILED;
					window = NULL;
	                return NULL;
	            }

	            gl_context = wglCreateContext(device_context);
	            if (!gl_context)
	            {
	                ReleaseDC(window,device_context);
	                DestroyWindow(window);
	                error_code = ERR_CONTEXT_CREATION_FAILED;
					window = NULL;
	                return NULL;
	            }


	            if (!wglMakeCurrent(device_context,gl_context))
	            {
	                wglDeleteContext(gl_context);
	                ReleaseDC(window,device_context);
	                DestroyWindow(window);
	                error_code = ERR_CONTEXT_BINDING_FAILED;
					window = NULL;
	                return NULL;
	            }
	            //GetClientRect(window, &Screen);
	            glExtensions.Initialize(device_context);
	            glExtensions.Init(EXT_WIN_CONTROL_BIT|EXT_MULTITEXTURE_BIT|EXT_OCCLUSION_QUERY_BIT|EXT_VERTEX_BUFFER_OBJECT_BIT|EXT_FRAME_BUFFER_OBJECT_BIT);

	            if (retry && wglChoosePixelFormat != NULL)
				{
	                wglDeleteContext(gl_context);
	                ReleaseDC(window,device_context);
	                DestroyWindow(window);
	                continue;
				}

				GLBinding	binding;
				binding.device_context = device_context;
				binding.gl_context = gl_context;
				created_contexts << binding;

	            if (wglSwapInterval)
	                wglSwapInterval(config.vertical_sync);

				break;
			}
			glViewport(0,0,width,height);

		   // pixelAspect = window_context.pixelAspectf(width,height);
			initDefaultExtensions();
		    initGL();
			return window;
		}








		bool OpenGL::CreateContext(HWND hWnd, TVisualConfig&config, const Resolution&)
		{
			GL_BEGIN
			error_code = ERR_NO_ERROR;
			RECT Screen;
			PIXELFORMATDESCRIPTOR pfd=
			{
				sizeof(PIXELFORMATDESCRIPTOR), //own size for internal copy-functions i presume
				1,	//version
				PFD_DRAW_TO_WINDOW |		//draw to window rather than to bitmap (have to look up later)
				PFD_SUPPORT_OPENGL |		//use openGL (could use "PFD_SUPPORT_DIRECTDRAW" additional maybe?)
				PFD_SUPPORT_COMPOSITION |	//aero
				PFD_DOUBLEBUFFER,			//swap-buffer-system
				PFD_TYPE_RGBA,				//color-type supporting alpha-channel
				config.color_buffer_bits,	//by default use 32 bits here
				0,0,0,0,0,0,
				config.alpha_buffer_bits,	//by default use 8 bits here
				0,
				config.accumulation_buffer_bits,	//should be zero by default
				0,0,0,0,							//ignored bits of accumulation_buffer
				config.depth_buffer_bits,			//depth-buffer-bits 16 by default
				config.stencil_buffer_bits,			//bits for stencil-buffer to use
				config.auxiliary_buffer_bits,		//auxiliary_buffer_bits (whatever that buffer can be used for)
				0/*PFD_MAIN_PLANE*/,						//draw to first layer only (would need to enable PFD_SWAP_LAYER_BUFFERS to use multible layers)
				0,									//reserved (for whatever)
				0,0,0								//"Layer-Masks ignored" - maybe this is to reduce the ammount of colors displayed... would UpdateProjection sense somehow - each bit ignored would half the number
			};
			GLint PixelFormat;
			device_context = GetDC(hWnd);
			
			
			if (!wglChoosePixelFormat)
			{
				PixelFormat = ChoosePixelFormat(device_context, &pfd);
				if (!PixelFormat)
				{
					error_code = ERR_CONFIG_UNSUPPORTED;
					ReleaseDC(window,device_context);
					device_context = NULL;
					GL_END
					return false;
				}
				if (config.fsaa_samples)
					error_code = ERR_RETRY;
			}
			else
			{
				int attribs[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_STEREO_ARB, GL_FALSE,
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
					WGL_COLOR_BITS_ARB, config.color_buffer_bits,
					WGL_ALPHA_BITS_ARB, config.alpha_buffer_bits,
					WGL_DEPTH_BITS_ARB, config.depth_buffer_bits,
					WGL_STENCIL_BITS_ARB, config.stencil_buffer_bits,
					config.fsaa_samples?WGL_SAMPLES_ARB:0, config.fsaa_samples, //zero terminate here if no samples set
					WGL_SAMPLE_BUFFERS_ARB,config.fsaa_samples?GL_TRUE:GL_FALSE,
					0,0
				};

				if (!myPickPixelFormat(device_context,attribs,PixelFormat,config.fsaa_samples))
				{
					ReleaseDC(window,device_context);
					device_context = NULL;
					error_code = ERR_CONFIG_REJECTED;
					GL_END
					return false;
				}
				DescribePixelFormat(device_context,PixelFormat,sizeof(pfd),&pfd);
				//ASSERT__(!!(pfd.dwFlags & PFD_SUPPORT_COMPOSITION)); //let's not go nuts over this. shit happens. WindowsXP also still happens :P
			}
			if (!SetPixelFormat(device_context,PixelFormat,&pfd))
			{
				ReleaseDC(window,device_context);
				device_context = NULL;
				error_code = ERR_PFD_SET_FAILED;
				GL_END
				return false;
			}
			gl_context = wglCreateContext(device_context);
			if (!gl_context)
			{
				ReleaseDC(window,device_context);
				device_context = NULL;
				error_code = ERR_CONTEXT_CREATION_FAILED;
				GL_END
				return false;
			}
			if (!wglMakeCurrent(device_context,gl_context))
			{
				wglDeleteContext(gl_context);
				ReleaseDC(window,device_context);
				device_context = NULL;
				gl_context = NULL;
				error_code = ERR_CONTEXT_BINDING_FAILED;
				GL_END
				return false;
			}
			GetClientRect(hWnd, &Screen);
			window = hWnd;
			glExtensions.Initialize(device_context);
			initDefaultExtensions();
			if (wglSwapInterval)
				wglSwapInterval(config.vertical_sync);

			initGL();
			if (error_code != ERR_NO_ERROR)
			{
				wglDeleteContext(gl_context);
				ReleaseDC(window,device_context);
				device_context = NULL;
				gl_context = NULL;
			}
			else
			{
				GLBinding	binding;
				binding.device_context = device_context;
				binding.gl_context = gl_context;
				created_contexts << binding;
			}
			GL_END
			return error_code == ERR_NO_ERROR;;
		}
	#elif SYSTEM_VARIANCE==LINUX


		OpenGL::~OpenGL()
		{
			GL_BEGIN
			if (visual)
				XFree(visual);
			GL_END
		}

		bool	OpenGL::CreateContext(Display*connection, TVisualConfig&config, TWindowAttributes&out_attributes)
		{
			GL_BEGIN
			display = connection;
			error_code = ERR_NO_ERROR;
			int attribs[] = {
				GLX_RGBA,
				GLX_USE_GL,
				GLX_DOUBLEBUFFER,
				GLX_RED_SIZE,	config.color_buffer_bits/3,
				GLX_GREEN_SIZE, config.color_buffer_bits/3,
				GLX_BLUE_SIZE,	config.color_buffer_bits/3,
				GLX_ALPHA_SIZE, config.alpha_buffer_bits,
				GLX_DEPTH_SIZE, config.depth_buffer_bits,
				GLX_STENCIL_SIZE, config.stencil_buffer_bits,
				config.fsaa_samples?GLX_SAMPLES_ARB:None, config.fsaa_samples, //zero terminate here if no samples set
				GLX_SAMPLE_BUFFERS_ARB,config.fsaa_samples?GL_TRUE:GL_FALSE,
				None
			};
			if (visual)
				XFree(visual);
			visual = glXChooseVisual(connection,DefaultScreen(connection),attribs);
			if (!visual)
			{
				int backup[] =
				{
					GLX_RGBA,
					GLX_DOUBLEBUFFER,
					GLX_RED_SIZE,	8,
					GLX_GREEN_SIZE, 8,
					GLX_BLUE_SIZE,	8,
					GLX_ALPHA_SIZE, 8,
					GLX_DEPTH_SIZE, 24,
					GLX_STENCIL_SIZE, 4,
					None
				};


				visual = glXChooseVisual(connection,DefaultScreen(connection),backup);
				if (!visual)
				{
					error_code = ERR_CONFIG_REJECTED;
					GL_END
					return false;
				}
			}
			gl_context = glXCreateContext(connection,visual,NULL,True);
			if (!gl_context)
			{
				error_code = ERR_CONTEXT_CREATION_FAILED;
				XFree(visual);
				visual = NULL;
				GL_END
				return false;
			}
			if (!glXIsDirect(display,gl_context))
				ErrMessage("warning: rendering indirect!");

			glXSwapIntervalSGI(config.vertical_sync);
			out_attributes.visual = visual->visual;
			out_attributes.depth = visual->depth;
			out_attributes.screen_number = visual->screen;

			GLBinding	binding;
			binding.display = connection;
			binding.gl_context = gl_context;
			binding.wnd = DefaultScreen(connection);
			created_contexts << binding;

			GL_END
			return true;
		}

		bool OpenGL::BindContext(Window window)
		{
			GL_BEGIN
			if (!glXMakeCurrent(display,window,gl_context))
			{
				error_code = ERR_CONTEXT_BINDING_FAILED;
				GL_END
				return false;
			}
			wnd = window;
			glExtensions.Initialize(display,visual->screen);
			initDefaultExtensions();

			initGL();

			GL_END
			return true;
		}


	#endif


	void						OpenGL::LinkContextClone (context_t  c)
	{
		GL_BEGIN
		#if SYSTEM_VARIANCE==LINUX
			ASSERT__(glXMakeCurrent(display,wnd,c));
		#elif SYSTEM==WINDOWS
		    ASSERT__(wglMakeCurrent(device_context,c));
		#endif
		GL_END
	}

	OpenGL::context_t					OpenGL::CreateContextClone()
	{
		GL_BEGIN
		#if SYSTEM_VARIANCE==LINUX
			context_t result = glXCreateContext(display,visual,gl_context,True);
			if (!result)
			{
				FATAL__("Failed to create render context clone");
				GL_END
				return result;
			}
			if (!glXIsDirect(display,result))
				ErrMessage("warning: OpenGL context clone renders indirect!");
		#elif SYSTEM==WINDOWS
			context_t result = wglCreateContext(device_context);
			if (!result)
			{
				FATAL__("Failed to create render context clone");
				GL_END
				return result;
			}
	        wglShareLists(gl_context,result);
	        //wglShareLists(result,gl_context);
		#endif
		GL_END
		return result;
	}


	OpenGL::context_t		OpenGL::linkContextClone()
	{
		GL_BEGIN
		#if SYSTEM_VARIANCE==LINUX
			context_t result = glXCreateContext(display,visual,gl_context,True);
			if (!result)
			{
				FATAL__("Failed to create render context clone");
				GL_END
				return result;
			}
			if (!glXIsDirect(display,result))
				ErrMessage("warning: OpenGL context clone renders indirect!");
			ASSERT__(glXMakeCurrent(display,wnd,result));
		#elif SYSTEM==WINDOWS
			context_t result = wglCreateContext(device_context);
			if (!result)
			{
				FATAL__("Failed to create render context clone");
				GL_END
				return result;
			}
	        wglShareLists(gl_context,result);
	        //wglShareLists(result,gl_context);
	        ASSERT__(wglMakeCurrent(device_context,result));
		#endif
		GL_END
		return result;
	}


	void						OpenGL::unlinkAndDestroyContextClone(context_t context)
	{
		GL_BEGIN
		#if SYSTEM_VARIANCE==LINUX
			glXMakeCurrent(display,None,NULL);
			glXDestroyContext(display,context);
		#elif SYSTEM==WINDOWS
			wglMakeCurrent(device_context, NULL);
			wglDeleteContext(context);
		#endif
		GL_END
	}


	bool OpenGL::linkCallingThread()
	{
		GL_BEGIN
		#if SYSTEM_VARIANCE==LINUX
			bool result = glXMakeCurrent(display,wnd,gl_context);
		#elif SYSTEM==WINDOWS
			bool result = wglMakeCurrent(device_context,gl_context)!=0;
		#endif
		GL_END
		return result;
	}

	void OpenGL::DestroyContext()
	{
		GL_BEGIN
		glFinish();
		for (index_t i = 0; i < created_contexts.Count(); i++)
			if (created_contexts[i].gl_context == gl_context)
				created_contexts.Erase(i--);
		#if SYSTEM==WINDOWS
			wglMakeCurrent(device_context, NULL);
			wglDeleteContext(gl_context);
			ReleaseDC(window,device_context);
		#elif SYSTEM_VARIANCE==LINUX
			glXMakeCurrent(display,None,NULL);
			glXDestroyContext(display,gl_context);
			XFree(visual);
			visual = NULL;
		#endif
		GL_END
	}


	void OpenGL::initGL()
	{
		GL_BEGIN
		glEnable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
		if (glBlendFuncSeparate)
			glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glShadeModel(GL_SMOOTH);
		glClearStencil(0);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glMatrixMode(GL_MODELVIEW);
		GL_END
	//	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&gl_max_texture_size);
	}


	const char*OpenGL::GetName()
	{
		return "OpenGL";
	}


	void OpenGL::NextFrameNoClr()
	{
		GL_BEGIN
		glFinish();
		//glFlush();
		#if SYSTEM==WINDOWS
			SwapBuffers(device_context);
		#elif SYSTEM_VARIANCE==LINUX
			glXSwapBuffers(display,wnd);
		#endif
		GL_END
	}

	void OpenGL::NextFrame()
	{
		GL_BEGIN
		glFinish();
		//glFlush();
		#if SYSTEM==WINDOWS
			SwapBuffers(device_context);
		#elif SYSTEM_VARIANCE==LINUX
			glXSwapBuffers(display,wnd);
		#endif
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		GL_END
	}

	void OpenGL::SetViewport(const RECT&region, const Resolution&windowRes)
	{
		glViewport(region.left,region.bottom,region.right-region.left,region.top-region.bottom);
	}

	const char*OpenGL::GetErrorStr()
	{
		#define ecase(token)	case token: return #token;
		switch (error_code)
		{
			ecase(ERR_NO_ERROR)
			ecase(ERR_RETRY)
			ecase(ERR_GENERAL_FAULT)
			ecase(ERR_CONFIG_UNSUPPORTED)
			ecase(ERR_CONFIG_REJECTED)
			ecase(ERR_PFD_SET_FAILED)
			ecase(ERR_CONTEXT_CREATION_FAILED)
			ecase(ERR_CONTEXT_BINDING_FAILED)
			ecase(ERR_TABLE_ENTRY)
			ecase(ERR_BAD_IMAGE)
		}
		#undef ecase
		return "undefined gl-error";
	}

	int OpenGL::GetErrorCode()
	{
		return error_code;
	}
	
	bool	OpenGL::useTexture(const FBO&object, bool clamp, bool override_safety)
	{
		GL_BEGIN
		if (state.render_setup.bound_texture_layers && !override_safety)
			FATAL__("illegal operation");
		if (!object.config.numColorTargets || !object.config.colorTarget[0].textureHandle)
		{
			glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
			glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glDisable(GL_TEXTURE_CUBE_MAP);
			state.single_texture_bound = false;
			GL_END
			return false;
		}
		glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
		glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
		glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,object.config.colorTarget[0].textureHandle);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
		
		
		state.single_texture_bound = true;
		GL_END
		return true;
	}
	
	

	bool	OpenGL::useTexture(const Texture::Reference&object, bool clamp, bool override_safety)
	{
		GL_BEGIN
		if (state.render_setup.bound_texture_layers && !override_safety)
			FATAL__("illegal operation");
		if (!object.handle)
		{
			glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
			glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			state.single_texture_bound = false;
			GL_END
			return false;
		}
		if (object.isCube())
		{
			glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
			glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP,object.getHandle());
		}
		elif (object.isLinear())
		{
			glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			glEnable(GL_TEXTURE_1D);
			glBindTexture(GL_TEXTURE_1D,object.getHandle());
			glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
		}
		else
		{
			glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,object.getHandle());
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
		}

		state.single_texture_bound = true;
		GL_END
		return true;
	}


	bool	OpenGL::useTexture(const Texture&object, bool clamp, bool override_safety)
	{
		GL_BEGIN
		if (state.render_setup.bound_texture_layers && !override_safety)
			FATAL__("illegal operation");
		if (!object.handle)
		{
			glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
			glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
			glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			state.single_texture_bound = false;
			GL_END
			return false;
		}
		switch (object.GetDimension())
		{
			case	TextureDimension::Linear:
				glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
				glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
				glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
				glEnable(GL_TEXTURE_1D);
				glBindTexture(GL_TEXTURE_1D,object.handle);
				glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
			break;
			case	TextureDimension::Planar:
				glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
				glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
				glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,object.handle);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
			break;
			case	TextureDimension::Volume:
				glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
				glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
				glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
				glEnable(GL_TEXTURE_3D);
				glBindTexture(GL_TEXTURE_3D,object.handle);
				glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
				glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
				glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
			break;
			case	TextureDimension::Cube:
				glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
				glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
				glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
				glEnable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP,object.handle);
			break;
		}
		state.single_texture_bound = true;
		GL_END
		return true;
	}

	bool OpenGL::useTexture(const Texture*object, bool clamp, bool override_safety)
	{
		if (object)
			return useTexture(*object,clamp,override_safety);
		GL_BEGIN
		glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
		glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
		glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
		state.single_texture_bound = false;
		GL_END
		return false;
	}


	void OpenGL::segment(const tCVertex&v0, const tCVertex&v1)
	{
		glBegin(GL_LINES);
				glColor4fv(v0.color.v);
			glVertex4fv(v0.v);
				glColor4fv(v1.color.v);
			glVertex4fv(v1.v);
		glEnd();
	}

	void OpenGL::segment(const TVertex&v0, const TVertex&v1)
	{
		glBegin(GL_LINES);
			glVertex4fv(v0.v);
			glVertex4fv(v1.v);
		glEnd();
	}


	void OpenGL::face(const tTVertex&v0, const tTVertex&v1, const tTVertex&v2, const tTVertex&v3)
	{
		glBegin(GL_QUADS);
				glTexCoord2fv(v0.coord.v);
			glVertex4fv(v0.v);
				glTexCoord2fv(v1.coord.v);
			glVertex4fv(v1.v);
				glTexCoord2fv(v2.coord.v);
			glVertex4fv(v2.v);
				glTexCoord2fv(v3.coord.v);
			glVertex4fv(v3.v);
		glEnd();
	}


	void OpenGL::face(const tTVertex&v0, const tTVertex&v1, const tTVertex&v2)
	{
		glBegin(GL_TRIANGLES);
				glTexCoord2fv(v0.coord.v);
			glVertex4fv(v0.v);
				glTexCoord2fv(v1.coord.v);
			glVertex4fv(v1.v);
				glTexCoord2fv(v2.coord.v);
			glVertex4fv(v2.v);
		glEnd();
	}


	void OpenGL::face(const tCTVertex&v0, const tCTVertex&v1, const tCTVertex&v2, const tCTVertex&v3)
	{
		glBegin(GL_QUADS);
				glColor4fv(v0.color.v);
				glTexCoord2fv(v0.coord.v);
			glVertex4fv(v0.v);
				glColor4fv(v1.color.v);
				glTexCoord2fv(v1.coord.v);
			glVertex4fv(v1.v);
				glColor4fv(v2.color.v);
				glTexCoord2fv(v2.coord.v);
			glVertex4fv(v2.v);
				glColor4fv(v3.color.v);
				glTexCoord2fv(v3.coord.v);
			glVertex4fv(v3.v);
		glEnd();
	}


	void OpenGL::face(const tCTVertex&v0, const tCTVertex&v1, const tCTVertex&v2)
	{
		glBegin(GL_TRIANGLES);
				glColor4fv(v0.color.v);
				glTexCoord2fv(v0.coord.v);
			glVertex4fv(v0.v);
				glColor4fv(v1.color.v);
				glTexCoord2fv(v1.coord.v);
			glVertex4fv(v1.v);
				glColor4fv(v2.color.v);
				glTexCoord2fv(v2.coord.v);
			glVertex4fv(v2.v);
		glEnd();
	}

	void OpenGL::face(const tNTVertex&v0, const tNTVertex&v1, const tNTVertex&v2, const tNTVertex&v3)
	{
		glBegin(GL_QUADS);
				glNormal3fv(v0.normal.v);
				glTexCoord2fv(v0.coord.v);
			glVertex4fv(v0.v);
				glNormal3fv(v1.normal.v);
				glTexCoord2fv(v1.coord.v);
			glVertex4fv(v1.v);
				glNormal3fv(v2.normal.v);
				glTexCoord2fv(v2.coord.v);
			glVertex4fv(v2.v);
				glNormal3fv(v3.normal.v);
				glTexCoord2fv(v3.coord.v);
			glVertex4fv(v3.v);
		glEnd();
	}

	void OpenGL::face(const tNTVertex&v0, const tNTVertex&v1, const tNTVertex&v2)
	{
		glBegin(GL_TRIANGLES);
				glNormal3fv(v0.normal.v);
				glTexCoord2fv(v0.coord.v);
			glVertex4fv(v0.v);
				glNormal3fv(v1.normal.v);
				glTexCoord2fv(v1.coord.v);
			glVertex4fv(v1.v);
				glNormal3fv(v2.normal.v);
				glTexCoord2fv(v2.coord.v);
			glVertex4fv(v2.v);
		glEnd();
	}

	void OpenGL::face(const tNVertex&v0, const tNVertex&v1, const tNVertex&v2, const tNVertex&v3)
	{
		glBegin(GL_QUADS);
				glNormal3fv(v0.normal.v);
			glVertex4fv(v0.v);
				glNormal3fv(v1.normal.v);
			glVertex4fv(v1.v);
				glNormal3fv(v2.normal.v);
			glVertex4fv(v2.v);
				glNormal3fv(v3.normal.v);
			glVertex4fv(v3.v);
		glEnd();
	}

	void OpenGL::face(const tNVertex&v0, const tNVertex&v1, const tNVertex&v2)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLES);
				glNormal3fv(v0.normal.v);
			glVertex4fv(v0.v);
				glNormal3fv(v1.normal.v);
			glVertex4fv(v1.v);
				glNormal3fv(v2.normal.v);
			glVertex4fv(v2.v);
		glEnd();
	}

	void OpenGL::face(const tCVertex&v0, const tCVertex&v1, const tCVertex&v2, const tCVertex&v3)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
				glColor4fv(v0.color.v);
			glVertex4fv(v0.v);
				glColor4fv(v1.color.v);
			glVertex4fv(v1.v);
				glColor4fv(v2.color.v);
			glVertex4fv(v2.v);
				glColor4fv(v3.color.v);
			glVertex4fv(v3.v);
		glEnd();
	}

	void OpenGL::face(const tCVertex&v0, const tCVertex&v1, const tCVertex&v2)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLES);
				glColor4fv(v0.color.v);
			glVertex4fv(v0.v);
				glColor4fv(v1.color.v);
			glVertex4fv(v1.v);
				glColor4fv(v2.color.v);
			glVertex4fv(v2.v);
		glEnd();
	}

	void OpenGL::face(const TVertex&v0, const TVertex&v1, const TVertex&v2, const TVertex&v3)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glVertex4fv(v0.v);
			glVertex4fv(v1.v);
			glVertex4fv(v2.v);
			glVertex4fv(v3.v);
		glEnd();
	}

	void OpenGL::face(const TVertex&v0, const TVertex&v1, const TVertex&v2)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLES);
			glVertex4fv(v0.v);
			glVertex4fv(v1.v);
			glVertex4fv(v2.v);
		glEnd();
	}

	String OpenGL::sampleTexture(unsigned index)
	{
		if (state.render_setup.texture_type[index] == TextureDimension::None)
			FATAL__("trying to sample disabled texture");

		switch (state.render_setup.texture_type[index])
		{
			case TextureDimension::Linear:
				return "texture1D(sampler"+String(index)+",(gl_TexCoord["+String(index)+"]).x)";
			case TextureDimension::Planar:
				return "texture2D(sampler"+String(index)+",(gl_TexCoord["+String(index)+"]).xy)";
			case TextureDimension::Volume:
				return "texture3D(sampler"+String(index)+",(gl_TexCoord["+String(index)+"]).xyz)";
			case TextureDimension::Cube:
				switch (state.render_setup.texcoord_type[index])
				{
					case TextureMapping::Default:
						return "textureCube(sampler"+String(index)+",gl_TexCoord["+String(index)+"].xyz)";
					case TextureMapping::Normal:
						return "textureCube(sampler"+String(index)+",normal)";
					case TextureMapping::Reflection:
						return "textureCube(sampler"+String(index)+",reflected)";
					case TextureMapping::Refraction:
						return "textureCube(sampler"+String(index)+",refracted)";
				}
		}
		FATAL__("unsupported sample request");
		return "";
	}

	void OpenGL::renderExplicit(GLuint type, index_t vertex_offset, GLsizei vertex_count)
	{
			//ShowMessage(vertex_count);
			glThrowError();
		GL_BEGIN
			if (state.indices_bound)
				glDrawElements(type, vertex_count, GL_UNSIGNED_INT, state.index_reference + vertex_offset);
			else
				glDrawArrays(type,static_cast<GLint>(vertex_offset),static_cast<GLsizei>(vertex_count));

		    //else
		      //  glDrawElements(type, vertex_count, state.index_type, (void*)((unsigned long)state.index_reference + vertex_offset*state.index_size));
		GL_END
	}


	void OpenGL::render(const SimpleGeometry&structure)
	{
		GL_BEGIN
		if (state.render_setup.bound_texture_layers || state.render_setup.bound_texcoord_layers)
			FATAL__("misplaced render-call");
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
		if (glBlendFuncSeparate)
			glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		//	ShowMessage("triangles: "+String(structure.len[ERR_TRIANGLES])+"; quads: "+String(structure.len[ERR_QUADS])+"; lines: "+String(structure.len[ERR_LINES]));

		if (structure.field[SimpleGeometry::Triangles].IsNotEmpty())
		{
			glVertexPointer(3,GL_FLOAT,sizeof(tCVertex),structure.field[SimpleGeometry::Triangles].First().v);
			glColorPointer(4,GL_FLOAT,sizeof(tCVertex),structure.field[SimpleGeometry::Triangles].First().color.v);
			glDrawArrays(GL_TRIANGLES,0,GLuint(structure.field[SimpleGeometry::Triangles].GetLength()));
		}
		if (structure.field[SimpleGeometry::Quads].IsNotEmpty())
		{
			glVertexPointer(3,GL_FLOAT,sizeof(tCVertex),structure.field[SimpleGeometry::Quads].First().v);
			glColorPointer(4,GL_FLOAT,sizeof(tCVertex),structure.field[SimpleGeometry::Quads].First().color.v);
			glDrawArrays(GL_QUADS,0,GLuint(structure.field[SimpleGeometry::Quads].GetLength()));
		}
		if (structure.field[SimpleGeometry::Lines].IsNotEmpty())
		{
			glVertexPointer(3,GL_FLOAT,sizeof(tCVertex),structure.field[SimpleGeometry::Lines].First().v);
			glColorPointer(4,GL_FLOAT,sizeof(tCVertex),structure.field[SimpleGeometry::Lines].First().color.v);
			glDrawArrays(GL_LINES,0,GLuint(structure.field[SimpleGeometry::Lines].GetLength()));
		}
		GL_END
	}



	void OpenGL::bindIndices(const IBO&iobj)
	{
		GL_BEGIN
		if (!iobj.IsEmpty())
		{
			glGetError();
			if (glBindBuffer)
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,iobj.getDeviceHandle());
			glThrowError();
			DBG_ASSERT__(glIsBuffer(iobj.getDeviceHandle()));
			//ShowMessage(iobj.countPrimitives());
			state.index_reference = iobj.getHostData();
			state.indices_bound = true;
		}
		else
		{
			unbindIndices();
		}
		
		GL_END
	}

	void OpenGL::unbindIndices()
	{
		if (glBindBuffer)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		state.indices_bound = false;
	}





	void OpenGL::Bind(const VertexBinding&binding, const float*field)
	{
		GL_BEGIN
	    const GLuint stride = binding.floats_per_vertex*sizeof(GLfloat);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(binding.vertex.length,GL_FLOAT,stride,field+binding.vertex.offset);
		if (binding.normal.length)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT,stride,field+binding.normal.offset);
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);
		glThrowError();
		if (binding.color.length)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(binding.color.length,GL_FLOAT,stride,field+binding.color.offset);
		}
		else
			glDisableClientState(GL_COLOR_ARRAY);

		glThrowError();
		count_t texcoords = std::min((count_t)glExtensions.maxTexcoordLayers,binding.texcoords.Count());

		for (index_t i = texcoords; i < state.render_setup.bound_texcoord_layers; i++)
		{
			glClientActiveTexture(GL_TEXTURE0+GLuint(i));
			glActiveTexture(GL_TEXTURE0+GLuint(i));
			
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		for (index_t i = 0; i < texcoords; i++)
		{
			glClientActiveTexture(GL_TEXTURE0+GLuint(i));
			glActiveTexture(GL_TEXTURE0+GLuint(i));

			if (binding.texcoords[i].length)
			{
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_R);

				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(binding.texcoords[i].length,GL_FLOAT,stride,field+binding.texcoords[i].offset);
			}
			else
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);

				if (binding.texcoords[i].generate_reflection_coords)
				{
					glEnable(GL_TEXTURE_GEN_S);
					glEnable(GL_TEXTURE_GEN_T);
					glEnable(GL_TEXTURE_GEN_R);
					glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
					glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
					glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
				}
				else
				{
					glDisable(GL_TEXTURE_GEN_S);
					glDisable(GL_TEXTURE_GEN_T);
					glDisable(GL_TEXTURE_GEN_R);
				}
			}
		}

		if (binding.tangent.IsSet() && texcoords < (count_t)glExtensions.maxTexcoordLayers)
		{
			index_t i = texcoords++;
			glClientActiveTexture(GL_TEXTURE0+GLuint(i));
			glActiveTexture(GL_TEXTURE0+GLuint(i));

			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);

			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(binding.tangent.length,GL_FLOAT,stride,field+binding.tangent.offset);
		}

		state.render_setup.bound_texcoord_layers = texcoords;

		glClientActiveTexture(GL_TEXTURE0);
		glActiveTexture(GL_TEXTURE0);
		glThrowError();
		GL_END

	}


	void OpenGL::bindVertices(const VBO::Reference&vobj, const VertexBinding&binding)
	{
		glBindBuffer(GL_ARRAY_BUFFER_ARB,vobj.GetHandle());
		DBG_ASSERT__(glIsBuffer(vobj.GetHandle()));
		glThrowError();
		Bind(binding,NULL);

	}


	void OpenGL::bindVertices(const VBO&vobj, const VertexBinding&binding)
	{
		GL_BEGIN
		const GLfloat*field = vobj.getHostData();
		if (glBindBuffer)
			glBindBuffer(GL_ARRAY_BUFFER_ARB,vobj.getDeviceHandle());
		DBG_ASSERT__(glIsBuffer(vobj.getDeviceHandle()));
		glThrowError();
		//ShowMessage(vobj.countPrimitives());

		Bind(binding,field);
		GL_END
	}

	void					OpenGL::SetCameraMatrices(const M::TMatrix4<>&view, const M::TMatrix4<>&projection, const M::TMatrix4<>&viewInvert)
	{
		GL_BEGIN
	    glLoadMatrix(view.v);
	    glMatrixMode(GL_PROJECTION);
	    glLoadMatrix(projection.v);
	    glMatrixMode(GL_MODELVIEW);
		onModelviewChange();
		GL_END

	}


	void					OpenGL::RestoreRendererState()
	{
		PopStateStackToCurrent();
		SetBlendMode(current.blendMode);
		SetRasterizer(current.fill,current.cull);
		SetDepthTest(current.depthTest,current.depthWrite);
	}

	void					OpenGL::SetDepthTest(bool depthTest, bool depthWrite)
	{
		if (depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		glDepthMask(depthWrite);
	}

	void					OpenGL::SetRasterizer(bool fill, bool cull)
	{
		if (fill)
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		else
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		if (cull)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	void					OpenGL::SetBlendMode(BlendMode mode)
	{
		switch (mode)
		{
			case BlendMode::AlphaBlend:
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
			break;
			case BlendMode::Add:
				glBlendFunc(GL_ONE,GL_ONE);
				glEnable(GL_BLEND);
			break;
			case BlendMode::FlareBlend:
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
				glEnable(GL_BLEND);
			break;
			case BlendMode::None:
				glBlendFunc(GL_ONE, GL_ZERO);
				glDisable(GL_BLEND);
			break;
		}
	}


	 
	void						OpenGL::RenderLights(const M::TVec3<>&cameraLocation)
	{
		Array<PLight>	lights;
		getSceneLights(lights, true);
		if (!lights_defined)
		{
			lights_defined = true;
			
		/*	spot.select(SimpleGeometry::Lines);
			spot.color(1,1,1);
			spot.vertex(0,0,0);
			spot.color(0,0,1);
			spot.vertex(0,1,1);
			spot.color(1,1,1);
			spot.vertex(0,0,0);
			spot.color(0,0,1);
			spot.vertex(0,-1,1);
			spot.color(1,1,1);
			spot.vertex(0,0,0);
			spot.color(0,0,1);
			spot.vertex(1,0,1);
			spot.color(1,1,1);
			spot.vertex(0,0,0);
			spot.color(0,0,1);
			spot.vertex(-1,0,1);
			for (unsigned i = 0; i < 40; i++)
			{
				float	f0 = (float)i/40.0f*2*M_PI,
						f1 = (float)(i+1)/40.0f*2*M_PI;
				spot.vertex(cos(f0),sin(f0),1);
				spot.vertex(cos(f1),sin(f1),1);
			}*/
			
			spot.select(SimpleGeometry::Triangles);
			for (unsigned i = 0; i < 40; i++)
			{
				float	f0 = (float)i/40.0f*2*M_PI,
						f1 = (float)(i+1)/40.0f*2*M_PI;
				spot.color(0,0,1,0);
				spot.vertex(cos(f0),sin(f0),1);
				spot.vertex(cos(f1),sin(f1),1);
				spot.color(1,1,1);
				spot.vertex(0,0,0);
			}
			
			spot.seal();
			
			
			omni.select(SimpleGeometry::Lines);
			omni.color(1,1,1);
			omni.vertex(2,0,0);
			omni.vertex(-2,0,0);
			omni.vertex(0,2,0);
			omni.vertex(0,-2,0);
			omni.vertex(0,0,2);
			omni.vertex(0,0,-2);
			omni.seal();
			
			

			direct.select(SimpleGeometry::Quads);
			for (unsigned i = 0; i < 40; i++)
			{
				float	f0 = (float)i/40.0f*2*M_PI,
						f1 = (float)(i+1)/40.0f*2*M_PI;
				direct.color(0,0,1,0);
				direct.vertex(cos(f1)*0.5,sin(f1)*0.5,0);
				direct.vertex(cos(f0)*0.5,sin(f0)*0.5,0);
				direct.color(1,1,1);
				direct.vertex(cos(f0)*0.5,sin(f0)*0.5,1);
				direct.vertex(cos(f1)*0.5,sin(f1)*0.5,1);
			}
			
			direct.select(SimpleGeometry::Triangles);
			direct.color(1,1,1);
			for (unsigned i = 0; i < 40; i++)
			{
				float	f0 = (float)i/40.0f*2*M_PI,
						f1 = (float)(i+1)/40.0f*2*M_PI;
				direct.vertex(cos(f1)*0.5,sin(f1)*0.5,1);
				direct.vertex(cos(f0)*0.5,sin(f0)*0.5,1);
				direct.vertex(0,0,1);
			}
			
			direct.seal();
			
			
		}
		for (index_t i = 0; i < lights.Count(); i++)
		{
		
			switch (lights[i]->GetType())
			{
				case Light::Spot:
				{
					float scale = M::Vec::distance(cameraLocation,lights[i]->GetPosition())/5;
					M::TMatrix4<> system;
					float angle = pow(0.94,lights[i]->GetSpotExponent());
					/*if (angle*45 > lights[i]->getSpotCutoff())
						angle = lights[i]->getSpotCutoff()/45;*/
					float z_scale = 1.0/M::clamped(angle,0.1,1);
					M::Mat::makeAxisSystem(lights[i]->GetPosition(),lights[i]->GetSpotDirection(),2,system);
					M::Vec::mult(system.x.xyz,scale/z_scale);
					M::Vec::mult(system.y.xyz,scale/z_scale);
					M::Vec::mult(system.z.xyz,scale*z_scale);
					
					enterSubSystem(system);
						render(spot);
					
					exitSubSystem();
				
				}
				break;
				case Light::Omni:
				{
					float scale = M::Vec::distance(cameraLocation,lights[i]->GetPosition())/10;
					M::TMatrix4<> system;
					M::Mat::Eye(system);
					M::Vec::copy(lights[i]->GetPosition(),system.w.xyz);
					M::Vec::mult(system.x.xyz,scale);
					M::Vec::mult(system.y.xyz,scale);
					M::Vec::mult(system.z.xyz,scale);
					enterSubSystem(system);
						render(omni);
					exitSubSystem();
				}
				break;
				case Light::Direct:
				{
					float scale = M::Vec::length(cameraLocation)/4;
					M::TMatrix4<> system;
					M::TVec3<>	offset;
					M::Vec::mult(lights[i]->GetPosition(),scale,offset);
					M::Mat::makeAxisSystem(offset,lights[i]->GetPosition(),2,system);
					M::Vec::mult(system.x.xyz,scale);
					M::Vec::mult(system.y.xyz,scale);
					M::Vec::mult(system.z.xyz,scale*2);
					enterSubSystem(system);
						render(direct);
					exitSubSystem();
				}
				break;
			}
		
		
		}
		
		
		
	}



	bool	GL::Buffer::isValid()	const
	{
		return !handle || (glIsBuffer && glIsBuffer(handle));
	}

	bool	GL::Texture::isValid()	const
	{
		return !handle || glIsTexture(handle);
	}

	bool	GL::Query::isValid()	const
	{
		return ((!handle || glIsQuery(handle))&&(!geometry_handle || glIsList(geometry_handle)));
	}

	bool	GL::FBO::isValid()	const
	{
		return !handle || (glIsFramebuffer && glIsFramebuffer(handle));
	}




	#if SYSTEM==WINDOWS
	HDC OpenGL::getDC()
	{
		return device_context;
	}

	HGLRC	OpenGL::getGLContext()
	{
		return gl_context;
	}
	#elif SYSTEM_VARIANCE==LINUX
	XVisualInfo* OpenGL::getVisual()
	{
		return visual;
	}
	#endif
}

