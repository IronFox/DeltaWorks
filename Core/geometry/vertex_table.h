#ifndef vertex_tableH
#define vertex_tableH

#include "../container/hashtable.h"
#include "../container/buffer.h"
#include "../math/basic.h"
#include "../math/vector.h"

namespace DeltaWorks
{

	struct TVertexIntPosition:public M::TVec3<__int64>	//!< Internal integer vertex position
	{
	
		bool		operator!=(const TVertexIntPosition&other)	const
		{
			return	x != other.x
					||
					y != other.y
					||
					z != other.z;
		}

	};




	/**
	@brief Range tolerant vertex map to fuse multiple occurrences of similar vertices
	
	Vertices are converted to integer coordinates and mapped via a hashtable.
	*/
	class VertexTable:protected Container::HashTable<TVertexIntPosition,index_t>
	{
	public:
		typedef M::TVec3<>	TPosition;


	protected:
		typedef HashTable<TVertexIntPosition,index_t>	Table;
		
		Container::Buffer<TPosition>					position_map;
		float											tolerance;
	public:
		/**/							VertexTable(float tolerance=1e-3f);	//!< Constructs a newvertex table using the specified tolerance
		
		void							clear();							//!< Clears all local data
		void							setTolerance(float new_tolerance);	//!< Changes the coordinate tolerance and clears all local data
		
		index_t							map(const float position[3]);		//!< Maps a vertex into the structure and returns its map index
		index_t							map(const TPosition&position);		//!< Maps a vertex into the structure and returns its map index
		const TPosition&				vertex(index_t index)	const;		//!< Queries an index from the local position table
		count_t							vertices()	const {return position_map.GetLength();}	//!< Queries the total number of diversive locations mapped to the vertex table
	};




}


#endif
