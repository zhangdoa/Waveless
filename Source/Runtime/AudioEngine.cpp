#include "AudioEngine.h"
#include "../Core/Math.h"

#define DR_FLAC_IMPLEMENTATION
#include "../../GitSubmodules/miniaudio/extras/dr_flac.h"  /* Enables FLAC decoding. */
#define DR_MP3_IMPLEMENTATION
#include "../../GitSubmodules/miniaudio/extras/dr_mp3.h"   /* Enables MP3 decoding. */
#define DR_WAV_IMPLEMENTATION
#include "../../GitSubmodules/miniaudio/extras/dr_wav.h"   /* Enables WAV decoding. */
#define MINIAUDIO_IMPLEMENTATION
#include "../../GitSubmodules/miniaudio/miniaudio.h"

namespace Waveless
{
	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

	uint64_t GenerateUUID()
	{
		return dist(e2);
	}

	ma_decoder_config deviceDecoderConfig;

	struct WsPlayableObject : public WsObject
	{
		ma_decoder_config decoderConfig;
	};

	struct WsEventPrototype : public WsPlayableObject
	{
		WavObject* wavObject;
	};

	struct WsEventInstance : public WsPlayableObject
	{
		ma_decoder decoder;
		ma_event stopEvent;
	};

	std::unordered_map<uint64_t, WsEventPrototype> g_eventPrototypes;
	std::vector<WsEventInstance*> g_eventInstances;
	std::queue<WsEventInstance*>g_untriggeredEventInstances;

	ma_device_config deviceConfig;
	ma_device device;
	ma_event terminateEvent;
	const int sizeOfTempBuffer = 4096;

	ma_uint32 low_pass_filter(ma_uint32 channels, float cutoffFreq, ma_uint32 sampleRate, float* pSampleState, float* pOutput, ma_uint32 frameCount)
	{
		auto th = 2.0f * PI<float> * cutoffFreq / (float)sampleRate;
		auto g = cosf(th) / (1.0f + sinf(th));
		auto a0 = (1.0f - g) / 2.0f;
		auto a1 = (1.0f - g) / 2.0f;
		auto b1 = -g;

		float temp[sizeOfTempBuffer];

		for (ma_uint32 i = 0; i < channels; ++i)
		{
			temp[i] = a0 * pSampleState[i];
		}

		for (ma_uint32 i = channels; i < frameCount * channels; i += channels)
		{
			for (ma_uint32 j = 0; j < channels; j++)
			{
				temp[i + j] = a0 * pOutput[i + j] + a1 * pOutput[i + j - channels] - b1 * temp[i + j - channels];
			}
		}

		for (ma_uint32 i = 0; i < frameCount * channels; ++i)
		{
			pOutput[i] = temp[i];
		}

		for (ma_uint32 i = 0; i < channels; ++i)
		{
			pSampleState[i] = temp[frameCount * channels - channels + i];
		}

		return frameCount;
	}

	ma_uint32 high_pass_filter(ma_uint32 channels, float cutoffFreq, ma_uint32 sampleRate, float* pSampleState, float* pOutput, ma_uint32 frameCount)
	{
		auto th = 2.0f * PI<float> * cutoffFreq / (float)sampleRate;
		auto g = cosf(th) / (1.0f + sinf(th));
		auto a0 = (1.0f + g) / 2.0f;
		auto a1 = -((1.0f + g) / 2.0f);
		auto b1 = -g;

		float temp[sizeOfTempBuffer];

		for (ma_uint32 i = 0; i < channels; ++i)
		{
			temp[i] = a0 * pSampleState[i];
		}

		for (ma_uint32 i = channels; i < frameCount * channels; i += channels)
		{
			for (ma_uint32 j = 0; j < channels; j++)
			{
				temp[i + j] = a0 * pOutput[i + j] + a1 * pOutput[i + j - channels] - b1 * temp[i + j - channels];
			}
		}

		for (ma_uint32 i = 0; i < frameCount * channels; ++i)
		{
			pOutput[i] = temp[i];
		}

		for (ma_uint32 i = 0; i < channels; ++i)
		{
			pSampleState[i] = temp[frameCount * channels - channels + i];
		}

		return frameCount;
	}

