#ifndef cgsTplH
#define cgsTplH




#define CGS_POINT		{}//{if (CGS::log) (*CGS::log)<<shortenFile(__FILE__)<<"("<<__LINE__<<") <"<<__func__<<">"<<nl; }   //define void to disable logging
#define CGS_MSG(msg)	{}//{if (CGS::log) (*CGS::log)<<shortenFile(__FILE__)<<"("<<__LINE__<<") <"<<__func__<<">: "<<String(msg)<<nl; }   //define void to disable logging

/*
template <class C> String vstructArrayCompare(const String&name,unsigned cnt0,C*pntr0,unsigned cnt1,C*pntr1, const String&intend="")
{
	String rs;
	if (cnt0 != cnt1)
		rs+="\n"+intend+name+": "+String(cnt0)+" != "+String(cnt1);
	if ((pntr0 == pntr1) && pntr0)
		rs+="\n"+intend+name+": pointers equal";
	else
		for (index_t i = 0; i < cnt0 && i < cnt1; i++)
			rs+="\n "+intend+"("+String(i)+") "+pntr0[i].difference(pntr1[i],"	"+intend);
	return rs;
}*/



// template <class Def>
	// void			Tracks<Def>::adoptData(Tracks<Def>&other)
	// {
		// if (this == &other)
			// return;
		// domain = other.domain;
		// name_field.adoptData(other.name_field);
		// entry_field.adoptData(other.entry_field);
		// rotation.adoptData(other.rotation);
		// flags = other.flags;
	// }



template <class Def>
template <class Def0> MaterialData<Def>& MaterialData<Def>::operator=(const MaterialData<Def0>&origin)
{
	coord_layers = origin.coord_layers;
	object_field = origin.object_field;
	return *this;
}


template <class Def> void MaterialData<Def>::postCopyLink(Geometry<Def>*domain)
{
	for (index_t i = 0; i < object_field.GetLength(); i++)
	{
		object_field[i].target = domain->lookupObject(object_field[i].tname);
		ASSERT_NOT_NULL__(object_field[i].target);
	}
}

template <class Def> void MaterialA<Def>::storeTargetNames()
{
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		data.object_field[i].tname = data.object_field[i].target->name;
		
	for (index_t i = 0; i < info.layer_field.GetLength(); i++)
		if (info.layer_field[i].source)
			info.layer_field[i].source_name = info.layer_field[i].source->name;
}

template <class Def>	template <class C>
	void	MaterialA<Def>::scale(const C&factor)
	{
		for (index_t i = 0; i < data.object_field.GetLength(); i++)
			data.object_field[i].vpool.scale(factor);
	}



template <class Def> void MaterialA<Def>::adoptData(MaterialA<Def>&other)
{
	if (this == &other)
		return;
	info.adoptData(other.info);
	data.adoptData(other.data);
	name.adoptData(other.name);
}

template <class Def>
count_t	MaterialA<Def>::countTriangles()	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].countTriangles();
	return rs;
}

template <class Def>
count_t	MaterialA<Def>::countQuads()	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].countQuads();
	return rs;
}


template <class Def>
count_t	MaterialA<Def>::countFaces()	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].countFaces();
	return rs;
}

template <class Def>
count_t	MaterialA<Def>::countTriangles(unsigned detail_layer)	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].countTriangles(detail_layer);
	return rs;
}

template <class Def>
count_t	MaterialA<Def>::countQuads(unsigned detail_layer)	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].countQuads(detail_layer);
	return rs;
}


template <class Def>
count_t	MaterialA<Def>::countFaces(unsigned detail_layer)	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].countFaces(detail_layer);
	return rs;
}


template <class Def>
count_t	MaterialA<Def>::countVertices()	const
{
	count_t rs = 0;
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs += data.object_field[i].vpool.vcnt;
	return rs;
}



template <class Def>
template <class Def0> MaterialA<Def>& MaterialA<Def>::operator=(const MaterialA<Def0>&origin)
{
	data = origin.data;
	info = origin.info;
	return *this;
}

template <class Def> void	MaterialA<Def>::postCopyLink(Geometry<Def>*domain, TextureResource*resource)
{
	data.postCopyLink(domain);
	info.postCopyLink(resource);
}

template <class Def> String MaterialA<Def>::ToString(const String&intend)	const
{
	String rs = "Material\n"+intend;
	rs+="Layers: "+String(info.layer_field.GetLength())+"\n"+intend;
	rs+="Objects: "+String(data.object_field.GetLength())+"\n";
	for (index_t i = 0; i < data.object_field.GetLength(); i++)
		rs+=intend+" ("+String(i)+") "+data.object_field[i].ToString(intend+"	")+"\n";
	return rs;
}



/*
template <class Def> String MaterialA<Def>::difference(MaterialA&other, const String&intend)
{
	String rs;
	if (info.layers != other.info.layers)
		rs+="\n"+intend+"info.layers "+String(info.layers)+" != "+String(other.info.layers);
	if ((info.layer == other.info.layer) && info.layer)
		rs+="\n"+intend+"info.layers: pointers equal";
	if (!(info == other.info))
		rs+="\n"+intend+"info reports difference";
	rs+=vstructArrayCompare("objects",objects,object,other.objects,other.object,intend+" ");
	return rs;
}*/

template <class Def> void MaterialA<Def>::linkTextures(TextureResource*resource)
{
	if (!resource)
		FATAL__("passing NULL is invalid here");
	if (!this)
		FATAL__("THIS MISSING");
	for (index_t i = 0; i < info.layer_field.GetLength(); i++)
	{
		TextureA*tex = resource->retrieve(info.layer_field[i].source_name);
		info.layer_field[i].source = tex;
		info.layer_field[i].cube_map = info.layer_field[i].source && info.layer_field[i].source->face_field.GetLength() == 6;
	}
}

template <class Def> void MaterialA<Def>::SetSize(count_t layer_count, count_t object_count)
{
	info.layer_field.SetSize(layer_count);
	data.SetSize(object_count);
}


template <class Def> VertexContainerA<Def>::VertexContainerA():vcnt(0),vlyr(0),vflags(0) {}


template <class Def> template <class C>
	void		VertexContainerA<Def>::scale(const C&factor)
	{
		UINT32 stride = vsize();
		typename Def::FloatType*at = vdata.pointer();
		Concurrency::parallel_for(UINT32(0),vcnt,[at,stride,factor](UINT32 i)
		{
			M::Vec::mult(M::Vec::ref3(at+i*stride),factor);
		});
	}



template <class Def> bool VertexContainerA<Def>::operator>(const VertexContainerA<Def>&other) const
{
	equal = false;
	if (vertex_crc > other.vertex_crc)
		return true;
	else
		if (vertex_crc < other.vertex_crc)
			return false;

	UINT32	vlen0 = VSIZE(vlyr,vflags)*vcnt,
			vlen1 = VSIZE(other.vlyr,other.vflags)*other.vcnt;
	if (vlen0 > vlen1)
		return true;
	else
		if (vlen0 < vlen1)
			return false;
	#ifndef FUZZY_COMPARE
	char rs = fastCompare(vdata.pointer(),other.vdata.pointer(),sizeof(typename Def::FloatType)*vlen0);
	if (rs > 0)
		return true;
	if (rs < 0)
		return false;
	#endif

	equal = true;
	return false;
}

template <class Def> bool VertexContainerA<Def>::operator<(const VertexContainerA<Def>&other) const
{
	return !equal;
}

template <class Def>
template <class Def0> VertexContainerA<Def>&	VertexContainerA<Def>::operator=(const VertexContainerA<Def0>&origin)
{
	vcnt = origin.vcnt;
	vlyr = origin.vlyr;
	vflags = origin.vflags;
	vdata = origin.vdata;
	return *this;
}





template <class Def> void VertexContainerA<Def>::stretch(UINT16 layers, UINT32 flags)
{
	if (layers == 0xFFFF)
		layers = vlyr;
	unsigned	step0 = VSIZE(vlyr,vflags),
				step1 = VSIZE(layers,flags);
	Ctr::Array<typename Def::FloatType> new_field(vcnt*step1);

	typename Def::FloatType	*from = vdata.pointer(),
							*to = new_field.pointer();
	unsigned	lyr = vmin(vlyr,layers);
	bool		tan = !!(vflags&HasTangentFlag) && !!(flags&HasTangentFlag),
				norm = !!(vflags&HasNormalFlag) && !!(flags&HasNormalFlag),
				col = !!(vflags&HasColorFlag) && !!(flags&HasColorFlag);
	
	for (UINT32 i = 0; i < vcnt; i++)
	{
		copy3(from,to);
		to += 3;
		from += 3;
		if (norm)
		{
			copy3(from,to);
			from+=3;
			to+=3;
		}
		elif (flags&HasNormalFlag)
			to += 3;
		if (tan)
		{
			copy3(from,to);
			from+=3;
			to+=3;
		}
		elif (flags&HasTangentFlag)
			to+=3;
		if (col)
		{
			copy4(from,to);
			from+=4;
			to+=4;
		}
		elif (flags&HasColorFlag)
			to+=4;
		M::Vec::copyD(from,to,lyr*2);
		from+=vlyr*2;
		to+=layers*2;
	}
	vdata.adoptData(new_field);
	vlyr = layers;
	vflags = flags;
}


template <class Def> void VertexContainerA<Def>::SetSize(count_t vertex_count, UINT16 layers, UINT32 flags)
{
	vdata.SetSize(vertex_count * VSIZE(layers,flags));
	vcnt = UINT32(vertex_count);
	vlyr = UINT32(layers);
	vflags = flags;
}

template <class Def> void VertexContainerA<Def>::adoptData(VertexContainerA<Def>&other)
{
	if (this == &other)
		return;
	vdata.adoptData(other.vdata);
	vcnt = other.vcnt;
	vlyr = other.vlyr;
	vflags = other.vflags;
}

template <class Def> void VertexContainerA<Def>::updateCRC()
{
	vertex_crc = CRC32::Sequence().Append(vdata.pointer(),vdata.GetContentSize()).Finish();
		//getChecksum(vdata);
}

template <class Def> unsigned VertexContainerA<Def>::vsize() const
{
	return VSIZE(vlyr,vflags);
}

template <class Def> count_t VertexContainerA<Def>::vlen() const
{
	return vdata.GetLength();
}






template <class Def > bool IndexContainerA<Def>::operator>(const IndexContainerA<Def>&other) const
{
	equal = false;
	if (index_crc > other.index_crc)
		return true;
	else
		if (index_crc < other.index_crc)
			return false;
	if (idata.GetLength() > other.idata.GetLength())
		return true;
	else
		if (idata.GetLength() < other.idata.GetLength())
			return false;
	if (triangles > other.triangles)
		return true;
	else
		if (triangles < other.triangles)
			return false;
	if (quads < other.quads)
		return true;
	else
		if (quads < other.quads)
			return false;
	#ifndef FUZZY_COMPARE
	char rs = fastCompare(idata,other.idata,idata.GetContentSize());
	if (rs > 0)
		return true;
	if (rs < 0)
		return false;
	#endif
	equal = true;
	return false;
}

template <class Def> bool IndexContainerA<Def>::operator<(const IndexContainerA<Def>&other) const
{
	return !equal;
}


template <class Def> void IndexContainerA<Def>::SetSize(UINT32 triangles_, UINT32 quads_)
{
	idata.SetSize(triangles_*3+quads_*4);
	triangles = triangles_;
	quads = quads_;
}

template <class Def>
template <class Def0> IndexContainerA<Def>& IndexContainerA<Def>::operator=(const IndexContainerA<Def0>&origin)
{
	idata = origin.idata;
	triangles = origin.triangles;
	quads = origin.quads;
	index_crc = origin.index_crc;
	return *this;
}

template <class Def> void IndexContainerA<Def>::adoptData(IndexContainerA<Def>&other)
{
	if (this == &other)
		return;
	idata.adoptData(other.idata);
	triangles = other.triangles;
	quads = other.quads;
	other.triangles = 0;
	other.quads = 0;
	index_crc = other.index_crc;
}

template <class Def> count_t	IndexContainerA<Def>::countFaces()					const
{
	return triangles+quads;
}

template <class Def> count_t	IndexContainerA<Def>::countTriangles()				const
{
	return triangles+quads*2;
}


template <class Def> count_t	IndexContainerA<Def>::countQuads()					const
{
	return quads;
}

/*
template <class Def> String IndexContainer<Def>::difference(SubGeometryChunk&other, const String&intend)
{
	String rs;
	#define COMPARE(token) if (token != other.token) \
							rs+="\n"+intend+#token+" "+String(token)+" != "+String(other.token)
	COMPARE(vcnt);
	COMPARE(vlyr);
	COMPARE(vbnd);
	COMPARE(icnt);
	COMPARE(ecnt);
	COMPARE(vertex_crc);
	COMPARE(index_crc);
	
	#undef COMPARE
	#define COMPARE(token, elements) {\
		unsigned difference = 0;\
		if ((token == other.token) && token)\
			rs+="\n"+intend+#token+": pointers equal";\
		else\
		{\
			for (index_t i = 0; i < element_field.GetLength(); i++)\
				if (token[i] != other.token[i])\
				{\
					if (!difference)\
						rs+="\n"+intend+" ("+String(i)+") {";\
					if (difference < 5)\
						rs+=FloatToStr(token[i])+" != "+FloatToStr(other.token[i])+", ";\
					difference++;\
				}\
			if (difference >= 5)\
				rs+="...";\
			if (difference)\
				rs+="}";\
		}}

	unsigned cnt = vmin(vcnt*(3+vbnd+2*vlyr),other.vcnt*(3+other.vbnd+2*other.vlyr));
	COMPARE(vdata,cnt);
	COMPARE(idata,vmin(icnt,other.icnt));
	#undef COMPARE
	return rs;
}*/



template <class Def> RenderObjectA<Def>::RenderObjectA():target(NULL),detail(0),particle_config(0)
{}


template <class Def> template <class C>
	void	RenderObjectA<Def>::scale(const C&factor)
	{
		vpool.scale(factor);
	}


template <class Def> String RenderObjectA<Def>::ToString(const String&intend)	const
{
	String result = "Object (Vertices: "+String(vpool.vcnt)+"; Detail: "+String(detail)+"; ";
	result += "Indices("+String(detail)+"): "+String(ipool.idata.GetLength())+"; ";

	return result+")";
}

template <class Def>
	void		RenderObjectA<Def>::invert()
	{
		//for (index_t i = 0; i < detail_layer_field.GetLength(); i++)
		{
			IndexContainerA<Def>&chunk = ipool;
			typename Def::IndexType*p = chunk.idata.pointer();
			for (UINT32 j = 0; j < chunk.triangles; j++)
			{
				swp(p[1],p[2]);
				p+=3;
			}
			for (UINT32 j = 0; j < chunk.quads; j++)
			{
				swp(p[0],p[3]);
				swp(p[1],p[2]);
				p+=4;
			}
			ASSERT_CONCLUSION(chunk.idata,p);
		}
	}


template <class Def> count_t RenderObjectA<Def>::countFaces()	const
{
	count_t rs = 0;
	//for (index_t j = 0; j < detail_layer_field.GetLength(); j++)
		rs+=ipool.countFaces();
	return rs;
}

template <class Def> count_t RenderObjectA<Def>::countTriangles()	const
{
	count_t rs = 0;
	//for (index_t j = 0; j < detail_layer_field.GetLength(); j++)
		rs += ipool.countTriangles();
	return rs;
}

template <class Def> count_t RenderObjectA<Def>::countQuads()	const
{
	count_t rs = 0;
	//for (index_t j = 0; j < detail_layer_field.GetLength(); j++)
		rs += ipool.countQuads();
	return rs;
}


template <class Def> count_t RenderObjectA<Def>::countFaces(unsigned detail_layer)	const
{
	if (detail_layer == detail)
		return ipool.countFaces();
	return 0;
}

template <class Def> count_t RenderObjectA<Def>::countTriangles(unsigned detail_layer)	const
{
	if (detail_layer == detail)
		return ipool.countTriangles();
	return 0;
}

template <class Def> count_t RenderObjectA<Def>::countQuads(unsigned detail_layer)	const
{
	if (detail_layer == detail)
		return ipool.countQuads();
	return 0;
}


template <class Def> bool RenderObjectA<Def>::operator>(const RenderObjectA<Def>&other) const
{
	if (vpool>other.vpool)
		return true;
	if (vpool<other.vpool)
	{
		equal = false;
		return false;
	}
	if (detail > other.detail)
		return true;
	if (detail < other.detail)
	{
		equal = false;
		return false;
	}
	{
		if (ipool > ipool)
			return true;
		if (ipool < ipool)
		{
			equal = false;
			return false;
		}
	}
	equal = true;
	return false;
}

template <class Def> bool RenderObjectA<Def>::operator<(const RenderObjectA<Def>&other) const
{
	return !equal;
}

template <class Def> void RenderObjectA<Def>::adoptData(RenderObjectA<Def>&other)
{
	if (this == &other)
		return;
	ipool.adoptData(other.ipool);
	vpool.adoptData(other.vpool);
	particle_field.adoptData(other.particle_field);
	particle_config = other.particle_config;
	target = other.target;
	tname = other.tname;
	detail = other.detail;
}


template <class Def> count_t RenderObjectA<Def>::countIndices()	const
{
	count_t result(0);
		result+=ipool.idata.GetLength();
	return result;
}

template <class Def>
template <class Def0> RenderObjectA<Def>& RenderObjectA<Def>::operator=(const RenderObjectA<Def0>&origin)
{
	ipool = origin.ipool;
	detail = origin.detail;
	vpool = origin.vpool;
	tname = origin.tname;
	target = NULL;
	return *this;
}


template <class Def> bool RenderObjectA<Def>::validIndices()
{
	for (index_t i = 0; i < ipool.idata.GetLength(); i++)
		if (ipool.idata[i] >= vpool.vcnt)
			return false;
	return ipool.triangles*3+ipool.quads*4 == ipool.idata.GetLength();
}


template <class Def> bool MaterialData<Def>::operator>(const MaterialData<Def>&other) const
{
	equal = false;
	if (object_field.GetLength() > other.object_field.GetLength())
		return true;
	if (object_field.GetLength() < other.object_field.GetLength())
	{
		equal = false;
		return false;
	}
	for (index_t i = 0; i < object_field.GetLength(); i++)
	{
		if (object_field[i].vpool > other.object_field[i].vpool)
			return true;
		if (object_field[i].vpool < other.object_field[i].vpool)
		{
			equal = false;
			return false;
		}
		if (object_field[i].detail > other.object_field[i].detail)
			return true;
		if (object_field[i].detail < other.object_field[i].detail)
		{
			equal = false;
			return false;
		}
		//for (index_t j = 0; j < object_field[i].detail_layer_field.GetLength(); j++)
		{
			if (object_field[i].ipool > other.object_field[i].ipool)
				return true;
			if (object_field[i].ipool < other.object_field[i].ipool)
				return false;
		}
	}
	equal = true;
	return false;
}

template <class Def> void MaterialData<Def>::adoptData(MaterialData<Def>&other)
{
	if (this == &other)
		return;
	object_field.adoptData(other.object_field);
	coord_layers = other.coord_layers;
}

template <class Def> bool MaterialData<Def>::operator<(const MaterialData<Def>&other) const
{
	return !equal;
}





/*
template <class Def> String RenderObjectA<Def>::difference(RenderObjectA&other, const String&intend)
{
	return vstructArrayCompare("chunks",chunks,chunk,other.chunks,other.chunk,intend+" ");
}*/

template <typename T>
	System<T>::System(Callback*callback_):callback(callback_)
	{
		loadIdentity(false);
	}
	
template <typename T>
	System<T>::System(const System<T>&other):matrix(other.matrix),callback(NULL)
	{}
	
template <typename T> template <typename T2>
	System<T>::System(const System<T2>&other):callback(NULL)
	{
		M::Mat::copy(other.matrix,matrix);
	}
	
template <typename T>
	System<T>&				System<T>::operator=(const System<T>&other)
	{
		matrix = other.matrix;
		return *this;
	}
	
template <typename T> template <typename T2>
	System<T>&				System<T>::operator=(const System<T2>&other)
	{
		M::Mat::copy(other.matrix,matrix);
		return *this;
	}
	
	
template <typename T>
	inline void	System<T>::loadIdentity(bool do_callback)
	{
		M::Mat::Eye(matrix);
		if (do_callback && callback)
			callback->onSystemChange();
	}
	
