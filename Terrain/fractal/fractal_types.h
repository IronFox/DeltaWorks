#pragma once

/*
	This module implements the more basic data structures required by the fractal interface in order to communicate with the kernel
*/

#include <container/array.h>
#include <container/aligned_array.h>
#include <engine/gl/buffer_object.h>
#include <engine/gl/texture.h>
#include <math/sse.h>
#include <geometry/coordinates.h>
#include <math/vector.h>

#include "kernel/config.h"

namespace DeltaWorks::Fractal
{
#if 0
	/**
		@brief Fractal vertex channels
	*/
	namespace Channel
	{
		enum eChannels
		{
			C0,
			C1,
			C2,
			C3,
			Oceanic,
			//Weight,
			Water,
			
			Count
		};
	}
#endif
	
	#undef VMOD
	#define VMOD	//volatile
	
	struct TChannelConfig
	{
		float	c0,		//!< General purpose channel #0
				c1,		//!< General purpose channel #0
				c2,		//!< General purpose channel #0
				c3,		//!< General purpose channel #0
				oceanic,//!< Oceanic noise channel
				//weight,
				water;	//!< Fractal water channel
		
		VMOD TChannelConfig&	operator=(const VMOD TChannelConfig&c)	VMOD
						{
							c0 = c.c0;
							c1 = c.c1;
							c2 = c.c2;
							c3 = c.c3;
							oceanic = c.oceanic;
							//weight = c.weight;
							water = c.water;
							return *this;
						}
	};
	
	/**
		@brief Fractal vertex structure
	*/
	struct TVertex
	{
		TChannelConfig	channel;		//!< Vertex channel configuration
		M::TVec3<>			position;		//!< Vertex position in segment space
		float			height;			//!< Vertex height in the range [-1,1]
		M::TVec3<>			normal;			//!< Vertex normal (not normalized)
						
						#define FRACTAL_HEIGHT	height
			
			
		VMOD TVertex&	operator=(const VMOD TVertex&v)	VMOD
						{
							channel = v.channel;
							position = v.position;
							height = v.height;
							normal = v.normal;
							return *this;
						}
							
							
	};
	
	#ifndef SSE_VECTOR	//SSE_VECTOR is a wrapper class type for __m128 for certain compilers, a direct definition of __m128 for others. Not a switch. Double check that it exists
		#error SSE_VECTOR undefined
	#endif
	
	/**
		@brief Retrieves the x-coordinate of a vertex position vector

		Technically no longer required since SSE position vectors have been deprecated. Still referenced though

		@param vector Vector to retrieve the x-coordinate of
		@return vector[0]
	*/
	inline float xCoord(const M::TVec3<>&vector)
	{
		return vector.x;
	}
	
	/**
		@brief Retrieves the x-coordinate of a vertex position vector

		Technically no longer required since SSE position vectors have been deprecated. Still referenced though

		@param vector Vector to retrieve the x-coordinate of
		@return vector[0]
	*/
	inline float xCoord(const volatile M::TVec3<>&vector)
	{
		return vector.x;
	}
	
	
	/**
		@brief River node used to describe a singular river vertex. Not in use
	*/
	struct TRiverNode
	{
		float			x,		//!< Grid x coordinate
						y,		//!< Grid y coordinate
						width,	//!< Node radius
						depth;	//!< Central node height
	};
	
	
	/**
		@brief Crater definition structure.
		
		The crater is interpreted as a cylinder that extends from the body core infinitely into one direction
	*/
	struct TCrater
	{
		M::TVec3<>		orientation,	//!< Normalized crater orientation vector
						base;			//!< Crater base point
		float			radius,			//!< Crater radius
						depth;			//!< Crater depth. This would usually relate to the radius but is not required to do so.


		int				CompareTo(const TCrater& other) const
		{
			if (radius > other.radius)
				return 1;
			if (radius < other.radius)
				return -1;
			return 0;


		}
	};
	
	/**
		@brief SSE compiled crater for faster SSE processing

		All four SSE components of each member hold the exact same value.
		See TCrater for details on variables.
	*/
	struct TSSECrater
	{
		M::TVec3<SSE_VECTOR>	orientation,
							base;
		SSE_VECTOR			radius,
							depth;
	};
	
	/**
		@brief Kernel ray intersection helper structure
	*/
	struct TBaseRayIntersection
	{
		bool			isSet;		//!< true if an intersection was detected, false otherwise
		const TVertex	*vertex[3];	//!< Corner vertices of the intersected triangle
		float			fc[3];		//!< Intersection factors (fc[0] <> vertex[0]->vertex[1]; fc[1] <> vertex[0]->vertex[2]; fc[2] <> ray)
	};
	
	/**
		@brief Kernel ground intersection helper structure
	*/
	struct TBaseGroundInfo
	{
		bool			isSet;						//!< true if the ground was detected, false otherwise
		float			height_over_ground,			//!< Height of the querying point over the detected ground as a factor of the direction vector used during detection
						ground_height;				//!< Ground height factor
		M::TVec3<>		normal,					//!< Interpolated normal
						position;				//!< Sector relative ground position
	};
	
