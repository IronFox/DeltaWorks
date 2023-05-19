#include <global_root.h>
#include "interface.h"


#include <container/sorter.h>

#undef EXCEPTION
#define EXCEPTION(message)	FRAC_FATAL(message)

//#define DEBUG_POINT		cout << "\rf"<<__LINE__;
/*#define BEGIN
#define END
#define LBEGIN*/




static unsigned __log_depth = 0;
char*__space_field=NULL;

const char*spaces(unsigned length)
{
	if (!__space_field)
	{
		__space_field = new char[256];
	}
	memset(__space_field,' ',length);
	__space_field[length] = 0;
	return __space_field;
}

#undef LBEGIN
#undef BEGIN
#undef END
#ifndef BEGIN
	#define BEGIN	//cout<<spaces(__log_depth++)<<__func__<<"()"<<endl;
	//#define BEGIN	//logfile<<"entering: "+String(__func__)+"\n";
#endif
#ifndef END
	#define END		//cout<<spaces(--__log_depth)<<__func__<<"() done\n";
	//#define END		//logfile<<"exiting: "+String(__func__)+"\n";
#endif


namespace DeltaWorks::Fractal
{
	#include "kernel/config.h"
	
	
	

	

	SynchronizedLogFile								logfile(L"fractal.log",true);
	SurfaceSegment									*Body::production(NULL);
	bool											Body::can_update=true;
	unsigned										num_sectors=0,
													max_sectors=2000,
													build_operations=1;
	static const float								ttl_loss_per_level = 0.8f;
	Time::Delta										build_time = Time::Delta::FromSeconds( 1000000);
	
	bool											mipmapping = true,
													texture_compression = false,
													background_subdiv = true;

	//::TContext										default_context = {10,1,10000,32};
	Time::Stamp										Body::trace_time;		//!< Global time when tracing began
	Vector<SurfaceSegment*>							Body::need_children;

	static PointerSet								array_table,
													segment_table;
	
	
	
	bool											SurfaceSegment::allow_non_crucial_texture_update(false);
	
	
	

	namespace BackgroundSubDiv
	{
		volatile bool								active = false;
		bool										signaled = false;
											

		System::BlockingPipe								begin_tickets,
													end_tickets;


		class Process:public System::ThreadMainObject
		{
		public:
		virtual	void								ThreadMain();

		};

		Process										process;

		System::Thread								thread(process,true);

		void										begin();
		void										end();

		//void										signalEnd();

	};
	
	
	//ReferenceVector<TSurfaceSegment>					surface_list;
	
	
	void		registerSegment(TSurfaceSegment*segment)
	{
		return;	//disabled
		if (segment_table.IsSet(segment))
			FATAL__("Trying to reregister segment "+PointerToStr(segment));
		
		std::cout << "register "<<segment<<std::endl;
		segment_table.Set(segment);
	}
	
	void		unregisterSegment(TSurfaceSegment*segment)
	{
		return;	//disabled
		std::cout << "unregister "<<segment<<std::endl;
		segment_table.Unset(segment);
	}
	
	void		registerArray(TVertex*field)
	{
		return;	//disabled
		if (array_table.IsSet(field))
			FATAL__("Trying to reregister array "+PointerToStr(field));
		array_table.Set(field);
	}

	void		registerArray(Array<TVertex>&field)
	{
		return;	//disabled
		if (array_table.IsSet(field.pointer()))
			FATAL__("Trying to reregister array "+PointerToStr(field.pointer()));
		array_table.Set(field.pointer());
	}
	
	
	void		unregisterArray(TVertex*field)
	{
		return;	//disabled
		array_table.Unset(field);
	}
	void		unregisterArray(Array<TVertex>&field)
	{
		return;	//disabled
		array_table.Unset(field.pointer());
	}
	
	
	bool		isRegistered(TSurfaceSegment*segment)
	{
		return true;	//disabled
		return segment_table.IsSet(segment) && array_table.IsSet(segment->vertex_field.pointer());
	}
	
	
	
	
	#define INVOKING_DISCARD(ADDR)		//logfile << ">>Discard(0x"<<IntToHex((int)(ADDR),8)<<")"<<nl;
	#define INVOKED_DISCARD(ADDR)		//logfile << "<<Discard(0x"<<IntToHex((int)(ADDR),8)<<")"<<nl;
	#define INVOKING_VIRTUAL(ADDR,FUNC)	//logfile << ">>"<<#FUNC<<"(0x"<<IntToHex((int)(ADDR),8)<<")"<<nl;
	#define INVOKED_VIRTUAL(ADDR,FUNC)	//logfile << "<<"<<#FUNC<<"(0x"<<IntToHex((int)(ADDR),8)<<")"<<nl;
	
	
	void				reportFatalException(const TCodeLocation&location, const String&message)
	{
		Except::Fatal(location,message);
	}
			

	void	setSeed(Random&random, const Seed64&seed)
	{
		random.seed(seed.toInt());
	}
			
	void	setSeed(Random&random, const DynamicSeed&seed)
	{
		random.seed(seed.toInt());
	}
			
	TSurfaceSegmentLink link(SurfaceSegment*sector, BYTE orientation)
	{
		TSurfaceSegmentLink rs;
		rs.segment = sector;
		rs.orientation = orientation;
		return rs;
	}
	
	void	setRadialHeight(M::TVec3<> &v,double height, const SurfaceSegment*context)
	{
		/*if (!_zero(context->sector.v))
		{
			v[0] += (double)context->sector.x*(double)context->super->context->sector_size;
			v[1] += (double)context->sector.y*(double)context->super->context->sector_size;
			v[2] += (double)context->sector.z*(double)context->super->context->sector_size;
			return;
		}*/
		M::TVec3<double>	absolute_vector,delta_vector;
		double			len,delta;
		context->convertToAbsolute(v,absolute_vector);
		
		len = M::Vec::length(absolute_vector);
		delta = height-len;
		if (M::vabs(len) < M::GetError<double>())
			return;
		M::Vec::mult(absolute_vector,delta/len,delta_vector);
		M::Vec::add(v,delta_vector);
	}
	
	
	String	randomWord()
	{
		static const char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_./\\";
		Random random;
		BYTE len = random.Next(3,32);
		String result;
		result.Resize(len);
		for (BYTE k = 0; k < len; k++)
			result.Set(k,alpha[random.NextIndex(strlen(alpha))]);
		return result;
	}
	
	DynamicSeed::DynamicSeed()
	{
		randomize();
	}
	
	DynamicSeed::DynamicSeed(const String&name)
	{
		hash(name);
	}
		
	UINT32				DynamicSeed::toInt()	const
	{
		return CRC32::Sequence(value.pointer(),value.GetLength()).Finish();
	}
	
	String			DynamicSeed::toHex()	const
	{
		return binaryToHex(value.pointer(),value.GetContentSize());
	}
	
	DynamicSeed&			DynamicSeed::hash(const String&string)
	{
		value.SetSize(string.GetLength());
		value.copyFrom(string.c_str(),string.GetLength());
		as_int = toInt();
		return *this;
	}
	
	DynamicSeed&			DynamicSeed::operator=(const ValueType&array)
	{
		this->value = array;
		as_int = toInt();
		return *this;
	}
	
	void			DynamicSeed::implant(const DynamicSeed&seed, BYTE child_id)
	{
		value.SetSize(seed.value.GetLength()+1);
		value.copyFrom(seed.value.pointer(),seed.value.GetLength());
		value.Last() = child_id;
		as_int = toInt();
		//cout << seed.array.GetLength() <<" -> "<<array.GetLength()<<endl;
	}
	
	void			DynamicSeed::rotate(unsigned depth, BYTE orientation)
	{
		BYTE step = (BYTE)depth;
		for (unsigned i = 0;
				i < value.GetLength();
				i++)
		{
			value[i]+=step;
			step += value[i]>0
					?orientation%value[i]
					:orientation;
		}
		as_int = toInt();
	}
	
	void			DynamicSeed::randomize()
	{
		Random random;
		unsigned len = random.Next(3,32);
		value.SetSize(len);
		for (unsigned i = 0; i < len; i++)
			value[i] = random.Next(255);
		as_int = toInt();
	}


	
	
	Seed64::Seed64()
	{
		randomize();
	}
	
	Seed64::Seed64(const String&name)
	{
		hash(name);
	}
		
	UINT32				Seed64::toInt()	const
	{
		//algorithm by Thomas Wang, Jan 1997
		UINT64 temp = (~value) + (value << 18); // key = (key << 18) - key - 1;
		temp = temp ^ (temp >> 31);
		temp = temp * 21; // key = (key + (key << 2)) + (key << 4);
		temp = temp ^ (temp >> 11);
		temp = temp + (temp << 6);
		temp = temp ^ (temp >> 22);
		return (UINT32) temp;
	}
	
	String			Seed64::toHex()	const
	{
		return IntToHex((__int64)value,16);
	}
	
	Hash64				Seed64::hashFunction;
	
	Seed64&			Seed64::hash(const String&string)
	{
		value = hashFunction(string.c_str(),string.GetLength());
		as_int = toInt();
		return *this;
	}
	
	Seed64&			Seed64::operator=(const UINT64&seed)
	{
		this->value = seed;
		as_int = toInt();
		return *this;
	}
	
	void			Seed64::implant(const Seed64&seed, BYTE child_id)
	{
		this->value = (seed.value << 2) | child_id;
		as_int = toInt();
		//cout << seed.array.GetLength() <<" -> "<<array.GetLength()<<endl;
	}
	
	void			Seed64::rotate(unsigned depth, BYTE orientation)
	{
		BYTE step = (BYTE)depth;
		BYTE*bytes = (BYTE*)&value;
		for (unsigned i = 0; i < 8; i++)
		{
			bytes[i]+=step;
			step += bytes[i]>0 ? orientation%bytes[i] : orientation;
		}
		as_int = toInt();
	}
	
	void			Seed64::randomize()
	{
		Random random;
		BYTE*bytes = (BYTE*)&value;
		for (unsigned i = 0; i < 8; i++)
			bytes[i] = random.Next(255);
		as_int = toInt();
	}



	

	SurfaceSegment::SurfaceSegment(unsigned exponent_, unsigned vertex_count_, Body*super_, SurfaceSegment*parent_,BYTE child_index_,const Time::Stamp now):
						exponent(exponent_),vertex_range((1<<exponent_)+1),vertex_count(vertex_count_),super(super_),
						noisiness(0),
						c0(NULL),c1(NULL),c2(NULL),c3(NULL),priority(0), delay_counter(frame_delay),
						attachment(NULL)
	{
		//availability_signaled = false;
		parent = parent_;
		
		registerSegment(this);
		
	
		if (parent)
			depth = parent->depth-1;
		else
			depth = 0;
		child_index = child_index_;
		if (parent)
			sector = parent->sector;
		else
			M::Vec::clear(sector);
		//central_height = M::Composite::build(0,0);
		flags = RequiresUpdate;
		num_sectors++;
		//surface_list.append(this);
	}

	SurfaceSegment::SurfaceSegment(unsigned exponent_, unsigned vertex_count_):
						exponent(exponent_),vertex_range((1<<exponent_)+1),vertex_count(vertex_count_),super(NULL),
						noisiness(0),
						c0(NULL),c1(NULL),c2(NULL),c3(NULL),priority(0), delay_counter(frame_delay),
						attachment(NULL)
	{
		//availability_signaled = false;
		parent = NULL;

		registerSegment(this);
		
		depth = 0;
		child_index = 0;
		M::Vec::clear(sector);
		//central_height = M::Composite::build(0,0);
		flags = RequiresUpdate;
		num_sectors++;
		//surface_list.append(this);
	}

