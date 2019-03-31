#include "DSP.h"

namespace Waveless
{
	ComplexArray DSP::gain(const ComplexArray & x, double gainLevel)
	{
		Math l_math;

		ComplexArray l_xProcessed;

		auto N = x.size();
		l_xProcessed.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			l_xProcessed.emplace_back(x[i] * l_math.dB2LinearMag(gainLevel));
		}

		return l_xProcessed;
	}

	ComplexArray DSP::LPF(const ComplexArray & x, double sampleRate, double cutOffFreq)
	{
		Math l_math;

		auto l_X = l_math.FFT(x);
		auto l_XBin = l_math.getFreqBin(l_X, sampleRate);
		auto l_xProcessed = LPF(l_XBin, cutOffFreq);

		return l_xProcessed;
	}

	ComplexArray DSP::LPF(const FrequencyBinArray & x, double cutOffFreq)
	{
		Math l_math;

		ComplexArray l_xProcessed;
		FrequencyBinArray l_XBinProcessed;

		auto N = x.size();
		l_xProcessed.reserve(N);
		l_XBinProcessed.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			// @TODO: cut-off curve
			auto l_mag = x[i].first <= cutOffFreq ? x[i].second : 0.0;
			l_XBinProcessed.emplace_back(x[i].first, l_mag);
		}

		l_xProcessed = l_math.synth(l_XBinProcessed);

		return l_xProcessed;
	}

	ComplexArray DSP::HPF(const ComplexArray & x, double sampleRate, double cutOffFreq)
	{
		Math l_math;

		auto l_X = l_math.FFT(x);
		auto l_XBin = l_math.getFreqBin(l_X, sampleRate);
		auto l_xProcessed = HPF(l_XBin, cutOffFreq);

		return l_xProcessed;
	}

	ComplexArray DSP::HPF(const FrequencyBinArray & x, double cutOffFreq)
	{
		Math l_math;

		ComplexArray l_xProcessed;
		FrequencyBinArray l_XBinProcessed;

		auto N = x.size();
		l_xProcessed.reserve(N);
		l_XBinProcessed.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			// @TODO: cut-off curve
			auto l_mag = x[i].first >= cutOffFreq ? x[i].second : 0.0;
			l_XBinProcessed.emplace_back(x[i].first, l_mag);
		}

		l_xProcessed = l_math.synth(l_XBinProcessed);

		return l_xProcessed;
	}
}