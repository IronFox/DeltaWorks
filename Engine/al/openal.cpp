#include "openal.h"
#include <general/timer.h>
#include <container/buffer.h>
#include <container/sorter.h>

#include <cstdlib>
#include <iostream>
 
//using namespace std;

namespace Engine
{

	namespace OpenAL
	{
		static Source	source_field[0x100];
	
		namespace Status
		{
			bool		focusLostMuted = false;

			bool		fadeGainIn = false;
			Timer::Time	fadeGainFrom;
			float		fadeGainSeconds = 0;

			CWaves		wave_loader;
			bool		initialized = false;
			ALCdevice	*pDevice = NULL;
			ALCcontext *pContext = NULL;
			float		globalGainScale = 1.f;
			float		globalMaxOverVolume = 1.f;	//Maximum absolute gain of any source with a gain > 1.f. If negative must be redetected among all
			bool		globalMaxOverVolumeChanged = false;	//Indicates that globalMaxOverVolume has changed.
		}



		namespace Registry
		{
	//		Buffer0<ALuint>	createdHandles;

			Buffer0<Source*>	playingSources;

			void	Replace(Source*toFind, Source*replaceWith);
			void	Remove(Source*);
			void	Add(Source*);
		}

	
		bool				PlayOnce(float x, float y, float z, const WaveBuffer&buffer, float gain/*=1.f*/, float referenceDistance/*=1.f*/, bool relative/*=false*/)
		{
			M::TVec3<>	field = {x,y,z};
			return PlayOnce(field,buffer,gain,referenceDistance,relative);
		}
	
		bool				PlayOnce(const M::TVec3<>&position, const WaveBuffer&buffer, float gain/*=1.f*/, float referenceDistance/*=1.f*/, bool relative/*=false*/)
		{
			for (index_t i = 0; i < ARRAYSIZE(source_field); i++)
			{
				if (source_field[i].IsPlaying())
					continue;
				if (!source_field[i].IsCreated())
					source_field[i].Create();
				source_field[i].SetPosition(position,relative);
				source_field[i].SetWaveBuffer(buffer);
				source_field[i].SetGain(gain);
				source_field[i].SetReferenceDistance(referenceDistance);
				source_field[i].Play();
				return true;
			}
			return false;
		}

	
	
	
	
	
	
		Source::Source()
		{}

		Source::Source(Source&&fleeting)
		{
			handle = fleeting.handle;
			fleeting.handle = 0;
		}

	
		Source::~Source()
		{
			Destroy();

			DBG_ASSERT__(!Registry::playingSources.Contains(this));
		}
	
	
		bool	Source::Create()
		{
			if (handle)
				return true;
			if (!Status::initialized)
				return false;
			alGenSources( 1, &handle );
			//Registry::createdHandles << handle;
			return handle != 0;
		}
	
		bool	Source::IsCreated()	const
		{
			return handle != 0;
		}
	
		void	Source::Destroy()
		{
			if (!handle)
				return;
			if (wantsToPlay)
				Registry::Remove(this);
			//ASSERT__(Registry::createdHandles.FindAndErase( handle ));
			alDeleteSources(1,&handle);
			handle = 0;
		}
	
			
		void	Source::ReplaceWaveBuffer(const WaveBuffer&wbo)
		{
			alSourcei( handle, AL_BUFFER, wbo.getHandle() );		
		}
	
		void	Source::SetWaveBuffer(const WaveBuffer&wbo)
		{
			alSourcei( handle, AL_BUFFER, wbo.getHandle() );
		}
	
		void	Source::EnqueueWaveBuffer(const WaveBuffer&wbo)
		{
			ALuint whandle = wbo.getHandle();
			alSourceQueueBuffers(handle,1,&whandle);
		}
	
		void	Source::ClearQueue()
		{
			Stop();
			if (handle)
			{
				ALint	processed = 0;
				alGetSourcei(handle, AL_BUFFERS_PROCESSED, &processed);
				while (processed > 0)
				{
					ALuint uiBuffer = 0;
					alSourceUnqueueBuffers(handle, 1, &uiBuffer);
					processed--;
				}
			}
		}
	
		void	Source::Rewind()
		{
			alSourceRewind(handle);
		}

		bool	Source::ALStop()
		{
			if (!ALIsPlaying())
				return false;
			wasPlaying = true;
			wasSwitchedOff = true;
			alSourceStop(handle);
			return true;
		}

