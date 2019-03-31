#include "stdafx.h"

using SignalArray = std::vector<double>;
using FrequencyArray = std::vector<std::complex<double>>;

class Math
{
public:
	Math() = default;
	~Math() = default;

	double dB2LinearAmp(double dB);

	double linear2dBAmp(double linear);

	double dB2LinearMag(double dB);

	double linear2dBMag(double linear);

	// Return in angular frequency
	FrequencyArray DFT(const SignalArray& x);

	// Input in angular frequency
	SignalArray IDFT(const FrequencyArray& X);

	const double PI = 3.1415926536;
};