	/**
		@brief Visual appearance of a vertex
	*/
	struct TVertexAppearance
	{
		M::TVec3<>			up,			//!< Normalized up axis from this vertex
						normal;		//!< Normalized ground normal
		float			snow_line,		//!< Snow coverage height barrier
						snow,			//!< Snow density
						water,			//!< Water availability (including snow proximity)
						sand,			//!< Sand coverage
						fertility,		//!< General fertility of this vertex [0,1]. This includes all below listed values
						temperature,	//!< General temperature of this vertex based on  sphere location (cold near poles, warm near equator) [0,1].
						planarity;		//!< Planarity of this vertex [0,1]. This value is included in fertility
	};
	
	/**
		@brief Surface cover vertex
		
		Can be used to put trees or buildings on vertices if appropriate
	*/
	struct TCoverVertex:public TVertexAppearance
	{
		TChannelConfig	channel;		//!< Base channel configuration of this vertex
		float			height;			//!< Vertex height factor
		M::TVec3<>			position;	//!< Sector relative vertex position
	};
	


	struct TBaseSurfaceSegment
	{
		#undef None
		enum Flags	//! Surface segment flags
		{
			None 					= 0x0000,	//!< Clear flags
			HasChildren				= 0x0001,	//!< surface patch has children
			EdgeMergedOffset		= 0x0002,	//!< Surface edge base flag (identical to Edge0Merged )
			Edge0Merged				= 0x0002,	//!< Edge to neighbor 0 has been merged in the past and need not be merged again (simply copied). This is identical to @b EdgeMergedOffset<<0
			Edge1Merged				= 0x0004,	//!< Edge to neighbor 1 has been merged in the past and need not be merged again (simply copied). This is identical to @b EdgeMergedOffset<<1
			Edge2Merged				= 0x0008,	//!< Edge to neighbor 2 has been merged in the past and need not be merged again (simply copied). This is identical to @b EdgeMergedOffset<<2
			HasData					= 0x0010,	//!< Fractal data has been generated
			RequiresUpdate			= 0x0020,	//!< Significant changes to the structure indicate that this surface should be updated
			RequiresTextureUpdate	= 0x0040,	//!< Only textures require an update
			SubdivLandscape			= 0x0080,	//!< Signals that the project() method should focus on segment children
			SubdivReflection		= 0x0100,	//!< Signals that the projectReflection() method should focus on segment children
			SubdivWater				= 0x0200,	//!< Signals that the projectWater() method should focus on segment children
			Edge0Open				= 0x0400,	//!< Edge to neighbor 0 is open and can never be merged. Identical to @b Edge0Open<<0
			Edge1Open				= 0x0800,	//!< Edge to neighbor 1 is open and can never be merged. Identical to @b Edge0Open<<1
			Edge2Open				= 0x1000,	//!< Edge to neighbor 2 is open and can never be merged. Identical to @b Edge0Open<<2
			AvailableEventTriggered	= 0x2000,	//!< Specifies that the onSegmentAvailable event has been triggered for this segment.
			ParentsProcessed		= 0x4000	//!< Indicates that parent vertex degeneration has been performed
		};
	
	
		M::TVec3<M::Composite::sector_t>	sector;		//!< Sector coordinates of this surface
		//TComposite		central_height;	//!< Absolute height of the center of this surface
		unsigned		depth;			//!< Face depth with 0 being the deepest possible face
		unsigned char	child_index;	//!< Index of this child as seen from its parent
		int				inner_int_seed,		//!< Seed used to generate inner vertex content
						outer_int_seed[3];	//!< Seed used to generate edge vertex content
		unsigned		flags;			//!< Various surface flags
	};
	
	
	struct TSurfaceSegment;
	
	/**
		@brief Link to a neighboring surface segment as seen from one surface segment

		Each surface segment contains three such link structures, one for each neighbor.
	*/
	struct TSurfaceSegmentLink
	{
		TSurfaceSegment				*segment;		//!< Link to the actual neighbor. May be NULL indicating that this neighbor does not exist
		BYTE						orientation;	//!< Neighbor orientation in the range [0,2]. Basically that is the index of the remote edge as seen from the containing surface segment
		bool						primary;		//!< Indicates that the segment that contains this link instance Owns this edge. The corresponding link structure of the respective neighbor must contain this variable negated
	
		TSurfaceSegmentLink():segment(NULL),orientation(0),primary(true)
		{}
	};
	
	/**
		@brief Surface segment struture

		This is the most that the fractal kernel knows about a surface segment.
	*/
	struct TSurfaceSegment:public TBaseSurfaceSegment
	{
		static const unsigned		floats_per_vbo_vertex=9;	//!< Total number of floats per compiled VBO geometry vertex