template <typename T>
	inline void	System<T>::setPosition(const T&x, const T&y, const T&z)
	{
		matrix.w.x = x;
		matrix.w.y = y;
		matrix.w.z = z;
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T>
	inline void	System<T>::setPositionNoCallback(const T&x, const T&y, const T&z)
	{
		matrix.w.x = x;
		matrix.w.y = y;
		matrix.w.z = z;
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::setPosition(const M::TVec3<T2>&p)
	{
		M::Vec::copy(p,matrix.w.xyz);
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::setPositionNoCallback(const M::TVec3<T2>&p)
	{
		M::Vec::copy(p,matrix.w.xyz);
	}
	
template <typename T>
	inline void	System<T>::moveTo(const T&x, const T&y, const T&z)
	{
		M::Vec::def(matrix.w.xyz,x,y,z);
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T>
	inline void	System<T>::moveToNoCallback(const T&x, const T&y, const T&z)
	{
		M::Vec::def(matrix.w.xyz,x,y,z);
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::moveTo(const M::TVec3<T2>&p)
	{
		M::Vec::copy(p,matrix.w.xyz);
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::moveToNoCallback(const M::TVec3<T2>&p)
	{
		M::Vec::copy(p,matrix.w.xyz);
	}
	
	
	
template <typename T>			
	inline void	System<T>::translate(const T&x, const T&y, const T&z)
	{
		matrix[12] += x;
		matrix[13] += y;
		matrix[14] += z;
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T>
	inline void	System<T>::translateBy(const T&x, const T&y, const T&z)
	{
		matrix[12] += x;
		matrix[13] += y;
		matrix[14] += z;
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::translate(const M::TVec3<T2>&vector)
	{
		M::Vec::add(matrix.w.xyz,vector);
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::translate(const M::TVec3<T2>&vector, const T&factor)
	{
		M::Vec::mad(matrix.w.xyz,vector,factor);
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T>
	void	System<T>::rotate(const T&angle, const T&axis_x,const T&axis_y, const T&axis_z)
	{
		M::TMatrix3<T>	rotation_matrix;

		M::Mat::BuildRotationMatrix(angle,axis_x,axis_y,axis_z,rotation_matrix);

		M::Mat::Mult(rotation_matrix,matrix.x.xyz);
		M::Mat::Mult(rotation_matrix,matrix.y.xyz);
		M::Mat::Mult(rotation_matrix,matrix.z.xyz);
		
		if (callback)
			callback->onSystemChange();
	}
	
template <typename T> template <typename T2>
	void	System<T>::rotate(const T&angle, const M::TVec3<T2>&axis)
	{
		M::TMatrix3<T>	rotation_matrix;
		M::Mat::BuildRotationMatrix(angle,axis,rotation_matrix);

		M::Mat::Mult(rotation_matrix,matrix.x.xyz);
		M::Mat::Mult(rotation_matrix,matrix.y.xyz);
		M::Mat::Mult(rotation_matrix,matrix.z.xyz);
	
		if (callback)
			callback->onSystemChange();
	}

template <typename T>			
	inline void	System<T>::translateNoCallback(const T&x, const T&y, const T&z)
	{
		matrix.w.x += x;
		matrix.w.y += y;
		matrix.w.z += z;
	}
	
	
template <typename T> template <typename T2>
	inline void	System<T>::translateNoCallback(const M::TVec3<T2>&vector)
	{
		M::Vec::add(matrix.w.xyz,vector);
	}
	
template <typename T> template <typename T2>
	inline void	System<T>::translateNoCallback(const M::TVec3<T2>&vector, const T&factor)
	{
		M::Vec::mad(matrix.w.xyz,vector,factor);
	}
	
template <typename T>
	void	System<T>::rotateNoCallback(const T&angle, const T&axis_x,const T&axis_y, const T&axis_z)
	{
		M::TMatrix3<T>	rotation_matrix;

		M::Mat::BuildRotationMatrix(angle,axis_x,axis_y,axis_z,rotation_matrix);

		M::Mat::Mult(rotation_matrix,matrix.x.xyz);
		M::Mat::Mult(rotation_matrix,matrix.y.xyz);
		M::Mat::Mult(rotation_matrix,matrix.z.xyz);
	}
	
template <typename T> template <typename T2>
	void	System<T>::rotateNoCallback(const T&angle, const M::TVec3<T2>&axis)
	{
		M::TMatrix3<T>	rotation_matrix;
		M::Mat::BuildRotationMatrix(angle,axis,rotation_matrix);

		M::Mat::Mult(rotation_matrix,matrix.x.xyz);
		M::Mat::Mult(rotation_matrix,matrix.y.xyz);
		M::Mat::Mult(rotation_matrix,matrix.z.xyz);
	}


	
template <class Def>
	Iterator<Def>::Iterator(Geometry<Def>*target):super(target),context(&target->object_field),current(target->object_field.pointer())
	{}
template <class Def>
	Iterator<Def>::Iterator(Geometry<Def>&target):super(&target),context(&target.object_field),current(target.object_field.pointer())
	{}

template <class Def>	
	SubGeometryA<Def>&		Iterator<Def>::operator*()		const
	{
		return *current;
	}
	
template <class Def>
	SubGeometryA<Def>*		Iterator<Def>::operator->()	const
	{
		return current;
	}
	
template <class Def>	
	bool					Iterator<Def>::operator++()
	{
		if (!current)
			return false;
		//check inner content (walk up by one)
		if (current->child_field.GetLength())
		{
			context_stack.append(context);
			entry_stack.append(current);
			context = &current->child_field;
			current = context->pointer();
			return true;
		}
		current++;
		//check termination (walk down all the way):
		while (context && context->concludedBy(current))
		{
			context = context_stack.drop(context_stack.Count()-1);
			current = entry_stack.drop(entry_stack.Count()-1);
			if (current)
				current++;
		}
		return current != NULL;
		
		
#if 0
		current++;
		//check termination (walk down by one):
		if (context && context->concludedBy(current))
		{
			context = context_stack.drop(context_stack.Count()-1);
			current = entry_stack.drop(entry_stack.Count()-1);
			return current != NULL;
		}
		if (!context)
			return false;
		
		//check inner content (walk up all the way):
		while (current->child_field.GetLength())
		{
			context_stack.append(context);
			entry_stack.append(current);
			context = &current->child_field;
			current = context->pointer();
		}
		return true;
#endif /*0*/
	}
	
template <class Def>
	bool				Iterator<Def>::operator++(int)
	{
		return operator++();
	}
	
template <class Def>
	Iterator<Def>::operator bool()	const
	{
		return current != NULL;
	}

	

template <class Def> SubGeometryA<Def>::SubGeometryA():system_link(&path),name(0)
{
	meta.flags = 0;
	meta.radius = 0;
	meta.volume = 0;
	meta.density = 1;
	meta.shortest_edge_length = 0.1f;
	M::Mat::Eye(meta.system);
}


template <class Def> void SubGeometryA<Def>::clear()
{
	child_field.free();
	phHull.clear();
	accelerator_field.free();
	mounting_field.free();
	wheel_field.free();
	tracks_field.free();
	vs_hull_field.free();
	meta.radius = 0;
	meta.volume = 0;
	meta.density = 1;
	meta.shortest_edge_length = 0.1f;
	meta.flags = 0;
	M::Mat::Eye(meta.system);
	system_link = &path;
}

template <class Def>
	void	SubGeometryA<Def>::clearFlags(UINT32 clear_mask, bool geometries, bool wheels, bool accelerators, bool constructs)
	{
		UINT32 mod = (0xFFFFFFFF^clear_mask);
		if (geometries)
			meta.flags &= mod;
		if (wheels)
			for (index_t i = 0; i < wheel_field.GetLength(); i++)
				wheel_field[i].flags &= mod;
		if (accelerators)
			for (index_t i = 0; i < accelerator_field.GetLength(); i++)
				accelerator_field[i].flags &= mod;
		if (constructs)
			for (index_t i = 0; i < tracks_field.GetLength(); i++)
				tracks_field[i].flags &= mod;
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].clearFlags(clear_mask,geometries,wheels,accelerators,constructs);
	}

template <class Def>
	void	SubGeometryA<Def>::flagAnimationTargets(UINT32 flag)
	{
		for (index_t i = 0; i < wheel_field.GetLength(); i++)
		{
			{
				AnimatorA<Def>&a = wheel_field[i].rotation;
				for (index_t j = 0; j < a.obj_trace_field.GetLength(); j++)
					a.obj_trace_field[j].target->meta.flags |= flag;
				for (index_t j = 0; j < a.whl_trace_field.GetLength(); j++)
					a.whl_trace_field[j].target->flags |= flag;
				for (index_t j = 0; j < a.acc_trace_field.GetLength(); j++)
					a.acc_trace_field[j].target->flags |= flag;
			}
			{
				AnimatorA<Def>&a = wheel_field[i].suspension;
				for (index_t j = 0; j < a.obj_trace_field.GetLength(); j++)
					a.obj_trace_field[j].target->meta.flags |= flag;
				for (index_t j = 0; j < a.whl_trace_field.GetLength(); j++)
					a.whl_trace_field[j].target->flags |= flag;
				for (index_t j = 0; j < a.acc_trace_field.GetLength(); j++)
					a.acc_trace_field[j].target->flags |= flag;
			}
		}
		/*
		for (index_t i = 0; i < tracks_field.GetLength(); i++)
		{
			{

				AnimatorA<Def>&a = tracks_field[i].rotation;
				for (index_t j = 0; j < a.obj_trace_field.GetLength(); j++)
					a.obj_trace_field[j].target->meta.flags |= flag;
				for (index_t j = 0; j < a.whl_trace_field.GetLength(); j++)
					a.whl_trace_field[j].target->flags |= flag;
				for (index_t j = 0; j < a.acc_trace_field.GetLength(); j++)
					a.acc_trace_field[j].target->flags |= flag;
			}
		}*/
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].flagAnimationTargets(flag);
	}

template <class Def>
	void	SubGeometryA<Def>::walkChildrenRec(void (*childCallback)(SubGeometryA<Def>*))
	{
		childCallback(this);
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].walkChildrenRec(childCallback);
	}

template <class Def>
	void	SubGeometryA<Def>::walkChildrenRec(SubGeometryCallback<Def>*callback)
	{
		callback->geometryCallback(this);
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].walkChildrenRec(callback);
	}
	

template <class Def>
	void	SubGeometryA<Def>::createVisualMapsRec(unsigned detail, unsigned max_depth)
	{
		if (detail < vs_hull_field.Count() && !vs_hull_field[detail].getMap())
		{
			typedef Math::Mesh<VsDef>	VsMesh;
			VsMesh&obj = vs_hull_field[detail];
			obj.buildMap(O_ALL,max_depth);
		
			for (index_t i = 0; i < obj.vertex_field.GetLength(); i++)
				M::Vec::clear(obj.vertex_field[i].normal);
			for (index_t i = 0; i < obj.triangle_field.GetLength(); i++)
			{
				typename VsMesh::Triangle & t = obj.triangle_field[i];
				_oTriangleNormal(t.v0->position,t.v1->position,t.v2->position,t.normal);
				M::Vec::add(t.v0->normal,t.normal);
				M::Vec::add(t.v1->normal,t.normal);
				M::Vec::add(t.v2->normal,t.normal);
				M::Vec::normalize0(t.normal);
			}
			for (index_t i = 0; i < obj.quad_field.GetLength(); i++)
			{
				typename VsMesh::Quad&q = obj.quad_field[i];
				_oTriangleNormal(q.v0->position,q.v1->position,q.v2->position,q.normal);
				_oAddTriangleNormal(q.v0->position,q.v2->position,q.v3->position,q.normal);
				M::Vec::add(q.v0->normal,q.normal);
				M::Vec::add(q.v1->normal,q.normal);
				M::Vec::add(q.v2->normal,q.normal);
				M::Vec::add(q.v3->normal,q.normal);
				M::Vec::normalize0(q.normal);
			}
			for (index_t i = 0; i < obj.vertex_field.GetLength(); i++)
				M::Vec::normalize0(obj.vertex_field[i].normal);
			for (index_t i = 0; i < obj.edge_field.GetLength(); i++)
			{
				typename VsMesh::Edge&e = obj.edge_field[i];
				M::Vec::clear(e.normal);
				if (e.n[0])
				{
					if (e.n[0].is_quad)
					{
						M::Vec::copy(e.n[0].quad->normal,e.normal);
					}
					else
						M::Vec::copy(e.n[0].triangle->normal,e.normal);
				}
				if (e.n[1])
				{
					if (e.n[1].is_quad)
					{
						M::Vec::add(e.normal,e.n[1].quad->normal);
					}
					else
						M::Vec::add(e.normal,e.n[1].triangle->normal);
				}
				M::Vec::normalize0(e.normal);
			}
		}
		
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].createVisualMapsRec(detail,max_depth);
	}
	
template <class Def>	template <class C>
	void	SubGeometryA<Def>::scale(const C&factor)
	{
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].scale(factor);
		
		phHull.scale(factor);
		for (index_t i = 0; i < vs_hull_field.GetLength(); i++)
			vs_hull_field[i].scale(factor);
		
		for (index_t i = 0; i < wheel_field.GetLength(); i++)
		{
			M::Vec::mult(wheel_field[i].position,factor);
			M::Vec::mult(wheel_field[i].contraction,factor);
			wheel_field[i].radius*=factor;
			wheel_field[i].width*=factor;
			wheel_field[i].suspension.scale(factor);
			wheel_field[i].rotation.scale(factor);
		}
			
		for (index_t i = 0; i < accelerator_field.GetLength(); i++)
			M::Vec::mult(accelerator_field[i].position,factor);
		
		for (index_t i = 0; i < mounting_field.GetLength(); i++)
			M::Vec::mult(mounting_field[i].position,factor);
		
		M::Vec::mult(meta.system.w.xyz,factor);
		M::Vec::mult(meta.center,factor);
		meta.radius *= factor;
		meta.volume *= factor*factor*factor;
		meta.shortest_edge_length *= factor;
	}


template <class Def> String SubGeometryA<Def>::ToString(const String&intend)	const
{

	String rs = "<"+name+">\n"+intend;
	rs+="Wheels: "+String(wheel_field.GetLength())+"\n"+intend;
	rs+="Accelerators: "+String(accelerator_field.GetLength())+"\n"+intend;
	rs+="Mountings: "+String(mounting_field.GetLength())+"\n"+intend;
	rs+="Hull: "+phHull.ToString()+"\n"+intend;
	for (index_t i = 0; i < vs_hull_field.GetLength(); i++)
		rs+="VHull("+String(i)+"): "+vs_hull_field[i].ToString()+"\n"+intend;
	rs+="Children: "+String(child_field.GetLength())+"\n";
	for (index_t i = 0; i < child_field.GetLength(); i++)
		rs+=intend+" ("+String(i)+") "+child_field[i].ToString(intend+"	")+"\n";

	return rs;
}

/*
template <class Def> String SubGeometryA<Def>::difference(SubGeometryA&other, const String&intend)
{
	String rs;
	if (vs_hull_field.GetLength() != other.vs_hull_field.GetLength())
		rs+="\n"+intend+"objects "+String(vs_hull_field.GetLength())+" != "+String(other.vs_hull_field.GetLength());
	if ((vs_hull == other.vs_hull) && vs_hull)
		rs+="\n"+intend+"vs_hull: pointers equal";
	else
		for (index_t i = 0; i < vs_hull_field.GetLength() && i < other.vs_hull_field.GetLength(); i++)
			rs+="\n"+intend+"vs_hull("+String(i)+") "+vs_hull[i].difference(other.vs_hull[i],intend+" ");
	rs+="\n"+intend+"phHull "+phHull.difference(other.phHull,intend+" ");

	if (constructs != other.constructs)
		rs+="\n"+intend+"constructs "+String(constructs)+" != "+String(other.constructs);
	if ((construct == other.construct) && construct)
		rs+="\n"+intend+"constructs: pointers equal";
	if (wheels != other.wheels)
		rs+="\n"+intend+"wheels "+String(wheels)+" != "+String(other.wheels);
	if ((wheel == other.wheel) && wheel)
		rs+="\n"+intend+"wheels: pointers equal";
	if (accelerators != other.accelerators)
		rs+="\n"+intend+"accelerators "+String(accelerators)+" != "+String(other.accelerators);
	if ((accelerator == other.accelerator) && accelerator)
		rs+="\n"+intend+"accelerators: pointers equal";
	rs+=vstructArrayCompare("child_field.GetLength()",child_field.GetLength(),child,other.child_field.GetLength(),other.child,intend+" ");
	return rs;
}*/

template <class Def> void SubGeometryA<Def>::saveToRiff(Riff::Chunk*riff)	const
{
	riff->AppendBlock("OBJ ",name.c_str(),name.GetLength());

	Riff::Chunk*inner = riff->AppendBlock(RIFF_LIST);
		{
			ByteStreamBuffer	m;
			//typename Def::SystemType	field[22];
			m.Append(meta.system.v,16);
			m.Append(meta.center.v,3);
			m << meta.radius
				<< meta.volume
				<< meta.density
				<< meta.shortest_edge_length;
			//field[19] = meta.radius;
			//ield[20] = meta.volume;
			//field[21] = meta.density;
			inner->AppendBlock("META",m);
		}
		
		for (index_t i = 0; i < vs_hull_field.GetLength(); i++)
		{
			typedef Mesh<VsDef>	VsMesh;
			const VsMesh&hull = vs_hull_field[i];
			{
				Ctr::Array<typename Def::FloatType> out(hull.vertex_field.GetLength()*3);
				for (index_t j = 0; j < hull.vertex_field.GetLength(); j++)
					M::Vec::copy(hull.vertex_field[j].position,M::Vec::ref3(out + j*3));
				inner->AppendBlock("VVX3",out);
    		}
			{
				Ctr::Array<UINT32>	out(hull.triangle_field.GetLength()*3);
				for (index_t j = 0; j < hull.triangle_field.GetLength(); j++)
				{
					const typename VsMesh::Triangle	&tri = hull.triangle_field[j];
					out[j*3 + 0] = static_cast<UINT32>(tri.vertex[0]-hull.vertex_field);
					out[j*3 + 1] = static_cast<UINT32>(tri.vertex[1]-hull.vertex_field);
					out[j*3 + 2] = static_cast<UINT32>(tri.vertex[2]-hull.vertex_field);
				}
				inner->AppendBlock("VFC3",out);
			}
			{
				Ctr::Array<UINT32>	out(hull.quad_field.GetLength()*4);
				for (index_t j = 0; j < hull.quad_field.GetLength(); j++)
				{
					const typename VsMesh::Quad	&quad = hull.quad_field[j];
					out[j*4 + 0] = static_cast<UINT32>(quad.vertex[0]-hull.vertex_field);
					out[j*4 + 1] = static_cast<UINT32>(quad.vertex[1]-hull.vertex_field);
					out[j*4 + 2] = static_cast<UINT32>(quad.vertex[2]-hull.vertex_field);
					out[j*4 + 3] = static_cast<UINT32>(quad.vertex[3]-hull.vertex_field);
				}
				inner->AppendBlock("VFC4",out);
			}			
			{
				Ctr::Array<INT32>out(hull.edge_field.GetLength()*4);	//2*vertex. 2*face. positive: triangle, negative: quad+1
				for (index_t j = 0; j < hull.edge_field.GetLength(); j++)
				{
					const typename VsMesh::Edge	&edge = hull.edge_field[j];
					
					out[j*4 + 0] = static_cast<UINT32>(edge.vertex[0]-hull.vertex_field);
					out[j*4 + 1] = static_cast<UINT32>(edge.vertex[1]-hull.vertex_field);
					out[j*4 + 2] = static_cast<UINT32>(hull.linkToIndex(edge.n[0]));
					out[j*4 + 3] = static_cast<UINT32>(hull.linkToIndex(edge.n[1]));
				}
				inner->AppendBlock("VEDG",out);
			}
		}

		if (!phHull.IsEmpty())
		{
			{
				Ctr::Array<typename Def::PhHullFloatType> out(phHull.vertex_field.GetLength()*3);
				for (index_t j = 0; j < phHull.vertex_field.GetLength(); j++)
				{
					M::Vec::copy(phHull.vertex_field[j].position,M::Vec::ref3(out + j*3));
					//ASSERT_IS_CONSTRAINED3__(out + j*3,-30,30);

				}
				inner->AppendBlock("HVX3",out);
    		}
		
			ByteStreamBuffer	out;	//3*vindex, group, name, grip, updrift
			for (index_t i = 0; i < phHull.triangle_field.GetLength(); i++)
			{
				const typename Mesh<PhDef>::Triangle	&triangle = phHull.triangle_field[i];
				out << (UINT32)(triangle.vertex[0]-phHull.vertex_field)
					<< (UINT32)(triangle.vertex[1]-phHull.vertex_field)
					<< (UINT32)(triangle.vertex[2]-phHull.vertex_field);
			}
			inner->AppendBlock("HTRI",out);
		
			out.Clear();
			for (index_t i = 0; i < phHull.quad_field.GetLength(); i++)
			{
				const typename Mesh<PhDef>::Quad	&quad = phHull.quad_field[i];
				out << (UINT32)(quad.vertex[0]-phHull.vertex_field)
					<< (UINT32)(quad.vertex[1]-phHull.vertex_field)
					<< (UINT32)(quad.vertex[2]-phHull.vertex_field)
					<< (UINT32)(quad.vertex[3]-phHull.vertex_field);
			}
			inner->AppendBlock("HQAD",out);

			{
				Ctr::Array<UINT32>out(phHull.edge_field.GetLength()*4);	//2*vertex. 2*face
				for (index_t j = 0; j < phHull.edge_field.GetLength(); j++)
				{
					const typename Mesh<PhDef>::Edge	&edge = phHull.edge_field[j];
						
					out[j*4 + 0] = static_cast<UINT32>(edge.vertex[0]-phHull.vertex_field);
					out[j*4 + 1] = static_cast<UINT32>(edge.vertex[1]-phHull.vertex_field);
					out[j*4 + 2] = static_cast<UINT32>(phHull.linkToIndex(edge.n[0]));
					out[j*4 + 3] = static_cast<UINT32>(phHull.linkToIndex(edge.n[1]));
				}
				inner->AppendBlock("HEDG",out);
			}
		}
		
		if (accelerator_field.GetLength())
		{
			Buffer<BYTE>	write_buffer;
			ByteStreamBuffer	a;
			for (index_t i = 0; i < accelerator_field.GetLength(); i++)
			{
				a.Clear();
				a.Append(accelerator_field[i].position.v,3);
				a.Append(accelerator_field[i].direction.v,3);
				a << accelerator_field[i].power << accelerator_field[i].zero_efficiency;
				a << accelerator_field[i].flame_length << accelerator_field[i].flame_width;
				a.Append(accelerator_field[i].flame_color.v,3);
				a.Append(accelerator_field[i].name.c_str(),accelerator_field[i].name.GetLength());
				inner->AppendBlock("ACC2", a);
			}
		}

		if (mounting_field.GetLength())
		{
			
		
			ByteStreamBuffer	a;//(sizeof(UINT32)+13*sizeof(typename Def::FloatType));
			for (index_t i = 0; i < mounting_field.GetLength(); i++)
			{
				a.Clear();
				a.Append(mounting_field[i].flags);
				a.Append(mounting_field[i].position.v,3);
				a.Append(mounting_field[i].direction.v,3);
				a.Append(mounting_field[i].vertical.v,3);
				a.Append(mounting_field[i].dimension.v,3);
				a.Append(mounting_field[i].max_payload);
				inner->AppendBlock("MNT ", a);
			}
		}

		if (wheel_field.GetLength())
		{
			ByteStreamBuffer	w;
			for (index_t i = 0; i < wheel_field.GetLength(); i++)
			{
				w.Clear();
				//w.push(wheel_field[i].name);
				//w.push(wheel_field[i].next?wheel_field[i].next->name:(tName)0);
				//w.push(wheel_field[i].prev?wheel_field[i].prev->name:(tName)0);
				w.Append(wheel_field[i].position.v,3);
				w.Append(wheel_field[i].contraction.v,3);
				w.Append(wheel_field[i].axis.v,3);
				w.Append(wheel_field[i].radius);
				w.Append(wheel_field[i].width);
				w.Append(wheel_field[i].parameter,ARRAYSIZE(wheel_field[i].parameter));
				w.Append(wheel_field[i].name.c_str(),wheel_field[i].name.GetLength());
				inner->AppendBlock("WHL2",w);
				wheel_field[i].suspension.saveToRiff(inner);
				wheel_field[i].rotation.saveToRiff(inner);
			}
		}
		if (tracks_field.IsNotEmpty())
		{
			ByteStreamBuffer	c;
			for (index_t i = 0; i < tracks_field.GetLength(); i++)
			{
				const Tracks<Def>&t = tracks_field[i];
				c.Clear();
				c << (UINT32)t.member_field.GetLength();

				for (index_t k = 0; k < t.member_field.GetLength(); k++)
				{
					c << t.member_field[k].name.GetLength();
					c.Append(t.member_field[k].name.c_str(),t.member_field[k].name.GetLength());
				}

				//for (index_t k = 0; k < tracks_field[i].entry_field.GetLength(); k++)
				//	c.push(tracks_field[i].entry_field[k]->name);
				inner->AppendBlock("TRK2",c);
				//tracks_field[i].rotation.saveToRiff(inner);
			}
		}
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].saveToRiff(inner);
}

template <class Def> void SubGeometryA<Def>::loadFromRiff(Riff::File&riff) //assume we're alread in the object-list, have name, smooth_groups, layers and materials set
{
	clear();

	if (riff.FindFirst("META") /*&& riff.GetSize() == sizeof(typename Def::SystemType)*22*/)
	{
		riff.OpenStream();
			riff.Stream(meta.system.v,16);
			riff.Stream(meta.center.v,3);
			riff.Stream(meta.radius);
			riff.Stream(meta.volume);
			riff.Stream(meta.density);
			if (!riff.Stream(meta.shortest_edge_length))
				meta.shortest_edge_length = 0.1f;
		riff.CloseStream();
	}

	count_t objects(0);
	if (riff.FindFirst("VVTX"))
		do
			if (riff.IsMultipleOf(sizeof(typename Def::FloatType)*4))
				objects++;
		while (riff.FindNext("VVTX"));
	
	if (riff.FindFirst("VVX3"))
		do
			if (riff.IsMultipleOf(sizeof(typename Def::FloatType)*3))
				objects++;
		while (riff.FindNext("VVX3"));

	vs_hull_field.SetSize(objects);
	count_t c(0);
	
	if (riff.FindFirst("VVTX"))
		do
			if (riff.IsMultipleOf(sizeof(typename Def::FloatType)*4))
			{
				typedef Mesh<VsDef>	VsMesh;
				VsMesh&hull = vs_hull_field[c];
				hull.clear();
				{
					Ctr::Array<typename Def::FloatType>	array;
					riff.Get(array);
					hull.vertex_field.SetSize(array.GetLength()/4);
					for (index_t i = 0; i < hull.vertex_field.GetLength(); i++)
					{
						hull.vertex_field[i].position = M::Vec::ref3(array + i*4);
						hull.vertex_field[i].index = i;
						hull.vertex_field[i].marked = false;
					}
				}
				if (riff.FindNext("VFCE") && riff.IsMultipleOf(sizeof(UINT32)*4))
				{
					Ctr::Array<UINT32>	array;
					riff.Get(array);
					hull.triangle_field.SetSize(array.GetLength()/4);
					for (index_t i = 0; i < hull.triangle_field.GetLength(); i++)
					{
						typename VsMesh::Triangle	&face = hull.triangle_field[i];
						face.vertex[0] = hull.vertex_field+array[i*4];
						face.vertex[1] = hull.vertex_field+array[i*4+1];
						face.vertex[2] = hull.vertex_field+array[i*4+2];
						face.index = i;
						face.marked = false;
					}
				}
				if (riff.FindNext("VEDG") && riff.IsMultipleOf(sizeof(UINT32)*4))
				{
					Ctr::Array<UINT32>	array;
					riff.Get(array);
					hull.edge_field.SetSize(array.GetLength()/4);
					for (index_t i = 0; i < hull.edge_field.GetLength(); i++)
					{
						typename VsMesh::Edge	&edge = hull.edge_field[i];
						edge.vertex[0] = hull.vertex_field+array[i*4];
						edge.vertex[1] = hull.vertex_field+array[i*4+1];
						hull.indexToLink(array[i*4+2],edge.n[0]);
						hull.indexToLink(array[i*4+3],edge.n[1]);
						edge.index = i;
						edge.marked = false;
					}
				}
				hull.correct();
				c++;
			}
		while (riff.FindNext("VVTX"));

	if (riff.FindFirst("VVX3"))
		do
			if (riff.IsMultipleOf(sizeof(typename Def::FloatType)*3))
			{
				typedef Mesh<VsDef>	VsMesh;
				VsMesh	&hull = vs_hull_field[c];
				hull.clear();
				{
					Ctr::Array<typename Def::FloatType>	array;
					riff.Get(array);
					hull.vertex_field.SetSize(array.GetLength()/3);
					for (index_t i = 0; i < hull.vertex_field.GetLength(); i++)
					{
						hull.vertex_field[i].position = M::Vec::ref3(array + i*3);
						hull.vertex_field[i].index = i;
						hull.vertex_field[i].marked = false;
					}
				}
				if (riff.FindNext("VFC3") && riff.IsMultipleOf(sizeof(UINT32)*3))
				{
					Ctr::Array<UINT32>	array;
					riff.Get(array);
					hull.triangle_field.SetSize(array.GetLength()/3);
					for (index_t i = 0; i < hull.triangle_field.GetLength(); i++)
					{
						typename VsMesh::Triangle	&face = hull.triangle_field[i];
						face.vertex[0] = hull.vertex_field+array[i*3];
						face.vertex[1] = hull.vertex_field+array[i*3+1];
						face.vertex[2] = hull.vertex_field+array[i*3+2];
						face.index = i;
						face.marked = false;
					}
				}
				if (riff.FindNext("VFC4") && riff.IsMultipleOf(sizeof(UINT32)*4))
				{
					Ctr::Array<UINT32>	array;
					riff.Get(array);
					hull.quad_field.SetSize(array.GetLength()/4);
					for (index_t i = 0; i < hull.quad_field.GetLength(); i++)
					{
						typename VsMesh::Quad	&face = hull.quad_field[i];
						face.vertex[0] = hull.vertex_field+array[i*4];
						face.vertex[1] = hull.vertex_field+array[i*4+1];
						face.vertex[2] = hull.vertex_field+array[i*4+2];
						face.vertex[3] = hull.vertex_field+array[i*4+3];
						face.index = i;
						face.marked = false;
					}
				}
				if (riff.FindNext("VEDG") && riff.IsMultipleOf(sizeof(UINT32)*4))
				{
					Ctr::Array<INT32>	array;
					riff.Get(array);
					hull.edge_field.SetSize(array.GetLength()/4);
					for (index_t i = 0; i < hull.edge_field.GetLength(); i++)
					{
						typename VsMesh::Edge	&edge = hull.edge_field[i];
						edge.vertex[0] = hull.vertex_field+array[i*4];
						edge.vertex[1] = hull.vertex_field+array[i*4+1];
						hull.indexToLink(array[i*4+2],edge.n[0]);
						hull.indexToLink(array[i*4+3],edge.n[1]);
						edge.index = i;
						edge.marked = false;
					}
				}
				hull.correct();
				c++;
			}
		while (riff.FindNext("VVX3"));
		

	phHull.clear();
	if (riff.FindFirst("HVTX") && riff.IsMultipleOf(sizeof(typename Def::PhHullFloatType)*4))
	{
		{
			Ctr::Array<typename Def::PhHullFloatType>	array;
			riff.Get(array);
			phHull.vertex_field.SetSize(array.GetLength()/4);
			for (index_t i = 0; i < phHull.vertex_field.GetLength(); i++)
			{
				typename Mesh<PhDef>::Vertex	&vertex = phHull.vertex_field[i];
				/*vertex.p0 = vertex.p1 = */
				vertex.position = M::Vec::ref3(array + i*4);
				M::Vec::clear(vertex.normal);
				//vertex.grip = 0;
				vertex.index = i;
				vertex.marked = false;
			}
		}
		
		static const size_t ph_face_size = sizeof(UINT32)*5+sizeof(typename Def::PhFloatType)*2+sizeof(uint64_t);
		if (riff.FindNext("HFCE") && riff.IsMultipleOf(ph_face_size))
		{
			
			unsigned cnt = riff.GetSize()/ph_face_size;
			phHull.triangle_field.SetSize(cnt);
			
			riff.OpenStream();
			for (unsigned i = 0; i < cnt; i++)
			{
				typename Mesh<PhDef>::Triangle	&face = phHull.triangle_field[i];
				UINT32	index[4];
				riff.Stream(index,4);
				face.vertex[0] = phHull.vertex_field+index[0];
				face.vertex[1] = phHull.vertex_field+index[1];
				face.vertex[2] = phHull.vertex_field+index[2];
				
				riff.Skip(sizeof(typename Def::PhFloatType)*2 + sizeof(name64_t) + sizeof(UINT32));
				/*riff.Stream(dummyf);
				riff.Stream(dummyf);
				riff.Stream(dummy_name);
				riff.Stream(dummy32);*/
				riff.Skip(4);
			}
			riff.CloseStream();
		}
		
		if (riff.FindNext("HEDG") && riff.IsMultipleOf(sizeof(UINT32)*4))
		{
			Ctr::Array<UINT32>	array;
			riff.Get(array);
			phHull.edge_field.SetSize(array.GetLength()/4);
			for (index_t i = 0; i < phHull.edge_field.GetLength(); i++)
			{
				typename Mesh<PhDef>::Edge	&edge = phHull.edge_field[i];
				edge.vertex[0] = phHull.vertex_field+array[i*4];
				edge.vertex[1] = phHull.vertex_field+array[i*4+1];
				phHull.indexToLink(array[i*4+2],edge.n[0]);
				phHull.indexToLink(array[i*4+3],edge.n[1]);
				edge.index = i;
				edge.marked = false;
			}
		}
	}
	else
		if (riff.FindFirst("HVX3") && riff.IsMultipleOf(sizeof(typename Def::PhHullFloatType)*3))
		{
			{
				Ctr::Array<typename Def::PhHullFloatType>	array;
				riff.Get(array);
				phHull.vertex_field.SetSize(array.GetLength()/3);
				for (index_t i = 0; i < phHull.vertex_field.GetLength(); i++)
				{
					typename Mesh<PhDef>::Vertex	&vertex = phHull.vertex_field[i];
					//vertex.p0 = vertex.p1 = 
					vertex.position = M::Vec::ref3(array + i*3);
					M::Vec::clear(vertex.normal);
					//ASSERT_IS_CONSTRAINED3__(array+i*3,-30,30);
					//vertex.grip = 0;
					vertex.index = i;
					vertex.marked = false;
				}
			}
			
			static const size_t ph_face_size3 = sizeof(UINT32)*3,
								ph_face_size4 = sizeof(UINT32)*4;
			if (riff.FindNext("HTRI"))
			{
				
				if ( riff.IsMultipleOf(ph_face_size3))
				{
					count_t cnt = riff.GetSize()/ph_face_size3;
					phHull.triangle_field.SetSize(cnt);
				
					riff.OpenStream();
					for (index_t i = 0; i < cnt; i++)
					{
						typename Mesh<PhDef>::Triangle	&face = phHull.triangle_field[i];
						UINT32	index[3];
						riff.Stream(index,3);
						face.vertex[0] = phHull.vertex_field+index[0];
						face.vertex[1] = phHull.vertex_field+index[1];
						face.vertex[2] = phHull.vertex_field+index[2];
					}
					riff.CloseStream();
				}
			}
			if (riff.FindNext("HQAD"))
			{
				if (riff.IsMultipleOf(ph_face_size4))
				{
					count_t cnt = riff.GetSize()/ph_face_size4;
					phHull.quad_field.SetSize(cnt);
				
					riff.OpenStream();
					for (index_t i = 0; i < cnt; i++)
					{
						typename Mesh<PhDef>::Quad	&face = phHull.quad_field[i];
						UINT32	index[4];
						riff.Stream(index,4);
						face.vertex[0] = phHull.vertex_field+index[0];
						face.vertex[1] = phHull.vertex_field+index[1];
						face.vertex[2] = phHull.vertex_field+index[2];
						face.vertex[3] = phHull.vertex_field+index[3];
					}
					riff.CloseStream();
				}
			}
						
			
			if (riff.FindNext("HEDG") && riff.IsMultipleOf(sizeof(UINT32)*4))
			{
				Ctr::Array<UINT32>	array;
				riff.Get(array);
				phHull.edge_field.SetSize(array.GetLength()/4);
				for (index_t i = 0; i < phHull.edge_field.GetLength(); i++)
				{
					typename Mesh<PhDef>::Edge	&edge = phHull.edge_field[i];
					edge.vertex[0] = phHull.vertex_field+array[i*4];
					edge.vertex[1] = phHull.vertex_field+array[i*4+1];
					phHull.indexToLink(array[i*4+2],edge.n[0]);
					phHull.indexToLink(array[i*4+3],edge.n[1]);

					edge.index = i;
					edge.marked = false;
				}
			}
		}
	phHull.verifyIntegrity();
	phHull.correct();

	/*
	for (index_t i = 0; i < phHull.vertex_field.GetLength(); i++)
		phHull.vertex_field[i].index = 0;*/
	
	for (index_t i = 0; i < phHull.triangle_field.GetLength(); i++)
	{
		typename Mesh<PhDef>::Triangle	&face = phHull.triangle_field[i];
		Obj::triangleNormal(face.vertex[0]->position,face.vertex[1]->position,face.vertex[2]->position,face.normal);
		M::Vec::add(face.vertex[0]->normal,face.normal);
		M::Vec::add(face.vertex[1]->normal,face.normal);
		M::Vec::add(face.vertex[2]->normal,face.normal);
		/*face.vertex[0]->grip += face.grip;
		face.vertex[1]->grip += face.grip;
		face.vertex[2]->grip += face.grip;*/
//		_normalize(face.normal);
		/*face.vertex[0]->index++;
		face.vertex[1]->index++;
		face.vertex[2]->index++;*/
		//M::Vec::center(face.vertex[0]->position,face.vertex[1]->position,face.vertex[2]->position,face.center);
		//M::Vec::copy(face.center,face.gcenter);
	}
	
	for (index_t i = 0; i < phHull.quad_field.GetLength(); i++)
	{
		typename Mesh<PhDef>::Quad	&face = phHull.quad_field[i];
		_oTriangleNormal(face.vertex[0]->position,face.vertex[1]->position,face.vertex[2]->position,face.normal);
		_oAddTriangleNormal(face.vertex[0]->position,face.vertex[2]->position,face.vertex[3]->position,face.normal);
		M::Vec::add(face.vertex[0]->normal,face.normal);
		M::Vec::add(face.vertex[1]->normal,face.normal);
		M::Vec::add(face.vertex[2]->normal,face.normal);
		M::Vec::add(face.vertex[3]->normal,face.normal);
		/*face.vertex[0]->grip += face.grip;
		face.vertex[1]->grip += face.grip;
		face.vertex[2]->grip += face.grip;
		face.vertex[3]->grip += face.grip;*/
//		_normalize(face.normal);
		/*face.vertex[0]->index++;
		face.vertex[1]->index++;
		face.vertex[2]->index++;
		face.vertex[3]->index++;*/
		/*M::Vec::copy(face.vertex[0]->position,face.center);
		M::Vec::add(face.center,face.vertex[1]->position);
		M::Vec::add(face.center,face.vertex[2]->position);
		M::Vec::add(face.center,face.vertex[3]->position);
		M::Vec::div(face.center,4.0);
		M::Vec::copy(face.center,face.gcenter);*/
	}
	
	
	for (index_t i = 0; i < phHull.vertex_field.GetLength(); i++)
	{
		if (phHull.vertex_field[i].index)
		{
			M::Vec::normalize0(phHull.vertex_field[i].normal);
			//phHull.vertex_field[i].grip /= phHull.vertex_field[i].index;
		}
		//phHull.vertex_field[i].index = i;
	}
	
	for (index_t i = 0; i < phHull.edge_field.GetLength(); i++)
	{
		typename Mesh<PhDef>::Edge	&edge = phHull.edge_field[i];
		M::Vec::clear(edge.normal);
		
		if (edge.n[0].is_quad)
		{
			M::Vec::copy(edge.n[0].quad->normal,edge.normal);
			//edge.grip = edge.n[0].quad->grip;
		}
		else
		{
			M::Vec::copy(edge.n[0].triangle->normal,edge.normal);
			//edge.grip = edge.n[0].triangle->grip;
		}

		if (edge.n[1].triangle)
			if (edge.n[1].is_quad)
			{
				M::Vec::add(edge.normal,edge.n[1].quad->normal);
				//edge.grip = (edge.grip+edge.n[1].quad->grip)/2;
			}
			else
			{
				M::Vec::add(edge.normal,edge.n[1].triangle->normal);
				//edge.grip = (edge.grip+edge.n[1].triangle->grip)/2;
			}
	}
	
	for (index_t i = 0; i < phHull.triangle_field.GetLength(); i++)
		M::Vec::normalize(phHull.triangle_field[i].normal);
	for (index_t i = 0; i < phHull.quad_field.GetLength(); i++)
		M::Vec::normalize(phHull.quad_field[i].normal);
	
	
	static const size_t AcceleratorSize = sizeof(tName)+8*sizeof(typename Def::PhFloatType),
						ExtAcceleratorSize = AcceleratorSize+5*sizeof(typename Def::FloatType),
						MinAcceleratorSize = 8*sizeof(typename Def::PhFloatType) + 5*sizeof(typename Def::FloatType),
						MountingSize = sizeof(UINT32)+sizeof(typename Def::FloatType)*13;

	unsigned mountings = 0;
	if (riff.FindFirst("MNT "))
		do
		{
			if (riff.GetSize() == MountingSize)
				mountings++;
		}
		while (riff.FindNext("MNT "));
	
	mounting_field.SetSize(mountings);
	mountings = 0;
	
	if (mounting_field.GetLength() && riff.FindFirst("MNT "))
		do
		{
			if (riff.GetSize() == MountingSize)
			{
				riff.OpenStream();
					riff.Stream(mounting_field[mountings].flags);
					riff.Stream(mounting_field[mountings].position.v,3);
					riff.Stream(mounting_field[mountings].direction.v,3);
					riff.Stream(mounting_field[mountings].vertical.v,3);
					riff.Stream(mounting_field[mountings].dimension.v,3);
					riff.Stream(mounting_field[mountings].max_payload);
				riff.CloseStream();
				mountings++;
			}
		}
		while (riff.FindNext("MNT "));
	
	count_t accelerators = 0;
	index_t acc_version = 0;
	const char*acc_key = "ACCL";
	if (riff.FindFirst("ACCL"))
	{
		do
		{
			if (riff.GetSize() == AcceleratorSize || riff.GetSize() == ExtAcceleratorSize)
				accelerators++;
		}
		while (riff.FindNext("ACCL"));
	}
	else
		if (riff.FindFirst("ACC2"))
		{
			acc_version = 1;
			acc_key = "ACC2";
			do
			{
				if (riff.GetSize() > MinAcceleratorSize)
					accelerators++;
			}
			while (riff.FindNext("ACC2"));
		}
	accelerator_field.SetSize(accelerators);
	accelerators = 0;
	if (accelerator_field.IsNotEmpty() && riff.FindFirst(acc_key))
		do
		{

			switch (acc_version)
			{
				case 0:
					if (riff.GetSize() == AcceleratorSize)
					{

						riff.OpenStream();
							name64_t	name;
							riff.Stream(name);
							accelerator_field[accelerators].name = name2str(name);

							riff.Stream(accelerator_field[accelerators].position.v,3);
							riff.Stream(accelerator_field[accelerators].direction.v,3);
							riff.Stream(accelerator_field[accelerators].power);
							riff.Stream(accelerator_field[accelerators].zero_efficiency);
						riff.CloseStream();

						accelerator_field[accelerators].flame_length = 0;
						accelerator_field[accelerators].flame_width = 0;
						M::Vec::def(accelerator_field[accelerators].flame_color,1,0.5,0);

					}
					elif (riff.GetSize() == ExtAcceleratorSize)
					{
						riff.OpenStream();
							name64_t name;
							riff.Stream(name);
							accelerator_field[accelerators].name = name2str(name);

							riff.Stream(accelerator_field[accelerators].position.v,3);
							riff.Stream(accelerator_field[accelerators].direction.v,3);
							riff.Stream(accelerator_field[accelerators].power);
							riff.Stream(accelerator_field[accelerators].zero_efficiency);
							riff.Stream(accelerator_field[accelerators].flame_length);
							riff.Stream(accelerator_field[accelerators].flame_width);
							riff.Stream(accelerator_field[accelerators].flame_color.v,3);
						riff.CloseStream();
					}
					else
						continue;
				break;
				case 1:
					if (riff.GetSize() > MinAcceleratorSize)
					{
						size_t name_length = riff.GetSize() - MinAcceleratorSize;
						accelerator_field[accelerators].name.setLength(name_length);
						riff.OpenStream();
							riff.Stream(accelerator_field[accelerators].position.v,3);
							riff.Stream(accelerator_field[accelerators].direction.v,3);
							riff.Stream(accelerator_field[accelerators].power);
							riff.Stream(accelerator_field[accelerators].zero_efficiency);
							riff.Stream(accelerator_field[accelerators].flame_length);
							riff.Stream(accelerator_field[accelerators].flame_width);
							riff.Stream(accelerator_field[accelerators].flame_color.v,3);

							riff.Stream(accelerator_field[accelerators].name.mutablePointer(),name_length);
						riff.CloseStream();
					}
					else
						continue;
				break;
				default:
					continue;
			}

			accelerators++;
		}
		while (riff.FindNext(acc_key));

	count_t wheels = 0;
	const char*whl_key = "WHEL";
	index_t	whl_version = 0;
		bool whl_begin = false;
	if (riff.FindFirst("WHEL"))
	{
		whl_begin= true;
	}	//default
	else if (riff.FindFirst("WHL2"))
	{
		whl_key = "WHL2";
		whl_version = 1;
		whl_begin = true;
	}
	if (whl_begin)
		do
		{
			wheels++;
		}
		while (riff.FindNext(whl_key));
	

	wheel_field.SetSize(wheels);
	wheels = 0;
	if (wheel_field.IsNotEmpty() && riff.FindFirst(whl_key))
		do
		{
			switch (whl_version)
			{
				case 0:
				{
					riff.OpenStream();
						name64_t name;
						riff.Stream(name);
						wheel_field[wheels].name = name2str(name);
						riff.Stream(name);	//ignore
						riff.Stream(name);	//ignore
						riff.Stream(wheel_field[wheels].position.v,3);
						riff.Stream(wheel_field[wheels].contraction.v,3);
						riff.Stream(wheel_field[wheels].axis.v,3);
						riff.Stream(wheel_field[wheels].radius);
						riff.Stream(wheel_field[wheels].width);
						riff.Stream(wheel_field[wheels].parameter,ARRAYSIZE(wheel_field[wheels].parameter));
					riff.CloseStream();
				}
				break;
				case 1:
				{
					riff.OpenStream();
						riff.Stream(wheel_field[wheels].position.v,3);
						riff.Stream(wheel_field[wheels].contraction.v,3);
						riff.Stream(wheel_field[wheels].axis.v,3);
						riff.Stream(wheel_field[wheels].radius);
						riff.Stream(wheel_field[wheels].width);
						riff.Stream(wheel_field[wheels].parameter,ARRAYSIZE(wheel_field[wheels].parameter));

						size_t name_length = riff.GetSize() - riff.StreamTell();
						wheel_field[wheels].name.setLength(name_length);
						riff.Stream(wheel_field[wheels].name.mutablePointer(),name_length);
					riff.CloseStream();
				}
				break;
				default:
					continue;
			}
			wheel_field[wheels].suspension.loadFromRiff(riff,1);	//must start at 1, because otherwise the animator uses findFirst(...) and that will cause issues
			wheel_field[wheels].rotation.loadFromRiff(riff,2);
			wheels++;
		}
		while (riff.FindNext(whl_key));

	/*for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		wheel_field[i].next = findWheel(wheel_field[i].nname);
		wheel_field[i].prev = findWheel(wheel_field[i].pname);
	}*/
	
	count_t tracks = 0;

	const char* trk_key = "CNST";
	index_t		trk_version = 0;
	bool		trk_begin = false;

	if (riff.FindFirst("CNST"))
	{
		trk_begin = true;
	}
	else
		if (riff.FindFirst("TRK2"))
		{
			trk_key = "TRK2";
			trk_version = 1;
			trk_begin = true;
		}
	if (trk_begin)
		do
		{
			tracks++;
		}
		while (riff.FindNext(trk_key));
	tracks_field.SetSize(tracks);
	tracks = 0;
	if (tracks_field.IsNotEmpty() && riff.FindFirst(trk_key))
		do
		{
			switch (trk_version)
			{
				case 0:
					if (riff.GetSize() && riff.GetSize() && !(riff.GetSize()%8))
					{
						tracks_field[tracks].member_field.SetSize(riff.GetSize()/8);
						name64_t	name;
						riff.OpenStream();
							for (index_t k = 0; k < tracks_field[tracks].member_field.GetLength(); k++)
								if (riff.Stream(name))
								{
									tracks_field[tracks].member_field[k].wheel = NULL;
									tracks_field[tracks].member_field[k].name = name2str(name);
								}
						riff.CloseStream();
					}
					else
						continue;
				break;
				case 1:
					UINT32	member_count;
					riff.OpenStream();
						if (!riff.Stream(member_count))
						{
							riff.CloseStream();
							continue;
						}
						tracks_field[tracks].member_field.SetSize(member_count);
						for (UINT32 k = 0; k < member_count; k++)
						{
							UINT32	name_length;
							if (!riff.Stream(name_length) || !riff.CanStreamBytes(name_length))
							{
								riff.CloseStream();
								tracks_field[tracks].member_field.free();
								continue;
							}
							tracks_field[tracks].member_field[k].name.setLength(name_length);
							riff.Stream(tracks_field[tracks].member_field[k].name.mutablePointer(),name_length);
						}
					riff.CloseStream();

				break;
			}
			//tracks_field[tracks].rotation.loadFromRiff(riff);
			tracks++;
		}
		while (riff.FindNext(trk_key));

	count_t children = 0;
	if (riff.FindFirst("OBJ "))
		do
		{
			if (riff.FindNext("LIST"))
				children++;
		}
		while (riff.FindNext("OBJ "));
	child_field.SetSize(children);
	children = 0;
	if (child_field.GetLength() && riff.FindFirst("OBJ "))
		do
		{
			child_field[children].name.setLength(riff.GetSize());
			riff.Get(child_field[children].name.mutablePointer());
			child_field[children].name.TrimThis();

			//if (riff.GetSize() == sizeof(name))
			{
				//riff.Get(&child_field[children].name);
				if (riff.FindNext("LIST"))
				{
					riff.Enter();
						child_field[children].loadFromRiff(riff);
					riff.DropBack();
					children++;
				}
			}
		}
		while (riff.FindNext("OBJ "));
		
}

template <class Def> bool SubGeometryA<Def>::isMember(SubGeometryA<Def>*obj) const
{
	if ((unsigned)(obj-child_field) < child_field.GetLength())
		return true;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		if (child_field[i].isMember(obj))
			return true;
	return false;
}

template <class Def> bool SubGeometryA<Def>::isMember(TWheel<Def>*whl) const
{
	if ((unsigned)(whl-wheel_field) < wheel_field.GetLength())
		return true;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		if (child_field[i].isMember(whl))
			return true;
	return false;
}

template <class Def> bool SubGeometryA<Def>::isMember(TAccelerator<Def>*acc) const
{
	if ((unsigned)(acc-accelerator_field) < accelerator_field.GetLength())
		return true;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		if (child_field[i].isMember(acc))
			return true;
	return false;
}



template <class Def> void SubGeometryA<Def>::linkAnimators(Geometry<Def>*domain)
{
	for (index_t i = 0; i < accelerator_field.GetLength(); i++)
	{
		accelerator_field[i].domain = this;
	}
	for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		wheel_field[i].domain = this;
		wheel_field[i].rotation.link(domain);
		wheel_field[i].suspension.link(domain);
	}
	for (index_t i = 0; i < tracks_field.GetLength(); i++)
	{
		//tracks_field[i].rotation.link(domain);
		tracks_field[i].domain = this;
	}
	for (index_t i = 0; i < child_field.GetLength(); i++)
	{
		child_field[i].linkAnimators(domain);
		//child_field[i].domain = this;	//maybe later
	}
}

template <class Def> void SubGeometryA<Def>::postCopyLink(Geometry<Def>*domain)
{
	system_link = &path;
	for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		wheel_field[i].rotation.link(domain);
		wheel_field[i].suspension.link(domain);
		/*wheel_field[i].next = findWheel(wheel_field[i].nname);
		if (!wheel_field[i].next)
			wheel_field[i].next = wheel_field+i;
		wheel_field[i].prev = findWheel(wheel_field[i].pname);
		if (!wheel_field[i].prev)
			wheel_field[i].prev = wheel_field+i;*/
	}
	for (index_t i = 0; i < tracks_field.GetLength(); i++)
	{
		for (index_t j = 0; j < tracks_field[i].member_field.GetLength(); j++)
			tracks_field[i].member_field[j].wheel = domain->lookupWheel(tracks_field[i].member_field[j].name);	//localize
		//tracks_field[i].rotation.link(domain);
	}
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].postCopyLink(domain);
}


