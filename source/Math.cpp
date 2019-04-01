#include "Math.h"

namespace Waveless
{
	static unsigned int FFTSize[] = { 2, 4, 8, 16, 32,64,128,256,512,1024,2048 };

	double Math::dB2LinearAmp(double dB)
	{
		return std::pow(10, dB / 10.0);
	}

	double Math::linear2dBAmp(double linear)
	{
		return 10.0 * std::log10(linear);
	}

	double Math::dB2LinearMag(double dB)
	{
		return std::pow(10, dB / 20.0);
	}

	double Math::linear2dBMag(double linear)
	{
		return 20.0 * std::log10(linear);
	}

	ComplexArray Math::genSine(double A, double f, double phi, double fs, double t)
	{
		ComplexArray x;
		auto N = size_t(std::round(t * fs));
		x.reserve(N);

		auto l_MagLinear = dB2LinearMag(A);

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

	ComplexArray Math::FFT(const ComplexArray & x)
	{
		// @TODO: window and zero padding
		auto N = x.size();
		return FFT_Impl(x);
	}

	ComplexArray Math::IFFT(const ComplexArray & x)
	{
		return IFFT_Impl(x);
	}

	ComplexArray Math::FFT_Impl(const ComplexArray & x)
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
		ComplexArray evenFreq = FFT_Impl(evenSamples);
		ComplexArray oddFreq = FFT_Impl(oddSamples);

		// combine result
		ComplexArray result(N);

		for (size_t i = 0; i < N; i++)
		{
			result[i] = evenFreq[i % (N / 2)] + w[i] * oddFreq[i % (N / 2)];
		}

		return result;
	}

	ComplexArray Math::IFFT_Impl(const ComplexArray & x)
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
			w[i] = std::complex<double>(cos(alpha), -sin(alpha));
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
		ComplexArray evenFreq = FFT_Impl(evenSamples);
		ComplexArray oddFreq = FFT_Impl(oddSamples);

		// combine result
		ComplexArray result(N);

		for (size_t i = 0; i < N; i++)
		{
			result[i] = (evenFreq[i % (N / 2)] + w[i] * oddFreq[i % (N / 2)]) / (double)N;
		}

		return result;
	}

	FrequencyBinArray Math::FreqDomainSeries2FreqBin(const ComplexArray& X, double sampleRate)
	{
		FrequencyBinArray binArray;
		auto N = X.size() / 2;
		binArray.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			auto freq = sampleRate * (double)i / (double)N;
			auto amp_real = linear2dBMag(std::abs(X[i].real()));
			auto amp_imag = linear2dBMag(std::abs(X[i].imag()));
			FrequencyBin bin(freq, std::complex<double>(amp_real, amp_imag));
			binArray.emplace_back(bin);
		}

		return binArray;
	}

	ComplexArray Math::FreqBin2FreqDomainSeries(const FrequencyBinArray & XBin)
	{
		ComplexArray X;
		auto N = XBin.size();
		X.reserve(N * 2);

		for (size_t i = 0; i < N; i++)
		{
			auto amp_real = dB2LinearMag(XBin[i].second.real());
			auto amp_imag = dB2LinearMag(XBin[i].second.imag());
			X.emplace_back(std::complex<double>(amp_real, amp_imag));
		}

		// @TODO: not correct coversion
		for (size_t i = 1; i < N; i++)
		{
			X.emplace_back(X[N - i].real(), -X[N - i].imag());
		}

		return X;
	}

	ComplexArray Math::synth(const FrequencyBinArray & XBin)
	{
		auto l_X = FreqBin2FreqDomainSeries(XBin);
		return IFFT_Impl(l_X);
	}
}