#ifndef graph_meshH
#define graph_meshH

#include "../container/buffer.h"
#include "../math/vector.h"
#include "../io/xml.h"
#include "../io/texture_compression.h"
#include "cgs.h"
#include "cgs_track_segment.h"
#include "converter/obj.h"
#include "vertex_table.h"


#define CIRCLE_CONTROL_FACTOR		0.39052429175126996746554085052687			//(4*(sqrt(2)-1)/3) / sqrt(2)
#define COMMON_CONTROL_FACTOR		0.3


namespace DeltaWorks
{
	inline	void	decValidIndex(index_t&index, index_t threshold)
	{
		if (index >= threshold && index != InvalidIndex)
			index--;
	}



	class SurfaceDescription
	{
	public:
		struct TVertex
		{
			float3				position,
									normal,
									tangent;	//!< Vertex tangent, perpendicular to the track's direction, and the normal (pointing to the right, when looking along track)
			float2				tcoord;
			float					tx;			//!< In the range [0,1], describes the t-position from the left to the right edge of the track
		};

		struct TControl
		{
			static const count_t	NumFloats = 15;
			union
			{
				struct
				{
					M::TVec3<>			position,
									direction,
									up;				//!< Up axis. Must be normalized and orthogonal to @a direction
					M::TVec2<>			scale;
					float			angle0,
									angle1,
									texcoord0,
									texcoord1;
				};
				float				field[NumFloats];
			};
		};

		struct InterpolatedSlice : public TControl
		{
			bool					buildLocal;
			float3					right;
			float					t,
									texcoordY;

			/**/					InterpolatedSlice():buildLocal(false)	{}
			/**/					InterpolatedSlice(const TControl&slice, float t_):TControl(slice),t(t_),buildLocal(false){M::Vec::cross(direction,up,right);}

			void					MakeVertex(float x, TVertex&vtx)	const;
			void					MakePoint(float x, M::TVec3<>&pt)	const;
			void					MakePoint(float x, M::TVec3<>&pt, M::TVec3<>&normal)	const;
			void					MakePoint(float x, M::TVec3<>&pt, M::TVec3<>&normal, M::TVec3<>&tangent)	const;
			count_t					CalculateSteps(float tolerance0,float tolerance1)	const;
			count_t					CalculateSteps(float tolerance)	const;
			float					EstimateLength()	const;
			bool					IsClosedLoop()		const;
			float					GetTexExtend()		const;

		};

		struct TConnector
		{
			TControl				state,
									slope;
		};

		typedef Container::Buffer<UINT32,0>				IndexContainer;
		typedef Container::Buffer<TVertex,0,Primitive>		VertexContainer;
		typedef Container::BasicBuffer<TVertex,Primitive>	OutVertexContainer;

		VertexContainer				vertices;
		IndexContainer				triangleIndices,
									quadIndices;

		struct Edge : public IndexContainer
		{
			enum direction_t
			{
				Node0,
				Node1,
				Left,
				Right
			};

			direction_t				direction;
		};
		Container::Buffer<Edge,0>			edges;
		Container::Buffer<TConnector,0>		subConnectors;

		unsigned					textureRepetitions;


		void						swap(SurfaceDescription&other)
									{
										vertices.swap(other.vertices);
										triangleIndices.swap(other.triangleIndices);
										quadIndices.swap(other.quadIndices);
										edges.swap(other.edges);
										subConnectors.swap(other.subConnectors);
										swp(textureRepetitions,other.textureRepetitions);
									}
		void						Compact()
									{
										vertices.compact();
										triangleIndices.compact();
										quadIndices.compact();
										edges.compact();
										subConnectors.compact();
									}
		void						Clear()
									{
										vertices.clear();
										triangleIndices.clear();
										quadIndices.clear();
										edges.clear();
										subConnectors.clear();
										textureRepetitions = 1;
									}

		void						GenerateNormals();

		void						BuildSegment(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end,const SurfaceDescription::TControl control_points[2], float tolerance0, float tolerance1);
		void						BuildArc(const OutVertexContainer&arc_vertices, float near_distance, float far_distance, float extend_along_track);
		void						BuildRails(const SurfaceDescription&source, const Container::BasicBuffer<float2>&profile, const M::TVec3<>&relativeTo);
		void						BuildRails(const SurfaceDescription&source, float innerExtend, float outerExtend, float upperExtend, float lowerExtend, const M::TVec3<>&relativeTo);
		void						BuildBarriers(const SurfaceDescription&source, float barrierPosition, float barrierHeight0, float barrierHeight1, const M::TVec3<>&relativeTo);
		float3						GetEdgeCenter()	const;
		void						GetEdgeExtend(const M::TMatrix4<>&transformBy, M::Box<>&result)	const;

