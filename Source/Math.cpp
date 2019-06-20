#include "Math.h"

namespace Waveless
{
	static std::vector<unsigned int> FFTSize = { 2, 4, 8, 16, 32,64,128,256,512,1024,2048 };

	double Math::DB2LinearAmp(double dB)
	{
		return std::pow(10, dB / 10.0);
	}

	double Math::Linear2dBAmp(double linear)
	{
		return 10.0 * std::log10(linear);
	}

	double Math::DB2LinearMag(double dB)
	{
		return std::pow(10, dB / 20.0);
	}

	double Math::Linear2dBMag(double linear)
	{
		return 20.0 * std::log10(linear);
	}

	ComplexArray Math::GenerateSine(double A, double f, double phi, double fs, double t)
	{
		ComplexArray x;
		auto N = size_t(std::round(t * fs));
		x.reserve(N);

		auto l_MagLinear = DB2LinearMag(A);

		for (size_t i = 0; i < N; i++)
		{
			auto sample = l_MagLinear * std::cos(2 * PI * f * (double)i / fs + phi);
			x.emplace_back(sample);
		}

		return x;
	}

	ComplexArray Math::DFT(const ComplexArray& x)
	{
		ComplexArray X;
		auto N = x.size();
		X.reserve(N);

		for (size_t k = 0; k < N; k++)
		{
			auto s = std::complex<double>(0.0, 0.0);

			for (size_t i = 0; i < N; i++)
			{
				s += x[i] * std::exp(std::complex<double>(0.0, -1.0) * 2.0 * PI * (double)k * ((double)i / (double)N));
			}

			X.emplace_back(s);
		}

		return X;
	}

	ComplexArray Math::IDFT(const ComplexArray & X)
	{
		ComplexArray x;
		auto N = X.size();
		x.reserve(N);

		for (size_t k = 0; k < N; k++)
		{
			auto s = std::complex<double>(0.0, 0.0);

			for (size_t i = 0; i < N; i++)
			{
				s += (1.0 / (double)N) * X[i] * std::exp(std::complex<double>(0.0, 1.0) * 2.0 * PI * (double)k * ((double)i / (double)N));
			}

			x.emplace_back(s);
		}

		return x;
	}

	std::vector<ComplexArray> Math::FFT(const ComplexArray & x)
	{
		// @TODO: window and zero padding
		auto N = x.size();
		bool l_isSizeSmallerThanFFTSize = false;

		for (auto i : FFTSize)
		{
			if (N <= i)
			{
				N = i;
				l_isSizeSmallerThanFFTSize = true;
				break;
			}
		}

		std::vector<ComplexArray> l_result;

		if (!l_isSizeSmallerThanFFTSize)
		{
			ComplexArray l_X;
			l_X.reserve(N);

			auto l_largestFFTChunckSize = FFTSize[FFTSize.size() - 1];
			auto l_lastChunkSize = N % l_largestFFTChunckSize;
			auto l_fullChunkNumber = (N - l_lastChunkSize) / l_largestFFTChunckSize;

			// full chunk
			for (size_t i = 0; i < l_fullChunkNumber; i++)
			{
				auto l_chunk = ComplexArray(&x[i * l_largestFFTChunckSize], &x[(i + 1) * l_largestFFTChunckSize]);
				auto l_FFTResult = FFT_SingleFrame(l_chunk);
				l_result.emplace_back(l_FFTResult);
			};

			// last chunk with zero padding
			auto l_zeroPaddingSize = l_largestFFTChunckSize - l_lastChunkSize;
			auto l_lastChunk = ComplexArray(&x[l_fullChunkNumber * l_largestFFTChunckSize], &x[x.size() - 1]);

			for (size_t i = 0; i < l_zeroPaddingSize + 1; i++)
			{
				l_lastChunk.emplace_back(std::complex<double>(0.0, 0.0));
			}

			auto l_FFTResult = FFT_SingleFrame(l_lastChunk);
			l_result.emplace_back(l_FFTResult);
		}
		else
		{
			ComplexArray l_X;
			l_X.reserve(N);

			// zero padding
			auto l_zeroPaddingSize = N - x.size();
			auto l_padx = x;
			for (size_t i = 0; i < l_zeroPaddingSize; i++)
			{
				l_padx.emplace_back(std::complex<double>(0.0, 0.0));
			}

			auto l_FFTResult = FFT_SingleFrame(l_padx);

			l_result.emplace_back(l_FFTResult);
		}

		return l_result;
	}

	ComplexArray Math::IFFT(const std::vector<ComplexArray> & X)
	{
		ComplexArray l_result;

		for (auto Xi : X)
		{
			auto N = Xi.size();
			auto l_x = IFFT_SingleFrame(Xi);
			for (auto& i : l_x)
			{
				i /= (double)N;
			}
			l_result.insert(std::end(l_result), std::begin(l_x), std::end(l_x));
		}

		return l_result;
	}

