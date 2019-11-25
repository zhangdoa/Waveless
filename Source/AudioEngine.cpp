#include "AudioEngine.h"

#define DR_FLAC_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/extras/dr_flac.h"  /* Enables FLAC decoding. */
#define DR_MP3_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/extras/dr_mp3.h"   /* Enables MP3 decoding. */
#define DR_WAV_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/extras/dr_wav.h"   /* Enables WAV decoding. */
#define MINIAUDIO_IMPLEMENTATION
#include "../GitSubmodules/miniaudio/miniaudio.h"

namespace Waveless
{
	ma_result result;
	ma_decoder decoder;
	ma_device_config deviceConfig;
	ma_device device;

	void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
	{
		ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;

		if (pDecoder == nullptr)
		{
			return;
		}

		ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

		(void)pInput;
	}

	void AudioEngine::Initialize()
	{
		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = ma_format_s16;
		deviceConfig.playback.channels = 2;
		deviceConfig.sampleRate = MA_SAMPLE_RATE_44100;
		deviceConfig.dataCallback = data_callback;
		deviceConfig.pUserData = nullptr;

		if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
		{
			printf("Failed to open playback device.\n");
			Terminate();
		}
	}

	void AudioEngine::Run()
	{
		if (ma_device_start(&device) != MA_SUCCESS)
		{
			printf("Failed to start playback device.\n");
			Terminate();
		}
	}

	void AudioEngine::Terminate()
	{
		ma_device_uninit(&device);
	}
}