	ma_uint32 read_and_mix_pcm_frames(WsEventInstance* eventInstance, float* pOutput, ma_uint32 frameCount)
	{
		float temp[sizeOfTempBuffer];

		ma_uint32 tempCapInFrames = ma_countof(temp) / eventInstance->decoderConfig.channels;
		ma_uint32 totalFramesRead = 0;

		while (totalFramesRead < frameCount)
		{
			ma_uint32 iSample;
			ma_uint32 framesReadThisIteration;
			ma_uint32 totalFramesRemaining = frameCount - totalFramesRead;
			ma_uint32 framesToReadThisIteration = tempCapInFrames;

			if (framesToReadThisIteration > totalFramesRemaining)
			{
				framesToReadThisIteration = totalFramesRemaining;
			}

			framesReadThisIteration = ma_decoder_read_pcm_frames(&eventInstance->decoder, temp, framesToReadThisIteration);

			if (framesReadThisIteration == 0)
			{
				break;
			}

			/* Mix the frames together. */
			for (iSample = 0; iSample < framesReadThisIteration * deviceDecoderConfig.channels; ++iSample)
			{
				pOutput[totalFramesRead * deviceDecoderConfig.channels + iSample] += temp[iSample];
			}

			totalFramesRead += framesReadThisIteration;

			if (framesReadThisIteration < framesToReadThisIteration)
			{
				break;  /* Reached EOF. */
			}
		}

		return totalFramesRead;
	}

	void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
	{
		for (auto i : g_eventInstances)
		{
			if (i->objectState == ObjectState::Active)
			{
				auto l_frame = read_and_mix_pcm_frames(i, reinterpret_cast<float*>(pOutput), frameCount);

				if (l_frame < frameCount)
				{
					i->objectState = ObjectState::Terminated;
					ma_event_signal(&i->stopEvent);
				}
			}
		}

		(void)pInput;
	}

	void AudioEngine::Initialize()
	{
		g_eventPrototypes.reserve(4096);
		g_eventInstances.reserve(512);

		deviceDecoderConfig = ma_decoder_config_init(ma_format_f32, 2, MA_SAMPLE_RATE_44100);

		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = deviceDecoderConfig.format;
		deviceConfig.playback.channels = deviceDecoderConfig.channels;
		deviceConfig.sampleRate = deviceDecoderConfig.sampleRate;
		deviceConfig.dataCallback = data_callback;
		deviceConfig.pUserData = nullptr;

		if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
		{
			printf("Failed to open playback device.\n");
			Terminate();
		}

		ma_event_init(device.pContext, &terminateEvent);

		if (ma_device_start(&device) != MA_SUCCESS)
		{
			printf("Failed to start playback device.\n");
			Terminate();
		}
	}

	void AudioEngine::Flush()
	{
		while (g_untriggeredEventInstances.size())
		{
			auto i = g_untriggeredEventInstances.front();
			i->objectState = ObjectState::Active;
			g_untriggeredEventInstances.pop();
		}
	}

	WsEventInstance* CreateEventInstance(const WsEventPrototype* l_eventPrototype)
	{
		// @TODO: Pool it
		auto l_eventInstance = new WsEventInstance();

		auto l_UUID = GenerateUUID();

		l_eventInstance->UUID = l_UUID;
		l_eventInstance->decoderConfig = l_eventPrototype->decoderConfig;

		if (ma_decoder_init_memory_raw(l_eventPrototype->wavObject->sample.data(), l_eventPrototype->wavObject->sample.size(), &l_eventInstance->decoderConfig, &deviceDecoderConfig, &l_eventInstance->decoder) != MA_SUCCESS)
		{
			printf("Failed to init decoder.\n");
		}

		ma_event_init(device.pContext, &l_eventInstance->stopEvent);

		return l_eventInstance;
	}

	uint64_t AudioEngine::Trigger(uint64_t UUID)
	{
		auto l_result = g_eventPrototypes.find(UUID);

		if (l_result != g_eventPrototypes.end())
		{
			auto l_eventPrototype = &l_result->second;
			auto l_eventInstance = CreateEventInstance(l_eventPrototype);

			g_eventInstances.emplace_back(l_eventInstance);
			g_untriggeredEventInstances.push(l_eventInstance);

			return l_eventInstance->UUID;
		}
		else
		{
			printf("Failed to find UUID.\n");
			return 0;
		}
	}

	void AudioEngine::Terminate()
	{
		for (auto i : g_eventInstances)
		{
			ma_event_wait(&i->stopEvent);
		}
		ma_device_uninit(&device);
	}

	uint64_t AudioEngine::AddEventPrototype(const WavObject & wavObject)
	{
		auto l_UUID = GenerateUUID();

		WsEventPrototype l_eventPrototype;

		l_eventPrototype.UUID = l_UUID;
		l_eventPrototype.wavObject = &const_cast<WavObject&>(wavObject);
		l_eventPrototype.decoderConfig = ma_decoder_config_init
		(
			ma_format(wavObject.header.fmtChunk.wBitsPerSample / 8),
			wavObject.header.fmtChunk.nChannels,
			wavObject.header.fmtChunk.nSamplesPerSec
		);

		g_eventPrototypes.emplace(l_UUID, l_eventPrototype);

		return l_UUID;
	}
}