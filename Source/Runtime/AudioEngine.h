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
		static WsResult Initialize();

		///
		/// Force all event instances to be executed
		///
		static WsResult Flush();

		///
		/// Trigger once an event prototype and get an event instance
		///
		static uint64_t Trigger(uint64_t UUID);

		///
		/// Terminate audio engine
		///
		static WsResult Terminate();

		///
		/// Add an event prototype from a wave object
		///
		static uint64_t AddEventPrototype(const WavObject& wavObject);

		///
		/// Apply low pass filter to an event instance
		///
		static WsResult ApplyLPF(uint64_t UUID, float cutOffFreq);

		///
		/// Apply high pass filter to an event instance
		///
		static WsResult ApplyHPF(uint64_t UUID, float cutOffFreq);
	};
}