		bool	Source::ALStart()
		{
			if (ALIsPlaying())
				return true;
			alGetError();
			alSourcePlay(handle);
			ALenum error = alGetError();
			if (error == AL_NO_ERROR)
			{
				wasPlaying = true;
				wasSwitchedOff = false;
				return true;
			}
			return false;
		}

	
		void	Source::Play()
		{
			if (!handle)
				return;
			if (ALIsPlaying())
				return;
			if (!wantsToPlay)
			{
				wantsToPlay = true;
				Registry::Add(this);
				ALStart();	//try
			}
			isPaused = false;
			//return alGetError()==AL_NO_ERROR;
		}
	
		void	Source::Stop()
		{
			if (wantsToPlay)
			{
				Registry::Remove(this);
				alSourceStop(handle);
			}
			wantsToPlay = false;
			wasPlaying = false;


	//		alSourceStop(handle);
		}
	
		bool	Source::CheckQueueIsDone()	const
		{
			if (handle == 0)
				return true;
			return !ALIsPlaying() && (!wasSwitchedOff || !wantsToPlay || !wasPlaying);
			//ALint	queued = 0;
			//alGetSourcei(handle, AL_BUFFERS_QUEUED, &queued);
			//ALint	iState = 0;
			//alGetSourcei(handle, AL_SOURCE_STATE, &iState);
			//return !queued && iState != AL_PLAYING;
		}

		bool	Source::ALIsPlaying()	const
		{
			if (!handle)
				return false;
			ALint	iState = 0;
			alGetSourcei(handle, AL_SOURCE_STATE, &iState);
			return iState == AL_PLAYING;
		}
	
		void	Source::Pause()
		{
			alSourcePause(handle);
			isPaused = true;
		}
	
		bool	Source::ALIsPaused()	const
		{
			ALint	iState = 0;
			alGetSourcei(handle, AL_SOURCE_STATE, &iState);
			return iState == AL_PAUSED;
		}
	
		void	Source::SetLooping(bool do_loop)
		{
			isLooping = do_loop;
			alSourcei(handle, AL_LOOPING, do_loop?AL_TRUE:AL_FALSE);
		}
	
		bool	Source::IsLooping()	const
		{
			ALint	iLoop = 0;
			alGetSourcei(handle, AL_LOOPING, &iLoop);
			return iLoop == AL_TRUE;
		}
	
		void	Source::SetVelocity(float x, float y, float z)
		{
			M::TVec3<> v = {x,y,z};
			SetVelocity(v);
		}
	
		void	Source::SetVelocity(const M::TVec3<>&velocity)
		{
			alSourcefv(handle, AL_VELOCITY, velocity.v);
		}
	
		void	Source::GetVelocity(M::TVec3<>&velocity_out)	const
		{
			alGetSourcefv(handle, AL_VELOCITY, velocity_out.v);
		}
	
		void	Source::SetPosition(const M::TVec3<>&position, bool relative)
		{
			alSourcefv(handle, AL_POSITION, position.v);
			alSourcei(handle,AL_SOURCE_RELATIVE,relative?AL_TRUE:AL_FALSE);
			this->position = position;
			this->isRelative = relative;
		}
	
		void	Source::SetPosition(float x, float y, float z, bool relative)
		{
			M::TVec3<> v = {x,y,z};
			SetPosition(v,relative);
		}
	
		void	Source::GetLocation(M::TVec3<>&position_out)
		{
			alGetSourcefv(handle, AL_POSITION,position_out.v);
		}
	
	
		bool	Source::IsRelativeLocation()	const
		{
			return isRelative;
			//ALint	val;
			//alGetSourcei(handle,AL_SOURCE_RELATIVE,&val);
			//return val == AL_TRUE;
		}
	
		void	Source::SetPitch(float pitch)
		{
			alSourcef(handle,AL_PITCH,pitch);
		}
	
		float	Source::GetPitch()		const
		{
			ALfloat val;
			alGetSourcef(handle,AL_PITCH,&val);
			return val;
		}
		
		void	Source::SetGain(float gain)
		{
			alSourcef(handle,AL_GAIN,std::min(1.f,gain * Status::globalGainScale));
			if (gain > 1.f)
			{
				if (gain > Status::globalMaxOverVolume && Status::globalMaxOverVolume > 0.f)
				{
					Status::globalMaxOverVolume = gain;
					Status::globalMaxOverVolumeChanged = true;
				}
			}
			if (gain < this->gain && this->gain > 1.f && this->gain == Status::globalMaxOverVolume)
			{
				Status::globalMaxOverVolume = -1;
				Status::globalMaxOverVolumeChanged = true;
			}
			this->gain = gain;
		}
	
		float	Source::ALGetGain()		const
		{
		
			ALfloat val;
			alGetSourcef(handle,AL_GAIN,&val);
			return val;
		}
	
