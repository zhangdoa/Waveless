#pragma once
#include "stdafx.h"
#include "Math.h"

namespace Waveless
{
	class AudioEngine
	{
	public:
		AudioEngine() = default;
		~AudioEngine() = default;

		static void Initialize();
		static void Run();
		static void Terminate();
	};
}