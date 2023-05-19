#include "../global_root.h"
#include "timer.h"

namespace DeltaWorks::Time
{
	namespace Details
	{
		
		#if TIME_IMPL == TIME_WIN_QUERY

			UINT64	frequency = 0;
			float	invFrequencyF = 0;
			double	invFrequencyD = 0;
			struct Constructor
			{
				Constructor()
				{
					DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
					if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency))
						FATAL__("QueryPerformanceFrequency failed");
					SetThreadAffinityMask(GetCurrentThread(), oldmask);

					invFrequencyF = 1.f / frequency;
					invFrequencyD = 1.0 / frequency;
				}
			} constructor;

			#define TICKS_PER_SECONDS	::DeltaWorks::Time::Details::frequency
			#define TICK_D		::DeltaWorks::Time::Details::invFrequencyD
			#define TICK_F		::DeltaWorks::Time::Details::invFrequencyF

		#endif
		#if TIME_IMPL == TIME_WIN
			#define TICKS_PER_SECONDS	10'000'000ll
			#define TICK_D		1e-7
			#define TICK_F		1e-7f
		#endif
		#if TIME_IMPL == TIME_SYS
			#define TICKS_PER_SECONDS	1'000'000ll
			#define TICK_D		1e-6
			#define TICK_F		1e-6f
		#endif

			//100-nanosecond intervals

		stamp_t		Now()
		{
			#if TIME_IMPL == TIME_CHRONO
				return chrono_clock_t::now();
			#endif
			#if TIME_IMPL == TIME_WIN_QUERY
				stamp_t time;
				DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
				QueryPerformanceCounter((LARGE_INTEGER*)&time);
				SetThreadAffinityMask(GetCurrentThread(), oldmask);
				return time;
			#endif
			#if TIME_IMPL == TIME_WIN
				FILETIME t;
				GetSystemTimePreciseAsFileTime(&t);
				return (UINT64(t.dwHighDateTime) << 32) | t.dwLowDateTime;
			#endif
			#if TIME_IMPL == TIME_SYS
				timeval t;
				gettimeofday(&t,NULL);
				return frequency*t.tv_sec + t.tv_usec;
			#endif
		}


		stamp_t		FromMicroSeconds(UINT64 microSeconds)
		{
			#if TIME_IMPL == TIME_CHRONO
				return stamp_t(delta_t(std::chrono::microseconds(microSeconds)));
			#endif
			#if TIME_IMPL == TIME_WIN || TIME_IMPL == TIME_WIN_QUERY
				return stamp_t(double(microSeconds)*TICKS_PER_SECONDS * 1e-6);
			#endif
			#if TIME_IMPL == TIME_SYS
				return microSeconds;
			#endif
		}

		delta_t		SecondsToDelta(long double seconds)
		{
			#if TIME_IMPL == TIME_CHRONO
				return std::chrono::nanoseconds(long long(seconds * 1e9l));
			#endif
			#if TIME_IMPL == TIME_WIN || TIME_IMPL == TIME_WIN_QUERY
				return delta_t(seconds*TICKS_PER_SECONDS);
			#endif
			#if TIME_IMPL == TIME_SYS
				return delta_t(seconds * 1e6l);
			#endif
		}

		
		delta_t		MillisecondsToDelta(long long milliseconds)
		{
			#if TIME_IMPL == TIME_CHRONO
				return std::chrono::milliseconds(milliseconds);
			#endif
			#if TIME_IMPL == TIME_WIN || TIME_IMPL == TIME_WIN_QUERY
				return delta_t(milliseconds *TICKS_PER_SECONDS * 1000);
			#endif
			#if TIME_IMPL == TIME_SYS
				return delta_t(milliseconds * 1e9l);
			#endif
		}


		constexpr delta_t ZeroDelta()
		{
			#if TIME_IMPL == TIME_CHRONO
				return delta_t::zero();
			#else
				return 0;
			#endif
		}


	}



	MicroStamp::MicroStamp(const Stamp&stamp):internal(stamp.ToMicroSecondsU())
	{}

	
	Stamp::Stamp():internal(Details::Now())
	{}

	Stamp::Stamp(const MicroStamp&micro):internal(Details::FromMicroSeconds(micro.GetTicks()))
	{
		//DBG_ASSERT__(internal < Details::Now());
		//DBG_ASSERT__(internal > Details::Now() - TICKS_PER_SECONDS*1000);
	}


	Delta			Stamp::operator-(const Stamp&other) const
	{
		#if TIME_IMPL == TIME_CHRONO
			return Delta(internal - other.internal,false);
		#else
			return Delta(Details::delta_t(internal - other.internal),false);
		#endif
	}

	Stamp			Stamp::operator+(const Delta&delta) const
	{
		return Stamp(internal + delta.internal,false);
	}

	Stamp			Stamp::operator-(const Delta&delta) const
	{
		return Stamp(internal - delta.internal,false);
	}

	Stamp&			Stamp::operator+=(const Delta&delta)
	{
		internal += delta.internal;
		#ifdef _DEBUG
			const auto now = Details::Now();
			const auto dt = Details::delta_t(now - internal);
			//ASSERT__(dt >= 0);
			//ASSERT__(dt < TICKS_PER_SECONDS*1000);
		#endif
		return *this;
	}

	Stamp&			Stamp::operator-=(const Delta&delta)
	{
		internal -= delta.internal;
		DBG_ASSERT__(internal < Details::Now());
		//DBG_ASSERT__(internal > Details::Now() - TICKS_PER_SECONDS*1000);
		return *this;
	}

	UINT64			Stamp::ToMicroSecondsU() const
	{
		#if TIME_IMPL == TIME_CHRONO
			typedef std::chrono::duration<UINT64,std::micro> out_t;
			out_t rs = std::chrono::duration_cast<out_t>(internal.time_since_epoch());
			return rs.count();
		#endif
		#if TIME_IMPL == TIME_WIN || TIME_IMPL == TIME_WIN_QUERY
			return static_cast<UINT64>(static_cast<double>(internal)*1'000'000.0*TICK_D);
		#endif
		#if TIME_IMPL == TIME_SYS
			return internal;
		#endif
	}

	void Stamp::Update()
	{
		internal = Details::Now();
	}


	Stamp::Stamp(const Details::stamp_t&s, bool):internal(s)
	{
		//DBG_ASSERT__(internal < Details::Now());
		//DBG_ASSERT__(internal > Details::Now() - TICKS_PER_SECONDS*1000000);
	}

	Delta			Stamp::Elapsed() const
	{
		auto now = Details::Now();
		auto delta = Details::delta_t(now - internal);
		DBG_ASSERT__(delta > Details::ZeroDelta());
		return Delta(delta,false);
	}

	float			Stamp::ElapsedSeconds() const
	{
		return Elapsed().ToSeconds();
	}


	UINT64			Stamp::ToTicks() const
	{
		#if TIME_IMPL == TIME_CHRONO
			return MicroStamp(*this).GetTicks();
		#else
			return internal;
		#endif;
	}

	/*static*/ Stamp	Stamp::FromTicks(UINT64 ticks)
	{
		#if TIME_IMPL == TIME_CHRONO
			return Stamp(MicroStamp(ticks));
		#else
			return Stamp(ticks,false);
		#endif
	}

	bool					Stamp::CheckIfOutdatedAndUpdate(const Delta&maxAge)
	{
		Stamp now;
		if (now - *this > maxAge)
		{
			*this = now;
			return true;
		}
		return false;
	}


	Delta::Delta():internal(Details::ZeroDelta())
	{}

	
	Delta::Delta(long double seconds):internal(Details::SecondsToDelta(seconds))
	{}
	Delta::Delta(double seconds):internal(Details::SecondsToDelta(seconds))
	{}

	/*explicit			*/	Delta::Delta(Details::delta_t internal):internal(internal)
	{}


	/*static*/ Delta			Delta::FromMilliseconds(long long ms)
	{
		return Delta(Details::MillisecondsToDelta(ms));
	}


	/*static*/ Delta			Delta::FromSeconds(long long s)
	{
		return Delta(Details::SecondsToDelta(s));
	}


	float	Delta::ToSeconds() const
	{
		#if TIME_IMPL == TIME_CHRONO
			typedef std::chrono::duration<float> out_t;
			out_t rs = internal;
			return rs.count();
		#endif
		#if TIME_IMPL == TIME_WIN || TIME_IMPL == TIME_WIN_QUERY
			return TICK_F * internal;
		#endif
		#if TIME_IMPL == TIME_SYS
			return 1e-6f * internal;
		#endif

	}

	double	Delta::ToSecondsD() const
	{
		#if TIME_IMPL == TIME_CHRONO
			typedef std::chrono::duration<double> out_t;
			out_t rs = internal;
			return rs.count();
		#endif
		#if TIME_IMPL == TIME_WIN || TIME_IMPL == TIME_WIN_QUERY
			return TICK_D * internal;
		#endif
		#if TIME_IMPL == TIME_SYS
			return 1e-6 * internal;
		#endif
	}

	Delta					Delta::operator+(const Delta&other) const
	{
		return Delta(internal + other.internal,false);
	}

	Delta&					Delta::operator+=(const Delta&other)
	{
		internal += other.internal;
		return *this;
	}

	Delta					Delta::operator-(const Delta&other) const
	{
		return Delta(internal - other.internal,false);
	}

	Delta&					Delta::operator-=(const Delta&other)
	{
		internal -= other.internal;
		return *this;
	}

	bool					Delta::operator>(const Delta&other) const
	{
		return internal > other.internal;
	}

	bool					Delta::operator>=(const Delta&other) const
	{
		return internal >= other.internal;
	}

	bool					Delta::operator<(const Delta&other) const
	{
		return internal < other.internal;
	}

	bool					Delta::operator<=(const Delta&other) const
	{
		return internal <= other.internal;
	}

	bool					Delta::operator==(const Delta&other) const
	{
		return internal == other.internal;
	}

	bool					Delta::operator!=(const Delta&other) const
	{
		return internal != other.internal;
	}
	
	bool					Delta::IsInRange(const Delta&min, const Delta&max) const
	{
		return internal >= min.internal && internal <= max.internal;
	}

	Delta::Delta(const Details::delta_t&d,bool):internal(d)
	{}

	bool					Delta::IsNotZero() const
	{
		#if TIME_IMPL == TIME_CHRONO
			return internal != Details::delta_t::zero();
		#else
			return internal != 0;
		#endif
	}

	bool					Delta::IsZero() const
	{
		#if TIME_IMPL == TIME_CHRONO
			return internal == Details::delta_t::zero();
		#else
			return internal == 0;
		#endif
	}

	bool					Delta::IsPositiveNonZero() const
	{
		#if TIME_IMPL == TIME_CHRONO
			return internal > Details::delta_t::zero();
		#else
			return internal > 0;
		#endif
	}

	bool					Delta::IsNegativeNonZero() const
	{
		#if TIME_IMPL == TIME_CHRONO
			return internal < Details::delta_t::zero();
		#else
			return internal < 0;
		#endif
	}

	Delta					Delta::operator*(double factor) const
	{
		#if TIME_IMPL == TIME_CHRONO
			return Delta(std::chrono::duration_cast<Details::delta_t>(internal * factor),false);
		#else
			return Delta(Details::delta_t(factor * internal),false);
		#endif;
	}

	Delta					Delta::operator-() const
	{
		return Delta(-internal,false);
	}



	/*static*/ const Delta	Delta::Zero = Delta(Details::ZeroDelta(),false);




	
	namespace Literals
	{
	
		Delta operator "" _d(long double days)
		{
			return Delta(days * (60ll * 60 * 24));
		}

		Delta operator "" _d(unsigned long long days)
		{
			return Delta(long double(days * (60ll * 60 * 24)));
		}
	
		Delta operator "" _h(long double hours)
		{
			return Delta(hours * (60ll * 60));
		}
	
		Delta operator "" _h(unsigned long long hours)
		{
			return Delta(long double(hours * (60ll * 60)));
		}
	
		Delta operator "" _m(long double minutes)
		{
			return Delta(minutes * (60));
		}
	
		Delta operator "" _m(unsigned long long minutes)
		{
			return Delta(long double(minutes * (60)));
		}
	
		Delta operator "" _s(long double seconds)
		{
			return Delta(seconds);
		}
	
		Delta operator "" _s(unsigned long long seconds)
		{
			return Delta(long double(seconds));
		}
	
		Delta operator "" _ms(long double millis)
		{
			return Delta(millis*1e-3l);
		}
	
		Delta operator "" _ms(unsigned long long millis)
		{
			return Delta(millis*1e-3l);
		}
	
		Delta operator "" _us(long double microseconds)
		{
			return Delta(microseconds*1e-6l);
		}
	
		Delta operator "" _us(unsigned long long microseconds)
		{
			return Delta(microseconds*1e-6l);
		}

		Delta operator "" _ns(long double nanos)
		{
			return Delta(nanos*1e-9l);
		}

		Delta operator "" _ns(unsigned long long nanos)
		{
			return Delta(nanos*1e-9l);
		}

	}

	#if 0













	Timer timer;


	Timer::Timer()
	{
		#if SYSTEM==WINDOWS
    
			DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
			if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency))
			{
				MessageBoxA(NULL,"not available", "timer-error",MB_OK);
				return;
			}
			SetThreadAffinityMask(GetCurrentThread(), oldmask);
			dresolution = 1.0/(double)frequency;
			fresolution = (float)dresolution;
		#elif SYSTEM==UNIX
			frequency = 1000000;
			dresolution = 0.000001;
			fresolution = 0.000001;
        
			// timeval t;
			// gettimeofday(&t,NULL);
			// started = frequency*t.tv_sec + t.tv_usec;
		#endif
		lastcall = now();
	}


	bool	Timer::CheckIfOutdatedAndUpdate(Time&stamp, float maxAgeInSeconds)	const
	{
		Time now = Now();
		if (stamp + ToTicks(maxAgeInSeconds) < now)
		{
			stamp = now;
			return true;
		}
		return false;
	}



	float Timer::getDeltaf()
	{
		Time   time,delta;
		#if SYSTEM==WINDOWS
			DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			SetThreadAffinityMask(GetCurrentThread(), oldmask);
		#elif SYSTEM==UNIX
			timeval t;
			gettimeofday(&t,NULL);
			time = frequency*t.tv_sec + t.tv_usec;
		#endif
		delta=time-lastcall;
		lastcall = time;
		return fresolution*delta;
	}

	double  Timer::getDeltad()
	{
		Time   time,delta;
		#if SYSTEM==WINDOWS
			DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			SetThreadAffinityMask(GetCurrentThread(), oldmask);
		#elif SYSTEM==UNIX
			timeval t;
			gettimeofday(&t,NULL);
			time = frequency*t.tv_sec + t.tv_usec;
		#endif

		delta=time-lastcall;
		lastcall = time;
		return dresolution*delta;
	}

	Timer::Time Timer::getDeltai()
	{
		Time   time,delta;
		#if SYSTEM==WINDOWS
			DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			SetThreadAffinityMask(GetCurrentThread(), oldmask);
		#elif SYSTEM==UNIX
			timeval t;
			gettimeofday(&t,NULL);
			time = frequency*t.tv_sec + t.tv_usec;
		#endif
		delta = time-lastcall;
		lastcall = time;
		return delta;
	}


	Timer::Time					Timer::GetAgeOf(Time stamp)					const
	{
		return now() - stamp;
	}

	float					Timer::GetSecondsSince(Time stamp)				const
	{
		return toSecondsf(now() - stamp);
	}

	double					Timer::GetSecondsSinceD(Time stamp)		const
	{
		return toSecondsd(now() - stamp);
	}



	Timer::Time Timer::now()	const
	{
		#if SYSTEM==WINDOWS
			Time time;
			DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			SetThreadAffinityMask(GetCurrentThread(), oldmask);
			return time;
		#elif SYSTEM==UNIX
			timeval t;
			gettimeofday(&t,NULL);
			return frequency*t.tv_sec + t.tv_usec;
		#endif
	}

	Timer::Time Timer::netTime()	const
	{
		#if SYSTEM==WINDOWS
			Time time;
			DWORD oldmask=SetThreadAffinityMask(GetCurrentThread(), 1);
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			SetThreadAffinityMask(GetCurrentThread(), oldmask);
			return time*1000000/frequency;
		#elif SYSTEM==UNIX
			timeval t;
			gettimeofday(&t,NULL);
			return frequency*t.tv_sec + t.tv_usec;
		#endif
	}

	Timer::Time			Timer::netToLocalTicks(const Time&mseconds)	const
	{
		return static_cast<Time>(static_cast<double>(mseconds)*static_cast<double>(frequency) /1000000.0);

	}
	Timer::Time			Timer::localToNetTicks(const Time&ticks)		const
	{
		return static_cast<Time>(static_cast<double>(ticks)*1000000.0*dresolution);
	}


	/*static*/ float           Timer::netToSecondsf(const Time&ticks)
	{
		return (float)ticks/1000000.0f;
	}

	/*static*/ double          Timer::netToSecondsd(const Time&ticks)
	{
		return (double)ticks/1000000.0;
	}

	/*static*/ Timer::Time          Timer::netToTicks(double seconds)
	{
		return static_cast<Time>(seconds*1000000.0);
	}



	Timer::Time Timer::getTicksPerSecond()	const
	{
		return frequency;
	}

	float Timer::toSecondsf(const Time&ticks)	const
	{
		return fresolution*ticks;
	}

	Timer::Time	Timer::toTicks(double seconds)	const
	{
		return (Time)(seconds*frequency);
	}


	double Timer::toSecondsd(const Time&ticks)	const
	{
		return dresolution*ticks;
	}
	#endif
}
