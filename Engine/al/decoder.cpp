#include "decoder.h"

#include <iostream>
#include <string.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <assert.h>


namespace DeltaWorks
{

const int kBitsPerSample = 16;
const int kNumChannels = 2;
const int kSampleRate = 44100;
const int kLeftoverSize = 4096; // in int16's, this seems to be the size MF AAC



void AudioDecoder::Open(const PathString&filename)
{
	try
	{
		Clear();

		data.filename = filename;
		//Defaults
		data.numChannels = kNumChannels;
		data.frameRate = kSampleRate;
		data.bitsPerSample = kBitsPerSample;



		HRESULT hr(S_OK);
		// Initialize the COM library.
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr))
			throw Except::IO::GeneralFault(CLOCATION,"SSMF: failed to initialize COM (CoInitializeEx)");

		// Initialize the Media Foundation platform.
		hr = MFStartup(MF_VERSION);
		if (FAILED(hr))
			throw Except::IO::GeneralFault(CLOCATION,"SSMF: failed to initialize Media Foundation (MFStartup)");

		// Create the source reader to read the input file.
		hr = MFCreateSourceReaderFromURL(filename.c_str(), NULL, data.reader.Init());
		if (FAILED(hr))
			throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"SSMF: Error opening input file:" +String(filename)+ ", with error (MFCreateSourceReaderFromURL): "+String( HRESULT_CODE(hr)));

		ConfigureAudioStream();
		ReadProperties();

		//Seek to position 0, which forces us to skip over all the header frames.
		//This makes sure we're ready to just let the Analyser rip and it'll
		//get the number of samples it expects (ie. no header frames).
		Rewind();
	}
	catch(...)
	{
		Clear();
		throw;
	}
}

AudioDecoder::~AudioDecoder()
{
	Clear();
}

void AudioDecoder::Clear()
{
	if (!IsActive())
		return;
	data = Data();
	extra.Clear();
    MFShutdown();
    CoUninitialize();
}



void AudioDecoder::SeekFrame(index_t frameIdx)
{
    PROPVARIANT prop;
    HRESULT hr(S_OK);
    __int64 mfSeekTarget(mfFromFrame(frameIdx) - 1);
    // minus 1 here seems to make our seeking work properly, otherwise we will
    // (more often than not, maybe always) seek a bit too far (although not
    // enough for our calculatedFrameFromMF <= nextFrame assertion in ::read).
    // Has something to do with 100ns MF units being much smaller than most
    // frame offsets (in seconds) -bkgood
    if (!IsActive())
		throw Except::IO::GeneralFault(CLOCATION,"SSMF: Inactive");

    // this doesn't fail, see MS's implementation
    hr = InitPropVariantFromInt64(mfSeekTarget < 0 ? 0 : mfSeekTarget, &prop);


    hr = data.reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION,"SSMF: failed to flush before seek");

    // http://msdn.microsoft.com/en-us/library/dd374668(v=VS.85).aspx
    hr = data.reader->SetCurrentPosition(GUID_NULL, prop);
    if (FAILED(hr)) {
        // nothing we can do here as we can't fail (no facility to other than
        // crashing mixxx)
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: failed to seek" + String(
            hr == MF_E_INVALIDREQUEST ? "Sample requests still pending" : ""));
    } else {
      //  result = sampleIdx;
    }
    PropVariantClear(&prop);

    // record the next frame so that we can make sure we're there the next
    // time we get a buffer from MFSourceReader
    data.seekTargetFrame = frameIdx;
    data.isSeeking = true;
    data.currentSamplePosition = frameIdx*data.numChannels;
}

count_t	AudioDecoder::BytesToFrames(size_t numBytes) const
{
	DBG_ASSERT2__((numBytes % data.bytesPerFrame) == 0,numBytes,data.bytesPerFrame)
	return numBytes / data.bytesPerFrame;
}