		static void					Interpolate(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end,const SurfaceDescription::TControl control_points[2], float t, InterpolatedSlice&out);
		static void					BuildControlPoints(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end,SurfaceDescription::TControl control_points[2], float control_factor0=COMMON_CONTROL_FACTOR, float control_factor1=COMMON_CONTROL_FACTOR);
		static void					UpdateSlope(const TControl&predecessor, const TControl&successor, bool smooth_direction, TConnector&slice);
		static void					UpdateRightSlope(const TControl&predecessor, bool smooth_direction, TConnector&slice);
		static void					UpdateLeftSlope(const TControl&successor, bool smooth_direction, TConnector&slice);
		/**
		@brief Generates vertices along the track arc surrounding the specified control. The number of generated vertices depends on the specified tolerance and the curvature of the local node
		*/
		static void					BuildArc(OutVertexContainer&arc_out, const TControl&control, float tolerance, bool flipped, bool global);
	private:
		static void					Subdivide(const SurfaceDescription::TConnector&begin, const SurfaceDescription::TConnector&end,const SurfaceDescription::TControl control_points[2], float tolerance, const InterpolatedSlice&ref0, const InterpolatedSlice&ref1, float t0, float t1, Container::Buffer<InterpolatedSlice,256,POD>&outIntersections, int force);
		static void					BuildArcFromSlice(OutVertexContainer&arc_out, InterpolatedSlice&slice, float tolerance, bool flipped);

	};
}
DECLARE__(SurfaceDescription,Swappable);
DECLARE__(SurfaceDescription::TVertex,POD);

namespace DeltaWorks
{
	class SurfaceNetwork
	{
	public:
		template <typename Adoptable>
			class GenericContainer : public Container::IndexTable<Adoptable,AdoptStrategy>
			{
				typedef Container::IndexTable<Adoptable, AdoptStrategy>	Super;

			private:
				index_t				counter;
			public:
				using Super::setNew;
				using Super::queryPointer;


				typedef typename Super::DataType	DataType;

				/**/				GenericContainer():counter(0) {}
				DataType&			Create(index_t&index)
									{
										DataType*rs;
										while (counter == InvalidIndex || (rs = setNew(counter))==NULL)
											counter++;
										index = counter++;
										DBG_ASSERT_EQUAL__(rs,queryPointer(index));
										return *rs;
									}
			};

		struct Attachment
		{
			virtual	~Attachment()	{};
		};
		typedef std::shared_ptr<Attachment>		PAttachment;


		struct Node : public SurfaceDescription::TControl
		{
			Ctr::Array<float>					subdivision[2];	//!< Describes the subdivision steps for outbound segments. Only contains borders between each two steps. All must be in the range (0,1), pairwise different, and ascending in order.
			Ctr::Array<index_t>					segments[2];		//!< Index of the segment bound to each subdivision. This field must be one element larger than subdivision
			SurfaceDescription::TControl	slope;
			UINT32							lodIndex;			//!< Strictly client variable that is merely provided but not used by the local structures
			index_t							nodeID;			//!< Temporary variable used during compactification. Not used otherwise
			PAttachment						attachment;		//!< Custom attachment. Initialized with NULL, and moved if the container is resized, but not deleted automatically

			/**/							Node():lodIndex(0),nodeID(InvalidIndex)
											{
												segments[0].SetSize(1);
												segments[1].SetSize(1);

												segments[0].First() = InvalidIndex;
												segments[1].First() = InvalidIndex;

												angle0 = -0.25f;
												angle1 = 0.25f;
												M::Vec::def(direction,1,0,0);
												M::Vec::clear(position);
												M::Vec::set(scale,1);
												texcoord0 = 0;
												texcoord1 = 1;
												M::Vec::def(up,0,1,0);
											}

			void							adoptData(Node&other)
											{
												((SurfaceDescription::TControl&)*this) = other;
												subdivision[0].adoptData(other.subdivision[0]);
												subdivision[1].adoptData(other.subdivision[1]);
												segments[0].adoptData(other.segments[0]);
												segments[1].adoptData(other.segments[1]);

												slope = other.slope;
												lodIndex = other.lodIndex;
												nodeID = other.nodeID;

												attachment.swap(other.attachment);
											}
			Node&							operator=(const SurfaceDescription::TControl&control)
											{
												((SurfaceDescription::TControl&)*this) = control;
												return *this;
											}

