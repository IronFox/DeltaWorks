#ifndef timerH
#define timerH

/******************************************************************

Theoretically plattform independent time-query-class.
Works under linux and windows so far.

******************************************************************/

#include "../global_root.h"

#if SYSTEM==UNIX
	#include <sys/time.h>
#endif


namespace DeltaWorks
{
	/*!
	\brief Accurate timer class

	The Timer class can be used to meassures time in microseconds.
	*/
	class Timer
	{
	private:
		float					fresolution;
		double					dresolution;
		UINT64					frequency,
								lastcall;
	public:
		typedef UINT64			Time;				//!< Time container. Currently using unsigned 64 bit integers

		/**/					Timer();
		float					getDeltaf();		//!< Query time since last call in seconds (float) \return Time since last call in seconds
		inline float			GetDelta()				{return getDeltaf();}
		double					getDeltad();		//!< Query time since last call in seconds (double) \return Time since last call in seconds
		inline double			GetDeltaD()			{return getDeltad();}
		Time					getDeltai();		//!< Query time since last call in ticks \return Time since last call in ticks
		inline Time				GetTimeDelta()		{return getDeltai();}
		Time					now()									const;	//!< Query native system time \return Current system time in ticks (zero time unknown)
		inline Time				Now()									const	{return now();}
		Time					netTime()								const;	//!< Query native system time in microseconds \return Current system time in micro seconds
		inline Time				GetNetTime()							const	{return netTime();}
		Time					getTicksPerSecond()						const;	//!< Query number of ticks per second \return Number of ticks per second (always 1000000 on unix)
		inline Time				GetTicksPerSecond()						const	{return getTicksPerSecond();}
		Time					GetAgeOf(Time stamp)					const;	//!< Calculates the time difference in ticks between the specified stamp and the current time
		float					GetSecondsSince(Time stamp)				const;	//!< Calculates the time difference in seconds between the specified stamp and the current time
		double					GetSecondsSinceD(Time stamp)			const;	//!< Calculates the time difference in seconds between the specified stamp and the current time
		float					toSecondsf(const Time&ticks)			const;	//!< Converts ticks into seconds (float) \param ticks Time in ticks \return Time equivalent in seconds
		double					toSecondsd(const Time&ticks)			const;	//!< Converts ticks into seconds (double) \param ticks Time in ticks \return Time equivalent in seconds
		inline float			ToSeconds(const Time&ticks)				const	{return toSecondsf(ticks);}
		inline double			ToSecondsD(const Time&ticks)			const	{return toSecondsd(ticks);}
		Time					toTicks(double seconds)					const;	//!< Converts seconds into ticks \param seconds Time (in seconds) to convert to ticks \return Time equivalent in ticks
		inline Time				ToTicks(double seconds)					const	{return toTicks(seconds);}
		
		/**
		Checks if the current timestamp is greater than the specified stamp plus maxAgeInSeconds.
		If outdated, then stamp is updated to Now() and the method returns true

		@param[inout] stamp Time stamp to check and potentially update to Now() if outdated
		@param maxAgeInSeconds Maximum time span in seconds that the specified stamp may be outdated by
		@return True, if the specified time stamp is older than the specified amount of seconds and has been updated. False otherwise
		*/
		bool					CheckIfOutdatedAndUpdate(Time&stamp, float maxAgeInSeconds)	const;

		Time					netToLocalTicks(const Time&mseconds)	const;
		inline Time				NetToLocalTicks(const Time&mseconds)	const	{return netToLocalTicks(mseconds);}
		Time					localToNetTicks(const Time&ticks)		const;
		inline Time				LocalToNetTicks(const Time&ticks)		const	{return localToNetTicks(ticks);}

		static float			netToSecondsf(const Time&mseconds);	//!< Converts net time (micro seconds) into seconds (float) \param mseconds Time in micro seconds \return Time in seconds
		static inline float		NetToSeconds(const Time&mseconds)	{return netToSecondsf(mseconds);}
		static double			netToSecondsd(const Time&mseconds);	//!< Converts net time (micro seconds) into seconds (double) \param mseconds Time in micro seconds \return Time in seconds
		static inline double	NetToSecondsD(const Time&mseconds)	{return netToSecondsd(mseconds);}
		static Time				netToTicks(double seconds);			//!< Converts net time (in seconds) into ticks (micro seconds)	@param seconds Time (in seconds) to convert to net ticks @return Time equivalent in net ticks
		static inline Time		NetToTicks(double seconds)			{return netToTicks(seconds);}
	};





	extern Timer timer;
}

#endif