template <class Def> void SubGeometryA<Def>::storeTargetNames()
{
	for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		wheel_field[i].rotation.storeTargetNames();
		wheel_field[i].suspension.storeTargetNames();
		//wheel_field[i].nname = wheel_field[i].next->name;
		//wheel_field[i].pname = wheel_field[i].prev->name;
	}
	for (index_t i = 0; i < tracks_field.GetLength(); i++)
	{
		for (index_t j = 0; j < tracks_field[i].member_field.GetLength(); j++)
			tracks_field[i].member_field[j].name = tracks_field[i].member_field[j].wheel->name;
		//tracks_field[i].rotation.storeTargetNames();
	}
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].storeTargetNames();
}


template <class Def> void SubGeometryA<Def>::checkLinkage()
{
	/*for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		if ((wheel_field[i].prev && !wheel_field.Owns(wheel_field[i].prev))
			||
			(wheel_field[i].next && !wheel_field.Owns(wheel_field[i].next)))
			FATAL__("WheelLinkage of "+name2str(name)+" broken");
	}
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].checkLinkage();
	*/
}


template <class Def>
template <class C> bool SubGeometryA<Def>::extractDimensions(const M::TMatrix4<typename Def::SystemType>&system, DimensionParser<C>&parser) const
{
	bool def;
	M::TMatrix4<typename Def::SystemType> lsystem;
	M::Mat::transformSystem(system,meta.system,lsystem);
	if (vs_hull_field.GetLength() && vs_hull_field[0].vertex_field.GetLength())
	{
		def = true;
		M::TVec3<C>	p;

		M::Mat::Transform(lsystem,vs_hull_field[0].vertex_field[0].position,p);
		M::Box<C>		local(p,p);

		for (index_t i = 1; i < vs_hull_field[0].vertex_field.GetLength(); i++)
		{
			M::Mat::Transform(lsystem,vs_hull_field[0].vertex_field[i].position,p);
			_oDetDimension(p,local);
		}
		parser.parse(local.min());
		parser.parse(local.max());
	}
	else
		def = false;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		def = child_field[i].extractDimensions(lsystem,parser)||def;
	return def;
}

