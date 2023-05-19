#pragma once

#include "sse_operators.h"

namespace DeltaWorks::Math::SSE
{
	inline void resolveSplineQuad(const SplineQuad<SSE_VECTOR>& quad, const SSE_VECTOR& x, const SSE_VECTOR& y, SSE_VECTOR out[3])
	{
		SSE_VECTOR	ix = SSE::one - x,
			x3 = x * x * x,
			x2 = x * x * ix * SSE::three,
			x1 = x * ix * ix * SSE::three,
			x0 = ix * ix * ix,
			iy = SSE::one - y,
			y3 = y * y * y,
			y2 = y * y * iy * SSE::three,
			y1 = y * iy * iy * SSE::three,
			y0 = iy * iy * iy;

		out[0] = quad.control[0][0][0] * x0 * y0 + quad.control[1][0][0] * x1 * y0 + quad.control[2][0][0] * x2 * y0 + quad.control[3][0][0] * x3 * y0
			+
			quad.control[0][1][0] * x0 * y1 + quad.control[1][1][0] * x1 * y1 + quad.control[2][1][0] * x2 * y1 + quad.control[3][1][0] * x3 * y1
			+
			quad.control[0][2][0] * x0 * y2 + quad.control[1][2][0] * x1 * y2 + quad.control[2][2][0] * x2 * y2 + quad.control[3][2][0] * x3 * y2
			+
			quad.control[0][3][0] * x0 * y3 + quad.control[1][3][0] * x1 * y3 + quad.control[2][3][0] * x2 * y3 + quad.control[3][3][0] * x3 * y3;

		out[1] = quad.control[0][0][1] * x0 * y0 + quad.control[1][0][1] * x1 * y0 + quad.control[2][0][1] * x2 * y0 + quad.control[3][0][1] * x3 * y0
			+
			quad.control[0][1][1] * x0 * y1 + quad.control[1][1][1] * x1 * y1 + quad.control[2][1][1] * x2 * y1 + quad.control[3][1][1] * x3 * y1
			+
			quad.control[0][2][1] * x0 * y2 + quad.control[1][2][1] * x1 * y2 + quad.control[2][2][1] * x2 * y2 + quad.control[3][2][1] * x3 * y2
			+
			quad.control[0][3][1] * x0 * y3 + quad.control[1][3][1] * x1 * y3 + quad.control[2][3][1] * x2 * y3 + quad.control[3][3][1] * x3 * y3;

		out[2] = quad.control[0][0][2] * x0 * y0 + quad.control[1][0][2] * x1 * y0 + quad.control[2][0][2] * x2 * y0 + quad.control[3][0][2] * x3 * y0
			+
			quad.control[0][1][2] * x0 * y1 + quad.control[1][1][2] * x1 * y1 + quad.control[2][1][2] * x2 * y1 + quad.control[3][1][2] * x3 * y1
			+
			quad.control[0][2][2] * x0 * y2 + quad.control[1][2][2] * x1 * y2 + quad.control[2][2][2] * x2 * y2 + quad.control[3][2][2] * x3 * y2
			+
			quad.control[0][3][2] * x0 * y3 + quad.control[1][3][2] * x1 * y3 + quad.control[2][3][2] * x2 * y3 + quad.control[3][3][2] * x3 * y3;
	}

	inline void resolveSplineQuadCenter(const SplineQuad<SSE_VECTOR>& quad, SSE_VECTOR out[3])
	{
		SSE_VECTOR	x3 = SSE::point125,
			x2 = SSE::point375,
			x1 = SSE::point375,
			x0 = SSE::point125,
			y3 = SSE::point125,
			y2 = SSE::point375,
			y1 = SSE::point375,
			y0 = SSE::point125;

		out[0] = quad.control[0][0][0] * x0 * y0 + quad.control[1][0][0] * x1 * y0 + quad.control[2][0][0] * x2 * y0 + quad.control[3][0][0] * x3 * y0
			+
			quad.control[0][1][0] * x0 * y1 + quad.control[1][1][0] * x1 * y1 + quad.control[2][1][0] * x2 * y1 + quad.control[3][1][0] * x3 * y1
			+
			quad.control[0][2][0] * x0 * y2 + quad.control[1][2][0] * x1 * y2 + quad.control[2][2][0] * x2 * y2 + quad.control[3][2][0] * x3 * y2
			+
			quad.control[0][3][0] * x0 * y3 + quad.control[1][3][0] * x1 * y3 + quad.control[2][3][0] * x2 * y3 + quad.control[3][3][0] * x3 * y3;

		out[1] = quad.control[0][0][1] * x0 * y0 + quad.control[1][0][1] * x1 * y0 + quad.control[2][0][1] * x2 * y0 + quad.control[3][0][1] * x3 * y0
			+
			quad.control[0][1][1] * x0 * y1 + quad.control[1][1][1] * x1 * y1 + quad.control[2][1][1] * x2 * y1 + quad.control[3][1][1] * x3 * y1
			+
			quad.control[0][2][1] * x0 * y2 + quad.control[1][2][1] * x1 * y2 + quad.control[2][2][1] * x2 * y2 + quad.control[3][2][1] * x3 * y2
			+
			quad.control[0][3][1] * x0 * y3 + quad.control[1][3][1] * x1 * y3 + quad.control[2][3][1] * x2 * y3 + quad.control[3][3][1] * x3 * y3;

		out[2] = quad.control[0][0][2] * x0 * y0 + quad.control[1][0][2] * x1 * y0 + quad.control[2][0][2] * x2 * y0 + quad.control[3][0][2] * x3 * y0
			+
			quad.control[0][1][2] * x0 * y1 + quad.control[1][1][2] * x1 * y1 + quad.control[2][1][2] * x2 * y1 + quad.control[3][1][2] * x3 * y1
			+
			quad.control[0][2][2] * x0 * y2 + quad.control[1][2][2] * x1 * y2 + quad.control[2][2][2] * x2 * y2 + quad.control[3][2][2] * x3 * y2
			+
			quad.control[0][3][2] * x0 * y3 + quad.control[1][3][2] * x1 * y3 + quad.control[2][3][2] * x2 * y3 + quad.control[3][3][2] * x3 * y3;
	}


}
