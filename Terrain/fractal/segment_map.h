#pragma once

/*
	This module implements the segment map structure, based on the base vertex map structures specified in 'fractal_types.h'
*/

#include "fractal_types.h"

namespace DeltaWorks::Fractal
{
	#define NUM_VERTICES2(VMAX) ((((VMAX)*(VMAX)) + 3LL*((VMAX)) +2LL)/2LL)
	//#define NUM_VERTICES(exponent) (((1LL<<(2*(exponent))) + 3LL*(1LL<<(exponent)) +2LL)/2LL)
	#define NUM_VERTICES(exponent) NUM_VERTICES2((1LL<<(exponent)))
	//(((1LL<<(2*(exponent))) + 3LL*(1LL<<(exponent)) +2LL)/2LL)
	
	struct TMapVertex:public TVertexDescriptor	//!< VertexMap information regarding one abstract vertex
	{
			unsigned				index,			//!< Index of this vertex in the vertex field
									indexAlongBorder;	//!< Index along the border (if along a border, 0 otherwise)
			BYTE					dir;			//!< Direction of the closest border (0xFF = NA)
			union
			{
				unsigned			sibling[4];		//!< Index of the nth (0-3) sibling vertex (if existing, otherwise 0xFFFFFFFF). Valid ONLY if @a grid_vertex is @b false
				unsigned			child[6];		//!< Index of the nth (0-5) child vertex (if existing, otherwise 0xFFFFFFFF). Valid ONLY if @a grid_vertex is @b true
			};

			//bool					grid_vertex;	//!< True if this vertex is directly derived from the respective parent surface
	};
	

	/**
		@brief Base vertex map

		VertexMap provides a range of information regarding the positioning and relationship of vertices.
		Each map instance is created for a specific exponent and cannot be applied to other exponents.
		<br>
		Do not create instances of this class. Query VertexMap<exponent>::instance instead.
	*/
	class VertexMap
	{
	public:
			struct TParentInfo	//! Abstract information about one parent-derived vertex
			{
				unsigned			index,		//!< Index in a local face vertex field
									origin[4];	//!< Index in the respective parent face vertex field (given the respective parent orientation)
			};
			
			TMapVertex				*vertex_descriptor;					//!< Vertex map
			unsigned				exponent,						//!< Base map exponent
									vertex_range,					//!< Maximum number of vertices in one direction in a face
									vertex_maximum,					//!< Highest possible vertex index in one direction (vertex_range-1)
									vertex_count,					//!< Total number of vertices in one face
									child_border_vertex_count,		//!< Number of non-grid vertices along one edge
									child_vertex_count,				//!< Number of newly generated vertices
									inner_vertex_count,				//!< Number of vertices that are not part of any edge
									parent_vertex_count,			//!< Number of vertices copied from the respective parent face
									inner_parent_vertex_count;		//!< Number of vertices copied from the respective parent face that are not part of any border
			unsigned				*border_index[3],				//!< Outer border vertex index map
									*child_border_index[3],			//!< Outer border vertex index map featuring only non-grid vertices
									*inner_border_index[3];			//!< Inner border vertex index map
			unsigned				*child_vertex_index,			//!< Child vertex index map
									*inner_vertex_index;			//!< Inner vertex index map (pointing to all vertices not part of an edge)
			TParentInfo				*parent_vertex_info,			//!< Parent vertex information map
									*inner_parent_vertex_info;		//!< Parent vertex information map covering only inner parents (parents that are not part of any edge)
			
	
			void					build();
			
	protected:
									//VertexMap(TMapVertex*const,unsigned, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned*, unsigned*,unsigned*,  unsigned*,unsigned*,unsigned*, unsigned*,unsigned*,unsigned*, unsigned*const, unsigned*const, unsigned*const, TParentInfo*const);
	public:

	/**
		@brief Queries the absolute index of a vertex via its grid coordinates

		Converts vertex grid coordinates (column, row) to a linear index.
		Invokes a fatal exception if the specified coordinates are invalid.


		@param col Column of the vertex in the range [0,row]
		@param row Row of the vertex in the range [0,vertex_maximum]
		@return Linear vertex index
	*/
	inline	unsigned				getIndex(unsigned col, unsigned row)	const
									{
										if (row >= vertex_range || col > row)
											FATAL__("Index-Retrival-Error for ("+IntToStr(col)+", "+IntToStr(row)+") range="+IntToStr(vertex_range));
										return row*(row+1)/2+col;
									}
	/**
		@brief Queries the absolute index of a vertex via its grid coordinates

		Converts vertex grid coordinates (column, row) to a linear index.
		Returns 0xFFFFFFFF if the specified coordinates are invalid.

		@param col Column of the vertex in the range [0,row]
		@param row Row of the vertex in the range [0,vertex_maximum]
		@return Linear vertex index or 0xFFFFFFFF if the specified grid coordinates are invalid
	*/
	inline	unsigned				getIndexC(unsigned col, unsigned row) const
									{
										if (row >= vertex_range || col > row)
											return UNSIGNED_UNDEF;
										return row*(row+1)/2+col;
									}
			//void					assembleDeviceFields();
	};



	template <unsigned Exponent, unsigned RangeReduction =0>
		class BaseTemplateMap
		{
		public:
			typedef VertexMap::TParentInfo	TParentInfo;

