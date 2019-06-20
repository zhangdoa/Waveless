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

		static double DB2LinearAmp(double dB);

		static double Linear2dBAmp(double linear);

		static double DB2LinearMag(double dB);

		static double Linear2dBMag(double linear);

		///
		/// Generate A real sinusoid signal series.
		///
		static ComplexArray GenerateSine(
			double A ///< Amplitude in dB
			, double f ///< Frequency in Hz
			, double phi ///< Initial phase offset
			, double fs ///< Sample rate in Hz
			, double t ///< Sample period in second
		);

		static ComplexArray DFT(const ComplexArray& x);

		static ComplexArray IDFT(const ComplexArray& X);

		static std::vector<ComplexArray> FFT(const ComplexArray& x);

		static ComplexArray IFFT(const std::vector<ComplexArray>& X);

		static ComplexArray FFT_SingleFrame(const ComplexArray& x);

		static ComplexArray IFFT_SingleFrame(const ComplexArray& x);

		///
		/// Convert the frequency domain signal series to a frequency bin collection.
		///
		static std::vector<FrequencyBinArray> FreqDomainSeries2FreqBin(
			const std::vector<ComplexArray>& X /// Input frequency domain signal series
			, double fs /// Sample rate in Hz
		);

		///
		/// Convert the frequency domain signal series to a frequency bin collection. Single frame impl.
		///
		static FrequencyBinArray FreqDomainSeries2FreqBin_SingleFrame(
			const ComplexArray& X /// Input frequency domain signal series
			, double fs /// Sample rate in Hz
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series.
		///
		static std::vector <ComplexArray> FreqBin2FreqDomainSeries(
			const std::vector<FrequencyBinArray>& XBin /// Input frequency bin collection
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series. Single frame impl.
		///
		static ComplexArray FreqBin2FreqDomainSeries_SingleFrame(
			const FrequencyBinArray& XBin /// Input frequency bin collection
		);

		///
		/// Synthenize a time domain signal series by a given frequency bin collection.
		///
		static ComplexArray Synth(
			const std::vector<FrequencyBinArray>& XBin /// Input frequency bin collection
		);

		///
		/// Synthenize a time domain signal series by a given frequency bin collection. Single frame impl.
		///
		static ComplexArray Synth_SingleFrame(
			const FrequencyBinArray& XBin /// Input frequency bin collection
		);

		inline static const double PI = 3.1415926536;
	};
}
