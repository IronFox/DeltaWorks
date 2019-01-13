#pragma once

#undef INLINE
#include "../third_party/open-simplex-noise/open-simplex-noise.h"

#include "basic.h"

namespace DeltaWorks
{

	/**
	Wrapper for Open Simplex Noise.
	https://github.com/smcameron/open-simplex-noise-in-c
	*/
	class SimplexNoise
	{
		osn_context*	ctx = nullptr;
	
	public:
		/**/		SimplexNoise(int64_t seed)
		{
			open_simplex_noise(seed, &ctx);
		}
	
		/**/		~SimplexNoise()
		{
			open_simplex_noise_free(ctx);
		}

		void		Recreate(int64_t seed)
		{
			open_simplex_noise_free(ctx);
			open_simplex_noise(seed, &ctx);
		}
	
		/**
		Samples semi-gaussian volumetric noise in the range [-1,+1]
		*/
		double		Get(double x, double y, double z)	const
		{
			return open_simplex_noise3(ctx, x, y, z);
		}

		/**
		@copydoc Get(double,double,double)
		@param frequencyModulation Factor multiplied to x, y, and z before computation
		*/
		double		Get(double x, double y, double z, double frequencyModulation) const
		{
			return Get(x * frequencyModulation, y * frequencyModulation, z * frequencyModulation);
		}

		/**
		Samples (close to) uniformly distributed volumetric noise in the range [0,1]
		*/
		double		GetUniform(double x, double y, double z) const
		{
			return ToUniformDistribution(Get(x,y,z));
		}

		/**
		@copydoc GetUniform(double,double,double)
		@param frequencyModulation Factor multiplied to x, y, and z before computation
		*/
		double		GetUniform(double x, double y, double z, double frequencyModulation) const
		{
			return GetUniform(x * frequencyModulation, y * frequencyModulation, z * frequencyModulation);
		}


		static double	ToUniformDistribution(double x0)
		{
			/*
			6.52302987e-01  -1.72903489e+01  -1.12122237e+00   2.77098705e+01
			6.83447908e-01  -1.38410486e+01  -1.54970218e-01   4.01738791e-01
			-3.72274565e-03   2.29025865e+00   4.93543117e-03
			*/


			static const double coefficients[10] = {
				//0.00154044, 2.15570602, 0.78622404, -8.43136551, 24.1497156, -23.4820823, -53.9080885, 148.4834025, -125.3396094, 36.5856659
				0.0, 2.14127769, 0.82418792, -8.11928605, 22.0528563, -16.92479066, -66.7294821, 163.6521446, -134.9893346, 39.0920684
			};


			using std::fabs;
			double x = fabs(x0);
			double x2 = x*x;
			double x3 = x2*x;
			double x4 = x3*x;
			double x5 = x4*x;
			double x6 = x5*x;
			double x7 = x6*x;
			double x8 = x7*x;
			double x9 = x8*x;
			double x10 = x9*x;
		
		
			double rs= M::clamp01(
				+ x9* coefficients[9]
				+ x8* coefficients[8]
				+ x7* coefficients[7]
				+ x6* coefficients[6]
				+ x5* coefficients[5]
				+ x4* coefficients[4]
				+ x3* coefficients[3]
				+ x2* coefficients[2]
				+ x* coefficients[1]
				+ coefficients[0]
				);

			if (x0 < 0)
				return 0.5 - 0.5 * rs;
			return 0.5 + 0.5 * rs;

			//return vmax(0.0,0.499877 + 1.12521 * x + 0.00298947 * x2 - 1.0534 *x3 - 0.00365494 *x4 + 0.431418 *x5);
		}
	};
}
