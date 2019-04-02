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

		std::vector<ComplexArray> FFT(const ComplexArray& x);

		ComplexArray IFFT(const std::vector<ComplexArray>& X);

		ComplexArray FFT_SingleFrame(const ComplexArray& x);

		ComplexArray IFFT_SingleFrame(const ComplexArray& x);

		///
		/// Convert the frequency domain signal series to a frequency bin collection.
		///
		std::vector<FrequencyBinArray> FreqDomainSeries2FreqBin(
			const std::vector<ComplexArray>& X /// Input frequency domain signal series
			, double fs /// Sample rate in Hz
		);

		///
		/// Convert the frequency domain signal series to a frequency bin collection. Single frame impl.
		///
		FrequencyBinArray FreqDomainSeries2FreqBin_SingleFrame(
			const ComplexArray& X /// Input frequency domain signal series
			, double fs /// Sample rate in Hz
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series.
		///
		std::vector <ComplexArray> FreqBin2FreqDomainSeries(
			const std::vector<FrequencyBinArray>& XBin /// Input frequency bin collection
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series. Single frame impl.
		///
		ComplexArray FreqBin2FreqDomainSeries_SingleFrame(
			const FrequencyBinArray& XBin /// Input frequency bin collection
		);

		///
		/// Synthenize a time domain signal series by a given frequency bin collection.
		///
		ComplexArray synth(
			const std::vector<FrequencyBinArray>& XBin /// Input frequency bin collection
		);

		///
		/// Synthenize a time domain signal series by a given frequency bin collection. Single frame impl.
		///
		ComplexArray synth_SingleFrame(
			const FrequencyBinArray& XBin /// Input frequency bin collection
		);

		const double PI = 3.1415926536;
	};
}
