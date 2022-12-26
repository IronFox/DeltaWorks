#include "../global_root.h"
#include "graph_mesh.h"


namespace DeltaWorks
{
	/*static*/	count_t								SurfaceNetwork::numLODs=4;
	/*static*/	float								SurfaceNetwork::minTolerance=0.05f;

	using namespace Math;
	
	static const UINT GL_MODULATE = 0x2100;
	static const UINT GL_DECAL = 0x2101;
	

	template <typename T>
		static inline void	parse(const XML::Node&node, const char*attrib_name,M::TVec2<T>&out)
		{
			static String string;
			static Ctr::Array<String>	segments;
		
			if (!node.Query(attrib_name,string))
				throw Except::IO::DriveAccess::FileFormatFault("Unable to retrieve attribute '"+String(attrib_name)+"' from XML node '"+node.name+"'");
			
			Explode(',',string,segments);
			if (segments.Count() != 2)
				throw Except::IO::DriveAccess::FileFormatFault("Unexpected segment count in attribute '"+String(attrib_name)+"' of XML node '"+node.name+"': "+String(segments.Count())+". Expected 2 comma separated segments in '"+string+"'");

			if (!convert(segments[0].Trim().c_str(),out.x))
				throw Except::IO::DriveAccess::FileFormatFault("Conversion of segment 0 ('"+segments[0].Trim()+"') for attribute '"+attrib_name+"' of XML node '"+node.name+"' failed");

			if (!convert(segments[1].Trim().c_str(),out.y))
				throw Except::IO::DriveAccess::FileFormatFault("Conversion of segment 1 ('"+segments[0].Trim()+"') for attribute '"+attrib_name+"' of XML node '"+node.name+"' failed");

		
		}

	void	Graph::LoadFromFile(const PathString&filename, bool compact)
	{
		XML::Container	xml;
		xml.LoadFromFile(filename);

	
		const XML::Node	*xnodes = xml.Find("graph/nodes"),
						*xedges = xml.Find("graph/edges");
		if (!xnodes || !xedges)
			throw Except::IO::DriveAccess::FileFormatFault("XML Graph file lacks 'nodes' and/or 'edges' tag");
	
		nodes.reset();
		edges.reset();
	
		for (index_t i = 0; i < xnodes->children.Count(); i++)
		{
			const XML::Node&xnode = xnodes->children[i];
			Node&node = nodes.Append();
			node.reset();
		
			parse(xnode,"position",node.position);
			parse(xnode,"direction",node.direction);
		}
	
	
		for (index_t i = 0; i < xedges->children.Count(); i++)
		{
			const XML::Node&xedge = xedges->children[i];
			Edge&edge = edges.Append();
			edge.reset();
		
			parse(xedge,"nodes",M::Vec::ref2(edge.node));
			parse(xedge,"control",M::Vec::ref2(edge.control_dist));
		
			if (nodes[edge.node[0]].out_edge != -1)
			{
				edges.pop();
				continue;
			}
			if (nodes[edge.node[1]].in_edge != -1)
			{
				edges.pop();
				continue;
			}
			nodes[edge.node[1]].in_edge = edges.Count()-1;
			nodes[edge.node[0]].out_edge = edges.Count()-1;
		}
	
		if (compact)
		{
			nodes.compact();
			edges.compact();
		}
	}

	void	Graph::SaveToFile(const PathString&filename)	const
	{
		XML::Container	xml;
	
		XML::Node	&xgraph = xml.Create("graph");
		{
			XML::Node	&xnodes = xgraph.Create("nodes");
			for (index_t i = 0; i < nodes.Count(); i++)
			{
				const Node&node = nodes[i];
				XML::Node	&xnode = xnodes.Create("node");
				xnode.Set("position",String(node.position.x)+", "+String(node.position.y));
				xnode.Set("direction",String(node.direction.x)+", "+String(node.direction.y));
			}
		}
		{
			XML::Node	&xedges = xgraph.Create("edges");
			for (index_t i = 0; i < edges.Count(); i++)
			{
				const Edge&edge = edges[i];
				XML::Node	&xedge = xedges.Create("edge");
				xedge.Set("nodes",String(edge.node[0])+", "+String(edge.node[1]));
				xedge.Set("control",String(edge.control_dist[0])+", "+String(edge.control_dist[1]));
			}
		}
	
		xml.SaveToFile(filename);
	}



	/*static*/	const char*			Graph::ToString(layout_t layout)
	{
	#undef ECASE
	#define ECASE(CASE)	case CASE: return #CASE;
		switch (layout)
		{
			ECASE(Plain)
			ECASE(Split)
			ECASE(Ramp)
			default:
				return "Unknown";
		}
	#undef ECASE
	}

	/*static*/	bool				Graph::decode(const String&string, layout_t&layout)
	{
		if (string == "Plain")
		{
			layout = Plain;
			return true;
		}
		if (string == "Split")
		{
			layout = Split;
			return true;
		}
		if (string == "Ramp")
		{
			layout = Ramp;
			return true;
		}
		return false;
	}



	/*

	static void generateCaps(Graph&xy_profile, Profile&zy_profile, float step, Buffer<ObjectMath::Object<TDef<float> > >&caps)
	{

		static Buffer<ObjectMath::Object<TDef<float> > >	shapes;
	
		bool simplify = true;
		while (simplify)
		{
			ObjectMath::Object<TDef<float> >&cap = caps.Append();
		
			shapes.reset();
		
			for (unsigned i = 0; i < xy_profile.nodes.Count(); i++)
				xy_profile.nodes[i].processed = false;
		
			unsigned processed = 0;
			while (processed < xy_profile.nodes.Count())
			{
				unsigned start_node = -1;
				for (unsigned i = 0; i < xy_profile.nodes.Count(); i++)
					if (!xy_profile.nodes[i].processed && xy_profile.nodes[i].in_edge==-1)
					{
						if (xy_profile.nodes[i].out_edge == -1)
						{
							xy_profile.nodes[i].processed = true;
							processed++;
							continue;
						}
						start_node = i;
						break;
					}
				if (processed == xy_profile.nodes.Count())
					break;
				if (start_node < xy_profile.nodes.Count())
					for (unsigned i = 0; i < xy_profile.nodes.Count(); i++)
						if (!xy_profile.nodes[i].processed)
						{
							start_node = i;
							break;
						}
				ASSERT__(start_node != -1);
			
				static Buffer<float>	points;
				points.reset();
			
				unsigned node = start_node;
				while (node < xy_profile.nodes.Count())
				{
					if (xy_profile.nodes[node].processed || xy_profile.nodes[node].out_edge>= xy_profile.edges.Count())
						break;
					Graph::Edge	&edge = xy_profile.edges[xy_profile.nodes[node].out_edge];
					unsigned next = edge.node[1];
					if (next == node)
						next = edge.node[0];
					if (next >= xy_profile.nodes.Count())
						break;
				
					Graph::Node	&n0 = xy_profile.nodes[node],
									&n1 = xy_profile.nodes[next];
					n0.processed = true;
				
					if (!!edge.control_dist[0] || !!edge.control_dist[1])
					{
						float len = M::Vec::distance2(n0.position,n1.position);
						M::Vec::mad2(n0.position,n0.direction,len*NURBS_CONST*edge.control_dist[0],edge.control[0]);
						M::Vec::mad2(n1.position,n1.direction,-len*NURBS_CONST*edge.control_dist[1],edge.control[1]);
						unsigned resolution = vmax((unsigned)M::Round(len/step),2);
						for (unsigned j = 1; j < resolution; j++)
						{
							float fc = (float)j/(resolution-1);
							float p[2],a[2];
							M::Vec::resolveNURBS2(n0.position,edge.control[0],edge.control[1],n1.position,fc,p);
							M::Vec::resolveNURBSaxis2(n0.position,edge.control[0],edge.control[1],n1.position,fc,a);
							if (_zero2(a))
								M::Vec::resolveNURBSaxis2(n0.position,edge.control[0],edge.control[1],n1.position,fc*0.99+0.5*0.01,a);
							float n[2];
							M::Vec::c2(a,n);
							swp(n[0],n[1]);
							n[1] *= -1;
						
							M::Vec::mad2(p,n,zy_profile.nodes.First().position
						
							points.append(p,2);
						}
					}
				}
				if (points.Count() > 2)
					ASSERT__(_oTriangulate(points.pointer(), points.Count()/2, shapes.appendAdopt()));
			}
			caps[0].clear();
			caps[0].join(shapes.pointer(),shapes.Count());
			step *= 2;
		}
	}
	*/
	/*
	void		Graph::generateOutline(float step, Outline&outline)
	{

		for (unsigned i = 0; i < nodes.Count(); i++)
			nodes[i].processed = false;
	
		unsigned processed = 0;
		while (processed < nodes.Count())
		{
			unsigned start_node = -1;
			for (unsigned i = 0; i < nodes.Count(); i++)
				if (!nodes[i].processed && nodes[i].in_edge==-1)
				{
					if (nodes[i].out_edge == -1)
					{
						nodes[i].processed = true;
						processed++;
						continue;
					}
					start_node = i;
					break;
				}
			if (processed == nodes.Count())
				break;
			if (start_node < nodes.Count())
				for (unsigned i = 0; i < nodes.Count(); i++)
					if (!nodes[i].processed)
					{
						start_node = i;
						break;
					}
			ASSERT__(start_node != -1);
		
			Shape&shape = outline.shapes.appendAdopt();
		
			unsigned node = start_node;
			while (node < nodes.Count())
			{
				if (nodes[node].processed || nodes[node].out_edge >= edges.Count())
					break;
				Graph::Edge	&edge = edges[nodes[node].out_edge];
				unsigned next = edge.node[1];
				if (next == node)
					next = edge.node[0];
				if (next >= xy_profile.nodes.Count())
					break;
			
				Graph::Node	&n0 = nodes[node],
								&n1 = nodes[next];
				n0.processed = true;
			
				if (!!edge.control_dist[0] || !!edge.control_dist[1])
				{
					float len = M::Vec::distance2(n0.position,n1.position);
					M::Vec::mad2(n0.position,n0.direction,len*NURBS_CONST*edge.control_dist[0],edge.control[0]);
					M::Vec::mad2(n1.position,n1.direction,-len*NURBS_CONST*edge.control_dist[1],edge.control[1]);
					unsigned resolution = vmax((unsigned)M::Round(len/step),2);
					for (unsigned j = 1; j < resolution; j++)
					{
						float fc = (float)j/(resolution-1);
						float p[2];
						Shape::Node&sn = shape.nodes.Append();
					
						M::Vec::resolveNURBS2(n0.position,edge.control[0],edge.control[1],n1.position,fc,sn.position);
						M::Vec::resolveNURBSaxis2(n0.position,edge.control[0],edge.control[1],n1.position,fc,sn.normal);
						swp(sn.normal[0],sn.normal[1]);
						sn.normal[0]*=-1;
					}
				}
			}
			shape.nodes.compactify();
		}
		outline.shapes.compactifyAdopt();
	}
	*/


	/*static*/ void		Graph::splitBend(SurfaceDescription::TVertex&vtx)
	{
		float z = (-vtx.position.z*0.5f+0.5f);

		static const float	z_stretch = 2.0f,//1.75f,
							r = 8.0f,
							angular_range = asin(z_stretch/r);

		float angle = angular_range * z;

		M::TVec3<>	x_axis = {(float)cos(angle),0,(float)sin(angle)},
				z_axis = {(float)-sin(angle),0,(float)cos(angle)},
				center = {(r - (float)cos(angle)*r)*2.0f*z_stretch, 0, (float)-sin(angle)*r*2.0f*z_stretch + 1};
		float x = vtx.position.x;
		vtx.position.x = center.x + x_axis.x * x;
		vtx.position.z = center.z + x_axis.z * x;
		x = vtx.normal.x;
		vtx.normal.x = x_axis.x * x + z_axis.x * vtx.normal.z;
		vtx.normal.z = x_axis.z * x + z_axis.z * vtx.normal.z;
		x = vtx.tangent.x;
		vtx.tangent.x = x_axis.x * x + z_axis.x * vtx.tangent.z;
		vtx.tangent.z = x_axis.z * x + z_axis.z * vtx.tangent.z;
	
		vtx.tcoord.y *= z_stretch;
	
		/*
		static const float c = 4.0f;

		M::TVec3<>	center = {z*z*c,0,vtx.position.z},
				z_axis = {2.0f*z*c,0.0f,-1.0f};
		M::Vec::normalize(z_axis);
		M::TVec3<>	x_axis = {-z_axis.z,0,z_axis.x},
				y_axis = {0,1,0};
		float x = vtx.position.x;
		vtx.position.x = center.x + x_axis.x * x;
		vtx.position.z = center.z + x_axis.z * x;


		//M::TVec2<>	z_begin = {0,-1},
			//	z_end = {1,1},

		//vtx.position.x += vtx.position.z;
		*/
	
	}



	/*static*/ void		Graph::ramp(SurfaceDescription::TVertex&vtx)
	{
		float z = (-vtx.position.z*0.5f+0.5f) * 0.98f;
	
		float inclination = vtx.position.y/2.0f * 0.98f;

		vtx.position.y *= (1.0f-z);

		M::TVec3<>	normal = {0,1,-inclination};
		M::Vec::normalize(normal);
		if (vtx.normal.y < 0)
			M::Vec::mult(normal,-1);

		M::Vec::interpolate(vtx.normal,normal,z,vtx.normal);

	}


	bool		Graph::defineSurface(const Profile::Node&p0,const Profile::Node&p1,index_t edge_index,float x_step, float z_step,float texture_size_x, float texture_size_y,SurfaceDescription&desc, layout_t layout, float simplify_x)	const
	{
		index_t voffset = desc.vertices.Count();
	
		const Edge	&edge = edges[edge_index];
		const Node	&n0 = nodes[edge.node[0]],
					&n1 = nodes[edge.node[1]];

		float	dist_x = M::Vec::distance(n0.position,n1.position),
				dist_z = M::Vec::distance(p0.position,p1.position);
		M::TVec2<>	z_control0,
				z_control1,
				edir,
				zdir;
		M::Vec::sub(n1.position,n0.position,edir);
		M::Vec::sub(p1.position,p0.position,zdir);
		M::Vec::normalize0(edir);
		M::Vec::normalize0(zdir);
	
		//float	x_step = step,
		//vmax(1.0f-vmin(,_dot2(n1.direction,edir)),0.01)/vmax(1-vmax(edge.control_dist[0],edge.control_dist[1]),0.01),
	
		//*vmax(1.0f-_dot2(n0.direction,edir)*0.5-(1-edge.control_dist[0])-_dot2(n1.direction,edir)*0.5-(1-edge.control_dist[1]),0),
		//		z_step = step;
		if (!edge.control_dist[0] && !edge.control_dist[1] && simplify_x)
			x_step *= 10;
		if (layout == Graph::Ramp && !p0.control_dist[1] && !p1.control_dist[0])
			z_step *= 10;	//either loading for split, in which case we need increased z-resolution anyway, or loading plain, in which case this geometry will be used for bending. only ramp has use of this setting
	

		using std::max;

		unsigned	res_x = max((unsigned)M::Round(dist_x/x_step),2u),
					res_z = max((unsigned)M::Round(dist_z/z_step),2u);
	
		M::TVec2<>		control[2];


		float curve_factor = 0.39f;
	
		if (edge.control_dist[0] > 0 || edge.control_dist[1] > 0)
			M::Vec::mad(n0.position,n0.direction,dist_x*curve_factor*edge.control_dist[0],control[0]);
		else
			M::Vec::mad(n0.position,edir,dist_x*curve_factor,control[0]);
		if (edge.control_dist[0] > 0 || edge.control_dist[1] > 0)
			M::Vec::mad(n1.position,n1.direction,-dist_x*curve_factor*edge.control_dist[1],control[1]);
		else
			M::Vec::mad(n1.position,edir,-dist_x*curve_factor,control[1]);
	
		if (p0.control_dist[1] > 0||p1.control_dist[0] > 0)
			M::Vec::mad(p0.position,p0.direction,dist_z*curve_factor*p0.control_dist[1],z_control0);
		else
			M::Vec::mad(p0.position,zdir,dist_z*curve_factor,z_control0);
		if (p0.control_dist[1] > 0||p1.control_dist[0] > 0)
			M::Vec::mad(p1.position,p1.direction,-dist_z*curve_factor*p1.control_dist[0],z_control1);
		else
			M::Vec::mad(p1.position,zdir,-dist_z*curve_factor,z_control1);
	
	
		static Container::PreAllocVector<Node>	profile;
		profile.reset();
		for (unsigned i = 0; i < res_x; i++)
		{
			float fc = (float)i/(res_x-1);
			Node&node = profile.Append();
			M::Vec::resolveBezierCurvePoint(n0.position,control[0],control[1],n1.position,fc,node.position);
			M::Vec::resolveBezierCurveAxis(n0.position,control[0],control[1],n1.position,fc,node.direction);
			if ((!i && !edge.control_dist[0]) || (i+1 == res_x && !edge.control_dist[1]))
				M::Vec::resolveBezierCurveAxis(n0.position,control[0],control[1],n1.position,(fc*0.9+0.5*0.1),node.direction);
			M::Vec::normalize0(node.direction);
		}
	
		for (unsigned z = 0; z < res_z; z++)
		{
			float fz = (float)z/(res_z-1);
			M::TVec2<> p,a,np;
			M::Vec::resolveBezierCurvePoint(p0.position,z_control0,z_control1,p1.position,fz,p);
			M::Vec::resolveBezierCurveAxis(p0.position,z_control0,z_control1,p1.position,fz,a);
			if (M::Vec::zero(a))
				M::Vec::resolveBezierCurveAxis(p0.position,z_control0,z_control1,p1.position,(fz*0.9+0.5*0.1),a);
			M::Vec::normalize0(a);
			np = a;
			swp(np.x,np.y);
			np.x *= -1;
		
			for (unsigned x = 0; x < res_x; x++)
			{
				float fx = (float)x/(res_x-1);
				const Node&node = profile[x];
				SurfaceDescription::TVertex&vtx = desc.vertices.Append();
				vtx.position.z = p.x;
			
				M::TVec2<> n = node.direction;
				swp(n.x,n.y);
				n.y *= -1;
			
				if (!x && !edge.control_dist[0])
				{
					M::TVec2<> n = n0.direction;
					swp(n.x,n.y);
					n.y *= -1;
					M::Vec::mad(node.position,n,p.y,vtx.position.xy);
				}
				elif (x+1 == res_x && !edge.control_dist[1])
				{
					M::TVec2<> n = n1.direction;
					swp(n.x,n.y);
					n.y *= -1;
					M::Vec::mad(node.position,n,p.y,vtx.position.xy);
				}
				else
					M::Vec::mad(node.position,n,p.y,vtx.position.xy);
			
			
				M::Vec::mult(n,np.y,vtx.normal.xy);
				vtx.normal.z = np.x;
				M::Vec::normalize0(vtx.normal);

				vtx.tangent.x = 1;
				vtx.tangent.y = 0;
				vtx.tangent.z = 0;

				vtx.tangent.x = vtx.normal.y;
				vtx.tangent.y = -vtx.normal.x;
				vtx.tangent.z = 0;
				//M::TVec3<>	temp;
				//M::Vec::cross(vtx.normal,vtx.tangent,temp);
				//M::Vec::cross(temp,vtx.normal,vtx.tangent);

				M::Vec::normalize0(vtx.tangent);



				//M::Vec::def(vtx.tcoord,(n0.x_coord*(1.0f-fx) + n1.x_coord*fx)/texture_size_x + 0.5f,vtx.position.z/texture_size_y);
				M::Vec::def(vtx.tcoord,(edge.x_coord[0]*(1.0f-fx) + edge.x_coord[1]*fx)/texture_size_x + 0.5f,vtx.position.z/texture_size_y);
			
				switch (layout)
				{
					case Split:
						splitBend(vtx);
					break;
					case Ramp:
						ramp(vtx);
					break;
				}
				if (z && x)
				{
					ASSERT_EQUAL__(voffset+z*res_x+x,desc.vertices.Count()-1);
					desc.quadIndices << UINT32(voffset+z*res_x+x);
					desc.quadIndices << UINT32(voffset+z*res_x+x-1);
					desc.quadIndices << UINT32(voffset+(z-1)*res_x+x-1);
					desc.quadIndices << UINT32(voffset+(z-1)*res_x+x);
				}
			}
		}
		return res_x > 2 || res_z > 2;
	}


