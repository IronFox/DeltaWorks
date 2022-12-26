#ifndef fractal_interfaceH
#define fractal_interfaceH

/*
	This module implements the general fractal interface: bodies, segments, index patterns, aspects, seeds, geometries, and hashing functions
*/



#include "../math/vclasses.h"
#include "../math/object.h"
#include "../math/object_def.h"
#include "../general/timer.h"
#include "../general/system.h"
#include "../general/thread_system.h"
#include "../general/random.h"
#include "../container/production_pipe.h"
#include "../container/sorter.h"
#include "../container/vectorList.h"
#include "../io/mutex_log.h"
#include "../io/delta_crc32.h"
#include "../general/auto.h"
#include "../geometry/coordinates.h"
#include "../io/log_stream.h"


namespace DeltaWorks
{

	/**
	@brief Fractal landscape generator
	*/
	namespace Fractal
	{
	
		typedef M::Composite::sector_t	sector_t;		//!< Sector coordinate type. Typically int32

		struct TMapVertex;
	

	
		class VertexMap;
		template <unsigned,unsigned /*=0*/>
			class TemplateMap;
	
		class Geometry;
	
	
		void				MakeSphereGeometry(Geometry&target);	//!< Adjusts the given geometry to approximate a sphere
		Geometry			MakeSphereGeometry();					//!< Creates a new sphere approximating geometry
		void				MakeHaloGeometry(Geometry&target);		//!< Adjusts the given geometry to approximate a halo shape
		Geometry			MakeHaloGeometry();						//!< Creates a new halo approximating geometry
	
		/**
		@brief Fractal geometry
	
		The fractal geometry is used to describe the rough geometry of a fractal shape. It may consist of only a singular triangle, or a sphere, or any shape as required.
		*/
		class Geometry 
		{
		public:
			typedef M::TVec3<>	Vertex;

			Ctr::Array<Vertex>	vertexField;	//!< Geometrical vertices
			Ctr::Array<UINT32>	triangleField,	//!< Triangular index path. Each three indices form one triangle using the vertices stored in \b vertex_field
								edgeField;		//!< Edge index path. Each two indices form one edge, using the vertices stored in @b vertexField
		};

	}

}

#endif