template <class Def>
template <class C> bool SubGeometryA<Def>::extractDimensions(M::Box<C>&dim) const
{
	bool def;
	if (vs_hull_field.GetLength()&&vs_hull_field[0].vertex_field.GetLength())
	{
		def = true;
		dim.SetMinAndMax(vs_hull_field[0].vertex_field[0].position);

		for (index_t i = 1; i < vs_hull_field[0].vertex_field.GetLength(); i++)
			dim.Include(vs_hull_field[0].vertex_field[i].position);
	}
	else
	{
		dim.SetMinAndMax(0);
		def = false;
	}
	return def;
}

template <class Def>
template <class C> void SubGeometryA<Def>::extractRadius(C&radius) const
{
	C vr = radius*radius;
	if (vs_hull_field.GetLength())
		for (index_t i = 0; i < vs_hull_field[0].vertex_field.GetLength(); i++)
		{
			C r = M::Vec::dot(vs_hull_field[0].vertex_field[i].position);
			if (r > vr)
				vr = r;
		}
	radius = vsqrt(vr);
}

template <class Def>
template <class C> void SubGeometryA<Def>::extractAverageVisualEdgeLength(index_t layer, C&length) const
{
	length = 0;
	if (layer < vs_hull_field.GetLength())
	{
		for (index_t i = 0; i < vs_hull_field[layer].edge_field.GetLength(); i++)
		{
			length += (C)M::Vec::quadraticDistance(vs_hull_field[layer].edge_field[i].vertex[0]->position,vs_hull_field[layer].edge_field[i].vertex[1]->position);
		}
		length = vsqrt(length)/(C)vs_hull_field[layer].edge_field.GetLength();
	}
}



template <class Def>
	template <typename T>
		void SubGeometryA<Def>::extractShortestVisualEdgeLength(index_t layer, T&length)	const
		{
			length = std::numeric_limits<T>::max();
			if (layer < vs_hull_field.GetLength())
			{
				const Mesh<VsDef>&mesh = vs_hull_field[layer];
				if (mesh.edge_field.IsNotEmpty())
				{
					for (index_t i = 0; i < mesh.edge_field.GetLength(); i++)
					{
						const typename Mesh<VsDef>::Edge&edge = mesh.edge_field[i];
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(edge.v0->position,edge.v1->position));
					}
				}
				else
				{
					for (index_t i = 0; i < mesh.triangle_field.GetLength(); i++)
					{
						const typename Mesh<VsDef>::Triangle&t = mesh.triangle_field[i];
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(t.v0->position,t.v1->position));
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(t.v1->position,t.v2->position));
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(t.v2->position,t.v0->position));
					}

					for (index_t i = 0; i < mesh.quad_field.GetLength(); i++)
					{
						const typename Mesh<VsDef>::Quad&q = mesh.quad_field[i];
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(q.v0->position,q.v1->position));
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(q.v1->position,q.v2->position));
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(q.v2->position,q.v3->position));
						_IncludeEdgeLength(length,M::Vec::quadraticDistance(q.v3->position,q.v0->position));
					}
				}

			}
	
			length = vsqrt(length);
		}


template <class Def>
template <class C> void SubGeometryA<Def>::extractAbsoluteRadius(C&radius) const
{
	if (vs_hull_field.GetLength() && vs_hull_field[0].vertex_field.GetLength())
	{
		C vr = radius*radius;
		for (index_t i = 0; i < vs_hull_field[0].vertex_field.GetLength(); i++)
		{
			C r;
			M::TVec3<C>	p;
			M::Mat::Transform(path,vs_hull_field[0].vertex_field[i].position,p);
			r = M::Vec::dot(p);
			if (r > vr)
				vr = r;
		}
		radius = vsqrt(vr);
	}
}

template <class Def>
template <class C> void SubGeometryA<Def>::extractAbsoluteRadius(const M::TVec3<C>&center, C&radius) const
{
	C vr = radius*radius;
	if (vs_hull_field.GetLength())
		for (index_t i = 0; i < vs_hull_field[0].vertex_field.GetLength(); i++)
		{
			C r;
			M::TVec3<C>	p;
			M::Mat::Transform(path,vs_hull_field[0].vertex_field[i].position,p);
			M::Vec::sub(p,center);
			r = M::Vec::dot(p);
			if (r > vr)
				vr = r;
		}
	radius = vsqrt(vr);
}

template <class Def>
template <class C> void SubGeometryA<Def>::addWeight(C&weight)		const
{
	weight += meta.volume*meta.density;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].addWeight(weight);
}



template <class Def> void SubGeometryA<Def>::defLinkageStr(String&rs, const String&intend)
{
	rs+=intend+"(0x"+IntToHex((int)(size_t)system_link,8)+")\n";
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].defLinkageStr(rs,intend+"	");
}


template <class Def>
template <class C> void SubGeometryA<Def>::extractRadiusRec(C&radius) const
{
	extractAbsoluteRadius(radius);
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].extractRadiusRec(radius);
}

template <class Def>
template <class C> void SubGeometryA<Def>::extractRadiusRec(const M::TVec3<C>&center, C&radius) const
{
	
	extractAbsoluteRadius(center,radius);
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].extractRadiusRec(center,radius);
}


template <class Def> void SubGeometryA<Def>::setSystemRec(const M::TMatrix4<typename Def::SystemType>&system)
{
	M::Mat::transformSystem(system,meta.system,path);
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].setSystemRec(path);
}

template <class Def> void SubGeometryA<Def>::resetLinkageRec()
{
	system_link = &path;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].resetLinkageRec();
}


template <class Def> void SubGeometryA<Def>::toInstance(StaticSubInstanceA<Def>&tree)
{
	tree.target = this;
	tree.child_field.SetSize(child_field.GetLength());
	tree.system = meta.system;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].toInstance(tree.child_field[i]);
}

template <class Def> void SubGeometryA<Def>::toInstance(AnimatableSubInstanceA<Def>&tree)
{
	tree.target = this;
	
	M::Vec::clear(tree.animation_status.translation);
	M::Vec::clear(tree.animation_status.rotation);

	tree.wheel_rotation_animators.SetSize(wheel_field.GetLength());
	tree.wheel_suspension_animators.SetSize(wheel_field.GetLength());
	//tree.construct_rotation_animators.SetSize(tracks_field.GetLength());
	for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		tree.wheel_rotation_animators[i].build(&wheel_field[i].rotation);
		tree.wheel_suspension_animators[i].build(&wheel_field[i].suspension);
	}
	/*for (index_t i = 0; i < tracks_field.GetLength(); i++)
		tree.construct_rotation_animators[i].build(&tracks_field[i].rotation);*/
	tree.wheel_status.SetSize(wheel_field.GetLength()*2);
	tree.accelerator_status.SetSize(accelerator_field.GetLength());
	tree.wheel_status.Fill(0);
	tree.accelerator_status.Fill(0);
	
	tree.child_field.SetSize(child_field.GetLength());
	tree.system = meta.system;
	
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].toInstance(tree.child_field[i]);
}

template <class Def> count_t SubGeometryA<Def>::countVertices(unsigned detail)
{
	count_t rs = detail<vs_hull_field.GetLength()?vs_hull_field[detail].vertex_field.GetLength():0;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		rs+=child_field[i].countVertices(detail);
	return rs;
}


template <class Def> bool SubGeometryA<Def>::checkIntegrity(Geometry<Def>*super, const String&path)
{
	error = "obj_err:";
	bool rs = true;
	String local = path+"/"+name;
	if (!phHull.valid())
	{
		error += "\nhull-error in "+local+": "+phHull.errorStr();
		rs = false;
	}
	for (index_t i = 0; i < vs_hull_field.GetLength(); i++)
		if (!vs_hull_field[i].valid())
		{
			error += "\nvs_hull("+String(i)+")-error in "+local+": "+vs_hull_field[i].errorStr();
			rs = false;
		}
	for (index_t i = 0; i < accelerator_field.GetLength(); i++)
	{
		if (accelerator_field[i].domain != this)
		{
			error += "\ndomain link of accelerator "+String(i)+" of "+local+" is broken";
			rs = false;
		}
	}
	
	for (index_t i = 0; i < wheel_field.GetLength(); i++)
	{
		if (wheel_field[i].domain != this)
		{
			error += "\ndomain link of wheel "+String(i)+" of "+local+" is broken";
			rs = false;
		}
		if (!wheel_field[i].rotation.valid(super))
		{
			error += "\nrotation-animation of wheel "+String(i)+" ("+wheel_field[i].name+") of "+local+" is broken";
			rs = false;
		}
		if (!wheel_field[i].suspension.valid(super))
		{
			error += "\nsuspension-animation of wheel "+String(i)+" ("+wheel_field[i].name+") of "+local+" is broken";
			rs = false;
		}
	}
	for (index_t i = 0; i < tracks_field.GetLength(); i++)
	{
		if (tracks_field[i].domain != this)
		{
			error += "\ndomain link of construct "+String(i)+" of "+local+" is broken";
			rs = false;
		}
		/*
		if (!tracks_field[i].rotation.valid(super))
		{
			error += "\nconstruct-animation of "+String(i)+" of "+local+" is broken";
			rs = false;
		}
		*/
		for (index_t j = 0; j < tracks_field[i].member_field.GetLength(); j++)
			if (!isMember(tracks_field[i].member_field[j].wheel))
			{
				error += "\nentry "+String(j)+" ('"+tracks_field[i].member_field[j].name+"') of construct "+String(i)+" is no member of "+local;
				rs = false;
			}
	}
	for (index_t i = 0; i < child_field.GetLength(); i++)
	{
		if (!child_field[i].checkIntegrity(super,local))
		{
			error += "\n"+child_field[i].error;
			rs = false;
		}
	}
	return rs;
}

template <class Def> void SubGeometryA<Def>::verifyIntegrity(Geometry<Def>*super, const String&path)
{
	if (!checkIntegrity(super,path))
		FATAL__(error);
}




template <class Def>
template <class Def0>
SubGeometryA<Def>::SubGeometryA(const SubGeometryA<Def0>&other)
{
	operator=(other);
}
	

template <class Def>
SubGeometryA<Def>::SubGeometryA(const SubGeometryA<Def>&other)
{
	operator=(other);
}

template <class Def>
	void	SubGeometryA<Def>::adoptData(SubGeometryA<Def>&other)
	{
		if (this == &other)
			return;
		child_field.adoptData(other.child_field);
		phHull.adoptData(other.phHull);
		vs_hull_field.adoptData(other.vs_hull_field);
		
		tracks_field.adoptData(other.tracks_field);
		accelerator_field.adoptData(other.accelerator_field);
		wheel_field.adoptData(other.wheel_field);



		
		for (index_t i = 0; i < tracks_field.GetLength(); i++)
			tracks_field[i].domain = this;
		for (index_t i = 0; i < accelerator_field.GetLength(); i++)
			accelerator_field[i].domain = this;
		for (index_t i = 0; i < wheel_field.GetLength(); i++)
			wheel_field[i].domain = this;
			
		
		
		name.adoptData(other.name);
		meta = other.meta;
		path = other.path;
		system_link = &path;
	}

template <class Def>
SubGeometryA<Def>& SubGeometryA<Def>::operator=(const SubGeometryA<Def>&other)
{
	child_field = other.child_field;	//self adapts
	phHull = other.phHull;			//self adapts
	vs_hull_field = other.vs_hull_field;	//self adapts
	
	tracks_field = other.tracks_field;	//needs post linking
	accelerator_field = other.accelerator_field;	//needs post linking
	wheel_field = other.wheel_field;			//needs post linking
	name = other.name;
	meta = other.meta;
	path = other.path;
	system_link = &path;
	
	return *this;
}
	
template <class Def>
template <class T>
SubGeometryA<Def>& SubGeometryA<Def>::operator=(const SubGeometryA<T>&other)
{
	child_field = other.child_field;	//self adapts
	phHull = other.phHull;			//self adapts
	vs_hull_field = other.vs_hull_field;	//self adapts
	
	tracks_field = other.tracks_field;	//needs post linking
	accelerator_field = other.accelerator_field;	//needs post linking
	wheel_field = other.wheel_field;			//needs post linking
	name = other.name;
	meta = other.meta;
	path = other.path;
	system_link = &path;
	
	return *this;
}


template <class C> DimensionParser<C>::DimensionParser():set(false)
{}

template <class C>
template <class C0> void DimensionParser<C>::parse(const C0 point[3])
{
	parse(M::Vec::ref3(point));
}

template <class C>
template <class C0> void DimensionParser<C>::parse(const M::TVec3<C0>&point)
{
	if (set)
		dimension.Include(point);
	else
	{
		dimension.Set(point,point);
		set = true;
	}
}

template <class C>
template <class C0> void DimensionParser<C>::parse(const M::Box<C0>&box)
{
	if (set)
	{
		dimension.Include(box);
	}
	else
	{
		dimension = box;
		set = true;
	}
}


template <class Def>
	void			Geometry<Def>::onSystemChange()
	{
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].setSystemRec(root_system.matrix);
	}

	
template <class Def>
	void	Geometry<Def>::clearFlags(UINT32 clear_mask, bool geometries, bool wheels, bool accelerators, bool constructs)
	{
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].clearFlags(clear_mask,geometries,wheels,accelerators,constructs);
	}

template <class Def>
	void	Geometry<Def>::flagAnimationTargets(UINT32 flag)
	{
		for (index_t i = 0; i < animator_field.GetLength(); i++)
		{
			AnimatorA<Def>&a = animator_field[i];
			for (index_t j = 0; j < a.obj_trace_field.GetLength(); j++)
				a.obj_trace_field[j].target->meta.flags |= flag;
			for (index_t j = 0; j < a.whl_trace_field.GetLength(); j++)
				a.whl_trace_field[j].target->flags |= flag;
			for (index_t j = 0; j < a.acc_trace_field.GetLength(); j++)
				a.acc_trace_field[j].target->flags |= flag;
		}
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].flagAnimationTargets(flag);
	}


template <class Def> Geometry<Def>::Geometry():root_system(this),system_link(&root_system.matrix),texture_resource(&local_textures)
{}

template <class Def> Geometry<Def>::Geometry(Riff::File&source):root_system(this),system_link(&root_system.matrix),texture_resource(&local_textures)
{
	loadFromRiff(source);
}

template <class Def> Geometry<Def>::Geometry(const Geometry<Def>&other):root_system(this),system_link(&root_system.matrix),texture_resource(&local_textures)
{
	copy(other);
}

template <class Def>
template <class Def0> Geometry<Def>::Geometry(const Geometry<Def0>&other):root_system(this),system_link(&root_system.matrix),texture_resource(&local_textures)
{
	copy(other);
}



template <class Def>
	void	Geometry<Def>::walkChildren(void (*childCallback)(SubGeometryA<Def>*))
	{
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].walkChildrenRec(childCallback);
	}

template <class Def>
	void	Geometry<Def>::walkChildren(SubGeometryCallback<Def>*callback)
	{
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].walkChildrenRec(callback);
	}

template <class Def>
	void	Geometry<Def>::createVisualMaps(unsigned detail, unsigned max_depth)
	{
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].createVisualMapsRec(detail,max_depth);
	}

template <class Def>template <class C>
	void	Geometry<Def>::scale(const C&factor)
	{
		radius *= factor;
		for (index_t i = 0; i < material_field.GetLength(); i++)
			material_field[i].scale(factor);
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].scale(factor);
		for (index_t i = 0; i < animator_field.GetLength(); i++)
			animator_field[i].scale(factor);
		for (index_t i = 0; i < connector_field.GetLength(); i++)
		{
			M::Vec::mult(connector_field[i].p0,factor);
			M::Vec::mult(connector_field[i].p1,factor);
		}
		M::Vec::mult(root_system.matrix.w.xyz,factor);
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].setSystemRec(root_system.matrix);		
	}
	
template <class Def>
	template <class C>
		void	Geometry<Def>::translate(const M::TVec3<C>&delta)
		{
			for (index_t i = 0; i < object_field.GetLength(); i++)
			{
				M::Vec::add(object_field[i].meta.system.w.xyz,delta);
				object_field[i].setSystemRec(root_system.matrix);		
			}
		}
	
	
template <class Def>
	void	Geometry<Def>::swapComponents(BYTE c0, BYTE c1)
	{
		if (c0 > 2 || c1 > 2 || c0 == c1)
			return;
		for (index_t i = 0; i < object_field.GetLength(); i++)
		{
			typename Def::SystemType*system = object_field[i].meta.system.v;
			swp(system[0+c0],system[0+c1]);
			swp(system[4+c0],system[4+c1]);
			swp(system[8+c0],system[8+c1]);
			swp(system[12+c0],system[12+c1]);
			system[0+c0]*=-1;
			system[4+c0]*=-1;
			system[8+c0]*=-1;
			system[12+c0]*=-1;
			//_mult(system+4*c0,-1);
		}
		for (index_t i = 0; i < animator_field.GetLength(); i++)
		{
			for (index_t j = 0; j < animator_field[i].obj_trace_field.GetLength(); j++)
				for (index_t k = 0; k < animator_field[i].obj_trace_field[j].step_field.GetLength(); k++)
				{
					swp(animator_field[i].obj_trace_field[j].step_field[k].range[c0],animator_field[i].obj_trace_field[j].step_field[k].range[c1]);
					swp(animator_field[i].obj_trace_field[j].step_field[k].range[3+c0],animator_field[i].obj_trace_field[j].step_field[k].range[3+c1]);
					animator_field[i].obj_trace_field[j].step_field[k].range[c0]*=-1;
				}
		}
		for (index_t i = 0; i < connector_field.GetLength(); i++)
		{
			swp(connector_field[i].p0.v[c0],connector_field[i].p0.v[c1]);
			swp(connector_field[i].p1.v[c0],connector_field[i].p1.v[c1]);
			swp(connector_field[i].direction.v[c0],connector_field[i].direction.v[c1]);
			connector_field[i].p0.v[c0]*=-1;
			connector_field[i].p1.v[c0]*=-1;
			connector_field[i].direction.v[c0]*=-1;
		}
		
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].setSystemRec(root_system.matrix);		
	}
	


template <class Def> count_t Geometry<Def>::countFaces()	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countFaces();
	return total;
}

template <class Def> count_t Geometry<Def>::countTriangles()	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countTriangles();
	return total;
}

template <class Def> count_t Geometry<Def>::countQuads()	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countQuads();
	return total;
}


template <class Def> count_t Geometry<Def>::countFaces(unsigned detail_layer)	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countFaces(detail_layer);
	return total;
}

template <class Def> count_t Geometry<Def>::countTriangles(unsigned detail_layer)	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countTriangles(detail_layer);
	return total;
}

template <class Def> count_t Geometry<Def>::countQuads(unsigned detail_layer)	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countQuads(detail_layer);
	return total;
}


template <class Def> count_t Geometry<Def>::countVertices()	const
{
	count_t total = 0;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		total += material_field[i].countVertices();
	return total;
}

template <class Def>
	unsigned		Geometry<Def>::maxDetail()								const
	{
		unsigned result(0);
		
		for (index_t i = 0; i < material_field.GetLength(); i++)
			for (index_t j = 0; j < material_field[i].data.object_field.GetLength(); j++)
				if (material_field[i].data.object_field[j].detail > result)
					result = material_field[i].data.object_field[j].detail;
		return result;
	}



template <class Def> String Geometry<Def>::ToString(bool details)	const
{
	String rs = "Geometry\n";
	rs+=" Vertices: "+String(countVertices())+"\n";
	rs+=" Faces: "+String(countFaces())+" ("+String(countFaces(0))+")\n";
	rs+=" Objects: "+String(object_field.GetLength())+"\n";
	if (details)
		for (index_t i = 0; i < object_field.GetLength(); i++)
			rs+="	("+String(i)+") "+object_field[i].ToString("	");
	rs+=" Materials: "+String(material_field.GetLength())+"\n";
	if (details)
		for (index_t i = 0; i < material_field.GetLength(); i++)
			rs+="	("+String(i)+") "+material_field[i].ToString("	");
	rs+=" Animators: "+String(animator_field.GetLength())+"\n";
	if (details)
		for (index_t i = 0; i < animator_field.GetLength(); i++)
			rs+="	("+String(i)+") "+animator_field[i].ToString("	");
	rs+=" Connectors: "+String(connector_field.GetLength())+"\n";
	if (details)
		for (index_t i = 0; i < connector_field.GetLength(); i++)
			rs+="	("+String(i)+") "+M::Vec::toString(connector_field[i].p0)+" - "+M::Vec::toString(connector_field[i].p1)+" -> "+M::Vec::toString(connector_field[i].direction)+"\n";
	rs+=" "+info;
	return rs;
}


template <class Def>
template <class Def0>Geometry<Def>& Geometry<Def>::operator=(const Geometry<Def0>&other)
{
	copy(other);
	return *this;
}

template <class Def>Geometry<Def>& Geometry<Def>::operator=(const Geometry<Def>&other)
{
	copy(other);
	return *this;
}
/*
template <class Def>void	Geometry<Def>::relink()
{
	for (auto it = animator_field.begin(); it != animator_field.end(); ++it)
		it->link(this);
	for (auto it = object_field.begin(); it != object_field.end(); ++it)
		it->postCopyLink(this);
	for (auto it = material_field.begin(); it != material_field.end(); ++it)
		it->postCopyLink(this,textures);
}*/


template <class Def>void	Geometry<Def>::adoptData(Geometry<Def>&other)
{
	CGS_POINT
	if (this == &other)
		return;
	other.storeTargetNames();

	object_field.adoptData(other.object_field);
	material_field.adoptData(other.material_field);
	animator_field.adoptData(other.animator_field);
	connector_field.adoptData(other.connector_field);
	local_textures.entry_field.adoptData(other.local_textures.entry_field);
	if (other.texture_resource == &other.local_textures)
		texture_resource = &local_textures;
	else
		texture_resource = other.texture_resource;

	object_table.adoptData(other.object_table);
	wheel_table.adoptData(other.wheel_table);
	accelerator_table.adoptData(other.accelerator_table);
	animator_table.adoptData(other.animator_table);

	root_system.matrix = other.root_system.matrix;
	radius = other.radius;
	info = other.info;
	xml.adoptData(other.xml);

	relinkEverything();

	CGS_POINT
}


template <class Def>
template <class Def0>
	void	Geometry<Def>::copy(const Geometry<Def0>&other)
{
	object_field = other.object_field;
	material_field = other.material_field;
	animator_field = other.animator_field;
	connector_field = other.connector_field;
	
	this->local_textures = other.local_textures;
	if (other.texture_resource == &other.local_textures)
		this->texture_resource = &this->local_textures;
	else
		this->texture_resource = other.texture_resource;

	object_table = other.object_table;
	wheel_table = other.wheel_table;
	accelerator_table = other.accelerator_table;
	animator_table = other.animator_table;

	root_system.matrix = other.root_system.matrix;
	radius = other.radius;
	info = other.info;
	xml = other.xml;
	
	system_link = &root_system.matrix;


	for (index_t i = 0; i < animator_field.GetLength(); i++)
		animator_field[i].link(this);
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].postCopyLink(this);
	for (index_t i = 0; i < material_field.GetLength(); i++)
		material_field[i].postCopyLink(this,texture_resource);
}



template <class Def>
	void Geometry<Def>::storeTargetNames()
	{
		for (index_t i = 0; i < animator_field.GetLength(); i++)
			animator_field[i].storeTargetNames();
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].storeTargetNames();
		for (index_t i = 0; i < material_field.GetLength(); i++)
			material_field[i].storeTargetNames();
	}
	
