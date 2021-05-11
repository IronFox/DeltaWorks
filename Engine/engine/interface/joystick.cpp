
#if SYSTEM==WINDOWS

#include "joystick.h"
#include <string/encoding.h>

namespace Engine
{
	Joystick::Joystick(InputMap&map_):map(map_),registered(false)
	{
		Reinit();
	}

	void Joystick::Reinit()
	{
		bool wasReg = registered;
		if (wasReg)
			UnregAnalogInputs();
		active.clear();
		#if SYSTEM == WINDOWS

			JOYINFO dummy;
			for (UINT i = 0; i < NumJoysticks; i++)
			{
				bool isThere = joyGetPos(i,&dummy) == JOYERR_NOERROR;
				if (!isThere)
					continue;
				State&st = active.Append();
				memset(&st.info,0,sizeof(st.info));
				st.info.dwSize = sizeof(st.info);
				st.info.dwFlags = JOY_RETURNALL;
				joyGetDevCaps(i,&st.caps,sizeof(st.caps));
				st.index = i;
				st.name = StringEncoding::UTF16::ToUTF8(st.caps.szPname);
			}
		#else
		
		
		#endif
		if (wasReg)
			RegAnalogInputs();
	}
	
	void Joystick::RegAnalogInputs()
	{
		if (registered)
			return;
		registered = true;
		foreach(active,a)
		{
			map.RegAnalog(a->name+".X",a->x,-1,1);
			map.RegAnalog(a->name+".Y",a->y,-1,1);
			map.RegAnalog(a->name+".Throttle",a->throttle,-1,1);
			if (a->caps.wCaps & JOYCAPS_HASR)
				map.RegAnalog(a->name+".Rudder",a->r,-1,1);
			String name = "Joystick"+String(a->index);
			map.RegAnalog(name+".X",a->x,-1,1);
			map.RegAnalog(name+".Y",a->y,-1,1);
			map.RegAnalog(name+".Throttle",a->throttle,-1,1);
			if (a->caps.wCaps & JOYCAPS_HASR)
				map.RegAnalog(name+".Rudder",a->r,-1,1);
		}
	}
	
	void Joystick::UnregAnalogInputs()
	{
		if (!registered)
			return;
		registered = false;
		foreach(active,a)
		{
			map.UnregAnalog(a->name+".X");
			map.UnregAnalog(a->name+".Y");
			map.UnregAnalog(a->name+".Throttle");
			if (a->caps.wCaps & JOYCAPS_HASR)
				map.UnregAnalog(a->name+".Rudder");
			String name = "Joystick"+String(a->index);
			map.UnregAnalog(name+".X");
			map.UnregAnalog(name+".Y");
			map.UnregAnalog(name+".Throttle");
			if (a->caps.wCaps & JOYCAPS_HASR)
				map.UnregAnalog(name+".Rudder");
		}
	}

	float Joystick::State::_ProcessAxis(float axisValue)	const
	{
		if (fabs(axisValue) < dead_zone)
			return 0;
		if (axisValue > 0)
			return (axisValue - dead_zone) / (1.f - dead_zone);

		return - ((-axisValue - dead_zone) / (1.f - dead_zone));
	}
	
	void Joystick::State::Process()
	{
		x = 0;
		y = 0;
		r = 0;
		throttle = 0;
		#if SYSTEM==WINDOWS
			if (joyGetPosEx(index,&info) != JOYERR_NOERROR)
				return;
			x = _ProcessAxis((float)(info.dwXpos-caps.wXmin)/(float)(caps.wXmax-caps.wXmin)*2.f-1.f);
			y = _ProcessAxis((float)(info.dwYpos-caps.wYmin)/(float)(caps.wYmax-caps.wYmin)*2.f-1.f);
			if (caps.wCaps & JOYCAPS_HASZ)
			{
				throttle = -((float)(info.dwZpos-caps.wZmin)/(float)(caps.wZmax-caps.wZmin)*2.0f-1.0f);
				//ShowMessage("throttle is at "+String(throttle)+" as a result of ("+String(info.dwZpos)+"-"+String(caps.wZmin)+")/("+String(caps.wZmax)+"-"+String(caps.wZmin)+")");
			}
			if (caps.wCaps & JOYCAPS_HASR)
			{
				r = _ProcessAxis((float)(info.dwRpos-caps.wRmin)/(float)(caps.wRmax-caps.wRmin)*2.f-1.f);
			}
			for (unsigned i = 0; i < 32; i++)
			{
				bool down = (info.dwButtons&(1<<i))!=0;
				unsigned key = Key::Joystick0Button0+index*32+i;
				if (down != joystick.map.pressed[key])
					if (down)
						joystick.map.keyDown(key);
					else
						joystick.map.keyUp(key);
			}
		#endif
	}


	const Joystick::State*	Joystick::GetActive(const String&name) const
	{
		foreach (active,a)
			if (a->name == name)
				return a;
		return NULL;
	}


	void Joystick::update()
	{
		foreach (active,a)
			a->Process();
	}
	
	Joystick		joystick(input);
}
#endif
