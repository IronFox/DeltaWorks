#ifndef profilerTplH
#define profilerTplH



namespace Engine
{
	namespace Profiler
	{

		template <class Font>
			Engine::Textout<Font>		TextoutManager<Font>::local;
		template <class Font>
			Engine::Textout<Font>		*TextoutManager<Font>::active(&local);
			
		template <class Font>
			Engine::Textout<Font>*		TextoutManager<Font>::get()
			{
				return active;
			}
			
		template <class Font>
			void				TextoutManager<Font>::Set(Engine::Textout<Font>*textout)
			{
				active = textout;
			}
		
		template <class GL, class Base>
			GL*					RenderGraph<GL,Base>::getRenderer()
			{
				if (!renderer)
					renderer = GL::globalInstance;
				return renderer;
			}
			
		template <class GL, class Base>
			void				RenderGraph<GL,Base>::init()
			{
				
				if (!geometry_defined)
				{
					background.select(SimpleGeometry::Quads);
					background.color(0,0,0,0.4);
					background.vertex(0,0);
					background.vertex(1,0);
					background.vertex(1,1);
					background.vertex(0,1);
					background.seal();
					
								
					geometry_defined = true;
				}
			}
		
		template <class GL, class Base>
			void	RenderGraph<GL,Base>::renderGeometry()
			{
				if (!getRenderer() || ibo.IsEmpty())
					return;
				
				renderer->bindMaterial(material,typename GL::Shader());
				renderer->bindVertices(vbo,binding);
				renderer->bindIndices(ibo);
				renderer->renderQuads(0,unsigned(indices.GetLength()));
				
				renderer->unbindAll();
			}
		
		template <class GL, class Base>
			void	RenderGraph<GL,Base>::beginUpdate()
			{
				indices.reset();
				vertices.reset();
				outlines.reset();
				outlines.select(SimpleGeometry::Lines);
			}
		
			template <class Font>
				void	C3dGraph::printCaptions()
				{
					Textout<Font>*textout = TextoutManager<Font>::get();
					if (data->countChannels() != text_location.GetLength())
						return;
					const M::TVec3<>*location = text_location.pointer();
					for (index_t i = 0; i < data->groups(); i++)
					{
						const Group&group = data->group(i);
						for (index_t j = 0; j < group.channels(); j++)
						{
							M::TVec3<> color;
							M::Vec::interpolate(group.color,group.channel(j).color,0.25,color);
							textout->SetColor(color);
							textout->MoveTo(*location);
							textout->Print(group.channel(j).name);
						
							location++;
						}
					}
					ASSERT_CONCLUSION(text_location,location);
				}
		
		template <class GL, class Base>
			void	RenderGraph<GL,Base>::shade()
			{
				if (dimensions < 3)
					return;
				unsigned stride = (4+dimensions);
				unsigned vcnt = vertices.Count()/stride;
				Array<M::TVec3<> >	normals(vcnt);
				normals.Fill(0);
				for (unsigned i = 0; i < indices.Count()/4; i++)
				{
					M::TVec3<>	n0,
							n1;
					_oTriangleNormal(vertices+indices[i*4]*stride,
									vertices+indices[i*4+1]*stride,
									vertices+indices[i*4+2]*stride,
									n0.v);
					_oTriangleNormal(vertices+indices[i*4]*stride,
									vertices+indices[i*4+2]*stride,
									vertices+indices[i*4+3]*stride,
									n1.v);
					M::Vec::add(n0,n1);
					M::Vec::add(normals[indices[i*4]],n0);
					M::Vec::add(normals[indices[i*4+1]],n0);
					M::Vec::add(normals[indices[i*4+2]],n0);
					M::Vec::add(normals[indices[i*4+3]],n0);
				}
				if (fabs(M::Vec::dot(C3dGraph::light)-1.0f)>M::GetError<float>())
					M::Vec::normalize0(C3dGraph::light);

				for (unsigned i = 0; i < vcnt; i++)
				{
					M::TVec3<>&n = normals[i];
					M::Vec::normalize0(n);
					float nd = M::Max(M::Vec::dot(n,C3dGraph::light),0.0f)*0.8+0.2;
					M::Vec::mult(M::Vec::ref3(vertices+i*stride+dimensions),nd);
				}
			}

