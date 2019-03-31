#include "Math.h"

namespace Waveless
{
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
		ComplexArray evenFreq = FFT(evenSamples);
		ComplexArray oddFreq = FFT(oddSamples);

		// combine result
		ComplexArray result(N);

		for (size_t i = 0; i < N; i++)
		{
			result[i] = evenFreq[i % (N / 2)] + w[i] * oddFreq[i % (N / 2)];
		}

		return result;
	}

	ComplexArray Math::IFFT(const ComplexArray & x)
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
		ComplexArray evenFreq = FFT(evenSamples);
		ComplexArray oddFreq = FFT(oddSamples);

		// combine result
		ComplexArray result(N);

		for (size_t i = 0; i < N; i++)
		{
			result[i] = (evenFreq[i % (N / 2)] + w[i] * oddFreq[i % (N / 2)]) / (double)N;
		}

		return result;
	}

	FrequencyBinArray Math::getFreqBin(const ComplexArray& X, double sampleRate)
	{
		FrequencyBinArray binArray;
		auto N = X.size();
		binArray.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			auto freq = sampleRate * (double)i / (double)N;
			// @TODO: not correct coversion
			auto amp = linear2dBMag(std::abs(X[i].real()));
			FrequencyBin bin(freq, std::complex<double>(amp, 0.0));
			binArray.emplace_back(bin);
		}

		return binArray;
	}

	ComplexArray Math::synth(const FrequencyBinArray & XBin)
	{
		ComplexArray x;
		auto N = XBin.size();
		x.reserve(N);

		ComplexArray X;
		X.reserve(N);

		for (size_t i = 0; i < N; i++)
		{
			X.emplace_back(dB2LinearMag(XBin[i].second.real()));
		}

		return IFFT(X);
	}
}