template <class Def>
	void	Geometry<Def>::relinkEverything()
	{
		for (index_t i = 0; i < animator_field.GetLength(); i++)
			animator_field[i].link(this);
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].postCopyLink(this);
		for (index_t i = 0; i < material_field.GetLength(); i++)
			material_field[i].postCopyLink(this,texture_resource == &local_textures ? &local_textures : NULL);	//don't relink external textures
	}





template <class Def> void Geometry<Def>::clear()
{
	object_field.free();
	material_field.free();
	animator_field.free();
	connector_field.free();
	texture_resource = &local_textures;

	info = "";
	xml = "";
	system_link = &root_system.matrix;
}

template <class Def> bool Geometry<Def>::SaveToFile(const PathString&filename)	const
{
	return SaveToFile(filename.c_str());
}

template <class Def> bool Geometry<Def>::SaveToFile(const PathString::char_t*filename)	const
{
	Riff::Chunk	riff;
	UINT32 version = CGS_FILE_VERSION;
	riff.AppendBlock("VERS",&version,sizeof(version));
	saveEmbedded(riff,true);
	return riff.SaveToFile(filename);
}

template <class Def> bool Geometry<Def>::LoadFromFile(const PathString&filename,TextureResource*resource/*=NULL*/)
{
	return LoadFromFile(filename.c_str(),resource);
}

template <class Def> bool Geometry<Def>::LoadFromFile(const PathString::char_t*filename,TextureResource*resource/*=NULL*/)//, bool post_strip, unsigned min_strip)
{
	Riff::File riff;
	if (!riff.Open(filename))
		return false;
	unsigned long version = 0;
	if (riff.FindFirst("VERS") && riff.GetSize() == sizeof(version))
		riff.Get(&version);
	if (version > CGS_FILE_VERSION)
	{
		riff.Close();
		return false;
	}
	loadFromRiff(riff,resource);//, post_strip,min_strip);
	CGS_MSG("loaded '"+String(filename)+"'");
	
	return true;
}

/*
template <class Def> String Geometry<Def>::difference(Geometry*other)
{
	String rs="dif_test";
	
	#define COMPARE(cnt,obj)	rs+= vstructArrayCompare(#cnt,cnt,obj,other->cnt,other->obj)
	
		COMPARE(materials,material);
		COMPARE(objects,object);
//		COMPARE(animators,animator);
//		COMPARE(connectors,connector);

	#undef COMPARE
	return rs;
}*/


template <class Def> void Geometry<Def>::saveEmbedded(Riff::Chunk&riff, bool embed_textures)	const
{

	if (info.GetLength())
		riff.AppendBlock("INFO",info.c_str(),info.GetLength());
	if (xml.GetLength())
	{
		Ctr::Array<BYTE> buffer(xml.GetLength()+4);
		(*(UINT32*)buffer.pointer()) = (UINT32)xml.GetLength();
		size_t compressed = BZ2::compress(xml.c_str(),xml.GetLength(),buffer.pointer()+4,buffer.GetContentSize()-4);
		if (compressed)
			riff.AppendBlock("CXML",buffer.pointer(),compressed+4);
	}
	if (embed_textures)
	{
		Riff::Chunk*tlist = openBlock(riff,DATA_TEXTURES);
			for (index_t i = 0; i < local_textures.entry_field.GetLength(); i++)
			{
				tlist->AppendBlock("ID  ",&local_textures.entry_field[i].name,sizeof(tName));
				for (BYTE k = 0; k < local_textures.entry_field[i].face_field.GetLength(); k++)
					tlist->AppendBlock("FACE",local_textures.entry_field[i].face_field[k]);
			}
	}
	ByteStreamBuffer	buffer;
	
	Riff::Chunk*mlist = openBlock(riff,DATA_MATERIAL);
		for (index_t i = 0; i < material_field.GetLength(); i++)
		{
			Riff::Chunk*mout = mlist->AppendBlock(RIFF_LIST);
			

			buffer.Clear();
			buffer << (UINT32)AlphaTestFlag*material_field[i].info.alpha_test;
			buffer.Append(material_field[i].info.ambient.v,4);
			buffer.Append(material_field[i].info.diffuse.v,4);
			buffer.Append(material_field[i].info.emission.v,4);
			buffer.Append(material_field[i].info.specular.v,4);
			buffer << material_field[i].info.shininess;
			buffer << material_field[i].info.alpha_threshold;
			

			mout->AppendBlock("MATR",buffer);
			mout->AppendBlock("NAME",material_field[i].name.c_str(),material_field[i].name.GetLength()+1);
			for (index_t j = 0; j < material_field[i].info.layer_field.GetLength(); j++)
			{
				buffer.Clear();
				buffer << (UINT32)material_field[i].info.layer_field[j].combiner;
				UINT32 flags = 0;
				if (material_field[i].info.layer_field[j].mirror_map)
					flags |= MirrorFlag;
				if (material_field[i].info.layer_field[j].clamp_x)
					flags |= ClampXFlag;
				if (material_field[i].info.layer_field[j].clamp_y)
					flags |= ClampYFlag;

				buffer << flags;
				buffer << (INT64)(material_field[i].info.layer_field[j].source?material_field[i].info.layer_field[j].source->name:0);
				mout->AppendBlock("TLYR",buffer);
			}

			const MaterialData<Def>&d = material_field[i].data;
			mout->AppendBlock("VCRD",&d.coord_layers,sizeof(UINT16));
			
			for (index_t j = 0; j < d.object_field.GetLength(); j++)
			{
				Riff::Chunk*obj = mout->AppendBlock(RIFF_LIST);

				ASSERT_EQUAL1__(d.object_field[j].vpool.vlyr,d.coord_layers,j);
				
				buffer.Clear();
				buffer << (UINT32)d.object_field[j].vpool.vcnt;
				buffer << (UINT32)d.object_field[j].vpool.vflags;
				buffer << (UINT32)d.object_field[j].detail;
				buffer.Append(d.object_field[j].target->name.c_str(),d.object_field[j].target->name.GetLength());
				
				obj->AppendBlock("ROB3",buffer);

				obj->AppendBlock("RVTX",d.object_field[j].vpool.vdata);

				
				//for (index_t k = 0; k < d.object_field[j].detail_layer_field.GetLength(); k++)
				{
					const IndexContainerA<Def>&chunk = d.object_field[j].ipool;
					obj->AppendBlock("IOBJ",chunk.idata);
					obj->AppendBlock("ICMP",chunk.composition,8);
					
					//obj->AppendBlock("SOBJ",chunk.sdata,chunk.sdata.GetContentSize());
					//obj->AppendBlock("QOBJ",chunk.qdata,chunk.qdata.GetContentSize());
				}
			}
		}
	Riff::Chunk*olist = openBlock(riff,DATA_OBJECTS);
 	for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].saveToRiff(olist);
	Riff::Chunk*alist = openBlock(riff,DATA_ANIMATOR);
		for (index_t i = 0; i < animator_field.GetLength(); i++)
			animator_field[i].saveToRiff(alist);
	Riff::Chunk*clist = openBlock(riff,DATA_CONNECTR);
		clist->AppendBlock("CNCT",connector_field);
}


template <class Def> void Geometry<Def>::loadMaterials(Riff::File&riff)//, bool post_strip, unsigned min_strip)
{
	Buffer<MaterialA<Def>,0,AdoptStrategy>		mbuffer;
	Buffer<RenderObjectA<Def>,0,AdoptStrategy>	obuffer;
	//M::Vector3<IndexContainer<Def> >		cbuffer;
	Buffer<TLayer,0>							lbuffer;

	//TFMaterialInfo<Def> info;
	//TFLayer				layr;
	TRenderObjectInfoIO	rinf;

	count_t total_indices(0);
	

	if (riff.FindFirst(RIFF_LIST))
		do
		{
			riff.Enter();
				if (riff.FindFirst("MATR"))
				{
					MaterialA<Def>&material = mbuffer.Append();
					
					riff.OpenStream();
					UINT32 flags;
					bool success = true;
					success = success && riff.Stream(flags);
					success = success && riff.Stream(material.info.ambient.v,4);
					success = success && riff.Stream(material.info.diffuse.v,4);
					success = success && riff.Stream(material.info.emission.v,4);
					success = success && riff.Stream(material.info.specular.v,4);
					success = success && riff.Stream(material.info.shininess);
					success = success && riff.Stream(material.info.alpha_threshold);
					riff.CloseStream();
					if (!success)
					{
						mbuffer.eraseLast();
						CGS_MSG("Stream errors while streaming material");
						riff.Exit();
						continue;
					}
					
					material.info.alpha_test = flags&AlphaTestFlag;
					
						
					if (riff.FindFirst("NAME"))
					{
						Ctr::Array<char> field(riff.GetSize()+1);
						riff.Get(field.pointer());
						field[field.size()-1] = 0;
						material.name = field;
					}
					
					CGS_MSG("new material ("+material.name+")");
					
					

					if (riff.FindFirst("TLYR"))
						do
						{
							TLayerIO	layr;
							riff.OpenStream();
							
							if (!(riff.Stream(layr.combiner))
								||
								!(riff.Stream(layr.flags))
								||
								!(riff.Stream(layr.sname)))
							{
								riff.CloseStream();
								continue;
							}
							riff.CloseStream();
							
							TLayer&layer = lbuffer.Append();
							layer.source_name = layr.sname;
							layer.cube_map = false;
							layer.mirror_map = !!(layr.flags&MirrorFlag);
							layer.clamp_x = !!(layr.flags&ClampXFlag);
							layer.clamp_y = !!(layr.flags&ClampYFlag);
							layer.combiner = layr.combiner;
						}
						while (riff.FindNext("TLYR"));


					bool valid = false;
					if (riff.FindFirst("VINF") && riff.GetSize() == sizeof(UINT32)*2)
					{
						UINT32 d;
						riff.OpenStream();
						riff.Stream(d);	//ignored
						riff.Stream(d);	material.data.coord_layers = (UINT16)d;
						riff.CloseStream();
						valid = true;
					}
					elif (riff.FindFirst("VCRD") && riff.GetSize() == sizeof(UINT16))
					{
						riff.Get(&material.data.coord_layers);
						valid = true;
					}

					if (valid)
					{
						if (riff.FindFirst(RIFF_LIST))
							do
							{
								riff.Enter();
								{
									RenderObjectA<Def>*object;
									if (riff.FindFirst("ROBJ"))
									{
										if (riff.GetSize() == sizeof(name64_t)+sizeof(rinf.vcnt)+sizeof(UINT32))
										{
											riff.OpenStream();
												name64_t name;
												riff.Stream(name);
												rinf.tname = name2str(name);
												riff.Stream(rinf.vcnt);
												UINT32 dummy;
												riff.Stream(dummy); //deprectaed additional unformated content
											riff.CloseStream();
											rinf.detail = 0;
											rinf.vflags = HasNormalFlag;
										}
										elif (riff.GetSize() == sizeof(name64_t)+sizeof(rinf.vcnt)+sizeof(UINT32)+sizeof(rinf.detail))
										{
											riff.OpenStream();
												name64_t name;
												riff.Stream(name);
												rinf.tname = name2str(name);
												riff.Stream(rinf.vcnt);
												UINT32 dummy;
												riff.Stream(dummy);	//deprectaed additional unformated content
												riff.Stream(rinf.detail);
												rinf.vflags = HasNormalFlag;
											riff.CloseStream();
										}
										else
										{
											CGS_MSG("ROBJ chunk is of unexpected size: "+String(riff.GetSize()));
												ShowMessage(String(riff.GetSize()));
											riff.Exit();
											continue;
										}
									}
									elif (riff.FindFirst("ROB2"))
									{
										if (riff.GetSize() == sizeof(name64_t)+sizeof(rinf.vcnt)+sizeof(rinf.vflags)+sizeof(rinf.detail))
										{
											riff.OpenStream();
												name64_t	name;
												riff.Stream(name);
												rinf.tname = name2str(name);
												riff.Stream(rinf.vcnt);
												riff.Stream(rinf.vflags);
												riff.Stream(rinf.detail);
											riff.CloseStream();
										}
										else
										{
											CGS_MSG("ROB2 chunk is of unexpected size: "+String(riff.GetSize()));
		//										ShowMessage(sizeof(rinf));
											riff.Exit();
											continue;
										}
									}
									elif (riff.FindFirst("ROB3"))
									{
										if (riff.GetSize() > sizeof(rinf.vcnt)+sizeof(rinf.vflags)+sizeof(rinf.detail))
										{
											riff.OpenStream();
												riff.Stream(rinf.vcnt);
												riff.Stream(rinf.vflags);
												riff.Stream(rinf.detail);

												size_t string_length = riff.GetSize() - riff.StreamTell();
												rinf.tname.setLength(string_length);
												riff.Stream(rinf.tname.mutablePointer(),string_length);
											riff.CloseStream();
										}
										else
										{
											CGS_MSG("ROB3 chunk is of unexpected size: "+String(riff.GetSize()));
		//										ShowMessage(sizeof(rinf));
											riff.Exit();
											continue;
										}
									}
									else
									{
										CGS_MSG("unable to find ROBJ, ROB2, or ROB3");
	//										ShowMessage(sizeof(rinf));
										riff.Exit();
										continue;
									}
									if (riff.FindFirst("RVTX"))
									{
										if (riff.GetSize() == rinf.vcnt*VSIZE(material.data.coord_layers,rinf.vflags)*sizeof(typename Def::FloatType))
										{
											object = &obuffer.Append();
											object->tname = rinf.tname;
											object->detail = rinf.detail;
											object->vpool.SetSize(rinf.vcnt,material.data.coord_layers,rinf.vflags);
											riff.Get(object->vpool.vdata);
											CGS_MSG("loaded "+String(object->vpool.vdata.GetLength())+" floats for "+rinf.tname+". loading indices...");
											object->vpool.updateCRC();
										}
										else
										{
											CGS_MSG("RVTX of wrong size ("+String(riff.GetSize())+" != "+String(rinf.vcnt)+"*(3+3+2*"+String(material.data.coord_layers)+"+"+String(rinf.vflags)+")*"+String(sizeof(typename Def::FloatType))+" (="+String(rinf.vcnt*VSIZE(material.data.coord_layers,rinf.vflags)*sizeof(typename Def::FloatType))+"))");
											riff.Exit();
											continue;
										}
									}
									else
									{
										CGS_MSG("RVTX not found");
										riff.Exit();
										continue;
									}
									if (riff.FindFirst("IOBJ"))
										//do
										{
											if (riff.IsMultipleOf(sizeof(typename Def::IndexType)))
											{
												IndexContainerA<Def>*chunk = &object->ipool;
												chunk->idata.SetSize(riff.GetSize()/sizeof(typename Def::IndexType));
												riff.Get(chunk->idata);
												total_indices += chunk->idata.GetLength();
												CGS_MSG("loaded indices: "+String(chunk->idata.GetLength())+" to a total of "+String(total_indices)+" indices");
												if (riff.Next())
												{
													if (riff.IsID("ICMP") && riff.GetSize()==8)
													{
														riff.Get(chunk->composition);
													}
													elif (riff.IsID("SOBJ") && riff.IsMultipleOf(sizeof(typename Def::IndexType)))
													{
														typename Def::IndexType indices;
														riff.OpenStream();
															riff.Stream(indices);
															chunk->triangles = indices/3;
														riff.CloseStream();
														
														if (riff.Next() && riff.IsID("QOBJ") && riff.IsMultipleOf(sizeof(typename Def::IndexType)))
														{
															typename Def::IndexType indices;
															riff.OpenStream();
																riff.Stream(indices);
																chunk->quads = indices/4;
															riff.CloseStream();
															//CGS_MSG("got strip data: "+_toString(chunk->sdata.pointer(),chunk->sdata.GetLength()));
														}
														else
															chunk->quads = 0;
														//CGS_MSG("got strip data: "+_toString(chunk->sdata.pointer(),chunk->sdata.GetLength()));
													}
													else
													{
														//CGS_MSG("generated strip data");
														chunk->triangles = UINT32(chunk->idata.GetLength()/3);
														chunk->quads = 0;
													}
												}
												else
												{
													//CGS_MSG("generated strip data");
													chunk->quads = 0;
												}
											}
											else
												CGS_MSG("odd shaped IOBJ");
										}
										//while (riff.FindNext("IOBJ"));
								}
								riff.Exit();
							}
							while (riff.FindNext(RIFF_LIST));
						obuffer.moveToArray(material.data.object_field);
						//material.data.object_field.adoptData(obuffer);
						/*material->data.object_field.SetSize(obuffer.Count());
						for (index_t i = 0; i < material->data.object_field.GetLength(); i++)
						{
							// CGS_MSG("casting object("+String(i)+"): "+String(obuffer[i]->vpool.vcnt)+"/"+String(obuffer[i]->countIndices()));
							material->data.object_field[i].adoptData(*obuffer[i]);
							// CGS_MSG("  -> "+String(material->data.object_field[i].vpool.vcnt)+" / "+String(material->data.object_field[i].countIndices()));
						}
						obuffer.clear();*/
					}
					lbuffer.moveToArray(material.info.layer_field);
					//material.info.layer_field.adoptData(lbuffer);
						/*.SetSize(lbuffer.Count());
					for (index_t i = 0; i < material->info.layer_field.GetLength(); i++)
						material->info.layer_field[i] = *lbuffer[i];
					lbuffer.clear();*/
				}
			riff.Exit();
		}
		while (riff.FindNext(RIFF_LIST));
	mbuffer.moveToArray(material_field);
	//material_field.adoptData(mbuffer);
	/*material_field.SetSize(mbuffer.Count());
	for (index_t i = 0; i < material_field.GetLength(); i++)
	{
		// CGS_MSG("casting material("+String(i)+"): "+String(mbuffer[i]->data.countChunks()));
		material_field[i].adoptData(*mbuffer[i]);
		// CGS_MSG("  -> "+String(material_field[i].data.countChunks()));
	}
	mbuffer.clear();*/
	
	CGS_MSG("materials loaded: "+String(mbuffer.Count()));
	for (index_t i = 0; i < material_field.GetLength(); i++)
		CGS_MSG("material "+String(i)+" '"+material_field[i].name+"' has "+String(material_field[i].countFaces())+"("+String(material_field[i].countFaces(0))+") face(s) and "+String(material_field[i].countVertices())+" vertices in "+String(material_field[i].data.object_field.GetLength())+" object(s)");
}

template <class Def> void Geometry<Def>::loadObjects(Riff::File&riff)
{
	count_t object_count = 0;
	if (riff.FindFirst("OBJ "))
		do
		{
			if (riff.Next() && riff.IsID(RIFF_LIST))
				object_count++;
		}
		while (riff.FindNext("OBJ "));
	object_field.SetSize(object_count);
	index_t at = 0;
	//Buffer<index_t>	path;
	if (riff.FindFirst("OBJ "))
		do
		{
			if (at < object_count)
			{
				object_field[at].name.setLength(riff.GetSize());
				riff.Get(object_field[at].name.mutablePointer());
				object_field[at].name.TrimThis();
				//riff.Get(&object_field[at].name);
				if (riff.Next() && riff.IsID(RIFF_LIST))
				{
					//path << at;
					riff.Enter();
						object_field[at++].loadFromRiff(riff);
					riff.DropBack();
					//path.eraseLast();
				}
			}
		}
		while (riff.FindNext("OBJ "));
}

template <class Def> void Geometry<Def>::loadAnimators(Riff::File&riff)
{
	count_t count = 0;
	if (riff.FindFirst("ANIM"))
	do
		count++;
	while (riff.FindNext("ANIM"));
	animator_field.SetSize(count);

	Buffer<index_t>	path;	//empty path

	for (index_t i = 0; i < count; i++)
		animator_field[i].loadFromRiff(riff,i);
}

template <class Def> void Geometry<Def>::loadConnectors(Riff::File&riff)
{
	if (riff.FindFirst("CNCT"))
	{
		count_t count = riff.GetSize()/sizeof(TConnector<Def>);
		connector_field.SetSize(count);
		riff.Get(connector_field);
	}
}

template <class Def> void Geometry<Def>::loadTextures(Riff::File&riff)
{
	CGS_POINT
	count_t count = 0;
	if (riff.FindFirst(RIFF_ID))
		do
			if (riff.GetSize() == 8)
				count++;
		while (riff.FindNext(RIFF_ID));
	CGS_POINT
	local_textures.entry_field.SetSize(count);
	CGS_POINT
	TextureA*texture = NULL;
	index_t at = 0;
	if (riff.First())
		do
		{
			switch (Riff::TID(riff.GetChunk().info.sid).Numeric())
			{
				case RIFF_ID:
					if (riff.GetSize() == 8)
					{
						CGS_POINT
						texture = &local_textures.entry_field[at++];
						riff.Get(&texture->name);
						CGS_POINT
					}
				break;
				case RIFF_FACE:
					if (texture)
					{
						CGS_POINT
						index_t index = texture->face_field.GetLength();
						texture->face_field.ResizePreserveContent(index+1);	//this has got to be the second most inefficient way to do this :S
						texture->face_field[index].SetSize(riff.GetSize());
						riff.Get(texture->face_field[index].pointer());
						CGS_POINT
					}
				break;
			}
		}
		while (riff.Next());

	for (index_t i = 0; i < local_textures.entry_field.GetLength(); i++)
		local_textures.entry_field[i].updateHash();
	CGS_POINT
}

template <class Def> void Geometry<Def>::loadEmbedded(Riff::File&riff, TextureResource*resource)//, bool post_strip, unsigned min_strip)
{
	clear();
	
	CGS_POINT
	
	if (riff.FindFirst("CXML") && riff.GetSize())
	{
		Ctr::Array<BYTE> buffer(riff.GetSize());
		riff.Get(buffer.pointer());
		UINT32 extracted = (*(UINT32*)buffer.pointer());
		Ctr::Array<char> extracted_buffer(extracted+1);
		size_t bz_result = BZ2::decompress(buffer.pointer()+4,buffer.GetContentSize()-4,extracted_buffer.pointer(),extracted_buffer.GetContentSize());
		if (bz_result==(size_t)extracted)
		{
			extracted_buffer[extracted] = 0;
			xml = extracted_buffer;
		}
		else
			if (bz_result)
				CGS_MSG("Unable to extract XML content. Stored size is "+String(extracted)+". BZ2 reports "+String((int)bz_result))
			else
				CGS_MSG("Unable to extract XML content: "+String(BZ2::errorStr()));
	}
	
	if (riff.FindFirst("INFO") && riff.GetSize())
	{
		Riff::RIFF_SIZE size = riff.GetSize();
		char*buffer = alloc<char>(size+1);
		riff.Get(buffer);
		buffer[size] = 0;
		for (Riff::RIFF_SIZE i = 0; i < size; i++)
			if (!buffer[i])
				buffer[i] = '\n';
		info = buffer;
		dealloc(buffer);
	}
	else
		if (riff.FindFirst("IDNT") && riff.GetSize())
		{
			Riff::RIFF_SIZE size = riff.GetSize();
			char*buffer = alloc<char>(size+1),
				*first(buffer),*name(NULL),*cont(NULL);
			riff.Get(buffer);
			buffer[size] = 0;
			for (Riff::RIFF_SIZE i = 0; i < size; i++)
				if (!buffer[i])
				{
					if (!name)
						name = first;
					else
						cont = first;
					first = buffer+i+1;
				}
			if (name)
				info = name;
			if (cont)
			{
				if (name)
					info+="\n";
				info += cont;
			}
			dealloc(buffer);
		}
	
	CGS_POINT
	
	if (riff.FindFirst(RIFF_DATA))
		do
			if (riff.GetSize() == sizeof(tName))
			{
				tName name;
				riff.Get(&name);
				if (!riff.Next() || !riff.IsID(RIFF_LIST))
					continue;
				riff.Enter();
					switch (name)
					{
						case DATA_MATERIAL:
							CGS_POINT
							loadMaterials(riff);//,post_strip, min_strip);
							CGS_POINT
						break;
						case DATA_OBJECTS:
							CGS_POINT
							loadObjects(riff);
							CGS_POINT
						break;
						case DATA_TEXTURES:
							CGS_POINT
							loadTextures(riff);
							CGS_POINT
						break;
						case DATA_ANIMATOR:
							CGS_POINT
							loadAnimators(riff);
							CGS_POINT
						break;
						case DATA_CONNECTR:
							CGS_POINT
							loadConnectors(riff);
							CGS_POINT
						break;
					}
				riff.DropBack();
			}
		while (riff.FindNext(RIFF_DATA));

	remap(~RelinkBit);

	if (!resource)
		texture_resource = &local_textures;
	else
		texture_resource = resource;
	CGS_POINT



	
	for (index_t i = 0; i < material_field.GetLength(); i++)
	{
		material_field[i].linkTextures(texture_resource);
		count_t vcnt(0);
		for (index_t j = 0; j < material_field[i].data.object_field.GetLength(); j++)
		{
			RenderObjectA<Def>&obj = material_field[i].data.object_field[j];

			obj.target = lookupObject(obj.tname);
			if (obj.target)
				vcnt ++;
				//#ifdef DEBUG
			else
				FATAL__("unable to find object \""+obj.tname+"\" in object "+String(j)+" material "+String(i));
				//#endif
		}
		if (vcnt < material_field[i].data.object_field.GetLength())
		{
			Ctr::Array<RenderObjectA<Def>,Adopt>	npntr(vcnt);
			vcnt = 0;
			for (index_t j = 0; j < material_field[i].data.object_field.GetLength(); j++)
				if (material_field[i].data.object_field[j].target)
				{
					npntr[vcnt++].adoptData(material_field[i].data.object_field[j]);
					//material_field[i].data.object_field[j].abandon();
				}
			material_field[i].data.object_field.adoptData(npntr);
		}
	}
	CGS_POINT

	
	for (index_t i = 0; i < animator_field.GetLength(); i++)
		animator_field[i].link(this);
	root_system.loadIdentity(false);
	CGS_POINT
	for (index_t i = 0; i < object_field.GetLength(); i++)
	{
		object_field[i].linkAnimators(this);
		object_field[i].setSystemRec(root_system.matrix);
	}
	CGS_POINT
	updateRadius();
	CGS_MSG("finished");
}