		template <class GL, class Base>
			void	RenderGraph<GL,Base>::finishUpdate()
			{
				indices.compact();
				vertices.compact();
				outlines.compact();
				
				unsigned stride = (dimensions+4);
				count_t vcnt = vertices.Count()/stride;
				if (opaque)
				{
					for (index_t i = 0; i < vcnt; i++)
					{
						float alpha = vertices[i*stride+dimensions+3]*fade_intensity + (1.0f-fade_intensity);
						M::Vec::interpolate(C3dGraph::background,M::Vec::ref3(vertices+i*stride+dimensions),alpha,M::Vec::ref3(vertices+i*stride+dimensions));
						vertices[i*stride+dimensions+3] = 1.0f;
					}
				}
				
				ASSERT2__(!(vertices.Count()%(dimensions+4)),vertices.Count(),dimensions);
				ASSERT1__(!(indices.Count()%4),indices.Count());
				
				for (index_t i = 0; i < indices.Count(); i++)
					ASSERT3__(indices[i]<vcnt,i,indices[i],vcnt);
				
				if (dimensions == 3)
					for (index_t i = 0; i < vcnt; i++)
					{
						ASSERT_IS_CONSTRAINED3__(vertices+i*(4+dimensions),-10,10);
					}

				vbo.load(vertices);
				ibo.load(indices);
			}
		





	
	template <class GL, class Font>
		ColumnGraph<GL,Font>::ColumnGraph(Data*data):Base(data,2),highest(0),caption_left(true)
		{}
		
		
	template <class GL, class Font>
		void		ColumnGraph<GL,Font>::Draw()
		{
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
			M::TMatrix4<float> matrix;
			Mat::Eye(matrix);
			Base::position.GetMin(matrix.w.xy);
			matrix[0] = Base::position.GetWidth();
			matrix[5] = Base::position.GetHeight();
			renderer->enterSubSystem(matrix);
				renderer->useTexture(NULL);
				Base::init();
				renderer->render(Base::background);
				Base::renderGeometry();
				
				renderer->render(Base::outlines);
				
				renderer->render(Base::frame);
			renderer->exitSubSystem();
			
			
			Textout<Font>*active_textout = TextoutManager<Font>::get();
			float indent = active_textout->GetScaledWidth('X');
			float x = Base::position.right+indent/2;
			if (caption_left)
			{
				float width = 0;
				
				if (Base::view != Solid)
				{
					for (unsigned i = 0; i < Base::data->groups(); i++)
					{
						const Group&group = Base::data->group(i);
						width = vmax(width,active_textout->GetScaledWidth(group.name));
						if (Base::view == Detailed && group.channels()>1)
						{
							for (unsigned j = 0; j < group.channels(); j++)
								width = vmax(width,active_textout->GetScaledWidth(group.channel(j).name)+indent);
						}
					}
				}
				
				x = Base::position.left-width-indent/2;
			}
			float y = Base::position.bottom;
			
			active_textout->MoveTo(Base::position.left+indent/2,Base::position.top-active_textout->state.y_scale);
			active_textout->SetColor(1,1,1);
			active_textout->Print(Base::name);
			
			
			if (view != Solid && print_channel_names)
			{
				//x+=indent;
				for (unsigned i = 0; i < data->groups(); i++)
				{
					const Group&group = data->group(i);
					if (view == Detailed && group.channels()>1)
					{
						x+=indent;
						for (unsigned j = 0; j < group.channels(); j++)
						{
							const Channel&channel = group.channel(j);
							M::TVec3<> color;
							M::Vec::interpolate(group.color,channel.color,0.25,color);
							active_textout->MoveTo(x,y);
							active_textout->SetColor(color);
							y+=active_textout->state.scale_y*active_textout->GetFont().GetHeight();
							active_textout->Print(channel.name);
						}
						x-=indent;
					}
					active_textout->MoveTo(x,y);
					active_textout->SetColor(group.color);
					y+=active_textout->state.scale_y*active_textout->GetFont().GetHeight();
					active_textout->Print(group.name);
				}
			}
			active_textout->SetColor(1,1,1);
			
			//active_textout->locate(position.right-active_textout->scaledLength(str_current)-indent/2, position.top-active_textout->state.y_scale);
			//active_textout->Print(str_current);
			
		}
	
	
	template <class GL, class Font>
		void		ColumnGraph<GL,Font>::addRow(const M::TVec3<>&color, unsigned res)
		{
			index_t offset = Base::vertices.Count()/(Base::dimensions+4);
			float	lowest = 1000000,
					highest = 0;
			for (unsigned t = 0; t < res; t++)
				if (h_field[t]+h_block[t] > 0)
				{
					if (h_field[t]+h_block[t] > highest)
						highest = h_field[t]+h_block[t];
					if (lowest > h_field[t])
						lowest = h_field[t];
				}
			float invrange = highest > lowest?(1.0f/(highest-lowest)):1.0f;
			Base::outlines.color(color.r,color.g,color.b);
			float bar_width = 1.0f/(res),
					spacing = bar_width/8;
			/*if (h_block[0])
				Base::outlines.vertex(0,h_field[0]);*/
			for (unsigned t = 0; t < res; t++)
			{
				float x = (float)t*bar_width;
				Base::vertices << x+spacing << h_field[t];
				Base::vertices << color.red+0.2 << color.green+0.2 << color.blue+0.2 << (0.1+0.4*(h_field[t]-lowest)*invrange+0.2);
				Base::vertices << x+spacing << (h_field[t]+h_block[t]);
				Base::vertices << color.red+0.2 << color.green+0.2 << color.blue+0.2 << (0.1+0.4*(h_field[t]+h_block[t]-lowest)*invrange+0.2);
				
				Base::vertices << x+bar_width-spacing << h_field[t];
				Base::vertices << color.red << color.green << color.blue << (0.1+0.4*(h_field[t]-lowest)*invrange);
				Base::vertices << x+bar_width-spacing << (h_field[t]+h_block[t]);
				Base::vertices << color.red << color.green << color.blue << (0.1+0.4*(h_field[t]+h_block[t]-lowest)*invrange);
				
				
				if (h_block[t])
				{
					Base::outlines.color3fv(color);
					//Base::outlines.color3fva(color,0);
					Base::outlines.vertex(x+spacing,(h_field[t]));
					//Base::outlines.color3fv(color);
					Base::outlines.vertex(x+spacing,(h_field[t]+h_block[t]));
					Base::outlines.vertex(x+spacing,(h_field[t]+h_block[t]));
					Base::outlines.color(color,0);
					Base::outlines.vertex(x+bar_width-spacing,(h_field[t]+h_block[t]));
					/*Base::outlines.vertex(x+bar_width-spacing,(h_field[t]+h_block[t]));
					Base::outlines.color3fva(color,0);
					Base::outlines.vertex(x+bar_width-spacing,(h_field[t]));*/
				}
				//if (t && t < res-1)
					//Base::outlines.vertex(x+bar_width,(h_field[t]+h_block[t]));
				
				h_field[t] += h_block[t];
				h_block[t] = 0;
			}
			/*if (h_block[res-1])
				Base::outlines.vertex(1,h_field[res-1]);*/
			for (unsigned t = 0; t < res; t++)
			{
				Base::indices << offset+t*4+1;
				Base::indices << offset+t*4;
				Base::indices << offset+t*4+2;
				Base::indices << offset+t*4+3;
				
			}
		}

