#include "timing.h"

/******************************************************************

Engine timing-unit.

******************************************************************/

namespace Engine
{

	Timing		timing;

	void Timing::Update()
	{
	    delta64 = now64.Elapsed();
	    now64 += delta64;
	    delta = delta64.ToSeconds();
	}

	void Timing::Initialize()
	{
		delta = 0.01f;
		delta64 = Time::Delta(delta);
		now64.Update();
		timeReference = now64;
	}
}