template <class Def> void Geometry<Def>::loadFromRiff(Riff::File&riff,TextureResource*resource/*=NULL*/)//, bool post_strip, unsigned min_strip)
{
	loadEmbedded(riff,resource);
}

template <class Def>
	const SubGeometryA<Def>*	Geometry<Def>::lookupChild(const index_t*path_begin, index_t path_length)	const
	{
		ASSERT__(path_length > 0);

		ASSERT_LESS__(*path_begin, object_field.GetLength());

		const SubGeometryA<Def>*current_obj = object_field + *path_begin;

		const index_t	*current = path_begin+1,
						*end = path_begin + path_length;

		while (current != end)
		{
			if (*current >= current_obj->child_field.GetLength())
			{
				FATAL__("Path entry "+String(current-path_begin)+" points to an invalid child of sub geometry '"+current_obj->name+"'");
				return NULL;
			}
			current_obj = current_obj->child_field + *current;
			current++;
		}

		return current_obj;
	}


template <class Def>
	SubGeometryA<Def>*	Geometry<Def>::lookupChild(const index_t*path_begin, index_t path_length)
	{
		ASSERT__(path_length > 0);
		ASSERT__(path_length < (count_t)-10);

		ASSERT_LESS__(*path_begin, object_field.GetLength());

		SubGeometryA<Def>*current_obj = object_field + *path_begin;

		const index_t	*current = path_begin+1,
						*end = path_begin + path_length;

		while (current != end)
		{
			if (*current >= current_obj->child_field.GetLength())
			{
				FATAL__("Path entry "+String(current-path_begin)+" points to an invalid child of sub geometry '"+current_obj->name+"'");
				return NULL;
			}
			current_obj = current_obj->child_field + *current;
			current++;
		}

		return current_obj;
	}

template <class Def> SubGeometryA<Def>*Geometry<Def>::lookupObject(const String&name)
{
	const Path*p = object_table.queryPointer(name);

	return p ? lookupChild(p->pointer(),p->Count()) : NULL;
}

template <class Def> const SubGeometryA<Def>*Geometry<Def>::lookupObject(const String&name) const
{
	const Path*p = object_table.queryPointer(name);

	return p ? lookupChild(p->pointer(),p->Count()) : NULL;
}



template <class Def>
	void		SubGeometryA<Def>::remap(BuildPath&path, Geometry<Def>&geometry, UINT32 bits)
	{
		if (bits & ObjectBit)
			geometry.object_table.Set(name,path.copyToArray());

		if (bits & (WheelBit | AnimatorBit))
			for (index_t i = 0; i < wheel_field.GetLength(); i++)
			{
				path << i;
					if (bits & WheelBit)
						geometry.wheel_table.Set(wheel_field[i].name,path.copyToArray());

					if (bits & AnimatorBit)
					{
						path << 1;
							geometry.animator_table.Set(wheel_field[i].name+'.'+wheel_field[i].suspension.name,path.copyToArray());
						path.eraseLast();
						path << 2;
							geometry.animator_table.Set(wheel_field[i].name+'.'+wheel_field[i].rotation.name,path.copyToArray());
						path.eraseLast();
					}

				path.eraseLast();
			}
		if (bits & AcceleratorBit)
			for (index_t i = 0; i < accelerator_field.GetLength(); i++)
			{
				path << i;
					geometry.accelerator_table.Set(accelerator_field[i].name,path.copyToArray());
				path.eraseLast();
			}

		for (index_t i = 0; i < child_field.GetLength(); i++)
		{
			path << i;
			child_field[i].remap(path,geometry,bits);
			path.eraseLast();
		}
	}



template <class Def>
	void		Geometry<Def>::remap(UINT32 bits)
	{
		if (bits & ObjectBit)
			object_table.clear();
		if (bits & AcceleratorBit)
			accelerator_table.clear();
		if (bits & AnimatorBit)
			animator_table.clear();
		if (bits & WheelBit)
			wheel_table.clear();


		Buffer<index_t,4>	path;
		for (index_t i = 0; i < object_field.GetLength(); i++)
		{
			path << i;

				object_field[i].remap(path,*this, bits);

			path.eraseLast();
			ASSERT__(path.IsEmpty());
		}
		if (bits & AnimatorBit)
			for (index_t i = 0; i < animator_field.GetLength(); i++)
			{
				path << i;
				
				animator_table.Set(animator_field[i].name,path.copyToArray());

				path.eraseLast();
				ASSERT__(path.IsEmpty());
			}
		if (bits & RelinkBit)
			relinkEverything();
	}



template <class Def> TWheel<Def>* Geometry<Def>::lookupWheel(const String&name)
{
	const Path*p = wheel_table.queryPointer(name);
	if (!p)
		return NULL;
	SubGeometryA<Def>*context = lookupChild(p->pointer(),p->Count()-1);

	const index_t*current = p->pointer() + p->Count()-1;
	if (*current >= context->wheel_field.GetLength())
	{
		FATAL__("Path entry "+String(current-p->pointer())+" points to an invalid wheel of sub geometry '"+context->name+"'");
		return NULL;
	}

	return context->wheel_field + *current;
}

template <class Def> const TWheel<Def>* Geometry<Def>::lookupWheel(const String&name) const
{
	const Path*p = wheel_table.queryPointer(name);
	if (!p)
		return NULL;
	const SubGeometryA<Def>*context = lookupChild(p->pointer(),p->Count()-1);

	const index_t*current = p->pointer() + p->Count()-1;
	if (*current >= context->wheel_field.GetLength())
	{
		FATAL__("Path entry "+String(current-p->pointer())+" points to an invalid wheel of sub geometry '"+context->name+"'");
		return NULL;
	}

	return context->wheel_field + *current;
}

template <class Def> TAccelerator<Def>*Geometry<Def>::lookupAccelerator(const String&name)
{
	const Path*p = accelerator_table.queryPointer(name);
	if (!p)
		return NULL;
	SubGeometryA<Def>*context = lookupChild(p->pointer(),p->Count()-1);

	const index_t*current = p->pointer() + p->Count()-1;
	if (*current >= context->accelerator_field.GetLength())
	{
		FATAL__("Path entry "+String(current-p->pointer())+" points to an invalid accelerator of sub geometry '"+context->name+"'");
		return NULL;
	}

	return context->accelerator_field + *current;
}

template <class Def> const TAccelerator<Def>*Geometry<Def>::lookupAccelerator(const String&name) const
{
	const Path*p = accelerator_table.queryPointer(name);
	if (!p)
		return NULL;
	const SubGeometryA<Def>*context = lookupChild(p->pointer(),p->Count()-1);

	const index_t*current = p->pointer() + p->Count()-1;
	if (*current >= context->accelerator_field.GetLength())
	{
		FATAL__("Path entry "+String(current-p->pointer())+" points to an invalid accelerator of sub geometry '"+context->name+"'");
		return NULL;
	}

	return context->accelerator_field + *current;
}

template <class Def> AnimatorA<Def>*Geometry<Def>::lookupAnimator(const String&name)
{
	const Path*p = animator_table.queryPointer(name);
	if (!p)
		return NULL;

	if (p->GetLength() == 1)
	{
		ASSERT_LESS__(p->First(),animator_field.GetLength());
		return animator_field + p->First();
	}

	SubGeometryA<Def>*context = lookupChild(p->pointer(),p->Count()-2);

	const index_t*current = p->pointer() + p->Count()-2;
	if (*current >= context->wheel_field.GetLength())
	{
		FATAL__("Path entry "+String(current-p->pointer())+" points to an invalid wheel of sub geometry '"+context->name+"'");
		return NULL;
	}
	TWheel<Def>*wheel = context->wheel_field+*current;
	current++;
	ASSERT_LESS__(current,p->pointer()+p->Count());	//MARKED TRANSITORY
	switch (*current)
	{
		case 1:
			return &wheel->suspension;
		break;
		case 2:
			return &wheel->rotation;
		break;
	}

	FATAL__("Unexpected final index in path ("+String(current)+"). Expected 1 or 2.");
	return NULL;
}

template <class Def> const AnimatorA<Def>*Geometry<Def>::lookupAnimator(const String&name) const
{
	const Path*p = animator_table.queryPointer(name);
	if (!p)
		return NULL;

	if (p->GetLength() == 1)
	{
		ASSERT_LESS__(p->First(),animator_field.GetLength());
		return animator_field + p->First();
	}

	const SubGeometryA<Def>*context = lookupChild(p->pointer(),p->Count()-2);

	const index_t*current = p->pointer() + p->Count()-2;
	if (*current >= context->wheel_field.GetLength())
	{
		FATAL__("Path entry "+String(current-p->pointer())+" points to an invalid wheel of sub geometry '"+context->name+"'");
		return NULL;
	}
	const TWheel<Def>*wheel = context->wheel_field+*current;
	current++;
	ASSERT_LESS__(current,p->pointer()+p->Count());	//MARKED TRANSITORY
	switch (*current)
	{
		case 1:
			return &wheel->suspension;
		break;
		case 2:
			return &wheel->rotation;
		break;
	}

	FATAL__("Unexpected final index in path ("+String(current)+"). Expected 1 or 2.");
	return NULL;
}


template <class Def>
template <class C> bool Geometry<Def>::extractDimensions(M::Box<C>&dim) const
{
	if (!object_field.GetLength())
		return false;
	DimensionParser<C> parser;
	M::TMatrix4<typename Def::SystemType> matrix = M::Matrix<typename Def::SystemType>::eye4;
	bool def = false;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		def = object_field[i].extractDimensions(matrix,parser)||def;
	dim = parser.dimension;
	return def;
}

template <class Def> void Geometry<Def>::checkLinkage()
{
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].checkLinkage();
}

template <class Def>
template <class C> void Geometry<Def>::extractRadius(C&radius) const
{
	radius = 0;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].extractRadiusRec(radius);
}

template <class Def>
template <class C>
void	Geometry<Def>::extractAverageVisualEdgeLength(unsigned layer, C&length)	const
{
	length = 0;
	count_t count = 0;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].addVisualEdgeLengthSqrRec(layer,length,count);
	length = vsqrt(length);
	if (count)
		length /= (C)count;
}


template <class Def>
template <class C> C Geometry<Def>::extractRadiusAround(const M::TVec3<C>&center) const
{
	C radius = 0;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].extractRadiusRec(center,radius);
	return radius;
}

template <class Def>
template <class C> void Geometry<Def>::resolveWeight(C&weight)					const
{
	weight = 0;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].addWeight(weight);
}



template <class Def> void Geometry<Def>::updateRadius()
{
	extractRadius(radius);
}

template <class Def> void Geometry<Def>::SetSize(count_t objects_, count_t materials_, count_t animators_, count_t connectors_)
{
	object_field.SetSize(objects_);
	material_field.SetSize(materials_);
	animator_field.SetSize(animators_);
	connector_field.SetSize(connectors_);
}

template <class Def> bool Geometry<Def>::isMember(SubGeometryA<Def>*obj) const
{
	if (object_field.Owns(obj))
		return true;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		if (object_field[i].isMember(obj))
			return true;
	return false;
}

template <class Def> bool Geometry<Def>::isMember(TWheel<Def>*whl) const
{
	for (index_t i = 0; i < object_field.GetLength(); i++)
		if (object_field[i].isMember(whl))
			return true;
	return false;
}

template <class Def> bool Geometry<Def>::isMember(TAccelerator<Def>*acc) const
{
	for (index_t i = 0; i < object_field.GetLength(); i++)
		if (object_field[i].isMember(acc))
			return true;
	return false;
}

template <class Def> bool Geometry<Def>::checkIntegrity()
{
	error = "errors:";
	bool rs = true;
	for (index_t i = 0; i < material_field.GetLength(); i++)
		for (index_t j = 0; j < material_field[i].data.object_field.GetLength(); j++)
		{
			if (!isMember(material_field[i].data.object_field[j].target))
			{
				error += "\ntarget-link of object "+String(j)+" (material "+String(i)+") is broken";
				rs = false;
			}
			if (!material_field[i].data.object_field[j].validIndices())
			{
				error += "\nindices broken in object "+String(j)+" (material "+String(i)+")";
				rs = false;
			}
		}
	for (index_t i = 0; i < animator_field.GetLength(); i++)
		if (!animator_field[i].valid(this))
		{
			error += "\nanimator "+String(i)+" is broken";
			rs = false;
		}
	for (index_t i = 0; i < object_field.GetLength(); i++)
	{
		if (!object_field[i].checkIntegrity(this))
		{
			error += "\n"+object_field[i].error;
			rs = false;
		}
	}
	return rs;
}

template <class Def> const String& Geometry<Def>::errorStr()
{
	return error;
}


template <class Def> void Geometry<Def>::verifyIntegrity()
{
	if (!checkIntegrity())
		FATAL__(errorStr());
}

template <class Def> String Geometry<Def>::getLinkageString()
{
	String rs = "(0x"+IntToHex((int)(size_t)system_link,8)+")\n ";
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].defLinkageStr(rs," ");
	return rs;
}

template <class Def> void Geometry<Def>::makeSimpleObject(const Float*vertex,count_t vertices,const Index*index,count_t indices)
{
	makeSimpleObject(vertex,vertices,0,index,indices);
}

template <class Def> void Geometry<Def>::makeSimpleObject(const Float*vertex,count_t vertices,count_t layers, const Index*index,count_t indices)
{
	clear();
	SetSize(1,1,0,0);
	material_field[0].data.object_field.SetSize(1);
	material_field[0].data.coord_layers = (UINT16)(layers);
	RenderObjectA<Def>&robj = material_field[0].data.object_field[0];
	ASSERT__(object_field.GetLength()==1);
	robj.target = object_field.pointer();
	ASSERT_LESS__(layers,0x10000);
	robj.vpool.SetSize(vertices,(UINT16)layers);
	
	robj.vpool.vdata.copyFrom(vertex);
	robj.detail=0;
	robj.ipool.idata.SetSize(indices);
	robj.ipool.triangles = unsigned(indices/3);
	robj.ipool.idata.copyFrom(index);
	object_field[0].vs_hull_field.SetSize(1);
	typedef Mesh<typename SubGeometryA<Def>::VsDef>	VsMesh;
	VsMesh	&obj = object_field[0].vs_hull_field[0];
	obj.vertex_field.SetSize(vertices);
	obj.triangle_field.SetSize(indices/3);
	obj.edge_field.free();
	obj.quad_field.free();
	UINT32 band = UINT32(3+3+layers*2);
	for (index_t i = 0; i < vertices; i++)
	{
		M::Vec::clear(obj.vertex_field[i].position);
		copy3(vertex+i*band,obj.vertex_field[i].position.v);
		obj.vertex_field[i].index = i;
		obj.vertex_field[i].marked = false;
	}
	for (index_t i = 0; i < indices/3; i++)
	{
		obj.triangle_field[i].vertex[0] = obj.vertex_field+index[i*3];
		obj.triangle_field[i].vertex[1] = obj.vertex_field+index[i*3+1];
		obj.triangle_field[i].vertex[2] = obj.vertex_field+index[i*3+2];
		obj.triangle_field[i].index = i;
		obj.triangle_field[i].marked = false;
	}
	ASSERT1__(obj.valid(),obj.errorStr());
	material_field[0].info.layer_field.SetSize(layers);
	for (index_t i = 0; i < layers; i++)
	{
		material_field[0].info.layer_field[i].combiner = 0x2100;
		material_field[0].info.layer_field[i].source = NULL;
	}
	
	
	object_field[0].name = str2name("child");
	M::Mat::Eye(object_field[0].meta.system);
	M::Vec::clear(object_field[0].meta.center);
	object_field[0].meta.radius = 1;
	object_field[0].meta.volume = 1;
	object_field[0].meta.density = 1;
	object_field[0].extractShortestVisualEdgeLength(0,object_field[0].meta.shortest_edge_length);
	M::Mat::Eye(object_field[0].path);
	object_field[0].system_link = &object_field[0].path;
	robj.tname = object_field[0].name;
	
	//verifyIntegrity();
}

template <class Def>
	void	Geometry<Def>::makeFromConstructor(const Constructor<Def>&ctr)
	{
		clear();
		SetSize(ctr.CountObjects(),1,0,0);
		ASSERT__(object_field.GetLength()==ctr.CountObjects());
		material_field[0].data.object_field.SetSize(ctr.CountLODs());
		material_field[0].data.coord_layers = (UINT16)(ctr.CountTextureLayers());
		material_field[0].name = "MainMaterial";
		ASSERT_LESS__(ctr.CountTextureLayers(),0x10000);

		RenderObjectA<Def>*robjects = material_field[0].data.object_field.pointer();
		for (index_t i = 0; i < ctr.CountObjects(); i++)
		{
			const typename Constructor<Def>::Object&cobj = ctr.GetObject(i);
			object_field[i].name = "Object"+String(i);
			object_field[i].vs_hull_field.SetSize(cobj.CountLODs());

			for (index_t l = 0; l < cobj.CountLODs(); l++)
			{
				RenderObjectA<Def>&robj = *(robjects++);
				robj.target = object_field.pointer() + i;
				robj.tname = object_field[i].name;
				robj.vpool.SetSize(cobj.CountVertices(l),(UINT16)ctr.CountTextureLayers(),ctr.GetVertexFlags());
	
				robj.vpool.vdata.copyFrom(cobj.GetVertices(l));
				robj.detail=(unsigned)l;
				robj.ipool.idata.SetSize(cobj.CountTriangleIndices(l) + cobj.CountQuadIndices(l));
				robj.ipool.triangles = unsigned(cobj.CountTriangles(l));
				robj.ipool.quads = unsigned(cobj.CountQuads(l));
				robj.ipool.idata.copyFrom(cobj.GetTriangleIndices(l),cobj.CountTriangleIndices(l));
				robj.ipool.idata.copyFrom(cobj.CountTriangleIndices(l),cobj.GetQuadIndices(l),cobj.CountQuadIndices(l));
			
				typedef Mesh<typename SubGeometryA<Def>::VsDef>	VsMesh;
				VsMesh	&obj = object_field[i].vs_hull_field[l];
				obj.vertex_field.SetSize(cobj.CountVertices(l));
				obj.triangle_field.SetSize(cobj.CountTriangles(l));
				obj.edge_field.free();
				obj.quad_field.SetSize(cobj.CountQuads(l));
				UINT32 band = (UINT32)cobj.GetVertexSize();
				const count_t vertices = cobj.CountVertices(l);
				const Float*vertex = cobj.GetVertices(l);
				for (index_t j = 0; j < vertices; j++)
				{
					M::Vec::clear(obj.vertex_field[j].position);
					copy3(vertex+j*band,obj.vertex_field[j].position.v);
					obj.vertex_field[j].index = j;
					obj.vertex_field[j].marked = false;
				}
				const count_t triangles = cobj.CountTriangles(l);
				const Index*index = cobj.GetTriangleIndices(l);
				for (index_t j = 0; j < triangles; j++)
				{
					obj.triangle_field[j].vertex[0] = obj.vertex_field+index[j*3];
					obj.triangle_field[j].vertex[1] = obj.vertex_field+index[j*3+1];
					obj.triangle_field[j].vertex[2] = obj.vertex_field+index[j*3+2];
					obj.triangle_field[j].index = j;
					obj.triangle_field[j].marked = false;
				}
				const count_t quads = cobj.CountQuads(l);
				index = cobj.GetQuadIndices(l);
				for (index_t j = 0; j < quads; j++)
				{
					obj.quad_field[j].vertex[0] = obj.vertex_field+index[j*4];
					obj.quad_field[j].vertex[1] = obj.vertex_field+index[j*4+1];
					obj.quad_field[j].vertex[2] = obj.vertex_field+index[j*4+2];
					obj.quad_field[j].vertex[3] = obj.vertex_field+index[j*4+3];
					obj.quad_field[j].index = j;
					obj.quad_field[j].marked = false;
				}
				ASSERT1__(obj.valid(),obj.errorStr());
			}

			//M::Mat::Eye(object_field[i].meta.system);
			object_field[i].meta.system = cobj.GetSystem();
			M::Vec::clear(object_field[i].meta.center);
			object_field[i].meta.radius = 1;
			object_field[i].meta.volume = 1;
			object_field[i].meta.density = 1;
			object_field[i].extractShortestVisualEdgeLength(0,object_field[i].meta.shortest_edge_length);
			object_field[i].path = object_field[i].meta.system;
			//M::Mat::Eye(object_field[i].path);
			object_field[i].system_link = &object_field[i].path;
		}
		material_field[0].info = ctr.colors;
		material_field[0].info.layer_field.SetSize(ctr.CountTextureLayers());
		for (index_t i = 0; i < ctr.CountTextureLayers(); i++)
		{
			material_field[0].info.layer_field[i].combiner = 0x2100;
			material_field[0].info.layer_field[i].source = NULL;
		}
	}



template <class Def> template <typename T0>
	void	Geometry<Def>::makeBox(const M::Box<T0>&b)
	{
		M::TVec3<T0>	lv,uv;
		b.GetMin(lv);
		b.GetMax(uv);
		const T0*l = lv.v,
				*u = uv.v;
		Float	vertex_path[] = {
								l[0],l[1],l[2],	-1,0,0,
								l[0],l[1],u[2],	-1,0,0,
								l[0],u[1],u[2],	-1,0,0,
								l[0],u[1],l[2],	-1,0,0,
								
								u[0],l[1],l[2],	0,0,-1,
								l[0],l[1],l[2],	0,0,-1,
								l[0],u[1],l[2],	0,0,-1,
								u[0],u[1],l[2],	0,0,-1,
	
								u[0],l[1],u[2], 1,0,0,
								u[0],l[1],l[2],	1,0,0,
								u[0],u[1],l[2],	1,0,0,
								u[0],u[1],u[2],	1,0,0,

								l[0],l[1],u[2],	0,0,1,
								u[0],l[1],u[2],	0,0,1,
								u[0],u[1],u[2],	0,0,1,
								l[0],u[1],u[2],	0,0,1,

								l[0],l[1],u[2],	0,-1,0,
								l[0],l[1],l[2],	0,-1,0,
								u[0],l[1],l[2],	0,-1,0,
								u[0],l[1],u[2],	0,-1,0,
								
								u[0],u[1],u[2],	0,1,0,
								u[0],u[1],l[2],	0,1,0,
								l[0],u[1],l[2],	0,1,0,
								l[0],u[1],u[2],	0,1,0};
		UINT32 index_path[36];
		for (BYTE k = 0; k < 6; k++)
		{
			index_path[k*2*3] = k*4;
			index_path[k*2*3+1] = k*4+1;
			index_path[k*2*3+2] = k*4+2;
			index_path[(k*2+1)*3] = k*4;
			index_path[(k*2+1)*3+1] = k*4+2;
			index_path[(k*2+1)*3+2] = k*4+3;
		}
		makeSimpleObject(vertex_path,ARRAYSIZE(vertex_path)/6,index_path,ARRAYSIZE(index_path));
		/*float r;
		extractRadius(r);
		if (r > 10000)
			ErrMessage("bad box");*/
					
	}

template <class Def>
	void	Geometry<Def>::makePlane(Float width, Float depth, count_t x_segments, count_t z_segments)
	{
		if (!x_segments || !z_segments)
			return;
		Ctr::Array<Float>	vertex_path((x_segments+1)*(z_segments+1)*6);
		for (index_t x = 0; x < x_segments+1; x++)
			for (index_t z = 0; z < z_segments+1; z++)
			{
				Float*vtx = vertex_path+((x_segments+1)*z+x)*6;
				M::Vec::def(M::Vec::ref3(vtx),((float)x/x_segments-0.5)*width,0,((float)z/z_segments-0.5)*depth);
				M::Vec::def(M::Vec::ref3(vtx+3),0,1,0);
			}
		
		Ctr::Array<UINT32>	index_path((x_segments)*(z_segments)*6);
		UINT32*at = index_path.pointer();
		for (index_t x = 0; x < x_segments; x++)
			for (index_t z = 0; z < z_segments; z++)
			{
				*at++ = UINT32(z*(x_segments+1)+x+1);
				*at++ = UINT32(z*(x_segments+1)+x);
				*at++ = UINT32((z+1)*(x_segments+1)+x+1);
			
				*at++ = UINT32((z+1)*(x_segments+1)+x+1);
				*at++ = UINT32(z*(x_segments+1)+x);
				*at++ = UINT32((z+1)*(x_segments+1)+x);
			}
		ASSERT_CONCLUSION(index_path,at);
		
		makeSimpleObject(vertex_path.pointer(),vertex_path.GetLength()/6,index_path.pointer(),index_path.GetLength());
	}