	template <class GL, class Font>
		void		ColumnGraph<GL,Font>::update()
		{
			static bool exported = false;
		
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
		
			if (!data->groups())
			{
				return;
			}
			Base::beginUpdate();
			
			unsigned res = Profiler::Config::channelResolution();
			if (!res)
				return;
			h_field.Resize(res);
			h_block.Resize(res);
			h_block.Fill(0);
			h_field.Fill(0);
			unsigned cnt = 0;
			for (unsigned i = 0; i < data->groups(); i++)
			{
				const Group&group = data->group(i);
				for (unsigned j = 0; j < group.channels(); j++)
				{
					const Channel&channel = group.channel(j);
					//ASSERT2__(channel.weighted_history.GetLength() == res,channel.weighted_history.GetLength(),res);
					ASSERT2__(channel.history.GetLength() == res,channel.history.GetLength(),res);
					const float*h = channel.history.pointer();
					for (unsigned t = 0; t < res; t++)
						h_block[t] += h[t];
					
					if (view == Detailed)
					{
						M::TVec3<> color;
						if (group.channels()>1)
							M::Vec::interpolate(group.color,channel.color,0.25,color);
						else
							M::Vec::copy(group.color,color);
						addRow(color,res);
					}
				}
				if (view == Simplified)
					addRow(group.color,res);
			}
			if (view == Solid)
			{
				M::TVec3<> color={0,0,0};
				for (index_t i = 0; i < data->groups(); i++)
					M::Vec::add(color,data->group(i).color);
				M::Vec::div(color,data->groups());
				
				addRow(color,res);
			}
			//ASSERT__(indices.Count() == cnt*(res-1)*4);
			
			
			
			highest = 0.004;
			for (unsigned i = 0; i < res; i++)
				if (h_field[i] > highest)
					highest = h_field[i];
			float scale = highest?1.0f/(highest*1.25f):1.0f;
			
			for (index_t i = 0; i < Base::vertices.Count()/(Base::dimensions+4); i++)
			{
				Base::vertices[i*(Base::dimensions+4)+1] *= scale;
			}
			
			for (index_t i = 0; i < Base::outlines.field[SimpleGeometry::Lines].Count(); i++)
			{
				Base::outlines.field[SimpleGeometry::Lines][i].position[1] *= scale;
			
			}
			
			
			
			
			Base::finishUpdate();
		}
		



		
		
	template <class GL, class Font>
		StackedGraph<GL,Font>::StackedGraph(Data*data):Base(data,2),scale(1),highest(0),caption_left(true)
		{}
		
		
	template <class GL, class Font>
		void		StackedGraph<GL,Font>::Draw()
		{
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
			M::TMatrix4<> matrix = M::Matrix<>::eye4;
			matrix.w.xy = Base::position.min();
			matrix.x.x = Base::position.GetWidth();
			matrix.y.y = Base::position.GetHeight();
			renderer->enterSubSystem(matrix);
			
			
			renderer->useTexture(NULL);
			Base::init();
			renderer->render(Base::background);
			Base::renderGeometry();
			
			renderer->render(Base::outlines);
			
			renderer->render(Base::frame);
			
			renderer->exitSubSystem();
			
			Engine::Textout<Font>*active_textout = TextoutManager<Font>::get();
			
			float indent = active_textout->GetScaledWidth('X');
			
			float x = Base::position.x.max+indent/2;
			
			if (caption_left)
			{
				float width = 0;

				if (Base::view != Solid)
				{
					for (index_t i = 0; i < Base::data->groups(); i++)
					{
						const Group&group = Base::data->group(i);
						width = M::Max(width,active_textout->GetScaledWidth(group.name));
						if (Base::view == Detailed && group.channels()>1)
						{
							for (index_t j = 0; j < group.channels(); j++)
								width = M::Max(width,active_textout->GetScaledWidth(group.channel(j).name)+indent);
						}
					}
				}
				
				x = Base::position.x.min-width-indent/2;
			}
			float y = Base::position.y.min;
			
			active_textout->MoveTo(Base::position.x.min+indent/2,Base::position.y.max-active_textout->state.y_scale);
			active_textout->SetColor(1,1,1);
			active_textout->Print(Base::name);
			
			
			const Channel*exclusive = NULL;
			if (group >= 0 && (index_t)group < data->groups() && channel >= 0 && (index_t)channel < data->group(group).channels())
				exclusive = &data->group(group).channel(channel);
			
			float 	base = Base::position.y.min,
					font_height = active_textout->state.scale_y*active_textout->GetFont().GetHeight();
			
			if ((view != Solid || exclusive) && print_channel_names)
			{
				//x+=indent;
				for (index_t i = 0; i < data->groups(); i++)
				{
					const Group&group = data->group(i);
					if (view == Detailed && group.channels() > 1)
					{
						x+=indent;
						for (index_t j = 0; j < group.channels(); j++)
						{
							const Channel&channel = group.channel(j);
							M::TVec3<> color;
							M::Vec::interpolate(group.color,channel.color,0.25,color);
							active_textout->MoveTo(x,y);
							if (!exclusive || exclusive==&channel)
								active_textout->SetColor(color);
							else
								active_textout->SetColor(color,0.25);
							
							y+=font_height;
							active_textout->Print(channel.name);
							
						}
						x-=indent;
					}
					
					if (this->group < 0 || channel < 0)
						for (index_t j = 0; j < group.channels(); j++)
						{
							const Channel&channel = group.channel(j);
							float this_height = channel.history.Last()*scale*Base::position.GetHeight();
							if (this_height>font_height)
							{
								M::TVec3<> color;
								if (group.channels() > 1)
									M::Vec::interpolate(group.color,channel.color,0.25,color);
								else
									M::Vec::copy(group.color,color);
								active_textout->SetColor(color,0.5);
								active_textout->MoveTo(x,y);
								
								active_textout->MoveTo(Base::position.x.max-active_textout->GetScaledWidth(channel.name),base+this_height*0.65-font_height*0.5);
								active_textout->Print(channel.name);
							
							
							}
							base += this_height;
						}
					
					
					active_textout->MoveTo(x,y);
					if (!exclusive)
						active_textout->SetColor(group.color);
					else
						active_textout->SetColor(group.color,0.25);
					y+=active_textout->state.scale_y*active_textout->GetFont().GetHeight();
					active_textout->Print(group.name);
				}
			}
			active_textout->SetColor(1,1,1);
			
			active_textout->MoveTo(position.x.max-active_textout->GetScaledWidth(str_current)-indent/2, position.y.max-active_textout->state.y_scale*active_textout->GetFont().GetHeight());
			active_textout->Print(str_current);
			
		}
	
	
	template <class GL, class Font>
		void		StackedGraph<GL,Font>::addRow(const M::TVec3<>&color)
		{
			unsigned res = data->resolution();
			index_t offset = Base::vertices.Count()/(Base::dimensions+4);
			float	lowest = 1000000,
					highest = 0;
			for (unsigned t = 0; t < res; t++)
				if (h_field[t]+h_block[t] > 0)
				{
					if (h_field[t]+h_block[t] > highest)
						highest = h_field[t]+h_block[t];
					if (lowest > h_field[t])
						lowest = h_field[t];
				}
			float invrange = highest > lowest?(1.0f/(highest-lowest)):1.0f;
			Base::outlines.color(color);
			for (unsigned t = 0; t < res; t++)
			{
				float x = (float)t/(res-1);
				Base::vertices << x << h_field[t];
				Base::vertices << color.red << color.green << color.blue << (0.1+0.4*(h_field[t]-lowest)*invrange)*x;
				
				Base::vertices << x << (h_field[t]+h_block[t]);
				Base::vertices << color.red << color.green << color.blue << (0.1+0.4*(h_field[t]+h_block[t]-lowest)*invrange)*x;
				
				Base::outlines.vertex(x,(h_field[t]+h_block[t]));
				if (t && t < res-1)
					Base::outlines.vertex(x,(h_field[t]+h_block[t]));
				
				h_field[t] += h_block[t];
				h_block[t] = 0;
			}
			for (unsigned t = 0; t < res-1; t++)
			{
				Base::indices << typename GL::IndexType(offset+t*2+1);
				Base::indices << typename GL::IndexType(offset+t*2);
				Base::indices << typename GL::IndexType(offset+t*2+2);
				Base::indices << typename GL::IndexType(offset+t*2+3);
			}
		}
	
