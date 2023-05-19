#ifndef kernel_configH
#define kernel_configH

#define FRACTAL_VERSION	"v5.6"

/**
	@brief Defines the tendency of parent vertices to override their inherited status

	May be in the range [0,1] defining how loosly the newly generated vertex is bound to its heritage.
	Reducing this value generates less structured noise.

	Observation: 1.0 is overkill. Don't reduce to below 0.1 though
*/
#define FRACTAL_DEGENERATION	0.1f
//#define FRACTAL_DEGENERATION	0.2f
//#define COLOR_CHANNELS	0

#define SECONDARY_PARENT_WEIGHT	0.577f	//!< Influence of secondary parents
//#define SECONDARY_PARENT_WEIGHT	1.0f	//!< Influence of secondary parents

#define CANYON_LOOPS		1		//!< Set 1 (or true) to enable traditional canyon loops on the landscapes


#define OCEANIC_CUSTOM_NOISE_FACTOR	0.05f


#define	WATER_GAIN_DOWN 	1.1		//!< Water factor applied to water from a superior parent vertex 
#define	WATER_GAIN_UP		0.9		//!< Water factor applied to water from an infirior parent vertex
#define	WATER_NOISE			0.05	//!< Water noise level

#define C0_CUSTOM_NOISE_FACTOR	1.125f
//#define C1_CUSTOM_NOISE_FACTOR	1.0f
//#define C2_CUSTOM_NOISE_FACTOR	1.0f			//canyon channel (in addition to other functions)
#define C3_CUSTOM_NOISE_FACTOR	1.5f
#define C4_CUSTOM_NOISE_FACTOR	1.25f

#define SSE_COORDINATES			0	//!< Set 1 to enable 4d SSE coordinates

#define TEXTURE_SEAM_EXTENSION	2	//!< Additional separation texels on both sides of the diagonal of a segment texture. Filled with M::Clamp vertex data

//#define	RIVERS						//!< Define to simulate rivers

#define CLAMP_HEIGHT			true	//!< Causes resulting height values to be M::Clamp to the range [0,1], using rounded off inversion


#if CANYON_LOOPS
	#define CANYON_SLOPE_BEGIN			0.02f	//!< Defines the lower edge of canyon slopes. Channel values beneath this value fully affect vertex height in the form of canyons
	#define CANYON_SLOPE_END			0.09f	//!< Defines the upper edge of canyon slopes. Channel values above this value do not affect vertex height in the form of canyons
	#define REDUCE_CANYON_SLOPE_NOISE	0
#endif


#endif
