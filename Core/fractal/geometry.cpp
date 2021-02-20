#include "geometry.h"
#include "../io/log.h"
#include "../general/thread_system.h"


namespace DeltaWorks
{


	namespace Fractal
	{
	
	
		//ReferenceVector<TSurfaceSegment>					surface_list;

	

		void				MakeHaloGeometry(Geometry&geometry)
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
	
		Geometry			MakeHaloGeometry()
		{
			Geometry result;
			MakeHaloGeometry(result);
			return result;
		}
	
		Geometry			MakeSphereGeometry()
		{
			Geometry result;
			MakeSphereGeometry(result);
			return result;
		}


		static void IcoCoords(M::TVec3<>*vertices, index_t idx0, index_t idx1)
		{
			static const float phi = (1.f + sqrt(5.f)) / 2.f;
			static const float normalize = 1.f / sqrt(1.f + phi*phi);

			index_t at = 0;
			for (int x = -1; x < 2; x+=2)
				for (int y = -1; y < 2; y+=2, at++)
				{
					vertices[at].v[idx0] = x * normalize;
					vertices[at].v[idx1] = phi * y * normalize;
				}

			bool brk = true;
		}

	
		void	MakeSphereGeometry(Geometry&geometry)
		{
			//4 sided:
			/*
			geometry.vertex_field.Resize(4);
			geometry.index_field.Resize(12);
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
			geometry.vertex_field.Resize(6);
			geometry.index_field.Resize(24);
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

			geometry.vertexField.Fill(M::Vector3<>::zero);

			/*
			IcoCoords(geometry.vertexField+0,1,2);
			IcoCoords(geometry.vertexField+4,0,1);
			IcoCoords(geometry.vertexField+8,2,0);
			*/
			
			{
				const double phi = (1.0 + sqrt(5.0)) / 2.0;
				const double r0 = sqrt(1.0 + phi*phi);
				const double ea = 2.0 * asin(1.0 / r0) * 180.0 / M_PI;
				float hAngle = 90 -ea;

				/*
				const double e = 2.0 / r0;
				const double f = e * sqrt(0.75);

				float hAngle = float(asin(f * 0.5) * 180.0 / M_PIF);
				*/


				M::Vec::sphereCoords(0,90,1,geometry.vertexField[0]);
	    
				M::Vec::sphereCoords(0, hAngle, 1, geometry.vertexField[1]);
				M::Vec::sphereCoords(-72, hAngle, 1, geometry.vertexField[2]);
				M::Vec::sphereCoords(-144, hAngle, 1, geometry.vertexField[3]);
				M::Vec::sphereCoords(-216, hAngle, 1, geometry.vertexField[4]);
				M::Vec::sphereCoords(-288, hAngle, 1, geometry.vertexField[5]);

				M::Vec::sphereCoords(-36, -hAngle, 1, geometry.vertexField[6]);
				M::Vec::sphereCoords(-108, -hAngle, 1, geometry.vertexField[7]);
				M::Vec::sphereCoords(-180, -hAngle, 1, geometry.vertexField[8]);
				M::Vec::sphereCoords(-252, -hAngle, 1, geometry.vertexField[9]);
				M::Vec::sphereCoords(-324, -hAngle, 1, geometry.vertexField[10]);

				M::Vec::sphereCoords(0, -90, 1, geometry.vertexField[11]);

			}


			Array<float>distances (geometry.edgeField.GetLength()/2);
			float min = std::numeric_limits<float>::max();
			float max = 0;
		
			for (index_t i = 0; i < geometry.edgeField.GetLength(); i+=2)
			{
				const auto e0 = geometry.edgeField[i];
				const auto e1 = geometry.edgeField[i+1];

				auto d = distances[i/2] = Vec::distance(
					geometry.vertexField[e0],
					geometry.vertexField[e1]
				);

				min = std::min(min,d);
				max = std::max(max,d);
			}

			float error = max/min - 1.f;
			ASSERT3__(error < 0.01f, min,max, max/min);
		}
	
	
	}
}
