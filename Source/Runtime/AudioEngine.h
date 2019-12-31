#pragma once
#include "../Core/Object.h"
#include "../Core/Math.h"
#include "../IO/WaveParser.h"

namespace Waveless
{
	class AudioEngine
	{
	public:
		AudioEngine() = default;
		~AudioEngine() = default;

		///
		/// Initialize audio engine
		///
		static void Initialize();

		///
		/// Force all event instances to be executed
		///
		static void Flush();

		///
		/// Trigger once an event prototype and get an event instance
		///
		static uint64_t Trigger(uint64_t UUID);

		///
		/// Terminate audio engine
		///
		static void Terminate();

		///
		/// Add an event prototype from a wave object
		///
		static uint64_t AddEventPrototype(const WavObject& wavObject);
	};
}