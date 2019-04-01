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
		ComplexArray gain(const ComplexArray& x, double gainLevel);

		// cutOffFreq in Hz
		ComplexArray LPF(const ComplexArray& x, double fs, double cutOffFreq);
		ComplexArray LPF(const FrequencyBinArray& x, double cutOffFreq);

		// cutOffFreq in Hz
		ComplexArray HPF(const ComplexArray& x, double fs, double cutOffFreq);
		ComplexArray HPF(const FrequencyBinArray& x, double cutOffFreq);
	};
}