		void	Source::SetReferenceDistance(float distance, float rolloffFactor)
		{
			alSourcef(handle,AL_REFERENCE_DISTANCE,distance);
			alSourcef(handle,AL_ROLLOFF_FACTOR,rolloffFactor);
			this->refDistance = distance;
			this->rollOffFactor = rollOffFactor;
		}
	
		float	Source::ALGetReferenceDistance()	const
		{
			ALfloat val;
			alGetSourcef(handle,AL_REFERENCE_DISTANCE,&val);
			return val;
		}
	

		void	Registry::Replace(Source*toFind, Source*replaceWith)
		{
			if (!Status::initialized)
				return;
			index_t at = playingSources.GetIndexOf(toFind);
			ASSERT__(at != InvalidIndex);
			playingSources[at] = replaceWith;
		}

		void	Registry::Remove(Source*s)
		{
			if (!Status::initialized)
				return;

			DBG_VERIFY__(playingSources.FindAndErase(s));
			DBG_ASSERT__(!playingSources.Contains(s));
		}
		void	Registry::Add(Source*s)
		{
			if (!Status::initialized)
				return;

			DBG_ASSERT__(!playingSources.Contains(s));
			playingSources << s;
		}



		void			Source::swap(Source&other)
		{
			swp((TSourceData&)*this,(TSourceData&)other);
			#ifdef _DEBUG
				name.swap(other.name);
			#endif
			if (wantsToPlay && !other.wantsToPlay)
				Registry::Replace(&other,this);
			elif (!wantsToPlay && other.wantsToPlay)
				Registry::Replace(this,&other);
		}


