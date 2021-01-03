#ifndef engine_renderer_openglTplH
#define engine_renderer_openglTplH




namespace Engine
{
	template <typename GLType>
		/*static*/	void		GL::Texture::formatAt(BYTE channels, PixelType type, bool compress, GLenum&internal_format, GLenum&import_format)
		{
			switch (channels)
			{
				case 1:
					if (type == PixelType::StrictColor)
					{
						internal_format = compress ? GL_COMPRESSED_RED: GLType::r_type_constant;
						import_format = internal_format == GL_DEPTH_COMPONENT24 ? GL_DEPTH_COMPONENT : GL_RED;
					}
					else
					{
						internal_format = compress ? GL_COMPRESSED_ALPHA : GLType::alpha_type_constant;
						import_format = internal_format == GL_DEPTH_COMPONENT24 ? GL_DEPTH_COMPONENT : GL_ALPHA;
					}
				break;
				case 2:
					internal_format = type == PixelType::StrictColor ? (compress ? GL_COMPRESSED_RG : GLType::rg_type_constant):(compress ? GL_COMPRESSED_LUMINANCE_ALPHA : GLType::luminance_alpha_type_constant);
					import_format = PixelType::StrictColor ? GL_RG : GL_LUMINANCE_ALPHA;
				break;
				case 3:
					internal_format = compress ? GL_COMPRESSED_RGB : GLType::rgb_type_constant;
					if (type == PixelType::BGRA)
						import_format = GL_BGR;
					else
						import_format = GL_RGB;
				break;
				case 4:
					internal_format = compress?GL_COMPRESSED_RGBA : GLType::rgba_type_constant;
					if (type == PixelType::BGRA)
						import_format = GL_BGRA;
					else
						import_format = GL_RGBA;
				break;
				default:
					FATAL__("Unsupported channel count: "+String(channels));
			}

		}


	template <typename data_t>
		void	GL::Texture::Reference::update(const data_t*data,TextureFilter filter, float anisotropy)
		{
			if (readonly)
				throw Except::Renderer::TextureTransfer::GeneralFault(Except::globalString("Update not possible: This texture is read-only"));

			OpenGL::ContextLock	context_lock;

			glGetError();
			GLenum target;
			switch (texture_dimension)
			{
				case TextureDimension::Linear:
					target = GL_TEXTURE_1D;
				break;
				case TextureDimension::Planar:
					target = GL_TEXTURE_2D;
				break;
				case TextureDimension::Volume:
					target = GL_TEXTURE_3D;
				break;
				default:
					throw Except::Renderer::TextureTransfer::GeneralFault(Except::globalString("Update not possible: Incompatible texture dimension"));
				break;
			}
			
			if (!handle)
				throw Except::Renderer::TextureTransfer::GeneralFault(Except::globalString("Update not possible: This texture is empty"));


			glPushAttrib(GL_TEXTURE_BIT);
			glBindTexture(target,handle);

			//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			bool mipmap = configureFilter(target, filter, anisotropy);

			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				glPopAttrib();
				throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL reports general fault while trying to configure texture handle ("+String(glGetErrorName(error))+")");
				return;
			}
			
			//cout << "load "<<width<<"*"<<height<<"*"<<(int)channels<<endl;
			glGetError();
			GLenum internal_format,import_format;


