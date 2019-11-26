#pragma once
#include "stdafx.h"
#include "Math.h"
#include "WaveParser.h"

namespace Waveless
{
	class AudioEngine
	{
	public:
		AudioEngine() = default;
		~AudioEngine() = default;

		static void Initialize();
		static void Run();
		static void Play(const WaveData& waveData);
		static void Terminate();
	};
}