		Array<TVertex>				vertex_field;		//!< Raw device space vertex data
		Vector<TCrater>				crater_field;		//!< Applied crater field
		Engine::GLVertexBufferObject	vbo;				//!< Vertex buffer object containing renderable vertex floats. Each vertex provides 9 floats, the first 3 for the sector relative position, 1 for the height, 3 for the normal, the final 2 for texture coordinates
		Engine::GLTexture			texture;			//!< Color/normal texture assigned to this surface
		TSurfaceSegmentLink			neighbor_link[3];	//!< Links to adjacent host surfaces
		TSurfaceSegment				*parent;			//!< Links to parent segment. Should be initialized with NULL
		Array<Array<float>,Adopt>	vbo_edge[3];		//!< Compiled vertex data derived from parent data
	};
	
	
	

	/**
		@brief Operation context

		An instance of this struct must be aligned to 16 bytes. It provides the outlining fractal configuration necessary to perform
		a large variety of operations.

	*/
	struct TContext
	{
		SSE_VECTOR		sse_variance,	//!< @a variance in the form of a 4 component SSE vector
						sse_fertility,	//!< @a fertility in the form of a 4 component SSE vector
						sse_sector_size,//!< @a sector_size in the form of a 4 component SSE vector
						sse_temperature,//!< @a temperature in the form of a 4 component SSE vector
						sse_oceanic_noise_level;	//!< @a oceanic_noise_level in the form of a 4 component SSE vector
	
		//TComposite		base_height;	//!< Base radius of the sphere context
		float			base_heightf,			//!< Base radius of the sphere context as one absolute single precision float
						variance,				//!< Height range that is multiplied with a vertex's height to retrieve the final base-relative height
						sector_size,			//!< Base sector size of the environment
						noise_level,			//!< Fractal noise level. Usually around 1.0
						oceanic_noise_level,	//!< Additional oceanic noise factor applied during oceanic subdivision. Multiplied with @a noise_level when applied. Usually very small (<0.1)
						fertility,				//!< Planet fertility level in the range [0,1]. Usually initialized with 1
						temperature,			//!< Planetary temperature in the range [0,1]. Usually initialized with 1
						crater_radius,			//!< Maximum crater radius in meters. Usually initialized >= 1
						crater_depth,			//!< Maximum crater depth as a factor of variance. Usually initialized with 1
						crater_min_radius;		//!< Minimum crater radius factor. This effectively determines the range of possible crater sizes that may occur (occuring crater sizes will be in the range [crater_min_radius*crater_radius,crater_radius]. Must be in the range [0,1]. Usually initialized with 0.1
						
		unsigned		recursive_depth,//!< Total number of recursive layers. Should be > 1
						edge_length,	//!< Number of vertices along each edge (copied from Map::range)
						crater_min_count,	//!< Minimum number of craters per top level surface segment. Usually initialized with 0
						crater_max_count;	//!< Maximum number of craters per top level surface segment. Usually initialized with 0
		bool			has_ocean,		//!< True if the fractal body features a oceanic water surface. Usually initialized with true
						has_canyons;	//!< True if the fractal body should be covered by (continuous) canyons. Usually initialized with true
		
		
	};


	/**
		@brief Global vertex definition structure
		
		This structure helps identifying the properties of a vertex. It will be expanded as needed
	*/
	struct TVertexDescriptor
	{
		bool			grid_vertex;	//!< True if this vertex exactly matches a parent vertex, false if it is required to be newly generated
		struct
		{
			unsigned	match,		//!< Exact matching index of this vertex in parent space. Only valid if @a grid_vertex is true
						parent0,	//!< Index of the first primary parent vertex in parent space. Only valid if @a grid_vertex is false
						parent1,	//!< Index of the second primary parent vertex in parent space. Only valid if @a grid_vertex is false
						parent2,	//!< Index of the first secondary parent vertex in parent space. Only valid if @a grid_vertex is false
						parent3;	//!< Index of the second secondary parent vertex in parent space. Only valid if @a grid_vertex is false and @a edge_index -1
		}				parent_space[4];	//!< Parent space field. Pass surface orientation as parameter to determine the exact location in parent space
		char			borderDirection;	//!< [0-2] if this vertex borders a surface edge, -1 otherwise. 0 is located between corner 0 and 1, 1 between 1 and 2, 2 between 2 and 0
		
		unsigned		x,	//!< Child space x index (see fractal diagram for details)
						y,	//!< Child space y index (see fractal diagram for details)
						parent0,	//!< Index of the first primary parent vertex in child space. Only valid if @a grid_vertex is false
						parent1,	//!< Index of the seconds primary parent vertex in child space. Only valid if @a grid_vertex is false
						parent2,	//!< Index of the first secondary parent vertex in child space. Only valid if @a grid_vertex is false
						parent3		//!< Index of the second secondary parent vertex in child space. Only valid if @a grid_vertex is false and @a edge_index -1
						;
	};
}
	