			bool							IsAvailable(bool outbound, index_t subdiv)	const
											{
												return subdiv < segments[outbound].Count() && segments[outbound][subdiv] == InvalidIndex;
											}
			index_t							GetFirstAvailable(bool outbound) const
											{
												for (index_t i = 0; i < segments[outbound].Count(); i++)
													if (segments[outbound][i] == InvalidIndex)
														return i;
												return InvalidIndex;
											}
			bool							AnyAvailable(bool outbound)	const	{return GetFirstAvailable(outbound) != InvalidIndex;}

			bool							BuildConnector(bool outbound, index_t subdiv, bool second_node, SurfaceDescription::TConnector&out_connector)	const;
			bool							BuildState(bool outbound, index_t subdiv, bool second_node, SurfaceDescription::TControl&out_state)	const;

			void							BuildArc(Container::BasicBuffer<SurfaceDescription::TVertex>&arc_out, index_t lod, bool flip, bool global)	const;
			Node&							SetPosition(float x,float y,float z)	{M::Vec::def(position,x,y,z); return *this;}
			Node&							SetDirection(float x,float y,float z)	{M::Vec::def(direction,x,y,z); M::Vec::normalize(direction); return *this;}
			Node&							SetUpAxis(float x, float y, float z)	{M::Vec::def(up,x,y,z); M::Vec::normalize(up); return *this;}
			Node&							SetTexCoords(float x0, float x1)		{texcoord0 = x0; texcoord1 = x1; return *this;}
			Node&							SetAngles(float a0, float a1)			{angle0 = a0; angle1 = a1; return *this;}
			Node&							SetScale(float scale_x, float scale_y)	{M::Vec::def(scale,scale_x,scale_y); return *this;}
			Node&							SplitEvenly(bool outbound, count_t num);
		};



		struct Segment
		{
			struct Connector
			{
				index_t						node,
											subdivisionStep;	//!< Index of the specified node's subdivision
				float						controlFactor;
				bool						outbound;			//!< True if this connector is mounted to the outbound side of the targeted node, false otherwise

				/**/						Connector():node(InvalidIndex),subdivisionStep(0),controlFactor(COMMON_CONTROL_FACTOR),outbound(false){}
				/**/						Connector(index_t node_, index_t subdivision_step_, bool outbound_):node(node_),subdivisionStep(subdivision_step_),controlFactor(COMMON_CONTROL_FACTOR),outbound(outbound_){}
			};

			SurfaceDescription::TControl	controls[2];
			Connector						connector[2];
			Ctr::Array<SurfaceDescription>		compiledSurfaces;	//three per lod: 0: stitched to fit lower resolution node[0], 1: stitched to fit lower resolution node[1], 2: not stitched (in case both nodes have reduced lod, use less detailed surface)

			PAttachment						attachment;		//!< Custom attachment. Initialized with NULL, and moved if the container is resized, but not deleted automatically
			index_t							segID;				//!< Temporary variable used during compactification. Not used otherwise, but valid afterwards

			/**/							Segment():segID(InvalidIndex){}

			void							adoptData(Segment&other)
											{
												controls[0] = other.controls[0];
												controls[1] = other.controls[1];
												connector[0] = other.connector[0];
												connector[1] = other.connector[1];
												compiledSurfaces.adoptData(other.compiledSurfaces);
												segID = other.segID;

												attachment.swap(other.attachment);
											}
			int								GetEndIndex(index_t nodeIndex)	const
											{
												if (connector[0].node == nodeIndex)
													return 0;
												if (connector[1].node == nodeIndex)
													return 1;
												FATAL__("Integrity violated");
												return 0;
											}
			float							GetMinimalDistanceTo(const M::TVec3<>&referencePointer, count_t scanResolution=10)	const;
		};


		struct Compacted
		{
			Ctr::Array<Node>						nodes;
			Ctr::Array<Segment>					segments;
			Ctr::Array<bool>						nodeIsFlipped;

			void							adoptData(Compacted&other)
											{
												nodes.adoptData(other.nodes);
												segments.adoptData(other.segments);
												nodeIsFlipped.adoptData(other.nodeIsFlipped);
											}
		};
		//typedef Compacted Compact;

		GenericContainer<Node>				nodes;
		GenericContainer<Segment>			segments;

		static count_t						numLODs;
		static float						minTolerance;