template <class Def>	
	void	Geometry<Def>::makeBox(Float lower_x, Float lower_y, Float lower_z, Float upper_x, Float upper_y, Float upper_z)
	{
		M::Box<Float> box ( lower_x, lower_y, lower_z,
							upper_x, upper_y, upper_z);
		makeBox(box);
	}

template <class Def> 
	void	Geometry<Def>::makeSphere(Float radius, count_t iterations)
	{
									
		TriangleMesh<float>	tobj;
		_oSphere(tobj,radius,iterations);

		typedef std::set<NormalPoint<float> > PointField;
		PointField						point_field;
		Ctr::Array<PointField::iterator>		map(tobj.vertices);

		index_t counter = 0;
		for (index_t i = 0; i < tobj.vertices; i++)
		{
			NormalPoint<float> np(M::Vec::ref3(tobj.data+i*tobj.band),M::Vec::ref3(tobj.data+i*tobj.band+3));
			np.index = counter;
			auto pair = point_field.insert(np);
			if (pair.second)
				counter++;
			map[i] = pair.first;
		}
		ASSERT_EQUAL__(counter,point_field.size());
		Ctr::Array<float>	vfield(point_field.size()*6);

		foreach (point_field,item)
		{
			index_t at = item->index;
			M::Vec::ref3(vfield+at*6) = *item;
			M::Vec::ref3(vfield+at*6+3) = item->normal;
		}
		
		Ctr::Array<UINT32>	ifield(tobj.vertices);
		for (index_t i = 0; i < tobj.vertices; i++)
			ifield[i] = UINT32(map[i]->index);
		
		makeSimpleObject(vfield.pointer(),vfield.Count(),ifield.pointer(),ifield.GetLength());
	}







template <class Def>
	void		Geometry<Def>::adoptInstance(AnimatableInstance<Def>&instance,bool copy_root_system/*=false*/)
	{
		instance.target = this;
		if (copy_root_system)
			instance.matrix = root_system.matrix;
		else
			instance.loadIdentity(false);
		PointerTable<double*>	map;
		PointerTable<TAnimationStatus*>	map2;
		instance.child_field.SetSize(object_field.GetLength());
		for (index_t i = 0; i < object_field.GetLength(); i++)
		{
			object_field[i].toInstance(instance.child_field[i]);
			instance.child_field[i].mapEntities(map,map2);
		}
		for (index_t i = 0; i < object_field.GetLength(); i++)
			instance.child_field[i].linkEntities(map,map2);
		
		instance.animator_field.SetSize(animator_field.GetLength());
		for (index_t i = 0; i < animator_field.GetLength(); i++)
		{
			instance.animator_field[i].build(animator_field+i);
			instance.animator_field[i].link(map,map2);
		}
		clearFlags();
		flagAnimationTargets();
		instance.updateAnimationMap();
		
		instance.update();
		CGS_MSG(M::Mat::Raw::ToString4x4(instance.child_field.Last().path.v));
	}




template <class Def>
	void		Geometry<Def>::adoptInstance(StaticInstance<Def>&instance,bool copy_root_system/*=false*/)
	{
		instance.target = this;
		if (copy_root_system)
			instance.matrix = root_system.matrix;
		else
			instance.loadIdentity(false);
		instance.child_field.SetSize(object_field.GetLength());
		for (index_t i = 0; i < object_field.GetLength(); i++)
			object_field[i].toInstance(instance.child_field[i]);
		instance.update();
	}



template <class Def> std::shared_ptr<StaticInstance<Def> > Geometry<Def>::createInstance(bool copy_root_system/*=false*/)
{
	std::shared_ptr<StaticInstance<Def> > result(new StaticInstance<Def>());
	adoptInstance(*result, copy_root_system);
	return result;
}

template <class Def> std::shared_ptr<AnimatableInstance<Def> > Geometry<Def>::createAnimatableInstance(bool copy_root_system/*=false*/)
{
	std::shared_ptr<AnimatableInstance<Def> > result(new AnimatableInstance<Def>());
	adoptInstance(*result,copy_root_system);
	return result;
}


template <class Def> void		Geometry<Def>::resetLinkage()
{
	system_link = &root_system.matrix;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		object_field[i].resetLinkageRec();
}

template <class Def> void		Geometry<Def>::resetSystems()
{
	M::Mat::Eye(root_system.matrix);
	for (index_t i = 0; i < object_field.GetLength(); i++)
	{
		object_field[i].resetLinkageRec();
		object_field[i].setSystemRec(root_system.matrix);
	}
}


template <class Def> count_t Geometry<Def>::countVertices(unsigned detail)
{
	count_t rs = 0;
	for (index_t i = 0; i < object_field.GetLength(); i++)
		rs+=object_field[i].countVertices(detail);
	return rs;
}

template <class Def> GeometryInstance<Def>::GeometryInstance():System<typename Def::SystemType>(this),target(NULL)
{}


template <class Def>
	void			StaticInstance<Def>::onSystemChange()
	{
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].updatePath(Super::matrix);
	}




template <class Def> StaticInstance<Def>::StaticInstance()
{}


template <class Def> template <typename T0, typename T1, typename T2>
	bool	StaticInstance<Def>::detectOpticalIntersection(const M::TVec3<T0>&b, const M::TVec3<T1>&d, T2&distance)	const
	{
		for (index_t i = 0; i < child_field.GetLength(); i++)
			if (child_field[i].detectOpticalIntersection(b,d,distance))
				return true;
		return false;
	}


template <class Def> template <typename T0, typename T1, typename T2>
	bool	StaticSubInstanceA<Def>::detectOpticalIntersection(const M::TVec3<T0>&b_, const M::TVec3<T1>&d_, T2&distance)	const
	{
		if (!SubGeometryInstance<Def>::target)
			return false;
		if (SubGeometryInstance<Def>::target->vs_hull_field.GetLength())
		{
			M::TMatrix4<typename Def::FloatType>	inverse;
			M::TVec3<typename Def::FloatType>		d,b;
			M::Mat::invertSystem(SubGeometryInstance<Def>::path,inverse);
			M::Mat::Transform(inverse,b_,b);
			M::Mat::rotate(inverse,d_,d);
			
			typedef Mesh<typename SubGeometryA<Def>::VsDef>	VsMesh;
			const VsMesh	&hull = SubGeometryInstance<Def>::target->vs_hull_field.First();
			for (index_t i = 0; i < hull.triangle_field.GetLength(); i++)
				if (Obj::detectOpticalIntersection(hull.triangle_field[i].v0->position,hull.triangle_field[i].v1->position,hull.triangle_field[i].v2->position,b,d,distance))
					return true;
			for (index_t i = 0; i < hull.quad_field.GetLength(); i++)
				if (Obj::detectOpticalIntersection(hull.quad_field[i].v0->position,hull.quad_field[i].v1->position,hull.quad_field[i].v2->position,b,d,distance)
					||
					Obj::detectOpticalIntersection(hull.quad_field[i].v0->position,hull.quad_field[i].v2->position,hull.quad_field[i].v3->position,b,d,distance))
					return true;
		}
		for (index_t i = 0; i < child_field.GetLength(); i++)
			if (child_field[i].detectOpticalIntersection(b_,d_,distance))
				return true;
		return false;
	}


template <class Def> void StaticInstance<Def>::update()
{
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].updatePath(Super::matrix);
}

template <class Def> void StaticInstance<Def>::restoreFromGeometry()
{
	if (GeometryInstance<Def>::target)
	{
		Super::matrix = GeometryInstance<Def>::target->root_system.matrix;
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].restoreFromGeometry();
		update();
	}
}


template <class Def> void StaticInstance<Def>::link()
{
	ASSERT_NOT_NULL__(GeometryInstance<Def>::target);
	M::TMatrix4<typename Def::SystemType>&m = Super::matrix;
	GeometryInstance<Def>::target->system_link = &m;
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].linkObjects();
}



template <class Def>
	AnimatorInstanceA<Def>::AnimatorInstanceA():target(NULL),progress(0),active(false)
	{}
	
template <class Def>
	void	AnimatorInstanceA<Def>::adoptData(AnimatorInstanceA<Def>&other)
	{
		if (this == &other)
			return;
		target = other.target;
		progress = other.progress;
		active = other.active;
		obj_animator_field.adoptData(other.obj_animator_field);
		whl_animator_field.adoptData(other.whl_animator_field);
		acc_animator_field.adoptData(other.acc_animator_field);
	}

template <class Def>
	void	AnimatorInstanceA<Def>::build(AnimatorA<Def>*target_)
	{
		target = target_;
		if (!target)
		{
			obj_animator_field.free();
			whl_animator_field.free();
			acc_animator_field.free();
			return;
		}
		obj_animator_field.SetSize(target->obj_trace_field.GetLength());
		whl_animator_field.SetSize(target->whl_trace_field.GetLength());
		acc_animator_field.SetSize(target->acc_trace_field.GetLength());
		
		for (index_t i = 0; i < obj_animator_field.GetLength(); i++)
		{
			obj_animator_field[i].target = target->obj_trace_field[i].target;
			obj_animator_field[i].target_trace = target->obj_trace_field+i;
			M::Vec::clear(obj_animator_field[i].current.translation);
			M::Vec::clear(obj_animator_field[i].current.rotation);
			obj_animator_field[i].out_target = NULL;
		}
		for (index_t i = 0; i < whl_animator_field.GetLength(); i++)
		{
			whl_animator_field[i].target = target->whl_trace_field[i].target;
			whl_animator_field[i].target_trace = target->whl_trace_field+i;
			whl_animator_field[i].current_acceleration = 0;
			whl_animator_field[i].current_brake_status = 0;
			whl_animator_field[i].out_target = NULL;
		}
	
		for (index_t i = 0; i < acc_animator_field.GetLength(); i++)
		{
			acc_animator_field[i].target_trace = target->acc_trace_field+i;
			acc_animator_field[i].target = target->acc_trace_field[i].target;
			acc_animator_field[i].current_acceleration = 0;
			acc_animator_field[i].out_target = NULL;
		}
	
	
	}
	
template <class Def>
	void	AnimatorInstanceA<Def>::link(const PointerTable<double*>&map,const PointerTable<TAnimationStatus*>&map2)
	{
		for (index_t i = 0; i < obj_animator_field.GetLength(); i++)
		{
			ASSERT__(map2.Query(obj_animator_field[i].target,obj_animator_field[i].out_target));
		}
		for (index_t i = 0; i < whl_animator_field.GetLength(); i++)
		{
			ASSERT__(map.Query(whl_animator_field[i].target,whl_animator_field[i].out_target));
		}
		for (index_t i = 0; i < acc_animator_field.GetLength(); i++)
		{
			ASSERT__(map.Query(acc_animator_field[i].target,acc_animator_field[i].out_target));
		}
	}
	
template <class Def>
	void	AnimatorInstanceA<Def>::update()
	{
		for (index_t i = 0; i < obj_animator_field.GetLength(); i++)
		{
			double field[6];
			obj_animator_field[i].target_trace->current(progress,field);
			obj_animator_field[i].current.translation = M::Vec::ref3(field);
			obj_animator_field[i].current.rotation = M::Vec::ref3(field+3);
			//if (_dot(obj_animator_field[i].current+3))
				//CGS_MSG("Adding "+_toString(obj_animator_field[i].current+3)+" to target, which is "+_toString(obj_animator_field[i].out_target+3));
			M::Vec::add(obj_animator_field[i].out_target->translation,obj_animator_field[i].current.translation);
			M::Vec::add(obj_animator_field[i].out_target->rotation,obj_animator_field[i].current.rotation);
		}
		for (index_t i = 0; i < whl_animator_field.GetLength(); i++)
		{
			whl_animator_field[i].target_trace->current(progress,whl_animator_field[i].current);
			M::Vec::add(M::Vec::ref2(whl_animator_field[i].out_target),M::Vec::ref2(whl_animator_field[i].current));
		}
		for (index_t i = 0; i < acc_animator_field.GetLength(); i++)
		{
			acc_animator_field[i].target_trace->current(progress,&acc_animator_field[i].current_acceleration);
			(*acc_animator_field[i].out_target) += acc_animator_field[i].current_acceleration;
		}
	}



template <class Def>
	void			AnimatableInstance<Def>::onSystemChange()
	{
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].updatePath(Super::matrix);
	}
	
template <class Def>
	void			AnimatableInstance<Def>::updateAnimationMap()
	{
		animation_map.sub_animator_field.reset();
		animation_map.animated_sub_field.reset();
		animation_map.animated_wheel_status_field.reset();
		animation_map.animated_accel_status_field.reset();
		
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].fillAnimationMap(animation_map);
		
		animation_map.sub_animator_field.compact();
		animation_map.animated_sub_field.compact();
		animation_map.animated_wheel_status_field.compact();
		animation_map.animated_accel_status_field.compact();
		
	}
	
	

	
template <class Def>
	AnimatorInstanceA<Def>*				AnimatableInstance<Def>::findAnimator(const String&name)
	{
		const Path*p = target->animator_table.queryPointer(name);
		if (!p)
			return NULL;
		ASSERT_EQUAL__(p->size(),1);
		ASSERT_LESS__(p->First(),animator_field.GetLength());
		return animator_field + p->First();
	}

template <class Def>
	AnimatorInstanceA<Def>*				AnimatableInstance<Def>::findInstanceOf(AnimatorA<Def>*animator)
	{
		if (!GeometryInstance<Def>::target || !animator)
			return NULL;
		if (GeometryInstance<Def>::target->animator_field.Owns(animator))
			return animator_field+GeometryInstance<Def>::target->animator_field.GetIndexOf(animator);
		for (index_t i = 0; i < child_field.GetLength(); i++)
			if (AnimatorInstanceA<Def>*rs = child_field[i].findInstanceOf(animator))
				return rs;
		return NULL;
	}


template <class Def> AnimatableInstance<Def>::AnimatableInstance()
{}	

template <class Def>
	void		AnimatableInstance<Def>::update()
	{
		if (!GeometryInstance<Def>::target)
			return;
		for (index_t i = 0; i < animation_map.animated_sub_field.GetLength(); i++)
		{
			M::Vec::clear(animation_map.animated_sub_field[i]->animation_status.translation);
			M::Vec::clear(animation_map.animated_sub_field[i]->animation_status.rotation);
		}
		for (index_t i = 0; i < animation_map.animated_wheel_status_field.GetLength(); i++)
			M::Vec::clear(M::Vec::ref2(animation_map.animated_wheel_status_field[i]));
		for (index_t i = 0; i < animation_map.animated_accel_status_field.GetLength(); i++)
			(*animation_map.animated_accel_status_field[i])=0;
		
		for (index_t i = 0; i < animator_field.GetLength(); i++)
			animator_field[i].update();
		for (index_t i = 0; i < animation_map.sub_animator_field.GetLength(); i++)
			animation_map.sub_animator_field[i]->update();

		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].updatePath(Super::matrix);
	}
	
template <class Def>
	void		AnimatableInstance<Def>::link()
	{
		ASSERT_NOT_NULL__(GeometryInstance<Def>::target);
		GeometryInstance<Def>::target->system_link = Super::getMatrix();
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].linkObjects();
	}
	
template <class Def>
	double*		AnimatableInstance<Def>::findStatusOfWheel(const Ctr::ArrayData<index_t>&path)
	{
		if (!path.GetLength() || path.First() >= child_field.GetLength())
			return NULL;
		AnimatableSubInstanceA<Def>*child = child_field+path.First();
		for (index_t i = 1; i+1 < path.GetLength(); i++)
			if (path[i] >= child->child_field.GetLength())
				return NULL;
			else
				child = child->child_field+path[i];
		if (path.Last()*2 >= child->wheel_status.GetLength())
			return NULL;
		return child->wheel_status+path.Last()*2;
	}
	
template <class Def>
	double*		AnimatableInstance<Def>::findStatusOfAccelerator(const Ctr::ArrayData<index_t>&path)
	{
		if (!path.GetLength() || path.First() >= child_field.GetLength())
			return NULL;
		AnimatableSubInstanceA<Def>*child = child_field+path.First();
		for (index_t i = 1; i+1 < path.GetLength(); i++)
			if (path[i] >= child->child_field.GetLength())
				return NULL;
			else
				child = child->child_field+path[i];
		if (path.Last() >= child->accelerator_status.GetLength())
			return NULL;
		return child->accelerator_status+path.Last();
	}

template <class Def>
	AnimatorInstanceA<Def>*		AnimatableInstance<Def>::findRotationAnimatorOfWheel(const Ctr::ArrayData<index_t>&path)
	{
		if (!path.GetLength() || path.First() >= child_field.GetLength())
			return NULL;
		AnimatableSubInstanceA<Def>*child = child_field+path.First();
		for (index_t i = 1; i+1 < path.GetLength(); i++)
			if (path[i] >= child->child_field.GetLength())
				return NULL;
			else
				child = child->child_field+path[i];
		if (path.Last() >= child->wheel_rotation_animators.Count())
			return NULL;
		return child->wheel_rotation_animators+path.Last();
	}

template <class Def>
	AnimatorInstanceA<Def>*		AnimatableInstance<Def>::findSuspensionAnimatorOfWheel(const Ctr::ArrayData<index_t>&path)
	{
		if (!path.GetLength() || path.First() >= child_field.GetLength())
			return NULL;
		AnimatableSubInstanceA<Def>*child = child_field+path.First();
		for (index_t i = 1; i+1 < path.GetLength(); i++)
			if (path[i] >= child->child_field.GetLength())
				return NULL;
			else
				child = child->child_field+path[i];
		if (path.Last() >= child->wheel_suspension_animators.Count())
			return NULL;
		return child->wheel_suspension_animators+path.Last();
	}

template <class Def>
	double*		AnimatableInstance<Def>::findStatusOf(TWheel<Def>*wheel)
	{
		if (!wheel)
			return NULL;
		for (index_t i = 0; i < child_field.GetLength(); i++)
		{
			if (double*rs = child_field[i].findStatusOf(wheel))
				return rs;
		}
		return NULL;
	}
	
template <class Def>
	void		AnimatableInstance<Def>::restoreFromGeometry(bool reset_animations)
	{
		if (GeometryInstance<Def>::target)
		{
			if (reset_animations)
			{
				for (index_t i = 0; i < animator_field.GetLength(); i++)
					animator_field[i].progress = 0;
			}
			M::Mat::copy(GeometryInstance<Def>::target->root_system.matrix,Super::matrix);
			for (index_t i = 0; i < child_field.GetLength(); i++)
				child_field[i].restoreFromGeometry(reset_animations);
			update();
		}
	}


	
	

template <class Def> SubGeometryInstance<Def>::SubGeometryInstance():target(NULL),system(M::Matrix<typename Def::SystemType>::eye4),path(M::Matrix<typename Def::SystemType>::eye4)
{}


template <class Def> StaticSubInstanceA<Def>::StaticSubInstanceA()
{}



template <class Def> void StaticSubInstanceA<Def>::linkObjects()
{
	if (SubGeometryInstance<Def>::target)
	{
		M::TMatrix4<typename Def::SystemType>&m = SubGeometryInstance<Def>::path;
		SubGeometryInstance<Def>::target->system_link = &m;
	}
	else
		FATAL__("target not found");
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].linkObjects();
}

template <class Def> void StaticSubInstanceA<Def>::updatePath(const M::TMatrix4<typename Def::SystemType>&parent)
{
	M::Mat::transformSystem(parent,SubGeometryInstance<Def>::system,SubGeometryInstance<Def>::path);
	//DBG_ASSERT_LESS__(M::Vec::dot(SubGeometryInstance<Def>::path.x.xyz),10000);
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].updatePath(SubGeometryInstance<Def>::path);
}

template <class Def> void StaticSubInstanceA<Def>::restoreFromGeometry()
{
	if (SubGeometryInstance<Def>::target)
		Super::system = SubGeometryInstance<Def>::target->meta.system;
	else
		FATAL__("target expected");
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].restoreFromGeometry();
}


template <class Def> AnimatableSubInstanceA<Def>::AnimatableSubInstanceA()
{}


template <class Def>
	AnimatorInstanceA<Def>*				AnimatableSubInstanceA<Def>::findInstanceOf(AnimatorA<Def>*animator)
	{
		for (index_t i = 0; i < wheel_rotation_animators.Count(); i++)
			if (wheel_rotation_animators[i].target == animator)
				return wheel_rotation_animators+i;
		/*for (index_t i = 0; i < construct_rotation_animators.Count(); i++)
			if (construct_rotation_animators[i].target == animator)
				return construct_rotation_animators+i;*/
		for (index_t i = 0; i < wheel_suspension_animators.Count(); i++)
			if (wheel_suspension_animators[i].target == animator)
				return wheel_suspension_animators+i;
	
	
		for (index_t i = 0; i < child_field.GetLength(); i++)
			if (AnimatorInstanceA<Def>*rs = child_field[i].findInstanceOf(animator))
				return rs;
		return NULL;
	}
	
template <class Def>
	double*		AnimatableSubInstanceA<Def>::findStatusOf(TWheel<Def>*wheel)
	{
		if (SubGeometryInstance<Def>::target->wheel_field.Owns(wheel))
		{
			return wheel_status+2*SubGeometryInstance<Def>::target->wheel_field.GetIndexOf(wheel);
		}
		for (index_t i = 0; i < child_field.GetLength(); i++)
		{
			if (double*rs = child_field[i].findStatusOf(wheel))
				return rs;
		}
		return NULL;
	}


template <class Def> void AnimatableSubInstanceA<Def>::linkObjects()
{
	if (SubGeometryInstance<Def>::target)
		SubGeometryInstance<Def>::target->system_link = Super::GetPath();
	else
		FATAL__("target not found");
	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].linkObjects();
}

#ifndef __CUDACC__

template <class Def> void AnimatableSubInstanceA<Def>::updatePath(const M::TMatrix4<typename Def::SystemType>&parent)
{
	M::TVec3<>	vector;
	float	angle;
	M::TMatrix3<>	rotation_matrix;
	M::TMatrix4<>	final_matrix;

	angle = M::Vec::length(animation_status.rotation);

	if (vabs(angle)>GetError<double>())
	{
		ASSERT__(target->meta.flags&AnimationTargetFlag);
		M::Vec::div(animation_status.rotation,angle,vector);
		M::Mat::BuildRotationMatrix(angle,vector,rotation_matrix);
		M::Mat::Mult(rotation_matrix,system.x.xyz,animated_system.x.xyz);
		M::Mat::Mult(rotation_matrix,system.y.xyz,animated_system.y.xyz);
		M::Mat::Mult(rotation_matrix,system.z.xyz,animated_system.z.xyz);
		//CGS_MSG("Angle is "+String(angle)+". vector is "+_toString(vector)+". base vector was "+_toString(animation_status+3));
	}
	else
		M::Mat::copyOrientation(system,animated_system);
	M::Vec::add(system.w.xyz,animation_status.translation,animated_system.w.xyz);	//translation

	M::Mat::resetBottomRow(animated_system);
	//copy3(animation_status,final_matrix+12);
	/*if (_dot(animation_status))
		CGS_MSG("Translation is "+_toString(final_matrix+12));*/
	
	M::Mat::transformSystem(parent,animated_system,path);

	for (index_t i = 0; i < child_field.GetLength(); i++)
		child_field[i].updatePath(path);
}

template <class Def>
	void AnimatableSubInstanceA<Def>::restoreFromGeometry(bool reset_animations)
	{
		if (reset_animations)
		{
			for (index_t i = 0; i < wheel_rotation_animators.GetLength(); i++)
				wheel_rotation_animators[i].progress = 0;
			for (index_t i = 0; i < wheel_suspension_animators.GetLength(); i++)
				wheel_suspension_animators[i].progress = 0;
			/*for (index_t i = 0; i < construct_rotation_animators.GetLength(); i++)
				construct_rotation_animators[i].progress = 0;*/
		}
		if (target)
			system = target->meta.system;
		else
			FATAL__("target expected");
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].restoreFromGeometry(reset_animations);
	}
#endif

template <class Def>
	void AnimatableSubInstanceA<Def>::mapEntities(PointerTable<double*>&map, PointerTable<TAnimationStatus*>&map2)
	{
		map2.Set(target,&animation_status);
		for (index_t i = 0; i < target->wheel_field.GetLength(); i++)
			map.Set(target->wheel_field+i,wheel_status+2*i);
		for (index_t i = 0; i < target->accelerator_field.GetLength(); i++)
			map.Set(target->accelerator_field+i,accelerator_status+i);
	
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].mapEntities(map,map2);
	}
	
