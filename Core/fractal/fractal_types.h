#ifndef fractal_typesH
#define fractal_typesH

/*
	This module implements the more basic data structures required by the fractal interface in order to communicate with the kernel
*/

#include "../container/array.h"
#include "../container/aligned_array.h"
//#include "../engine/gl/buffer_object.h"
//#include "../engine/gl/texture.h"
#include "../geometry/coordinates.h"
#include "../math/vector.h"
#include "../container/vectorList.h"
#include "../image/image.h"


namespace DeltaWorks
{

	namespace Fractal
	{



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
}

#endif