		void								swap(SurfaceNetwork&other)
											{						
												nodes.swap(other.nodes);
												segments.swap(other.segments);
											}
		void								adoptData(SurfaceNetwork&other)
											{
												nodes.adoptData(other.nodes);
												segments.adoptData(other.segments);
											}
		
		index_t								Link(const Segment::Connector&slot0, const Segment::Connector&slot1, bool updateControls, bool build);
		/**
		@brief Attempts to create an optimal connection between the two specified nodes

		@param node0 Node index to connect from
		@param node1 Node index to connect to
		@param manage_slots Set true to automatically adjust node slot count and occupation as needed. Compares distances.
		@return Index of the new connection, or InvalidIndex, if creation was impossible
		*/
		index_t								SmartLink(index_t node0, index_t node1, bool manageSlots,bool updateControls, bool build, Container::IndexSet*affectedSegments=NULL);
		index_t								FindSegment(const Segment::Connector&slot0, const Segment::Connector&slot1)	const;
		void								Unlink(index_t segmentIndex);
		void								ManagedUnlink(index_t segmentIndex,bool updateControls, bool build, Container::IndexSet*affectedSegments = NULL);
		void								ManagedRemoveNode(index_t nodeIndex, bool updateConnectedSlopes, bool updateConnectedSegmentControls, bool build, Container::BasicBuffer<std::pair<index_t,PAttachment> >*removedSegments, Container::IndexSet*affectedSegments=NULL);
		/**
		@brief Copies all local data into a more compact form with no unused memory inbetwen
		*/
		void								Compact(Compacted&target);
		/**
		@brief Similar to the above but uses adoptData() were appropriate
		*/
		void								MoveCompact(Compacted&target);

		void								UpdateSlopes(index_t node, bool updateConnectedSegmentControls, bool rebuildSegments, Container::IndexSet*affectedSegments=NULL);
		void								UpdateSlopes(Node&node, bool updateConnectedSegmentControls, bool rebuildSegments, Container::IndexSet*affectedSegments=NULL);
		void								UpdateControls(index_t segment, bool rebuild);
		void								UpdateControls(Segment&segment, bool rebuild);
		void								RebuildSegment(index_t segment);
		void								RebuildSegment(Segment&segment);
		void								UpdateAllSlopes(bool updateAllControls, bool rebuildSegments);

		void								VerifyIntegrity()	const;

		bool								NodeIsFlipped(index_t node)	const;

		static void							MakeEven(Ctr::Array<float>&subdivison_field);

		static void							CompileBarrierGeometry(CGS::Geometry<>&target,const Segment&segment, float position,float height0,float height1,name64_t texture, CGS::TextureResource*resource=NULL);
		static void							CompileArcGeometry(CGS::Geometry<>&target, const Node&node, float near_distance, float far_distance, float extend_along_track, name64_t texture, name64_t normal_texture, bool node_is_flipped, CGS::TextureResource*resource=NULL);
		static void							CompileRailGeometry(CGS::Geometry<>&target, const Segment&segment, float innerExtend, float outerExtend, float upperExtend, float lowerExtend, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource=NULL);
		static void							CompileRailGeometry(CGS::Geometry<>&target, const Segment&segment, const Container::BasicBuffer<float2>&profile, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource=NULL);
		static void							CompileFromDescriptions(CGS::Geometry<>&target, const Ctr::Array<SurfaceDescription>&lods, float shortest_edge, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource =NULL);
		static void							CompileFromDescriptions(CGS::Geometry<>&target, const Ctr::Array<SurfaceDescription>&lods, const SurfaceDescription&phHull, float shortest_edge, name64_t texture, name64_t normal_texture, CGS::TextureResource*resource =NULL);

	private:
		index_t								SmartMakeRoom(Node&node,index_t node_index,bool outbound,const Node&opposing_node);

	};




	class GraphNode
	{
	public:
		M::TVec2<>								position,
											direction;
		bool								processed,
											selected;
	};


	class LineShape
	{
	public:
		class Node
		{
		public:
			M::TVec2<>							position,
											normal;
		};
		
		Container::Buffer<Node>						nodes;
		
		
		void								adoptData(LineShape&other)
											{
												nodes.adoptData(other.nodes);
											}
	};





	class Outline
	{
	public:
		Container::Buffer0<LineShape,Adopt>	shapes;
		
		
		count_t					countNodes()	const
								{
									count_t rs = 0;
									for (index_t i = 0; i < shapes.Count(); i++)
										rs += shapes[i].nodes.Count();
									return rs;
								}
	};