	#if 0
	bool		Graph::defineProfileLessSurface(index_t edge_index,float step,float texture_size,Buffer<TVertex>&vout,Buffer<UINT32>&iout, layout_t layout)	const
	{

	/*	Profile::Node	p0,p1;

		p0.reset();
		p1.reset();
		p0.position.x = -1;
		p1.position.x = +1;
		return defineSurface(p0,p1,edge_index,step,texture_size,vout,iout,layout);*/



		index_t voffset = vout.GetLength();
	
		const Edge	&edge = edges[edge_index];
		const Node	&n0 = nodes[edge.node[0]],
					&n1 = nodes[edge.node[1]];

		float	dist_x = M::Vec::distance(n0.position,n1.position);

		M::TVec2<>	edir;
		M::Vec::sub(n1.position,n0.position,edir);
		M::Vec::normalize0(edir);
	
		float	x_step = step,
				z_step = step;
		if (layout != Graph::Plain && !edge.control_dist[0] && !edge.control_dist[1])
			x_step *= 10;
		if (layout == Graph::Ramp)
			z_step *= 10;	//either loading for split, in which case we need increased z-resolution anyway, or loading plain, in which case this geometry will be used for bending. only ramp has use of this setting
	
		unsigned	res_x = vmax((unsigned)M::Round(dist_x/x_step),2),
					res_z = vmax((unsigned)M::Round(2/z_step),2);
	
		M::TVec2<>		control[2];

	
	
		if (edge.control_dist[0] > 0 || edge.control_dist[1] > 0)
			M::Vec::mad(n0.position,n0.direction,dist_x*NURBS_CONST*edge.control_dist[0],control[0]);
		else
			M::Vec::mad(n0.position,edir,dist_x*NURBS_CONST,control[0]);
		if (edge.control_dist[0] > 0 || edge.control_dist[1] > 0)
			M::Vec::mad(n1.position,n1.direction,-dist_x*NURBS_CONST*edge.control_dist[1],control[1]);
		else
			M::Vec::mad(n1.position,edir,-dist_x*NURBS_CONST,control[1]);
	
	
	
		static Buffer<Node>	profile;
		profile.reset();
		for (unsigned i = 0; i < res_x; i++)
		{
			float fc = (float)i/(res_x-1);
			Node&node = profile.Append();
			M::Vec::resolveBezierCurvePoint(n0.position,control[0],control[1],n1.position,fc,node.position);
			M::Vec::resolveBezierCurveAxis(n0.position,control[0],control[1],n1.position,fc,node.direction);
			if ((!i && !edge.control_dist[0]) || (i+1 == res_x && !edge.control_dist[1]))
				M::Vec::resolveBezierCurveAxis(n0.position,control[0],control[1],n1.position,(fc*0.9+0.5*0.1),node.direction);
			M::Vec::normalize0(node.direction);
		}
	
		for (unsigned z = 0; z < res_z; z++)
		{
			float fz = (float)z/(float)(res_z-1)*2-1;
			//M::TVec2<> np = ;
			//M::Vec::resolveBezierCurvePoint(p0.position,z_control0,z_control1,p1.position,fz,p);
			//M::Vec::resolveBezierCurveAxis(p0.position,z_control0,z_control1,p1.position,fz,a);
			//if (M::Vec::zero(a))
			//	M::Vec::resolveBezierCurveAxis(p0.position,z_control0,z_control1,p1.position,(fz*0.9+0.5*0.1),a);

			//M::Vec::normalize0(a);
			//np = a;
			//swp(np.x,np.y);
			//np.x *= -1;
		
			for (unsigned x = 0; x < res_x; x++)
			{
				float fx = (float)x/(res_x-1);
				const Node&node = profile[x];
				TVertex&vtx = vout.Append();
				vtx.position.z = fz;
			
				M::TVec2<> n = node.direction;
				swp(n.x,n.y);
				n.y *= -1;
			
				if (!x && !edge.control_dist[0])
				{
					M::TVec2<> n = n0.direction;
					swp(n.x,n.y);
					n.y *= -1;
					M::Vec::copy(node.position,vtx.position.xy);
				}
				elif (x+1 == res_x && !edge.control_dist[1])
				{
					M::TVec2<> n = n1.direction;
					swp(n.x,n.y);
					n.y *= -1;
					M::Vec::copy(node.position,vtx.position.xy);
				}
				else
					M::Vec::copy(node.position,vtx.position.xy);
			
			
				M::Vec::copy(n,vtx.normal.xy);
				vtx.normal.z = 0;
			
				M::Vec::normalize0(vtx.normal);
				vtx.tangent.xy = node.direction;
				vtx.tangent.z = 0;
				M::Vec::normalize0(vtx.tangent);

				M::Vec::def(vtx.tcoord,fx*dist_x/texture_size,fz/texture_size);
			
				switch (layout)
				{
					case Split:
						splitBend(vtx);
					break;
					case Ramp:
						ramp(vtx);
					break;
				}
				if (z && x)
				{
					ASSERT_EQUAL__(voffset+z*res_x+x,vout.GetLength()-1);
					iout << UINT32(voffset+z*res_x+x);
					iout << UINT32(voffset+z*res_x+x-1);
					iout << UINT32(voffset+(z-1)*res_x+x-1);
					iout << UINT32(voffset+(z-1)*res_x+x);
				}
			}
		}
		return res_x > 2 || res_z > 2;
	}
	#endif

	/*static*/		float	Graph::splitIntersectionFactor(const SurfaceDescription::TVertex&v0,const SurfaceDescription::TVertex&v1)
	{
		float sub = v1.position.x - v0.position.x;
		using std::fabs;
		if (fabs(sub) <= TypeInfo<float>::error)
			return -1;
		float rs = -v0.position.x / sub;
		if (rs <= TypeInfo<float>::error || rs >= 1.0f- TypeInfo<float>::error)
			return -1;
		return rs;
		//return clamp(rs;

	}

	/*static*/		UINT32	Graph::insertInterpolatedVertex(Container::PreAllocVector<SurfaceDescription::TVertex,0,POD>&vout,UINT32 v0_i,UINT32 v1_i,float factor)
	{
		UINT32 result = (UINT32)vout.Count();
		SurfaceDescription::TVertex&new_vtx = vout.Append();

		const SurfaceDescription::TVertex&v0 = vout[v0_i],
							&v1 = vout[v1_i];
		M::Vec::interpolate(v0.position,v1.position,factor,new_vtx.position);
		M::Vec::interpolate(v0.normal,v1.normal,factor,new_vtx.normal);
		M::Vec::normalize(new_vtx.normal);
		M::Vec::interpolate(v0.tangent,v1.tangent,factor,new_vtx.tangent);
		M::Vec::normalize(new_vtx.tangent);
		M::Vec::interpolate(v0.tcoord,v1.tcoord,factor,new_vtx.tcoord);
		return result;
	}

	namespace MeshGraphDetail
	{



		/*

		static Buffer<UINT32>					visual_iout,
												hull_iout,
												visual_triangle_iout,
												hull_triangle_iout;*/

		//static CGS::TextureA					global_texture;


	}

	/*static*/	void									Graph::splitHull(SurfaceDescription&desc_out)
	{
			static Container::PreAllocVector<UINT32> iout2;
			iout2.reset();
			for (index_t i = 0; i < desc_out.quadIndices.Count(); i+=4)
			{
				const UINT32		v_i[4] = {	desc_out.quadIndices[i],
												desc_out.quadIndices[i+1],
												desc_out.quadIndices[i+2],
												desc_out.quadIndices[i+3]};
				const SurfaceDescription::TVertex v[4] = {	desc_out.vertices [v_i[0]],
															desc_out.vertices [v_i[1]],
															desc_out.vertices [v_i[2]],
															desc_out.vertices [v_i[3]]};
				
				if (v[0].position.x >= -TypeInfo<float>::error && v[1].position.x >= -TypeInfo<float>::error && v[2].position.x >= -TypeInfo<float>::error && v[3].position.x >= -TypeInfo<float>::error)
				{
					iout2.Append(desc_out.quadIndices+i,4);
				}
				elif (v[0].position.x < 0 && v[1].position.x < 0 && v[2].position.x < 0 && v[3].position.x < 0)
				{
					//all invalid
				}
				else
				{
					//needs intersection
					float	fc[4];
					for (BYTE k = 0; k < 4; k++)
						fc[k] = splitIntersectionFactor(v[k],v[(k+1)%4]);
					bool done = false;
					for (BYTE k = 0; k < 2; k++)
						if (fc[k] >= 0 && fc[k+2] >= 0)
						{
							UINT32	new_v0 = insertInterpolatedVertex(desc_out.vertices,v_i[k],v_i[k+1],fc[k]),
									new_v1 = insertInterpolatedVertex(desc_out.vertices,v_i[k+2],v_i[(k+3)%4],fc[k+2]);
							if (v[k+1].position.x >= -TypeInfo<float>::error)
								iout2 << new_v0 << v_i[k+1] << v_i[k+2] << new_v1;
							else
								iout2 << new_v1 << v_i[(k+3)%4] << v_i[k] << new_v0;
							done = true;
							break;
						}
					if (!done)
						for (BYTE k = 0; k < 4; k++)
							if (fc[k] > 0 && fc[(k+1)%4] > 0)
							{
								UINT32	new_v0 = insertInterpolatedVertex(desc_out.vertices,v_i[k],v_i[(k+1)%4],fc[k]),
										new_v1 = insertInterpolatedVertex(desc_out.vertices,v_i[(k+1)%4],v_i[(k+2)%4],fc[(k+1)%4]);
								if (v[(k+1)%4].position.x > 0)
								{
									desc_out.triangleIndices << new_v0 << v_i[(k+1)%4] << new_v1;
								}
								else
								{
									iout2 << new_v0 << new_v1 << v_i[(k+2)%4] << v_i[(k+3)%4];
									desc_out.triangleIndices << new_v0 << v_i[(k+3)%4] << v_i[k];
								}
								done = true;
								break;
							}
				}

			}
			desc_out.quadIndices.swap(iout2);
			//iout.reset();	//temporarily disable quads

			static Container::PreAllocVector<SurfaceDescription::TVertex,0,POD>	vout2;
			vout2.reset();
			for (index_t i = 0; i < desc_out.vertices.Count(); i++)
			{
				if (desc_out.vertices[i].position.x >= -TypeInfo<float>::error)
					vout2 << desc_out.vertices[i];
				else
				{
					index_t degrade_after = vout2.Count();
					Concurrency::parallel_for(index_t(0), desc_out.quadIndices.Count(), [degrade_after,&desc_out](index_t j)
					{
						if (desc_out.quadIndices[j] > degrade_after)
							desc_out.quadIndices[j]--;
					});
					Concurrency::parallel_for(index_t(0), desc_out.triangleIndices.Count(), [degrade_after,&desc_out](index_t j)
					{
						if (desc_out.triangleIndices[j] > degrade_after)
							desc_out.triangleIndices[j]--;
					});

				}
			}

			desc_out.vertices.swap(vout2);
			count_t duplicate_to = desc_out.vertices.Count();
			for (index_t i = 0; i < duplicate_to; i++)
			{
				SurfaceDescription::TVertex&duplicate = desc_out.vertices.Append();
				const SurfaceDescription::TVertex&original = desc_out.vertices[i];

				duplicate.position.x = -original.position.x;
				duplicate.position.yz = original.position.yz;

				duplicate.normal.x = -original.normal.x;
				duplicate.normal.yz = original.normal.yz;

				duplicate.tangent.x = -original.tangent.x;
				duplicate.tangent.yz = original.tangent.yz;
				M::Vec::mult(duplicate.tangent,-1);

				duplicate.tcoord = original.tcoord;
				duplicate.tcoord.x = 1.0f-duplicate.tcoord.x;
			}

			count_t duplicate_quad_to = desc_out.quadIndices.Count();
			for (index_t i = 0; i < duplicate_quad_to; i+=4)
			{
				UINT32*duplicate = desc_out.quadIndices.appendRow(4);
				const UINT32*original = desc_out.quadIndices+i;

				duplicate[0] = static_cast<UINT32>(original[3] + duplicate_to);
				duplicate[1] = static_cast<UINT32>(original[2] + duplicate_to);
				duplicate[2] = static_cast<UINT32>(original[1] + duplicate_to);
				duplicate[3] = static_cast<UINT32>(original[0] + duplicate_to);
			}
			count_t duplicate_triangles_to = desc_out.triangleIndices.Count();
			for (index_t i = 0; i < duplicate_triangles_to; i+=3)
			{
				UINT32*duplicate = desc_out.triangleIndices.appendRow(3);
				const UINT32*original = desc_out.triangleIndices+i;

				duplicate[0] = static_cast<UINT32>(original[2] + duplicate_to);
				duplicate[1] = static_cast<UINT32>(original[1] + duplicate_to);
				duplicate[2] = static_cast<UINT32>(original[0] + duplicate_to);
			}
	}



	void		GraphTextureResource::Insert(const PathString&filename, name64_t as_name, bool bump_map,float bump_strength /*=0.01f*/)
	{
		if (container.IsSet(as_name))
			throw Except::Program::UniquenessViolation("64bit name '"+name2str(as_name)+"' is already defined");

		Image	image;
		Magic::LoadFromFile(image,filename);

		CGS::TextureA*new_texture = container.define(as_name);
		new_texture->name = as_name;
		new_texture->face_field.SetSize(1);

		if (!bump_map)
			TextureCompression::compress(image, new_texture->face_field.First(),TextureCompression::NoCompression);
		else
		{
			Image	normal_map;
			image.ToNormalMap(normal_map,1.0f,1.0f,bump_strength,true,0);
			TextureCompression::compress(normal_map, new_texture->face_field.First(),TextureCompression::NoCompression);
			//png.saveToFileQ(normal_map,"test.png");
		}
		new_texture->updateHash();
	}


	GraphTextureResource		GraphMesh::texture_resource;


	void		Graph::updateDirectionsAndXCoords()
	{
		for (index_t i = 0; i < nodes.Count(); i++)
			nodes[i].processed = false;
		bool done = false;
		count_t counter = 0;
		while (!done)
		{
			done = true;
			for (index_t i = 0; i < edges.Count(); i++)
			{
				Edge&e = edges[i];
				Node&n0 = nodes[e.node[0]],
					&n1 = nodes[e.node[1]];
				bool base_edge = n0.position.x >= 0 && n1.position.x < 0;

				if (n0.processed && n1.processed)
				{
					e.x_coord[0] = n0.x_coord;
					e.x_coord[1] = n1.x_coord;
					if (!base_edge && (M::Sign(n0.x_coord) != M::Sign(n1.x_coord)))//must not happen. ever.
					{
						e.x_coord[1] = n0.x_coord - M::Vec::distance(n0.position,n1.position);
					}
					continue;
				}
				done = false;
				if (n0.processed)
				{
					e.x_coord[0] = n0.x_coord;
					e.x_coord[1] = n1.x_coord = n0.x_coord - M::Vec::distance(n0.position,n1.position);
					n1.processed = true;
					continue;
				}
				if (n1.processed)
				{
					e.x_coord[1] = n1.x_coord;
					e.x_coord[0] = n0.x_coord = n1.x_coord + M::Vec::distance(n0.position,n1.position);
					n0.processed = true;
					continue;
				}

				if (base_edge)
				{
					float fx = n1.position.x / (n1.position.x - n0.position.x);
					float len = M::Vec::distance(n0.position,n1.position);
					n0.x_coord = (1.0f-fx)*len;
					n1.x_coord = -fx * len;
					e.x_coord[0] = n0.x_coord;
					e.x_coord[1] = n1.x_coord;
					n0.processed = true;
					n1.processed = true;
				}
			}
			if (counter++ >= edges.Count() * 2)
				break;
		}
		for (index_t i = 0; i < nodes.Count(); i++)
		{
			M::TVec2<>	prev,
					next;
			Node&node = nodes[i];
			if (node.in_edge < edges.Count())
			{
				BYTE index = edges[node.in_edge].GetIndexOf(i);
				if (edges[node.in_edge].control_dist[index])
					continue;
				prev = nodes[edges[node.in_edge].node[!index]].position;
			}
			else
				prev = node.position;
			if (node.out_edge<edges.Count())
			{
				BYTE index = edges[node.out_edge].GetIndexOf(i);
				if (edges[node.out_edge].control_dist[index])
					continue;
				next = nodes[edges[node.out_edge].node[!index]].position;
			}
			else
				next = node.position;
			M::Vec::sub(next,prev,node.direction);
			M::Vec::normalize0(node.direction);
		}
	}