template <class Def>
	void AnimatableSubInstanceA<Def>::linkEntities(const PointerTable<double*>&map, const PointerTable<TAnimationStatus*>&map2)
	{
		for (index_t i = 0; i < wheel_rotation_animators.GetLength(); i++)
			wheel_rotation_animators[i].link(map,map2);
		for (index_t i = 0; i < wheel_suspension_animators.GetLength(); i++)
			wheel_suspension_animators[i].link(map,map2);
		/*for (index_t i = 0; i < construct_rotation_animators.GetLength(); i++)
			construct_rotation_animators[i].link(map,map2);*/

		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].linkEntities(map,map2);
	}

#ifndef __CUDACC__

template <class Def>
	void AnimatableSubInstanceA<Def>::	fillAnimationMap(typename AnimatableInstance<Def>::TAnimationMap&map)
	{
		if (target->meta.flags & AnimationTargetFlag)
			map.animated_sub_field << this;
		map.sub_animator_field.appendAddresses(wheel_rotation_animators);
		map.sub_animator_field.appendAddresses(wheel_suspension_animators);
		//map.sub_animator_field.appendAddresses(construct_rotation_animators);
		
		for (index_t i = 0; i < target->wheel_field.GetLength(); i++)
			if (target->wheel_field[i].flags & AnimationTargetFlag)
				map.animated_wheel_status_field << wheel_status+2*i;
		for (index_t i = 0; i < target->accelerator_field.GetLength(); i++)
			if (target->accelerator_field[i].flags & AnimationTargetFlag)
				map.animated_accel_status_field << wheel_status+i;
		
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].fillAnimationMap(map);
	}
#endif
	
	
template <class Def>
	void AnimatableSubInstanceA<Def>::resetAnimationStatus()
	{
		Vec::clear(animation_status.translation);
		Vec::clear(animation_status.rotation);
		wheel_status.Fill(0);
		accelerator_status.Fill(0);
	
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].resetAnimationStatus();
	}	
template <class Def>
	void AnimatableSubInstanceA<Def>::updateAnimations()
	{
		for (index_t i = 0; i < wheel_rotation_animators.GetLength(); i++)
			wheel_rotation_animators[i].update();
		for (index_t i = 0; i < wheel_suspension_animators.GetLength(); i++)
			wheel_suspension_animators[i].update();
		// for (index_t i = 0; i < construct_rotation_animators.GetLength(); i++)
			// construct_rotation_animators[i].update();
	
		for (index_t i = 0; i < child_field.GetLength(); i++)
			child_field[i].updateAnimations();
	}



template <class Def> AnimatorA<Def>::AnimatorA():name(0)
{}

template <class Def> 
	void	AnimatorA<Def>::revert()
	{
		for (index_t i = 0; i < obj_trace_field.GetLength(); i++)
			obj_trace_field[i].revert();
		for (index_t i = 0; i < acc_trace_field.GetLength(); i++)
			acc_trace_field[i].revert();
		for (index_t i = 0; i < whl_trace_field.GetLength(); i++)
			whl_trace_field[i].revert();
	}


template <class Def> 
	double	AnimatorA<Def>::GetLength()	const
	{
		double rs = 0;
		for (index_t i = 0; i < obj_trace_field.Count(); i++)
			if (obj_trace_field[i].step_field.GetLength())
				rs = vmax(rs,obj_trace_field[i].step_field.Last().end);
		for (index_t i = 0; i < acc_trace_field.Count(); i++)
			if (acc_trace_field[i].step_field.GetLength())
				rs = vmax(rs,acc_trace_field[i].step_field.Last().end);
		for (index_t i = 0; i < whl_trace_field.Count(); i++)
			if (whl_trace_field[i].step_field.GetLength())
				rs = vmax(rs,whl_trace_field[i].step_field.Last().end);
		return rs;
	}

template <class Def>	template <class C>
	void	AnimatorA<Def>::scale(const C&factor)
	{
		for (index_t i = 0; i < obj_trace_field.Count(); i++)
			for (index_t j = 0; j < obj_trace_field[i].step_field.GetLength(); j++)
			{
				M::Vec::mult(M::Vec::ref3(obj_trace_field[i].step_field[j].range),factor);	//only affect translaction
				M::Vec::mult(M::Vec::ref3(obj_trace_field[i].step_field[j].slope0),factor);
				M::Vec::mult(M::Vec::ref3(obj_trace_field[i].step_field[j].slope1),factor);
			}
	}


template <class Def> template <class Obj, unsigned Band>
	void AnimatorA<Def>::storeTargetNames(Ctr::ArrayData<TraceA<Obj,Band> >&field)
	{
		for (index_t i = 0; i < field.GetLength(); i++)
			field[i].tname = field[i].target->name;
	}

template <class Def> void AnimatorA<Def>::storeTargetNames()
{
	storeTargetNames(obj_trace_field);
	storeTargetNames(acc_trace_field);
	storeTargetNames(whl_trace_field);
}

template <class Def> bool AnimatorA<Def>::valid(Geometry<Def>*structure)
{
	for (index_t i = 0; i < obj_trace_field.GetLength(); i++)
		if (!structure->isMember(obj_trace_field[i].target))
			return false;
	for (index_t i = 0; i < acc_trace_field.GetLength(); i++)
		if (!structure->isMember(acc_trace_field[i].target))
			return false;
	for (index_t i = 0; i < whl_trace_field.GetLength(); i++)
		if (!structure->isMember(whl_trace_field[i].target))
			return false;
	return true;
}

template <class Def> String AnimatorA<Def>::ToString(const String&intend)	const
{
	return "Animator (ObjecCTraces: "+String(obj_trace_field.GetLength())+"; AcceleratorTraces: "+String(acc_trace_field.GetLength())+"; WheelTraces: "+String(whl_trace_field.GetLength())+")";
}

template <class Def> void AnimatorA<Def>::SetSize(count_t obj_count, count_t acc_count, count_t whl_count)
{
	obj_trace_field.SetSize(obj_count);
	acc_trace_field.SetSize(acc_count);
	whl_trace_field.SetSize(whl_count);
}


template <class Def>
template <class Def0> AnimatorA<Def>& AnimatorA<Def>::operator=(const AnimatorA<Def0>&source)
{
	name = source.name;
	obj_trace_field = source.obj_trace_field;
	acc_trace_field = source.acc_trace_field;
	whl_trace_field = source.whl_trace_field;
	return *this;
}

template <class Def> void AnimatorA<Def>::adoptData(AnimatorA<Def>&other)
{
	if (this == &other)
		return;
	name.adoptData(other.name);
	obj_trace_field.adoptData(other.obj_trace_field);
	acc_trace_field.adoptData(other.acc_trace_field);
	whl_trace_field.adoptData(other.whl_trace_field);
}


template <class T, unsigned B> TraceA<T,B>::TraceA():target(NULL)
{}

template <class T, unsigned B>
	void	TraceA<T,B>::revert()
	{
		step_field.revert();
		for (index_t i = 0; i < step_field.GetLength(); i++)
		{
			VecUnroll<B>::mult(step_field[i].range,-1);
			VecUnroll<B>::mult(step_field[i].slope0,-1);
			VecUnroll<B>::mult(step_field[i].slope1,-1);
		}
	}
	
template <class T, unsigned B>
	void	TraceA<T,B>::adoptData(TraceA<T,B>&other)
	{
		if (this == &other)
			return;
		tname.adoptData(other.tname);
		target = other.target;
		step_field.adoptData(other.step_field);
	}
	
template <class T, unsigned B>
	void	TraceA<T,B>::calculateSlopes()
	{
		if (!step_field.GetLength())
			return;
		//CGS_MSG("Auto generating slopes for trace of length "+String(step_field.GetLength()));
		VecUnroll<B>::div(step_field.First().range,step_field.First().end-step_field.First().start,step_field.First().slope0);
		//CGS_MSG("Initial slope calculated at "+_toString(step_field.First().slope0,B));
		for (index_t i = 0; i < step_field.GetLength(); i++)
		{
			double v0[B],v1[B],v2[B];
			if (i)
				VecUnroll<B>::div(step_field[i-1].range,step_field[i-1].end-step_field[i-1].start,v0);
			VecUnroll<B>::div(step_field[i].range,step_field[i].end-step_field[i].start,v1);
			if (i+1 < step_field.GetLength())
				VecUnroll<B>::div(step_field[i+1].range,step_field[i+1].end-step_field[i+1].start,v2);
			if (i)
				VecUnroll<B>::center(v0,v1,step_field[i].slope0);
			if (i+1 < step_field.GetLength())
				VecUnroll<B>::center(v1,v2,step_field[i].slope1);
		}
		VecUnroll<B>::div(step_field.Last().range,step_field.Last().end-step_field.Last().start,step_field.Last().slope1);
		//CGS_MSG("Final slope calculated at "+_toString(step_field.Last().slope1,B));
	}


template <class T, unsigned B>
	void	TraceA<T,B>::current(const double&time, double out[B])	const
	{
		VecUnroll<B>::clear(out);
		if (!step_field.GetLength() || time <= step_field.First().start)
			return;
	
		for (index_t i = 0; i < step_field.GetLength(); i++)
		{
			//CGS_MSG("Processing step "+String(i)+"/"+String(step_field.GetLength()));
			const TStep<B>&step = step_field[i];
			if (time < step.start)
				return;
			if (time > step.end)
			{
				//CGS_MSG("Exceeded. Skipping to next step (if any)");
				VecUnroll<B>::add(out,step.range);
				continue;
			}
			
			double	len = step.end-step.start,
					fc = (time-step.start)/(len),
					a[B],
					b[B],
					c[B];
					//s0[B],
					//s1[B];
/* 			CGS_MSG("start="+String(step.start));
			CGS_MSG("end="+String(step.end));
			CGS_MSG("fc="+String(fc)+" (time="+String(time)+")");
			CGS_MSG("slope0="+_toString(step.slope0,B));
			CGS_MSG("slope1="+_toString(step.slope1,B));
			CGS_MSG("range="+_toString(step.range,B)); */
			//_divV<double,double,double,B>(step.slope0,len,s0);
			//_divV<double,double,double,B>(step.slope1,len,s1);

			VecUnroll<B>::add(step.slope0,step.slope1,a);
			VecUnroll<B>::mad(a,step.range,-2);	//a = s0+s1-2*delta
			
			VecUnroll<B>::mult(step.range,3,b);
			VecUnroll<B>::sub(b,step.slope1);
			VecUnroll<B>::mad(b,step.slope0,-2);	//b = 3*delta-s1-2*s0
			
			VecUnroll<B>::copy(step.slope0,c);	//c = s0
		/* 	
			CGS_MSG("a="+_toString(a,B));
			CGS_MSG("b="+_toString(b,B));
			CGS_MSG("c="+_toString(c,B)); */
			
			
			VecUnroll<B>::mad(out,a,fc*fc*fc);
			VecUnroll<B>::mad(out,b,fc*fc);
			VecUnroll<B>::mad(out,c,fc);
			return;
		}
	}
	

template <class Def>
template <class T, unsigned B>
/*static*/	void AnimatorA<Def>::saveTraces(const Ctr::ArrayData<TraceA<T,B> >&traces,Riff::Chunk*riff,const char*riff_name)
{
	for (index_t i = 0; i < traces.Count(); i++)
	{
		ByteStreamBuffer trace;
		trace << (UINT32)traces[i].target->name.GetLength();
		trace.Append(traces[i].target->name.c_str(),traces[i].target->name.GetLength());
		double delta;
		for (index_t j = 0; j < traces[i].step_field.GetLength(); j++)
		{
			delta = traces[i].step_field[j].end - traces[i].step_field[j].start;
			trace.Append(delta);
			trace.Append(traces[i].step_field[j].range,B);
			trace.Append(traces[i].step_field[j].slope0,B);
			trace.Append(traces[i].step_field[j].slope1,B);
		}
		riff->AppendBlock(riff_name,trace);
	}
}



template <class Def>
template <class T, unsigned B>
void AnimatorA<Def>::loadTraces(Ctr::ArrayData<TraceA<T,B> >&array,Riff::File&riff,const char*riff_name0, const char*riff_name1)
{
	index_t format = 0;
	const char* riff_name = riff_name0;
	bool begin = false;
	count_t cnt = 0;
	if (riff.FindFirst(riff_name))
	{
		begin = true;
	}
	else
		if (riff.FindFirst(riff_name1))
		{
			format = 1;
			begin = true;
			riff_name = riff_name1;
		}
	if (begin)
		do
		{
			cnt++;
		}
		while (riff.FindNext(riff_name));
	array.SetSize(cnt);
	index_t at = 0;
	if (begin)
	if (riff.FindFirst(riff_name))
		do
		{
			TraceA<T,B>&trace = array[at++];
			switch (format)
			{
				case 0:
				{
					if (riff.GetSize() < sizeof(name64_t))
						continue;
					trace.step_field.SetSize((riff.GetSize()-sizeof(name64_t))/((1+B*3)*sizeof(double)));
					riff.OpenStream();
						name64_t	name64;
						riff.Stream(name64);
						trace.tname = name2str(name64);
						trace.tname.TrimThis();

						double time(0),len;
						for (index_t i = 0; i < trace.step_field.GetLength(); i++)
						{
							trace.step_field[i].start = time;
							riff.Stream(len);
							time+=len;
							trace.step_field[i].end = time;
							riff.Stream(trace.step_field[i].range,B);
							riff.Stream(trace.step_field[i].slope0,B);
							riff.Stream(trace.step_field[i].slope1,B);
						}
					riff.CloseStream();
				}
				break;
				case 1:
				{
					riff.OpenStream();
						UINT32 name_length;
						if (!riff.Stream(name_length) || !riff.CanStreamBytes(name_length))
						{
							riff.CloseStream();
							continue;
						}
						trace.tname.setLength(name_length);
						riff.Stream(trace.tname.mutablePointer(),name_length);
						trace.step_field.SetSize((riff.GetSize()-riff.StreamTell())/((1+B*3)*sizeof(double)));
						double time(0),len;
						for (index_t i = 0; i < trace.step_field.GetLength(); i++)
						{
							trace.step_field[i].start = time;
							riff.Stream(len);
							time+=len;
							trace.step_field[i].end = time;
							riff.Stream(trace.step_field[i].range,B);
							riff.Stream(trace.step_field[i].slope0,B);
							riff.Stream(trace.step_field[i].slope1,B);
						}
					riff.CloseStream();
				}
				break;
			}
		}
		while (riff.FindNext(riff_name));
}


template <class Def> void AnimatorA<Def>::loadFromRiff(Riff::File&riff, index_t index_in_context)
{
	bool first = index_in_context == 0;

	if ((first && !riff.FindFirst("ANIM")) || (!first && !riff.FindNext("ANIM")))
		return;
	size_t name_length = riff.GetSize();
	name.setLength(name_length);
	riff.Get(name.mutablePointer());
	name.TrimThis();


	if (!riff.FindNext("LIST"))
		return;
	riff.Enter();
		loadTraces(obj_trace_field,riff,"OTRC","OTR2");
		loadTraces(acc_trace_field,riff,"ATRC","ATR2");
		loadTraces(whl_trace_field,riff,"WTRC","WTR2");
	riff.DropBack();
}

template <class Def> double AnimatorA<Def>::executionTime()	const
{
	double len = 0;
			
	for (index_t i = 0; i < obj_trace_field.GetLength(); i++)
		for (index_t j = 0; j < obj_trace_field[i].step_field.GetLength(); j++)
			if (obj_trace_field[i].step_field[j].end > len)
				len = obj_trace_field[i].step_field[j].end;
	for (index_t i = 0; i < acc_trace_field.GetLength(); i++)
		for (index_t j = 0; j < acc_trace_field[i].step_field.GetLength(); j++)
			if (acc_trace_field[i].step_field[j].end > len)
				len = acc_trace_field[i].step_field[j].end;
	for (index_t i = 0; i < whl_trace_field.GetLength(); i++)
		for (index_t j = 0; j < whl_trace_field[i].step_field.GetLength(); j++)
			if (whl_trace_field[i].step_field[j].end > len)
				len = whl_trace_field[i].step_field[j].end;
	return len;
}



template <class Def> void AnimatorA<Def>::link(Geometry<Def>*domain)
{
	count_t invalid;
	
	invalid = 0;
	for (index_t i = 0; i < obj_trace_field.GetLength(); i++)
	{
		obj_trace_field[i].target = domain->lookupObject(obj_trace_field[i].tname);
		if (!obj_trace_field[i].target)
			invalid ++;
	}
	if (invalid)
	{
		Ctr::Array<TraceA<SubGeometryA<Def>,6>,Adopt>	new_field(obj_trace_field.GetLength()-invalid);
		index_t at = 0;
		for (index_t i = 0; i < obj_trace_field.GetLength(); i++)
		{
			if (obj_trace_field[i].target)
				new_field[at++] = obj_trace_field[i];
		}
		obj_trace_field.adoptData(new_field);
	}
	
	
	invalid = 0;
	for (index_t i = 0; i < acc_trace_field.GetLength(); i++)
	{
		acc_trace_field[i].target = domain->lookupAccelerator(acc_trace_field[i].tname);
		if (!acc_trace_field[i].target)
			invalid++;
	}
	if (invalid)
	{
		Ctr::Array<TraceA<TAccelerator<Def>,1>,Adopt>	new_field(acc_trace_field.GetLength()-invalid);
		index_t at = 0;
		for (index_t i = 0; i < acc_trace_field.GetLength(); i++)
		{
			if (acc_trace_field[i].target)
				new_field[at++] = acc_trace_field[i];
		}
		acc_trace_field.adoptData(new_field);
	}
	
	invalid = 0;
	for (index_t i = 0; i < whl_trace_field.GetLength(); i++)
	{
		whl_trace_field[i].target = domain->lookupWheel(whl_trace_field[i].tname);
		if (!whl_trace_field[i].target)
			invalid++;
	}
	if (invalid)
	{
		Ctr::Array<TraceA<TWheel<Def>,2>,Adopt>		new_field(whl_trace_field.GetLength()-invalid);
		index_t at = 0;
		for (index_t i = 0; i < whl_trace_field.GetLength(); i++)
		{
			if (whl_trace_field[i].target)
				new_field[at++] = whl_trace_field[i];
		}
		whl_trace_field.adoptData(new_field);
	}
}

template <class Def> void AnimatorA<Def>::saveToRiff(Riff::Chunk*riff)	const
{
	riff->AppendBlock("ANIM",name.c_str(),name.GetLength());
	Riff::Chunk*list = riff->AppendBlock("LIST");
		saveTraces(obj_trace_field,list,"OTR2");
		saveTraces(acc_trace_field,list,"ATR2");
		saveTraces(whl_trace_field,list,"WTR2");
}

template <class Def> bool VertexContainerA<Def>::equal;
template <class Def> bool MaterialData<Def>::equal;
template <class Def> bool IndexContainerA<Def>::equal;
template <class Def> bool RenderObjectA<Def>::equal;

template <class Def> String Geometry<Def>::error;
template <class Def> String SubGeometryA<Def>::error;

template <class Def> 
	count_t			Constructor<Def>::CountLODs()				const
	{
		count_t result = 0;
		foreach (objects,obj)
			result += obj->CountLODs();
		return result;
	}


template <class Def> 
	void			Constructor<Def>::Object::SetComputeNormalsBegin()
	{
		if (!(config.vertexFlags & HasNormalFlag))
			return;
		normalsFromQuad = currentLOD->quadIndices.Count();
		normalsFromTriangle = currentLOD->triangleIndices.Count();
		normalsFromVertex = currentLOD->vertexData.GetLength() / config.vsize;
	}

template <class Def> 
	void			Constructor<Def>::Object::ComputeNormals()
	{
		if (!(config.vertexFlags & HasNormalFlag))
			return;
		const count_t numVertices = currentLOD->vertexData.GetLength() / config.vsize;
		const count_t numQuadIndices = currentLOD->quadIndices.Count();
		const count_t numTriIndices = currentLOD->triangleIndices.Count();
		Float*vertexData = currentLOD->vertexData.pointer();
		for (index_t i = normalsFromVertex; i < numVertices; i++)
		{
			Float*nrm = vertexData + i * config.vsize + 3;
			M::Vec::clear(M::Vec::ref3(nrm));
		}
		const Index*indexData = currentLOD->triangleIndices.pointer();
		for (index_t i = normalsFromTriangle; i < numTriIndices; i+=3)
		{
			M::TVec3<Float>	normal;
			Float	*v0 = vertexData + indexData[i] * config.vsize,
					*v1 = vertexData + indexData[i+1] * config.vsize,
					*v2 = vertexData + indexData[i+2] * config.vsize;

			Obj::triangleNormal(M::Vec::ref3(v0),M::Vec::ref3(v1),M::Vec::ref3(v2),normal);
			M::Vec::add(M::Vec::ref3(v0+3),normal);
			M::Vec::add(M::Vec::ref3(v1+3),normal);
			M::Vec::add(M::Vec::ref3(v2+3),normal);
		}
		const Index*quadData = currentLOD->quadIndices.pointer();
		for (index_t i = normalsFromQuad; i < numQuadIndices; i+=4)
		{
			M::TVec3<Float>	normal,normal2;
			Float	*v0 = vertexData + quadData[i] * config.vsize,
					*v1 = vertexData + quadData[i+1] * config.vsize,
					*v2 = vertexData + quadData[i+2] * config.vsize,
					*v3 = vertexData + quadData[i+3] * config.vsize;

			Obj::triangleNormal(M::Vec::ref3(v0),M::Vec::ref3(v1),M::Vec::ref3(v2),normal);
			Obj::triangleNormal(M::Vec::ref3(v0),M::Vec::ref3(v2),M::Vec::ref3(v3),normal2);
			M::Vec::add(normal,normal2);
			M::Vec::add(M::Vec::ref3(v0+3),normal);
			M::Vec::add(M::Vec::ref3(v1+3),normal);
			M::Vec::add(M::Vec::ref3(v2+3),normal);
			M::Vec::add(M::Vec::ref3(v3+3),normal);
		}
		for (index_t i = normalsFromVertex; i < numVertices; i++)
		{
			Float*nrm = vertexData + i * config.vsize + 3;
			M::Vec::normalize0(M::Vec::ref3(nrm));
		}
		SetComputeNormalsBegin();
	}


template <class Def> 
	void			Constructor<Def>::Object::VerifyIntegrity(bool verifyAllVerticesAreUsed)	const
	{
		for (index_t l = 0; l < lods.Count(); l++)
		{
			const LOD&lod = lods[l];
			ASSERT__(!(lod.vertexData.GetLength()%config.vsize));
			const count_t numVertices = lod.vertexData.GetLength()/config.vsize;
			ASSERT__(!(lod.triangleIndices.Count()%3));
			ASSERT__(!(lod.quadIndices.Count()%4));
			for (index_t i = 0; i < lod.triangleIndices.Count(); i++)
			{
				ASSERT_LESS__(lod.triangleIndices[i],numVertices);
			}
			for (index_t i = 0; i < lod.quadIndices.Count(); i++)
			{
				ASSERT_LESS__(lod.quadIndices[i],numVertices);
			}
			if (verifyAllVerticesAreUsed)
			{
				Ctr::Array<bool>	used(numVertices);
				used.Fill(false);
				for (index_t i = 0; i < lod.triangleIndices.GetLength(); i++)
					used[lod.triangleIndices[i]] = true;
				for (index_t i = 0; i < lod.quadIndices.GetLength(); i++)
					used[lod.quadIndices[i]] = true;
				for (index_t i = 0; i < used.GetLength(); i++)
					ASSERT1__(used[i],i);
			}
		}
	}

template <class Def> 
	void			Constructor<Def>::VerifyIntegrity(bool verifyAllVerticesAreUsed)	const
	{
		for (index_t i = 0; i < objects.Count(); i++)
		{
			ASSERT_EQUAL__(config.vsize,objects[i].config.vsize);
			ASSERT_EQUAL__(config.numTextureLayers,objects[i].config.numTextureLayers);
			ASSERT_EQUAL__(config.vertexFlags,objects[i].config.vertexFlags);
			objects[i].VerifyIntegrity(verifyAllVerticesAreUsed);
		}
	}


template <class Def>
	M::Box<typename Constructor<Def>::Float>			Constructor<Def>::Object::GetBoundingBox(index_t lod /*=0*/)const
	{
		Float	min = std::numeric_limits<Float>::max(),	//no error
				max = std::numeric_limits<Float>::min();
		M::Box<Float>	result(min,min,min,max,max,max);
		const count_t numVertices = lods[lod].vertexData.GetLength()/config.vsize;
		const Float*vdata = lods[lod].vertexData.pointer();
		for (index_t i = 0; i < numVertices; i++)
		{
			const Float*vtx = vdata + i*config.vsize;
			result.Include(M::Vec::ref3(vtx));
		}
		return result;
	}





#endif
