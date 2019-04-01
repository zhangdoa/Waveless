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

		///
		/// Generate A real sinusoid signal series.
		///
		ComplexArray genSine(
			double A ///< Amplitude in dB
			, double f ///< Frequency in Hz
			, double phi ///< Initial phase offset
			, double fs ///< Sample rate in Hz
			, double t ///< Sample period in second
		);

		ComplexArray DFT(const ComplexArray& x);

		ComplexArray IDFT(const ComplexArray& X);

		ComplexArray FFT(const ComplexArray& x);

		ComplexArray IFFT(const ComplexArray& x);

		///
		/// Convert the frequency domain signal series to a frequency bin collection.
		///
		FrequencyBinArray FreqDomainSeries2FreqBin(
			const ComplexArray& X /// Input frequency domain signal series
			, double fs /// Sample rate in Hz
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series.
		///
		ComplexArray FreqBin2FreqDomainSeries(
			const FrequencyBinArray& XBin /// Input frequency bin collection
		);

		///
		/// Synthenize a time domain signal series by a given frequency bin collection.
		///
		ComplexArray synth(
			const FrequencyBinArray & XBin /// Input frequency bin collection
		);

		const double PI = 3.1415926536;

	private:
		ComplexArray FFT_Impl(const ComplexArray& x);

		ComplexArray IFFT_Impl(const ComplexArray& x);
	};
}