	template <typename Hull>
	static void	convertDescriptionToHull(const SurfaceDescription&desc, Hull&hull)
	{
		hull.clear();
		hull.quad_field.SetSize(desc.quadIndices.Count()/4);
		hull.triangle_field.SetSize(desc.triangleIndices.Count()/3);
	
		static VertexTable		vertex_table;
		static Ctr::Array<index_t>	vertex_map;

		vertex_table.setTolerance(1e-8f);
		//vertex_table.clear();	done by setTolerance()
		vertex_map.SetSize(desc.vertices.Count());

		for (UINT32 i = 0; i < desc.vertices.Count(); i++)
			vertex_map[i] = vertex_table.map(desc.vertices[i].position);

		hull.vertex_field.SetSize(vertex_table.vertices());

		for (index_t i = 0; i < vertex_table.vertices(); i++)
			M::Vec::copy(vertex_table.vertex(i),hull.vertex_field[i].position);

		const UINT32*iat = desc.triangleIndices.pointer();
		for (UINT32 i = 0; i < desc.triangleIndices.Count()/3; i++)
		{
			hull.triangle_field[i].v0 = hull.vertex_field+vertex_map[*iat++];
			hull.triangle_field[i].v1 = hull.vertex_field+vertex_map[*iat++];
			hull.triangle_field[i].v2 = hull.vertex_field+vertex_map[*iat++];
		}
		iat = desc.quadIndices.pointer();
		for (UINT32 i = 0; i < desc.quadIndices.Count()/4; i++)
		{
			hull.quad_field[i].v0 = hull.vertex_field+vertex_map[*iat++];
			hull.quad_field[i].v1 = hull.vertex_field+vertex_map[*iat++];
			hull.quad_field[i].v2 = hull.vertex_field+vertex_map[*iat++];
			hull.quad_field[i].v3 = hull.vertex_field+vertex_map[*iat++];
		}
		//ASSERT_CONCLUSION(robj.ipool.idata,iat);
	
		DBG_ASSERT1__(hull.valid(),hull.errorStr());
	}


	count_t		GraphMesh::createGeometry(CGS::Geometry<>&target, float step, float texture_size_x, float texture_size_y, Graph::layout_t layout, name64_t texture, name64_t normal_texture, name64_t glow_texture, bool export_visual_sub_hulls)	const
	{
		//target.clear();
		target.object_field.SetSize(1);
		target.object_field[0].name = String(str2name("object"));
	
		target.material_field.SetSize(1);
		//target.material_field[0].data.object_field.Resize(detail_levels);
		M::Vec::set(target.material_field[0].info.specular,0.7);
		//target.material_field[0].info.shininess = 0;

		target.texture_resource = &texture_resource;
	

		//if (texture != 0)
		{
			target.material_field[0].info.layer_field.SetSize(1 + (normal_texture!=0) + (glow_texture!=0));
			target.material_field[0].info.layer_field[0].combiner = GL_MODULATE;
			target.material_field[0].info.layer_field[0].cube_map = false;
			target.material_field[0].info.layer_field[0].clamp_x = false;
			target.material_field[0].info.layer_field[0].clamp_y = false;
			target.material_field[0].info.layer_field[0].source = texture_resource.retrieve(texture);
			target.material_field[0].info.layer_field[0].source_name = texture;

			index_t offset = 1;
			if (normal_texture != 0)
			{
				target.material_field[0].info.layer_field[offset].combiner = GL_MODULATE;
				target.material_field[0].info.layer_field[offset].cube_map = false;
				target.material_field[0].info.layer_field[offset].clamp_x = false;
				target.material_field[0].info.layer_field[offset].clamp_y = false;
				target.material_field[0].info.layer_field[offset].source = texture_resource.retrieve(normal_texture);
				target.material_field[0].info.layer_field[offset].source_name = normal_texture;
				offset++;
			}
			if (glow_texture != 0)
			{
				target.material_field[0].info.layer_field[offset].combiner = GL_DECAL;
				target.material_field[0].info.layer_field[offset].cube_map = false;
				target.material_field[0].info.layer_field[offset].clamp_x = false;
				target.material_field[0].info.layer_field[offset].clamp_y = false;
				target.material_field[0].info.layer_field[offset].source = texture_resource.retrieve(glow_texture);
				target.material_field[0].info.layer_field[offset].source_name = glow_texture;
				offset++;
			}
		}
	
	
	
	
		static Container::PreAllocVector<Mesh<TDef<float> >,0>	caps;
		caps.reset();
	
		//if (has_caps)
			//generateCaps(xy_profile,step,caps);

		//updateDirections()
	

		static Container::PreAllocVector<SurfaceDescription,0,Swap>	visual_hulls;
		static SurfaceDescription					physical_hull;
		visual_hulls.reset();
		physical_hull.Clear();

	
		//static Buffer<Graph::TVertex>			visual_vout,hull_vout;
		static Container::PreAllocVector<CGS::RenderObjectA<>,0>	robjs;
		robjs.reset();
	
		float this_step = step;
	
		float z_factor = 1.0f;
		if (layout == Graph::Split)
			z_factor *= 0.25f;
		bool can_reduce = true;
		while (can_reduce)
		{
			CGS::RenderObjectA<>&robj = robjs.Append();
			robj.target = target.object_field.pointer();
			robj.tname = target.object_field.pointer()->name;

			SurfaceDescription&visual_hull = visual_hulls.Append();
			visual_hull.Clear();
		
			can_reduce = false;
			for (index_t l = 0; l+1 < visual_profile.nodes.Count(); l++)
			{
				const Profile::Node		&n0 = visual_profile.nodes[l],
										&n1 = visual_profile.nodes[l+1];
			
				for (index_t j = 0; j < visual_graph.edges.Count(); j++)
				{
					can_reduce |= visual_graph.defineSurface(n0,n1,j,this_step,this_step*z_factor,texture_size_x,texture_size_y,visual_hull,layout,true);
				}
			}


		//	cout << "got "<<visual_iout.Count()<<" visual triangle indices and "<<hull_iout.Count()<<" hull triangle indices. that's a factor of "<<(float)visual_iout.Count()/hull_iout.Count()<<endl;

			if (layout == Graph::Split)
			{
				Graph::splitHull(visual_hull);
				//Graph::splitHull(hull_iout,hull_vout,hull_triangle_iout);
			}

			robj.vpool.SetSize(visual_hull.vertices.Count(),1,CGS::HasNormalFlag |CGS::HasTangentFlag);
			//ASSERT_EQUAL__(robj.vpool.vsize(),11 + 2*...yadda, you get idea);
			float*vtx = robj.vpool.vdata.pointer();
			for (index_t i = 0; i < visual_hull.vertices.GetLength(); i++)
			{
				M::Vec::ref3(vtx) = visual_hull.vertices[i].position;
				M::Vec::ref3(vtx+3) = visual_hull.vertices[i].normal;
				M::Vec::ref3(vtx+6) = visual_hull.vertices[i].tangent;
				M::Vec::ref2(vtx+9) = visual_hull.vertices[i].tcoord;
				//if (normal_texture != 0)
					//M::Vec::ref2(vtx+11) = visual_hull.vertices[i].tcoord;	//normal map texcoords
				//_c2(vout[i].tcoord,vtx+6);
				vtx += 11;
				//if (normal_texture != 0)
					//vtx += 2;
				//vtx += 8;
			}
			ASSERT__(!(visual_hull.quadIndices.Count()%4));
			ASSERT__(!(visual_hull.triangleIndices.Count()%3));
			robj.ipool.SetSize(UINT32(visual_hull.triangleIndices.Count()/3),UINT32(visual_hull.quadIndices.Count()/4));
			memcpy(robj.ipool.idata.pointer(),visual_hull.triangleIndices.pointer(),visual_hull.triangleIndices.Count()*sizeof(UINT32));
			memcpy(robj.ipool.idata.pointer()+visual_hull.triangleIndices.Count(),visual_hull.quadIndices.pointer(),visual_hull.quadIndices.Count()*sizeof(UINT32));
			//robj.ipool.idata.copyFrom
				//copyFrom(iout.pointer(),iout.GetLength());
			this_step*=2;
		}


		for (index_t l = 0; l+1 < physical_profile.nodes.Count(); l++)
		{
			const Profile::Node		&n0 = physical_profile.nodes[l],
									&n1 = physical_profile.nodes[l+1];
			
			for (index_t j = 0; j < physical_graph.edges.Count(); j++)
			{
				physical_graph.defineSurface(n0,n1,j,step*0.5f,step*0.5f*z_factor, texture_size_x, texture_size_y,physical_hull,layout, false);
			}
		}

		std::cout << "Physical hull complexity:"<<std::endl;
		std::cout << " Vertices: "<<physical_hull.vertices.Count()<<std::endl;
		std::cout << " Triangles: "<<physical_hull.triangleIndices.Count()<<std::endl;
		std::cout << " Quads: "<<physical_hull.quadIndices.Count()<<std::endl;


		if (layout == Graph::Split)
			Graph::splitHull(physical_hull);


		target.material_field[0].data.object_field.SetSize(robjs.Count());
		for (index_t i = 0; i < robjs.Count(); i++)
		{
			target.material_field[0].data.object_field[i].adoptData(robjs[i]);
			target.material_field[0].data.object_field[i].detail = unsigned(i);
		}
		target.material_field[0].data.coord_layers = 1;//+ (normal_texture != 0);
		//target.material_field[0].data.dimensions = 3;
	
		//const CGS::RenderObjectA<>&robj = target.material_field[0].data.object_field.First();
		CGS::SubGeometryA<>&obj = target.object_field.First();
		obj.vs_hull_field.SetSize(visual_hulls.Count());
		for (index_t i = 0; i < obj.vs_hull_field.GetLength(); i++)
		{
			Mesh<CGS::SubGeometryA<>::VsDef>&vs_hull = obj.vs_hull_field[i];
			const SurfaceDescription&hull = visual_hulls[i];
			if (export_visual_sub_hulls || !i)
				convertDescriptionToHull(hull,vs_hull);
			else
				vs_hull.clear();
		}

		convertDescriptionToHull(physical_hull,obj.phHull);
	
		obj.meta.shortest_edge_length = step * z_factor;
		//ShowMessage("generated "+String(robjs.Count())+" sub defail level(s)");
	
		M::Mat::Eye(target.object_field[0].meta.system);
		M::Vec::clear(target.object_field[0].meta.center);
		target.object_field[0].meta.radius = 1;
		target.object_field[0].meta.volume = 1;
		target.object_field[0].meta.density = 1;
		M::Mat::Eye(target.object_field[0].path);
		target.object_field[0].system_link = &target.object_field[0].path;

		if (layout == Graph::Split)
		{
			target.connector_field.SetSize(2);
			SurfaceDescription::TVertex vtx;
			M::Vec::def(vtx.position,0,0,-1);
			M::Vec::def(vtx.normal,0,1,0);
			M::Vec::def(vtx.tangent,1,0,0);
			M::Vec::def(vtx.tcoord,0,1);
			Graph::splitBend(vtx);
			CGS::TConnector<>	&c0 = target.connector_field[0],
								&c1 = target.connector_field[1];
			M::Vec::sub(vtx.position,vtx.tangent,c0.p1);
			M::Vec::add(vtx.position,vtx.tangent,c0.p0);
			M::Vec::cross(vtx.normal,vtx.tangent,c0.direction);

			c1.direction.x = -c0.direction.x;
			c1.direction.yz = c0.direction.yz;

			c1.p0.x = -c0.p0.x;
			c1.p0.yz = c0.p0.yz;
			c1.p1.x = -c0.p1.x;
			c1.p1.yz = c0.p1.yz;

			swp(c1.p0,c1.p1);
		}
		else
			target.connector_field.free();
	
		return robjs.Count();
		//target.updateSystems();
	}

	void	Graph::eraseEdge(index_t index)
	{
		for (index_t i = 0; i < nodes.Count(); i++)
		{
			if (nodes[i].out_edge!=-1 && nodes[i].out_edge > index)
				nodes[i].out_edge--;
			if (nodes[i].in_edge!=-1 && nodes[i].in_edge > index)
				nodes[i].in_edge--;
		}
		
		nodes[edges[index].node[0]].out_edge = -1;
		nodes[edges[index].node[1]].in_edge = -1;
		edges.Erase(index);
	}

	void	Graph::verifyIntegrity(const TCodeLocation&location)	const
	{
		for (index_t i = 0; i < nodes.Count(); i++)
		{
			const Node&node = nodes[i];
			if (node.out_edge != -1)
			{
				if (node.out_edge >= edges.Count())
					Except::TriggerFatal(location,"invalid out_edge index ("+String(node.out_edge)+"/"+String(edges.Count())+")");
			
				const Edge&edge = edges[node.out_edge];
				if (edge.node[1] == i)
					Except::TriggerFatal(location,"edge node confusion");
				if (edge.node[0] != i)
					Except::TriggerFatal(location,"edge "+String(node.out_edge)+" doesn't know node "+String(i));
			}
			if (node.in_edge != -1)
			{
				if (node.in_edge >= edges.Count())
					Except::TriggerFatal(location,"invalid in_edge index ("+String(node.in_edge)+"/"+String(edges.Count()));
			
				const Edge&edge = edges[node.in_edge];
				if (edge.node[0] == i)
					Except::TriggerFatal(location,"edge node confusion");
				if (edge.node[1] != i)
					Except::TriggerFatal(location,"edge "+String(node.in_edge)+" doesn't know node "+String(i));
			}
		}
	
		for (index_t i = 0; i < edges.Count(); i++)
		{
			const Edge&edge = edges[i];
		
			if (edge.node[0] >= nodes.Count())
				Except::TriggerFatal(location,"node.x ("+String(edge.node[0])+"), referenced by edge "+String(i)+" does not exist");
			if (edge.node[1] >= nodes.Count())
				Except::TriggerFatal(location,"node.y ("+String(edge.node[1])+"), referenced by edge "+String(i)+" does not exist");
		
			if (nodes[edge.node[0]].in_edge == i)
				Except::TriggerFatal(location,"edge node confusion at edge "+String(i));
			if (nodes[edge.node[1]].out_edge == i)
				Except::TriggerFatal(location,"edge node confusion at edge "+String(i));
		
			if (nodes[edge.node[0]].out_edge != i)
				Except::TriggerFatal(location,"node "+String(edge.node[0])+" doesn't know edge "+String(i));
			if (nodes[edge.node[1]].in_edge != i)
				Except::TriggerFatal(location,"node "+String(edge.node[1])+" doesn't know edge "+String(i));
		}
	}


	void		Graph::readFrom(const XML::Node*xnodes, const XML::Node*xedges)
	{
		nodes.reset();
		edges.reset();
		if (!xnodes || !xedges)
			return;
		for (index_t i = 0; i < xnodes->children.Count(); i++)
		{
			const XML::Node	&xnode = xnodes->children[i];
			Graph::Node&node = nodes.Append();
			node.reset();
		
			parse(xnode,"position",node.position);
			parse(xnode,"direction",node.direction);
		}
	
	
		for (index_t i = 0; i < xedges->children.Count(); i++)
		{
			const XML::Node	&xedge = xedges->children[i];
			Graph::Edge&edge = edges.Append();
			edge.reset();
		
			parse(xedge,"nodes",M::Vec::ref2(edge.node));
			parse(xedge,"control",M::Vec::ref2(edge.control_dist));
		
			if (nodes[edge.node[0]].out_edge != -1)
			{
				edges.eraseLast();
				continue;
			}
			if (nodes[edge.node[1]].in_edge != -1)
			{
				edges.eraseLast();
				continue;
			}
			nodes[edge.node[1]].in_edge = edges.Count()-1;
			nodes[edge.node[0]].out_edge = edges.Count()-1;
		}


	}


	void		Profile::readFrom(const XML::Node*xprofile)
	{
		nodes.reset();
		if (!xprofile)
			return;
		for (index_t i = 0; i < xprofile->children.Count(); i++)
		{
			const XML::Node	&xnode = xprofile->children[i];
			Profile::Node&node = nodes.Append();
			node.reset();
		
			parse(xnode,"position",node.position);
			parse(xnode,"direction",node.direction);
			parse(xnode,"control",M::Vec::ref2(node.control_dist));
		}
	}


	void		GraphMesh::LoadFromFile(const PathString&filename,bool compact)
	{
		XML::Container	xml;
		xml.LoadFromFile(filename);
	
		XML::Node	*xmesh = xml.Find("mesh"),
					*xvisual_profile = xmesh->Find("profile"),
					*xphysical_profile = xmesh->Find("physical_profile"),
					*xvisual_graph = xmesh->Find("graph"),
					*xvisual_nodes = xvisual_graph?xvisual_graph->Find("nodes"):xmesh->Find("nodes"),	//format variances
					*xvisual_edges = xvisual_graph?xvisual_graph->Find("edges"):xmesh->Find("edges"),
					*xphysical_nodes = xmesh->Find("physical_graph/nodes"),
					*xphysical_edges = xmesh->Find("physical_graph/edges");
		if (!xvisual_nodes)
			throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("mesh/graph/nodes missing in XML document"));

