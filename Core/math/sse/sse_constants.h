#pragma once

#include "sse_types.h"

namespace DeltaWorks::Math::SSE
{

	static	const	SSE_VECTOR	one = _mm_set1_ps(1.0f),
		two = _mm_set1_ps(2.0f),
		three = _mm_set1_ps(3.0f),
		four = _mm_set1_ps(4.0f),
		five = _mm_set1_ps(5.0f),
		six = _mm_set1_ps(6.0f),
		seven = _mm_set1_ps(7.0f),
		eight = _mm_set1_ps(8.0f),
		nine = _mm_set1_ps(9.0f),
		ten = _mm_set1_ps(10.0f),
		twenty = _mm_set1_ps(20.0f),
		thirty = _mm_set1_ps(30.0f),
		hundred = _mm_set1_ps(100.0f),
		c255 = _mm_set1_ps(255.0f),
		c127 = _mm_set1_ps(127.0f),
		c128 = _mm_set1_ps(128.0f),
		zero = _mm_setzero_ps(),
		minus_one = _mm_set1_ps(-1.0f),
		minus_two = _mm_set1_ps(-2.0f),
		minus_three = _mm_set1_ps(-3.0f),
		minus_four = _mm_set1_ps(-4.0f),
		half = _mm_set1_ps(0.5f),
		onethird = _mm_set1_ps(1.0f / 3.0f),
		twothirds = _mm_set1_ps(2.0f / 3.0f),
		point125 = _mm_set1_ps(0.125f),
		point25 = _mm_set1_ps(0.25f),
		point375 = _mm_set1_ps(0.375f),
		point75 = _mm_set1_ps(0.75f),
		point01 = _mm_set1_ps(0.01f),
		point02 = _mm_set1_ps(0.02f),
		point03 = _mm_set1_ps(0.03f),
		point04 = _mm_set1_ps(0.04f),
		point05 = _mm_set1_ps(0.05f),
		point1 = _mm_set1_ps(0.1f),
		point2 = _mm_set1_ps(0.2f),
		point3 = _mm_set1_ps(0.3f),
		point4 = _mm_set1_ps(0.4f),
		point5 = _mm_set1_ps(0.5f),
		point6 = _mm_set1_ps(0.6f),
		point7 = _mm_set1_ps(0.7f),
		point8 = _mm_set1_ps(0.8f),
		point9 = _mm_set1_ps(0.9f),
		onepoint1 = _mm_set1_ps(1.1f),
		onepoint2 = _mm_set1_ps(1.2f),
		onepoint3 = _mm_set1_ps(1.3f),
		onepoint4 = _mm_set1_ps(1.4f),
		onepoint5 = _mm_set1_ps(1.5f);
}


