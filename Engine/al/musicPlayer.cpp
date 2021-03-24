#include "musicPlayer.h"

namespace Engine
{
	namespace OpenAL
	{


		void	MusicPlayer::Stop()
		{
			if (!Super::IsActive())
				return;
			quit = true;
			
		}

		void	MusicPlayer::FadeOut(float fadeOutSeconds)
		{
			this->fadeOutSeconds = fadeOutSeconds;
			this->fadeOutStarted.Update();
			this->doFade = true;
		}


		bool MusicPlayer::GetData(ALuint target, ALenum format, count_t samplesPerSecond)
		{
			if (decoder.IsAtEnd())
			{
				if (loop)
					decoder.Rewind();
				else
					return false;
			}
			count_t framesRead = decoder.ReadFrames(bufferFrameSize,buffer.pointer());
			alBufferData(target, format, buffer.pointer(), ALsizei( framesRead * decoder.GetFrameByteSize()), ALsizei(samplesPerSecond));
			return true;
		}



		void	MusicPlayer::Begin(const PathString&source, bool loop, float fadeInSeconds /*= 0.5f*/)
		{
			if (!TryBegin(source,loop,fadeInSeconds))
			{
				String ex;
				exceptionMutex.lock();
				ex.swap(exception);
				exceptionMutex.unlock();
				throw Except::Exception(CLOCATION,ex);
			}
		}




		bool	MusicPlayer::TryBegin(const PathString&source, bool loop, float fadeInSeconds /*= 0.5f*/)
		{
			Stop();

			Super::AwaitCompletion();
			if (exception.IsNotEmpty())
				return false;

			this->source = source;
			this->loop = loop;
			this->doFade = false;
			this->quit = false;
			doFadeIn = fadeInSeconds > 0;
			this->fadeInSeconds = fadeInSeconds;
			fadeInStarted.Update();
			Super::Start();
			return true;
		}

		String			MusicPlayer::GetLastErrorText()
		{
			String rs;
			exceptionMutex.lock();
			exception.swap(rs);
			exceptionMutex.unlock();
			return rs;
		}




			 
		MusicPlayer::~MusicPlayer()
		{
		}

		void	MusicPlayer::SetVolume(float volume)
		{
			this->volume = volume;
		}


		void	MusicPlayer::ThreadMain()
		{
			isPlaying = true;
			ALuint			output=0,buffer[4]={0};
			try
			{
				decoder.Open(source);

				alGenSources( 1, &output );
				alGenBuffers(ARRAYSIZE(buffer),buffer);

				const count_t framesPerSecond = decoder.GetFramesPerSecond();
				const count_t numChannels = decoder.GetChannels();

				bufferFrameSize = framesPerSecond*5;
				this->buffer.SetSize(bufferFrameSize*numChannels*decoder.GetBitsPerSample()/8);

				const ALenum format = FindFormat(numChannels,decoder.GetBitsPerSample());

				for (index_t i = 0; i < ARRAYSIZE(buffer); i++)
					GetData(buffer[i],format,framesPerSecond*numChannels);

				alSourceQueueBuffers(output,ARRAYSIZE(buffer),buffer);


				float lastVolume = volume;
				if (doFadeIn)
					alSourcef(output,AL_GAIN,0.f);
				else
					alSourcef(output,AL_GAIN,0.5f*volume);

				alSourcePlay(output);


				while (!quit)
				{
					if (!doFade && !doFadeIn)
						Sleep(100);
					else
						Sleep(1);
					ALint	processed = 0;
					alGetSourcei(output, AL_BUFFERS_PROCESSED, &processed);
					while (processed > 0)
					{
						ALuint uiBuffer = 0;
						alSourceUnqueueBuffers(output, 1, &uiBuffer);
						if (!GetData(uiBuffer,format,framesPerSecond*numChannels))
							quit = true;
						else
							alSourceQueueBuffers(output,1,&uiBuffer);
						processed--;
					}

					if (doFade)
					{
						float fadeAt = fadeOutStarted.ElapsedSeconds() / fadeOutSeconds;
						if (fadeAt > 1)
						{
							break;
						}

						alSourcef(output,AL_GAIN,M::CubicFactor(1.f-fadeAt) * 0.5f * volume);
						
					}
					elif (doFadeIn)
					{
						float fadeAt = fadeInStarted.ElapsedSeconds() / fadeInSeconds;
						if (fadeAt > 1)
						{
							doFadeIn = false;
							fadeAt = 1;
						}
						alSourcef(output,AL_GAIN,M::CubicFactor(fadeAt) * 0.5f * volume);
					}
					else
						if (lastVolume != volume)
						{
							lastVolume = volume;
							alSourcef(output,AL_GAIN,0.5f*volume);
						}
				}
				alSourceStop(output);

				decoder.Clear();

				alDeleteSources(1,&output);
				alDeleteBuffers(ARRAYSIZE(buffer),buffer);

			}
			catch (const std::exception&ex)
			{
				if (output)
					alDeleteSources(1,&output);
				if (buffer[0])
					alDeleteBuffers(ARRAYSIZE(buffer),buffer);
				exceptionMutex.lock();
				exception = ex.what();
				exceptionMutex.unlock();
			}
			catch(...)
			{}
			isPlaying = false;
		}






	}







}