			formatAt<GLType<data_t> >(texture_channels,texture_type,false, internal_format, import_format);
			if (mipmap && (!glGenerateMipmap)) //glGenerateMipmap apparently doesn't like compression
			{
				if (texture_dimension == TextureDimension::Linear)
					gluBuild1DMipmaps(target,internal_format,texture_width,import_format,GLType<data_t>::constant,data);
				else
					gluBuild2DMipmaps(target,internal_format,texture_width,texture_height,import_format,GLType<data_t>::constant,data);
			}
			else
			{
				if (texture_dimension == TextureDimension::Linear)
					//glTexImage1D( target, 0, internal_format, texture_width, 0, import_format, GLType<data_t>::constant, data);
					glTexSubImage1D(target,0,0,texture_width,import_format, GLType<data_t>::constant, data);
				else
					//glTexImage2D( target, 0, internal_format, texture_width, texture_height, 0, import_format, GLType<data_t>::constant, data);
					glTexSubImage2D(target,0,0,0,texture_width,texture_height,import_format, GLType<data_t>::constant, data);
				GLenum error = glGetError();
				if (error != GL_NO_ERROR)
				{
					glPopAttrib();
					throw Except::Renderer::TextureTransfer::GeneralFault("First chance: OpenGL reports general fault while trying to load texture ("+String(glGetErrorName(error))+") using internal format 0x"+IntToHex((int)internal_format,4)+" and import format 0x"+IntToHex((int)import_format,4));
					return;
				}
				if (mipmap)
				{
					ASSERT_NOT_NULL__(glGenerateMipmap);	//this should be redundant
					glGenerateMipmap(target);
				}
			}
			glPopAttrib();
			error = glGetError();
			if (error != GL_NO_ERROR)
				throw Except::Renderer::TextureTransfer::GeneralFault("Second chance: OpenGL reports general fault while trying to load texture ("+String(glGetErrorName(error))+") using internal format 0x"+IntToHex((int)internal_format,4)+" and import format 0x"+IntToHex((int)import_format,4));
		}



	template <typename data_t>
		void	GL::Texture::loadPixels(const data_t*data, GLuint width_, GLuint height_, BYTE channels_, PixelType type, float anisotropy, bool clamp_texcoords, TextureFilter filter, bool compress)
		{
			if (!width_ || !height_ || !channels_ || channels_ > 4)
			{
				throw Except::Renderer::TextureTransfer::ParameterFault("Invalid data dimensions (width="+String(width_)+", height="+String(height_)+", channels="+String(channels_)+")");
				return;
			}
			texture_type = type;

			OpenGL::ContextLock	context_lock;

			glGetError();
			bool is_1D = height_ == 1;
			GLenum target = is_1D?GL_TEXTURE_1D:GL_TEXTURE_2D;


			glPushAttrib(GL_TEXTURE_BIT);
			if (!handle)
			{
				glGetError();
				glGenTextures(1,&handle);
				GLenum error = glGetError();
				if (handle == 0 || error != GL_NO_ERROR)
				{
					glPopAttrib();
					throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL refused to create texture handle ("+String(glGetErrorName(error))+")");
					return;	//in case throw is ignored
				}
			}
			glBindTexture(target,handle);

			glTexParameteri( target, GL_TEXTURE_WRAP_S, clamp_texcoords?GL_CLAMP_TO_EDGE:GL_REPEAT);
			if (!is_1D)
				glTexParameteri( target, GL_TEXTURE_WRAP_T, clamp_texcoords?GL_CLAMP_TO_EDGE:GL_REPEAT);
			//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			bool mipmap = configureFilter(target, filter, anisotropy);

			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				glPopAttrib();
				throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL reports general fault while trying to configure texture handle ("+String(glGetErrorName(error))+")");
				return;
			}
			
			texture_width = width_;
			texture_height = height_;
			texture_channels = channels_;
			texture_dimension = is_1D ? TextureDimension::Linear : TextureDimension::Planar;
			//cout << "load "<<width<<"*"<<height<<"*"<<(int)channels<<endl;
			glGetError();
			GLenum internal_format,import_format;


			formatAt<GLType<data_t> >(texture_channels,type,compress, internal_format, import_format);
			if (data && mipmap && (compress || !glGenerateMipmap)) //glGenerateMipmap apparently doesn't like compression
			{
				if (is_1D)
					gluBuild1DMipmaps(target,internal_format,texture_width,import_format,GLType<data_t>::constant,data);
				else
					gluBuild2DMipmaps(target,internal_format,texture_width,texture_height,import_format,GLType<data_t>::constant,data);
			}
			else
			{
				if (is_1D)
					glTexImage1D( target, 0, internal_format, texture_width, 0, import_format, GLType<data_t>::constant, data);
				else
					glTexImage2D( target, 0, internal_format, texture_width, texture_height, 0, import_format, GLType<data_t>::constant, data);
				GLenum error = glGetError();
				if (error != GL_NO_ERROR)
				{
					glPopAttrib();
					throw Except::Renderer::TextureTransfer::GeneralFault("First chance: OpenGL reports general fault while trying to load texture ("+String(glGetErrorName(error))+") using internal format 0x"+IntToHex((int)internal_format,4)+" and import format 0x"+IntToHex((int)import_format,4));
					return;
				}
				if (mipmap && data != NULL)
				{
					ASSERT_NOT_NULL__(glGenerateMipmap);	//this should be redundant
					glGenerateMipmap(target);
				}
			}
			glPopAttrib();
			error = glGetError();
			if (error != GL_NO_ERROR)
				throw Except::Renderer::TextureTransfer::GeneralFault("Second chance: OpenGL reports general fault while trying to load texture ("+String(glGetErrorName(error))+") using internal format 0x"+IntToHex((int)internal_format,4)+" and import format 0x"+IntToHex((int)import_format,4));
		}



	template <typename data_t>
		void			GL::Texture::loadCube(const data_t*data0, const data_t*data1, const data_t*data2, const data_t*data3, const data_t*data4, const data_t*data5, GLuint width_, GLuint height_, BYTE channels_, TextureFilter filter, bool compress)
		{
			if (!width_ || !height_ || !channels_ || channels_ > 4)
				throw Except::Renderer::TextureTransfer::ParameterFault("Invalid data dimensions (width="+String(width_)+", height="+String(height_)+", channels="+String(channels_)+")");
			OpenGL::ContextLock	context_lock;
			glPushAttrib(GL_TEXTURE_BIT);

			if (!handle)
			{
				glGetError();
				glGenTextures(1,&handle);
				GLenum error = glGetError();
				if (handle==0 || error != GL_NO_ERROR)
				{
					glPopAttrib();
					throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL refused to create texture handle ("+String(glGetErrorName(error))+")");
				}
			}
			texture_width = width_;
			texture_height = height_;
			texture_channels = channels_;
			texture_dimension = TextureDimension::Cube;
			//cout << "load "<<width<<"*"<<height<<"*"<<(int)channels<<endl;
			glGetError();
			glBindTexture(GL_TEXTURE_CUBE_MAP,handle);

			bool mipmap = configureFilter(GL_TEXTURE_CUBE_MAP, filter, 1.0f);

			typedef GLType<data_t>	Type;

			GLenum internal_format, import_format;
			formatAt<Type>(texture_channels,PixelType::Color,compress, internal_format, import_format);
			if (mipmap && (compress || !glGenerateMipmap)) //glGenerateMipmap apparently doesn't like compression
			{
				gluBuild2DMipmaps( GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, internal_format, texture_width, texture_height, import_format, Type::constant, data0);
				gluBuild2DMipmaps( GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, internal_format, texture_width, texture_height, import_format, Type::constant, data1);
				gluBuild2DMipmaps( GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, internal_format, texture_width, texture_height, import_format, Type::constant, data2);
				gluBuild2DMipmaps( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, internal_format, texture_width, texture_height, import_format, Type::constant, data3);
				gluBuild2DMipmaps( GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, internal_format, texture_width, texture_height, import_format, Type::constant, data4);
				gluBuild2DMipmaps( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, internal_format, texture_width, texture_height, import_format, Type::constant, data5);
			}
			else
			{
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, 0, internal_format, texture_width, texture_height, 0, import_format, Type::constant, data0);
				glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0, internal_format, texture_width, texture_height, 0, import_format, Type::constant, data1);
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0, internal_format, texture_width, texture_height, 0, import_format, Type::constant, data2);
				glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, 0, internal_format, texture_width, texture_height, 0, import_format, Type::constant, data3);
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0, internal_format, texture_width, texture_height, 0, import_format, Type::constant, data4);
				glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0, internal_format, texture_width, texture_height, 0, import_format, Type::constant, data5);


				if (mipmap)
				{
					ASSERT_NOT_NULL__(glGenerateMipmap);
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				}
			}

			//for (GLenum k = 0; k < 6; k++)
			{
				glTexParameteri( GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
				glTexParameteri( GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
				glTexParameteri( GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
			}
			glPopAttrib();
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw Except::Renderer::TextureTransfer::GeneralFault("OpenGL reports general fault while trying to load texture ("+String(glGetErrorName(error))+")");
		}

	template <typename Nature>
		void			GL::Texture::loadCube(const GenericImage<Nature> faces[6], TextureFilter filter, bool compress)
		{
			if (!faces)
				throw Except::Renderer::TextureTransfer::ParameterFault("Pointer to faces is NULL");

			
			GenericImage<Nature>	temp[6];
			const GenericImage<Nature>*image;
			if (alignableImages(faces,6))
			{
				temp[0].readFrom(&faces[0]);
				temp[1].readFrom(&faces[1]);
				temp[2].readFrom(&faces[2]);
				temp[3].readFrom(&faces[3]);
				temp[4].readFrom(&faces[4]);
				temp[5].readFrom(&faces[5]);
				alignImages(temp,6);
				image = temp;
			}
			else
				image = faces;
			loadCube(image[0].GetData(),image[1].GetData(),image[2].GetData(),image[3].GetData(),image[4].GetData(),image[5].GetData(),image[0].GetWidth(),image[0].GetHeight(),image[0].channels(),filter,compress);
		}

	template <typename Nature>
		void			GL::Texture::loadCube(const GenericImage<Nature>&face0, const GenericImage<Nature>&face1, const GenericImage<Nature>&face2, const GenericImage<Nature>&face3, const GenericImage<Nature>&face4, const GenericImage<Nature>&face5, TextureFilter filter, bool compress)
		{
			GenericImage<Nature>	temp[6];
			const GenericImage<Nature>*image[6] =	{	&face0,
														&face1,
														&face2,
														&face3,
														&face4,
														&face5};
			if (alignableImages(image,6))
			{
				temp[0].readFrom(&face0);
				temp[1].readFrom(&face1);
				temp[2].readFrom(&face2);
				temp[3].readFrom(&face3);
				temp[4].readFrom(&face4);
				temp[5].readFrom(&face5);
				alignImages(temp,6);
				image[0] = &temp[0];
				image[1] = &temp[1];
				image[2] = &temp[2];
				image[3] = &temp[3];
				image[4] = &temp[4];
				image[5] = &temp[5];
			}
			loadCube(image[0]->GetData(),image[1]->GetData(),image[2]->GetData(),image[3]->GetData(),image[4]->GetData(),image[5]->GetData(),image[0]->GetWidth(),image[0]->GetHeight(),image[0]->channels(),filter,compress);
		}




	template <typename TextureType>
		void OpenGL::genericBindMaterial(const MaterialConfiguration&config, const TextureType*list, bool has_shader)
		{
			if (state.render_setup.material != &config)
				state.render_setup.material = &config;

			if (!state.render_setup.bound_texture_layers)
			{
				if (glActiveTexture)
					glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
				glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
				glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
				glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
			}
			glThrowError();

			glMaterial(GL_FRONT_AND_BACK,GL_AMBIENT,config.ambient.v);
			glMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE,config.diffuse.v);
			glMaterial(GL_FRONT_AND_BACK,GL_SPECULAR,config.specular.v);
			glMaterial(GL_FRONT_AND_BACK,GL_EMISSION,config.emission.v);
			glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,config.shininess_exponent);

			glThrowError();

			if (config.alpha_test)
			{
				glDisable(GL_BLEND);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GEQUAL,config.alpha_threshold);
				glThrowError();
			}
			else
			{
				if (config.blend)
					glEnable(GL_BLEND);
				else
					glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);
				glThrowError();
			}

			/*if (glClientActiveTexture)
				for (unsigned i = 0; i < state.render_setup.bound_layers; i++)
				{
					glClientActiveTexture(GL_TEXTURE0+state.uvw_target[i]);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}*/

			//state.uvw_layers = 0;
			/*state.vertex = config.vertex;
			state.normal = config.normal;
			state.color = config.color;
			state.tangent = config.tangent;
			state.floats_per_vertex = config.floats_per_vertex;*/
			state.render_setup.expect_tangent_normals = false;
			//state.render_setup.dot3_bound = false;

			count_t layer_count = 
								std::min<count_t>(
									glExtensions.maxTextureLayers
									,
									(	
										glActiveTexture!=NULL
										?
										config.layers.Count()
										:
										std::min<count_t>(1,config.layers.Count())
									)
								);
			//unsigned bound_layers = 0;
			{
				for (index_t source_layer = 0; source_layer < layer_count; source_layer++)
				{
					const MaterialLayer&layer = config.layers[source_layer];
					bool	cube = list && !list[source_layer]->IsEmpty() && list[source_layer]->isCube(),
							normal_map = list && !list[source_layer]->IsEmpty() && layer.IsNormalMap();

					if (glActiveTexture)
						glActiveTexture(GL_TEXTURE0+static_cast<GLenum>(source_layer));
					if (normal_map)
						state.render_setup.expect_tangent_normals  |=  (layer.content_type == PixelType::TangentSpaceNormal);
					//glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,layer.combiner);
					glThrowError();
					if (!layer.enabled)
					{
						glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
						glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
						glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
						glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
						state.render_setup.texture_type[source_layer] = TextureDimension::None;
						state.render_setup.texcoord_type[source_layer] = TextureMapping::None;
						glThrowError();

					}
					else
					{
						switch (layer.system_type)
						{
							case SystemType::Identity:
								if(state.render_setup.matrix_type[source_layer] != layer.system_type)
								{
									glMatrixMode(GL_TEXTURE);
									glLoadIdentity();
									glMatrixMode(GL_MODELVIEW);
								}
							break;
							case SystemType::Custom:
								if (!layer.custom_system || M::VecUnroll<16>::zero(layer.custom_system->v))
									FATAL__("trying to bind zero matrix to layer "+String(source_layer)+" of material '"+config.name+"'");
								glMatrixMode(GL_TEXTURE);
									glLoadMatrixf(layer.custom_system->v);
								glMatrixMode(GL_MODELVIEW);
							break;
							case SystemType::Environment:
							{
								if (state.render_setup.matrix_type[source_layer] != layer.system_type)
								{
									glMatrixMode(GL_TEXTURE);
										glLoadMatrixf(environment_matrix.v);
										glScalef(-1,-1,-1);
									glMatrixMode(GL_MODELVIEW);
								}
							}
							break;
						}
						glThrowError();
						state.render_setup.matrix_type[source_layer] = layer.system_type;


						if (!cube)
						{
							glDisable(GL_TEXTURE_CUBE_MAP);
							state.render_setup.texcoord_type[source_layer] = TextureMapping::Default;
							glThrowError();

							if (list && !list[source_layer]->IsEmpty())
							{
								state.render_setup.texture_type[source_layer] = list[source_layer]->GetDimension();
								switch (list[source_layer]->GetDimension())
								{
									case TextureDimension::Linear:
										glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
										glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
										glEnable(GL_TEXTURE_1D);
										glBindTexture(GL_TEXTURE_1D,list[source_layer]->getHandle());
										glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,layer.clamp_x?GL_CLAMP_TO_EDGE:GL_REPEAT);
									break;
									case TextureDimension::Planar:
										glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
										glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
										glEnable(GL_TEXTURE_2D);
										glBindTexture(GL_TEXTURE_2D,list[source_layer]->getHandle());
										glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,layer.clamp_x?GL_CLAMP_TO_EDGE:GL_REPEAT);
										glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,layer.clamp_y?GL_CLAMP_TO_EDGE:GL_REPEAT);
									break;
									case TextureDimension::Volume:
										glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
										glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
										glEnable(GL_TEXTURE_3D);
										glBindTexture(GL_TEXTURE_3D,list[source_layer]->getHandle());
										glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,layer.clamp_x?GL_CLAMP_TO_EDGE:GL_REPEAT);
										glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,layer.clamp_y?GL_CLAMP_TO_EDGE:GL_REPEAT);
										glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,layer.clamp_z?GL_CLAMP_TO_EDGE:GL_REPEAT);
									break;
									default:
									break;
								}
							}
							else
							{
								state.render_setup.texture_type[source_layer] = TextureDimension::None;

								glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
								glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
								glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
							}
							glThrowError();

						}
						else
						{


							glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
							glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
							glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
							if (!list[source_layer]->IsEmpty())
							{
								state.render_setup.texture_type[source_layer] = list[source_layer]->GetDimension();
								glEnable(GL_TEXTURE_CUBE_MAP);
								glBindTexture(GL_TEXTURE_CUBE_MAP,list[source_layer]->getHandle());
							}
							else
							{
								glDisable(GL_TEXTURE_CUBE_MAP);
								state.render_setup.texture_type[source_layer] = TextureDimension::None;
							}
							glThrowError();
							/*if (config.texcoords[source_layer].IsEmpty())
							{
								if (!has_shader)
								{
									if (layer.reflect)
									{
										glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
										glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
										glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
										state.render_setup.texcoord_type[bound_layers] = TextureMapping::Reflection;
									}
									else
									{
										glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP);
										glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP);
										glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_NORMAL_MAP);
										state.render_setup.texcoord_type[bound_layers] = TextureMapping::Normal;
									}
									glEnable(GL_TEXTURE_GEN_S);
									glEnable(GL_TEXTURE_GEN_T);
									glEnable(GL_TEXTURE_GEN_R);
								}
								else
								{
									glDisable(GL_TEXTURE_GEN_S);	//let's not take chances here
									glDisable(GL_TEXTURE_GEN_T);
									glDisable(GL_TEXTURE_GEN_R);
								}
							}
							else*/
							{
								//state.uvw_section[state.uvw_layers] = config.texcoords[source_layer];
								state.render_setup.texcoord_type[source_layer] = TextureMapping::Default;
								//glDisable(GL_TEXTURE_GEN_S);
								//glDisable(GL_TEXTURE_GEN_T);
								//glDisable(GL_TEXTURE_GEN_R);
							}
						}
					}
				}

				if (Shader::globalSkyTexture.IsNotEmpty())
				{
					const index_t source_layer = layer_count;
					if (glActiveTexture)
						glActiveTexture(GL_TEXTURE0+static_cast<GLenum>(layer_count));

					if (state.render_setup.matrix_type[source_layer] != SystemType::Environment)
					{
						glMatrixMode(GL_TEXTURE);
							glLoadMatrixf(environment_matrix.v);
							glScalef(-1,-1,-1);
						glMatrixMode(GL_MODELVIEW);
					}
					state.render_setup.matrix_type[source_layer] = SystemType::Environment;
					state.render_setup.texture_type[source_layer] = TextureDimension::Cube;

					glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
					glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
					glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
					glEnable(GL_TEXTURE_CUBE_MAP);
					glBindTexture(GL_TEXTURE_CUBE_MAP,Shader::globalSkyTexture.getHandle());
					glThrowError();

					layer_count++;
				}
			
				for (index_t index = layer_count; index < state.render_setup.bound_texture_layers; index++)
				{
					if (glActiveTexture)
						glActiveTexture(GL_TEXTURE0+static_cast<GLenum>(index));
					glDisable(GL_TEXTURE_1D);	glBindTexture(GL_TEXTURE_1D,0);
					glDisable(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D,0);
					glDisable(GL_TEXTURE_3D);	glBindTexture(GL_TEXTURE_3D,0);
					glDisable(GL_TEXTURE_CUBE_MAP);	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
					glThrowError();
					//glDisable(GL_TEXTURE_GEN_S);
					//glDisable(GL_TEXTURE_GEN_T);
					//glDisable(GL_TEXTURE_GEN_R);
					if (state.render_setup.matrix_type[index] != SystemType::Identity)
					{
						glMatrixMode(GL_TEXTURE);
							glLoadIdentity();
						glMatrixMode(GL_MODELVIEW);
						state.render_setup.matrix_type[index] = SystemType::Identity;
					}
					glThrowError();
				}
			}
			//ASSERT__(shader.IsEmpty() || shader.requires_tangents == state.render_setup.expect_tangent_normals);

		//	glEnable(GL_BLEND);
			//glDisable(GL_ALPHA_TEST);
			state.render_setup.bound_texture_layers = layer_count;
	
			state.render_setup.changed = true;
		}


	/*

	template <class C>
	bool OpenGL::VertexBufferObject(VertexBufferObject*target,C*data,unsigned length)
	{
	    GLfloat*array = length?new GLfloat[length]:NULL;
	    _copy(data,array,length);
	    bool success = VertexBufferObject(target,array,length);
	    if (array)
	        delete[] array;
	    return success;
	}




	template <class C>
	GLuint OpenGL::VertexBufferObject(C*data, unsigned length)
	{
	    GLfloat*array = alloc<GLfloat>(length);
	    _copy(data,array,length);
	    VertexBufferObject result = VertexBufferObject<GLfloat>(array,length);
	    deloc(array);
	    return result;
	}




	template <class C>
	bool OpenGL::indexObject(IndexObject*target,C*data,unsigned length)
	{
	    GLuint*array = alloc<GLuint>(length);
	    _copy(data,array,length);
	    bool success = indexObject<GLuint>(target,array,length);
	    deloc(array);
	    return success;
	}




	template <class C>
	GLuint OpenGL::indexObject(C*data, unsigned length)
	{
	    GLuint*array = alloc<GLuint>(length);
	    _copy(data,array,length);
	    IndexObject result = indexObject(array,length);
	    deloc(array);
	    return result;
	}

	*/


	inline void OpenGL::onModelviewChange()
	{
	    if (lighting_enabled)
		{
			if (verbose)
				log_file<<"matrix loaded updating lights ("<<state.render_setup.num_enabled_lights<<")"<<nl;
			for (index_t i = 0; i < state.render_setup.num_enabled_lights; i++)
				updateLightPosition(state.render_setup.enabled_light_field[i]);
		}
	    else
	        state.matrix_changed = true;
	}




	inline	void		OpenGL::restoreCamera()
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		onModelviewChange();
	}


	inline	void		OpenGL::storeCamera()
	{
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
	}






	inline void OpenGL::renderVertexStrip(unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    glDrawArrays(GL_TRIANGLE_STRIP,vertex_offset,vertex_count);
		GL_END
	}


	template <class C>
	inline void OpenGL::renderSub(const C system[4*4], unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    glPushMatrix();
	        glMultMatrix(system);
	        renderExplicit(GL_TRIANGLES,vertex_offset,vertex_count);
	    glPopMatrix();
		GL_END
	}

	template <class C>
	inline void OpenGL::renderQuadsSub(const C system[4*4], unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    glPushMatrix();
	        glMultMatrix(system);
	        renderExplicit(GL_QUADS,vertex_offset,vertex_count);
	    glPopMatrix();
		GL_END
	}

	inline void OpenGL::render(unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    renderExplicit(GL_TRIANGLES,vertex_offset,vertex_count);
		GL_END
	}

	inline void OpenGL::renderQuads(unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    renderExplicit(GL_QUADS,vertex_offset,vertex_count);
		GL_END
	}

	template <class C>
	inline void OpenGL::renderStrip(const C system[4*4], unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    glPushMatrix();
	        glMultMatrix(system);
	        renderExplicit(GL_TRIANGLE_STRIP,vertex_offset,vertex_count);
	    glPopMatrix();
		GL_END
	}

	template <class C>
	inline void OpenGL::renderQuadStrip(const C system[4*4], unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    glPushMatrix();
	        glMultMatrix(system);
	        renderExplicit(GL_QUAD_STRIP,vertex_offset,vertex_count);
	    glPopMatrix();
		GL_END
	}

	inline void OpenGL::renderStrip(unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    renderExplicit(GL_TRIANGLE_STRIP,vertex_offset,vertex_count);
		GL_END
	}

	inline void OpenGL::renderQuadStrip(unsigned vertex_offset, unsigned vertex_count)
	{
		GL_BEGIN
	    renderExplicit(GL_QUAD_STRIP,vertex_offset,vertex_count);
		GL_END
	}


	template <class C> inline   void OpenGL::enterSubSystem(const M::TMatrix4<C>&system)
	{
		GL_BEGIN
	    glPushMatrix();
	    glMultMatrix(system.v);
		GL_END
	}

	inline   void OpenGL::exitSubSystem()
	{
		GL_BEGIN
	    glPopMatrix();
		GL_END
	}

	inline   void OpenGL::depthMask(bool mask)
	{
	    glDepthMask(mask);
	}

	inline	void OpenGL::setDepthTest(eDepthTest depthTest)
	{
		switch (depthTest)
		{
			case NoDepthTest:
				glDisable(GL_DEPTH_TEST);
			break;
			case NormalDepthTest:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
			break;
			case InvertedDepthTest:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_GREATER);
			break;
		}
	}


	inline   void OpenGL::cullNormal()
	{
	    glEnable(GL_CULL_FACE);
	    glCullFace(GL_BACK);
	}

	inline   void OpenGL::cullInverse()
	{
	    glEnable(GL_CULL_FACE);
	    glCullFace(GL_FRONT);
	}

	inline   void OpenGL::cullAll()
	{
	    glEnable(GL_CULL_FACE);
	    glCullFace(GL_FRONT_AND_BACK);
	}

	inline   void OpenGL::cullNone()
	{
	    glDisable(GL_CULL_FACE);
	}

	inline   void OpenGL::setFog(const Fog&fog,bool enabled)
	{
		GL_BEGIN
	    if (enabled)
	        glEnable(GL_FOG);
	    else
	        glDisable(GL_FOG);
	    float   nearPlane = fabs(fog.nearRange),
	            farPlane = fabs(fog.farRange);
	    if (farPlane < nearPlane)
	        swp(nearPlane,farPlane);

	    glFogf(GL_FOG_START, nearPlane);
	    glFogf(GL_FOG_END, farPlane);
	    glFogf(GL_FOG_DENSITY,fog.density);
	    glFogfv(GL_FOG_COLOR,fog.color.v);
	    switch (fog.type)
	    {
	        case Fog::Linear:
	            glFogi(GL_FOG_MODE,GL_LINEAR);
	        break;
	        case Fog::Exp:
	            glFogi(GL_FOG_MODE,GL_EXP);
	        break;
	        case Fog::Exp2:
	            glFogi(GL_FOG_MODE,GL_EXP2);
	        break;
	    }
		GL_END
	}

	inline   void OpenGL::setFog(bool enabled)
	{
	    if (enabled)
	        glEnable(GL_FOG);
	    else
	        glDisable(GL_FOG);
	}


	inline   void OpenGL::SetBackbufferClearColor(float red, float green, float blue, float alpha)
	{
	    glClearColor(red,green,blue,alpha);
	}

	inline   void OpenGL::SetBackbufferClearColor(const M::TVec3<>&color, float alpha)
	{
	    glClearColor(color.red,color.green,color.blue,alpha);
	}

	inline   void OpenGL::SetBackbufferClearColor(const M::TVec4<>&color)
	{
	    glClearColor(color.red,color.green,color.blue,color.alpha);
	}
	inline	void	OpenGL::setDefaultBlendFunc()
	{
		if (glBlendFuncSeparate)
			glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	inline	void	OpenGL::setDefaultBlendMode()
	{
		if (glBlendFuncSeparate)
			glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	inline	void	OpenGL::defaultBlendFunc()
	{
		if (glBlendFuncSeparate)
			glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	inline	void	OpenGL::defaultBlendMode()
	{
		if (glBlendFuncSeparate)
			glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	inline	void	OpenGL::setFlareBlendFunc()
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	}

	inline	void	OpenGL::setFlareBlendMode()
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	}
	inline	void	OpenGL::flareBlendFunc()
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	}

	inline	void	OpenGL::flareBlendMode()
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	}

	/*static*/	inline	void	OpenGL::overrideEmission(const M::TVec4<>&emission_color)
	{
		//float color[4] = {emission_color.red,emission_color.green, emission_color.blue,1.0f};
		glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,emission_color.v);
	}
	
	
	template <typename T>
		inline GLenum bufferType()
		{
			FATAL__("Trying to load unsupported data type into OpenGL buffer");
		}
	
	template <>
		inline GLenum bufferType<GLfloat>()
		{
			return GL_ARRAY_BUFFER_ARB;
		}
	
	template <>
		inline GLenum bufferType<GLuint>()
		{
			return GL_ELEMENT_ARRAY_BUFFER_ARB;
		}

	
	template <typename T>
		inline bool GL::GeometryBuffer<T>::load(const T*field, size_t num_units)
		{
			return OpenGL::queryGeometryUpload() && Buffer::loadData(field, num_units*sizeof(T), bufferType<T>());
		}

	template <typename T>
		inline void GL::SmartGeometryBuffer<T>::load(const T*field, size_t num_units)
		{
			SmartBuffer::loadData(field, num_units*sizeof(T), bufferType<T>());
		}

}

#endif