	template <class GL, class Font>
		void		StackedGraph<GL,Font>::addMaxRow(const M::TVec3<>&color)
		{
			unsigned res = data->resolution();
			index_t offset = Base::vertices.Count()/(Base::dimensions+4);
			float	lowest = 1000000,
					highest = 0;
			for (unsigned t = 0; t < res; t++)
				if (h_block[t] > 0)
				{
					if (h_block[t] > highest)
						highest = h_block[t];
					if (lowest > h_field[t])
						lowest = h_field[t];
				}
			float invrange = highest > lowest?(1.0f/(highest-lowest)):1.0f;
			Base::outlines.color(color);
			for (unsigned t = 0; t < res; t++)
			{
				float x = (float)t/(res-1);
				Base::vertices << x << h_field[t];
				Base::vertices << color.red << color.green << color.blue << (0.1+0.4*(h_field[t]-lowest)*invrange)*x;
				
				Base::vertices << x << (h_block[t]);
				Base::vertices << color.red << color.green << color.blue << (0.1+0.4*(h_block[t]-lowest)*invrange)*x;
				
				Base::outlines.vertex(x,(h_block[t]));
				if (t && t < res-1)
					Base::outlines.vertex(x,(h_block[t]));
				
				h_field[t] = h_block[t];
				h_block[t] = 0;
			}
			for (unsigned t = 0; t < res-1; t++)
			{
				Base::indices << typename GL::IndexType(offset+t*2+1);
				Base::indices << typename GL::IndexType(offset+t*2);
				Base::indices << typename GL::IndexType(offset+t*2+2);
				Base::indices << typename GL::IndexType(offset+t*2+3);
			}
		}

	template <class GL, class Font>
		void		StackedGraph<GL,Font>::addLineRow(const M::TVec3<>&color)
		{
			unsigned res = data->resolution();
			index_t	offset = Base::vertices.Count()/(Base::dimensions+4);
			float	lowest = 1000000,
					highest = 0;
			for (unsigned t = 0; t < res; t++)
				if (h_field[t]+h_block[t] > 0)
				{
					if (h_field[t]+h_block[t] > highest)
						highest = h_field[t]+h_block[t];
					if (lowest > h_field[t])
						lowest = h_field[t];
				}
			float invrange = highest > lowest?(1.0f/(highest-lowest)):1.0f;
			Base::outlines.color(color);
			for (unsigned t = 0; t < res; t++)
			{
				float x = (float)t/(res-1);
				Base::outlines.vertex(x,(h_field[t]+h_block[t]));
				if (t && t < res-1)
					Base::outlines.vertex(x,(h_field[t]+h_block[t]));
				
				h_field[t] += h_block[t];
				//if (h_block[t])
					//cout << h_block[t]<<endl;
				h_block[t] = 0;
			}
		}

