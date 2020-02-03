#pragma once
#include "stdafx.h"

namespace Waveless
{
	template<class T>
	inline static const T PI = T(3.14159265358979323846264338327950288L);

	using Complex = std::complex<double>;
	using ComplexArray = std::valarray<Complex>;

	using Freq = double;
	using FreqBin = std::pair<Freq, Complex>;
	using FreqBinArray = std::vector<FreqBin>;

	struct FreqBinData
	{
		Complex m_DCOffset;
		FreqBinArray m_FreqBinArray;
	};

	enum class WindowType
	{
		Rectangular,
		Hann,
		Hamming,
		Blackman,
		Nuttall,
		BlackmanNuttall,
		BlackmanHarris
	};

	struct WindowDesc
	{
		WindowType m_WindowType;
		size_t m_WindowSize;
	};

	class Math
	{
	public:
		Math() = default;
		~Math() = default;

		static uint64_t GenerateUUID();

		static double DB2LinearMag(double dB);

		static double Linear2dBMag(double linear);

		static double DB2LinearAmp(double dB);

		static double Linear2dBAmp(double linear);

		///
		/// Generate A real sinusoid signal series.
		///
		static ComplexArray GenerateSine(
			double A ///< Amplitude in dB
			, double f ///< Freq in Hz
			, double phi ///< Initial phase offset
			, double fs ///< Sample rate in Hz
			, double t ///< Sample period in second
		);

		static ComplexArray DFT(const ComplexArray& x);

		static ComplexArray IDFT(const ComplexArray& X);

		static ComplexArray GenerateWindowFunction(WindowDesc windowDesc);

		static std::vector<ComplexArray> FFT(const ComplexArray& x, WindowDesc windowDesc);

		static ComplexArray IFFT(const std::vector<ComplexArray>& X, WindowDesc windowDesc);

		static void FFT_SingleFrame(ComplexArray& x);

		static void IFFT_SingleFrame(ComplexArray& x);

		///
		/// Convert the frequency domain signal series to a frequency bin collection.
		///
		static std::vector<FreqBinData> FreqDomainSeries2FreqBin(
			const std::vector<ComplexArray>& X ///< Input frequency domain signal series
			, double fs ///< Sample rate in Hz
		);

		///
		/// Convert the frequency domain signal series to a frequency bin collection. Single frame version.
		///
		static FreqBinData FreqDomainSeries2FreqBin_SingleFrame(
			const ComplexArray& X ///< Input frequency domain signal series
			, double fs ///< Sample rate in Hz
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series.
		///
		static std::vector <ComplexArray> FreqBin2FreqDomainSeries(
			const std::vector<FreqBinData>& XBinData ///< Input frequency bin collection
		);

		///
		/// Convert the frequency bin collection to a frequency domain signal series. Single frame version.
		///
		static ComplexArray FreqBin2FreqDomainSeries_SingleFrame(
			const FreqBinData& XBinData ///< Input frequency bin collection
		);

		///
		/// Synthesize a time domain signal series by a given frequency bin collection.
		///
		static ComplexArray Synth(
			const std::vector<FreqBinData>& XBinData, ///< Input frequency bin collection
			WindowDesc windowDesc ///< STFT window description
		);

		///
		/// Synthesize a time domain signal series by a given frequency bin collection. Single frame version.
		///
		static ComplexArray Synth_SingleFrame(
			const FreqBinData& XBinData ///< Input frequency bin collection
		);
	};
}
