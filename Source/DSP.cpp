#include "DSP.h"

namespace Waveless
{
	ComplexArray DSP::Gain(const ComplexArray & x, double gainLevel)
	{
		Math l_math;

		ComplexArray l_xProcessed;

		auto N = x.size();
		l_xProcessed.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			l_xProcessed.emplace_back(x[i] * l_math.DB2LinearAmp(gainLevel));
		}

		return l_xProcessed;
	}

	ComplexArray DSP::LPF(const ComplexArray & x, double fs, double cutOffFreq)
	{
		Math l_math;

		auto l_X = l_math.FFT_SingleFrame(x);
		auto l_XBin = l_math.FreqDomainSeries2FreqBin_SingleFrame(l_X, fs);
		auto l_xProcessed = LPF(l_XBin, cutOffFreq);

		return l_xProcessed;
	}

	ComplexArray DSP::LPF(const FreqBinData & XBinData, double cutOffFreq)
	{
		Math l_math;

		ComplexArray l_xProcessed;
		FreqBinData l_XBinProcessed;

		auto N = XBinData.m_FreqBinArray.size();

		l_xProcessed.reserve(N);
		l_XBinProcessed.m_FreqBinArray.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			// @TODO: cut-off curve
			auto l_mag = XBinData.m_FreqBinArray[i].first <= cutOffFreq ? XBinData.m_FreqBinArray[i].second : 0.0;
			l_XBinProcessed.m_FreqBinArray.emplace_back(XBinData.m_FreqBinArray[i].first, l_mag);
		}

		l_xProcessed = l_math.Synth_SingleFrame(l_XBinProcessed);

		return l_xProcessed;
	}

	ComplexArray DSP::HPF(const ComplexArray & x, double fs, double cutOffFreq)
	{
		Math l_math;

		auto l_X = l_math.FFT_SingleFrame(x);
		auto l_XBin = l_math.FreqDomainSeries2FreqBin_SingleFrame(l_X, fs);
		auto l_xProcessed = HPF(l_XBin, cutOffFreq);

		return l_xProcessed;
	}

	ComplexArray DSP::HPF(const FreqBinData & XBinData, double cutOffFreq)
	{
		Math l_math;

		ComplexArray l_xProcessed;
		FreqBinData l_XBinProcessed;

		auto N = XBinData.m_FreqBinArray.size();

		l_xProcessed.reserve(N);
		l_XBinProcessed.m_FreqBinArray.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			// @TODO: cut-off curve
			auto l_mag = XBinData.m_FreqBinArray[i].first >= cutOffFreq ? XBinData.m_FreqBinArray[i].second : 0.0;
			l_XBinProcessed.m_FreqBinArray.emplace_back(XBinData.m_FreqBinArray[i].first, l_mag);
		}

		l_xProcessed = l_math.Synth_SingleFrame(l_XBinProcessed);

		return l_xProcessed;
	}
}