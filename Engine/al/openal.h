#ifndef openalH
#define openalH

#include "al.h"
#include "alc.h"
#include "efx.h"
#include "efx-creative.h"
#include "xram.h"
#include "CWaves.h"


#include <math/vector.h>
#include <global_string.h>

#pragma comment(lib,"OpenAL32.lib")


namespace Engine
{

	namespace OpenAL
	{
		using namespace DeltaWorks;

		namespace Listener
		{
			void				CheckSources();	//!< Called by SetPosition automatically
		}

		class WaveBuffer
		{
		protected:
			ALuint				bufferHandle;
			friend WaveBuffer	Load(const String&filename, String*);
			friend void			Discard(WaveBuffer&wbo);
		public:
			/**/				WaveBuffer():bufferHandle(0) {}
			inline bool			IsEmpty()	const	{return bufferHandle == 0;}
			inline bool			IsNotEmpty()	const	{return bufferHandle != 0;}
			inline ALuint		getHandle()	const	{return bufferHandle;}
			inline ALuint		GetHandle()	const	{return bufferHandle;}
		};
	
		typedef WaveBuffer	Wave;
	
		struct TSourceData
		{
			ALuint			handle=0;
			M::TVec3<>		position;
			bool			isRelative=false,
							wantsToPlay=false,
							isPaused=false,
							wasPlaying=false,
							wasSwitchedOff = false,
							isLooping=false;
			float			priority = 1.f,
							gain=1.f,
							refDistance=1.f,
							rollOffFactor=1.f;
		};
	
		class Source : private TSourceData
		{
		private:
							Source(const Source&other)
							{}
			void			operator=(const Source&other)
							{}

			bool			ALIsPlaying()	const;
			bool			ALIsPaused()	const;
			float			ALGetGain()		const;
			float			ALGetReferenceDistance()	const;
			/**
			Stops the OpenAL playback
			@return true, if playback was playing and is now stopped, false otherwise
			*/
			bool			ALStop();
			bool			ALStart();


			friend void		Listener::CheckSources();
		public:
			#ifdef _DEBUG
				String		name;
				void		*reference = nullptr;
			#endif
			/**/			Source();
			/**/			Source(Source&&fleeting);
			virtual			~Source();
	
			bool			Create();
			bool			IsCreated()	const;
			void			Destroy();
			
			void			SetWaveBuffer(const WaveBuffer&wbo);
			void			ReplaceWaveBuffer(const WaveBuffer&wbo);
			void			EnqueueWaveBuffer(const WaveBuffer&wbo);
			void			ClearQueue();
			void			Rewind();
			void			Play();
			void			Stop();
			bool			IsPlaying()	const	{return wantsToPlay;}
			void			Pause();
			bool			IsPaused()	const;
			void			SetLooping(bool do_loop);
			bool			IsLooping()	const;
			void			SetVelocity(const M::TVec3<>&velocity);
			void			SetVelocity(float x, float y, float z);
			void			GetVelocity(M::TVec3<>&velocity_out)	const;
			void			SetPosition(float x, float y, float z, bool relative=false);
			void			SetPosition(const M::TVec3<>&position, bool relative=false);
			void			GetLocation(M::TVec3<>&positionOut);
			inline void		GetPosition(M::TVec3<>&positionOut)	{GetLocation(positionOut);}
			bool			IsRelativeLocation()	const;
			void			SetPitch(float pitch);
			float			GetPitch()	const;
			void			SetGain(float gain);
			float			GetGain()		const	{return gain;}
			inline void		SetVolume(float volume)	{SetGain(volume);}
			inline float	GetVolume()		const	{return gain;}
			void			SetReferenceDistance(float distance, float rolloffFactor = 1.f);
			float			GetReferenceDistance()	const	{return refDistance;}
			float			GetRollOffFactor() const		{return rollOffFactor;}

			bool			CheckQueueIsDone()	const;
			void			UpdatePriority();
			inline float	GetPriority() const {return priority;}

			void			swap(Source&other);
			friend void		swap(Source&a, Source&b)	{a.swap(b);}
		};
	
		typedef Source	Player;

		/**
		Initializes the global device handle and audio context.
		This must be called before any sound operations are executed. Any listener update, wave loading operation, or source creation will fail silently otherwise.
		You can avoid calling Init() in order to completely disable sound output of an otherwise unmodified application
		*/
		bool				Init();
		bool				IsInitialized();
		/**
		Destroys the global OpenAL context and device handle.
		Does not destroy audio sources but playback will probably stop on its own.
		*/
		void				Shutdown();
	
		WaveBuffer			Load(const String&filename, String*error_out = NULL);
		void				Discard(WaveBuffer&wbo);
	
		void				SetSpeedOfSound(float speed);
		float				GetSpeedOfSound();
	
	
		bool				PlayOnce(float x, float y, float z, const WaveBuffer&buffer, float gain=1.f, float referenceDistance=1.f, bool relative=false);
		bool				PlayOnce(const M::TVec3<>&position, const WaveBuffer&buffer, float gain=1.f, float referenceDistance=1.f, bool relative=false);
	
		/**
		Signals OpenAL that a an existing window focus has been lost.
		This mutes all OpenAL sound output. Calling this function multiple times has no effect.
		*/
		void				SignalWindowFocusRestoration();
		/**
		Signals OpenAL that a previously existing window focus has been restored.
		This reenables sound output. Calling this function multiple times has no effect.
		*/
		void				SignalWindowFocusLoss();
	
		ALenum				FindFormat(count_t numChannels, count_t numBitsPerSample);

		namespace Listener
		{
			void					CheckSources();	//!< Called by SetPosition automatically

			void					SetOrientation(const M::TVec3<>&direction, const M::TVec3<>&up, bool negate_direction=false, bool negate_up=false);
			const M::TVec3<>&		GetDirectionVector();
			const M::TVec3<>&		GetUpVector();
			void					SetVelocity(float x, float y, float z);
			void					SetVelocity(const M::TVec3<>&velocity);
			const M::TVec3<>&		GetVelocity();
			/**
			Changes the listener position in the world.
			This function doubles as a priority cleanup. It is assumed you call this function around once per frame.
			*/
			void					SetPosition(const M::TVec3<>&position);
			const M::TVec3<>&		GetLocation();
			inline const M::TVec3<>&GetPosition()	{return GetLocation();}
			void					SetGain(float volume);

			void					SetMaxPlayingSources(count_t);
			count_t					GetMaxPlayingSources();

			/**
			Registers a callback function that is called in the event that the maximum number of concurrently playing audio sources would be reduced as a result of playback errors
			@param f Function to call. If the function returns false, the reduction is not executed, but chances are @a f will be called very often in this case. If @a f is NULL then callback will be disabled
			*/
			void					RegisterMaxPlayingSourcesCallback(bool (*f)(count_t));

		}



	}

}


#endif
