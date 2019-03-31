#pragma once
#include "stdafx.h"

namespace Waveless
{
	using ComplexArray = std::vector<std::complex<double>>;
	using FrequencyBin = std::pair<double, std::complex<double>>;
	using FrequencyBinArray = std::vector<FrequencyBin>;

	class Math
	{
	public:
		Math() = default;
		~Math() = default;

		double dB2LinearAmp(double dB);

		double linear2dBAmp(double linear);

		double dB2LinearMag(double dB);

		double linear2dBMag(double linear);

		ComplexArray DFT(const ComplexArray& x);

		ComplexArray IDFT(const ComplexArray& X);

		ComplexArray FFT(const ComplexArray& x);

		ComplexArray IFFT(const ComplexArray& x);

		// freq in Hz, magnitude in dB
		FrequencyBinArray getFreqBin(const ComplexArray& X, double sampleRate);

		ComplexArray synth(const FrequencyBinArray & XBin);

		const double PI = 3.1415926536;
	};
}
