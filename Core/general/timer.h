#ifndef timerH
#define timerH

/******************************************************************

Plattform independent time-query-class.

******************************************************************/

#include "../global_root.h"

#undef TIME_IMPL
#undef TIME_CHRONO
#undef TIME_SYS
#undef TIME_WIN

#define TIME_CHRONO 1
#define TIME_SYS 2
#define TIME_WIN 3
#define TIME_WIN_QUERY 4

#define TIME_IMPL	TIME_CHRONO


#ifdef __has_include
	#if !__has_include(<chrono>)
		#if __has_include(<sys/time.h>)
			#undef TIME_IMPL
			#define TIME_IMPL	TIME_SYS
		#endif
		#if __has_include(<Windows.h>)
			#undef TIME_IMPL
			#define TIME_IMPL	TIME_WIN
		#endif
	#endif
#endif


#if TIME_IMPL == TIME_SYS
	#include <sys/time.h>
#endif
#if TIME_IMPL == TIME_WIN
	//#include <Windows.h>
#endif
#if TIME_IMPL == TIME_CHRONO
	#include <chrono>
#endif


/**
High-precision timing helper classes
*/
namespace DeltaWorks::Time
{
	namespace Details
	{
		#if TIME_IMPL == TIME_CHRONO
			typedef typename std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type hr_steady_clock;


			typedef hr_steady_clock chrono_clock_t;
			typedef std::chrono::time_point<chrono_clock_t> stamp_t;
			typedef stamp_t::duration delta_t;
		#else
			typedef UINT64 stamp_t;
			typedef INT64 delta_t;
		#endif
	}


	class Delta;
	class Stamp;

	/**
	Micro-second timestamp, used for network time transmission
	where time resolution may vay.
	*/
	class MicroStamp
	{
	public:
		/**/					MicroStamp(const Stamp&);
		/**/					MicroStamp(UINT64 ticks):internal(ticks)	{}
		UINT64					GetTicks() const	{return internal;}
	private:
		UINT64					internal;
	};

	/**
	Maximum precision time stamp. Auto-initialized to current time
	*/
	class Stamp
	{
	public:
		/**/					Stamp();
		/**/					Stamp(const MicroStamp&);

		Delta					operator-(const Stamp&other) const;
		Stamp					operator+(const Delta&delta) const;
		Stamp					operator-(const Delta&delta) const;
		Stamp&					operator+=(const Delta&delta);
		Stamp&					operator-=(const Delta&delta);

		/**
		Updates the local time stamp to the current time.
		Semantically identical to *this = Stamp();
		*/
		void					Update();

		/**
		Calculates the delta between the current timestamp and the local one
		*/
		Delta					Elapsed() const;
		float					ElapsedSeconds() const;

		UINT64					ToMicroSecondsU() const;

		UINT64					ToTicks() const;
		static Stamp			FromTicks(UINT64 ticks);

		/**
		Checks if the local time stamp is older than the specified maximum age,
		and updates if so. Returns true if updated, false if not outdated and unchanged.
		*/
		bool					CheckIfOutdatedAndUpdate(const Delta&maxAge);
	private:
		friend class Delta;

		/**/					Stamp(const Details::stamp_t&, bool);
		Details::stamp_t		internal;
	};


	/**
	High precision time difference between two time stamps
	*/
	class Delta
	{
	private:
		explicit				Delta(Details::delta_t internal);
	public:
		static const Delta		Zero;

		/**
		Initializes the local time delta with zero
		*/
		/**/					Delta();
		explicit				Delta(long double seconds);
		explicit				Delta(double seconds);

		float					ToSeconds() const;
		double					ToSecondsD() const;

		static Delta			FromMilliseconds(long long ms);
		static Delta			FromSeconds(long long s);

		bool					IsInRange(const Delta&min, const Delta&max) const;
		Delta					operator+(const Delta&other) const;
		Delta&					operator+=(const Delta&other);
		Delta					operator-(const Delta&other) const;
		Delta&					operator-=(const Delta&other);
		bool					operator>(const Delta&other) const;
		bool					operator>=(const Delta&other) const;
		bool					operator<(const Delta&other) const;
		bool					operator<=(const Delta&other) const;
		bool					operator==(const Delta&other) const;
		bool					operator!=(const Delta&other) const;
		bool					IsNotZero() const;
		bool					IsZero() const;
		bool					IsPositiveNonZero() const;
		bool					IsNegativeNonZero() const;
		Delta					operator*(double factor) const;
		Delta					operator-() const;

		Details::delta_t		GetInternal() const {
			return internal;
		}
	private:
		friend class Stamp;

		/**/					Delta(const Details::delta_t&, bool);
		Details::delta_t		internal;
	};


	namespace Literals
	{
		Delta operator "" _d(long double days);
		Delta operator "" _d(unsigned long long days);
		Delta operator "" _h(long double hours);
		Delta operator "" _h(unsigned long long hours);
		Delta operator "" _m(long double minutes);
		Delta operator "" _m(unsigned long long minutes);
		Delta operator "" _s(long double seconds);
		Delta operator "" _s(unsigned long long seconds);
		Delta operator "" _ms(long double millis);
		Delta operator "" _ms(unsigned long long millis);
		Delta operator "" _us(long double microseconds);
		Delta operator "" _us(unsigned long long microseconds);
		Delta operator "" _ns(long double nanos);
		Delta operator "" _ns(unsigned long long nanos);
	}
}

#endif