	template <class GL, class Font>
		void		StackedGraph<GL,Font>::update()
		{
			static bool exported = false;
		
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
		
			if (!data->groups())
			{
				return;
			}
			Base::beginUpdate();
			
			unsigned res = data->resolution();
			if (!res)
				return;
			h_field.SetSize(res);
			h_block.SetSize(res);
			h_block.Fill(0);
			h_field.Fill(0);
			unsigned cnt = 0;
			if (group < 0 || (unsigned)group >= data->groups() || channel < 0 || (unsigned)channel >= data->group(group).channels())
			{
				for (unsigned i = 0; i < data->groups(); i++)
				{
					const Group&group = data->group(i);
					for (unsigned j = 0; j < group.channels(); j++)
					{
						const Channel&channel = group.channel(j);
						ASSERT2__(channel.history.GetLength() == res,channel.history.GetLength(),res);
						const float*h = channel.history.pointer();
						for (unsigned t = 0; t < res; t++)
							h_block[t] += h[t];
						
						if (view == Detailed)
						{
							M::TVec3<>	color;
							if (group.channels()>1)
								M::Vec::interpolate(group.color,channel.color,0.25,color);
							else
								M::Vec::copy(group.color,color);
							addRow(color);
						}
					}
					if (view == Simplified)
						addRow(group.color);
				}
				if (view == Solid)
				{
					M::TVec3<> color={0,0,0};
					for (index_t i = 0; i < data->groups(); i++)
						M::Vec::add(color,data->group(i).color);
					M::Vec::div(color,data->groups());
					
					addRow(color);
				}

				using StringConversion::ToString;

				str_current = ToString(h_field.Last());
				
			}
			else
			{
				const Group&gr = data->group(group);
				const Channel&ch = gr.channel(channel);
				

				ASSERT2__(ch.maximum_history.GetLength() == res,ch.maximum_history.GetLength(),res);
				ASSERT2__(ch.minimum_history.GetLength() == res,ch.minimum_history.GetLength(),res);
				ASSERT2__(ch.history.GetLength() == res,ch.history.GetLength(),res);
				{
					const float*h = ch.minimum_history.pointer();
					for (unsigned t = 0; t < res; t++)
						h_block[t] += h[t];
					
					addLineRow(gr.color);
				}
				{
					const float*h = ch.history.pointer();
					for (unsigned t = 0; t < res; t++)
						h_block[t] += h[t];
					
					addMaxRow(gr.color);
				}
				{
					const float*h = ch.maximum_history.pointer();
					for (unsigned t = 0; t < res; t++)
						h_block[t] += h[t];
					
					addMaxRow(gr.color);
				}
				str_current = "~"+String(ch.history.Last(),2)+" ["+String(ch.minimum_history.Last(),2)+", "+String(ch.maximum_history.Last(),2)+"]";
				
			}
			//ASSERT__(indices.Count() == cnt*(res-1)*4);
			
			
			
			highest = 0;
			for (unsigned i = 0; i < res; i++)
				if (h_field[i] > highest)
					highest = h_field[i];
			scale = highest?1.0f/(highest*1.25f):1.0f;
			
			for (index_t i = 0; i < Base::vertices.Count()/(Base::dimensions+4); i++)
			{
				Base::vertices[i*(Base::dimensions+4)+1] *= scale;
			}
			
			for (index_t i = 0; i < Base::outlines.field[SimpleGeometry::Lines].Count(); i++)
			{
				Base::outlines.field[SimpleGeometry::Lines][i].y *= scale;
			
			}
			
			
			
			
			Base::finishUpdate();
		}
		



		
		
		
		
	
	template <class GL, class Font>
		StairGraph<GL,Font>::StairGraph(Data*data):Base(data,3),highest(0)
		{
			depth = 0.3;
		}
		
		
	template <class GL, class Font>
		void		StairGraph<GL,Font>::Draw()
		{
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
				
			renderer->enterSubSystem(Base::system);
			
			renderer->useTexture(NULL);
			Base::init();
			Base::renderGeometry();
			
			renderer->render(Base::outlines);
			{
				M::TMatrix4<> scaled;
				Mat::Eye(scaled);
				scaled.x.x = width*1.01;
				scaled.y.y = height*1.01;
				scaled.z.z = depth*1.01;
				renderer->enterSubSystem(scaled);
				renderer->render(frame);
				renderer->exitSubSystem();
			}
			
			C3dGraph::printCaptions<Font>();
			
			renderer->exitSubSystem();
			
/* 			Textout<Font>*active_textout = TextoutManager<Font>::get();
			
			float indent = active_textout->scaledLength('X');
			
			float x = position.right+indent/2;
			
			if (caption_left)
			{
				float width = 0;
				
				if (view != Solid)
				{
					for (unsigned i = 0; i < data->groups(); i++)
					{
						const Group&group = data->group(i);
						width = vmax(width,active_textout->scaledLength(group.name));
						if (view == Detailed)
						{
							for (unsigned j = 0; j < group.channels(); j++)
								width = vmax(width,active_textout->scaledLength(group.channel(j).name)+indent);
						}
					}
				}
				
				x = position.left-width-indent/2;
			}
			float y = position.bottom;
			
			active_textout->locate(position.left+indent/2,position.top-active_textout->state.y_scale);
			active_textout->SetColor(1,1,1);
			active_textout->Print(name);
			
			
			if (view != Solid)
			{
				//x+=indent;
				for (unsigned i = 0; i < data->groups(); i++)
				{
					const Group&group = data->group(i);
					if (view == Detailed)
					{
						x+=indent;
						for (unsigned j = 0; j < group.channels(); j++)
						{
							const Channel&channel = group.channel(j);
							float color[3];
							_interpolate(group.color,channel.color,0.5,color);
							active_textout->locate(x,y);
							active_textout->SetColor3fv(color);
							y+=active_textout->state.scale_y;
							active_textout->Print(channel.name);
						}
						x-=indent;
					}
					active_textout->locate(x,y);
					active_textout->SetColor3fv(group.color);
					y+=active_textout->state.scale_y;
					active_textout->Print(group.name);
				}
			}
			active_textout->SetColor(1,1,1);
			
			active_textout->locate(position.right-active_textout->scaledLength(str_current)-indent/2, position.top-active_textout->state.y_scale);
			active_textout->Print(str_current);
 */			
		}
	
	
	template <class GL, class Font>
		void		StairGraph<GL,Font>::addRow(const M::TVec3<>&color, float depth)
		{
			unsigned res = data->resolution();
			if (res < 2)
				return;
			unsigned offset = Base::vertices.Count()/(Base::dimensions+4);
			float	lowest = 1000000,
					highest = 0;
			for (unsigned t = 0; t < res; t++)
				if (h_field[t]+h_block[t] > 0)
				{
					if (h_field[t]+h_block[t] > highest)
						highest = h_field[t]+h_block[t];
					if (lowest > h_field[t])
						lowest = h_field[t];
				}
			float invrange = highest > lowest?(1.0f/(highest-lowest)):1.0f;
			if (highest <= lowest)
				lowest = highest = 0;
			Base::outlines.color(color.g,color.g,color.b);
			for (unsigned t = 0; t < res; t++)
			{
				float	x = (float)t/(res-1),
						px = (x*2-1)*Base::width,
						a0 = x*0.9,//(0.3+0.7*(h_field[t]-lowest)*invrange)*x,
						a1 = x*0.9;//(0.3+0.7*(h_field[t]+h_block[t]-lowest)*invrange)*x;
				float d = h_block[t];
				if (!d)
				{
					a0 = a1 = 0;
				}
				Base::vertices << px << h_field[t] << Base::depth*depth;
				Base::vertices.append(color,3) << a0;
				
				Base::vertices << px << h_field[t] << -Base::depth*depth;
				Base::vertices.append(color,3) << a0;
				
				Base::vertices << px << (h_field[t]+d) << Base::depth*depth;
				Base::vertices.append(color,3) << a1;
				
				Base::vertices << px << (h_field[t]+d) << -Base::depth*depth;
				Base::vertices.append(color,3) << a1;
				
				
				Base::vertices << px << (h_field[t]+d) << Base::depth*depth;
				if (d)
					Base::vertices.append(color,3) << M::Min(x*2,1);
				else
					Base::vertices.append(color,3) << 0;
				
				Base::vertices << px << (h_field[t]+d) << -Base::depth*depth;
				if (d)
					Base::vertices.append(color,3) << M::Min(x*2,1);
				else
					Base::vertices.append(color,3) << 0;
				
				bool broken = false;
				if (t)
				{
					float gain = (h_field[t]+h_block[t])/(h_field[t-1]+h_block[t-1]);
					if (gain > 1.1 || gain < 0.9)
					{
						broken = true;
						Base::vertices << px << (h_field[t]+d) << Base::depth*depth;
						if (d)
							Base::vertices.append(color,3) << M::Min(x*2,1);
						else
							Base::vertices.append(color,3) << 0;
						
						Base::vertices << px << (h_field[t]+d) << -Base::depth*depth;
						if (d)
							Base::vertices.append(color,3) << M::Min(x*2,1);
						else
							Base::vertices.append(color,3) << 0;
						offset+=2;
					
					}
				}
				if (t<res-1)
				{
					//unsigned offset = Base::vertices.Count()/(Base::dimensions+4)-6;
					
					Base::indices << offset+2-broken*2;
					Base::indices << offset-broken*2;
					Base::indices << offset+6;
					Base::indices << offset+8;
				
					Base::indices << offset+10;
					Base::indices << offset+11;
					Base::indices << offset+5;
					Base::indices << offset+4;
/* 					Base::indices << offset+5-broken*2;
					Base::indices << offset+4-broken*2;
 */				
					Base::indices << offset+1-broken*2;
					Base::indices << offset+3-broken*2;
					Base::indices << offset+9;
					Base::indices << offset+7;

				}

				offset += 6;
				
				
			}
			{
				unsigned t = res-1;
				float	x = 1.0f,
						px = Base::width,
						a0 = 1.0f,//(0.3+0.7*(h_field[t]-lowest)*invrange),
						a1 = 1.0f;//(0.3+0.7*(h_field[t]+h_block[t]-lowest)*invrange),
				float d = h_block[t];
				if (!d)
				{
					a0 = a1 = 0;
				}
				Base::vertices << px << h_field[t] << Base::depth*depth;
				Base::vertices.append(color,3) << a0;
					
				Base::vertices << px << h_field[t] << -Base::depth*depth;
				Base::vertices.append(color,3) << a0;
					
				Base::vertices << px << (h_field[t]+d) << Base::depth*depth;
				Base::vertices.append(color,3) << a1;
					
				Base::vertices << px << (h_field[t]+d) << -Base::depth*depth;
				Base::vertices.append(color,3) << a1;
			}
			{
				unsigned t = 0;
				float	x = 0.0f,
						px = -Base::width,
						a0 = (0.0),
						a1 = (0.0);
				float d = h_block[t];
				if (!d)
				{
					a0 = a1 = 0;
				}
				Base::vertices << px << h_field[t] << Base::depth*depth;
				Base::vertices.append(color,3) << a0;
					
				Base::vertices << px << h_field[t] << -Base::depth*depth;
				Base::vertices.append(color,3) << a0;
					
				Base::vertices << px << (h_field[t]+d) << Base::depth*depth;
				Base::vertices.append(color,3) << a1;
					
				Base::vertices << px << (h_field[t]+d) << -Base::depth*depth;
				Base::vertices.append(color,3) << a1;
			}

			
			Base::indices << offset+0;
			Base::indices << offset+1;
			Base::indices << offset+3;
			Base::indices << offset+2;
			
			Base::indices << offset+5;
			Base::indices << offset+4;
			Base::indices << offset+6;
			Base::indices << offset+7;
			
			for (unsigned t = 0; t < res; t++)
			{
				h_field[t] += h_block[t];
				h_block[t] = 0;
			}

		}