		if (!xvisual_edges)
			throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("mesh/graph/edges missing in XML document"));

		if (!xvisual_profile)
			throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("mesh/profile missing in XML document"));
	
		visual_graph.readFrom(xvisual_nodes,xvisual_edges);
		physical_graph.readFrom(xphysical_nodes,xphysical_edges);
		visual_profile.readFrom(xvisual_profile);
		physical_profile.readFrom(xphysical_profile);
	
	
		if (compact)
		{
			visual_graph.nodes.compact();
			visual_graph.edges.compact();
			visual_profile.nodes.compact();
			physical_graph.nodes.compact();
			physical_graph.edges.compact();
			physical_profile.nodes.compact();
		}
		visual_graph.updateDirectionsAndXCoords();

		if (physical_graph.nodes.IsNotEmpty())
			physical_graph.updateDirectionsAndXCoords();
		else
			physical_graph = visual_graph;

		if (physical_profile.nodes.IsEmpty())
		{
			physical_profile.nodes.Append().reset().moveTo(-1,0);
			physical_profile.nodes.Append().reset().moveTo(1,0);
		}
	}

	static void		writeNode(XML::Node&xprofile, const Profile::Node&node)
	{
		XML::Node	&xnode = xprofile.Create("node");
		xnode.Set("position",String(node.position.x)+", "+String(node.position.y));
		xnode.Set("direction",String(node.direction.x)+", "+String(node.direction.y));
		xnode.Set("control",String(node.control_dist[0])+", "+String(node.control_dist[1]));
	}


	void		Graph::WriteTo(XML::Node&xmesh)	const
	{
		{
			XML::Node	&xnodes = xmesh.Create("nodes");
			for (index_t i = 0; i < nodes.Count(); i++)
			{
				const Graph::Node&node = nodes[i];
				XML::Node	&xnode = xnodes.Create("node");
				xnode.Set("position",String(node.position.x)+", "+String(node.position.y));
				xnode.Set("direction",String(node.direction.x)+", "+String(node.direction.y));
			}
		}
		{
			XML::Node	&xedges = xmesh.Create("edges");
			for (index_t i = 0; i < edges.Count(); i++)
			{
				const Graph::Edge&edge = edges[i];
				XML::Node	&xedge = xedges.Create("edge");
				xedge.Set("nodes",String(edge.node[0])+", "+String(edge.node[1]));
				xedge.Set("control",String(edge.control_dist[0])+", "+String(edge.control_dist[1]));
			}
		}
	}

	void		GraphMesh::SaveToFile(const PathString&filename)	const
	{
		XML::Container	xml;
	
		{
			XML::Node	&xmesh = xml.Create("mesh");
			{
				XML::Node	&xprofile = xmesh.Create("profile");

				for (index_t i = 0; i < visual_profile.nodes.Count(); i++)
					writeNode(xprofile,visual_profile.nodes[i]);
			}
			{
				XML::Node	&xprofile = xmesh.Create("physical_profile");
				for (index_t i = 0; i < physical_profile.nodes.Count(); i++)
					writeNode(xprofile,physical_profile.nodes[i]);
			}
			visual_graph.WriteTo(xmesh.Create("graph"));
			physical_graph.WriteTo(xmesh.Create("physical_graph"));
	
			xmesh.Set("caps",has_caps?"true":"false");
		}
		xml.SaveToFile(filename);
	}

	/*static*/	void				GraphMesh::buildStub(Graph::layout_t layout, float step, SurfaceDescription&desc)
	{
		desc.Clear();
		if (layout == Graph::Plain)
			return;
	}

	float				SurfaceDescription::InterpolatedSlice::GetTexExtend()	const
	{
		float len = 0;
		float3	last_p;
		{
			const float	cs = cos(angle0*M_PI),
						sn = sin(angle0*M_PI);
			last_p = float3::reinterpret(position) + float3::reinterpret(up) * -cs * scale.y + right*sn*scale.x;
		}
		for (index_t i = 1; i <= 10; i++)
		{
			float angle = float(i)/10.0f * (angle1 - angle0) + angle0;
			const float	cs = cos(angle*M_PI),
						sn = sin(angle*M_PI);
			float3	p = float3::reinterpret(position) + float3::reinterpret(up) * -cs * scale.y + right*sn*scale.x;
			len += M::Vec::distance(p,last_p);
			last_p = p;
		}
		return len / (texcoord1 - texcoord0);
	}

	count_t				SurfaceDescription::InterpolatedSlice::CalculateSteps(float tolerance)	const
	{
		using std::max;
		using std::fabs;
		return max((count_t)ceil((angle1 - angle0)*M_PI*(fabs(scale.x)+fabs(scale.y))/tolerance/8),(count_t)2);
	}

	count_t				SurfaceDescription::InterpolatedSlice::CalculateSteps(float tolerance0, float tolerance1)	const
	{
		float tolerance = tolerance0 + (tolerance1-tolerance0)*t;
		return CalculateSteps(tolerance);
	}

	bool				SurfaceDescription::InterpolatedSlice::IsClosedLoop()		const
	{
		return (angle1 - angle0) >= 2.f - TypeInfo<float>::error;
	}


	float				SurfaceDescription::InterpolatedSlice::EstimateLength()	const
	{
		float rs = 0;

		const float range = angle1 - angle0;

		using std::fabs;
		const M::TVec2<>	factor = {scale.y != 0? fabs(scale.x/scale.y) : 1 , scale.x != 0 ? scale.y / scale.x : 1};

		M::TVec2<>	p0,p1;

		const float	cs = cos(angle0*M_PI),
					sn = sin(angle0*M_PI);
		//pt = float3::reinterpret(position) + float3::reinterpret(up) * (1.f -cs) * scale.y + right*sn*scale.x;
		p0.x = sn*scale.x;
		p0.y = (1.f -cs) * scale.y;


		for (index_t i = 1; i <= 10; i++)
		{
			float x = float(i)/10.f;

			const float	angle = (angle0 + range*x),
						cs = cos(angle*M_PI),
						sn = sin(angle*M_PI);
			//pt = float3::reinterpret(position) + float3::reinterpret(up) * (1.f -cs) * scale.y + right*sn*scale.x;
			p1.x = sn*scale.x;
			p1.y = (1.f -cs) * scale.y;

			rs += M::Vec::distance(p0,p1);

			p0 = p1;
		}

		return rs;
	}



	void			SurfaceDescription::InterpolatedSlice::MakeVertex(float x, TVertex&vtx)	const
	{
		MakePoint(x,vtx.position,vtx.normal,vtx.tangent);
		vtx.tcoord.x = x * (texcoord1 - texcoord0) + texcoord0;
		vtx.tcoord.y = texcoordY;
		vtx.tx = x;
	}

	void			SurfaceDescription::InterpolatedSlice::MakePoint(float x, M::TVec3<>&pt)	const
	{
		float angle = x * (angle1 - angle0) + angle0;
	
		using std::fabs;
		M::TVec2<>	factor = {scale.y != 0? fabs(scale.x/scale.y) : 1 , scale.x != 0 ? scale.y / scale.x : 1};
		const float	cs = cos(angle*M_PI),
					sn = sin(angle*M_PI);
		if (buildLocal)
		{
			M::Vec::def(pt, sn*scale.x,0,(1.f -cs) * scale.y);
		}
		else
		{
			//pt = float3::reinterpret(position) + float3::reinterpret(up) * (1.f -cs) * scale.y + right*sn*scale.x;
			M::Vec::mad(position,up,(1.f -cs) * scale.y,pt);
			M::Vec::mad(pt,right,sn*scale.x);
		}
	}

	void			SurfaceDescription::InterpolatedSlice::MakePoint(float x, M::TVec3<>&pt, M::TVec3<>&normal)	const
	{
		float angle = x * (angle1 - angle0) + angle0;
	
		using std::fabs;
		M::TVec2<>	factor = {scale.y != 0? fabs(scale.x/scale.y) : 1 , scale.x != 0 ? scale.y / scale.x : 1};
		const float	cs = cos(angle*M_PI),
					sn = sin(angle*M_PI);
		if (buildLocal)
		{
			M::Vec::def(pt, sn*scale.x,0,(1.f -cs) * scale.y);
			M::Vec::def(normal, -sn*factor.y, 0, cs*factor.x);
		}
		else
		{
			//pt = float3::reinterpret(position) + float3::reinterpret(up) * (1.f -cs) * scale.y + right*sn*scale.x;
			M::Vec::mad(position,up,(1.f -cs) * scale.y,pt);
			M::Vec::mad(pt,right,sn*scale.x);

			//normal = (float3::reinterpret(up) * cs * factor.x + right*-sn*factor.y).normalized();
			M::Vec::mult(up,cs*factor.x,normal);
			M::Vec::mad(normal,right,-sn*factor.y);
		}
		M::Vec::normalize(normal);

	}

	void			SurfaceDescription::InterpolatedSlice::MakePoint(float x, M::TVec3<>&pt, M::TVec3<>&normal, M::TVec3<>&tangent)	const
	{
		float angle = x * (angle1 - angle0) + angle0;
	
		using std::fabs;
		M::TVec2<>	factor = {scale.y != 0? fabs(scale.x/scale.y) : 1 , scale.x != 0 ? scale.y / scale.x : 1};
		const float	cs = cos(angle*M_PI),
					sn = sin(angle*M_PI);

		if (buildLocal)
		{
			M::Vec::def(pt, sn*scale.x,0,(1.f -cs) * scale.y);
			M::Vec::def(normal, -sn*factor.y, 0, cs*factor.x);
			M::Vec::def(tangent, cs*scale.x, 0, sn*scale.y);
		}
		else
		{
			//pt = float3::reinterpret(position) + float3::reinterpret(up) * (1.f -cs) * scale.y + right*sn*scale.x;
			M::Vec::mad(position,up,(1.f -cs) * scale.y,pt);
			M::Vec::mad(pt,right,sn*scale.x);

			//normal = (float3::reinterpret(up) * cs * factor.x + right*-sn*factor.y).normalized();
			M::Vec::mult(up,cs*factor.x,normal);
			M::Vec::mad(normal,right,-sn*factor.y);

			//tangent = (float3::reinterpret(up) * sn * scale.y + right*cs*scale.x).normalized();
			M::Vec::mult(up,sn*scale.y,tangent);
			M::Vec::mad(tangent,right,cs*scale.x);
		}
		M::Vec::normalize(normal);
		M::Vec::normalize(tangent);

	}

	/*static*/	void		SurfaceDescription::BuildArcFromSlice(OutVertexContainer&arc_out, InterpolatedSlice&slice, float tolerance, bool flipped)
	{
		if (flipped)
		{
			slice.angle0 *= -1.f;
			slice.angle1 *= -1.f;
			swp(slice.angle0,slice.angle1);
		}
		const count_t steps = slice.CalculateSteps(tolerance);
		//arc_out.SetSize(steps);
		arc_out.reset();
		for (index_t i = 0; i < steps; i++)
		{
			TVertex&vtx = arc_out.Append();
			float fi = float(i) / float(steps-1);
			slice.MakeVertex(fi,vtx);
		}
	}

	/*static*/	void		SurfaceDescription::BuildArc(OutVertexContainer&arc_out, const TControl&control, float tolerance, bool flipped, bool global)
	{
		InterpolatedSlice	slice(control,0);
		slice.buildLocal = !global;
		BuildArcFromSlice(arc_out,slice,tolerance,flipped);
	}




	void				SurfaceDescription::BuildSegment(const SurfaceDescription::TConnector&begin_, const SurfaceDescription::TConnector&end_, const SurfaceDescription::TControl control_points[2], float tolerance0, float tolerance1)
	{
		this->Clear();

		Container::PreAllocVector<InterpolatedSlice,256,POD>	steps;
		InterpolatedSlice	begin(begin_.state,0),
							end(end_.state,1);
		steps << begin;
		using std::max;
		Subdivide(begin_,end_,control_points,M::sqr(max(tolerance0,tolerance1)),begin,end,0,1,steps,1);
		steps << end;

		float length = 0;
		for (index_t i = 1; i < steps.Count(); i++)
		{
			float delta = M::Vec::distance(steps[i-1].position,steps[i].position);
			float tlen = 1.0f/(steps[i-1].GetTexExtend() * 0.5f + steps[i].GetTexExtend()*0.5f);
			length += tlen * delta;
		}
		textureRepetitions = M::Round(length); 
		float factor = float(textureRepetitions) / length;
		length = 0;
		steps.First().texcoordY = 0;
		for (index_t i = 1; i < steps.Count(); i++)
		{
			float delta = M::Vec::distance(steps[i-1].position,steps[i].position);
			float tlen = 1.0f/(steps[i-1].GetTexExtend() * 0.5f + steps[i].GetTexExtend()*0.5f);
			length += tlen * delta;
			steps[i].texcoordY = length * factor;
		}


		const bool	closed0 = begin.angle1 - begin.angle0 >= 2.f - TypeInfo<float>::error,
					closed1 = end.angle1 - end.angle0 >= 2.f - TypeInfo<float>::error;
		Edge*edges = NULL;
		this->edges.appendRow(2);
		if (!closed0 || !closed1)
		{
			edges = this->edges.appendRow(2);
			edges[0].direction = Edge::Left;
			edges[1].direction = Edge::Right;
		}
		Edge*endEdges = this->edges.pointer();
		{
			endEdges[0].direction = Edge::Node0;
			endEdges[1].direction = Edge::Node1;
		}

		{
			const InterpolatedSlice	&next = steps.First();
			const count_t	next_resolution = next.CalculateSteps(tolerance0,tolerance1)+1;
			for (index_t i = 0; i < next_resolution; i++)
			{
				endEdges[0] << (UINT32)(next_resolution - i -1);
				TVertex&vtx = this->vertices.Append();
				float fi = float(i) / float(next_resolution-1);
				next.MakeVertex(fi,vtx);
			}

			if (edges)
			{
				edges[0] << 0;
				edges[1] << (UINT32)(next_resolution-1);
			}
		}

		index_t	vtx_offset = 0;
		for (index_t i = 0; i+1 < steps.Count(); i++)
		{
			const InterpolatedSlice	&prev = steps[i],
									&next = steps[i+1];
			const count_t	prev_resolution = prev.CalculateSteps(tolerance0,tolerance1)+1,
							next_resolution = next.CalculateSteps(tolerance0,tolerance1)+1,
							core_resolution = std::min(prev_resolution,next_resolution);

			float3	right;
			M::Vec::cross(next.direction,next.up,right);
			for (index_t j = 0; j < next_resolution; j++)
			{
				TVertex&vtx = this->vertices.Append();
				float fi = float(j) / float(next_resolution-1);
				next.MakeVertex(fi,vtx);
				if (i+2 == steps.Count())
					endEdges[1] << (UINT32)(vtx_offset+prev_resolution+j);
			}

			for (index_t i = 0; i+1 < core_resolution; i++)
			{
				this->quadIndices	<< (UINT32)(vtx_offset+i) << (UINT32)(vtx_offset+i+1)
									<< (UINT32)(vtx_offset+prev_resolution+i+1) << (UINT32)(vtx_offset+prev_resolution+i);
			}

			for (index_t i = prev_resolution-1; i+1 < next_resolution; i++)
			{
				this->triangleIndices	<< (UINT32)(vtx_offset+prev_resolution-1)
										<< (UINT32)(vtx_offset+prev_resolution+i+1)<< (UINT32)(vtx_offset+prev_resolution+i);
			}

			for (index_t i = next_resolution-1; i+1 < prev_resolution; i++)
			{
				this->triangleIndices	<< (UINT32)(vtx_offset+i) << (UINT32)(vtx_offset+i+1)
										<< (UINT32)(vtx_offset+prev_resolution+next_resolution-1);
			}

			if (edges)
			{
				edges[0] << (UINT32)(vtx_offset+prev_resolution);
				edges[1] << (UINT32)(vtx_offset+prev_resolution+next_resolution-1);
			}

			vtx_offset += prev_resolution;
		}

		if (edges)
			edges[0].Revert();

	}

	/*static*/	void				SurfaceDescription::Subdivide(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end,const SurfaceDescription::TControl control_points[2], float tolerance, const InterpolatedSlice&ref0, const InterpolatedSlice&ref1, float t0, float t1, Container::PreAllocVector<InterpolatedSlice,256,POD>&outIntersections, int force)
	{
		bool subdiv;

		InterpolatedSlice	interpolated;
		float t = t0*0.5+t1*0.5;
		Interpolate(begin,end,control_points,t,interpolated);

		if (force > 0)
			subdiv = true;
		elif (force <= -8)
			subdiv = false;
		else
		{
			InterpolatedSlice	reference;
			M::Vec::center(ref0.position,ref1.position,reference.position);

			subdiv = M::Vec::quadraticDistance(reference.position,interpolated.position) > tolerance;

			if (!subdiv)
			{
				M::Vec::center(ref0.direction,ref1.direction,reference.direction);
				M::Vec::center(ref0.up,ref1.up,reference.up);
				M::Vec::center(ref0.right,ref1.right,reference.right);
				reference.angle0 = ref0.angle0*0.5 + ref1.angle0*0.5;
				reference.angle1 = ref0.angle1*0.5 + ref1.angle1*0.5;
				M::Vec::center(ref0.scale,ref1.scale,reference.scale);
				TVertex	p0,p1;
				reference.MakeVertex(0,p0);
				interpolated.MakeVertex(0,p1);
			
				subdiv = M::Vec::quadraticDistance(p0.position,p1.position) > tolerance;
				if (!subdiv)
				{
					reference.MakeVertex(1,p0);
					interpolated.MakeVertex(1,p1);

					subdiv = M::Vec::quadraticDistance(p0.position,p1.position) > tolerance;

				}
			}
		}

		if (subdiv)
			Subdivide(begin,end,control_points,tolerance,ref0,interpolated,t0,t,outIntersections,force-1);
		outIntersections << interpolated;
		if (subdiv)
			Subdivide(begin,end,control_points,tolerance,interpolated,ref1,t,t1,outIntersections,force-1);
	}



	/*static*/	void				SurfaceDescription::Interpolate(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end,const SurfaceDescription::TControl control_points[2], float t, InterpolatedSlice&out)
	{
		static const count_t	NumFloats = SurfaceDescription::TControl::NumFloats;
		VecUnroll<NumFloats>::resolveBezierCurvePoint(begin.state.field, control_points[0].field, control_points[1].field, end.state.field, t, out.field);
		M::Vec::resolveBezierCurveAxis(begin.state.position, control_points[0].position, control_points[1].position, end.state.position,t,out.direction);

		out.t = t;
	
		
		M::Vec::normalize0(out.direction);

		M::TVec3<>	temp;
		M::Vec::cross(out.up,out.direction,temp);
		M::Vec::cross(out.direction,temp,out.up);
		M::Vec::normalize0(out.up);
		M::Vec::cross(out.direction,out.up,out.right);

		if (out.angle1 - out.angle0 > 2.f)
		{
			float center = (out.angle0 + out.angle1)*0.5f;
			out.angle0 = center - 1.f;
			out.angle1 = center + 1.f;
			out.texcoord0 = 0;
			out.texcoord1 = 1;
		}
	}


	/*static*/	void				SurfaceDescription::UpdateSlope(const TControl&predecessor, const TControl&successor, bool smooth_direction, TConnector&connector)
	{
		VecUnroll<TControl::NumFloats>::sub(successor.field,predecessor.field,connector.slope.field);
		VecUnroll<TControl::NumFloats>::mult(connector.slope.field,0.5f);
		if (smooth_direction)
		{
			M::Vec::copy(connector.slope.position,connector.state.direction);
			M::Vec::normalize0(connector.state.direction);
		}
		else
			M::Vec::mult(connector.state.direction,M::Vec::distance(successor.position,predecessor.position)*0.5,connector.slope.position);
	}

	/*static*/	void				SurfaceDescription::UpdateRightSlope(const SurfaceDescription::TControl&predecessor, bool smooth_direction, TConnector&connector)
	{
		VecUnroll<TControl::NumFloats>::sub(connector.state.field,predecessor.field,connector.slope.field);
		if (smooth_direction)
		{
			M::Vec::copy(connector.slope.position,connector.state.direction);
			M::Vec::normalize0(connector.state.direction);
		}
		else
			M::Vec::mult(connector.state.direction,M::Vec::distance(predecessor.position,connector.state.position),connector.slope.position);
	}

	/*static*/	void				SurfaceDescription::UpdateLeftSlope(const SurfaceDescription::TControl&successor, bool smooth_direction, TConnector&connector)
	{
		VecUnroll<TControl::NumFloats>::sub(successor.field,connector.state.field,connector.slope.field);
		if (smooth_direction)
		{
			M::Vec::copy(connector.slope.position,connector.state.direction);
			M::Vec::normalize0(connector.state.direction);
		}
		else
			M::Vec::mult(connector.state.direction,M::Vec::distance(successor.position,connector.state.position),connector.slope.position);
	}



	/*static*/	void				SurfaceDescription::BuildControlPoints(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end, SurfaceDescription::TControl control_points[2], float control_factor0/*=0.39*/, float control_factor1/*=0.39*/)
	{
		M::TVec2<>	sd;
	
		static const count_t	NumFloats = SurfaceDescription::TControl::NumFloats;
		//float	d[SurfaceDescription::TControl::NumFloats];
		//VecUnroll<NumFloats>::sub(end_points[1].state.field,end_points[0].data.field,d);
		//float len = VecUnroll<3>::length(d);

		//VecUnroll<3>::mad(end_points[0].data.field,end_points[0].slope.field,control_factor0*len,control_points[0].field);
		VecUnroll<NumFloats>::mad(begin.state.field,begin.slope.field,control_factor0,control_points[0].field);

		VecUnroll<NumFloats>::mad(end.state.field,end.slope.field,-control_factor1,control_points[1].field);
	}

	void				SurfaceNetwork::Compact(Compacted&target)
	{
		target.nodes.SetSize(nodes.Count());
		target.segments.SetSize(segments.Count());
		target.nodeIsFlipped.SetSize(nodes.Count());

		bool*node_flipped_to = target.nodeIsFlipped.pointer();
		nodes.visitAllEntries([this,&target,&node_flipped_to](index_t key, Node&node)
		{
			(*node_flipped_to++) = NodeIsFlipped(key);
		});
		ASSERT_CONCLUSION(target.nodeIsFlipped,node_flipped_to);

		Node*node_to = target.nodes.pointer();

		nodes.visitAllEntries([this,&target,&node_to](index_t key, Node&node)
		{
			node.nodeID = node_to - target.nodes.pointer();
			(*node_to++) = node;
		});
		ASSERT_CONCLUSION(target.nodes,node_to);

		Segment*seg_to = target.segments.pointer();
		segments.visitAllEntries([this,&target,&seg_to](index_t key, Segment&segment)
		{
			segment.segID = seg_to - target.segments.pointer();
			(*seg_to) = segment;
		
			seg_to->connector[0].node = nodes.require(seg_to->connector[0].node).nodeID;
			seg_to->connector[1].node = nodes.require(seg_to->connector[1].node).nodeID;
			DBG_ASSERT_LESS__(seg_to->connector[0].node,target.nodes.size());
			target.nodes[seg_to->connector[0].node].segments[seg_to->connector[0].outbound][seg_to->connector[0].subdivisionStep] = seg_to - target.segments.pointer();
			DBG_ASSERT_LESS__(seg_to->connector[1].node,target.nodes.size());
			target.nodes[seg_to->connector[1].node].segments[seg_to->connector[1].outbound][seg_to->connector[1].subdivisionStep] = seg_to - target.segments.pointer();
			seg_to++;
		});
		ASSERT_CONCLUSION(target.segments,seg_to);
	}

	void				SurfaceNetwork::MoveCompact(Compacted&target)
	{
		target.nodes.SetSize(nodes.Count());
		target.segments.SetSize(segments.Count());
		target.nodeIsFlipped.SetSize(nodes.Count());

		bool*node_flipped_to = target.nodeIsFlipped.pointer();
		nodes.visitAllEntries([this,&target,&node_flipped_to](index_t key, Node&node)
		{
			(*node_flipped_to++) = NodeIsFlipped(key);
		});
		ASSERT_CONCLUSION(target.nodeIsFlipped,node_flipped_to);

		Node*node_to = target.nodes.pointer();

		nodes.visitAllEntries([this,&target,&node_to](index_t key, Node&node)
		{
			node.nodeID = node_to - target.nodes.pointer();
			(*node_to++).adoptData(node);
		});
		ASSERT_CONCLUSION(target.nodes,node_to);

		Segment*seg_to = target.segments.pointer();
		segments.visitAllEntries([this,&target,&seg_to](index_t key, Segment&segment)
		{
			segment.segID = seg_to - target.segments.pointer();
			(*seg_to).adoptData(segment);
		
			seg_to->connector[0].node = nodes.queryPointer(seg_to->connector[0].node)->nodeID;
			seg_to->connector[1].node = nodes.queryPointer(seg_to->connector[1].node)->nodeID;
			DBG_ASSERT_LESS__(seg_to->connector[0].node,target.nodes.size());
			target.nodes[seg_to->connector[0].node].segments[seg_to->connector[0].outbound][seg_to->connector[0].subdivisionStep] = seg_to - target.segments.pointer();
			DBG_ASSERT_LESS__(seg_to->connector[1].node,target.nodes.size());
			target.nodes[seg_to->connector[1].node].segments[seg_to->connector[1].outbound][seg_to->connector[1].subdivisionStep] = seg_to - target.segments.pointer();
			seg_to++;
		});
		ASSERT_CONCLUSION(target.segments,seg_to);
	}

	#if 0

	float							SurfaceNetwork::Segment::getMinimalDistanceTo(const M::TVec3<>&referencePointer, count_t scan_resolution/*=10*/)	const
	{
		float result = std::numeric_limits<float>::max();

		DBG_ASSERT_GREATER__(scan_resolution,1);

		for (index_t i = 0; i < scan_resolution; i++)
		{
			const float fi = float(i)/float(scan_resolution-1);

			M::Vec::resolveBezierCurvePoint(begin.

			VecUnroll<NumFloats>::ResolveBezierCurvePoint(begin.state.field, control_points[0].field, control_points[1].field, end.state.field, t, out.field);

		}


		return result;
	}
	#endif /*0*/
		
	index_t								SurfaceNetwork::Link(const Segment::Connector&slot0, const Segment::Connector&slot1, bool update_controls, bool build)
	{
		Node*n0 = nodes.queryPointer(slot0.node),
			*n1 = nodes.queryPointer(slot1.node);

		if (n0 == n1 || !n0 || !n1)
			return InvalidIndex;
	
		if (!n0->IsAvailable(slot0.outbound,slot0.subdivisionStep)
			||
			!n1->IsAvailable(slot1.outbound,slot1.subdivisionStep))
			return InvalidIndex;


		index_t result;
		Segment&seg = segments.Create(result);

		n0->segments[slot0.outbound][slot0.subdivisionStep] = result;
		n1->segments[slot1.outbound][slot1.subdivisionStep] = result;
		seg.connector[0] = slot0;
		seg.connector[1] = slot1;

		if (update_controls)
		{
			UpdateControls(seg,build);
		}
		return result;
	}

	void								SurfaceNetwork::UpdateSlopes(index_t node_index, bool update_connected_segments, bool rebuild_segments, Container::IndexSet*affectedSegments)
	{
		Node*node = nodes.queryPointer(node_index);
		DBG_ASSERT_NOT_NULL__(node);
		if (node)
			UpdateSlopes(*node,update_connected_segments,rebuild_segments,affectedSegments);
	}

	void								SurfaceNetwork::UpdateSlopes(Node&node, bool update_connected_segments, bool rebuild_segments, Container::IndexSet*affectedSegments)
	{
		static const count_t NumFloats = SurfaceDescription::TControl::NumFloats;

		SurfaceDescription::TControl	local_state,
										remote_state;
		float	delta[NumFloats],distance=0;
		VecUnroll<NumFloats>::clear(node.slope.field);
		count_t summed = 0;

		for (BYTE k = 0; k < 2; k++)
		{
			for (index_t i = 0; i < node.segments[k].Count(); i++)
			{
				index_t seg_index = node.segments[k][i];
				if (seg_index != InvalidIndex)
				{
					if (affectedSegments)
						affectedSegments->Set(seg_index);

					Segment*seg = segments.queryPointer(node.segments[k][i]);
					DBG_ASSERT_NOT_NULL__(seg);
					if (!seg)
					{
						node.segments[k][i] = InvalidIndex;
						continue;
					}
					Node*other = NULL;
					Segment::Connector c;
					if (nodes.queryPointer(seg->connector[0].node) == &node)
					{
						other = nodes.queryPointer(seg->connector[1].node);
						c = seg->connector[1];
					}
					elif (nodes.queryPointer(seg->connector[1].node) == &node)
					{
						other = nodes.queryPointer(seg->connector[0].node);
						c = seg->connector[0];
					}
					else
					{
						ASSERT__(nodes.Owns(&node));
						FATAL__("Data inconsistency detected");
						node.segments[k][i] = InvalidIndex;
					}

					if (other)
					{
						if (node.BuildState(k!=0,i,false,local_state)
							&&
							other->BuildState(c.outbound,c.subdivisionStep,true,remote_state))
						{
							distance += M::Vec::distance(local_state.position,remote_state.position);
							if (k)
								VecUnroll<NumFloats>::sub(remote_state.field,local_state.field,delta);
							else
								VecUnroll<NumFloats>::sub(local_state.field,remote_state.field,delta);
							VecUnroll<NumFloats>::add(node.slope.field,delta);
							summed++;
						}
						else
							FATAL__("inconsistent state");
					}
				}
			}
		}

		VecUnroll<NumFloats>::mult(node.slope.field,1.0f/float(summed));
		M::Vec::mult(node.direction,distance/float(summed),node.slope.position);

		if (update_connected_segments)
		{
			for (BYTE k = 0; k < 2; k++)
			{
				for (index_t i = 0; i < node.segments[k].Count(); i++)
					if (node.segments[k][i] != InvalidIndex)
					{
						UpdateControls(node.segments[k][i],rebuild_segments);
					}
			}
		}
	}


	/**
	@brief Performans a managed removal of the specified node.

	All segments currently connected to this node are erased automatically.
	@param nodeIndex Index of the node to erase. Must exist or a fatal is raised.
	@param updateConnectedSlopes Updates the slopes of all nodes that have lost a segment this way
	@param updateConnectedSegmentControls Updates all controls of all segments that were affected by this operation
	@param build Rebuilds the surface description of all segments that were affected by this operation
	@param[out] removedSegments Optional outbuffer to catch any removed segments. Cleared automatically
	@param[out] affectedSegments Optional outmap to catch any affected segments that still exist. Cleared automatically
	*/
	void		SurfaceNetwork::ManagedRemoveNode(index_t nodeIndex, bool updateConnectedSlopes, bool updateConnectedSegmentControls, bool build, Container::BasicVector<std::pair<index_t,PAttachment> >*removedSegments, Container::IndexSet*affectedSegments)
	{
		if (removedSegments)
			removedSegments->clear();
	
		static Container::IndexSet	localAffected;
		if (!affectedSegments && updateConnectedSegmentControls)
			affectedSegments = &localAffected;

		if (affectedSegments)
			affectedSegments->clear();
		static Container::PreAllocVector<index_t>	affectedNodes;
		if (updateConnectedSlopes)
			affectedNodes.clear();

		SurfaceNetwork::Node&node = nodes.require(nodeIndex);


		for (BYTE k = 0; k < 2; k++)
		{
			foreach (node.segments[k],segID)
			{
				SurfaceNetwork::Segment&seg = *segments.queryPointer(*segID);
				if (&seg == NULL)
					continue;
				int otherEnd = !seg.GetEndIndex(nodeIndex);
				if (updateConnectedSlopes)
					affectedNodes << seg.connector[otherEnd].node;

				if (removedSegments)
					removedSegments->append(std::make_pair(*segID,seg.attachment));
			
				{
					const Segment::Connector&c = seg.connector[otherEnd];
					Node&n = nodes.require(c.node);
					ASSERT_LESS__(c.subdivisionStep,n.segments[c.outbound].Count());

					n.segments[c.outbound].Erase(c.subdivisionStep);
					for (index_t i = c.subdivisionStep; i < n.segments[c.outbound].Count(); i++)
						if (n.segments[c.outbound][i] != InvalidIndex)
						{
							Segment&seg = segments.require(n.segments[c.outbound][i]);
							seg.connector[seg.GetEndIndex(c.node)].subdivisionStep--;
						}
					n.subdivision[c.outbound].SetSize(n.subdivision[c.outbound].GetLength()-1);
					MakeEven(n.subdivision[c.outbound]);
				}
				segments.Unset(*segID);
			}
		}
		if (updateConnectedSlopes)
			foreach (affectedNodes, nodeID)
			{
				SurfaceNetwork::Node*node = nodes.queryPointer(*nodeID);
				if (node)
					UpdateSlopes(*node,false,false,affectedSegments);
			}
		nodes.Unset(nodeIndex);

		if (updateConnectedSegmentControls)
			affectedSegments->visitAllKeys([this,build](index_t segID)
			{
				SurfaceNetwork::Segment*seg = segments.queryPointer(segID);
				if (seg)
					UpdateControls(*seg,build);
			});
	}

	SurfaceNetwork::Node&	SurfaceNetwork::Node::SplitEvenly(bool outbound, count_t num_slots)
	{
		ASSERT__(num_slots > 0);
		ASSERT_EQUAL__(segments[outbound].Count(),1);
		ASSERT_EQUAL__(segments[outbound].First(),InvalidIndex); 
	
		segments[outbound].SetSize(num_slots);
		segments[outbound].Fill(InvalidIndex); 
		subdivision[outbound].SetSize(num_slots-1);
		SurfaceNetwork::MakeEven(subdivision[outbound]);
		return *this;
	}

	/*static*/ void SurfaceNetwork::MakeEven(Ctr::Array<float>&subdivison_field)
	{
		for (index_t i = 0; i < subdivison_field.GetLength(); i++)
		{
			subdivison_field[i] = float(i+1)/float(subdivison_field.GetLength()+1);
		}
	}

	bool								SurfaceNetwork::Node::BuildState(bool outbound, index_t subdiv, bool second_node, SurfaceDescription::TControl&out_state)	const
	{
		DBG_ASSERT_LESS_OR_EQUAL__(subdiv,this->subdivision[outbound].Count());

		if (subdiv >= segments[outbound].Count())
			return false;
			//subdiv = slopes[outbound].Count()-1;

		out_state = *this;
		bool flip;
		if (outbound ^ second_node)
		{
			flip = false;
			//out.slope = slopes[outbound][subdiv];
		}
		else
		{
			flip = true;
			M::Vec::mult(out_state.direction,-1);
			//VecUnroll<SurfaceDescription::TControl::NumFloats>::mult(slopes[outbound][subdiv].field,-1,out.state.field);
		}

		if (subdivision[outbound].IsEmpty())
			return subdiv == 0;
		if (subdiv == 0)
		{
			out_state.angle0 = angle0;
			out_state.angle1 = angle0 + (angle1-angle0)*subdivision[outbound].First();

			out_state.texcoord0 = texcoord0;
			out_state.texcoord1 = texcoord0 + (texcoord1-texcoord0)*subdivision[outbound].First();
		}
		elif (subdiv == subdivision[outbound].Count())
		{
			out_state.angle0 = angle0 + (angle1-angle0)*subdivision[outbound].Last();
			out_state.angle1 = angle1;

			out_state.texcoord0 = texcoord0 + (texcoord1-texcoord0)*subdivision[outbound].Last();
			out_state.texcoord1 = texcoord1;
		}
		else
		{
			out_state.angle0 = angle0 + (angle1-angle0)*subdivision[outbound][subdiv-1];
			out_state.angle1 = angle0 + (angle1-angle0)*subdivision[outbound][subdiv];

			out_state.texcoord0 = texcoord0 + (texcoord1-texcoord0)*subdivision[outbound][subdiv-1];
			out_state.texcoord1 = texcoord0 + (texcoord1-texcoord0)*subdivision[outbound][subdiv];
		}

		if (flip)
		{
			out_state.angle0 *= -1;
			out_state.angle1 *= -1;
			out_state.texcoord0 = 1.f - out_state.texcoord0;
			out_state.texcoord1 = 1.f - out_state.texcoord1;
			swp(out_state.angle0,out_state.angle1);
			swp(out_state.texcoord0,out_state.texcoord1);
		}
		return true;
	}


	bool								SurfaceNetwork::Node::BuildConnector(bool outbound, index_t subdiv, bool second_node, SurfaceDescription::TConnector&out)	const
	{
		if (!BuildState(outbound,subdiv,second_node,out.state))
			return false;


		if (outbound ^ second_node)
		{
			out.slope = slope;
		}
		else
		{
		
			VecUnroll<SurfaceDescription::TControl::NumFloats>::mult(slope.field,-1,out.slope.field);
		}

		return true;
	}



	void								SurfaceNetwork::UpdateAllSlopes(bool update_all_controls, bool rebuild_segments)
	{
		nodes.visitAllValues([this](Node&n)
		{
			UpdateSlopes(n,false,false,NULL);
		});

		if (update_all_controls)
			segments.visitAllValues([this,rebuild_segments](Segment&s)
			{
				UpdateControls(s,rebuild_segments);
			});
	}


	void								SurfaceNetwork::UpdateControls(index_t segment, bool rebuild)
	{
		if (segment == InvalidIndex)
			return;
		Segment*seg = segments.queryPointer(segment);
		DBG_ASSERT_NOT_NULL__(seg);
		if (seg)
			UpdateControls(*seg,rebuild);
	}

	void								SurfaceNetwork::UpdateControls(Segment&segment, bool rebuild)
	{
		const Node	*n0 = nodes.queryPointer(segment.connector[0].node),
					*n1 = nodes.queryPointer(segment.connector[1].node);
		ASSERT_NOT_NULL__(n0);
		ASSERT_NOT_NULL__(n1);
		SurfaceDescription::TConnector	c0,
										c1;

		DBG_VERIFY__(n0->BuildConnector(segment.connector[0].outbound,segment.connector[0].subdivisionStep,false,c0));
		DBG_VERIFY__(n1->BuildConnector(segment.connector[1].outbound,segment.connector[1].subdivisionStep,true,c1));

		SurfaceDescription::BuildControlPoints(c0,c1,segment.controls,segment.connector[0].controlFactor,segment.connector[1].controlFactor);

		if (rebuild)
			RebuildSegment(segment);
	}

	void								SurfaceNetwork::RebuildSegment(index_t segment)
	{
		if (segment == InvalidIndex)
			return;
		Segment*seg = segments.queryPointer(segment);
		DBG_ASSERT_NOT_NULL__(seg);
		if (seg)
			RebuildSegment(*seg);
	}


	void							SurfaceNetwork::Node::BuildArc(Container::BasicVector<SurfaceDescription::TVertex>&arc_out, index_t lod, bool flipped, bool global)	const
	{
		float tolerance = minTolerance*(1<<lod);
		SurfaceDescription::BuildArc(arc_out,*this,tolerance,flipped, global);
	}



	void								SurfaceNetwork::RebuildSegment(Segment&segment)
	{
		const Node	*n0 = nodes.queryPointer(segment.connector[0].node),
					*n1 = nodes.queryPointer(segment.connector[1].node);
		DBG_ASSERT_NOT_NULL__(n0);
		DBG_ASSERT_NOT_NULL__(n1);
		if (!n0 || !n1)
			return;

		SurfaceDescription::TConnector	c0,c1;
		DBG_VERIFY__(n0->BuildConnector(segment.connector[0].outbound,segment.connector[0].subdivisionStep,false,c0));
		DBG_VERIFY__(n1->BuildConnector(segment.connector[1].outbound,segment.connector[1].subdivisionStep,true,c1));

		segment.compiledSurfaces.SetSize(numLODs*3);
		float tolerance = minTolerance;
		for (index_t i = 0; i < numLODs; i++)
		{
			segment.compiledSurfaces[i*3].BuildSegment(c0,c1,segment.controls,tolerance*2.0f,tolerance);
			segment.compiledSurfaces[i*3+1].BuildSegment(c0,c1,segment.controls,tolerance,tolerance*2.0f);
			segment.compiledSurfaces[i*3+2].BuildSegment(c0,c1,segment.controls,tolerance,tolerance);

			tolerance *= 2.0f;
		}
	}


	index_t							SurfaceNetwork::SmartMakeRoom(Node&node,index_t node_index,bool outbound,const Node&opposing_node)
	{
		count_t	need_slots = 1;
		for (index_t i = 0; i < node.segments[outbound].Count(); i++)
			need_slots += node.segments[outbound][i] != InvalidIndex;

		struct SortEntry
		{
			index_t	segment_index,
					remote_node_index;
			BYTE	segment_end_index;
			float3	remote_node_position;
			float	projected_x;
		};

		Ctr::Array<SortEntry>	sort(need_slots);
		SortEntry*out = sort.pointer();

		float3 x;
		M::Vec::cross(node.direction,node.up,x);

		float3 d;
		{
			out->segment_index = InvalidIndex;
			out->remote_node_index = InvalidIndex;
			out->segment_end_index = 0;
			out->remote_node_position = opposing_node.position;
			M::Vec::sub(out->remote_node_position,node.position,d);
			out->projected_x = M::Vec::dot(d,x) / M::Vec::dot(d,node.direction);
			out++;
		}



	
		if (need_slots > 1)
		{

			for (index_t i = 0; i < node.segments[outbound].Count(); i++)
				if (node.segments[outbound][i] != InvalidIndex)
				{
					out->segment_index = node.segments[outbound][i];
					const Segment&seg = *segments.queryPointer(node.segments[outbound][i]);
					out->segment_end_index = seg.GetEndIndex(node_index);
					out->remote_node_index = seg.connector[!out->segment_end_index].node;
					DBG_ASSERT_NOT_EQUAL__(out->remote_node_index,node_index);
					out->remote_node_position = nodes.queryPointer(out->remote_node_index)->position;

					M::Vec::sub(out->remote_node_position,node.position,d);
					out->projected_x = M::Vec::dot(d,x) / M::Vec::dot(d,node.direction);

					out++;
				}
		}

		ASSERT_CONCLUSION(sort,out);


		Sorting::ByComparator::quickSort(sort,[](const SortEntry&a, const SortEntry&b)->int {return a.projected_x > b.projected_x ? 1 : (a.projected_x < b.projected_x ? -1 : 0);});

		if (!outbound)
			sort.revert();

		index_t result = InvalidIndex;
		node.segments[outbound].SetSize(sort.Count());

		node.subdivision[outbound].SetSize(sort.Count()-1);
		for (index_t i = 0; i < node.subdivision[outbound].Count(); i++)
		{
			node.subdivision[outbound][i] = float(i+1)/float(sort.Count());
		}

		std::cout << "smart reorder of node "<<node_index<<" "<<(outbound?"outbound":"inbound")<<":"<<std::endl;
		for (index_t i = 0; i < sort.Count(); i++)
		{

			const SortEntry&entry = sort[i];
			if (entry.segment_index != InvalidIndex)
			{
				Segment&seg = *segments.queryPointer(entry.segment_index);
				seg.connector[entry.segment_end_index].subdivisionStep = i;
				node.segments[outbound][i] = entry.segment_index;
				std::cout << "mapped segment "<<entry.segment_index<<" ("<<seg.connector[0].node<<" to "<<seg.connector[1].node<<") connector["<<(int)entry.segment_end_index<<"] to slot "<<i<<std::endl;
			}
			else
			{
				DBG_ASSERT_EQUAL__(result,InvalidIndex);
				node.segments[outbound][i] = InvalidIndex;
				result = i;
				std::cout << "mapped new segment to slot "<<i<<std::endl;
			}
		}


		DBG_ASSERT_NOT_EQUAL__(result,InvalidIndex);

		return result;
	}


	index_t							SurfaceNetwork::SmartLink(index_t node0, index_t node1, bool manage_slots, bool update_controls, bool build, Container::IndexSet*affectedSegments)
	{
		#ifdef _DEBUG
			VerifyIntegrity();
		#endif

		Node*n0 = nodes.queryPointer(node0),
			*n1 = nodes.queryPointer(node1);
		if (n0 == n1 || !n0 || !n1)
			return InvalidIndex;
		float3	delta;
		M::Vec::sub(n1->position,n0->position,delta);


		bool outbound0 = M::Vec::dot(n0->direction,delta)>0.0f;
		if (!manage_slots && !n0->AnyAvailable(outbound0))
		{
			outbound0 = !outbound0;
			if (!n0->AnyAvailable(outbound0))
				return InvalidIndex;
		}
		bool outbound1 = M::Vec::dot(n1->direction,delta)<0.0f;
		if (!manage_slots && !n1->AnyAvailable(outbound1))
		{
			outbound1 = !outbound1;
			if (!n1->AnyAvailable(outbound1))
				return InvalidIndex;
		}

		index_t result;
		Segment&seg = segments.Create(result);
		if (affectedSegments)
			affectedSegments->Set(result);
		//DBG_ASSERT_EQUAL__(segments.queryPointer(result),&seg);
		index_t	subdiv0 = manage_slots ? SmartMakeRoom(*n0,node0,outbound0,*n1) : n0->GetFirstAvailable(outbound0);
		//DBG_ASSERT_EQUAL__(segments.queryPointer(result),&seg);

		index_t	subdiv1 = manage_slots ? SmartMakeRoom(*n1,node1,outbound1,*n0) : n1->GetFirstAvailable(outbound1);

		DBG_ASSERT_EQUAL__(segments.queryPointer(result),&seg);
		n0->segments[outbound0][subdiv0] = result;
		n1->segments[outbound1][subdiv1] = result;
		seg.connector[0] = Segment::Connector(node0,subdiv0,outbound0);
		seg.connector[1] = Segment::Connector(node1,subdiv1,outbound1);


		for (index_t i = 0; i < n0->segments[outbound0].Count(); i++)
		{
			if (n0->segments[outbound0][i] != InvalidIndex)
			{
				const Segment&seg = segments.require(n0->segments[outbound0][i]);
				BYTE end_point = seg.GetEndIndex(node0);
				DBG_ASSERT_EQUAL__(seg.connector[end_point].subdivisionStep ,i);
			}
		}
		for (index_t i = 0; i < n1->segments[outbound1].Count(); i++)
		{
			if (n1->segments[outbound1][i] != InvalidIndex)
			{
				const Segment&seg = segments.require(n1->segments[outbound1][i]);
				BYTE end_point = seg.GetEndIndex(node1);
				DBG_ASSERT_EQUAL__(seg.connector[end_point].subdivisionStep ,i);
			}
		}



		if (update_controls)
		{
			UpdateSlopes(*n0,false,false,NULL);
			UpdateSlopes(*n1,false,false,NULL);
			UpdateControls(seg,build);
			foreach (n0->segments[outbound0],seg)
				if (*seg != result)
				{
					if (affectedSegments)
						affectedSegments->Set(*seg);
					UpdateControls(*seg,build);
				}
			foreach (n1->segments[outbound1],seg)
				if (*seg != result)
				{
					if (affectedSegments)
						affectedSegments->Set(*seg);
					UpdateControls(*seg,build);
				}

			foreach (n0->segments[!outbound0],seg)
			{
				if (affectedSegments)
					affectedSegments->Set(*seg);
				UpdateControls(*seg,build);
			}
			foreach (n1->segments[!outbound1],seg)
			{
				if (affectedSegments)
					affectedSegments->Set(*seg);
				UpdateControls(*seg,build);
			}
		}

		#ifdef _DEBUG
			VerifyIntegrity();
		#endif


		return result;
	}


	void							SurfaceNetwork::VerifyIntegrity()	const
	{
		std::cout << "Verifying integrity"<<std::endl;
		nodes.visitAllEntries([this](index_t id, const Node&n)
		{
			std::cout << " Node "<<id<<": "<<std::endl;
			for (int k = 0; k < 2; k++)
			{
				if (n.segments[k].IsNotEmpty())
					ASSERT_EQUAL1__(n.subdivision[k].Count()+1,n.segments[k].Count(),k);
				foreach (n.subdivision[k],subdiv)
					ASSERT_IS_CONSTRAINED__(*subdiv,0,1);
				foreach (n.segments[k],sid)
				{
					if (*sid != InvalidIndex)
					{
						const Segment*seg = segments.queryPointer(*sid);
						if (!seg)
						{
							FATAL__("Targeted segment does not exist");
							continue;
						}
						seg->GetEndIndex(id);	//trigger verification
					}
				}
			}
		});
		std::cout << "Verifying integrity"<<std::endl;
		segments.visitAllEntries([this](index_t id, const Segment&seg)
		{
			std::cout << " Segment "<<id<<": "<<seg.connector[0].node<<" to "<<seg.connector[1].node<<std::endl;
			for (int k = 0; k < 2; k++)
			{
				std::cout << "  Checking connector "<<k<<": node "<<seg.connector[k].node<<" "<<(seg.connector[k].outbound?"outbound":"inbound")<<" subdiv "<<seg.connector[k].subdivisionStep<<std::endl;
				const Node	*n = nodes.queryPointer(seg.connector[k].node);
				if (!n)
				{
					FATAL__("Targeted node does not exist");
					continue;	//shit happens
				}

				if (seg.connector[k].subdivisionStep >= n->segments[seg.connector[k].outbound].Count())
				{
					FATAL__("Targeted subdivison "+String(seg.connector[k].subdivisionStep)+" points beyond node capacity "+String(n->segments[seg.connector[k].outbound].Count()));
					continue;
				}
				std::cout << "   Subdiv at "<<(seg.connector[k].subdivisionStep == 0 ? 0.f : n->subdivision[seg.connector[k].outbound][seg.connector[k].subdivisionStep-1])<<std::endl;
				ASSERT_EQUAL__(n->segments[seg.connector[k].outbound][seg.connector[k].subdivisionStep],id);
			}
		});
	}

	void SurfaceDescription::BuildBarriers(const SurfaceDescription&source, float barrierPosition, float barrierHeight0, float barrierHeight1, const M::TVec3<>&relativeTo)
	{
		vertices.reset();
		quadIndices.reset();
		triangleIndices.reset();

		foreach (source.edges,edge)
		{
			if (edge->direction != Edge::Left && edge->direction != Edge::Right)
				continue;
			index_t vertex_offset = vertices.Count();
			TVertex*const vfield = vertices.appendRow(2*edge->GetLength());
			Concurrency::parallel_for(index_t(0),edge->GetLength(),[vfield,&source,edge,barrierPosition,barrierHeight0,barrierHeight1,relativeTo](index_t i)
			{
				const TVertex&v = source.vertices[edge->at(i)];
				TVertex*vout = vfield + i*2;
				vout[0] = v;
				if (edge->direction == SurfaceDescription::Edge::Right)
					vout[0].position += v.tangent * barrierPosition + v.normal * barrierHeight0;
				else
					vout[0].position += v.tangent * -barrierPosition + v.normal * barrierHeight0;
				vout[0].position -= relativeTo;
				vout[0].tcoord.y = v.tcoord.y;
				vout[0].tcoord.x = 0;
				vout[1] = vout[0];
				vout[1].position += v.normal * (barrierHeight1 - barrierHeight0);
				vout[1].tcoord.y = v.tcoord.y;
				vout[1].tcoord.x = 1.f;
			});

			for (index_t i = 0; i+1 < edge->GetLength(); i++)
			{
				const UINT32	t = static_cast<UINT32>(vertex_offset+i*2),
								n = static_cast<UINT32>(vertex_offset+(i+1)*2);
				quadIndices		<< t   << t+1 << n+1 << n
								<< t+1 << t << n << n+1;
			}
		}

	}

	float3 SurfaceDescription::GetEdgeCenter() const
	{
		float3 result(0);
		count_t counter = 0;
		foreach (edges, edge)
			foreach (*edge,index)
			{
				result += vertices[*index].position;
				counter++;
			}
		result /= counter;
		return result;
	}

	void	SurfaceDescription::GetEdgeExtend(const M::TMatrix4<>&transformBy, M::Box<>&result)	const
	{
		result.SetAllMax(std::numeric_limits<float>::min());
		result.SetAllMin(std::numeric_limits<float>::max());
		foreach (edges, edge)
			foreach (*edge,index)
			{
				M::TVec3<> rotated;
				M::Mat::Transform(transformBy,vertices[*index].position,rotated);
				result.Include(rotated);
			}
	}

	void SurfaceDescription::BuildRails(const SurfaceDescription&source, const Container::BasicVector<float2>&profile, const M::TVec3<>&relativeTo)
	{
		vertices.reset();
		quadIndices.reset();
		triangleIndices.reset();

		//float2 texExt = float2(fabs(outerExtend - innerExtend), fabs(upperExtend - lowerExtend))*2.0f;

		const count_t numVerticesPerSlice = (profile.Count()-1)*2;

		Ctr::Array<float>	texcoord(profile.Count());
		Ctr::Array<float2>	tangent(profile.Count()-1);
		Ctr::Array<float2>	normal(profile.Count()-1);
		float2 center(0);
		{
			float at = 0.f;
			texcoord[0] = 0;
			center += profile[0];

			for (index_t i = 1; i < profile.Count(); i++)
			{
				at += M::Vec::distance(profile[i-1],profile[i]);
				texcoord[i] = at;
				tangent[i-1] = (profile[i] - profile[i-1]).Normalize();
				normal[i-1] = tangent[i-1].Normal();
				center += profile[i];
			}
		}
		center /= profile.Count();


		foreach (source.edges,edge)
		{
			if (edge->direction != Edge::Left && edge->direction != Edge::Right)
				continue;

			bool isLeftEdge = edge->direction == SurfaceDescription::Edge::Left;
			index_t vertex_offset = vertices.Count();
			TVertex*const vfield = vertices.appendRow(numVerticesPerSlice*edge->GetLength()+numVerticesPerSlice*2+2);	//top, top, left, left, bottom, bottom, right, right. two caps
			//if (edge->leftEdge)
			{
				float xFactor = isLeftEdge ? -1.f : 1.f;
				Concurrency::parallel_for(index_t(0),edge->GetLength(),[xFactor,numVerticesPerSlice,vfield,&source,edge,&profile,&texcoord,&tangent,&normal,relativeTo](index_t i)
				{
					TVertex v = source.vertices[edge->at(i)];
					v.position -= relativeTo;
					TVertex*vout = vfield + i*numVerticesPerSlice;

					for (index_t i = 1; i < profile.GetLength(); i++)
					{
						const index_t v0 = (i-1)*2,
										v1 = v0 +1;
						vout[v0].position = v.position +  v.tangent * xFactor * profile[i-1].x + v.normal * profile[i-1].y;
						vout[v0].tcoord.x = texcoord[i-1] * 2.f;
						vout[v0].normal =  v.tangent * normal[i-1].x * xFactor + v.normal * normal[i-1].y;
						vout[v0].tangent = - v.tangent * tangent[i-1].x - v.normal * tangent[i-1].y * xFactor;
						vout[v0].tcoord.y = v.tcoord.y;
						vout[v0].tx = v.tx;

						vout[v1].position = v.position + v.tangent * profile[i].x * xFactor + v.normal * profile[i].y;
						vout[v1].tcoord.x = texcoord[i] * 2.f;
						vout[v1].tangent = vout[v0].tangent;
						vout[v1].normal = vout[v0].normal;
						vout[v1].tcoord.y = v.tcoord.y;
						vout[v1].tx = v.tx;
					}
				});

				//front cap:
				{
					TVertex v = source.vertices[edge->First()];
					v.position -= relativeTo;
					TVertex*vout = vfield + numVerticesPerSlice*edge->GetLength();
					vout[0].position = v.position + v.tangent * center.x * xFactor + v.normal * center.y;
					vout[0].tangent = v.tangent* xFactor;
					vout[0].normal = -(v.normal | v.tangent) * xFactor;
					vout[0].tcoord = center * 2.f;
					vout[0].tx = v.tx;
					for (index_t i = 1; i < profile.GetLength(); i++)
					{
						const index_t	v0 = (i-1)*2+1,
										v1 = v0 +1;
						vout[v0].position = v.position + v.tangent * profile[i-1].x * xFactor + v.normal * profile[i-1].y;
						vout[v0].tangent = vout[0].tangent;
						vout[v0].normal = vout[0].normal;
						vout[v0].tcoord = profile[i-1] * 2.f;
						vout[v0].tx = vout[0].tx;

						vout[v1].position = v.position + v.tangent * profile[i].x * xFactor + v.normal * profile[i].y;
						vout[v1].tangent = vout[0].tangent;
						vout[v1].normal = vout[0].normal;
						vout[v1].tcoord = profile[i] * 2.f;
						vout[v1].tx = vout[0].tx;
					}
				}
				//rear cap:
				{
					TVertex v = source.vertices[edge->Last()];
					v.position -= relativeTo;
					TVertex*vout = vfield + numVerticesPerSlice*edge->GetLength() + numVerticesPerSlice + 1;
					vout[0].position = v.position + v.tangent * center.x * xFactor + v.normal * center.y;
					vout[0].tangent = -v.tangent* xFactor;
					vout[0].normal = -(v.tangent | v.normal)* xFactor;
					vout[0].tcoord = center * 2.f;
					vout[0].tx = v.tx;
					for (index_t i = 1; i < profile.GetLength(); i++)
					{
						const index_t	v0 = (i-1)*2+1,
										v1 = v0 +1;
						vout[v0].position = v.position + v.tangent * profile[i-1].x * xFactor + v.normal * profile[i-1].y;
						vout[v0].tangent = vout[0].tangent;
						vout[v0].normal = vout[0].normal;
						vout[v0].tcoord = profile[i-1] * 2.f;
						vout[v0].tx = vout[0].tx;

						vout[v1].position = v.position + v.tangent * profile[i].x * xFactor + v.normal * profile[i].y;
						vout[v1].tangent = vout[0].tangent;
						vout[v1].normal = vout[0].normal;
						vout[v1].tcoord = profile[i] * 2.f;
						vout[v1].tx = vout[0].tx;
					}
				}
				for (index_t i = 0; i+1 < edge->GetLength(); i++)
				{
					const UINT32	t = static_cast<UINT32>(vertex_offset+i*numVerticesPerSlice),
									n = static_cast<UINT32>(vertex_offset+(i+1)*numVerticesPerSlice);

					for (index_t j = 1; j < profile.GetLength(); j++)
					{
						quadIndices << static_cast<UINT32>(t + (j-1)*2) << static_cast<UINT32>(t + (j-1)*2 + 1)
									<< static_cast<UINT32>(n + (j-1)*2 + 1) << static_cast<UINT32>(n + (j-1)*2);
					}


				}
				{
					const UINT32	front = static_cast<UINT32>(vertex_offset+numVerticesPerSlice*edge->GetLength()),
									rear = static_cast<UINT32>(vertex_offset+numVerticesPerSlice*edge->GetLength() + numVerticesPerSlice + 1);
					for (index_t j = 1; j < profile.GetLength(); j++)
					{
						triangleIndices << front << front + static_cast<UINT32>(((j-1)*2 + 1) % numVerticesPerSlice) + 1 << front + static_cast<UINT32>(j-1)*2+1;
					}
					triangleIndices << front << front + 1 << front + static_cast<UINT32>(profile.GetLength()-1)*2;
					for (index_t j = 1; j < profile.GetLength(); j++)
						triangleIndices << rear << rear + static_cast<UINT32>(j-1)*2+1 << rear + static_cast<UINT32>(((j-1)*2 + 1) % numVerticesPerSlice) + 1;
					triangleIndices << rear << rear + static_cast<UINT32>(profile.GetLength()-1)*2 << rear + 1;
				}
			}

		}
	}

	void SurfaceDescription::BuildRails(const SurfaceDescription&source, float innerExtend, float outerExtend, float upperExtend, float lowerExtend, const M::TVec3<>&relativeTo)
	{
		vertices.reset();
		quadIndices.reset();
		triangleIndices.reset();

		using std::fabs;
		float2 texExt = float2(fabs(outerExtend - innerExtend), fabs(upperExtend - lowerExtend))*2.0f;

		foreach (source.edges,edge)
		{
			if (edge->direction != Edge::Left && edge->direction != Edge::Right)
				continue;

			bool isLeftEdge = edge->direction == SurfaceDescription::Edge::Left;
			index_t vertex_offset = vertices.Count();
			TVertex*const vfield = vertices.appendRow(8*edge->GetLength()+8);	//top, top, left, left, bottom, bottom, right, right. two caps
			if (isLeftEdge)
			{
				Concurrency::parallel_for(index_t(0),edge->GetLength(),[texExt,vfield,&source,edge,innerExtend,upperExtend,outerExtend,lowerExtend,relativeTo](index_t i)
				{
					TVertex v = source.vertices[edge->at(i)];
					v.position -= relativeTo;
					TVertex*vout = vfield + i*8;
					vout[0] = v;
					vout[0].position += v.tangent * innerExtend + v.normal * upperExtend;
					vout[0].tcoord.x = 0;
					vout[1] = v;
					vout[1].position += v.tangent * -outerExtend + v.normal * upperExtend;
					vout[1].tcoord.x = texExt.x;
					vout[2] = vout[1];
					vout[2].normal = -v.tangent;
					vout[2].tangent = v.normal;
					vout[2].tcoord.x = 0;
					vout[3] = vout[2];
					vout[3].position -= v.normal * (upperExtend+lowerExtend);
					vout[3].tcoord.x = texExt.x;
					vout[4] = vout[3];
					vout[4].normal = -v.normal;
					vout[4].tangent = -v.tangent;
					vout[4].tcoord.x = 0;
					vout[5] = vout[4];
					vout[5].position += v.tangent * (innerExtend+outerExtend);
					vout[5].tcoord.x = texExt.x;
					vout[6] = vout[5];
					vout[6].normal = v.tangent;
					vout[6].tangent = -v.normal;
					vout[6].tcoord.x = 0;
					vout[7] = vout[6];
					vout[7].position = vout[0].position;
					vout[7].tcoord.x = texExt.x;
				});

				{
					TVertex v = source.vertices[edge->First()];
					v.position -= relativeTo;
					TVertex*vout = vfield + 8*edge->GetLength();
					vout[0] = v;
					vout[0].normal = v.normal | v.tangent;
					vout[0].tangent = -v.tangent;
					vout[0].position += v.tangent * innerExtend + v.normal * upperExtend;
					vout[0].tcoord = float2(0,0);
					vout[1] = vout[0];
					vout[1].position = v.position + v.tangent * -outerExtend + v.normal * upperExtend;
					vout[1].tcoord = float2(0,1);
					vout[2] = vout[1];
					vout[2].position -= v.normal * (upperExtend+lowerExtend);
					vout[2].tcoord = float2(texExt.y,1);
					vout[3] = vout[2];
					vout[3].position += v.tangent * (innerExtend+outerExtend);
					vout[3].tcoord = float2(texExt.y,0);
				}
				{
					TVertex v = source.vertices[edge->Last()];
					v.position -= relativeTo;
					TVertex*vout = vfield + 8*edge->GetLength() +4;
					vout[0] = v;
					vout[0].normal = v.tangent | v.normal;
					vout[0].position += v.tangent * innerExtend + v.normal * upperExtend;
					vout[0].tcoord = float2(0,0);
					vout[1] = vout[0];
					vout[1].position = v.position + v.tangent * -outerExtend + v.normal * upperExtend;
					vout[1].tcoord = float2(0,1);
					vout[2] = vout[1];
					vout[2].position -= v.normal * (upperExtend+lowerExtend);
					vout[2].tcoord = float2(texExt.y,1);
					vout[3] = vout[2];
					vout[3].position += v.tangent * (innerExtend+outerExtend);
					vout[3].tcoord = float2(texExt.y,0);
				}
				for (index_t i = 0; i+1 < edge->GetLength(); i++)
				{
					const UINT32	t = static_cast<UINT32>(vertex_offset+i*8),
									n = static_cast<UINT32>(vertex_offset+(i+1)*8);
					quadIndices	<< t   << t+1 << n+1 << n
									<< t+2 << t+3 << n+3 << n+2
									<< t+4 << t+5 << n+5 << n+4
									<< t+6 << t+7 << n+7 << n+6;
				}
				{
					const UINT32	t = static_cast<UINT32>(vertex_offset+edge->GetLength()*8);
					quadIndices	<< t+3 << t+2 << t+1 << t+0
									<< t+4 << t+5 << t+6 << t+7
									;
				}
			}
			else
			{
				Concurrency::parallel_for(index_t(0),edge->GetLength(),[texExt,vfield,&source,edge,innerExtend,upperExtend,outerExtend,lowerExtend,relativeTo](index_t i)
				{
					TVertex v = source.vertices[edge->at(i)];
					v.position -= relativeTo;
					TVertex*vout = vfield + i*8;
					vout[0] = v;
					vout[0].position += v.tangent * -innerExtend + v.normal * upperExtend;
					vout[0].tcoord.x = 0;
					vout[1] = v;
					vout[1].position += v.tangent * outerExtend + v.normal * upperExtend;
					vout[1].tcoord.x = texExt.x;
					vout[2] = vout[1];
					vout[2].normal = v.tangent;
					vout[2].tangent = v.normal;
					vout[2].tcoord.x = 0;
					vout[3] = vout[2];
					vout[3].position -= v.normal * (upperExtend+lowerExtend);
					vout[3].tcoord.x = texExt.x;
					vout[4] = vout[3];
					vout[4].normal = -v.normal;
					vout[4].tangent = -v.tangent;
					vout[4].tcoord.x = 0;
					vout[5] = vout[4];
					vout[5].position -= v.tangent * (innerExtend+outerExtend);
					vout[5].tcoord.x = texExt.x;
					vout[6] = vout[5];
					vout[6].normal = -v.tangent;
					vout[6].tangent = -v.normal;
					vout[6].tcoord.x = 0;
					vout[7] = vout[6];
					vout[7].position = vout[0].position;
					vout[7].tcoord.x = texExt.x;
				});
				{
					TVertex v = source.vertices[edge->First()];
					v.position -= relativeTo;
					TVertex*vout = vfield + 8*edge->GetLength();
					vout[0] = v;
					vout[0].normal = v.tangent | v.normal;
					vout[0].tangent = -v.tangent;
					vout[0].position += v.tangent * -innerExtend + v.normal * upperExtend;
					vout[0].tcoord = float2(0,0);
					vout[1] = vout[0];
					vout[1].position = v.position + v.tangent * outerExtend + v.normal * upperExtend;
					vout[1].tcoord = float2(0,1);
					vout[2] = vout[1];
					vout[2].position -= v.normal * (upperExtend+lowerExtend);
					vout[2].tcoord = float2(texExt.y,1);
					vout[3] = vout[2];
					vout[3].position -= v.tangent * (innerExtend+outerExtend);
					vout[3].tcoord = float2(texExt.y,0);
				}
				{
					TVertex v = source.vertices[edge->Last()];
					v.position -= relativeTo;
					TVertex*vout = vfield + 8*edge->GetLength() +4;
					vout[0] = v;
					vout[0].normal = v.normal | v.tangent;
					vout[0].position += v.tangent * -innerExtend + v.normal * upperExtend;
					vout[0].tcoord = float2(0,0);
					vout[1] = vout[0];
					vout[1].position = v.position + v.tangent * outerExtend + v.normal * upperExtend;
					vout[1].tcoord = float2(0,1);
					vout[2] = vout[1];
					vout[2].position -= v.normal * (upperExtend+lowerExtend);
					vout[2].tcoord = float2(texExt.y,1);
					vout[3] = vout[2];
					vout[3].position -= v.tangent * (innerExtend+outerExtend);
					vout[3].tcoord = float2(texExt.y,0);
				}
				for (index_t i = 0; i+1 < edge->GetLength(); i++)
				{
					const UINT32	t = static_cast<UINT32>(vertex_offset+i*8),
									n = static_cast<UINT32>(vertex_offset+(i+1)*8);
					quadIndices	<< t   << t+1 << n+1 << n
									<< t+2 << t+3 << n+3 << n+2
									<< t+4 << t+5 << n+5 << n+4
									<< t+6 << t+7 << n+7 << n+6;
				}
				{
					const UINT32	t = static_cast<UINT32>(vertex_offset+edge->GetLength()*8);
					quadIndices	<< t+3 << t+2 << t+1 << t+0
									<< t+4 << t+5 << t+6 << t+7
									;
				}
			}
		}
	}

	void	SurfaceDescription::GenerateNormals()
	{
		for (index_t i = 0; i < vertices.Count(); i++)
			M::Vec::clear(vertices[i].normal);
		float3	normal,normal1;
		for (index_t i = 0; i < triangleIndices.Count(); i+=3)
		{
			const UINT32*t = triangleIndices+i;
			Obj::triangleNormal(vertices[t[0]].position,vertices[t[1]].position,vertices[t[2]].position,normal);
			M::Vec::add(vertices[t[0]].normal,normal);
			M::Vec::add(vertices[t[1]].normal,normal);
			M::Vec::add(vertices[t[2]].normal,normal);
		}
		for (index_t i = 0; i < quadIndices.Count(); i+=4)
		{
			const UINT32*q = quadIndices+i;
			Obj::triangleNormal(vertices[q[0]].position,vertices[q[1]].position,vertices[q[2]].position,normal);
			Obj::triangleNormal(vertices[q[0]].position,vertices[q[2]].position,vertices[q[3]].position,normal1);
			normal += normal1;
			M::Vec::add(vertices[q[0]].normal,normal);
			M::Vec::add(vertices[q[1]].normal,normal);
			M::Vec::add(vertices[q[2]].normal,normal);
			M::Vec::add(vertices[q[3]].normal,normal);
		}

		for (index_t i = 0; i < vertices.Count(); i++)
			M::Vec::normalize0(vertices[i].normal);

	}


	void SurfaceDescription::BuildArc(const OutVertexContainer&arc_vertices, float near_distance, float far_distance, float extend_along_track)
	{
		vertices.reset();
		for (index_t i = 0; i < arc_vertices.Count(); i++)
		{
			const TVertex&v = arc_vertices[i];
			float3 binormal;
			M::Vec::cross(v.normal,v.tangent,binormal);
			//need 8 versions per vertex, plus another 4 for the end and beginning vertices

			//forward upper:
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = v.normal;
				vtx.tangent = v.tangent;
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 1.f;
			}
			//backward upper:
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = v.normal;
				vtx.tangent = v.tangent;
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 0.f;
			}

			//forward lower:
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				M::Vec::mult(v.normal,-1.f,vtx.normal);
				M::Vec::mult(v.tangent,-1.f,vtx.tangent);
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 1.f;
			}
			//backward lower:
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				M::Vec::mult(v.normal,-1.f,vtx.normal);
				M::Vec::mult(v.tangent,-1.f,vtx.tangent);
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 0.f;
			}











			//forward upper (front):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = binormal;
				vtx.tangent = v.tangent;
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 0.2f;
			}
			//forward lower (front):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				vtx.normal = binormal;
				vtx.tangent = v.tangent;
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 0.f;
			}

			//backward upper (back):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				M::Vec::mult(binormal,-1.f,vtx.normal);
				M::Vec::mult(v.tangent,-1.f,vtx.tangent);
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 0.2f;
			}

			//backward lower (back):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				M::Vec::mult(binormal,-1.f,vtx.normal);
				M::Vec::mult(v.tangent,-1.f,vtx.tangent);
				vtx.tx = v.tx;
				vtx.tcoord.x = v.tcoord.x;
				vtx.tcoord.y = 0.f;
			}
		}
		//left:
		{
			const TVertex&v = arc_vertices.First();
			float3 binormal;
			M::Vec::cross(v.normal,v.tangent,binormal);

			//forward upper (front):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = -float3::reinterpret(v.tangent);
				vtx.tangent = -binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 1.f;
				vtx.tcoord.y = 1.f;
			}
			//forward lower (front):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				vtx.normal = -float3::reinterpret(v.tangent);
				vtx.tangent = -binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 1.f;
				vtx.tcoord.y = 0.f;
			}

			//backward lower (back):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				vtx.normal = -float3::reinterpret(v.tangent);
				vtx.tangent = -binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 0.f;
				vtx.tcoord.y = 0.f;
			}
			//backward upper (back):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = -float3::reinterpret(v.tangent);
				vtx.tangent = -binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 0.f;
				vtx.tcoord.y = 1.f;
			}


		}
		//right:
		{
			const TVertex&v = arc_vertices.Last();
			float3 binormal;
			M::Vec::cross(v.normal,v.tangent,binormal);

			//forward upper (front):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = float3::reinterpret(v.tangent);
				vtx.tangent = binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 1.f;
				vtx.tcoord.y = 1.f;
			}
			//forward lower (front):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				vtx.normal = float3::reinterpret(v.tangent);
				vtx.tangent = binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 1.f;
				vtx.tcoord.y = 0.f;
			}



			//backward lower (back):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-far_distance);
				vtx.normal = float3::reinterpret(v.tangent);
				vtx.tangent = binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 0.f;
				vtx.tcoord.y = 0.f;
			}
			//backward upper (back):
			{
				TVertex&vtx = vertices.Append();
				M::Vec::mad(v.position,binormal,-extend_along_track, vtx.position);
				M::Vec::mad(vtx.position,v.normal,-near_distance);
				vtx.normal = float3::reinterpret(v.tangent);
				vtx.tangent = binormal;
				vtx.tx = v.tx;
				vtx.tcoord.x = 0.f;
				vtx.tcoord.y = 1.f;
			}
		}
		triangleIndices.reset();
		quadIndices.reset();
		edges.reset();
		for (index_t i = 0; i+1 < arc_vertices.Count(); i++)
		{
			const UINT32	t = static_cast<UINT32>(i*8),
							n = static_cast<UINT32>((i+1)*8);
			quadIndices
					<< t << t+1 << n+1 << n		//upper plate
					<< t+3 << t+2 << n+2 << n+3	//lower plate	
					<< t+5 << t+4 << n+4 << n+5	//front plate
					<< t+6 << t+7 << n+7 << n+6	//back plate
					;
		}

		{
			const UINT32 offset = static_cast<UINT32>(arc_vertices.Count()*8);
			quadIndices
					<< offset << offset+1 << offset+2 << offset+3
					<< offset+7 << offset+6 << offset+5 << offset+4
					;
		}
	}


	bool	SurfaceNetwork::NodeIsFlipped(index_t node_id)	const
	{
		const Node&node = *nodes.queryPointer(node_id);
		for (BYTE outbound = 0; outbound < 2; outbound++)
		{
			for (index_t i = 0; i < node.segments[outbound].Count(); i++)
				if (node.segments[outbound][i] != InvalidIndex)
				{
					const Segment&seg = *segments.queryPointer(node.segments[outbound][i]);
					BYTE second_node = seg.connector[1].node == node_id;
					if (second_node ^ outbound)
						return false;
					return true;
				}
		}
		return false;
	}

	/*static*/	void		SurfaceNetwork::CompileBarrierGeometry(CGS::Geometry<>&target,const Segment&segment, float position,float height0,float height1,name64_t texture, CGS::TextureResource*resource/*=NULL*/)
	{
		if (segment.compiledSurfaces.Count() < 3*SurfaceNetwork::numLODs)
		{
			FATAL__("Unable to compile barrier: insufficient compiled surfaces");
			return;
		}
		static Ctr::Array<SurfaceDescription>					lods;

		lods.SetSize(SurfaceNetwork::numLODs);

		M::TVec3<>	center = {0,0,0};
		count_t counter = 0;
		foreach (segment.compiledSurfaces.First().edges, edge)
			foreach (*edge,index)
			{
				M::Vec::add(center,segment.compiledSurfaces.First().vertices[*index].position);
				counter++;
			}
		M::Vec::div(center,counter);
	

		for (index_t i = 0; i < SurfaceNetwork::numLODs; i++)
		{
			lods[i].BuildBarriers(segment.compiledSurfaces[3*i+2],position, height0, height1, center );
		}
		float shortest_edge = height1-height0;


		CompileFromDescriptions(target,lods,shortest_edge,texture,0,resource);

		target.material_field.First().info.ambient.rgb = float3(1.f);
		target.material_field.First().info.diffuse.rgb = float3(0.f);
		target.material_field.First().info.specular.rgb = float3(0.f);
		target.root_system.moveTo(center);
	}


	/*static*/	void		SurfaceNetwork::CompileRailGeometry(CGS::Geometry<>&target, const Segment&segment, float innerExtend, float outerExtend, float upperExtend, float lowerExtend, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource /*=NULL*/)
	{
		if (segment.compiledSurfaces.Count() < 3*SurfaceNetwork::numLODs)
		{
			FATAL__("Unable to compile rail: insufficient compiled surfaces");
			return;
		}
		static Ctr::Array<SurfaceDescription>					lods;

		lods.SetSize(SurfaceNetwork::numLODs);

		M::TVec3<>	center = segment.compiledSurfaces[2].GetEdgeCenter();

		for (index_t i = 0; i < SurfaceNetwork::numLODs; i++)
		{
			lods[i].BuildRails(segment.compiledSurfaces[3*i+2],innerExtend, outerExtend, upperExtend, lowerExtend, center );
		}
		float shortest_edge = innerExtend+outerExtend;


		CompileFromDescriptions(target,lods,shortest_edge,texture,normal_texture,resource);

		target.root_system.moveTo(center);

	}

	/*static*/	void		SurfaceNetwork::CompileRailGeometry(CGS::Geometry<>&target, const Segment&segment, const Container::BasicVector<float2>&profile, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource /*=NULL*/)
	{
		if (segment.compiledSurfaces.Count() < 3*SurfaceNetwork::numLODs)
		{
			FATAL__("Unable to compile rail: insufficient compiled surfaces");
			return;
		}
		static Ctr::Array<SurfaceDescription>					lods;

		lods.SetSize(SurfaceNetwork::numLODs);

		M::TVec3<>	center = segment.compiledSurfaces[2].GetEdgeCenter();

		for (index_t i = 0; i < SurfaceNetwork::numLODs; i++)
		{
			lods[i].BuildRails(segment.compiledSurfaces[3*i+2],profile, center );
		}
		float shortest_edge = std::numeric_limits<float>::max();
		for (index_t i = 1; i < profile.GetLength(); i++)
			shortest_edge = std::min(shortest_edge,M::Vec::quadraticDistance(profile[i-1],profile[i]));
		shortest_edge = sqrt(shortest_edge);

		CompileFromDescriptions(target,lods,shortest_edge,texture,normal_texture,resource);

		target.root_system.moveTo(center);

	}

	/*static*/ void SurfaceNetwork::CompileArcGeometry(CGS::Geometry<>&target, const Node&node, float near_distance, float far_distance, float extend_along_track,name64_t texture, name64_t normal_texture, bool nodeIsFlipped, CGS::TextureResource*resource /*=NULL*/)
	{

		static Ctr::Array<Container::PreAllocVector<SurfaceDescription::TVertex> >	arc_vertices;
		static Ctr::Array<SurfaceDescription>					arc_descriptions;

		arc_vertices.SetSize(SurfaceNetwork::numLODs);
		arc_descriptions.SetSize(SurfaceNetwork::numLODs);


		for (index_t i = 0; i < SurfaceNetwork::numLODs; i++)
		{

			node.BuildArc(arc_vertices[i],i, nodeIsFlipped, false);
			arc_descriptions[i].BuildArc(arc_vertices[i], near_distance, far_distance, extend_along_track);
		}

		float shortest_edge = std::numeric_limits<float>::max();

		using std::min;
		for (index_t i = 1; i < arc_vertices.First().Count(); i++)
			shortest_edge = min(shortest_edge,M::Vec::quadraticDistance(arc_vertices.First()[i-1].position,arc_vertices.First()[i].position));
		shortest_edge = sqrt(shortest_edge);


		CompileFromDescriptions(target,arc_descriptions,shortest_edge,texture,normal_texture,resource);

		M::Vec::copy(node.position,target.root_system.matrix.w.xyz);
		M::Vec::copy(node.direction,target.root_system.matrix.y.xyz);
		M::Vec::normalize0(target.root_system.matrix.y.xyz);
		M::Vec::cross(target.root_system.matrix.y.xyz,node.up,target.root_system.matrix.x.xyz);
		M::Vec::normalize0(target.root_system.matrix.x.xyz);
		M::Vec::cross(target.root_system.matrix.x.xyz,target.root_system.matrix.y.xyz,target.root_system.matrix.z.xyz);
		M::Mat::resetBottomRow(target.root_system.matrix);
		target.root_system.update();
	}



	/*static*/ void SurfaceNetwork::CompileFromDescriptions(CGS::Geometry<>&target, const Ctr::Array<SurfaceDescription>&lods, float shortest_edge, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource /*=NULL*/)
	{
		target.object_field.SetSize(1);
		target.object_field[0].name = String(str2name("object"));
	
		target.material_field.SetSize(1);
		//target.material_field[0].data.object_field.Resize(detail_levels);
		M::Vec::set(target.material_field[0].info.specular,0.7);
		//target.material_field[0].info.shininess = 0;

		if (!resource)
			resource = &GraphMesh::texture_resource;

		target.texture_resource = resource;
	
		//if (texture != 0)
		{
			M::Vec::def(target.material_field[0].info.ambient,0.4,0.4,0.4,1);
			target.material_field[0].info.attachment.reset();
			target.material_field[0].info.layer_field.SetSize((texture!=0) + (normal_texture!=0));
			if (texture != 0)
			{
				target.material_field[0].info.layer_field[0].combiner = GL_MODULATE;
				target.material_field[0].info.layer_field[0].cube_map = false;
				target.material_field[0].info.layer_field[0].clamp_x = false;
				target.material_field[0].info.layer_field[0].clamp_y = false;
				target.material_field[0].info.layer_field[0].source = texture != 0 ? resource->retrieve(texture) : NULL;
				target.material_field[0].info.layer_field[0].source_name = texture;
			}

			index_t offset = (texture!=0);
			if (normal_texture != 0)
			{
				target.material_field[0].info.layer_field[offset].combiner = GL_MODULATE;
				target.material_field[0].info.layer_field[offset].cube_map = false;
				target.material_field[0].info.layer_field[offset].clamp_x = false;
				target.material_field[0].info.layer_field[offset].clamp_y = false;
				target.material_field[0].info.layer_field[offset].source = resource->retrieve(normal_texture);
				target.material_field[0].info.layer_field[offset].source_name = normal_texture;
				offset++;
			}
		}


	
		static Container::PreAllocVector<CGS::RenderObjectA<>,0>	robjs;
		robjs.reset();


		for (index_t i = 0; i < lods.Count(); i++)
		{
			CGS::RenderObjectA<>&robj = robjs.Append();
			robj.target = target.object_field.pointer();
			robj.tname = target.object_field.pointer()->name;
			robj.detail = (unsigned)i;


			const SurfaceDescription&hull = lods[i];
			bool has_texcoords = (texture != 0 || normal_texture != 0);
			robj.vpool.SetSize(hull.vertices.Count(),has_texcoords,CGS::HasNormalFlag |CGS::HasTangentFlag);
			//ASSERT_EQUAL__(robj.vpool.vsize(),11 + 2*...yadda, you get idea);
			float*vtx = robj.vpool.vdata.pointer();
			const int vsize = 9 + 2*has_texcoords;
			Concurrency::parallel_for(index_t(0), hull.vertices.GetLength(),[vtx,&hull,vsize,has_texcoords](index_t i)
			{
				float*v = vtx + vsize*i;
				M::Vec::ref3(v) = hull.vertices[i].position;
				M::Vec::ref3(v+3) = hull.vertices[i].normal;
				M::Vec::ref3(v+6) = hull.vertices[i].tangent;
				if (has_texcoords)
					M::Vec::ref2(v+9) = hull.vertices[i].tcoord;
			});
			/*
			for (index_t i = 0; i < hull.vertices.GetLength(); i++)
			{
				M::Vec::ref3(vtx) = hull.vertices[i].position;
				M::Vec::ref3(vtx+3) = hull.vertices[i].normal;
				M::Vec::ref3(vtx+6) = hull.vertices[i].tangent;
				M::Vec::ref2(vtx+9) = hull.vertices[i].tcoord;
				vtx += 11;
			}*/
			ASSERT__(!(hull.quadIndices.Count()%4));
			ASSERT__(!(hull.triangleIndices.Count()%3));
			robj.ipool.SetSize(UINT32(hull.triangleIndices.Count()/3),UINT32(hull.quadIndices.Count()/4));
			memcpy(robj.ipool.idata.pointer(),hull.triangleIndices.pointer(),hull.triangleIndices.Count()*sizeof(UINT32));
			memcpy(robj.ipool.idata.pointer()+hull.triangleIndices.Count(),hull.quadIndices.pointer(),hull.quadIndices.Count()*sizeof(UINT32));
			//robj.ipool.idata.copyFrom
				//copyFrom(iout.pointer(),iout.GetLength());
		}


		target.material_field[0].data.object_field.SetSize(robjs.Count());
		for (index_t i = 0; i < robjs.Count(); i++)
			target.material_field[0].data.object_field[i].adoptData(robjs[i]);

		target.material_field[0].data.coord_layers = 1;//+ (normal_texture != 0);
		//target.material_field[0].data.dimensions = 3;
	
		//const CGS::RenderObjectA<>&robj = target.material_field[0].data.object_field.First();
		CGS::SubGeometryA<>&obj = target.object_field.First();
		obj.vs_hull_field.SetSize(lods.Count());
		for (index_t i = 0; i < obj.vs_hull_field.GetLength(); i++)
		{
			Mesh<CGS::SubGeometryA<>::VsDef>&vs_hull = obj.vs_hull_field[i];
			const SurfaceDescription&hull = lods[i];
			if (/*export_visual_sub_hulls ||*/ !i)
				convertDescriptionToHull(hull,vs_hull);
			else
				vs_hull.clear();
		}

		//convertDescriptionToHull(physical_hull,obj.phHull);
	
		obj.meta.shortest_edge_length = shortest_edge*0.2f;
		//ShowMessage("generated "+String(robjs.Count())+" sub defail level(s)");
	
		M::Mat::Eye(target.object_field[0].meta.system);
		M::Vec::clear(target.object_field[0].meta.center);
		target.object_field[0].meta.radius = 1;
		target.object_field[0].meta.volume = 1;
		target.object_field[0].meta.density = 1;
		M::Mat::Eye(target.object_field[0].path);
		target.object_field[0].system_link = &target.object_field[0].path;

		target.connector_field.free();
	
		//return robjs.Count();
	}



	/*static*/ void SurfaceNetwork::CompileFromDescriptions(CGS::Geometry<>&target, const Ctr::Array<SurfaceDescription>&lods, const SurfaceDescription&phHull, float shortest_edge, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource /*=NULL*/)
	{
		target.object_field.SetSize(1);
		target.object_field[0].name = String(str2name("object"));
	
		target.material_field.SetSize(1);
		//target.material_field[0].data.object_field.Resize(detail_levels);
		M::Vec::set(target.material_field[0].info.specular,0.7);
		//target.material_field[0].info.shininess = 0;

		if (!resource)
			resource = &GraphMesh::texture_resource;

		target.texture_resource = resource;
	
		//if (texture != 0)
		{
			M::Vec::def(target.material_field[0].info.ambient,0.4,0.4,0.4,1);
			target.material_field[0].info.attachment.reset();
			target.material_field[0].info.layer_field.SetSize((texture!=0) + (normal_texture!=0));
			if (texture != 0)
			{
				target.material_field[0].info.layer_field[0].combiner = GL_MODULATE;
				target.material_field[0].info.layer_field[0].cube_map = false;
				target.material_field[0].info.layer_field[0].clamp_x = false;
				target.material_field[0].info.layer_field[0].clamp_y = false;
				target.material_field[0].info.layer_field[0].source = texture != 0 ? resource->retrieve(texture) : NULL;
				target.material_field[0].info.layer_field[0].source_name = texture;
			}

			index_t offset = (texture!=0);
			if (normal_texture != 0)
			{
				target.material_field[0].info.layer_field[offset].combiner = GL_MODULATE;
				target.material_field[0].info.layer_field[offset].cube_map = false;
				target.material_field[0].info.layer_field[offset].clamp_x = false;
				target.material_field[0].info.layer_field[offset].clamp_y = false;
				target.material_field[0].info.layer_field[offset].source = resource->retrieve(normal_texture);
				target.material_field[0].info.layer_field[offset].source_name = normal_texture;
				offset++;
			}
		}


	
		static Container::PreAllocVector<CGS::RenderObjectA<>,0>	robjs;
		robjs.reset();


		for (index_t i = 0; i < lods.Count(); i++)
		{
			CGS::RenderObjectA<>&robj = robjs.Append();
			robj.target = target.object_field.pointer();
			robj.tname = target.object_field.pointer()->name;
			robj.detail = (unsigned)i;


			const SurfaceDescription&hull = lods[i];
			bool has_texcoords = (texture != 0 || normal_texture != 0);
			robj.vpool.SetSize(hull.vertices.Count(),has_texcoords,CGS::HasNormalFlag |CGS::HasTangentFlag);
			//ASSERT_EQUAL__(robj.vpool.vsize(),11 + 2*...yadda, you get idea);
			float*vtx = robj.vpool.vdata.pointer();
			const int vsize = 9 + 2*has_texcoords;
			Concurrency::parallel_for(index_t(0), hull.vertices.GetLength(),[vtx,&hull,vsize,has_texcoords](index_t i)
			{
				float*v = vtx + vsize*i;
				M::Vec::ref3(v) = hull.vertices[i].position;
				M::Vec::ref3(v+3) = hull.vertices[i].normal;
				M::Vec::ref3(v+6) = hull.vertices[i].tangent;
				if (has_texcoords)
					M::Vec::ref2(v+9) = hull.vertices[i].tcoord;
			});
			/*
			for (index_t i = 0; i < hull.vertices.GetLength(); i++)
			{
				M::Vec::ref3(vtx) = hull.vertices[i].position;
				M::Vec::ref3(vtx+3) = hull.vertices[i].normal;
				M::Vec::ref3(vtx+6) = hull.vertices[i].tangent;
				M::Vec::ref2(vtx+9) = hull.vertices[i].tcoord;
				vtx += 11;
			}*/
			ASSERT__(!(hull.quadIndices.Count()%4));
			ASSERT__(!(hull.triangleIndices.Count()%3));
			robj.ipool.SetSize(UINT32(hull.triangleIndices.Count()/3),UINT32(hull.quadIndices.Count()/4));
			memcpy(robj.ipool.idata.pointer(),hull.triangleIndices.pointer(),hull.triangleIndices.Count()*sizeof(UINT32));
			memcpy(robj.ipool.idata.pointer()+hull.triangleIndices.Count(),hull.quadIndices.pointer(),hull.quadIndices.Count()*sizeof(UINT32));
			//robj.ipool.idata.copyFrom
				//copyFrom(iout.pointer(),iout.GetLength());
		}


		target.material_field[0].data.object_field.SetSize(robjs.Count());
		for (index_t i = 0; i < robjs.Count(); i++)
			target.material_field[0].data.object_field[i].adoptData(robjs[i]);

		target.material_field[0].data.coord_layers = 1;//+ (normal_texture != 0);
		//target.material_field[0].data.dimensions = 3;
	
		//const CGS::RenderObjectA<>&robj = target.material_field[0].data.object_field.First();
		CGS::SubGeometryA<>&obj = target.object_field.First();
		obj.vs_hull_field.SetSize(lods.Count());
		for (index_t i = 0; i < obj.vs_hull_field.GetLength(); i++)
		{
			Mesh<CGS::SubGeometryA<>::VsDef>&vs_hull = obj.vs_hull_field[i];
			const SurfaceDescription&hull = lods[i];
			if (/*export_visual_sub_hulls ||*/ !i)
				convertDescriptionToHull(hull,vs_hull);
			else
				vs_hull.clear();
		}

		convertDescriptionToHull(phHull,obj.phHull);
	
		obj.meta.shortest_edge_length = shortest_edge*0.2f;
		//ShowMessage("generated "+String(robjs.Count())+" sub defail level(s)");
	
		M::Mat::Eye(target.object_field[0].meta.system);
		M::Vec::clear(target.object_field[0].meta.center);
		target.object_field[0].meta.radius = 1;
		target.object_field[0].meta.volume = 1;
		target.object_field[0].meta.density = 1;
		M::Mat::Eye(target.object_field[0].path);
		target.object_field[0].system_link = &target.object_field[0].path;

		target.connector_field.free();
	
		//return robjs.Count();
	}


	index_t							SurfaceNetwork::FindSegment(const Segment::Connector&slot0, const Segment::Connector&slot1)	const
	{
		const Node	*n0 = nodes.queryPointer(slot0.node),
					*n1 = nodes.queryPointer(slot1.node);
		if (n0 == n1 || !n0 || !n1)
			return InvalidIndex;
		if (slot0.subdivisionStep >= n0->segments[slot0.outbound].Count())
			return InvalidIndex;
		if (slot1.subdivisionStep >= n1->segments[slot1.outbound].Count())
			return InvalidIndex;
		if (n0->segments[slot0.outbound][slot0.subdivisionStep] == n1->segments[slot1.outbound][slot1.subdivisionStep])
			return n0->segments[slot0.outbound][slot0.subdivisionStep];
		return InvalidIndex;
	}


	void							SurfaceNetwork::Unlink(index_t segment_index)
	{
		if (segment_index == InvalidIndex)
			return;
		Segment*seg = segments.queryPointer(segment_index);
		DBG_ASSERT_NOT_NULL__(seg);
		if (!seg)
			return;

		for (BYTE k = 0; k < 2; k++)
		{
			const Segment::Connector&c = seg->connector[k];
			Node*n = nodes.queryPointer(c.node);
			ASSERT_NOT_NULL__(n);
			ASSERT_LESS__(c.subdivisionStep,n->segments[c.outbound].Count());
			n->segments[c.outbound][c.subdivisionStep] = InvalidIndex;
		}

		segments.Unset(segment_index);
	}

	void							SurfaceNetwork::ManagedUnlink(index_t segment_index,bool update_controls, bool build, Container::IndexSet*affectedSegments)
	{
		if (segment_index == InvalidIndex)
			return;
		Segment*seg = segments.queryPointer(segment_index);
		DBG_ASSERT_NOT_NULL__(seg);
		if (!seg)
			return;

		Node*nodes_[2];
		for (BYTE k = 0; k < 2; k++)
		{
			const Segment::Connector&c = seg->connector[k];
			Node*n = nodes_[k] = nodes.queryPointer(c.node);
			ASSERT_NOT_NULL__(n);
			ASSERT_LESS__(c.subdivisionStep,n->segments[c.outbound].Count());

			n->segments[c.outbound].Erase(c.subdivisionStep);
			for (index_t i = c.subdivisionStep; i < n->segments[c.outbound].Count(); i++)
				if (n->segments[c.outbound][i] != InvalidIndex)
				{
					Segment*seg = segments.queryPointer(n->segments[c.outbound][i]);
					ASSERT_NOT_NULL__(seg);
					seg->connector[seg->GetEndIndex(c.node)].subdivisionStep--;
				}
			n->subdivision[c.outbound].SetSize(n->subdivision[c.outbound].GetLength()-1);
			MakeEven(n->subdivision[c.outbound]);
		}
		segments.Unset(segment_index);

		UpdateSlopes(*nodes_[0],update_controls,build,affectedSegments);
		UpdateSlopes(*nodes_[1],update_controls,build,affectedSegments);
	}

}

//Image				GraphMesh::texture;
//CGS::TextureA		GraphMesh::loaded_texture;