		bool	Init()
		{
			using namespace Status;
			if (initialized)
			{
				return true;
			}
			const char*defaultDeviceName = (const char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
			if (!defaultDeviceName || !strlen(defaultDeviceName))
			{
				std::cout << "OpenAL: no default device available\n";
				return false;
			}
			std::cout << "OpenAL: default device is '"<<defaultDeviceName<<"'"<<std::endl;
			
		
			pDevice = alcOpenDevice(defaultDeviceName);
			if (pDevice)
			{
				std::cout << "OpenAL: device opened\n";
				pContext = alcCreateContext(pDevice, NULL);
				if (pContext)
				{
					std::cout << "OpenAL: context created\n";
					alcMakeContextCurrent(pContext);
					initialized = true;
					std::cout << "OpenAL: all done\n";
					return true;
				}
				else
				{
					std::cout << "OpenAL: context creation failed. closing device...\n";
					alcCloseDevice(pDevice);
				}
			}
			std::cout << "OpenAL: all done. initialization failed. returning\n";
			return false;
		}

		void	Shutdown()
		{
			alcDestroyContext(Status::pContext);
			alcCloseDevice(Status::pDevice);
			Status::initialized = false;
		}

	
		bool				IsInitialized()
		{
			return Status::initialized;
		}

	
		WaveBuffer			Load(const String&filename, String*error_out)
		{
			WaveBuffer	result;
			if (!Status::initialized)
				return result;
		
			WAVEID			WaveID;
			ALint			iDataSize, iFrequency;
			ALenum			eBufferFormat;
			ALchar			*pData;

			if (error_out)
				(*error_out) = "No error";

			using namespace Status;

			WAVERESULT rs = wave_loader.LoadWaveFile(filename.c_str(), &WaveID);
			if (WV_SUCCEEDED(rs))
			{
				WAVEFORMATEX	format;
				if ((WV_SUCCEEDED(wave_loader.GetWaveSize(WaveID, (unsigned long*)&iDataSize))) &&
					(WV_SUCCEEDED(wave_loader.GetWaveData(WaveID, (void**)&pData))) &&
					(WV_SUCCEEDED(wave_loader.GetWaveFrequency(WaveID, (unsigned long*)&iFrequency))) &&
					(WV_SUCCEEDED(wave_loader.GetWaveFormatExHeader(WaveID, &format))) 
					)
				{
					eBufferFormat = FindFormat(format.nChannels,format.wBitsPerSample);
					alGetError();
					alGenBuffers(1,&result.bufferHandle);
					alGetError();
					alBufferData(result.bufferHandle, eBufferFormat, pData, iDataSize, iFrequency);
					if (alGetError() != AL_NO_ERROR)
					{
						if (error_out)
							(*error_out) = "OpenAL internal error";
					
						alDeleteBuffers(1,&result.bufferHandle);
						result.bufferHandle = 0;
					}
				}
				wave_loader.DeleteWaveFile(WaveID);
			}
			else
				if (error_out)
				{
					(*error_out) = "Failed to load file '"+filename+"'";
					#undef ECASE
					#define ECASE(_CASE_)	case _CASE_: (*error_out) += " ("#_CASE_")"; break;
					switch (rs)
					{
						ECASE(WR_OK)
						ECASE(WR_OUTOFMEMORY)
						ECASE(WR_BADWAVEFILE)
						default:
							(*error_out) += " (unknown error)";
					}	
				}

			return result;
		}
	
		void				Discard(WaveBuffer&wbo)
		{
			if (!wbo.bufferHandle)
				return;
			alDeleteBuffers(1,&wbo.bufferHandle);
			wbo.bufferHandle = 0;
		}
	
	
		void				SetSpeedOfSound(float speed)
		{
			alSpeedOfSound(speed);
		}
	
		float				GetSpeedOfSound()
		{
			return alGetFloat(AL_SPEED_OF_SOUND);
		}
	
		namespace Listener
		{
			namespace Status
			{
				M::TVec3<> velocity,location;
				count_t	maxPlayingSources = 16;
				float	baseVolume = 1.f;
				bool (*reduceCallback)(count_t) = nullptr;
			}

			void					RegisterMaxPlayingSourcesCallback(bool (*f)(count_t))
			{
				Status::reduceCallback = f;
			}


			void					SetGain(float volume)
			{
				if (OpenAL::Status::globalMaxOverVolume >= 1.f && !OpenAL::Status::globalMaxOverVolumeChanged)
					if (OpenAL::Status::focusLostMuted)
						alListenerf(AL_GAIN,0);
					else
						alListenerf(AL_GAIN,volume * OpenAL::Status::globalMaxOverVolume);
				Status::baseVolume = volume;
			}

			void			SetOrientation(const M::TVec3<float>&direction, const M::TVec3<float>&up, bool negate_direction, bool negate_up)
			{
				ALfloat field[6];
				M::TVec3<float>	&d = M::Vec::ref3(field),
								&u = M::Vec::ref3(field+3);
				d = direction;
				u = up;
				M::Vec::normalize0(d);
				M::Vec::normalize0(u);
				if (negate_direction)
					M::Vec::mult(d,-1);
				if (negate_up)
					M::Vec::mult(u,-1);
				//field[2] *= -1;
				//field[5] *= -1;
				alListenerfv(AL_ORIENTATION,field);
			}
			
			static float field[6];
			
			const M::TVec3<>&	GetDirectionVector()
			{
				alGetListenerfv(AL_ORIENTATION,field);
				return M::Vec::ref3(field);
			}
			
			const M::TVec3<>&	GetUpVector()
			{
				alGetListenerfv(AL_ORIENTATION,field);
				return M::Vec::ref3(field+3);
			}
			void			SetVelocity(float x, float y, float z)
			{
				M::TVec3<> v = {x,y,z};
				SetVelocity(v);
			}
			
			void			SetVelocity(const M::TVec3<>& velocity)
			{
				alListenerfv(AL_VELOCITY,velocity.v);
			}
			
			
			const M::TVec3<>&	GetVelocity()
			{
				alGetListenerfv(AL_VELOCITY,Status::velocity.v);
				return Status::velocity;
			}
			
			void			SetPosition(const M::TVec3<>&position)
			{
				if (!OpenAL::Status::initialized)
					return;
				//float p[3] = {position[0],position[1],-position[2]};
				alListenerfv(AL_POSITION,position.v);

				CheckSources();


			}

			int PrioritySortComparator(Source*a, Source*b)
			{
				if (a->GetPriority() > b->GetPriority())
					return -1;
				return 1;
			}

			void					SetMaxPlayingSources(count_t newCnt)
			{
				Status::maxPlayingSources = newCnt;
			}
			count_t					GetMaxPlayingSources()
			{
				return Status::maxPlayingSources;
			}



			void			CheckSources()
			{
				GetLocation();

				{
					using namespace OpenAL::Status;
					if (fadeGainIn && !focusLostMuted)
					{
						float at = timer.GetSecondsSince(fadeGainFrom) / fadeGainSeconds;
						if (at > 1)
						{
							at = 1;
							fadeGainIn = false;
						}
						alListenerf(AL_GAIN,Listener::Status::baseVolume * OpenAL::Status::globalMaxOverVolume * at);
					}
				}
			


				for (index_t i = 0; i < Registry::playingSources.Count(); i++)
				{
					Source*s = Registry::playingSources[i];
					if (s->CheckQueueIsDone())
					{
						count_t preStop = Registry::playingSources.Count();
						s->Stop();
						if (preStop = Registry::playingSources.Count()+1)
							i--;
						else
						{
							DBG_FATAL__("Source::Stop() should have removed exactly one audio source from the registry");
						}
					}
					else
					{
						s->UpdatePriority();
					}
				}
				Sorting::ByComparator::QuickSort(Registry::playingSources,PrioritySortComparator);
				for (index_t i = Status::maxPlayingSources; i < Registry::playingSources.Count(); i++)
				{
					Registry::playingSources[i]->ALStop();
				}
				for (index_t i = 0; i < Status::maxPlayingSources && i < Registry::playingSources.Count(); i++)
				{
					while (!Registry::playingSources[i]->ALStart())
					{
						bool retry = false;
						for (index_t j = i+1; j < Status::maxPlayingSources && j < Registry::playingSources.Count(); j++)
						{
							if (Registry::playingSources[j]->ALStop())
							{
								retry = true;
								break;
							}
						}

						if (!retry)
						{
							if (Status::reduceCallback)
							{
								if (Status::reduceCallback(i))
									Status::maxPlayingSources = i;
							}
							else
								if (Status::maxPlayingSources > 8)
									Status::maxPlayingSources = i;
							break;
						}
					}
				}

				if (OpenAL::Status::globalMaxOverVolumeChanged)
				{
					using namespace OpenAL::Status;
					globalMaxOverVolumeChanged = false;
					if (OpenAL::Status::globalMaxOverVolume < 1.f)
					{
						OpenAL::Status::globalMaxOverVolume = 1.f;
						for (index_t i = 0; i < Registry::playingSources.Count(); i++)
							globalMaxOverVolume = std::max(globalMaxOverVolume,Registry::playingSources[i]->gain);
					}
					globalGainScale = 1.f / OpenAL::Status::globalMaxOverVolume;
					alListenerf(AL_GAIN,Status::baseVolume * OpenAL::Status::globalMaxOverVolume);
					for (index_t i = 0; i < Status::maxPlayingSources && i < Registry::playingSources.Count(); i++)
					{
						float gain = Registry::playingSources[i]->gain * globalGainScale;
						DBG_ASSERT_LESS_OR_EQUAL__(gain,1.f);
						alSourcef(Registry::playingSources[i]->handle,AL_GAIN,gain);
					}
					

				}

			}
			
			const M::TVec3<>&	GetLocation()
			{
				alGetListenerfv(AL_POSITION,Status::location.v);
				return Status::location;
			}
			
		}

		void	Source::UpdatePriority()
		{
			float distance = isRelative ? M::Vec::length(position) : M::Vec::distance(position,Listener::Status::location);

			float attenuation = refDistance / (refDistance + (rollOffFactor * (distance - refDistance)));

			priority = gain * attenuation;
		}

		void				SignalWindowFocusRestoration()
		{
			if (OpenAL::Status::focusLostMuted)
			{
				OpenAL::Status::focusLostMuted = false;
				OpenAL::Status::fadeGainIn = true;
				OpenAL::Status::fadeGainFrom = timer.Now();
				OpenAL::Status::fadeGainSeconds = 2;
			}
		}

		void				SignalWindowFocusLoss()
		{
			if (!OpenAL::Status::focusLostMuted)
			{
				OpenAL::Status::focusLostMuted = true;
				alListenerf(AL_GAIN,0);
			}
		}

		ALenum FindFormat(count_t numChannels, count_t numBitsPerSample)
		{
			#undef FORMAT_ID
			#define FORMAT_ID(CH,BITS)	((CH) * 64 + (BITS))
			switch (FORMAT_ID(numChannels,numBitsPerSample))
			{
				case FORMAT_ID(1,8):
					return alGetEnumValue("AL_FORMAT_MONO8");
				case FORMAT_ID(1,16):
					return alGetEnumValue("AL_FORMAT_MONO16");
				case FORMAT_ID(2,8):
					return alGetEnumValue("AL_FORMAT_STEREO8");
				case FORMAT_ID(2,16):
					return alGetEnumValue("AL_FORMAT_STEREO16");
				case FORMAT_ID(4,16):
					return alGetEnumValue("AL_FORMAT_QUAD16");
				case FORMAT_ID(6,16):
					return alGetEnumValue("AL_FORMAT_51CHN16");
				case FORMAT_ID(7,16):
					return alGetEnumValue("AL_FORMAT_61CHN16");
				case FORMAT_ID(8,16):
					return alGetEnumValue("AL_FORMAT_71CHN16");
			}
			throw Except::Program::ParameterFault(CLOCATION,"OpenAL: Unsupported paramter combination: "+String(numChannels)+", "+String(numBitsPerSample));
			//return AL_NONE;
		}

	}
}
