#pragma once
#include "stdafx.h"
#include "Math.h"

namespace Waveless
{
	class DSP
	{
	public:
		DSP() = default;
		~DSP() = default;

		// gain level in dB
		static ComplexArray Gain(const ComplexArray& x, double gainLevel);

		// cutOffFreq in Hz
		static ComplexArray LPF(const ComplexArray& x, double fs, double cutOffFreq);
		static ComplexArray LPF(const FreqBinData& x, double cutOffFreq);

		// cutOffFreq in Hz
		static ComplexArray HPF(const ComplexArray& x, double fs, double cutOffFreq);
		static ComplexArray HPF(const FreqBinData& x, double cutOffFreq);
	};
}