	SurfaceSegment*	SurfaceSegment::anyChild(unsigned depth)
	{
		if (depth > this->depth)
			return NULL;
		if (depth == this->depth)
			return this;
		if (!(flags & HasChildren))
			return NULL;
		SurfaceSegment*result;
		for (BYTE k = 0; k < 4; k++)
		{
			if (result = child[k]->anyChild(depth))
				return result;
		}
		return NULL;
	}

	
	void SurfaceSegment::finishBuild()
	{
		/*ASSERT__(vertex_field.GetLength() > 0);
		if (vertex_field.onDevice())
			ASSERT_NOT_NULL__(vertex_field.devicePointer())
		else
			ASSERT_NOT_NULL__(vertex_field.hostPointer());*/
			
		//ARRAY_DEBUG_POINT(f,super->full_map->border_index[0][0]);
		
		SurfaceSegment::corner[0] = vertex_field[super->full_map->border_index[0][0]].position;
		SurfaceSegment::corner[1] = vertex_field[super->full_map->border_index[1][0]].position;
		SurfaceSegment::corner[2] = vertex_field[super->full_map->border_index[2][0]].position;
		

		
		unsigned	middle_x = (super->full_map->vertex_range)/3,
					middle_y = (2*super->full_map->vertex_range)/3;
		TVertex middle = vertex_field[super->full_map->getIndex(middle_x,middle_y)];
		#if SSE_COORDINATES	
			SSE::exportVector3(middle.position,SurfaceSegment::water_center.v);
		#else
			SurfaceSegment::water_center = middle.position;
		#endif
		M::TVec3<>	dir;
		resolveDirection(middle.position, *this, *super->context, dir);
		
		M::Vec::mad(water_center,dir,-super->context->variance*middle.height);
		//Adaption::setHeight(SurfaceSegment::water_center.v,0, middle.height,*this,*super->context);		

		//DEBUG_POINT(f)
		updateSphere();

		//Resample variance
		{
			const VertexMap&map = *super->full_map;
			/*float	h0 = vertex_field[map.border_index[1][0]].height,
					h1 = vertex_field[map.border_index[2][0]].height,
					h2 = vertex_field[map.border_index[0][0]].height,
					hx = h1-h0,
					hy = h2-h0;*/

			/*float	min = vertex_field[0].height,
					max = min;*/
			static const unsigned step = map.vertex_range/8;
			noisiness = 0;
			count_t samples=0;
			for (unsigned y = step; y+step < map.vertex_range; y+=step)
				for (unsigned x = step; x+step <= y; x+=step)
				{
					float h =	(vertex_field[map.getIndex(x-step,y)].height
								+
								vertex_field[map.getIndex(x+step,y)].height
								+
								vertex_field[map.getIndex(x-step,y-step)].height
								+
								vertex_field[map.getIndex(x+step,y+step)].height
								+
								vertex_field[map.getIndex(x,y-step)].height
								+
								vertex_field[map.getIndex(x,y+step)].height)/6.0f;
					noisiness += M::Sqr(vertex_field[map.getIndex(x,y)].height-h);
					//noisiness = M::vmax(noisiness,M::Sqr(vertex_field[map.getIndex(x,y)].height-h));
					samples++;
				}
				noisiness = M::Sqr(noisiness /samples)*super->context->variance/local_sphere.radius*1e7f;
				//noisiness = M::Min(M::vpow(noisiness*super->context->variance/local_sphere.radius*1000.0f,2.0f),1.0f);
			/*

			for (unsigned i = resolution; i < vertex_field.GetLength(); i+=resolution)
			{
				float h = h0 + hx*float(map.vertex_descriptor[i].x)/float(map.vertex_maximum) + hy*(1.0f-float(map.vertex_descriptor[i].y)/float(map.vertex_maximum));
				variance = M::vmax(variance,vabs(h-vertex_field[i].height));
			}
			variance = M::vpow(variance*super->context->variance/local_sphere.radius,10.0f);*/
			//noisiness /= ;
		}

		
		for (BYTE k = 0; k < 3; k++)
		{
			if (neighbor_link[k].segment)
			{
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].segment = this;
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].orientation = k;
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].primary = !orderOfEdge(k);
				neighbor_link[k].primary = orderOfEdge(k);
			}
		}
		
		flags |= RequiresUpdate;
	}
	

	void SurfaceSegment::build()
	{
		ASSERT_NOT_NULL__(parent);
		ASSERT_NOT_NULL__(super);
		
		#ifdef FRACTAL_BUILD_STATISTIC
			Time::Stamp t;
		#endif
		
		vertex_field.SetSize(super->full_map->vertex_count);
		registerArray(vertex_field);
		Kernel::generateVertices(*this,*super->context,*super->full_map);
		Kernel::generateNormals(*this,*super->context,*super->full_map);
		
		flags |= HasData;
		finishBuild();
		
		#ifdef FRACTAL_BUILD_STATISTIC
			build_time += Time::Stamp()-t;
			build_operations++;
		#endif
	}
	
	void	SurfaceSegment::mergeEdges()
	{
		ASSERT1__(isRegistered(this),this);
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment)
				ASSERT2__(isRegistered(neighbor_link[k].segment),k,neighbor_link[k].segment);
	
		#ifdef FRACTAL_BUILD_STATISTIC
			Time::Stamp t;
		#endif
		if (Kernel::mergeEdges(*this,*super->context,*super->full_map))
		{
			#ifdef FRACTAL_BUILD_STATISTIC
				build_time += Time::Stamp()-t;
				build_operations++;
			#endif
		}
	}
	
	
	void	SurfaceSegment::update()	//is invoked once per each frame when the local surface segment is considered visible
	{
		
		if (flags & (RequiresUpdate | RequiresTextureUpdate))
		{
			if (!(flags & HasData))
				build();
			
			if (!(flags & RequiresUpdate))	//no complete rebuild necessary
			{
				if (!allow_non_crucial_texture_update || --delay_counter > 0)
					return;
			}
			delay_counter = frame_delay;
			allow_non_crucial_texture_update = false;
			
			#ifdef FRACTAL_BUILD_STATISTIC
				Time::Stamp t;
			#endif
			Kernel::updateTexture(*this,*super->context,*super->full_map,Fractal::mipmapping,Fractal::texture_compression);
			
			flags &= ~RequiresTextureUpdate;
			if (flags & RequiresUpdate)
			{
				Kernel::updateVBO(*this,*super->context,*super->full_map,*super->reduced_map,*super->vbo_vertex_indices);
				flags &= ~RequiresUpdate;

				#ifdef FRACTAL_BUILD_STATISTIC
					build_time += Time::Stamp()-t;
					build_operations++;
				#endif
				if (onUpdate)
					onUpdate(this);
			}
			#ifdef FRACTAL_BUILD_STATISTIC
			else
			{

				build_time += Time::Stamp() - t;
				build_operations++;
			}
			#endif

			
		}
	}
	
	void	SurfaceSegment::getCoverVertices(ArrayData<TCoverVertex>&cover_vertices)	const
	{
		Kernel::retrieveCover(*this,*super->context,*super->vbo_vertex_indices,cover_vertices);
	}

	void	SurfaceSegment::getFullPrimaryCoverage(ArrayData<TCoverVertex>&cover_vertices)	const
	{
		Kernel::retrieveFullPrimaryCover(*this,*super->context,*super->full_map,cover_vertices);
	}

	
	bool SurfaceSegment::orderOfEdge(BYTE edge)	const
	{
		const auto	&c0 = corner[(edge+1)%3],
					&c1 = corner[edge];
		if (c0.x > c1.x)
			return true;
		if (c0.x < c1.x)
			return false;
		if (c0.y > c1.y)
			return true;
		if (c0.y < c1.y)
			return false;
		if (c0.z > c1.z)
			return true;
		if (c0.z < c1.z)
			return false;
		FATAL__("Requesting order of collapsed edge (edge="+String(edge)+", c0="+c0.ToString()+", c1="+c1.ToString()+")");
		return false;
	}
		
	void SurfaceSegment::createChildren(const Time::Stamp&time, bool set_flag)
	{
		if (!(flags & HasData))
			reportFatalException(CLOCATION,"children requested but data not yet generated");
	
		if (flags&HasChildren)
			return;
			
		if (!( ((flags & Edge0Merged) || (flags & Edge0Open))
			&& ((flags & Edge1Merged) || (flags & Edge1Open))
			&& ((flags & Edge2Merged) || (flags & Edge2Open))))
			reportFatalException(CLOCATION,"edges not all merged but creating children (flags are "+IntToHex((int)flags,8)+")");
			
		
		//logfile << "(creating children)\n";
		
	
		//static Mutex   mutex;
		//mutex.lock();	//To ensure fixed child creation order (no children of neighbors created this very instance) - no longer necessary since children are generated sequentially
		ASSERT_IS_NULL__(c0);
			c0 = new SurfaceSegment(exponent, vertex_count, super,this,0,time);
				M::Vec::def3(c0->neighbor_link,resolveChild(neighbor_link[0],1),TSurfaceSegmentLink(),resolveChild(neighbor_link[2],0));
				c0->outer_seed[0].implant(outer_seed[0],orderOfEdge(0));
				c0->outer_seed[1].implant(inner_seed,0);
				c0->outer_seed[2].implant(outer_seed[2],!orderOfEdge(2));
				c0->inner_seed.implant(outer_seed[1],0);
				c0->reinit();
				c0->flags |= flags & (Edge0Open|Edge2Open);
			c1 = new SurfaceSegment(exponent, vertex_count, super,this,1,time);
				M::Vec::def3(c1->neighbor_link,resolveChild(neighbor_link[0],0),resolveChild(neighbor_link[1],1),TSurfaceSegmentLink());
				c1->outer_seed[0].implant(outer_seed[0],!orderOfEdge(0));
				c1->outer_seed[1].implant(outer_seed[1],orderOfEdge(1));
				c1->outer_seed[2].implant(inner_seed,1);
				c1->inner_seed.implant(outer_seed[2],1);
				c1->reinit();
				c1->flags |= flags & (Edge0Open|Edge1Open);
			c2 = new SurfaceSegment(exponent, vertex_count, super,this,2,time);
				M::Vec::def3(c2->neighbor_link,TSurfaceSegmentLink(),resolveChild(neighbor_link[1],0),resolveChild(neighbor_link[2],1));
				c2->outer_seed[0].implant(inner_seed,2);
				c2->outer_seed[1].implant(outer_seed[1],!orderOfEdge(1));
				c2->outer_seed[2].implant(outer_seed[2],orderOfEdge(2));
				c2->inner_seed.implant(outer_seed[0],2);
				c2->reinit();
				c2->flags |= flags & (Edge1Open|Edge2Open);
			c3 = new SurfaceSegment(exponent, vertex_count, super,this,3,time);
				M::Vec::def3(c3->neighbor_link,link(c2,0),link(c0,1),link(c1,2));
				c3->outer_seed[0] = c2->outer_seed[0];
				c3->outer_seed[1] = c0->outer_seed[1];
				c3->outer_seed[2] = c1->outer_seed[2];
				c3->inner_seed.implant(inner_seed,3);
				c3->reinit();
				//c3 cannot have open edges
		if (set_flag)
			flags |= HasChildren;
	}

	

	
	DataSurface::DataSurface(unsigned exponent_, unsigned vertex_count_):
						SurfaceSegment(exponent_,vertex_count_)
	{
		vertex_field.SetSize(vertex_count_);
		registerArray(vertex_field);
	
	}

	//Log	fractal_log("fractal.log",true);


	void			SurfaceSegment::copyVBOedge(BYTE edge, bool first_half)
	{
		//only copy geometrical edge. textural edge is irrelevant to us:
		const unsigned geometrical_exponent = super->reduced_map->exponent;
		//copy edge(s) from parent but only up to the geometrical exponent. we add one new edge at the bottom, directly 
		vbo_edge[edge].ResizePreserveContent(M::Min(parent->vbo_edge[edge].Count()+1,geometrical_exponent));


		static const unsigned	seam_texels = 1+TEXTURE_SEAM_EXTENSION;
		const unsigned texture_dimension = super->context->edge_length+1+TEXTURE_SEAM_EXTENSION;
		TVBOCompileParameter	parameter;
		parameter.context = super->context;
		parameter.coord_base = ((float)seam_texels)/(float)(texture_dimension);
		parameter.coord_stretch = (1.0f-parameter.coord_base)/(float)super->reduced_map->vertex_maximum;
		parameter.segment = parent;	//accessing parent vertices. hence the surrounding segment is not this but rather the parent
		parameter.map = super->full_map;

		M::TVec3<>		transition;
		M::Vec::sub(parent->sector,sector,transition);
		M::Vec::mult(transition,super->context->sector_size);

		ASSERT__(vbo_edge[edge].Count() > 0);
		{
			
			unsigned 	num_vertices = ((1<<(geometrical_exponent-1))+1),
						num_floats = num_vertices*floats_per_vbo_vertex;
			ASSERT__(num_floats>0);
			vbo_edge[edge].First().SetSize(num_floats);
			float*to = vbo_edge[edge].First().pointer();
			for (unsigned j = 0; j < num_vertices; j++)
			{
				ASSERT2__(j*2 < super->reduced_map->vertex_range,j*2,super->reduced_map->vertex_range);
				unsigned geometrical_index = super->reduced_map->border_index[edge][j*2];
				ASSERT2__(geometrical_index < super->reduced_map->vertex_count,geometrical_index,super->reduced_map->vertex_count);
				const TMapVertex&info = super->reduced_map->vertex_descriptor[geometrical_index];
				ASSERT__(info.grid_vertex);
				ASSERT__(child_index < 4);
				ASSERT2__(info.parent_space[child_index].match<super->vbo_vertex_indices->Count(),info.parent_space[child_index].match,super->vbo_vertex_indices->Count());
				UINT32 index = (*super->vbo_vertex_indices)[info.parent_space[child_index].match];
				ASSERT_NOT_NULL__(parent);
				ASSERT2__(index < parent->vertex_field.GetLength(),index,parent->vertex_field.GetLength());
				compileVBOVertex(parent->vertex_field[index],info,parameter,to);
				M::Vec::add(M::Vec::ref3(to),transition);
				to += floats_per_vbo_vertex;
			}
		}

		for (unsigned i = 1; i < vbo_edge[edge].Count(); i++)
		{
			unsigned 	num_inner_vertices = 1<<(geometrical_exponent-i-1),
						num_vertices = (num_inner_vertices+1),
						num_floats = num_vertices*floats_per_vbo_vertex,
						offset = num_inner_vertices*floats_per_vbo_vertex*!first_half;
			
			/*fractal_log<<"copying parent edge "<<(int)edge<<" at level "<<i<<endl;
			fractal_log<<" exponent="<<geometrical_exponent<<endl;
			fractal_log<<" child_index="<<child_index<<endl;
			fractal_log<<" num_floats="<<num_floats<<endl;
			fractal_log<<" num_inner_vertices="<<num_inner_vertices<<endl;
			fractal_log<<" num_vertices="<<num_vertices<<endl;
			fractal_log<<" offset="<<offset<<endl;
			fractal_log<<" transition="<<_toString(transition)<<endl;
			fractal_log<<" parent_edge_begin="<<((SurfaceSegment*)parent)->corner[edge].ToString()<<endl;
			fractal_log<<" parent_edge_end="<<((SurfaceSegment*)parent)->corner[(edge+1)%3].ToString()<<endl;*/
			vbo_edge[edge][i].SetSize(num_floats);

			ASSERT_EQUAL__(parent->vbo_edge[edge][i-1].GetLength(),2*num_floats-floats_per_vbo_vertex);
			const float	*from = parent->vbo_edge[edge][i-1].pointer()+offset;
			float*to = vbo_edge[edge][i].pointer();
			for (unsigned j = 0; j < num_vertices; j++)
			{
				//fractal_log<<"  translating raw vertex "<<j<<" at "<<_toString(from);
				M::Vec::add(M::Vec::ref3(from),transition,M::Vec::ref3(to)); from+=3; to+=3;	//translate coordinates
				//fractal_log<<" to "<<_toString(to-3)<<endl;
				Fractal::_c4(from,to); from+=4; to+=4;	//copy height and normal, now left to texture coordinates which must not be copied from parent
				Fractal::_c2(vbo_edge[edge][i-1]+j*2*floats_per_vbo_vertex+floats_per_vbo_vertex-2,to);	from+=2; to+=2;	//copy texture coordinates from next lower level
				//fractal_log<<" texcoords copied: "<<_toString(to-2,2)<<endl;




			}
		}


	}
	
	void			SurfaceSegment::reinit()
	{
		if (parent)
		{
			depth = parent->depth-1;
			
			sector = parent->sector;



			
			unsigned	i0 = super->full_map->border_index[0][0],
						i1 = super->full_map->border_index[1][0],
						i2 = super->full_map->border_index[2][0];
			unsigned	pi0 = super->full_map->vertex_descriptor[i0].parent_space[child_index].match,
						pi1 = super->full_map->vertex_descriptor[i1].parent_space[child_index].match,
						pi2 = super->full_map->vertex_descriptor[i2].parent_space[child_index].match;
			
			corner[0] = parent->vertex_field[pi0].position;
			corner[1] = parent->vertex_field[pi1].position;
			corner[2] = parent->vertex_field[pi2].position;

			M::TVec3<> center;
			M::Vec::center(corner[0],corner[1],corner[2],center);
			
				

			M::TVec3<sector_t> sdelta;
			
			sdelta.x = M::Round(center.x/super->context->sector_size);
			sdelta.y = M::Round(center.y/super->context->sector_size);
			sdelta.z = M::Round(center.z/super->context->sector_size);
			//M::Vec::div(center,super->context->sector_size,sdelta);
			M::Vec::add(sector,sdelta);
			
			if (parent->crater_field.GetLength())
			{
				M::TVec3<double>	absolute;
				M::Vec::copy(sdelta,absolute);
				M::Vec::mult(absolute,super->context->sector_size);
				float radius = M::Vec::distance(corner[0],center);
				
				for (unsigned j = 0; j < parent->crater_field.Count(); j++)
				{
					if (M::Vec::distance(parent->crater_field[j].base,center)-parent->crater_field[j].radius > radius*1.2)
						continue;
					
					TCrater&spawn = crater_field.Append();
					spawn = parent->crater_field[j];
					M::Vec::sub(spawn.base,absolute);
				}
				crater_field.compact();
			}

			for (BYTE k = 0; k < 3; k++)
			{
				if (neighbor_link[k].segment)
				{
					neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].segment = this;
					neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].orientation = k;
					neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].primary = !orderOfEdge(k);
				}
				neighbor_link[k].primary = orderOfEdge(k);
			}		
			

			if (child_index != 3)
			{
				copyVBOedge(child_index,true);
				copyVBOedge((child_index+2)%3,false);
				vbo_edge[(child_index+1)%3].free();
			}
			else
			{
				vbo_edge[0].free();
				vbo_edge[1].free();
				vbo_edge[2].free();
			}
			
			//lout << "sector central sector set to "<<_toString(sector)<<nl;
		}
		else
		{
			vbo_edge[0].free();
			vbo_edge[1].free();
			vbo_edge[2].free();

			M::Vec::clear(sector);
			if (super)
				depth = super->context->recursive_depth-1;
			else
				depth = 0;
		}
		
			
		
		
		M::TVec3<double> absolute;
		M::Vec::copy(sector,absolute);
		double sector_height = M::Vec::length(absolute);
		//central_height.offset = (int)sector_height;
		//central_height.remainder = (float)(sector_height-(double)central_height.offset)*super->context->sector_size;
		
		inner_seed.rotate(depth,child_index);
		
		
		TSurfaceSegment::inner_int_seed = inner_seed.as_int;
		TSurfaceSegment::outer_int_seed[0] = outer_seed[0].as_int;
		TSurfaceSegment::outer_int_seed[1] = outer_seed[1].as_int;
		TSurfaceSegment::outer_int_seed[2] = outer_seed[2].as_int;
		
		
		//lout << "central height is now <"<<central_height.offset<<", "<<central_height.remainder<<"> ("<<M::Composite::toDouble(central_height,super->context->sector_size)<<")"<<nl;
		

		/*outer_seed[0].implant(seed,0);
		outer_seed[1].implant(seed,1);
		outer_seed[2].implant(seed,3);
		inner_seed.implant(seed,4);*/
	}
	
	String			SurfaceSegment::identify()	const
	{
		const SurfaceSegment*segment = this;
		String rs;
		while (segment)
		{
			if (segment->parent)
				rs = String(segment->child_index)+rs;
			else
				rs = segment->super->name+'/'+String(segment->child_index)+'/'+rs;
			segment = (const SurfaceSegment*)segment->parent;
		}
		return rs;
	}
	
	void			SurfaceSegment::modifySphere(const M::AbstractSphere<float>&child_sphere)
	{
		global_sphere.Merge(child_sphere);
		if (parent)
			((SurfaceSegment*)parent)->modifySphere(global_sphere);
	}
	
	TSurfaceSegmentLink			SurfaceSegment::resolveChild(const TSurfaceSegmentLink&ref, BYTE index)
	{
		if (!ref.segment || !(ref.segment->flags&HasChildren))
			return TSurfaceSegmentLink();
		return link(((SurfaceSegment*)ref.segment)->child[(index+ref.orientation)%3],ref.orientation);
	}
	
	TSurfaceSegmentLink			SurfaceSegment::resolveChildRec(const TSurfaceSegmentLink&ref, BYTE index)
	{
		if (!ref.segment || /*!((SurfaceSegment*)ref.segment)->child[(index+ref.orientation)%3]*/!(ref.segment->flags&HasChildren) || ((SurfaceSegment*)ref.segment)->priority <= 0)
			return ref;
		return link(((SurfaceSegment*)ref.segment)->child[(index+ref.orientation)%3],ref.orientation);
	}
	
			
	SurfaceSegment::~SurfaceSegment()
	{
		BEGIN
		if (vertex_field.GetLength())
			unregisterArray(vertex_field);
		unregisterSegment(this);
		if (Body::production == this)
			Body::production = NULL;
		if (attachment)
			Discard(attachment);
		/*lout << "destructor invocation on "<<this<<". unhooking..."<<nl;
		flush(lout);*/
		//logfile << "unhooking\r\n";
		if (!application_shutting_down)
			unhook();	//cheap cheat, but whatever, if it works ;P
		//logfile << "unhooked\r\n";
		if (c0)
			delete c0;
		if (c1)
			delete c1;
		if (c2)
			delete c2;
		if (c3)
			delete c3;

		num_sectors--;
		/*lout << "destructor finished"<<nl;
		flush(lout);*/
		
		//surface_list.drop(this);

		END
	}
	
			
	void		SurfaceSegment::setNeighbors(const TSurfaceSegmentLink&link0, const TSurfaceSegmentLink&link1, const TSurfaceSegmentLink&link2)
	{
		BEGIN
		neighbor_link[0] = link0;
		neighbor_link[1] = link1;
		neighbor_link[2] = link2;
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment)
			{
				TSurfaceSegmentLink&n = neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation];
				n.orientation = k;
				n.segment = this;
				n.primary = !orderOfEdge(k);
				neighbor_link[k].primary = orderOfEdge(k);
			}
		END
	}
	/*
	void		DataSurface::copyEdge(BYTE k)
	{
		BEGIN
		TSurfaceSegmentLink&n = neighbor_link[k];
		if (!n.segment)
			return;
		unsigned*border0 = super->full_map->border_index[k],
				*border1 = super->full_map->border_index[n.orientation];
		for (unsigned i = 0; i < vertex_range; i++)
			vertex_field[border0[i]] = n.segment->vertex_field[border1[vertex_range-1-i]];
		END
	}
	*/
	

	
	
	
	void		SurfaceSegment::resetLinkage()
	{
		unhook();
		neighbor_link[0] = TSurfaceSegmentLink();
		neighbor_link[1] = TSurfaceSegmentLink();
		neighbor_link[2] = TSurfaceSegmentLink();
	}
	
	void		SurfaceSegment::unhook()
	{
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment)
			{
				//ASSERT__(surface_list(neighbor_link[k].segment)>0);
				/*lout << "attempting to unSet neighbor link (orientation "<<(int)neighbor_link[k].orientation<<") on neighbor "<<(int)k<<" ("<<neighbor_link[k].segment<<")"<<nl;
				flush(lout);*/
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation] = TSurfaceSegmentLink();
				neighbor_link[k] = TSurfaceSegmentLink();
			}
		/*lout << "unhooked"<<nl;
		flush(lout);*/
	}
	

	void	DataSurface::finishConstruction()
	{
		if (flags & RequiresUpdate)
		{
			Kernel::generateNormals(*this,*super->context,*super->full_map);
			flags |= HasData;
			//SurfaceSegment::update();
		}
		/*
		for (BYTE k = 0; k < 3; k++)
		{
			if (neighbor_link[k].segment)
			{
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].segment = this;
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].orientation = k;
				neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].primary = !orderOfEdge(k);
				neighbor_link[k].primary = orderOfEdge(k);
			}
		}*/
	}


	/*
		Old, complicated, deprecated
	*/
	/*
	void		SurfaceSegment::resolve(const TSurfaceSegmentLink&l0, const TSurfaceSegmentLink&l1, const TSurfaceSegmentLink&l2)
	{
	//    logfile << "visaddr("<<PointerToHex(this,8)<<"): "<<IntToHex((int)l0.link,8)<<" "<<IntToHex((int)l1.link,8)<<" "<<IntToHex((int)l2.link)<<nl;
		surface.delta0 = l0.segment?l0.segment->depth:this->depth;
		surface.delta1 = l1.segment?l1.segment->depth:this->depth;
		surface.delta2 = l2.segment?l2.segment->depth:this->depth;
		
	//    logfile << "vis: "<<vis0 << " "<<vis1 << " "<<vis2 << nl;
	
		if (priority > 0 && (flags&HasChildren))
		{
			c0->resolve(resolveChildRec(l0,1), link(c3,1),resolveChildRec(l2,0));
			c1->resolve(resolveChildRec(l0,0),resolveChildRec(l1,1),link(c3,2));
			c2->resolve(link(c3,0),resolveChildRec(l1,0),resolveChildRec(l2,1));
			c3->resolve();
		}
    }
	*/
	/*
		New, simpler, faster
	*/
	void		SurfaceSegment::resolve(unsigned d0, unsigned d1, unsigned d2)
	{
	//    logfile << "visaddr("<<PointerToHex(this,8)<<"): "<<IntToHex((int)l0.link,8)<<" "<<IntToHex((int)l1.link,8)<<" "<<IntToHex((int)l2.link)<<nl;

		surface.delta0 = d0;
		surface.delta1 = d1;
		surface.delta2 = d2;
		

			
	//    logfile << "vis: "<<vis0 << " "<<vis1 << " "<<vis2 << nl;
	
		if (priority > 0 && (flags&HasChildren))
		{
			if (d0 == this->depth && neighbor_link[0].segment && (neighbor_link[0].segment->flags&HasChildren) && (neighbor_link[0].segment->flags&SubdivLandscape))	//advance
				d0--;
			if (d1 == this->depth && neighbor_link[1].segment && (neighbor_link[1].segment->flags&HasChildren) && (neighbor_link[1].segment->flags&SubdivLandscape))	//advance
				d1--;
			if (d2 == this->depth && neighbor_link[2].segment && (neighbor_link[2].segment->flags&HasChildren) && (neighbor_link[2].segment->flags&SubdivLandscape))	//advance
				d2--;
			
			c0->resolve(d0, this->depth-1, d2);
			c1->resolve(d0, d1,	this->depth-1);
			c2->resolve(this->depth-1,	d1,	d2);
			c3->resolve();
		}

    }

	
	void		SurfaceSegment::resolve()
	{
		//resolve(neighbor_link[0],neighbor_link[1],neighbor_link[2]);
		resolve(this->depth,this->depth,this->depth);
	}
	

	/*
		Old, complicated, deprecated
	*/
	/*
	void		SurfaceSegment::resolveReflection(const TSurfaceSegmentLink&l0, const TSurfaceSegmentLink&l1, const TSurfaceSegmentLink&l2)
	{

	//    logfile << "visaddr("<<PointerToHex(this,8)<<"): "<<IntToHex((int)l0.link,8)<<" "<<IntToHex((int)l1.link,8)<<" "<<IntToHex((int)l2.link)<<nl;
		reflection.delta0 = l0.segment?l0.segment->depth:this->depth;
		reflection.delta1 = l1.segment?l1.segment->depth:this->depth;
		reflection.delta2 = l2.segment?l2.segment->depth:this->depth;
		
	//    logfile << "vis: "<<vis0 << " "<<vis1 << " "<<vis2 << nl;
	
		if ((flags & SubdivReflection) && (flags&HasChildren))
		{
			c0->resolveReflection(resolveChildRec(l0,1), link(c3,1),resolveChildRec(l2,0));
			c1->resolveReflection(resolveChildRec(l0,0),resolveChildRec(l1,1),link(c3,2));
			c2->resolveReflection(link(c3,0),resolveChildRec(l1,0),resolveChildRec(l2,1));
			c3->resolveReflection();
		}

    }
	*/
	/*
		New, simpler, faster
	*/
	void		SurfaceSegment::resolveReflection(unsigned d0, unsigned d1, unsigned d2)
	{

		reflection.delta0 = d0;
		reflection.delta1 = d1;
		reflection.delta2 = d2;
		if ((flags & SubdivReflection) && (flags&HasChildren))
		{
			if (d0 == this->depth && neighbor_link[0].segment &&  (neighbor_link[0].segment->flags&HasChildren) && (neighbor_link[0].segment->flags&SubdivReflection))	//advance
				d0--;
			if (d1 == this->depth && neighbor_link[1].segment &&  (neighbor_link[1].segment->flags&HasChildren) && (neighbor_link[1].segment->flags&SubdivReflection))	//advance
				d1--;
			if (d2 == this->depth && neighbor_link[2].segment &&  (neighbor_link[2].segment->flags&HasChildren) && (neighbor_link[2].segment->flags&SubdivReflection))	//advance
				d2--;
			
			c0->resolveReflection(d0, this->depth-1, d2);
			c1->resolveReflection(d0, d1,	this->depth-1);
			c2->resolveReflection(this->depth-1,	d1,	d2);
			c3->resolveReflection();
		}

    }	
	
	void		SurfaceSegment::resolveReflection()
	{
		//resolveReflection(neighbor_link[0],neighbor_link[1],neighbor_link[2]);
		resolveReflection(this->depth,this->depth,this->depth);
	}
	
	void		SurfaceSegment::resolveWater(unsigned d0, unsigned d1, unsigned d2)
	{

		water.delta0 = d0;
		water.delta1 = d1;
		water.delta2 = d2;
		if ((flags & SubdivWater) && (flags&HasChildren))
		{
			if (d0 == this->depth && neighbor_link[0].segment &&  (neighbor_link[0].segment->flags&HasChildren) && (neighbor_link[0].segment->flags&SubdivWater))	//advance
				d0--;
			if (d1 == this->depth && neighbor_link[1].segment &&  (neighbor_link[1].segment->flags&HasChildren) && (neighbor_link[1].segment->flags&SubdivWater))	//advance
				d1--;
			if (d2 == this->depth && neighbor_link[2].segment &&  (neighbor_link[2].segment->flags&HasChildren) && (neighbor_link[2].segment->flags&SubdivWater))	//advance
				d2--;
			
			unsigned dn = this->depth-1;
			c0->resolveWater(d0, dn, d2);
			c1->resolveWater(d0, d1, dn);
			c2->resolveWater(dn, d1, d2);
			c3->resolveWater(dn, dn, dn);
		}

    }	
	
	void		SurfaceSegment::resolveWater()
	{
		//resolveReflection(neighbor_link[0],neighbor_link[1],neighbor_link[2]);
		resolveWater(this->depth,this->depth,this->depth);
	}
	

	void		SurfaceSegment::dropChildren()
	{
		if (c0)
			delete c0;
		if (c1)
			delete c1;
		if (c2)
			delete c2;
		if (c3)
			delete c3;
		c0 = NULL;
		c1 = NULL;
		c2 = NULL;
		c3 = NULL;
		
		if (Body::production == this)	//can't continue production on this segment with the children gone
			Body::production = NULL;
		
		flags &= ~HasChildren;
		updateSphere();
	}
	
	int			SurfaceSegment::setPriority(const Aspect&view,bool override_only,const Time::Stamp&time)
	{
		ASSERT__(flags & HasData);
		
		bool has_override = !!(visibility & Visibility::Override);
		visibility = Visibility::Invisible;
		flags &= ~SubdivLandscape;
		
		if (!has_override)
		{
			priority = 0;
			
			if (override_only)
				return 0;
		}
		
		//const Aspect&view = *project_parameters.view;
		
		M::TVec3<> view_relative_center,axis,normal,sector_relative_view;
		
		//getTranslation(view.coordinates,super->center,this->sector,translation,super->context->sector_size);
		view.getTranslation(this->sector,translation,super->context->sector_size);
		
		
		#if 0
			_add(translation,global_sphere.center.vector,view_relative_center);
		
			_sub(view_relative_center,view.retraction_delta,axis);
			_oTriangleNormal(corner[0].v,corner[1].v,corner[2].v,normal);
			_normalize(normal);
		
			float	d = distance_square = _dot(axis);
		#else
			M::Vec::add(translation,global_sphere.center,view_relative_center);
			M::Vec::sub(view_relative_center,view.retraction_delta,axis);

			M::Vec::sub(view.retraction_delta,translation,sector_relative_view);
			float d = M::Obj::quadraticTriangleDistance(corner[0],corner[1],corner[2],sector_relative_view,normal);
			M::Vec::normalize0(normal);
		#endif
		float	r = M::Sqr(global_sphere.radius);
		float	fc = (float)(depth+1)/(float)super->context->recursive_depth,
				fc2 = M::vpow(2.0,-double(super->context->recursive_depth-depth));
		
		M::Vec::normalize(axis);
		float	effective_distance = d,
				focus_barrier = r*M::Sqr(2*(0.35+0.65*fc)*view.lod);//*(1.0f+noisiness);	//M::vmax(0.0f,-_dot(axis,normal))+
				//focus_barrier = r*variance*M::Sqr(2*view.lod*(fc+(1.0f-fc)*M::vmax(0.0f,-_dot(axis,normal))));
		/*if (effective_distance > 2*focus_barrier && depth < super->context->recursive_depth-1)
			return 0;*/
		//visibility = Visibility::Range;
		


		
		if (!override_only && view.frustum.IsVisible(view_relative_center,global_sphere.radius))
			visibility |= Visibility::Primary;
		if ((visibility & (Visibility::Primary)) || has_override)
		{
			if (effective_distance <= focus_barrier && depth > view.min_layer)
			{
				bool can_subdivide = ((flags & Edge0Merged) || (flags & Edge0Open))
					&& ((flags & Edge1Merged) || (flags & Edge1Open))
					&& ((flags & Edge2Merged) || (flags & Edge2Open));
				
				if (visibility & (Visibility::Primary))
				{
					priority = (int)((focus_barrier/effective_distance)*100)+1;
					if (can_subdivide)
						flags |= SubdivLandscape;
				}
					//else leave priority as it is
				if (!can_subdivide)
				{
					//want to subdivide but neighbors aren't available...
					
					if (!parent)	//this might happen if the base mesh wasn't closed
					{
						//flags |= Edge0Merged | Edge1Merged | Edge2Merged;
						FATAL__("Trying to determine priority of un-merged parent-less surface segment. Such a segment should not exist.");
					}
					else
						for (BYTE k = 0; k < 3; k++)
							if (!(flags & (Edge0Merged << k)) && !(flags & (Edge0Open << k)))
							{
								//edge k not merged. we need all
								//cout << "setPriority(...): invoking set flag method"<<endl;
								((SurfaceSegment*)parent)->triggerOverrideFlag(child_index,k,time,priority);	//pass on override priority if not of primary visibility. otherwise pass on local priority
								//cout << "setPriority(...): done"<<endl;
							}
					priority = 0;//set to 0 in case it's currently not zero because of override. segments that cannot subdivide cannot do even if triggered by override
				}
			}
			/*else
				priority = 0;	//ignore override requests for segments that are out of range? */
		}
		else
			priority = 0;	//this should be implied
		
		

		/*
		if (view.require_reflection)
		{
			double abs[3];
			_mad(global_sphere.center.vector,sector,super->context->sector_size,abs);
			double h = _length(abs)-super->context->base_heightf;
			if (h > 0 && h < super->context->variance/3)
			{
				float reflected_center[3];
				_c3(global_sphere.center.vector,reflected_center);
				
				Adaption::setHeight(reflected_center,-h,*this,*super->context);
				_add(reflected_center,translation);
				if (view.frustum.isVisible(reflected_center,global_sphere.radius))
				{
					visibility |= Visibility::Reflection;
				}
			}
		}
		*/
		
		//priority = 0;
	
		return priority;
	}
	
	bool			SurfaceSegment::triggerOverrideFlag(const Time::Stamp&now, unsigned priority_to_set)
	{
		//cout << "triggerOverrideFlag(): setting flag"<<endl;
		BEGIN
		if (!this || !(flags&HasData) || (parent && !(parent->flags&HasChildren)))
		{
			END
			return false;
		}
		visibility |= Visibility::Override;
		referenced = now;
		
		bool can_subdivide =
			(	((flags & Edge0Merged) || (flags & Edge0Open))
			&&	((flags & Edge1Merged) || (flags & Edge1Open))
			&&	((flags & Edge2Merged) || (flags & Edge2Open))
			);
		if (can_subdivide)
			priority += priority_to_set;
		else
		{
			//cout << "triggerOverrideFlag():  cannot subdivide"<<endl;
			for (BYTE k = 0; k < 3; k++)
				if (!(flags & (Edge0Merged << k)) && !(flags & (Edge0Open << k)))
				{
					//edge k not merged. we need all
					//cout << "triggerOverrideFlag():  invoking override flag set"<<endl;
					((SurfaceSegment*)parent)->triggerOverrideFlag(child_index,k,now, priority_to_set);
				}
		}
		//cout << "triggerOverrideFlag():  setting flag for parents"<<endl;
		SurfaceSegment*p = ((SurfaceSegment*)parent);
		while (p)
		{
			//cout << "triggerOverrideFlag():  setting flag for "<<p<<endl;
			p->visibility |= Visibility::Override;
			p->referenced = now;
			p->priority += priority_to_set;
			p = ((SurfaceSegment*)p->parent);
		}
		END
		return true;
	}
	
	void			SurfaceSegment::triggerOverrideFlag(BYTE child_index, BYTE neighbor_index,const Time::Stamp&now, unsigned priority_to_set)
	{
		BEGIN
		ASSERT__(!!(flags&HasChildren));	//how on earth could this thing fire...???
		
		ASSERT1__(child_index <= 3, child_index);
		//cout << "triggerOverrideFlag(...): invoked for "<<(int)child_index<<", "<<(int)neighbor_index<<endl;
		if (child_index == 3)
		{
			BYTE down = (neighbor_index+2)%3;
			//cout << "triggerOverrideFlag(...):  invoking nested for child "<<down<<endl;
			child[down]->triggerOverrideFlag(now,priority_to_set);
		}
		else
			if (((child_index+1)%3) == neighbor_index)
			{
				//cout << "triggerOverrideFlag(...):  invoking nested for central child"<<endl;
				c3->triggerOverrideFlag(now,priority_to_set);
			}
			else
				if (!((SurfaceSegment*)neighbor_link[neighbor_index].segment)->triggerOverrideFlag(now,priority_to_set))	//NULL-pointer sensitive
					if (parent)
					{
						//cout << "triggerOverrideFlag(...):  invoking recursive"<<endl;
						((SurfaceSegment*)parent)->triggerOverrideFlag(this->child_index,neighbor_index,now,priority_to_set);
					}
					else
						FATAL__("Parent expected");
		END
	}
	
	void			SurfaceSegment::setReflectionPriority(const Aspect&view)
	{
		ASSERT__(flags&HasData);
		
		flags &= ~SubdivReflection;
		
		
		//const Aspect&view = *project_parameters.view;
		
		M::TVec3<> center,axis,normal;
		view.getTranslation(this->sector,translation,super->context->sector_size);
		//getTranslation(view.coordinates,super->center,this->sector,translation,super->context->sector_size);
		M::Vec::add(translation,global_sphere.center,center);
		
		M::Vec::sub(center,view.retraction_delta,axis);
		_oTriangleNormal(corner[0],corner[1],corner[2],normal);
		M::Vec::normalize(normal);
		

		
		float	d = distance_square = M::Vec::dot(axis),
				r = M::Sqr(global_sphere.radius);
		float fc = (float)(depth+1)/(float)super->context->recursive_depth;
		
		M::Vec::normalize(axis);

				
		M::TVec3<double> abs;
		M::Vec::copy(sector,abs);
		M::Vec::mult(abs,super->context->sector_size);
		M::Vec::add(abs,global_sphere.center);
		double h = M::Vec::length(abs)-super->context->base_heightf;
		if (h+global_sphere.radius > 0 /*&& h-global_sphere.radius < super->context->variance/4*/)
		{
			float	effective_distance = d,
					focus_barrier = r*10.0f*64.0f/(1<<super->exponent) *M::Sqr((0.35+0.65*fc)*(view.lod*0.8))*(fc+(1.0f-fc)*M::vmax(0.0f,-M::Vec::dot(axis,normal)));
				
			M::TVec3<> reflected_center;
			M::Vec::copy(global_sphere.center,reflected_center);
			
			Height::apply(reflected_center,-h,*this,*super->context);
			M::Vec::add(reflected_center,translation);
			if (view.frustum.IsVisible(reflected_center,global_sphere.radius))
			{
				visibility |= Visibility::Reflection;
				if (effective_distance <= focus_barrier && depth > view.min_layer /*&& depth > super->context->recursive_depth/2*/)
					flags |= SubdivReflection;					
			}
		}
	}
	
	void			SurfaceSegment::setWaterPriority(const Aspect&view)
	{
		ASSERT__(flags&HasData);
		
		flags &= ~SubdivWater;

		if (depth+4 > super->context->recursive_depth)	//adjust to landscape visibility for the top three layers. might have to find another way of deciding here. maybe an absolute depth value?
		{
			if (visibility & Visibility::Primary)
				visibility |= Visibility::Water;
			if (flags & SubdivLandscape)
				flags |= SubdivWater;


			return;
		}




		
		
		//const Aspect&view = *project_parameters.view;
		
		M::TVec3<> center,axis,normal;
		
		M::TVec3<double> abs;
		M::Vec::copy(sector,abs);
		M::Vec::mult(abs,super->context->sector_size);
		M::Vec::add(abs,global_sphere.center);
		double h = M::Vec::length(abs);
		M::Vec::div(abs,h,normal);
		h-=super->context->base_heightf;
		
		
		
		//getTranslation(view.coordinates,super->center,this->sector,translation,super->context->sector_size);
		view.getTranslation(this->sector,translation,super->context->sector_size);
		
		M::Vec::add(translation,global_sphere.center,center);
		M::Vec::sub(center,view.retraction_delta,axis);
		
		

		
		float	d = distance_square = M::Vec::dot(axis),
				r = M::Sqr(global_sphere.radius);
		float fc = (float)(depth+1)/(float)super->context->recursive_depth;
		
		M::Vec::normalize(axis);

				

		if (h-global_sphere.radius < 0)
		{
			float	effective_distance = d,
					focus_barrier = r*10.0f*64.0f/(1<<super->exponent) *M::Sqr((0.35+0.65*fc)*(view.lod*0.9))*(fc+(1.0f-fc)*M::vmax(0.0f,-M::Vec::dot(axis,normal)));
		
			M::TVec3<> water_center;
			M::Vec::add(this->water_center,translation,water_center);

			if (view.frustum.IsVisible(water_center,global_sphere.radius))
			{
				visibility |= Visibility::Water;
				if (effective_distance <= focus_barrier && depth > view.min_layer && h+global_sphere.radius*0.5 > 0)
					flags |= SubdivWater;
			}
		}
	}
	
	void		SurfaceSegment::registerLeafSegments()
	{
		if (!(flags&HasChildren))
		{
			Body::need_children << this;
			return;
		}
		ASSERT__((flags&HasData));
		ASSERT_NOT_NULL__(c0);
		ASSERT_NOT_NULL__(c1);
		ASSERT_NOT_NULL__(c2);
		ASSERT_NOT_NULL__(c3);
		c0->registerLeafSegments();
		c1->registerLeafSegments();
		c2->registerLeafSegments();
		c3->registerLeafSegments();
	}


	void		SurfaceSegment::project(const Aspect&view, float ttl, const Time::Stamp&time, bool override_only)
	{
		//LBEGIN
		ASSERT__(!BackgroundSubDiv::active);
		ASSERT__(flags&HasData);
		if (!depth)
		{
			priority = 0;
			flags &= ~SubdivLandscape;
		}
		

		
		if ((visibility&Visibility::Primary) && !(flags&AvailableEventTriggered))
		{
			if (onSegmentAvailable)
				onSegmentAvailable(this);
			flags |= AvailableEventTriggered;
		}
		if (!override_only && (visibility&Visibility::Primary) && attachment)
			super->attachment_buffer << this;
			
			
		if (priority > 0)	//subdiv by visibility or override
		{
			if (!(flags&HasChildren))
			{
				Body::need_children << this;
				//lout << "inserting"<<nl;
			}
			else
				//if ((c0->flags&HasData) && (c1->flags&HasData) && (c2->flags&HasData) && (c3->flags&HasData))
				if (c0->flags&c1->flags&c2->flags&c3->flags&HasData)	//strange but should work
				{
					//lout << "focusing on children on level "<<depth<<nl;
					bool override_only_ = override_only || /*!(visibility&Visibility::Primary) || */!(flags&SubdivLandscape);
					/*if (!override_only_)
						flags |= SubdivLandscape;*/
					c0->setPriority(view,override_only_,time);
					c1->setPriority(view,override_only_,time);
					c2->setPriority(view,override_only_,time);
					c3->setPriority(view,override_only_,time);
					
					c0->project(view,ttl*ttl_loss_per_level,time,override_only_);
					c1->project(view,ttl*ttl_loss_per_level,time,override_only_);
					c2->project(view,ttl*ttl_loss_per_level,time,override_only_);
					c3->project(view,ttl*ttl_loss_per_level,time,override_only_);
						

					referenced = time;
					
					//END
					if (!override_only && !(flags&SubdivLandscape) && (visibility&Visibility::Primary))
					{
						update();
						super->face_buffer << this;
						if (depth < super->least_traced_level)
							super->least_traced_level = depth;
					}
					return;
				}
				//else
					//lout << "cannot go on. not all children have data"<<nl;
		}
			
		if (visibility&Visibility::Primary)
		{
			update();
			super->face_buffer << this;
			if (depth < super->least_traced_level)
				super->least_traced_level = depth;
		}
		if (visibility != Visibility::Invisible)
			referenced = time;
		if (depth)
			checkTimeouts(time,ttl);

	
		//END
    }
	
	void		SurfaceSegment::projectReflection(const Aspect&view)
	{
		//LBEGIN

		if ((flags&SubdivReflection) && (!depth/* || !priority*/))
		{
			//lout<< "depth is 0. setting priority 0"<<nl;
			flags &= ~SubdivReflection;
		}
		
		if (flags&SubdivReflection)
		{
			if (!(flags&HasChildren))
			{
				//don't request water surfaces
				flags &= ~SubdivReflection;
			}
			else
				if ((c0->flags&HasData) && (c1->flags&HasData) && (c2->flags&HasData) && (c3->flags&HasData))
				{
					//lout << "focusing on children on level "<<depth<<nl;
				
					c0->setReflectionPriority(view);
					c1->setReflectionPriority(view);
					c2->setReflectionPriority(view);
					c3->setReflectionPriority(view);
					
					c0->projectReflection(view);
					c1->projectReflection(view);
					c2->projectReflection(view);
					c3->projectReflection(view);

					//referenced = time;
					//END
					return;
				}
				//else
					//lout << "cannot go on. not all children have data"<<nl;
		}
			
		if (visibility&Visibility::Reflection)
		{
			update();
			super->reflection_buffer << this;
		}
		/*if (visibility != Visibility::Invisible)
			referenced = time;*/
	
		//END
    }
	
	void		SurfaceSegment::projectWater(const Aspect&view)
	{
		//LBEGIN

		if ((flags&SubdivWater) && (!depth || !(flags&SubdivLandscape)))
		{
			//lout<< "depth is 0. setting priority 0"<<nl;
			flags &= ~SubdivWater;
		}
		
		if (flags&SubdivWater)
		{
			if (!(flags&HasChildren))
			{
				//don't request water surfaces
				flags &= ~SubdivWater;
			}
			else
				if ((c0->flags&HasData) && (c1->flags&HasData) && (c2->flags&HasData) && (c3->flags&HasData))
				{
					//lout << "focusing on children on level "<<depth<<nl;
				
					c0->setWaterPriority(view);
					c1->setWaterPriority(view);
					c2->setWaterPriority(view);
					c3->setWaterPriority(view);
					
					c0->projectWater(view);
					c1->projectWater(view);
					c2->projectWater(view);
					c3->projectWater(view);

					//referenced = time;
					//END
					return;
				}
				//else
					//lout << "cannot go on. not all children have data"<<nl;
		}
			
		if (visibility&Visibility::Water)
		{
			update();
			super->water_buffer << this;
		}
		/*if (visibility != Visibility::Invisible)
			referenced = time;*/
	
		//END
    }
	
	void		SurfaceSegment::checkTimeouts(const Time::Stamp&time, float ttl)
	{
		float delta = (time-referenced).ToSeconds();
		if ((delta > ttl || delta < 0) && !(visibility & Visibility::Override))
		{
			dropChildren();
		}
		else
		{
			if ((flags&HasChildren))
			{
				c0->checkTimeouts(time,ttl*ttl_loss_per_level);
				c1->checkTimeouts(time,ttl*ttl_loss_per_level);
				c2->checkTimeouts(time,ttl*ttl_loss_per_level);
				c3->checkTimeouts(time,ttl*ttl_loss_per_level);
			}
		}
	}
	
	void		SurfaceSegment::include(SurfaceSegment*child)
	{
		M::TVec3<> delta,center;
		super->getTranslation(child->sector,this->sector,delta);
		M::Vec::sub(child->global_sphere.center,delta,center);
		global_sphere.Include(center,child->global_sphere.radius);
	}
	
	void		SurfaceSegment::defaultDefineSphere(M::AbstractSphere<float>&sphere)
	{
		M::Vec::center(corner[0],corner[1],corner[2],sphere.center);
		sphere.radius = sqrt(M::vmax(M::vmax(M::Vec::quadraticDistance(corner[0],sphere.center),M::Vec::quadraticDistance(corner[1],sphere.center)),M::Vec::quadraticDistance(corner[2],sphere.center)));
	}
	
	bool		SurfaceSegment::rayCast(const M::Composite::Coordinates&b_, const M::TVec3<>&d, TRayIntersection&intersection)
	{
		intersection.isSet = false;
		if (!(flags&HasData))
			return false;
		M::TVec3<> b;
		M::Vec::sub(b_.sector,sector,b);
		M::Vec::mult(b,super->context->sector_size);
		M::Vec::add(b,b_.remainder);
		//lout << "inner base ("<<_toString(b_.sector)<<"relative to "<<_toString(sector)<<") is "<<_toString(b)<<nl;
		if (!M::Obj::detectOpticalSphereIntersection(global_sphere.center, global_sphere.radius, b, d))
			return false;
		
		if ((flags&HasChildren))
		{
			bool changed = false;
			TRayIntersection	my_intersection;
			if (c0->rayCast(b_,d,my_intersection) && (!intersection.isSet || my_intersection.fc[2] < intersection.fc[2]))
			{
				intersection = my_intersection;
				changed = true;
			}
			if (c1->rayCast(b_,d,my_intersection) && (!intersection.isSet || my_intersection.fc[2] < intersection.fc[2]))
			{
				intersection = my_intersection;
				changed = true;
			}
			if (c2->rayCast(b_,d,my_intersection) && (!intersection.isSet || my_intersection.fc[2] < intersection.fc[2]))
			{
				intersection = my_intersection;
				changed = true;
			}
			if (c3->rayCast(b_,d,my_intersection) && (!intersection.isSet || my_intersection.fc[2] < intersection.fc[2]))
			{
				intersection = my_intersection;
				changed = true;
			}
			//if (changed)
				//lout << "returning true"<<nl;
			return changed;
		}
		if (Kernel::rayCast(*this,b,d,*super->triangle_field,intersection))
		{
			intersection.segment = this;
			return true;
		}
		return false;
	}

	bool		SurfaceSegment::groundQuery(const M::Composite::Coordinates&b_, const M::TVec3<>&d, unsigned min_layer, TGroundInfo&ground)
	{
		if (!(flags&HasData))
			return false;
		M::TVec3<> b;
		M::Vec::sub(b_.sector,sector,b);
		M::Vec::mult(b,super->context->sector_size);
		M::Vec::add(b,b_.remainder);
		float b2 = M::Vec::dot(b);
		if (b2 > M::Sqr(super->context->base_heightf)*1.5)
		//if (b2 > M::Sqr(global_sphere.radius)*1.5)
		{
			return false;
		}
		

		
		//lout << "inner base ("<<_toString(b_.sector)<<"relative to "<<_toString(sector)<<") is "<<_toString(b)<<nl;
		
		M::TVec3<>	d1,r;
		M::Vec::subtract(b,global_sphere.center,d1);
		if (M::Vec::dot(d1) > M::Sqr(super->context->base_heightf)*1.5)
		//if (b2 > M::Sqr(global_sphere.radius)*1.5)
		{
			return false;
		}
		
		M::Vec::cross(d,d1,r);
		float dist2 = M::Vec::dot(r);
		if (dist2 > M::Sqr(global_sphere.radius))
			return false;
			
		M::TVec3<> fc;
		if (!M::Obj::detTriangleRayIntersection(corner[0], corner[1], corner[2], b, d, fc))
			return false;
		if (fc.x < -0.01 || fc.y < -0.01 || fc.x+fc.y>1.01)
			return false;

		
					
		
		if ((flags&HasChildren) && depth > min_layer)
		{
			if (c0->groundQuery(b_,d,min_layer,ground))
				return true;
			if (c1->groundQuery(b_,d,min_layer,ground))
				return true;
			if (c2->groundQuery(b_,d,min_layer,ground))
				return true;
			if (c3->groundQuery(b_,d,min_layer,ground))
				return true;
			return false;
		}

		if (/*depth > 2 ||*/ M::Vec::quadraticDistance(global_sphere.center,b) > M::Sqr(global_sphere.radius*2.5))
		{
			ground.segment = this;
			ground.isSet = true;
			ground.height_over_ground = fc.z;
			ground.ground_height = 0; //erm...
			M::Vec::mad(b,d,fc.z,ground.position);

			_oTriangleNormal(corner[0],corner[1],corner[2],ground.normal);
			return true;
		}
		else

			if (Kernel::groundQuery(*this,b,d,*super->triangle_field,ground))
			{
				ground.segment = this;
				return true;
			}
		return false;
	}

	
	void		SurfaceSegment::updateSphere()
	{
		BEGIN
		defaultDefineSphere(local_sphere);
		
		

		global_sphere = local_sphere;
	
		if ((flags&HasChildren))
		{
			include(c0);
			include(c1);
			include(c2);
			include(c3);
		}
	
	
		if (parent)
			((SurfaceSegment*)parent)->modifySphere(global_sphere);
		END
	}
	
	void		SurfaceSegment::checkLinkage()
	{
		BEGIN
		ASSERT1__(isRegistered(this),this);
		if (parent && ((SurfaceSegment*)parent)->child[child_index] != this)
			EXCEPTION("parent-orientation broken");
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment)
			{
				ASSERT1__(isRegistered(neighbor_link[k].segment),this);
				if (neighbor_link[k].segment == this)
					EXCEPTION("self referential error");
				if (neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].segment != this || neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].orientation != k)
					EXCEPTION("neighbor-link "+IntToStr(k)+" broken");
				if (neighbor_link[k].primary == neighbor_link[k].segment->neighbor_link[neighbor_link[k].orientation].primary)
					EXCEPTION("primary flag identical");
					
				if (parent && ((SurfaceSegment*)neighbor_link[k].segment)->parent != parent)
				{
					bool found = false;
					for (BYTE i = 0; i < 3; i++)
						if (parent->neighbor_link[i].segment == ((SurfaceSegment*)neighbor_link[k].segment)->parent)
							found = true;
					if (!found)
						EXCEPTION("parents don't know each other");
				}
				
				
				/*if (!_similar(neighbor_link[k].segment->corner[neighbor_link[k].orientation].position,corner[(k+1)%3].position)
					||
					!_similar(neighbor_link[k].segment->corner[(neighbor_link[k].orientation+1)%3].position,corner[k].position))
					EXCEPTION("corners differ");*/
			}
		if ((flags&HasChildren))
			for (BYTE k = 0; k < 4; k++)
				if (child[k])
					child[k]->checkLinkage();
	
		END
	}


	void		DataSurface::syncGeneratedBorders(unsigned level)
	{
		const VertexMap*root_map = this->super->full_map;
		unsigned	offset = 1<<level,
					step = offset << 1;
		
		unsigned*rmap[3];
		for (BYTE k = 0; k < 3; k++)
		{
			TSurfaceSegmentLink&n = neighbor_link[k];
			rmap[k] = root_map->border_index[n.orientation];
		}
		
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment && neighbor_link[k].primary)
			{
				for (unsigned i = offset; i < vertex_range; i+=step)
					neighbor_link[k].segment->vertex_field[rmap[k][vertex_range-1-i]] = vertex_field[root_map->border_index[k][i]];
			}
				
		#if 0
		//logfile << __LINE__<<nl;
		
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment && neighbor_link[k].primary)
			{
				/*neighbor[k].link->primary[neighbor[k].orientation] = false;
				neighbor[k].link->neighbor[neighbor[k].orientation] = link(this,k);*/
				
				for (unsigned i = step; i < map->vertex_range; i+=2*step)
				{
					::TChannelConfig	&vthis = vertex_field[map->border_index[k][i]].channel,
										&vthat = ((DataSurface*)neighbor_link[k].segment)->vertex_field[rmap[k][vertex_range-1-i]].channel;
					vthis.c0 = (vthis.c0+vthat.c0)/2;
					vthis.c1 = (vthis.c1+vthat.c1)/2;
					vthis.c2 = (vthis.c2+vthat.c2)/2;
					#if FRACTAL_KERNEL!=MINIMAL_KERNEL
						vthis.c3 = (vthis.c3+vthat.c3)/2;
					#endif
					#if FRACTAL_KERNEL==FULL_KERNEL
						vthis.c4 = (vthis.c4+vthat.c4)/2;
					#endif
					vthis.oceanic = (vthis.oceanic+vthat.oceanic)/2;
					vthis.age = (vthis.age+vthat.age)/2;
					vthis.water = (vthis.water+vthat.water)/2;
					vthat = vthis;
				}
			}
			
		#endif
		//logfile << __LINE__<<nl;
	}
	
	void		DataSurface::generateLayer(unsigned level)
	{
		BEGIN
		unsigned step = 1<<level;
		
		const VertexMap*root_map = super->full_map;
		
		Random	random(inner_seed.toInt());

		unsigned linear;
		int seed;
		for (unsigned y = step; y < vertex_range; y+=2*step)
		{
			{
				linear = root_map->getIndex(0,y);
				TVertex&v = vertex_field[linear];
				seed = outer_seed[0].as_int+(((linear*15+3)*(linear*11+2)));				
				generate3center(vertex_field[root_map->getIndex(0,y-step)],vertex_field[root_map->getIndex(0,y+step)],
								vertex_field[root_map->getIndex(2*step,y+step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				
				
				Height::apply(		v.position,v.height*super->context->variance,*this,*super->context);
				
				
				
			}
			for (unsigned x = 2*step; x <= y; x+=2*step)
			{
				linear = root_map->getIndex(x,y);
				TVertex&v = vertex_field[linear];
				seed = inner_seed.as_int+(((linear*15+3)*(linear*11+2)));				
				generate4center(vertex_field[root_map->getIndex(x,y-step)],vertex_field[root_map->getIndex(x,y+step)],
								vertex_field[root_map->getIndex(x-2*step,y-step)],vertex_field[root_map->getIndex(x+2*step,y+step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				Height::apply(	v.position,v.height*super->context->variance,*this,*super->context);
			}
			
			{
				linear = root_map->getIndex(y,y);
				TVertex&v = vertex_field[linear];
				
				seed = outer_seed[2].as_int+(((linear*15+3)*(linear*11+2)));				
				generate3center(vertex_field[root_map->getIndex(y-step,y-step)],vertex_field[root_map->getIndex(y+step,y+step)],
								vertex_field[root_map->getIndex(y-step,y+step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				
				Height::apply(	v.position,v.height*super->context->variance,*this,*super->context);
			}
			for (unsigned x = step; x < y; x+=2*step)
			{
				linear = root_map->getIndex(x,y);
				TVertex&v = vertex_field[linear];
				seed = inner_seed.as_int+(((linear*15+3)*(linear*11+2)));				
				generate4center(vertex_field[root_map->getIndex(x-step,y-step)],vertex_field[root_map->getIndex(x+step,y+step)],
								vertex_field[root_map->getIndex(x-step,y+step)],vertex_field[root_map->getIndex(x+step,y-step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				Height::apply(	v.position,v.height*super->context->variance,*this,*super->context);
			}
		}
		for (unsigned y = 2*step; y < vertex_range-1; y+=2*step)
			for (unsigned x = step; x <=y; x+=2*step)
			{
				linear = root_map->getIndex(x,y);
				TVertex&v = vertex_field[linear];
				seed = inner_seed.as_int+(((linear*15+3)*(linear*11+2)));				
				generate4center(vertex_field[root_map->getIndex(x-step,y)],vertex_field[root_map->getIndex(x+step,y)],
								vertex_field[root_map->getIndex(x-step,y-2*step)],vertex_field[root_map->getIndex(x+step,y+2*step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				Height::apply(	v.position,v.height*super->context->variance,*this,*super->context);
			}
									
		for (unsigned x = step; x <=vertex_range-1; x+=2*step)
		{
			linear = root_map->getIndex(x,vertex_range-1);
			TVertex&v = vertex_field[linear];
			seed = outer_seed[1].as_int+(((linear*15+3)*(linear*11+2)));				
			generate3center(vertex_field[root_map->getIndex(x-step,vertex_range-1)],vertex_field[root_map->getIndex(x+step,vertex_range-1)],
							vertex_field[root_map->getIndex(x-step,vertex_range-1-2*step)],
							v,
							seed,*this,*super->context,
							crater_field.pointer(),unsigned(crater_field.GetLength())
							);
			Height::apply(	v.position,v.height*super->context->variance,*this,*super->context);
		}

		flags |= HasData;
		END
    }
		
	void		DataSurface::generateLayer(unsigned level,void(*updateHeight)(TVertex&vertex))
	{
		BEGIN
		unsigned step = 1<<level;
		
		const VertexMap*root_map = super->full_map;
		
		Random random(inner_seed.toInt());

		unsigned linear;
		int seed;
		for (unsigned y = step; y < vertex_range; y+=2*step)
		{
			{
				linear = root_map->getIndex(0,y);
				TVertex&v = vertex_field[linear];
				seed = outer_seed[0].as_int+(((linear*15+3)*(linear*11+2)));				
				generate3center(vertex_field[root_map->getIndex(0,y-step)],vertex_field[root_map->getIndex(0,y+step)],
								vertex_field[root_map->getIndex(2*step,y+step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				updateHeight(v);
			}
			for (unsigned x = 2*step; x <= y; x+=2*step)
			{
				linear = root_map->getIndex(x,y);
				TVertex&v = vertex_field[linear];
				seed = inner_seed.as_int+(((linear*15+3)*(linear*11+2)));				
				generate4center(vertex_field[root_map->getIndex(x,y-step)],vertex_field[root_map->getIndex(x,y+step)],
								vertex_field[root_map->getIndex(x-2*step,y-step)],vertex_field[root_map->getIndex(x+2*step,y+step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				updateHeight(v);
			}
			
			{
				linear = root_map->getIndex(y,y);
				TVertex&v = vertex_field[linear];
				
				seed = outer_seed[2].as_int+(((linear*15+3)*(linear*11+2)));				
				generate3center(vertex_field[root_map->getIndex(y-step,y-step)],vertex_field[root_map->getIndex(y+step,y+step)],
								vertex_field[root_map->getIndex(y-step,y+step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				
				updateHeight(v);
			}
			for (unsigned x = step; x < y; x+=2*step)
			{
				linear = root_map->getIndex(x,y);
				TVertex&v = vertex_field[linear];
				seed = inner_seed.as_int+(((linear*15+3)*(linear*11+2)));				
				generate4center(vertex_field[root_map->getIndex(x-step,y-step)],vertex_field[root_map->getIndex(x+step,y+step)],
								vertex_field[root_map->getIndex(x-step,y+step)],vertex_field[root_map->getIndex(x+step,y-step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				updateHeight(v);
			}
		}
		for (unsigned y = 2*step; y < vertex_range-1; y+=2*step)
			for (unsigned x = step; x <=y; x+=2*step)
			{
				linear = root_map->getIndex(x,y);
				TVertex&v = vertex_field[linear];
				seed = inner_seed.as_int+(((linear*15+3)*(linear*11+2)));				
				generate4center(vertex_field[root_map->getIndex(x-step,y)],vertex_field[root_map->getIndex(x+step,y)],
								vertex_field[root_map->getIndex(x-step,y-2*step)],vertex_field[root_map->getIndex(x+step,y+2*step)],
								v,
								seed,*this,*super->context,
								crater_field.pointer(),unsigned(crater_field.GetLength())
								);
				updateHeight(v);
			}
									
		for (unsigned x = step; x <=vertex_range-1; x+=2*step)
		{
			linear = root_map->getIndex(x,vertex_range-1);
			TVertex&v = vertex_field[linear];
			seed = outer_seed[1].as_int+(((linear*15+3)*(linear*11+2)));				
			generate3center(vertex_field[root_map->getIndex(x-step,vertex_range-1)],vertex_field[root_map->getIndex(x+step,vertex_range-1)],
							vertex_field[root_map->getIndex(x-step,vertex_range-1-2*step)],
							v,
							seed,*this,*super->context,
							crater_field.pointer(),unsigned(crater_field.GetLength())
							);
			updateHeight(v);
		}

		flags |= HasData;
		END
    }
	
	

	/*
	void		DataSurface::mergeBorderNormals(bool update)
	{
		BEGIN
		const Map*root_map = super->full_map;
		for (BYTE k = 0; k < 3; k++)
			if (neighbor_link[k].segment)
			{
				for (unsigned i = 1; i < vertex_range-1; i++)
				{
					TVertex	&local = vertex_field[root_map->border_index[k][i]],
							&remote = ((DataSurface*)neighbor_link[k].segment)->vertex_field[root_map->border_index[neighbor_link[k].orientation][vertex_range-i-1]];
					_add(local.normal,remote.normal);
					_normalize(local.normal);
					_c3(local.normal,remote.normal);
				}
				if (update)
					((DataSurface*)neighbor_link[k].segment)->flags |= RequiresUpdate;
			}
		END
	}
	*/



	Body::Body(const String&name_, float sector_size_, unsigned exponent_, unsigned recursion_depth,const ArrayData<unsigned>*vbo_vertex_indices_, const ArrayData<unsigned>*triangle_field_,const VertexMap*full_map_,const VertexMap*reduced_map_):
								name(name_),
								exponent(exponent_),
								full_map(full_map_),reduced_map(reduced_map_),
								vbo_vertex_indices(vbo_vertex_indices_),
								triangle_field(triangle_field_),
								least_traced_level(recursion_depth-1), range(0)
	{
		BYTE*context_at = context_field;
		#ifndef __GNUC__
			while (((__int64)context_at)&15)
				context_at++;
		#else
			while (((long)context_at)&15)
				context_at++;
		#endif
		context = (TContext*)context_at;
	
		context->has_canyons = true;
	
		context->noise_level = 1;
		context->oceanic_noise_level = 0.04f;
		context->sse_oceanic_noise_level = _mm_set1_ps(context->oceanic_noise_level);
		
		context->sector_size = sector_size_;
		context->sse_sector_size = _mm_set1_ps(sector_size_);
		context->recursive_depth = recursion_depth;
		context->edge_length = full_map->vertex_range;
		
		context->crater_min_count = 0;
		context->crater_max_count = 0;
		context->crater_radius = 2;
		context->crater_depth = 1;
		context->crater_min_radius = 0.1;
		
		context->fertility = 1.0f;
		context->sse_fertility = _mm_set1_ps(1.0f);
		context->temperature = 1.0f;
		context->sse_temperature = _mm_set1_ps(1.0f);
		context->has_ocean = true;
	}
	

	
	void		Body::clearFaces()
	{
		for (index_t i = 0; i < faces.Count(); i++)
		{
			INVOKING_DISCARD(faces[i]);
			Discard(faces[i]);
			INVOKED_DISCARD(faces[i]);
		}
		faces.free();
	}
	

	SurfaceSegment*	Body::anySegment(unsigned depth)
	{
		if (!this)
			return NULL;
		SurfaceSegment*result;
		for (index_t i = 0; i < faces.Count(); i++)
		{
			if (result = faces[i]->anyChild(depth))
				return result;
		}
		return NULL;
	}

	void				Body::subdivideAllSegments(bool generate_render_data)
	{
		if (!this)
			return;
		need_children.reset();
		for (index_t i = 0; i < faces.Count(); i++)
			faces[i]->registerLeafSegments();
		bool original_can_update = can_update;
		can_update = generate_render_data;
		while (processStep());
		can_update = original_can_update;
	}



	void		Body::purgeSegments()
	{
		for (index_t i = 0; i < faces.Count(); i++)
			faces[i]->dropChildren();
	}
	
	bool		Body::rayCast(const M::Composite::Coordinates&b, const M::TVec3<>&d, TRayIntersection&intersection)
	{
		
		intersection.isSet = false;
		TRayIntersection my_intersection;
		for (index_t i = 0; i < faces.Count(); i++)
		{
			if (faces[i]->rayCast(b,d,my_intersection) && (!intersection.isSet || my_intersection.fc[2] < intersection.fc[2]))
				intersection = my_intersection;
		}
		return intersection.isSet;
	}
	
	bool		Body::groundQuery(const M::Composite::Coordinates&b, unsigned min_layer, TGroundInfo&ground)
	{
		BackgroundSubDiv::end();
		M::TVec3<> d;
		ground.isSet = false;
		b.convertToAbsolute(d,context->sector_size);
		M::Vec::normalize0(d);
		M::Vec::mult(d,-1);
		for (index_t i = 0; i < faces.Count(); i++)
			if (faces[i]->groundQuery(b,d,min_layer,ground))
				return true;
		return false;
	}
	
	void		Body::defaultInit(TChannelConfig&vertex, Random&random, const TContext*context)
	{
		vertex.c0 = random.NextFloat(0,1);	//general height, riffs
		vertex.c1 = random.NextFloat(0,1);	//general height, riffs
		vertex.c2 = random.NextFloat(0,1);	//general height
		vertex.c3 = random.NextFloat(0,1);	//general height, height offset (1/2)
		vertex.oceanic = random.NextFloat(0,1);	//global scale height offset (low noise)
		

		vertex.water = random.NextFloat(0.0,0.5*context->fertility);	//water density
		//vertex.weight = 1.0f;//Adaption::ageAt(context->recursive_depth);
		
	}
	
	
	void		Body::rebuildFromGeometry(const Geometry&geometry, pInit init_function)
	{
		lout << "building shape from geometry"<<nl;
		lout << " vertices="<<geometry.vertexField.GetLength()<<nl;
		lout << " triangles="<<geometry.triangleField.GetLength()/3<<nl;
		
		context->sse_fertility = _mm_set1_ps(context->fertility);
		context->sse_temperature = _mm_set1_ps(context->temperature);
		context->sse_variance = _mm_set1_ps(context->variance);
		context->sse_sector_size = _mm_set1_ps(context->sector_size);
		context->sse_oceanic_noise_level = _mm_set1_ps(context->oceanic_noise_level);
		
		
		
		clearFaces();
		faces.SetSize(geometry.triangleField.GetLength()/3);
		for (index_t i = 0; i < faces.Count(); i++)
		{
			faces[i] = makeFace();
			ASSERT2__(isRegistered(faces[i]),i,faces[i]);
		}
		
		Random	random(root_seed.toInt());
		
		
		
		
		craters.reset();
		
		

		
		for (unsigned i = 0; i < faces.Count(); i++)
		{
			unsigned num_craters = random.Next(context->crater_min_count, context->crater_max_count);
			if (!num_craters)
				continue;
				
			
				
			M::TVec3<> d,dx,dy;
			
			const Geometry::Vertex	&v0 = geometry.vertexField[geometry.triangleField[i*3+0]],
									&v1 = geometry.vertexField[geometry.triangleField[i*3+1]],
									&v2 = geometry.vertexField[geometry.triangleField[i * 3 + 2]];
			
			M::Vec::sub(v1,v0,dx);
			M::Vec::sub(v2,v0,dy);
			
			for (unsigned j = 0; j < num_craters; j++)
			{
				float	x,
						y,
						r;
				while (true)
				{
					x = random.NextFloat(0,1);
					y = random.NextFloat(0,1);
					r = M::vpow(random.NextFloat(0,1),20);
					if (x+y <= 1 && r >= context->crater_min_radius)
						break;
				}
				
				M::Vec::mad(v0,dx,x,d);
				M::Vec::mad(d,dy,y);
				M::Vec::normalize0(d);
				
				TCrater&crater = craters.Append();
				crater.orientation = d;
				crater.radius = context->crater_radius*r;
				crater.depth = context->crater_depth;
				M::Vec::mult(crater.orientation,context->base_heightf,crater.base);
			}
		}


		Sorting::ByMethod::QuickSort(craters);//smallest to largest
		craters.Revert();	//revert to largest -> smallest
		
		
		
		
		

		
		Array<TVertex>		vertex_field(geometry.vertexField.GetLength());
		for (index_t i = 0; i < geometry.vertexField.GetLength(); i++)
		{
			init_function(vertex_field[i].channel,random,context);
			vertex_field[i].height = height<CLAMP_HEIGHT>(vertex_field[i].channel,context->has_canyons);
		}
		
		//link neighbors:
		
		typedef M::Mesh<M::TFaceGraphDef<TDef<float> > >	GraphMesh;
		GraphMesh	graph;
		
		_oMakeGraph(graph,geometry.triangleField,Array<unsigned>());

		for (index_t i = 0; i < faces.Count(); i++)
		{
			SurfaceSegment*f = faces[i];
			f->inner_seed.implant(root_seed,(BYTE)i);
			f->inner_seed.rotate(context->recursive_depth-1,(BYTE)i);
			for (BYTE k = 0; k < 3; k++)
				f->outer_seed[k].implant(f->inner_seed,k);
		}
		
		
		for (index_t i = 0; i < faces.Count(); i++)
		{
			DataSurface*f = faces[i];
			f->child_index = (BYTE)i;
			f->super = this;
			
			TVertex	*corner_field[3] = {f->vertex_field+full_map->border_index[0][0],f->vertex_field+full_map->border_index[1][0],f->vertex_field+full_map->border_index[2][0]};
			
			for (BYTE k = 0; k < 3; k++)
			{
				if (graph.triangle_field[i].n[k])
				{
					f->neighbor_link[k] =
							link(
									faces[(int)(graph.triangle_field[i].n[k].triangle-graph.triangle_field)],
		 							graph.triangle_field[i].n[k].GetIndexOf(graph.triangle_field[i].vertex[(k+1)%3])
								);
					f->outer_seed[k] = ((SurfaceSegment*)f->neighbor_link[k].segment)->outer_seed[f->neighbor_link[k].orientation];
				}
				else
				{
					f->neighbor_link[k] = TSurfaceSegmentLink();
				}
				//logfile << __LINE__<<nl;
				
				if (geometry.triangleField[i * 3 + k] >= geometry.vertexField.GetLength())
					EXCEPTION("index exception");
				//logfile << "0x"<<PointerToHex(faces[i]->root_map)<<nl;
				//logfile << i<<"/"<<sector_count<<", "<<k<<nl;
				TVertex&corner = *corner_field[k];
				const Geometry::Vertex&vertex = geometry.vertexField[geometry.triangleField[i * 3 + k]];
				corner = vertex_field[geometry.triangleField[i * 3 + k]];
				
				M::Vec::copy(vertex,(corner.position));
				M::Vec::setLen((corner.position),context->base_heightf);
			}
			M::TVec3<> center;
			M::Vec::center((corner_field[0]->position),(corner_field[1]->position),(corner_field[2]->position),center);
			float radius =  M::Vec::distance((corner_field[0]->position),center);
			for (index_t j = 0; j < craters.Count(); j++)
			{
				if (M::Vec::distance(craters[j].base,center)-craters[j].radius > radius)
					continue;
				
				f->crater_field << craters[j];
			}
			f->crater_field.compact();
			
			
			
			
			for (BYTE k = 0; k < 3; k++)
			{
				TVertex&corner = f->vertex_field[full_map->border_index[k][0]];
				//_c3(vertex.position,corner.position);
				
				Height::apply(corner.position,context->variance*corner.height,*faces[i],*context);
				
				faces[i]->corner[k] = corner.position;
			}
			for (BYTE k = 0; k < 3; k++)
				faces[i]->neighbor_link[k].primary = faces[i]->orderOfEdge(k);
			/*
			float	dx[3],
					dy[3];
			_sub(f->corner[0].v,f->corner[1].v,dy);
			_sub(f->corner[2].v,f->corner[1].v,dx);
			for (unsigned j = 0; j < full_map->vertex_count; j++)
			{
				_mad(f->corner[1].v,dx,(float)full_map->vertex_descriptor[j].x/(float)full_map->vertex_maximum,f->vertex_field[j].position);
				_mad(f->vertex_field[j].position,dy,1.0f-(float)full_map->vertex_descriptor[j].y/(float)full_map->vertex_maximum);
			}*/
			f->flags |= TBaseSurfaceSegment::HasData;
			
			//logfile << __LINE__<<nl;
			f->reinit();
			//logfile << __LINE__<<nl;
			/*for (BYTE k = 0; k < 3; k++)
			{
				TVertex&corner = faces[i]->vertex_field[faces[i]->root_map->border_index[k][0]];
				Adaption::setHeight(corner.uniform_position,faces[i]->uniform_height,*faces[i],sector_size);
				//_c3(faces[i]->getCorner(k).uniform_position,faces[i]->getCorner(k).position);
			}*/
			//logfile << __LINE__<<nl;
		}
		
		//logfile << __LINE__<<nl;
		//lout << "generating layers"<<nl;
		
		for (unsigned j = exponent-1; j >0; j--)
		{
			for (index_t i = 0; i < faces.Count(); i++)
			{
				faces[i]->generateLayer(j);
				//exit(0);
			}
			for (index_t i = 0; i < faces.Count(); i++)
				faces[i]->syncGeneratedBorders(j);
		}
		for (index_t i = 0; i < faces.Count(); i++)
			Kernel::generateFinalLayer(*faces[i], *context,*full_map);
		for (index_t i = 0; i < faces.Count(); i++)
			faces[i]->syncGeneratedBorders(0);
		
		
		//logfile << __LINE__<<nl;

		/*lout << "calculating normals"<<nl;
		
		for (unsigned i = 0; i < faces.Count(); i++)
			faces[i]->fillAllNormals();*/
		//lout << "finalizing"<<nl;
		//logfile << __LINE__<<nl;
		for (index_t i = 0; i < faces.Count(); i++)
		{
			//faces[i]->mergeBorderNormals();
			DataSurface*f = faces[i];
			
			f->flags |= TBaseSurfaceSegment::RequiresUpdate;
			f->finishConstruction();
			f->updateSphere();
			
			for (BYTE k = 0; k < 3; k++)
				if (f->neighbor_link[k].segment)
					f->flags |= (TBaseSurfaceSegment::Edge0Merged << k);
				else
					f->flags |= (TBaseSurfaceSegment::Edge0Open << k);
			//faces[i]->requires_update = true;
		}
		for (index_t i = 0; i < faces.Count(); i++)
			faces[i]->SurfaceSegment::update();
		//logfile << __LINE__<<nl;
		
		
		checkLinkage();
	}
	
	

	void		Body::reset()
	{
		face_buffer.reset();
		reflection_buffer.reset();
		water_buffer.reset();
		attachment_buffer.reset();
		//cloud_buffer.reset();
		least_traced_level = context->recursive_depth-1;
	}
	
	/*
	void		Body::clearChildren(unsigned depth)
	{
		least_traced_level = depth;
		for (unsigned i = 0; i < faces.Count(); i++)
        	faces[i]->dropChildren();
	}
	*/
	
	bool	Body::processStep()
	{
		BEGIN
		//DEBUG_POINT
		static unsigned step;
		if (!production)	//surface deleted
			step = 0;
		else
		{
			ASSERT_NOT_NULL__(production->c0);
			ASSERT_NOT_NULL__(production->c1);
			ASSERT_NOT_NULL__(production->c2);
			ASSERT_NOT_NULL__(production->c3);
		}
		
		#if SYSTEM==WINDOWS
			if (can_update)
				ASSERT__(wglGetCurrentContext() != 0);
		#endif
		
		switch (step)
		{
			case 0:
				//DEBUG_POINT
				if (!need_children.Count())
				{
					END
					return false;
				}
				if (SurfaceSegment::max_load && num_sectors+4 > SurfaceSegment::max_load)
				{
					need_children.reset();
					END
					return false;
				}
				production = need_children.pop();
				if (production->flags&TBaseSurfaceSegment::HasChildren)
				{
					//need_children.reset();
					//this case happens if a segment that just finished constructing has re-requested children during the last frame (which, in fact, is quite likely)
					production = NULL;
					return need_children.Count()>0;
				}

				production->createChildren(trace_time,false);
			break;
			case 1:
				production->c0->build();
			break;
			case 2:
				production->c1->build();
			break;
			case 3:
				production->c2->build();
			break;
			case 4:
				production->c3->build();
			break;
			case 5:
				//DEBUG_POINT
				//ASSERT_NOT_NULL__(production);
				//ASSERT_NOT_NULL__(production->c0);
				//DEBUG_POINT
				production->c0->mergeEdges();
				//DEBUG_POINT
			break;
			case 6:
				//DEBUG_POINT
				//ASSERT_NOT_NULL__(production);
				//ASSERT_NOT_NULL__(production->c1);
				//DEBUG_POINT
				production->c1->mergeEdges();
				//DEBUG_POINT
			break;
			case 7:
				//DEBUG_POINT
				//ASSERT_NOT_NULL__(production);
				//ASSERT_NOT_NULL__(production->c2);
				//DEBUG_POINT
				production->c2->mergeEdges();
				//DEBUG_POINT
			break;
			case 8:
				//DEBUG_POINT
				//ASSERT_NOT_NULL__(production);
				//ASSERT_NOT_NULL__(production->c3);
				//DEBUG_POINT
				production->c3->mergeEdges();
				//DEBUG_POINT
			break;
			case 9:
				if (can_update)
					production->c0->update();
			break;
			case 10:
				if (can_update)
					production->c1->update();
			break;
			case 11:
				if (can_update)
					production->c2->update();
			break;
			case 12:
				if (can_update)
					production->c3->update();
				production->flags |= TBaseSurfaceSegment::HasChildren;
			break;
		}
		step++;
		if (step > 12)
			step = 0;
			
		//DEBUG_POINT
		END

		return true;

	}

	void	BackgroundSubDiv::Process::ThreadMain()
	{
		while (!application_shutting_down)
		{
			int token;
			BackgroundSubDiv::begin_tickets >> token;

			while (BackgroundSubDiv::active && Body::processStep());

			BackgroundSubDiv::end_tickets << token;
		}

	}

	void	BackgroundSubDiv::begin()
	{
		BackgroundSubDiv::active = true;
		Kernel::background = true;
		Body::can_update = false;
		int token;
		BackgroundSubDiv::begin_tickets << token;

	}

	void	BackgroundSubDiv::end()
	{
		if (!active && !signaled)
			return;
		BackgroundSubDiv::active = false;
		Kernel::background = false;
		int token;
		BackgroundSubDiv::end_tickets >> token;
		signaled = false;
		Body::can_update = true;
		Body::flushQueue();
	}

	void	BackgroundSubDiv::signalEnd()
	{
		if (signaled)
			return;
		signaled = active;
		active = false;
	}


	void	Body::flushQueue()
	{
		need_children.reset();
	}

	void	Body::exportQueue(ArrayData<SurfaceSegment*>&target)
	{
		need_children.copyToArray(target);

	}

	
	void	Body::subdivide(float time_frame)
	{
		BEGIN
		if (!need_children.Count() && !production)
		{
			END
			return;
		}

		if (!SurfaceSegment::may_subdivide)
		{
			need_children.reset();
			END
			return;
		}

		
		//lout << "processing "<<need_children.GetLength()<<" entries"<<nl;
		if (need_children.IsNotEmpty())
			Sorting::ByComparator::QuickSort(need_children, [](const SurfaceSegment* a, const SurfaceSegment* b) {return a->priority - b->priority; });

		
		Time::Stamp started;
		do
		{
			if (!processStep())
			{
				END
				return;
			}
		}
		while ((Time::Stamp()-started).ToSeconds() < time_frame /*&& production*/);
		
		if (background_subdiv)
			BackgroundSubDiv::begin();
		else
			need_children.reset();
		END
	}

	
	void		Body::process(const Time::Stamp&time, float top_level_ttl, const Aspect&view)
	{
		BEGIN
		BackgroundSubDiv::end();
		trace_time = time;

		reset();
		//cout << "Body::process(...): projecting"<<endl;
		SurfaceSegment::allow_non_crucial_texture_update = true;
	
		for (index_t i = 0; i < faces.Count(); i++)
		{
			/*faces[i]->project_parameters.time = &trace_time;
			faces[i]->project_parameters.ttl = top_level_ttl;
			faces[i]->project_parameters.view = &view;*/
			faces[i]->setPriority(view, false, time);
			faces[i]->project(view,top_level_ttl,time,false);
			if (context->has_ocean)
			{
				faces[i]->setWaterPriority(view);
				faces[i]->projectWater(view);
				
				if (view.require_reflection)
				{
					faces[i]->setReflectionPriority(view);
					faces[i]->projectReflection(view);
				}
			}
			
			/*if (view.clouds)
				faces[i]->projectClouds(true);*/
		}
		//cout << "Body::process(...): projected. resolving"<<endl;
		
		//logfile << "-clearing waste pipe\n";
		SurfaceSegment*sector;
		
		//logfile << "-writing project map\n";
		/*for (unsigned i = 0; i < face_buffer.Count(); i++)
			face_buffer[i]->update();*/
		
	
		for (index_t i = 0; i < faces.Count(); i++)
		{
			faces[i]->resolve();
			faces[i]->resolveReflection();
			faces[i]->resolveWater();
		}
		

		//cout << "Body::process(...): done"<<endl;
		
		
		END
	}
	

	void		Body::processInvisible(const Time::Stamp&time, float top_level_ttl)
	{
		BEGIN
		BackgroundSubDiv::end();
		trace_time = time;

		reset();
		//cout << "Body::process(...): projecting"<<endl;
	
		for (unsigned i = 0; i < faces.Count(); i++)
			faces[i]->checkTimeouts(time,top_level_ttl);
		
		END
	}
	

	
	float		Body::currentOverload()
	{
		ASSERT__(!BackgroundSubDiv::active);
		float rs = 0;
		for (unsigned i = 0; i < need_children.Count(); i++)
			rs += need_children[i]->priority;
		return rs;
	}
	
	void		Body::checkLinkage()
	{
		for (unsigned i = 0; i < faces.Count(); i++)
			faces[i]->checkLinkage();
	}
	/*
	SurfaceSegment*		Body::detectVisualPathCollision(const double p0[3],const double p1[3],const TSector&sector, float&scalar_out, float point_out[3],float normal_out[3], TSector&sector_out)
	{
		VisualPathCollisionCallback	callback(p0,p1,sector,scalar_out,point_out,normal_out,sector_out);
		return recursiveLookup(callback);
	}
	SurfaceSegment*		Body::detectVisualCollision(const double p0[3],const double p1[3],const TSector&sector, float&scalar_out, float point_out[3],float normal_out[3], TSector&sector_out)
	{
		VisualCollisionCallback	callback(p0,p1,sector,scalar_out,point_out,normal_out,sector_out);
		return recursiveLookup(callback);
	}
	SurfaceSegment*		Body::detectCollision(const double p0[3],const double p1[3],const TSector&sector, float&scalar_out, float point_out[3],float normal_out[3], TSector&sector_out)
	{
		SurfaceSegment*rs(NULL);
		if (rs==detectVisualCollision(p0,p1,sector,scalar_out,point_out,normal_out,sector_out))
			return rs;
		if (rs==detectVisualPathCollision(p0,p1,sector,scalar_out,point_out,normal_out,sector_out))
			return rs;
		CollisionCallback	callback(p0,p1,sector,scalar_out,point_out,normal_out,sector_out);
		return recursiveLookup(callback);
	}
	
	SurfaceSegment*		Body::recursiveLookup(Callback&callback, unsigned depth)
	{
		SurfaceSegment*rs;
		if (last_match)
		{
			if (rs = last_match->recursiveLookup(callback,depth))
			{
				last_match = rs;
				return rs;
			}
			if (last_match->n0.segment && (rs = last_match->n0.segment->recursiveLookup(callback,depth)))
			{
				last_match = rs;
				return rs;
			}
			if (last_match->n1.segment && (rs = last_match->n1.segment->recursiveLookup(callback,depth)))
			{
				last_match = rs;
				return rs;
			}
			if (last_match->n2.segment && (rs = last_match->n2.segment->recursiveLookup(callback,depth)))
			{
				last_match = rs;
				return rs;
			}
		}
		for (unsigned i = 0; i < faces.Count(); i++)
			if (rs = faces[i]->recursiveLookup(callback,depth))
			{
				last_match = rs;
				return rs;
			}
		return NULL;
	}
*/

	AbstractIndexPatterns::AbstractIndexPatterns(UINT32*const pattern_, UINT32*const pattern_offset_, UINT32*const core_pattern_, UINT32 num_patterns_, UINT32 exponent_):
						pattern(pattern_),pattern_offset(pattern_offset_),core_pattern(core_pattern_),num_patterns(num_patterns_),exponent(exponent_),
						step0((exponent_+1)*(exponent_+1)),step1(exponent_+1)
	{}
	
	unsigned	AbstractIndexPatterns::getPatternIndexFor(const SurfaceSegment*seg, const SurfaceSegment::TNeighborDepthConfig&config)	const
	{
		//logfile << "attempting to get pattern index for 0x"<<PointerToHex(sector)<<nl;
        unsigned	d0 = config.delta0 - seg->depth,
					d1 = config.delta1 - seg->depth,
					d2 = config.delta2 - seg->depth;
		//logfile << d0<<", "<<d1<<", "<<d2<<nl;
        if (d0 > exponent)
            d0 = exponent;
        if (d1 > exponent)
            d1 = exponent;
        if (d2 > exponent)
            d2 = exponent;
		//logfile << "M::Clamp: "<<d0<<", "<<d1<<", "<<d2<<nl;
        return  d0*step0 + d1*step1 + d2;
	}
	
	unsigned	AbstractIndexPatterns::getPatternIndexFor(const SurfaceSegment::TNeighborDepthConfig&config,const SurfaceSegment*seg)	const
	{
		//logfile << "attempting to get pattern index for 0x"<<PointerToHex(sector)<<nl;
        unsigned	d0 = config.delta0 - seg->depth,
					d1 = config.delta1 - seg->depth,
					d2 = config.delta2 - seg->depth;
		//logfile << d0<<", "<<d1<<", "<<d2<<nl;
        if (d0 > exponent)
            d0 = exponent;
        if (d1 > exponent)
            d1 = exponent;
        if (d2 > exponent)
            d2 = exponent;
		//logfile << "M::Clamp: "<<d0<<", "<<d1<<", "<<d2<<nl;
        return  d0*step0 + d1*step1 + d2;
	}
	

	void				makeHaloGeometry(Geometry&geometry)
	{
		static const unsigned resolution = 20;
	
		geometry.vertexField.SetSize(resolution*2);
		geometry.triangleField.SetSize(resolution*6);
		unsigned *f = geometry.triangleField.pointer();
		#undef t
		#define t(i0,i1,i2)	{(*f++) = i0; (*f++) = i1; (*f++) = i2;}
			for (unsigned i = 0; i < resolution; i++)
			{
				t(i,(i+1)%resolution,i+resolution);
				t((i+1)%resolution,(i+1)%resolution+resolution,i+resolution);
			}
		#undef t
		ASSERT_CONCLUSION(geometry.triangleField,f);

		float step = 360.0f/resolution;
		Geometry::Vertex*v = geometry.vertexField.pointer();
		for (unsigned i = 0; i < resolution; i++)
			M::Vec::sphereCoords(step*i,-step/2,1,(*v++));
		for (unsigned i = 0; i < resolution; i++)
			M::Vec::sphereCoords(step*i+step/2,step/2,1,(*v++));
		
		ASSERT_CONCLUSION(geometry.vertexField, v);
	}
	
	Geometry			makeHaloGeometry()
	{
		Geometry result;
		makeHaloGeometry(result);
		return result;
	}
	
	Geometry			makeSphereGeometry()
	{
		Geometry result;
		makeSphereGeometry(result);
		return result;
	}

	
	void	makeSphereGeometry(Geometry&geometry)
	{
		//4 sided:
		/*
		geometry.vertex_field.resize(4);
		geometry.index_field.resize(12);
		unsigned *f = geometry.index_field;
		#undef t
		#define t(i0,i1,i2)	{(*f++) = i0; (*f++) = i1; (*f++) = i2;}
		    t(1,3,2);
		    t(0,2,3);
		    t(1,0,3);
		    t(2,0,1);
		#undef t
		ASSERT_CONCLUSION(geometry.index_field,f);

		_v3(geometry.vertex_field[0].position,-1,-1,-1);
		_v3(geometry.vertex_field[1].position,1,-1,1);
		_v3(geometry.vertex_field[2].position,-1,1,1);
		_v3(geometry.vertex_field[3].position,1,1,-1);
		*/
		
		//8 sided:
		/*
		geometry.vertex_field.resize(6);
		geometry.index_field.resize(24);
		unsigned *f = geometry.index_field;
		#undef t
		#define t(i0,i1,i2)	{(*f++) = i0; (*f++) = i1; (*f++) = i2;}
		    t(0,1,2);
		    t(0,2,3);
		    t(0,3,4);
		    t(0,4,1);
			
			t(5,2,1);
			t(5,3,2);
			t(5,4,3);
			t(5,1,4);
		#undef t
		ASSERT_CONCLUSION(geometry.index_field,f);

		_v3(geometry.vertex_field[0].position,0,1,0);
		_v3(geometry.vertex_field[1].position,-1,0,0);
		_v3(geometry.vertex_field[2].position,0,0,1);
		_v3(geometry.vertex_field[3].position,1,0,0);
		_v3(geometry.vertex_field[4].position,0,0,-1);
		_v3(geometry.vertex_field[5].position,0,-1,0);
		*/
	
		//20 sided:
		
		geometry.vertexField.SetSize(12);
		geometry.triangleField.SetSize(60);
		geometry.edgeField.SetSize(60);
		UINT32 *f = geometry.triangleField.pointer();
		#undef t
		#define t(i0,i1,i2)	{(*f++) = i0; (*f++) = i1; (*f++) = i2;}
		    t(0,1,2);
		    t(0,2,3);
		    t(0,3,4);
		    t(0,4,5);
		    t(0,5,1);

		    t(2,1,6);
		    t(3,2,7);
		    t(4,3,8);
		    t(5,4,9);
		    t(1,5,10);

		    t(2,6,7);
		    t(3,7,8);
		    t(4,8,9);
		    t(5,9,10);
		    t(1,10,6);

		    t(7,6,11);
		    t(8,7,11);
		    t(9,8,11);
		    t(10,9,11);
		    t(6,10,11);
		#undef t
		ASSERT_CONCLUSION(geometry.triangleField,f);

		UINT32 *e = geometry.edgeField.pointer();
		#undef E
		#define E(i0,i1)	{(*e++) = i0; (*e++) = i1; }
		    E(0,1);
		    E(0,2);
		    E(0,3);
		    E(0,4);
		    E(0,5);

			E(5, 1);
			E(1, 2);
			E(2, 3);
			E(3, 4);
			E(4, 5);

			E(1, 10);
			E(1, 6);
			E(2, 6);
			E(2, 7);
			E(3, 7);
			E(3, 8);
			E(4, 8);
			E(4, 9);
			E(5, 9);
			E(5, 10);

			E(10, 6);
			E(6, 7);
			E(7, 8);
			E(8, 9);
			E(9, 10);

			E(6, 11);
			E(7, 11);
			E(8, 11);
			E(9, 11);
			E(10, 11);
		#undef E
		ASSERT_CONCLUSION(geometry.edgeField,e);


		M::Vec::sphereCoords(0,90,1,geometry.vertexField[0]);
	    
		M::Vec::sphereCoords(0, 30, 1, geometry.vertexField[1]);
		M::Vec::sphereCoords(-72, 30, 1, geometry.vertexField[2]);
		M::Vec::sphereCoords(-144, 30, 1, geometry.vertexField[3]);
		M::Vec::sphereCoords(-216, 30, 1, geometry.vertexField[4]);
		M::Vec::sphereCoords(-288, 30, 1, geometry.vertexField[5]);

		M::Vec::sphereCoords(-36, -30, 1, geometry.vertexField[6]);
		M::Vec::sphereCoords(-108, -30, 1, geometry.vertexField[7]);
		M::Vec::sphereCoords(-180, -30, 1, geometry.vertexField[8]);
		M::Vec::sphereCoords(-252, -30, 1, geometry.vertexField[9]);
		M::Vec::sphereCoords(-324, -30, 1, geometry.vertexField[10]);

		M::Vec::sphereCoords(0, -90, 1, geometry.vertexField[11]);
		
	}
	
	RollSpace::RollSpace(double modal_range_):modal_range(modal_range_)
	{
		M::Vec::clear(texture_root);
		M::Vec::def(texture_up,0,1,0);
		M::Vec::def(texture_x,1,0,0);
		M::Vec::def(texture_y,0,0,1);
		M::Mat::Eye(invert);
	}
	
	void	RollSpace::roll(const M::TVec3<>&up, const M::TVec3<>&location)
	{
		M::TVec3<double>	up_ = {up.x,up.y,up.z},
						location_ = {location.x,location.y,location.z};
		roll(up_,location_);
	}
	
	void	RollSpace::roll(const M::TVec3<>&up, const M::TVec3<double>&location)
	{
		M::TVec3<double> up_ = {up.x,up.y,up.z};
		roll(up_,location);
	}

	void	RollSpace::mod(double&val)	const
	{
		int64_t step = (int64_t)(val/modal_range);
		if (step < 0)
			step--;
		val -= modal_range*step;
	}
	
	void	RollSpace::roll(const M::TVec3<double>& up, const M::TVec3<double>&delta)
	{
	    M::TMatrix4<double> sys;
		M::TVec3<double>	projection,temp;
		double translation = M::Vec::dot(delta,up);
		//_mad(texture_root,up,-translation);
		//_add(texture_root,delta);
		
	
	    M::Vec::add(up,texture_up,projection);
	    M::Vec::mad(texture_x,projection,-M::Vec::dot(texture_x,up)/M::Vec::dot(projection,up));
	    M::Vec::copy(up,texture_up);
	    M::Vec::cross(texture_x,up,texture_y);
	    M::Vec::normalize0(texture_up);
	    M::Vec::normalize0(texture_x);
	    M::Vec::normalize0(texture_y);
		
		//texture_root[0] = fmod(texture_root[0],modal_range);
		//texture_root[1] = fmod(texture_root[1],modal_range);
		//texture_root[2] = fmod(texture_root[2],modal_range);

		//mod(texture_root[0]);
		//mod(texture_root[1]);
		//mod(texture_root[2]);

		
	    //shiver me timbers. the seas have got me - yar.
		//project root to plane:
		M::Vec::mad(texture_root,texture_x,-M::Vec::dot(texture_x,delta));
		M::Vec::mad(texture_root,texture_y,-M::Vec::dot(texture_y,delta));
		M::Vec::mad(texture_root,texture_up,-M::Vec::dot(texture_up,texture_root));
		
		//modulate root:
		{
			double	x = M::Vec::dot(texture_root,texture_x),
					y = M::Vec::dot(texture_root,texture_y);
			mod(x);
			mod(y);
			M::Vec::mult(texture_x,x,texture_root);
			M::Vec::mad(texture_root,texture_y,y);
		}
		/*
		mod(texture_root[0]);
		mod(texture_root[1]);
		mod(texture_root[2]);*/
		
		/*int mod[3];
		_div(texture_root,modal_range,mod);
		_mad(texture_root,mod,-modal_range);*/
		

	    M::Vec::copy(texture_x,sys.x.xyz);
	    M::Vec::copy(texture_y,sys.y.xyz);
	    M::Vec::copy(texture_up,sys.z.xyz);
	    M::Vec::copy(texture_root,sys.w.xyz);
		M::Mat::resetBottomRow(sys);
	    M::Mat::invertSystem(sys,invert);
	}


	void	RollSpace::exportTo(M::TMatrix4<>&system, double range, bool include_translation)	const
	{
	    M::Vec::div(invert.x.xyz,range,system.x.xyz);
	    M::Vec::div(invert.y.xyz,range,system.y.xyz);
		M::Vec::div(invert.z.xyz,range,system.z.xyz);
		if (include_translation)
			M::Vec::div(invert.w.xyz,range,system.w.xyz);
		else
			M::Vec::clear(system.w.xyz);
		M::Mat::resetBottomRow(system);
	}



	
	String	getConfiguration()
	{
		return			 	"Fractal kernel " FRACTAL_VERSION "\n"
							"Secondary parent weight="+String(SECONDARY_PARENT_WEIGHT*100)+"%\n"
							"\nVertex size="+String(sizeof(TVertex))+
							"\nWater gain down="+String(WATER_GAIN_DOWN)+
							"\nWater gain up="+String(WATER_GAIN_UP)+
							"\nWater noise="+String(WATER_NOISE);
	}
	
	float		relativeSeamWidth(unsigned exponent)
	{
		return (1.0f+(float)TEXTURE_SEAM_EXTENSION)/(float)((1<<exponent)+1+1+TEXTURE_SEAM_EXTENSION);
	}
	float		relativeTexelSize(unsigned exponent)
	{
		return (1.0f)/(float)((1<<exponent)+1+1+TEXTURE_SEAM_EXTENSION);
	}
	
	
	void	(*SurfaceSegment::onUpdate)(SurfaceSegment*surface)=NULL;
	void	(*SurfaceSegment::onSegmentAvailable)(SurfaceSegment*surface)=NULL;
	unsigned		SurfaceSegment::max_load=0;
	bool			SurfaceSegment::may_subdivide = true;
	
}