	/**
	A profile defines a slice through through the graph track. The shape defined by a profile is always a closed loop

	*/
	class Profile
	{
	public:
		/**
		@brief single profile node
		*/
		class Node:public GraphNode
		{
		public:
				float		control_dist[2];	//!< control point distances, both in the range [0,1], along GraphNode::direction. control_dist[0] is used for connections to the predecessor, control_dist[1] to the successor
				M::TVec2<>		control[2];			//!< provided for convenience only. not used by the local system
				
				
				Node&		reset()
							{
								M::Vec::clear(position);
								M::Vec::def(direction,1,0);
								control_dist[0] = control_dist[1] = 0;
								processed = false;
								selected = false;
								return *this;
							}
				Node&		moveTo(float x, float y)
							{
								M::Vec::def(position,x,y);
								return *this;
							}
		};



		Container::Buffer<Node>		nodes;

		void				swap(Profile&other)
							{
								nodes.swap(other.nodes);
							}
		void				adoptData(Profile&other)
							{
								nodes.adoptData(other.nodes);
							}
		void				clearSelection()
							{
								for (auto it = nodes.begin(); it != nodes.end(); ++it)
									it->selected = false;
								//edges cannot be selected (yet)
							}
		void				clear(bool free_memory)
							{
								if (free_memory)
								{
									nodes.clear(0);
								}
								else
								{
									nodes.reset();
								}
							}

		void				readFrom(const XML::Node*xprofile);
	};

	class GraphMesh;

	class Graph
	{
	public:
		enum layout_t
		{
			Plain,
			Split,
			Ramp,

			LayoutTypes

		};

		
		static const char*		ToString(layout_t);
		static bool				decode(const String&string, layout_t&);
	protected:
		static void				splitHull(SurfaceDescription&desc_out);
		static float			splitIntersectionFactor(const SurfaceDescription::TVertex&v0,const SurfaceDescription::TVertex&v1);
		static UINT32			insertInterpolatedVertex(Container::Buffer<SurfaceDescription::TVertex,0,POD>&vout,UINT32 v0,UINT32 v1,float factor);
	protected:
		
		bool					defineSurface(const Profile::Node&p0,const Profile::Node&p1,index_t edge_index,float x_step, float z_step, float texture_size_x, float texture_size_y,SurfaceDescription&desc_out, layout_t layout, float simplify_x)	const;
			//bool				defineProfileLessSurface(index_t edge_index,float step,float texture_size,Buffer<TVertex>&vout,Buffer<UINT32>&iout, layout_t layout)	const;
		
		friend class GraphMesh;
	public:
		static void				splitBend(SurfaceDescription::TVertex&vtx);
		static void				ramp(SurfaceDescription::TVertex&vtx);

		class Node:public GraphNode
		{
		public:
			index_t		in_edge,
						out_edge;
			float		x_coord;
			
			void		reset()
			{
				in_edge = InvalidIndex;
				out_edge = InvalidIndex;
				processed = false;
				selected = false;
				x_coord = 0;
				M::Vec::def(direction,1,0);
			}
		};


		class Edge
		{
		public:
			index_t		node[2];
			float		control_dist[2];
			float		x_coord[2];


			//#ifdef GRAPH_EDIT
				M::TVec2<>	control[2];	//needed by graph editor
			//#endif
				
				
			void 		reset()
			{
				node[0] = node[1] = InvalidIndex;
				x_coord[0] = x_coord[1] = 0;
				control_dist[0] = control_dist[1] = 1;
			}

			inline	BYTE	GetIndexOf(index_t node_)	const
			{
				if (node_ == node[0])
					return 0;
				if (node_ == node[1])
					return 1;
				return 0xFF;
			}
		};
		



		Container::Buffer<Node>		nodes;
		Container::Buffer<Edge>		edges;
		

		void				swap(Graph&other)
							{
								nodes.swap(other.nodes);
								edges.swap(other.edges);
							}
		void				adoptData(Graph&other)
							{
								nodes.adoptData(other.nodes);
								edges.adoptData(other.edges);
							}

		void				clearSelection()
							{
								for (auto it = nodes.begin(); it != nodes.end(); ++it)
									it->selected = false;
								//edges cannot be selected (yet)
							}
		void				clear(bool free_memory)
							{
								if (free_memory)
								{
									nodes.clear(0);
									edges.clear(0);
								}
								else
								{
									nodes.reset();
									edges.reset();
								}
							}

