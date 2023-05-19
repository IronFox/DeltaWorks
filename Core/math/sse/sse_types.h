
namespace DeltaWorks::Math::SSE
{
	#ifndef __GNUC__
		#define SSE_VECTOR	__m128
		#define SSE_DEFINE4	_mm_setr_ps
		#define SSE_DEFINE1	_mm_set1_ps
	#else
		class Vector
		{
		public:
				__m128		value;
			
		inline				Vector()
							{}
		inline				Vector(const __m128&v):value(v)
							{}
		inline				Vector(float w, float x, float y, float z):value(_mm_setr_ps(w,x,y,z))
							{}
		inline				Vector(float v):value(_mm_set1_ps(v))
							{}
					
		inline operator __m128() const
							{
								return value;
							}
		};
		#define SSE_VECTOR	::DeltaWorks::Math::SSE::Vector
		#define SSE_DEFINE1	::DeltaWorks::Math::SSE::Vector
		#define SSE_DEFINE4	::DeltaWorks::Math::SSE::Vector
		
	#endif	

}