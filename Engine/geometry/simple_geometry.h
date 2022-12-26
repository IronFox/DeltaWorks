#ifndef engine_simple_geometryH
#define engine_simple_geometryH

/******************************************************************

Language-independent simple geometry-container.

******************************************************************/


#include <math/vector.h>
#include <container/vectorList.h>

namespace Engine
{
	using namespace DeltaWorks;

	struct	TVertex:public M::TVec4<> //! Basic vertex. Homogenous coordinates (4th component should usually be 1.0f)
	{};

	struct	TColorVertex:public TVertex //! Colored vertex
	{
			M::TVec4<>					color; //!< RGBA color components
	};

	struct	TNormalVertex:public TVertex //! Vertex with a normal
	{
			M::TVec3<>					normal; //!< 3d normal
	};

	struct TNormalTextureVertex:public TNormalVertex //! Vertex with a normal and 2d texture-coordinates
	{
			M::TVec2<>					coord; //!< 2d texture coordinates
	};

	struct TColorTextureVertex:public TColorVertex	//! Colored vertex with 2d texture-coordinates
	{
			M::TVec2<>					coord; //!< 2d texture coordinates
	};

	struct TTextureVertex:public TVertex	//! Vertex with 2d texture-coordinates
	{
			M::TVec2<>					coord; //!< 2d texture coordinates
	};



	typedef TColorVertex			tCVertex;
	typedef TNormalVertex			tNVertex;
	typedef TColorTextureVertex		tCTVertex;
	typedef TNormalTextureVertex	tNTVertex;
	typedef TTextureVertex			tTVertex;







	template <unsigned len>
		struct	TPrimitive //! Basic eve-primitive (line, triangle, or quad)
		{
				tCVertex				vertex[len]; //!< Vertex field
		};


	class SimpleGeometry	//! Simple geometrical object
	{
	public:
			enum Mode{Lines=0,Triangles=1,Quads=2};
	private:
			tCVertex				v;
			unsigned				at;
			Mode					m;
			bool					sealed;

			void					pushVertex();

	public:

			Ctr::Vector<TColorVertex>	field[3];	//!< One colored vertex field for each lines, triangles and quads

									SimpleGeometry();
			void					compact();
			void					reset();												//!< Resets all buffers
			void					Resize(unsigned len);									//!< Resizes each of the buffers to the specified size
			void					select(Mode mode);										//!< Selects the next primitive-type for definition. The previous primitive-type must be fully defined. \param mode New primitive type
			void					seal();													//!< Seals the structure disallowing further editing. Also compactifies the fields
			void					translate(float x_delta, float y_delta, float z_delta); //!< Translates all primitives by the specified delta values.
			void					color(float r, float g, float b, float a=1.0f);			//!< Specifies the color of the current vertex
			void					color(const M::TVec4<>&color);
			void					color(const M::TVec3<>&color);
			void					color(const M::TVec3<>&color, float alpha);
			void					color3fv(const float*c);								//!< Specifies the color of the current vertex
			void					color4fv(const float*c);								//!< Specifies the color of the current vertex
			void					color3fva(const float*c, float a);						//!< Specifies the color of the current vertex
			void					vertex(float x, float y, float z=0.0f, float w=1.0f);	//!< Specifies the coordinates of the current vertex and increments the vertex position by one
	};
}

#endif