			static	const unsigned		range		= (1<<Exponent)+1 - RangeReduction,					//!< Copied to \a VertexMap::vertex_range
										vertexMax = (1 << Exponent) - RangeReduction,
										vertices	= NUM_VERTICES2(vertexMax),			//!< Copied to \a VertexMap::vertex_count
										child_border_vertices = (1 << (Exponent - 1)) - RangeReduction,		//!< Copied to @a VertexMap::child_border_vertex_count
										parent_vertices	= NUM_VERTICES(Exponent-1),		//!< Copied to \a VertexMap::parent_vertex_count
										child_vertices	= vertices-parent_vertices,		//!< Copied to \a VertexMap::child_vertex_count
										inner_vertices = vertices - 3*(range-1),	//!< Copied to \a VertexMap::inner_vertex_count
										inner_parent_vertices = parent_vertices-3*child_border_vertices;	//!< Number of parent vertices that are not part of any edge
		
		
				TMapVertex				info[vertices];										//!< Mapped to \a VertexMap::vertex_descriptor
		
				unsigned				border[3][range],									//!< Mapped to \a VertexMap::border_index
										child_border[3][child_border_vertices],				//!< Mapped to @a VertexMap::child_border_index
										inner_border[3][range-1],							//!< Mapped to \a VertexMap::inner_border_index
										child_vertex[child_vertices>0?child_vertices:1],	//!< Mapped to \a VertexMap::child_vertex_index
										inner[inner_vertices>0?inner_vertices:1];			//!< Mapped to @a VertexMap::inner_vertex_index
										
				TParentInfo				parent_vertex[parent_vertices>0?parent_vertices:1],	//!< Mapped to \a VertexMap::parent_vertex_info
										inner_parent_vertex[inner_parent_vertices>0?inner_parent_vertices:1];	//!< Mapped to @a VertexMap::inner_parent_vertex_info;

				

				void					Create(VertexMap&target)
				{
					target.vertex_descriptor = info;
					target.exponent = Exponent;
					target.vertex_range = range;
					target.vertex_maximum = range-1;
					target.vertex_count = vertices;
					if (RangeReduction == 0)
					{
						target.child_border_vertex_count = child_border_vertices;
						target.child_border_index[0] = child_border[0];
						target.child_border_index[1] = child_border[1];
						target.child_border_index[2] = child_border[2];
						target.child_vertex_count = child_vertices;
						target.child_vertex_index = child_vertex;
						target.parent_vertex_count = parent_vertices;
						target.parent_vertex_info = parent_vertex;
						target.inner_parent_vertex_count = inner_parent_vertices;
						target.inner_parent_vertex_info = inner_parent_vertex;
					}
					else
					{
						target.child_border_vertex_count = 0;
						target.child_border_index[0] = nullptr;
						target.child_border_index[1] = nullptr;
						target.child_border_index[2] = nullptr;
						target.child_vertex_count = 0;
						target.child_vertex_index = nullptr;
						target.parent_vertex_count = 0;
						target.parent_vertex_info = 0;
						target.inner_parent_vertex_count = 0;
						target.inner_parent_vertex_info = 0;
					}
					target.inner_vertex_count = inner_vertices;
					target.inner_vertex_index = inner;
					target.border_index[0] = border[0];
					target.border_index[1] = border[1];
					target.border_index[2] = border[2];
					target.inner_border_index[0] = inner_border[0];
					target.inner_border_index[1] = inner_border[1];
					target.inner_border_index[2] = inner_border[2];

					target.build();				
				
				
				}
			/**
				@brief Queries the absolute index of a vertex via its grid coordinates

				Converts vertex grid coordinates (column, row) to a linear index.
				Invokes a fatal exception if the specified coordinates are invalid.


				@param col Column of the vertex in the range [0,row]
				@param row Row of the vertex in the range [0,vertex_maximum]
				@return Linear vertex index
			*/
			inline static unsigned			getStaticIndex(unsigned col, unsigned row)
											{
												if (row >= range || col > row)
													FATAL__("Index-Retrival-Error for ("+IntToStr(col)+", "+IntToStr(row)+") range="+IntToStr(range));
												return row*(row+1)/2+col;
											}
			/**
				@brief Queries the absolute index of a vertex via its grid coordinates

				Converts vertex grid coordinates (column, row) to a linear index.
				Returns 0xFFFFFFFF if the specified coordinates are invalid.

				@param col Column of the vertex in the range [0,row]
				@param row Row of the vertex in the range [0,vertex_maximum]
				@return Linear vertex index or 0xFFFFFFFF if the specified grid coordinates are invalid
			*/
			inline static unsigned			getStaticIndexC(unsigned col, unsigned row)
											{
												if (row >= range || col > row)
													return UNSIGNED_UNDEF;
												return row*(row+1)/2+col;
											}
		
		};
	
	/**
		\brief Derivative exponential fractal index map
		
		This derivative index map retrieves the required array sizes from the specified exponent parameter.
	*/
	template <unsigned Exponent, unsigned RangeReduction =0>
		class TemplateMap:public VertexMap, public BaseTemplateMap<Exponent,RangeReduction>
		{
			typedef BaseTemplateMap<Exponent, RangeReduction> Super;
		public:
		
										TemplateMap()
										{
											Super::Create(*this);

										}

		

		};


	/**
		@brief Global single instance maps

		This class instantiates one map instance per queries exponent.
		<br>Usage: VertexMap<Exponent>::instance

		@tparam Exponent Exponent to use the map of.
	*/
	template <unsigned Exponent>
		class Map
		{
		public:
		static	TemplateMap<Exponent>	instance;	//!< Global map instance
		};

	template <unsigned Exponent>
		TemplateMap<Exponent>	Map<Exponent>::instance;

}