	ComplexArray Math::FFT_SingleFrame(const ComplexArray & x)
	{
		auto N = x.size();

		// base case
		if (N == 1)
		{
			return ComplexArray(1, x[0]);
		}

		ComplexArray w(N);
		for (size_t i = 0; i < N; i++)
		{
			double alpha = 2 * PI * i / N;
			w[i] = std::complex<double>(cos(alpha), sin(alpha));
		}

		// divide
		ComplexArray evenSamples(N / 2);
		ComplexArray oddSamples(N / 2);

		for (int i = 0; i < N / 2; i++)
		{
			evenSamples[i] = x[i * 2];
			oddSamples[i] = x[i * 2 + 1];
		}

		// conquer
		ComplexArray evenFreq = FFT_SingleFrame(evenSamples);
		ComplexArray oddFreq = FFT_SingleFrame(oddSamples);

		// combine result
		ComplexArray result(N);

		for (size_t i = 0; i < N; i++)
		{
			result[i] = evenFreq[i % (N / 2)] + w[i] * oddFreq[i % (N / 2)];
		}

		return result;
	}

	ComplexArray Math::IFFT_SingleFrame(const ComplexArray & X)
	{
		auto N = X.size();

		// base case
		if (N == 1)
		{
			return ComplexArray(1, X[0]);
		}

		ComplexArray w(N);
		for (size_t i = 0; i < N; i++)
		{
			double alpha = 2 * PI * i / N;
			w[i] = std::complex<double>(cos(alpha), -sin(alpha));
		}

		// divide
		ComplexArray evenSamples(N / 2);
		ComplexArray oddSamples(N / 2);

		for (int i = 0; i < N / 2; i++)
		{
			evenSamples[i] = X[i * 2];
			oddSamples[i] = X[i * 2 + 1];
		}

		// conquer
		ComplexArray evenFreq = IFFT_SingleFrame(evenSamples);
		ComplexArray oddFreq = IFFT_SingleFrame(oddSamples);

		// combine result
		ComplexArray result(N);

		for (size_t i = 0; i < N; i++)
		{
			result[i] = evenFreq[i % (N / 2)] + w[i] * oddFreq[i % (N / 2)];
		}

		return result;
	}

	std::vector<FrequencyBinArray> Math::FreqDomainSeries2FreqBin(const std::vector<ComplexArray>& X, double fs)
	{
		std::vector<FrequencyBinArray> l_result;

		for (auto Xi : X)
		{
			auto l_XBin = FreqDomainSeries2FreqBin_SingleFrame(Xi, fs);
			l_result.emplace_back(l_XBin);
		}

		return l_result;
	}

	std::vector<ComplexArray> Math::FreqBin2FreqDomainSeries(const std::vector<FrequencyBinArray>& XBin)
	{
		std::vector<ComplexArray> l_result;

		for (auto XBini : XBin)
		{
			auto l_X = FreqBin2FreqDomainSeries_SingleFrame(XBini);
			l_result.emplace_back(l_X);
		}

		return l_result;
	}

	FrequencyBinArray Math::FreqDomainSeries2FreqBin_SingleFrame(const ComplexArray& X, double sampleRate)
	{
		FrequencyBinArray XBin;
		auto N = X.size();
		XBin.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			auto freq = sampleRate * (double)i / (double)N;
			//auto amp_real = linear2dBMag(std::abs(X[i].real()));
			//amp_real = X[i].real() > 0.0 ? amp_real : -amp_real;
			auto amp_real = X[i].real();
			//auto amp_imag = linear2dBMag(std::abs(X[i].imag()));
			//amp_imag = X[i].imag() > 0.0 ? amp_imag : -amp_imag;
			auto amp_imag = X[i].imag();
			FrequencyBin bin(freq, std::complex<double>(amp_real, amp_imag));
			XBin.emplace_back(bin);
		}

		return XBin;
	}

	ComplexArray Math::FreqBin2FreqDomainSeries_SingleFrame(const FrequencyBinArray & XBin)
	{
		ComplexArray X;
		auto N = XBin.size();
		X.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			//auto amp_real = dB2LinearMag(XBin[i].second.real());
			//amp_real = XBin[i].second.real() > 0.0 ? amp_real : -amp_real;
			auto amp_real = XBin[i].second.real();
			//auto amp_imag = dB2LinearMag(XBin[i].second.imag());
			//amp_imag = XBin[i].second.imag() > 0.0 ? amp_imag : -amp_imag;
			auto amp_imag = XBin[i].second.imag();
			X.emplace_back(std::complex<double>(amp_real, amp_imag));
		}

		return X;
	}

	ComplexArray Math::Synth(const std::vector<FrequencyBinArray>& XBin)
	{
		ComplexArray l_result;

		for (auto XBini : XBin)
		{
			auto l_x = Synth_SingleFrame(XBini);
			l_result.insert(std::end(l_result), std::begin(l_x), std::end(l_x));
		}

		return l_result;
	}

	ComplexArray Math::Synth_SingleFrame(const FrequencyBinArray & XBin)
	{
		auto l_X = FreqBin2FreqDomainSeries_SingleFrame(XBin);
		return IFFT_SingleFrame(l_X);
	}
}