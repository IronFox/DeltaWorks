#pragma once

#include "sse_types.h"
#include "sse_constants.h"



namespace S = ::DeltaWorks::Math::SSE;



inline SSE_VECTOR operator+(float value, const SSE_VECTOR& v)
{
	return _mm_add_ps(_mm_set1_ps(value), v);
}
inline SSE_VECTOR operator*(float value, const SSE_VECTOR& v)
{
	return _mm_mul_ps(_mm_set1_ps(value), v);
}
inline SSE_VECTOR operator-(float value, const SSE_VECTOR& v)
{
	return _mm_sub_ps(_mm_set1_ps(value), v);
}
inline SSE_VECTOR operator/(float value, const SSE_VECTOR& v)
{
	return _mm_div_ps(_mm_set1_ps(value), v);
}

inline SSE_VECTOR operator+(const SSE_VECTOR& v, float value)
{
	return _mm_add_ps(v, _mm_set1_ps(value));
}
inline SSE_VECTOR operator*(const SSE_VECTOR& v, float value)
{
	return _mm_mul_ps(v, _mm_set1_ps(value));
}
inline SSE_VECTOR operator-(const SSE_VECTOR& v, float value)
{
	return _mm_sub_ps(v, _mm_set1_ps(value));
}
inline SSE_VECTOR operator/(const SSE_VECTOR& v, float value)
{
	return _mm_div_ps(v, _mm_set1_ps(value));
}

inline void operator+=(SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	v0 = _mm_add_ps(v0, v1);
}
inline void operator*=(SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	v0 = _mm_mul_ps(v0, v1);
}
inline void operator-=(SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	v0 = _mm_sub_ps(v0, v1);
}
inline void operator/=(SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	v0 = _mm_div_ps(v0, v1);
}

inline SSE_VECTOR operator+(const SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	return _mm_add_ps(v0, v1);
}
inline SSE_VECTOR operator*(const SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	return _mm_mul_ps(v0, v1);
}
inline SSE_VECTOR operator-(const SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	return _mm_sub_ps(v0, v1);
}
inline SSE_VECTOR operator-(const SSE_VECTOR& v0)
{
	return _mm_mul_ps(v0, S::minus_one);
}
inline SSE_VECTOR operator/(const SSE_VECTOR& v0, const SSE_VECTOR& v1)
{
	return _mm_div_ps(v0, v1);
}