	template <class GL, class Font>
		void		StairGraph<GL,Font>::update()
		{
			static bool exported = false;
		
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
		
			if (!data->groups())
			{
				return;
			}
			Base::beginUpdate();
			Base::text_location.SetSize(data->countChannels());
			
			unsigned res = data->resolution();
			if (!res)
				return;
			h_field.Resize(res);
			h_block.Resize(res);
			h_block.Fill(0);
			h_field.Fill(0);
			unsigned cnt = 0;
			float depth = 1;
			for (index_t i = 0; i < data->groups(); i++)
			{
				const Group&group = data->group(i);
				for (index_t j = 0; j < group.channels(); j++)
				{
					const Channel&channel = group.channel(j);
					ASSERT2__(channel.history.GetLength() == res,channel.history.GetLength(),res);
					const float*h = channel.history.pointer();
					for (unsigned t = 0; t < res; t++)
						h_block[t] += h[t];
					
					if (view == Detailed)
					{
						M::TVec3<>	color;
						if (group.channels()>1)
							M::Vec::interpolate(group.color,channel.color,0.25,color);
						else
							M::Vec::copy(group.color,color);
						addRow(color,depth);
					}
					M::Vec::def(Base::text_location[cnt],Base::width,h_field.Last()+h_block.Last(),depth*Base::depth);
					depth *= 0.8;
					cnt++;
				}
				if (view == Simplified)
					addRow(group.color,depth);
				depth *= 0.6;
			}
			if (view == Solid)
			{
				M::TVec3<> color={0,0,0};
				for (index_t i = 0; i < data->groups(); i++)
					M::Vec::add(color,data->group(i).color);
				M::Vec::div(color,data->groups());
				
				addRow(color,1.0f);
			}
			//ASSERT__(indices.Count() == cnt*(res-1)*4);
			
			
			
			highest = 0;
			for (unsigned i = 0; i < res; i++)
				if (h_field[i] > highest)
					highest = h_field[i];
			str_current = h_field.Last();
			float scale = highest?Base::height/(highest*1.25f):1.0f;
			
			for (index_t i = 0; i < Base::vertices.Count()/(Base::dimensions+4); i++)
			{
				Base::vertices[i*(Base::dimensions+4)+1] *= scale;
			}
			
			for (index_t i = 0; i < Base::outlines.field[SimpleGeometry::Lines].Count(); i++)
			{
				Base::outlines.field[SimpleGeometry::Lines][i].position[1] *= scale;
			
			}
			
			Base::shade();
			
			for (index_t i = 0; i < Base::text_location.GetLength(); i++)
				Base::text_location[i].y *= scale;

			
			Base::finishUpdate();
		}
	
	
	
	
	template <class GL, class Font>
		HeightGraph<GL,Font>::HeightGraph(Data*data):Base(data,3),highest(0)
		{
			height = 0.3;
		}
		
		
	template <class GL, class Font>
		void		HeightGraph<GL,Font>::Draw()
		{
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
				
			renderer->enterSubSystem(Base::system);
			
			renderer->useTexture(NULL);
			Base::init();
			Base::renderGeometry();
			
			renderer->render(Base::outlines);
			{
				M::TMatrix4<> scaled;
				Mat::Eye(scaled);
				scaled.x.x = width*1.01;
				scaled.y.y = height*1.01;
				scaled.z.z = depth*1.01;
				renderer->enterSubSystem(scaled);
				renderer->render(frame);
				renderer->exitSubSystem();
			}
			
			C3dGraph::printCaptions<Font>();
			renderer->exitSubSystem();
			
/* 			Textout<Font>*active_textout = TextoutManager<Font>::get();
			
			float indent = active_textout->scaledLength('X');
			
			float x = position.right+indent/2;
			
			if (caption_left)
			{
				float width = 0;
				
				if (view != Solid)
				{
					for (unsigned i = 0; i < data->groups(); i++)
					{
						const Group&group = data->group(i);
						width = vmax(width,active_textout->scaledLength(group.name));
						if (view == Detailed)
						{
							for (unsigned j = 0; j < group.channels(); j++)
								width = vmax(width,active_textout->scaledLength(group.channel(j).name)+indent);
						}
					}
				}
				
				x = position.left-width-indent/2;
			}
			float y = position.bottom;
			
			active_textout->locate(position.left+indent/2,position.top-active_textout->state.y_scale);
			active_textout->SetColor(1,1,1);
			active_textout->Print(name);
			
			
			if (view != Solid)
			{
				//x+=indent;
				for (unsigned i = 0; i < data->groups(); i++)
				{
					const Group&group = data->group(i);
					if (view == Detailed)
					{
						x+=indent;
						for (unsigned j = 0; j < group.channels(); j++)
						{
							const Channel&channel = group.channel(j);
							float color[3];
							_interpolate(group.color,channel.color,0.5,color);
							active_textout->locate(x,y);
							active_textout->SetColor3fv(color);
							y+=active_textout->state.scale_y;
							active_textout->Print(channel.name);
						}
						x-=indent;
					}
					active_textout->locate(x,y);
					active_textout->SetColor3fv(group.color);
					y+=active_textout->state.scale_y;
					active_textout->Print(group.name);
				}
			}
			active_textout->SetColor(1,1,1);
			
			active_textout->locate(position.right-active_textout->scaledLength(str_current)-indent/2, position.top-active_textout->state.y_scale);
			active_textout->Print(str_current);
 */			
		}
	

