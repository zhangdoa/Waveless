#include "AudioEngine.h"
#include "../Core/Math.h"
#include "../Core/Logger.h"

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

	struct PlayableObject : public Object
	{
		ma_decoder_config decoderConfig;
	};

	struct EventPrototype : public PlayableObject
	{
		WavObject* wavObject;
	};

	struct EventInstance : public PlayableObject
	{
		ma_decoder decoder;
		ma_event stopEvent;
		float sampleStateLPF[8] = { 0 };
		float sampleStateHPF[8] = { 0 };
		float cutOffFreqLPF = 0.0f;
		float cutOffFreqHPF = 0.0f;
	};

	std::unordered_map<uint64_t, EventPrototype> g_eventPrototypes;
	std::unordered_map<WavObject*, uint64_t> g_registeredEventPrototypes;
	std::unordered_map<uint64_t, EventInstance*> g_eventInstances;
	std::queue<EventInstance*> g_untriggeredEventInstances;

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

		// First frame
		// Y0 = a0 * X0 + a1 * X-1 - b1 * Y-1
		for (ma_uint32 i = 0; i < channels; ++i)
		{
			temp[i] = a0 * pOutput[i] + pSampleState[i];
		}

		// Yn = a0 * Xn + a1 * Xn-1 - b1 * Yn-1
		for (ma_uint32 i = 1; i < frameCount; ++i)
		{
			for (ma_uint32 j = 0; j < channels; ++j)
			{
				temp[i * channels + j] = a0 * pOutput[i * channels + j] + a1 * pOutput[(i - 1) * channels + j] - b1 * temp[(i - 1) * channels + j];
			}
		}

		// Save a1 * XN - b1 * YN of last frame
		for (ma_uint32 i = 0; i < channels; ++i)
		{
			auto XN = pOutput[frameCount * channels - channels + i];
			auto YN = temp[frameCount * channels - channels + i];
			pSampleState[i] = a1 * XN - b1 * YN;
		}

		// Save Yn
		for (ma_uint32 i = 0; i < frameCount * channels; ++i)
		{
			pOutput[i] = temp[i];
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

		// First frame
		// Y0 = a0 * X0 + a1 * X-1 - b1 * Y-1
		for (ma_uint32 i = 0; i < channels; ++i)
		{
			temp[i] = a0 * pOutput[i] + pSampleState[i];
		}

		// Yn = a0 * Xn + a1 * Xn-1 - b1 * Yn-1
		for (ma_uint32 i = 1; i < frameCount; ++i)
		{
			for (ma_uint32 j = 0; j < channels; ++j)
			{
				temp[i * channels + j] = a0 * pOutput[i * channels + j] + a1 * pOutput[(i - 1) * channels + j] - b1 * temp[(i - 1) * channels + j];
			}
		}

		// Save a1 * XN - b1 * YN of last frame
		for (ma_uint32 i = 0; i < channels; ++i)
		{
			auto XN = pOutput[frameCount * channels - channels + i];
			auto YN = temp[frameCount * channels - channels + i];
			pSampleState[i] = a1 * XN - b1 * YN;
		}

		// Save Yn
		for (ma_uint32 i = 0; i < frameCount * channels; ++i)
		{
			pOutput[i] = temp[i];
		}

		return frameCount;
	}

	ma_uint32 read_and_mix_pcm_frames(EventInstance* eventInstance, float* pOutput, ma_uint32 frameCount)
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

			// Decode and apply filters
			framesReadThisIteration = ma_decoder_read_pcm_frames(&eventInstance->decoder, temp, framesToReadThisIteration);

			if (framesReadThisIteration == 0)
			{
				break;
			}

			if (eventInstance->cutOffFreqLPF != 0.0f)
			{
				low_pass_filter(deviceDecoderConfig.channels, eventInstance->cutOffFreqLPF, deviceDecoderConfig.sampleRate, eventInstance->sampleStateLPF, temp, framesToReadThisIteration);
			}
			if (eventInstance->cutOffFreqHPF != 0.0f)
			{
				high_pass_filter(deviceDecoderConfig.channels, eventInstance->cutOffFreqHPF, deviceDecoderConfig.sampleRate, eventInstance->sampleStateHPF, temp, framesToReadThisIteration);
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
			if (i.second->objectState == ObjectState::Active)
			{
				auto l_frame = read_and_mix_pcm_frames(i.second, reinterpret_cast<float*>(pOutput), frameCount);

				if (l_frame < frameCount)
				{
					i.second->objectState = ObjectState::Terminated;
					ma_event_signal(&i.second->stopEvent);
				}
			}
		}

		(void)pInput;
	}

	WsResult AudioEngine::Initialize()
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
			Logger::Log(LogLevel::Error, "Failed to open playback device.");
			Terminate();
			return WsResult::Fail;
		}

		ma_event_init(device.pContext, &terminateEvent);

		if (ma_device_start(&device) != MA_SUCCESS)
		{
			Logger::Log(LogLevel::Error, "Failed to start playback device.");
			Terminate();
			return WsResult::Fail;
		}

		return WsResult::Success;
	}

	WsResult AudioEngine::Flush()
	{
		while (g_untriggeredEventInstances.size())
		{
			auto i = g_untriggeredEventInstances.front();
			i->objectState = ObjectState::Active;
			g_untriggeredEventInstances.pop();
		}

		return WsResult::Success;
	}

	EventInstance* CreateEventInstance(const EventPrototype* l_eventPrototype)
	{
		// @TODO: Pool it
		auto l_eventInstance = new EventInstance();

		auto l_UUID = GenerateUUID();

		l_eventInstance->UUID = l_UUID;
		l_eventInstance->decoderConfig = l_eventPrototype->decoderConfig;

		if (ma_decoder_init_memory_raw(l_eventPrototype->wavObject->sample.data(), l_eventPrototype->wavObject->sample.size(), &l_eventInstance->decoderConfig, &deviceDecoderConfig, &l_eventInstance->decoder) != MA_SUCCESS)
		{
			Logger::Log(LogLevel::Error, "Failed to init decoder.");
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

			g_eventInstances.emplace(l_eventInstance->UUID, l_eventInstance);
			g_untriggeredEventInstances.push(l_eventInstance);

			return l_eventInstance->UUID;
		}
		else
		{
			Logger::Log(LogLevel::Error, "Failed to find UUID: ", UUID);
			return 0;
		}
	}

	WsResult AudioEngine::Terminate()
	{
		for (auto i : g_eventInstances)
		{
			ma_event_wait(&i.second->stopEvent);
		}
		ma_device_uninit(&device);

		return WsResult::Success;
	}

	uint64_t AudioEngine::AddEventPrototype(const WavObject & wavObject)
	{
		auto l_result = g_registeredEventPrototypes.find(&const_cast<WavObject&>(wavObject));

		if (l_result != g_registeredEventPrototypes.end())
		{
			Logger::Log(LogLevel::Warning, "EventPrototype has been added.");
			return l_result->second;
		}
		else
		{
			auto l_UUID = GenerateUUID();

			EventPrototype l_eventPrototype;

			l_eventPrototype.UUID = l_UUID;
			l_eventPrototype.wavObject = &const_cast<WavObject&>(wavObject);
			l_eventPrototype.decoderConfig = ma_decoder_config_init
			(
				ma_format(wavObject.header.fmtChunk.wBitsPerSample / 8),
				wavObject.header.fmtChunk.nChannels,
				wavObject.header.fmtChunk.nSamplesPerSec
			);

			g_eventPrototypes.emplace(l_UUID, l_eventPrototype);
			g_registeredEventPrototypes.emplace(l_eventPrototype.wavObject, l_UUID);

			return l_UUID;
		}
	}

	WsResult AudioEngine::ApplyLPF(uint64_t UUID, float cutOffFreq)
	{
		auto l_result = g_eventInstances.find(UUID);
		if (l_result != g_eventInstances.end())
		{
			if (cutOffFreq * 2.0f > l_result->second->decoderConfig.sampleRate)
			{
				Logger::Log(LogLevel::Warning, "Cut-off frequency is beyond the sample rate");
				return WsResult::Fail;
			}
			else
			{
				l_result->second->cutOffFreqLPF = cutOffFreq;
				return WsResult::Success;
			}
		}
		else
		{
			Logger::Log(LogLevel::Warning, "Can't find EventInstance.");
			return WsResult::IDNotFound;
		}
	}

	WsResult AudioEngine::ApplyHPF(uint64_t UUID, float cutOffFreq)
	{
		auto l_result = g_eventInstances.find(UUID);
		if (l_result != g_eventInstances.end())
		{
			if (cutOffFreq * 2.0f > l_result->second->decoderConfig.sampleRate)
			{
				Logger::Log(LogLevel::Warning, "Cut-off frequency is beyond the sample rate");
				return WsResult::Fail;
			}
			else
			{
				l_result->second->cutOffFreqHPF = cutOffFreq;
				return WsResult::Success;
			}
		}
		else
		{
			Logger::Log(LogLevel::Warning, "Can't find EventInstance.");
			return WsResult::IDNotFound;
		}
	}
}