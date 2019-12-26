#include "AudioEngine.h"

#define DR_FLAC_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/extras/dr_flac.h"  /* Enables FLAC decoding. */
#define DR_MP3_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/extras/dr_mp3.h"   /* Enables MP3 decoding. */
#define DR_WAV_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/extras/dr_wav.h"   /* Enables WAV decoding. */
#define MINIAUDIO_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/miniaudio.h"

#include <random>
#include <cmath>
#include <unordered_map>

namespace Waveless
{
	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

	uint64_t GenerateUUID()
	{
		return dist(e2);
	}

	struct
	{
		ma_format format;
		ma_uint32 channels;
		ma_uint32 sampleRate;
	} deviceSetting;

	struct WsEventPrototype : public WsObject
	{
		WavObject* wavObject;
	};

	struct WsEventInstance : public WsObject
	{
		ma_pcm_converter pcmConverter;
		ma_decoder decoder;
		ma_event stopEvent;
	};

	std::unordered_map<uint64_t, WsEventPrototype> g_eventPrototypes;
	std::vector<WsEventInstance*> g_eventInstances;

	ma_device_config deviceConfig;
	ma_device device;
	ma_event terminateEvent;

	ma_uint32 on_pcm_converter_read(ma_pcm_converter* pDSP, void* pFramesOut, ma_uint32 frameCount, void* pUserData)
	{
		auto l_eventInstance = reinterpret_cast<WsEventInstance*>(pUserData);
		auto l_frame = ma_decoder_read_pcm_frames(&l_eventInstance->decoder, pFramesOut, frameCount);
		return l_frame;
	}

	ma_uint32 read_and_mix_pcm_frames_f32(ma_pcm_converter* pcmConverter, float* pOutputF32, ma_uint32 frameCount)
	{
		float temp[4096];
		ma_uint32 tempCapInFrames = ma_countof(temp) / deviceSetting.channels;
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

			framesReadThisIteration = (ma_uint32)ma_pcm_converter_read(pcmConverter, temp, framesToReadThisIteration);

			if (framesReadThisIteration == 0)
			{
				break;
			}

			/* Mix the frames together. */
			for (iSample = 0; iSample < framesReadThisIteration * deviceSetting.channels; ++iSample)
			{
				pOutputF32[totalFramesRead * deviceSetting.channels + iSample] += temp[iSample];
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
				auto l_frame = read_and_mix_pcm_frames_f32(&i->pcmConverter, reinterpret_cast<float*>(pOutput), frameCount);

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

		deviceSetting.format = ma_format_s16;
		deviceSetting.channels = 2;
		deviceSetting.sampleRate = MA_SAMPLE_RATE_44100;

		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = deviceSetting.format;
		deviceConfig.playback.channels = deviceSetting.channels;
		deviceConfig.sampleRate = deviceSetting.sampleRate;
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
		for (auto& i : g_eventInstances)
		{
			i->objectState = ObjectState::Active;
		}
	}

	WsEventInstance* CreateEventInstance(const WsEventPrototype* l_eventPrototype)
	{
		// @TODO: Pool it
		auto l_eventInstance = new WsEventInstance();

		auto l_header = reinterpret_cast<StandardWavHeader*>(l_eventPrototype->wavObject->header);

		auto l_UUID = GenerateUUID();

		l_eventInstance->UUID = l_UUID;

		auto decoderConfig = ma_decoder_config_init(ma_format(l_header->fmtChunk.wBitsPerSample / 8), l_header->fmtChunk.nChannels, l_header->fmtChunk.nSamplesPerSec);

		if (ma_decoder_init_memory_raw(l_eventPrototype->wavObject->sample.data(), l_eventPrototype->wavObject->sample.size(), &decoderConfig, &decoderConfig, &l_eventInstance->decoder) != MA_SUCCESS)
		{
			printf("Failed to init decoder.\n");
		}

		auto converterConfig = ma_pcm_converter_config_init(ma_format(l_header->fmtChunk.wBitsPerSample / 8), l_header->fmtChunk.nChannels, l_header->fmtChunk.nSamplesPerSec, deviceSetting.format, deviceSetting.channels, deviceSetting.sampleRate, on_pcm_converter_read, l_eventInstance);

		if (ma_pcm_converter_init(&converterConfig, &l_eventInstance->pcmConverter) != MA_SUCCESS)
		{
			printf("Failed to init pcm converter.\n");
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

		g_eventPrototypes.emplace(l_UUID, l_eventPrototype);

		return l_UUID;
	}
}