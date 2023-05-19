#pragma once

/******************************************************************

Fractal-Core for generation of Fractal landscapes.
Use in combination with EveCudaFractalModule.

******************************************************************/

namespace DeltaWorks
{


template <unsigned Exponent>
	Fractal::TemplateSurface<Exponent>::TemplateSurface():DataSurface(Exponent,TemplateMap<Exponent>::vertices)
	{}
	
template <unsigned Exponent>
	Fractal::TemplateSurface<Exponent>::TemplateSurface(Body*super_):DataSurface(Exponent,TemplateMap<Exponent>::vertices)
	{
		super = super_;
	}

template <unsigned Exponent>
	Fractal::TemplateSurface<Exponent>::TemplateSurface(Body*super_,TemplateSurface<Exponent>*parent_,BYTE child_index, const Time::Stamp&time):DataSurface(Exponent,TemplateMap<Exponent>::vertices)
	{
		super = super_;
		parent = parent_;
		this->child_index = child_index;
		referenced = time;
	}
	
	
template <unsigned Exponent>
	Fractal::TemplateSurface<Exponent>::TemplateSurface(Body*super_,
		TemplateSurface<Exponent>*parent_,
		BYTE child_index, 
		const TSurfaceSegmentLink&n0_, 
		const TSurfaceSegmentLink&n1_, 
		const TSurfaceSegmentLink&n2_, 
		const Time::Stamp&time
	):DataSurface(Exponent,TemplateMap<Exponent>::vertices)
	{
		referenced = time;
		super = super_;
		parent = parent_;
		this->child_index = child_index;
		neighbor_link[0] = n0_;
		neighbor_link[1] = n1_;
		neighbor_link[2] = n2_;
		//logfile << "created 0x"<<IntToHex((int)this,8)<<" with super 0x"<<IntToHex((int)super,8)<<nl;
		reinit();
	}
	




	


template <unsigned Exponent> 
	void			Fractal::CorePattern<Exponent>::createCorePattern()
	{
	    BEGIN
	    UINT32*p = core_pattern;

	    for (unsigned y = 3; y < range-1; y++)
	    {
	        for (unsigned x = 1; x < y-2; x++)
	        {
	            (*p++) = MapType::getStaticIndex(x,y);
	            (*p++) = MapType::getStaticIndex(x+1,y);
	            (*p++) = MapType::getStaticIndex(x,y-1);
	            (*p++) = MapType::getStaticIndex(x+1,y);
	            (*p++) = MapType::getStaticIndex(x+1,y-1);
	            (*p++) = MapType::getStaticIndex(x,y-1);
	        }
	        (*p++) = MapType::getStaticIndex(y-2,y);
	        (*p++) = MapType::getStaticIndex(y-1,y);
	        (*p++) = MapType::getStaticIndex(y-2,y-1);
	    }

	    if (p != core_pattern + ARRAYSIZE(core_pattern))
	        FATAL__("bad core-pattern. local: "+IntToStr(p-core_pattern)+" global: "+IntToStr(ARRAYSIZE(core_pattern)));

	    END
	}
	
template <unsigned Exponent>
	void Fractal::IndexPatterns<Exponent>::makePattern(unsigned r0, unsigned r1, unsigned r2)
	{
	    BEGIN
	    (*ocurrent++) = current - pattern;

	    unsigned r[3] = {r0,r1,r2};	//resolution, number of elements along the edge
	    for (BYTE k = 0; k < 3; k++)
	    {
	        unsigned overload = (range-1) / r[k];
			ASSERT2__(overload > 0, range,r[k]);
	        unsigned*outer = Map<Exponent>::instance.border[k],
	                *inner = Map<Exponent>::instance.inner_border[k];

			//begin
			(*current++) = outer[0];
			(*current++) = outer[overload];
			(*current++) = inner[1];
			
			unsigned iat = 1,
					oat = overload;
			
			//end
			if (r[k]>1)
			{
				(*current++) = outer[range-1-overload];
				(*current++) = outer[range-1];
				(*current++) = inner[range-3];


				
				//segments:
				//for (unsigned i = 0; i+1 < r[k]; i++)	//go 4
				//unsigned i = 0;
				while (oat+overload < range)
				{
					unsigned end = oat+overload*2 < range?oat+overload/2:oat+overload-1;
				
					while (iat < end && iat+1<range-2)
					{
						ASSERT__(iat > 0);
						(*current++) = outer[oat];
						(*current++) = inner[iat+1];
						(*current++) = inner[iat];
						iat++;
					}
					if (oat+overload*2 < range)
					{
						(*current++) = outer[oat];
						(*current++) = outer[oat+overload];
						(*current++) = inner[iat];
					}
					oat += overload;
				}
			}
			else
			{
				while (iat+3<range)
				{
					(*current++) = outer[oat];
					(*current++) = inner[iat+1];
					(*current++) = inner[iat];
					iat++;
				}
			}
	    }

	    memcpy(current,CorePattern<Exponent>::core_pattern,sizeof(CorePattern<Exponent>::core_pattern));
	    current+=ARRAYSIZE(CorePattern<Exponent>::core_pattern);
	    END
}
	
	 
template <unsigned Exponent> 
	Fractal::IndexPatterns<Exponent>::IndexPatterns():AbstractIndexPatterns(pattern,pattern_offset,CorePattern<Exponent>::core_pattern,ARRAYSIZE(pattern_offset)-1,Exponent)
	{}
	
template <unsigned Exponent>
	void Fractal::IndexPatterns<Exponent>::create()
	{
		if (created)
			return;
		BEGIN
		created = true;
		createCorePattern();
	    current = pattern;
	    ocurrent = pattern_offset;

		unsigned range0 = range-1;
		while (range0 > 0)
		{
			unsigned range1 = range-1;
			while (range1 > 0)
			{
				unsigned range2 = range-1;
				while (range2 > 0)
				{
					makePattern(range0,range1,range2);
					range2>>=1;
				}
				range1>>=1;
			}
			range0>>=1;
		}
		(*ocurrent++) = current - pattern;

		if (current-pattern > ARRAYSIZE(pattern))
			FATAL__("bad pattern-creation");

		if (ocurrent-pattern_offset > ARRAYSIZE(pattern_offset))
			FATAL__("bad pattern-creation ("+IntToStr(ocurrent-pattern_offset)+">"+IntToStr(ARRAYSIZE(pattern_offset))+")");

		END
	}

template <unsigned Exponent>
	void Fractal::PrimaryIndexPattern<Exponent>::create()
	{
		if (created)
			return;
		BEGIN
		created = true;
		CorePattern<Exponent>::createCorePattern();

		UINT32*current = pattern;

		unsigned	r0 = range-1,
					r1 = range-1,
					r2 = range-1;
				
	    //(*ocurrent++) = current - pattern;

	    unsigned r[3] = {r0,r1,r2};	//resolution, number of elements along the edge
	    for (BYTE k = 0; k < 3; k++)
	    {
	        unsigned overload = (range-1) / r[k];
			ASSERT2__(overload > 0, range,r[k]);
	        unsigned*outer = Map<Exponent>::instance.border[k],
	                *inner = Map<Exponent>::instance.inner_border[k];

			//begin
			(*current++) = outer[0];
			(*current++) = outer[overload];
			(*current++) = inner[1];
			
			unsigned iat = 1,
					oat = overload;
			
			//end
			if (r[k]>1)
			{
				(*current++) = outer[range-1-overload];
				(*current++) = outer[range-1];
				(*current++) = inner[range-3];


				
				//segments:
				//for (unsigned i = 0; i+1 < r[k]; i++)	//go 4
				//unsigned i = 0;
				while (oat+overload < range)
				{
					unsigned end = oat+overload*2 < range?oat+overload/2:oat+overload-1;
				
					while (iat < end && iat+1<range-2)
					{
						ASSERT__(iat > 0);
						(*current++) = outer[oat];
						(*current++) = inner[iat+1];
						(*current++) = inner[iat];
						iat++;
					}
					if (oat+overload*2 < range)
					{
						(*current++) = outer[oat];
						(*current++) = outer[oat+overload];
						(*current++) = inner[iat];
					}
					oat += overload;
				}
			}
			else
			{
				while (iat+3<range)
				{
					(*current++) = outer[oat];
					(*current++) = inner[iat+1];
					(*current++) = inner[iat];
					iat++;
				}
			}
	    }

	    memcpy(current,CorePattern<Exponent>::core_pattern,sizeof(CorePattern<Exponent>::core_pattern));
	    current+=ARRAYSIZE(CorePattern<Exponent>::core_pattern);
	    END

		pattern_length = current - pattern;

		if (current-pattern > ARRAYSIZE(pattern))
			FATAL__("bad pattern-creation");


		END
	}

#if 0
template <unsigned Exponent>
	unsigned	Fractal::ExtIndexPatterns<Exponent>::extVertexCount()
	{
		unsigned result = TemplateMap<Exponent>::vertices;
		for (unsigned g = 1; g <= Exponent; g++)
			result += (1<<(Exponent-g))*3;
		return result;
	}
#endif

template <unsigned Exponent>
	bool	Fractal::ExtIndexPatterns<Exponent>::getVertexLocation(unsigned index, unsigned&generation, BYTE&edge, unsigned&edge_local_index)
	{
		if (index < TemplateMap<Exponent>::vertices)
		{
			generation = 0;
			edge = 0;
			edge_local_index = 0;
			return false;
		}
		index -= TemplateMap<Exponent>::vertices;
		for (unsigned g = 1; g <= Exponent; g++)
		{
			const unsigned edge_vertex_count = (1<<(Exponent-g));

			unsigned edge_cnt = index/edge_vertex_count;
			if (edge_cnt < 3)
			{
				edge = edge_cnt;
				generation = g;
				edge_local_index = index%edge_vertex_count;
				return true;
			}
			index -= edge_vertex_count*3;
		}
		return false;
	}


template <unsigned Exponent>
	unsigned Fractal::ExtIndexPatterns<Exponent>::getBaseVertexIndex(unsigned generation)
	{
		unsigned result = TemplateMap<Exponent>::vertices;
		for (unsigned g = 1; g < generation; g++)
			result += (1<<(Exponent-g))*3;
		return result;
	}

template <unsigned Exponent>
	void Fractal::ExtIndexPatterns<Exponent>::makePattern(unsigned g0, unsigned g1, unsigned g2)
	{
	    BEGIN
	    (*ocurrent++) = current - pattern;

		unsigned g[3] = {g0,g1,g2};
		
	    for (BYTE k = 0; k < 3; k++)
	    {
	        unsigned overload = 1<<g[k];
			//(range-1) / r[k];
			//ASSERT2__(overload > 0, range,r[k]);
	        unsigned*outer = Map<Exponent>::instance.border[k],
	                *inner = Map<Exponent>::instance.inner_border[k];

			const unsigned edge_vertex_count = (1<<(Exponent-g[k]));	//number of vertices, including beginnung but not ending corner vertex
			const unsigned base_vertex_index = getBaseVertexIndex(g[k]);
			//cout << "base for generation "<<g[k]<<" is "<<base_vertex_index<<endl;
			
			#undef GET_OUTER
			#define GET_OUTER(_INDEX_)\
				(\
					!(g[k])?\
						Map<Exponent>::instance.border[k][_INDEX_]\
					:(\
						(_INDEX_)<MapType::range-1?\
						(\
							base_vertex_index+edge_vertex_count*k\
							+(_INDEX_)/overload\
						)\
						:(\
							base_vertex_index+edge_vertex_count*((k+1)%3)\
						)\
					)\
				)
					
					
					
			//begin
			(*current++) = GET_OUTER(0);
			(*current++) = GET_OUTER(overload);
			(*current++) = inner[1];
			
			unsigned iat = 1,
					oat = overload;
			
			//end
			if (g[k]<Exponent)
			{
				(*current++) = GET_OUTER(range-1-overload);
				(*current++) = GET_OUTER(range-1);
				(*current++) = inner[range-3];


				
				//segments:
				//for (unsigned i = 0; i+1 < r[k]; i++)	//go 4
				//unsigned i = 0;
				while (oat+overload < range)
				{
					unsigned end = oat+overload*2 < range?oat+overload/2:oat+overload-1;
				
					while (iat < end && iat+1<range-2)
					{
						ASSERT__(iat > 0);
						(*current++) = GET_OUTER(oat);
						(*current++) = inner[iat+1];
						(*current++) = inner[iat];
						iat++;
					}
					if (oat+overload*2 < range)
					{
						(*current++) = GET_OUTER(oat);
						(*current++) = GET_OUTER(oat+overload);
						(*current++) = inner[iat];
					}
					oat += overload;
				}
			}
			else
			{
				while (iat+3<range)
				{
					(*current++) = GET_OUTER(oat);
					(*current++) = inner[iat+1];
					(*current++) = inner[iat];
					iat++;
				}
			}

	    }

	    memcpy(current,CorePattern<Exponent>::core_pattern,sizeof(CorePattern<Exponent>::core_pattern));
	    current+=ARRAYSIZE(CorePattern<Exponent>::core_pattern);
	    END
	}

	


template <unsigned Exponent>
	void Fractal::ExtIndexPatterns<Exponent>::create()
	{
		if (created)
			return;
		BEGIN
		created = true;
		std::cout << "createCorePattern()"<< std::endl;
		createCorePattern();

		for (unsigned g0 = 0; g0 <= Exponent; g0++)
			for (unsigned g1 = 0; g1 <= Exponent; g1++)
				for (unsigned g2 = 0; g2 <= Exponent; g2++)
					ExtIndexPatterns<Exponent>::makePattern(g0,g1,g2);
		
		(*ocurrent++) = current - pattern;

		if (current-pattern > ARRAYSIZE(pattern))
			FATAL__("bad pattern-creation");

		if (ocurrent-pattern_offset > ARRAYSIZE(pattern_offset))
			FATAL__("bad pattern-creation ("+IntToStr(ocurrent-pattern_offset)+">"+IntToStr(ARRAYSIZE(pattern_offset))+")");

		END
	}




	
template <unsigned Exponent, unsigned ExponentReduction>
	Fractal::TemplateBody<Exponent,ExponentReduction>::TemplateBody(float sector_size, unsigned max_recursion_level):
		Body("[body]",sector_size,Exponent,max_recursion_level,&vbo_vertex_index_field,&triangles,&Map<Exponent>::instance,&Map<Exponent-ExponentReduction>::instance)
		{}

template <unsigned Exponent, unsigned ExponentReduction>
	Fractal::TemplateBody<Exponent,ExponentReduction>::TemplateBody(const String&name, float sector_size, unsigned max_recursion_level):
		Body(name,sector_size,Exponent,max_recursion_level,&vbo_vertex_index_field,&triangles,&Map<Exponent>::instance,&Map<Exponent-ExponentReduction>::instance)
		{}

template <unsigned Exponent, unsigned ExponentReduction>
	Fractal::DataSurface*		Fractal::TemplateBody<Exponent,ExponentReduction>::makeFace()
	{
		return SignalNew(new TemplateSurface<Exponent>());
	}
	

	
	
	
template <unsigned Exponent, unsigned ExponentReduction>
	void		Fractal::TemplateBody<Exponent,ExponentReduction>::assembleMaps(unsigned lookup_reduction)
	{
		if (triangles.length())
			return;
		
		
		vbo_vertex_index_field.SetSize(ReducedMap::vertices);
		unsigned at_index = 0;
		
		for (unsigned i = 0; i < FullMap::vertices; i++)
		{
			//cout << " "<<i;
			const TMapVertex&v = Map<Exponent>::instance.info[i];
			
			if (!(v.x%(1<<ExponentReduction)) && !(v.y%(1<<ExponentReduction)))
			{
				ASSERT2__(at_index < vbo_vertex_index_field.length(),at_index,vbo_vertex_index_field.length());
				vbo_vertex_index_field[at_index++] = i;
			}
		}
		ASSERT__(at_index == ReducedMap::vertices);
		
		Vector<unsigned>	triangle_indices;
		unsigned step = 1<<lookup_reduction;
		for (unsigned y = step; y < FullMap::range; y+=step)
			for (unsigned x = 0; x < y; x+=step)
			{
				triangle_indices << FullMap::getIndex(x,y);
				triangle_indices << FullMap::getIndex(x+step,y);
				triangle_indices << FullMap::getIndex(x,y-step);
				if (x+step < y)
				{
					triangle_indices << FullMap::getIndex(x,y-step);
					triangle_indices << FullMap::getIndex(x+step,y);
					triangle_indices << FullMap::getIndex(x+step,y-step);
				}
			}
		triangle_indices.copyToArray(triangles);
	}
	
	

template <unsigned Exponent> 
	UINT32			Fractal::PrimaryIndexPattern<Exponent>::pattern[range*range*3];
template <unsigned Exponent> 
	UINT32			Fractal::PrimaryIndexPattern<Exponent>::pattern_length=0;
	
template <unsigned Exponent> 
	UINT32			Fractal::IndexPatterns<Exponent>::pattern[(Exponent+1)*(Exponent+1)*(Exponent+1)*range*range*3];
template <unsigned Exponent> 
	UINT32			Fractal::IndexPatterns<Exponent>::pattern_offset[(Exponent+1)*(Exponent+1)*(Exponent+1)+1];
template <unsigned Exponent> 
	UINT32			Fractal::CorePattern<Exponent>::core_pattern[(range-4)*(range-4)*3];
template <unsigned Exponent> 
	UINT32			*Fractal::IndexPatterns<Exponent>::current = Fractal::IndexPatterns<Exponent>::pattern;
template <unsigned Exponent> 
	UINT32			*Fractal::IndexPatterns<Exponent>::ocurrent = Fractal::IndexPatterns<Exponent>::pattern_offset;
					
/*
template <unsigned Exponent> 
	Fractal::TemplateMap<Exponent>	Fractal::IndexPatterns<Exponent>::map;
*/

template <unsigned Exponent> 
	bool			Fractal::IndexPatterns<Exponent>::created(false);
template <unsigned Exponent> 
	bool			Fractal::PrimaryIndexPattern<Exponent>::created(false);

/*	
template <unsigned Exponent, unsigned ExponentReduction>
	Fractal::TemplateMap<Exponent>		Fractal::TemplateBody<Exponent,ExponentReduction>::full_map;
template <unsigned Exponent, unsigned ExponentReduction>
	Fractal::TemplateMap<Exponent-ExponentReduction>	Fractal::TemplateBody<Exponent,ExponentReduction>::reduced_map;
*/
template <unsigned Exponent, unsigned ExponentReduction>
	Array<unsigned>				Fractal::TemplateBody<Exponent,ExponentReduction>::vbo_vertex_index_field;
template <unsigned Exponent, unsigned ExponentReduction>
	Array<unsigned>				Fractal::TemplateBody<Exponent,ExponentReduction>::triangles;








}
