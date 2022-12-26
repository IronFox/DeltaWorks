#ifndef engineJoystickH
#define engineJoystickH

#if SYSTEM==WINDOWS

#include <math/basic.h>
#include "input.h"

#pragma comment(lib,"Winmm.lib")

namespace Engine
{

	class	Joystick
	{
	private:
	#if SYSTEM==WINDOWS
		static const UINT NumJoysticks = 16;
	//	JOYINFOEX	info[NumJoysticks];
	//	JOYCAPS		caps[NumJoysticks];
	#endif
	public:
		InputMap	&map;

		struct State
		{
			UINT		index;
			float		x,
						y,
						r,
						throttle,
						dead_zone;
			String		name;
			JOYINFOEX	info;
			JOYCAPS		caps;

			/**/		State():x(0),y(0),r(0),throttle(0),dead_zone(0.05)	{}

		private:
			void		Process();
			float		_ProcessAxis(float axisValue)	const;

			friend class Joystick;
		};
	private:
		Ctr::PreAllocVector<State,NumJoysticks>	active;
		bool			registered;
	public:
		count_t			CountActive()	const	{return active.Count();}
		const State*	GetActive(index_t i) const {return i < active.Count() ? active+i : NULL;}
		const State*	GetActive(const String&name) const;	


						Joystick(InputMap&input_map);
		void			RegAnalogInputs();			//!< Registers analog input types to the input map						
		void			UnregAnalogInputs();			//!< Unregisters analog input types to the input map						
		void			update();

		void			Reinit();
	};
	
	
	
	extern Joystick	joystick;



}


#endif

#endif