		/**
			@brief Updates the edge controls of all edges

			Calling this method is only needed when using the local graph instance in some sort of editor, that needs those controls.
			Loading and building does neither write nor read to these controls.
		*/
		void				updateEdgeControls(float control_factor0=COMMON_CONTROL_FACTOR, float control_factor1=COMMON_CONTROL_FACTOR)
							{
								for (index_t i = 0; i < edges.Count(); i++)
								{
									Edge&edge = edges[i];
									const Graph::Node	&n0 = nodes[edge.node[0]],
														&n1 = nodes[edge.node[1]];
									float len = M::Vec::distance(n0.position,n1.position);

									M::Vec::mad(n0.position,n0.direction,len*control_factor0*edge.control_dist[0],edge.control[0]);
									M::Vec::mad(n1.position,n1.direction,-len*control_factor1*edge.control_dist[1],edge.control[1]);
								}
							}

		void				LoadFromFile(const PathString&filename,bool compactify=true);
		void				SaveToFile(const PathString&filename)	const;

		void				WriteTo(XML::Node&node)	const;
		void				readFrom(const XML::Node*xnodes, const XML::Node*xedges);
		
		
		void				verifyIntegrity(const TCodeLocation&location)	const;
		void				eraseEdge(index_t index);
		void				updateDirectionsAndXCoords();
		

	};

	class GraphTextureResource : public CGS::TextureResource
	{
	private:
		Container::GenericHashContainer<name64_t,CGS::TextureA>	container;

	public:
		void					Insert(const PathString&filename, name64_t as_name, bool bump_map, float bump_strength = 0.01f);

		virtual count_t			countEntries()
		{
			throw Except::Program::FunctionalityNotImplemented();
		}
		virtual CGS::TextureA*	entry(index_t index)
		{
			throw Except::Program::FunctionalityNotImplemented();
		}
								
		virtual CGS::TextureA*	retrieve(const name64_t&name)
		{
			return container.lookup(name);
		}
	};

	class GraphMesh
	{
	private:

	public:
		Profile					visual_profile,
								physical_profile;
		Graph					visual_graph,
								physical_graph;
		bool					has_caps;

		
		static	GraphTextureResource	texture_resource;


		/**/					GraphMesh():has_caps(false)
								{}

		void					adoptData(GraphMesh&other)
		{
			visual_profile.adoptData(other.visual_profile);
			visual_graph.adoptData(other.visual_graph);
			physical_profile.adoptData(other.physical_profile);
			physical_graph.adoptData(other.physical_graph);
			has_caps = other.has_caps;
		}
		void					swap(GraphMesh&other)
		{
			visual_profile.swap(other.visual_profile);
			visual_graph.swap(other.visual_graph);
			physical_profile.swap(other.physical_profile);
			physical_graph.swap(other.physical_graph);
			swp(has_caps,other.has_caps);
		}

		void					clearSelection()
		{
			visual_profile.clearSelection();
			physical_profile.clearSelection();
			visual_graph.clearSelection();
			physical_graph.clearSelection();
		}
		void					clear(bool free_memory)
		{
			physical_profile.clear(free_memory);
			visual_profile.clear(free_memory);
			physical_graph.clear(free_memory);
			visual_graph.clear(free_memory);
		}
		/**
		@brief Exports the local graph mesh definition to a GS geometry
		@param target GS geometry to write to
		@param step Sub division steps along bended edges. Straight edges use reduced sub-division
		@param texture_size Texture tile size
		@param split Set true to generate a split symmetric geometry, useful for splitting the track. Not made to be bent
		@param texture Texture name to use. Set 0 to not use any texture
		@param normal_texture Texture name to use as normal map. Set to 0 to not use any normal map
		@param glow_texture Texture to use as a glowing decal texture, that is not affected by lighting
		@param export_visual_sub_hulls Set true to export all visual hulls into to the CGS-geometry. Set false to only export physical and highest detail hull
		@return Number of detail levels created
		*/
		count_t				createGeometry(CGS::Geometry<>&target, float step, float texture_size_x, float texture_size_y, Graph::layout_t layout, name64_t texture, name64_t normal_texture, name64_t glow_texture, bool export_visual_sub_hulls)	const;


		void				LoadFromFile(const PathString&filename,bool compactify=true);
		void				SaveToFile(const PathString&filename)	const;


		static void			buildStub(Graph::layout_t layout, float step, SurfaceDescription&desc);

	};
}


#endif
