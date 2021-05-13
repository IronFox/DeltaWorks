#ifndef engine_timingH
#define engine_timingH

/******************************************************************

Engine timing-unit.

******************************************************************/


#include <general/timer.h>

namespace Engine
{

	using namespace DeltaWorks;

	class Timing
	{
	public:
		Time::Stamp		now64,
						timeReference;
		Time::Delta		delta64;

	    float   		delta=1;
		double			now=0;

	    void    		Update();
	    void    		Initialize();
	};

	extern Timing timing;

}

#endif