	template <class GL, class Font>
		void		HeightGraph<GL,Font>::update()
		{
			static bool exported = false;
		
			GL*renderer = Base::getRenderer();
			if (!renderer)
				return;
		
			if (!data->groups())
			{
				return;
			}
			Base::beginUpdate();
			Base::text_location.SetSize(data->countChannels());
			
			unsigned res = data->resolution();
			if (res<2)
				return;
			
			float native_depth = 0;
			for (unsigned i = 0; i < data->groups(); i++)
			{
				const Group&group = data->group(i);
				native_depth += group.channels();
			}
			
			float	scale = Base::depth/native_depth,
					offset = 0.5;
			
			
			highest = 0;
			
			unsigned cnt = 0;
			float depth = 1;
			for (unsigned i = 0; i < data->groups(); i++)
			{
				
				const Group&group = data->group(i);
				if (group.channels())
				{
					unsigned voffset = Base::vertices.Count()/(Base::dimensions+4);
					unsigned row_stride = (res*2+4);
					
					float base_offset = offset;
					
					for (index_t j = 0; j < group.channels(); j++)
					{
						const Channel&channel = group.channel(j);
						ASSERT2__(channel.history.GetLength() == res,channel.history.GetLength(),res);
						const float*h = channel.history.pointer();

						M::TVec3<> color;
						if (group.channels()>1)
							M::Vec::interpolate(group.color,channel.color,0.25,color);
						else
							M::Vec::copy(group.color,color);
						//addRow(color,depth);
						M::Vec::def(Base::text_location[cnt],width,h[res-1],(offset*scale*2-1)*depth+scale/2.0);
						for (unsigned k = 0; k < res; k++)
						{
							if (h[k] > highest)
								highest = h[k];
							float px = (float)k/(float)(res-1);
							Base::vertices << (px*2-1)*width << h[k] << (offset*scale*2-1)*depth-scale/2.0;
							Base::vertices.append(color.v,3)<<px;
							Base::vertices << (px*2-1)*width << h[k] << (offset*scale*2-1)*depth+scale/2.0;
							Base::vertices.append(color.v,3)<<px;
							if (k < res-1)
							{
								/*Base::indices << voffset+j*res*2+k*2 << voffset+(j+1)*res*2+k*2+0;
								Base::indices << voffset+(j+1)*res*2+k*2+1 << voffset+j*res*2+k*2+2;*/
								

								Base::indices << voffset+j*row_stride+k*2 << voffset+(j)*row_stride+k*2+1;
								Base::indices << voffset+(j)*row_stride+k*2+3 << voffset+j*row_stride+k*2+2;
								
								if (j<group.channels()-1)
								{
									Base::indices << voffset+j*row_stride+k*2+1 << voffset+(j+1)*row_stride+k*2;
									Base::indices << voffset+(j+1)*row_stride+k*2+2 << voffset+j*row_stride+k*2+3;
								}
							}
						}
						unsigned vat = Base::vertices.Count()/(Base::dimensions+4);
						Base::vertices << width << h[res-1] << (offset*scale*2-1)*depth-scale/2.0;
						Base::vertices.append(color,3)<<0.9;
						Base::vertices << width << h[res-1] << (offset*scale*2-1)*depth+scale/2.0;
						Base::vertices.append(color,3)<<0.9;
						Base::vertices << width << 0 << (offset*scale*2-1)*depth-scale/2.0;
						Base::vertices.append(color,3)<<0.9;
						Base::vertices << width << 0 << (offset*scale*2-1)*depth+scale/2.0;
						Base::vertices.append(color,3)<<0.9;
						
						Base::indices << vat << vat+1;
						Base::indices << vat+3 << vat+2;
						
						if (j<group.channels()-1)
						{
							Base::indices << vat+3 << vat+1;
							Base::indices << vat+row_stride << vat+2+row_stride;
						}

						if (view == Detailed)
						{
						}
						offset++;
						cnt++;
					}
					
					if (group.channels())
					{
						float end_offset = offset -1.0f;
						unsigned vat = Base::vertices.Count()/(Base::dimensions+4);
						const float*h0 = group.channel(0).history.pointer(),
									*h1 = group.channel(group.channels()-1).history.pointer();
						for (unsigned k = 0; k < res; k++)
						{
							float px = (float)k/(float)(res-1);
							Base::vertices << (px*2-1)*width << h0[k] << (base_offset*scale*2-1)*depth-scale/2.0*depth;
							Base::vertices.append(group.color,3)<<px*0.9;
							Base::vertices << (px*2-1)*width << 0 << (base_offset*scale*2-1)*depth-scale/2.0*depth;
							Base::vertices.append(group.color,3)<<px*0.9;
							
							Base::vertices << (px*2-1)*width << h1[k] << (end_offset*scale*2-1)*depth+scale/2.0*depth;
							Base::vertices.append(group.color,3)<<px*0.9;
							Base::vertices << (px*2-1)*width << 0 << (end_offset*scale*2-1)*depth+scale/2.0*depth;
							Base::vertices.append(group.color,3)<<px*0.9;
							
							if (k < res-1)
							{
								Base::indices << vat+k*4+1 << vat+k*4+0;
								Base::indices << vat+k*4+4 << vat+k*4+5;
								
								Base::indices << vat+k*4+2 << vat+k*4+3;
								Base::indices << vat+k*4+7 << vat+k*4+6;
							}
						}
					}
					
					
					
					//offset +=0.2;
					
					
					if (view == Simplified)
					{}
				}
			}
			if (view == Solid)
			{
				M::TVec3<> color={0,0,0};
				for (index_t i = 0; i < data->groups(); i++)
					M::Vec::add(color,data->group(i).color);
				M::Vec::div(color,data->groups());
				
			}
			str_current = "";
			scale = highest?Base::height/(highest*1.25f):1.0f;
			
			for (index_t i = 0; i < Base::vertices.Count()/(Base::dimensions+4); i++)
			{
				Base::vertices[i*(Base::dimensions+4)+1] *= scale;
			}
			
			for (index_t i = 0; i < Base::outlines.field[SimpleGeometry::Lines].Count(); i++)
			{
				Base::outlines.field[SimpleGeometry::Lines][i].position[1] *= scale;
			
			}
			
			Base::shade();
			
			for (index_t i = 0; i < Base::text_location.GetLength(); i++)
				Base::text_location[i].y *= scale;
			
			
			Base::finishUpdate();
		}
		
		
	template <class GL, class Base>
		bool	RenderGraph<GL,Base>::geometry_defined=false;
		
	template <class GL, class Base>
		SimpleGeometry	RenderGraph<GL,Base>::background;

}
}

#endif
