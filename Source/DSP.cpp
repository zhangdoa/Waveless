#include "DSP.h"

namespace Waveless
{
	ComplexArray DSP::Gain(const ComplexArray & x, double gainLevel)
	{
		auto N = x.size();
		ComplexArray l_xProcessed(N);

		for (size_t i = 0; i < N; i++)
		{
			l_xProcessed[i] = x[i] * Math::DB2LinearAmp(gainLevel);
		}

		return l_xProcessed;
	}

	ComplexArray DSP::LPF(const ComplexArray & x, double fs, double cutOffFreq)
	{
		auto l_X = x;
		Math::FFT_SingleFrame(l_X);
		auto l_XBin = Math::FreqDomainSeries2FreqBin_SingleFrame(l_X, fs);
		auto l_xProcessed = LPF(l_XBin, cutOffFreq);

		return l_xProcessed;
	}

	ComplexArray DSP::LPF(const FreqBinData & XBinData, double cutOffFreq)
	{
		auto N = XBinData.m_FreqBinArray.size();

		ComplexArray l_xProcessed(N);
		FreqBinData l_XBinProcessed;

		l_XBinProcessed.m_FreqBinArray.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			// @TODO: cut-off curve
			auto l_mag = XBinData.m_FreqBinArray[i].first <= cutOffFreq ? XBinData.m_FreqBinArray[i].second : 0.0;
			l_XBinProcessed.m_FreqBinArray.emplace_back(XBinData.m_FreqBinArray[i].first, l_mag);
		}

		l_xProcessed = Math::Synth_SingleFrame(l_XBinProcessed);

		return l_xProcessed;
	}

	ComplexArray DSP::HPF(const ComplexArray & x, double fs, double cutOffFreq)
	{
		auto l_X = x;
		Math::FFT_SingleFrame(l_X);
		auto l_XBin = Math::FreqDomainSeries2FreqBin_SingleFrame(l_X, fs);
		auto l_xProcessed = HPF(l_XBin, cutOffFreq);

		return l_xProcessed;
	}

	ComplexArray DSP::HPF(const FreqBinData & XBinData, double cutOffFreq)
	{
		auto N = XBinData.m_FreqBinArray.size();

		ComplexArray l_xProcessed(N);
		FreqBinData l_XBinProcessed;

		l_XBinProcessed.m_FreqBinArray.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			// @TODO: cut-off curve
			auto l_mag = XBinData.m_FreqBinArray[i].first >= cutOffFreq ? XBinData.m_FreqBinArray[i].second : 0.0;
			l_XBinProcessed.m_FreqBinArray.emplace_back(XBinData.m_FreqBinArray[i].first, l_mag);
		}

		l_xProcessed = Math::Synth_SingleFrame(l_XBinProcessed);

		return l_xProcessed;
	}
}