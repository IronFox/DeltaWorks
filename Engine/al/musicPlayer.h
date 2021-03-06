#pragma once

#include "decoder.h"
#include <general/thread_system.h>
#include "openal.h"
#include <general/timer.h>

namespace Engine
{
	namespace OpenAL
	{
		class MusicPlayer : public Sys::ThreadObject
		{
		public:
			typedef Sys::ThreadObject	Super;

			/**/			MusicPlayer() {};
			/**/			MusicPlayer(const MusicPlayer&)=delete;
			virtual			~MusicPlayer();
			void			operator=(const MusicPlayer&)=delete;

			/**
			Begins playback of the specified file source.
			Throws exception on failure
			*/
			void			Begin(const PathString&source, bool loop, float fadeInSeconds = 0.5f);
			/**
			Attempts to begin playback of the specified file source.
			Returns false on failure
			*/
			bool			TryBegin(const PathString&source, bool loop, float fadeInSeconds = 0.5f);

			/**
			Fetches the text associated with the last error and resets it to empty.
			Use if TryBegin() returned false
			*/
			String			GetLastErrorText();

			/**
			Updates playback volume.
			*/
			void			SetVolume(float volume);

			float			GetVolume() const	{return volume;}
			/**
			Checks if the local player is playing and not fading out
			*/
			bool			IsPlaying() const	{return isPlaying && !doFade;}
			bool			IsFadingOut() const {return doFade && isPlaying;}

			void			ThreadMain() override;
			void			Stop();
			void			FadeOut(float fadeOutSeconds);
		private:
			bool			GetData(ALuint target, ALenum format, count_t samplesPerSecond);

			Timer::Time		fadeOutStarted,fadeInStarted;
			float			fadeOutSeconds,fadeInSeconds;
			String			exception;
			Sys::Mutex		exceptionMutex;
			Array<BYTE>		buffer;
			count_t			bufferFrameSize;
			float			volume = 1;
			AudioDecoder	decoder;
			PathString		source;
			bool			loop,doFadeIn=false,isPlaying=false;
			volatile bool	quit=false,doFade=false;
		};






	}







}
