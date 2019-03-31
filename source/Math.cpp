#include "Math.h"

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

FrequencyArray Math::DFT(const SignalArray& x)
{
	FrequencyArray X;
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

SignalArray Math::IDFT(const FrequencyArray & X)
{
	SignalArray x;
	auto N = X.size();
	x.reserve(N);

	for (size_t k = 0; k < N; k++)
	{
		auto s = std::complex<double>(0.0, 0.0);

		for (size_t i = 0; i < N; i++)
		{
			s += (1.0 / (double)N) * X[i] * std::exp(std::complex<double>(0.0, 1.0) * 2.0 * PI * (double)k * ((double)i / (double)N));
		}

		x.emplace_back(s.real());
	}

	return x;
}