count_t AudioDecoder::ReadFrames(const count_t numFrames, void *destination)
{
	try
	{
	    if (!IsActive())
			throw Except::IO::GeneralFault(CLOCATION,"SSMF: Inactive");

		BYTE *destBuffer = (BYTE*)destination;
		size_t bytesNeeded = numFrames * data.numChannels * data.bitsPerSample/8;
		//size_t framesNeeded = numFrames;

		while (bytesNeeded && (extra >> *destBuffer))
		{
			destBuffer++;
			bytesNeeded--;
		}
		if (!bytesNeeded)
			return numFrames;

		const count_t frameSize = data.bytesPerFrame;

		while (bytesNeeded > 0)
		{
			HRESULT hr(S_OK);
			DWORD dwFlags(0);
			__int64 timestamp(0);
			Handle<IMFSample> pSample;
			bool error(false); // set to true to break after releasing

			hr = data.reader->ReadSample(
				MF_SOURCE_READER_FIRST_AUDIO_STREAM, // [in] DWORD dwStreamIndex,
				0,                                   // [in] DWORD dwControlFlags,
				NULL,                                // [out] DWORD *pdwActualStreamIndex,
				&dwFlags,                            // [out] DWORD *pdwStreamFlags,
				&timestamp,                          // [out] LONGLONG *pllTimestamp,
				pSample.Init());                     // [out] IMFSample **ppSample
			if (FAILED(hr))
				throw Except::IO::GeneralFault(CLOCATION, "ReadSample() failed.");


			if (dwFlags & MF_SOURCE_READERF_ERROR)
				throw Except::IO::GeneralFault(CLOCATION, "SSMF: ReadSample set ERROR");
			
			if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
			{
				data.isEOF = true;
				break;
			}



			if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
				throw Except::IO::GeneralFault(CLOCATION, "SSMF: Type change");
			
			if (pSample == NULL) {
				// generally this will happen when dwFlags contains ENDOFSTREAM,
				// so it'll be caught before now -bkgood
				continue;
			} // we now own a ref to the instance at pSample

			Handle<IMFMediaBuffer> pMBuffer;
			// I know this does at least a memcopy and maybe a malloc, if we have
			// xrun issues with this we might want to look into using
			// IMFSample::GetBufferByIndex (although MS doesn't recommend this)
			if (FAILED(hr = pSample->ConvertToContiguousBuffer(pMBuffer.Init())))
				throw Except::IO::GeneralFault(CLOCATION, "SSMF: ConvertToContiguousBuffer() failed");
			
			BYTE *buffer = (NULL);
			DWORD bufferByteLength = (0);
			hr = pMBuffer->Lock(&buffer, NULL, &bufferByteLength);
			if (FAILED(hr))
				throw Except::IO::GeneralFault(CLOCATION, "SSMF: Lock() failed");

			const count_t bufferFrameLength = BytesToFrames( bufferByteLength );


			//bufferLength /= (data.m_iBitsPerSample / 8 * data.numChannels); // now in frames

			if (data.isSeeking)
			{
				__int64 bufferPosition(frameFromMF(timestamp));

				if (data.seekTargetFrame >= bufferPosition &&
					data.seekTargetFrame < bufferPosition + __int64(bufferFrameLength))
				{
					// m_nextFrame is in this buffer.
					const count_t skipFrames = count_t(data.seekTargetFrame - bufferPosition);
					data.currentSamplePosition += skipFrames;
					const size_t skipBytes = skipFrames * frameSize;
					buffer += skipBytes;
					bufferByteLength -= DWORD(skipBytes);
					data.isSeeking = false;
				} else {
					// we need to keep going forward
					pMBuffer->Unlock();

					data.currentSamplePosition += bufferFrameLength;

					continue;
				}
			}
			if (bytesNeeded <= bufferByteLength)
			{
				memcpy(destBuffer,buffer,bytesNeeded);
				for (index_t i = bytesNeeded; i < bufferByteLength; i++)
					extra << buffer[i];
				destBuffer += bytesNeeded;
				bytesNeeded = 0;
				break;
			}
			else
			{
				memcpy(destBuffer,buffer,bufferByteLength);
				destBuffer += bufferByteLength;
				bytesNeeded -= bufferByteLength;
			}
			pMBuffer->Unlock();
		}

		const count_t bytesRead = destBuffer - (BYTE*)destination;
		const count_t framesRead = bytesRead / frameSize;
		const count_t samplesRead = bytesRead / (data.bitsPerSample/8);

		data.seekTargetFrame += framesRead;
		data.currentSamplePosition += samplesRead;
	
		return framesRead;
	}
	catch (...)
	{
		Clear();
		throw;
	}
}

count_t AudioDecoder::CountTotalSamples() const
{
    return count_t(ceil( data.totalSeconds * data.frameRate * data.numChannels ));
}

//-------------------------------------------------------------------
// configureAudioStream
//
// Selects an audio stream from the source file, and configures the
// stream to deliver decoded PCM audio.
//-------------------------------------------------------------------


void AudioDecoder::GetUINT32(const GUID&id, UINT32&v) const
{
	const HRESULT hr = data.audioType->GetUINT32(id, &v);
	//if (FAILED(hr))
		//throw Except::IO::GeneralFault(CLOCATION, "SSMF: Failed to acquire attribute from media type");

}

/** Cobbled together from:
    http://msdn.microsoft.com/en-us/library/dd757929(v=vs.85).aspx
    and http://msdn.microsoft.com/en-us/library/dd317928(VS.85).aspx
    -- Albert
    If anything in here fails, just bail. I'm not going to decode HRESULTS.
    -- Bill
    */
void AudioDecoder::ConfigureAudioStream()
{
    HRESULT hr(S_OK);

    // deselect all streams, we only want the first
    hr = data.reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: failed to deselect all streams");

    hr = data.reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: failed to select first audio stream");

//Debugging:
//Let's get some info
	// Get the complete uncompressed format.
    //hr = data.reader->GetCurrentMediaType(
    //    MF_SOURCE_READER_FIRST_AUDIO_STREAM,
    //   &data.audioType);
	hr = data.reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
									   0, //Index of the media type to retreive... (what does that even mean?)
									   data.audioType.Init());
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: failed to retrieve completed media type");
	
	UINT32 allSamplesIndependent	= 0;
	UINT32 fixedSizeSamples		= 0;
	UINT32 sampleSize				= 0;
	UINT32 bitsPerSample			= 0;
	UINT32 blockAlignment			= 0;
	UINT32 numChannels				= 0;
	UINT32 samplesPerSecond		= 0;
	GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, allSamplesIndependent);
	GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, fixedSizeSamples);
	GetUINT32(MF_MT_SAMPLE_SIZE, sampleSize);
	GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
	GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlignment);
	GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
	GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, samplesPerSecond);

	std::cout << "bitsPerSample: " << bitsPerSample << std::endl;
	std::cout << "allSamplesIndependent: " << allSamplesIndependent << std::endl;
	std::cout << "fixedSizeSamples: " << fixedSizeSamples << std::endl;
	std::cout << "sampleSize: " << sampleSize << std::endl;
	std::cout << "bitsPerSample: " << bitsPerSample << std::endl;
	std::cout << "blockAlignment: " << blockAlignment << std::endl;
	std::cout << "numChannels: " << numChannels << std::endl;
	std::cout << "samplesPerSecond: " << samplesPerSecond << std::endl;

	data.numChannels = numChannels;
	data.frameRate = samplesPerSecond/numChannels;
	data.bitsPerSample = bitsPerSample;
	//For compressed files, the bits per sample is undefined, so by convention we're
	//going to get 16-bit integers out.
	if (data.bitsPerSample == 0)
	{
		data.bitsPerSample = kBitsPerSample;
	}

	data.bytesPerSample = data.bitsPerSample/8;
	if ((data.bitsPerSample%8)!=0)
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: Bits per sample of loaded media ("+String(data.bitsPerSample)+") is not multiple of 8");
	data.bytesPerFrame = data.bytesPerSample * data.numChannels;

    hr = MFCreateMediaType(data.audioType.Init());
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: failed to create media type");

    hr = data.audioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: failed to set major type");

    hr = data.audioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION,  "SSMF: failed to set subtype");
/*
    hr = data.audioType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set samples independent";
        return false;
    }

    hr = data.audioType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, true);
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set fixed size samples";
        return false;
    }

    hr = data.audioType->SetUINT32(MF_MT_SAMPLE_SIZE, kLeftoverSize);
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set sample size";
        return false;
    }

    // MSDN for this attribute says that if bps is 8, samples are unsigned.
    // Otherwise, they're signed (so they're signed for us as 16 bps). Why
    // chose to hide this rather useful tidbit here is beyond me -bkgood
    hr = data.audioType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, kBitsPerSample);
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set bits per sample";
        return false;
    }


    hr = data.audioType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,
        numChannels * (kBitsPerSample / 8));
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set block alignment";
        return false;
    }
	*/

	/*
	//MediaFoundation will not convert between mono and stereo without a transform!
    hr = data.audioType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, kNumChannels);
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set number of channels";
        return false;
    }

	
	//MediaFoundation will not do samplerate conversion without a transform in the pipeline.
    hr = data.audioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, kSampleRate);
    if (FAILED(hr)) {
        std::cerr << "SSMF: failed to set sample rate";
        return false;
    }
	*/

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    hr = data.reader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        NULL, data.audioType);


    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION,  "SSMF: failed to set media type");

    // Get the complete uncompressed format.
    hr = data.reader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        data.audioType.Init());
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION,  "SSMF: failed to retrieve completed media type");

    // Ensure the stream is selected.
    hr = data.reader->SetStreamSelection(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        true);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION,  "SSMF: failed to select first audio stream (again)");


}

void AudioDecoder::ReadProperties()
{
    PROPVARIANT prop;
    HRESULT hr = S_OK;

    //Get the duration, provided as a 64-bit integer of 100-nanosecond units
    hr = data.reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION, &prop);
    if (FAILED(hr))
		throw Except::IO::GeneralFault(CLOCATION, "SSMF: error getting duration");
    // QuadPart isn't available on compilers that don't support _int64. Visual
    // Studio 6.0 introduced the type in 1998, so I think we're safe here
    // -bkgood
    data.totalSeconds = secondsFromMF(prop.hVal.QuadPart);
    //data.m_mfDuration = prop.hVal.QuadPart;
    std::cout << "SSMF: Duration: " << data.totalSeconds << std::endl;
    PropVariantClear(&prop);

}


/**
 * Convert a 100ns Media Foundation value to a number of seconds.
 */
double AudioDecoder::secondsFromMF(__int64 mf) const
{
    return static_cast<double>(mf) / 1e7;
}

/**
 * Convert a number of seconds to a 100ns Media Foundation value.
 */
__int64 AudioDecoder::mfFromSeconds(double sec)	const
{
    return sec * 1e7;
}

/**
 * Convert a 100ns Media Foundation value to a frame offset.
 */
__int64 AudioDecoder::frameFromMF(__int64 mf) const
{
    return static_cast<double>(mf) * data.frameRate / 1e7;
}

/**
 * Convert a frame offset to a 100ns Media Foundation value.
 */
__int64 AudioDecoder::mfFromFrame(__int64 frame) const
{
    return static_cast<double>(frame) / data.frameRate * 1e7;
}


}
