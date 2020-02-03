#include "Math.h"

namespace Waveless
{
	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

	uint64_t Math::GenerateUUID()
	{
		return dist(e2);
	}

	double Math::DB2LinearMag(double dB)
	{
		return std::pow(10, dB / 10.0);
	}

	double Math::Linear2dBMag(double linear)
	{
		return 10.0 * std::log10(linear);
	}

	double Math::DB2LinearAmp(double dB)
	{
		return std::pow(10, dB / 20.0);
	}

	double Math::Linear2dBAmp(double linear)
	{
		return 20.0 * std::log10(linear);
	}

	ComplexArray Math::GenerateSine(double A, double f, double phi, double fs, double t)
	{
		auto N = size_t(std::round(t * fs));
		ComplexArray x(N);

		auto l_Intensity = DB2LinearAmp(A);

		for (size_t i = 0; i < N; i++)
		{
			x[i] = l_Intensity * std::sin(2 * PI<double> * f * (double)i / fs + phi);
		}

		return x;
	}

	ComplexArray Math::DFT(const ComplexArray& x)
	{
		auto N = x.size();
		ComplexArray X(N);

		for (size_t k = 0; k < N; k++)
		{
			auto s = Complex(0.0, 0.0);

			for (size_t i = 0; i < N; i++)
			{
				s += x[i] * std::exp(Complex(0.0, -1.0) * 2.0 * PI<double> * (double)k * ((double)i / (double)N));
			}

			X[k] = s;
		}

		return X;
	}

	ComplexArray Math::IDFT(const ComplexArray & X)
	{
		auto N = X.size();
		ComplexArray x(N);

		for (size_t k = 0; k < N; k++)
		{
			auto s = Complex(0.0, 0.0);

			for (size_t i = 0; i < N; i++)
			{
				s += (1.0 / (double)N) * X[i] * std::exp(Complex(0.0, 1.0) * 2.0 * PI<double> * (double)k * ((double)i / (double)N));
			}

			x[k] = s;
		}

		return x;
	}

	ComplexArray Math::GenerateWindowFunction(WindowDesc windowDesc)
	{
		auto N = windowDesc.m_WindowSize;

		ComplexArray l_result(N);

		switch (windowDesc.m_WindowType)
		{
		case Waveless::WindowType::Rectangular:
			l_result = 1.0;
			break;
		case Waveless::WindowType::Hann:
			for (size_t i = 0; i < N; i++)
			{
				l_result[i] = pow(sin(PI<double> * (double)i / (double)N), 2.0);
			}
			break;
		case Waveless::WindowType::Hamming:
			for (size_t i = 0; i < N; i++)
			{
				l_result[i] = 0.53836 + 0.46164 * cos(2.0 * PI<double> * (double)i / (double)N);
			}
			break;
		case Waveless::WindowType::Blackman:
			for (size_t i = 0; i < N; i++)
			{
				l_result[i] = 0.42659 - 0.49656 * cos(2.0 * PI<double> * (double)i / (double)N) + 0.076849 * cos(4.0 * PI<double> * (double)i / (double)N);
			}
			break;
		case Waveless::WindowType::Nuttall:
			for (size_t i = 0; i < N; i++)
			{
				l_result[i] = 0.355768 - 0.487396 * cos(2.0 * PI<double> * (double)i / (double)N) + 0.144232 * cos(4.0 * PI<double> * (double)i / (double)N) - 0.012604 * cos(6.0 * PI<double> * (double)i / (double)N);
			}
			break;
		case Waveless::WindowType::BlackmanNuttall:
			for (size_t i = 0; i < N; i++)
			{
				l_result[i] = 0.3635819 - 0.4891775 * cos(2.0 * PI<double> * (double)i / (double)N) + 0.1365995 * cos(4.0 * PI<double> * (double)i / (double)N) - 0.0106411 * cos(6.0 * PI<double> * (double)i / (double)N);
			}
			break;
		case Waveless::WindowType::BlackmanHarris:
			for (size_t i = 0; i < N; i++)
			{
				l_result[i] = 0.35875 - 0.48829 * cos(2.0 * PI<double> * (double)i / (double)N) + 0.14128 * cos(4.0 * PI<double> * (double)i / (double)N) - 0.01168 * cos(6.0 * PI<double> * (double)i / (double)N);
			}
			break;
		default:
			break;
		}

		return l_result;
	}

	std::vector<ComplexArray> Math::FFT(const ComplexArray & x, WindowDesc windowDesc)
	{
		auto N = x.size();
		std::vector<ComplexArray> l_result;

		if (N < windowDesc.m_WindowSize)
		{
			N = windowDesc.m_WindowSize;

			// zero padding
			ComplexArray l_paddedChunk(N);

			for (size_t i = 0; i < x.size(); i++)
			{
				l_paddedChunk[i] = x[i];
			}

			auto l_zeroPaddingSize = N - x.size();
			for (size_t i = 0; i < l_zeroPaddingSize; i++)
			{
				l_paddedChunk[i + x.size()] = Complex(0.0, 0.0);
			}

			FFT_SingleFrame(l_paddedChunk);

			l_result.emplace_back(l_paddedChunk);
		}
		else
		{
			auto l_window = GenerateWindowFunction(windowDesc);

			ComplexArray l_X(N);

			auto l_FFTFrameSize = windowDesc.m_WindowSize;
			auto l_lastFrameSize = N % l_FFTFrameSize;
			auto l_frameCount = (N - l_lastFrameSize) / l_FFTFrameSize;

			for (size_t i = 0; i < l_frameCount * 2 - 1; i++)
			{
				auto l_chunk = ComplexArray(x[std::slice(i * l_FFTFrameSize / 2, l_FFTFrameSize, 1)]);
				l_chunk *= l_window;

				FFT_SingleFrame(l_chunk);
				l_result.emplace_back(l_chunk);
			};

			// full size chunks
			// @TODO: window overlapping
			if (l_lastFrameSize)
			{
				// last chunk with zero padding
				ComplexArray l_paddedLastChunk(l_FFTFrameSize);
				auto l_lastChunk = ComplexArray(x[std::slice(l_frameCount * l_FFTFrameSize, l_lastFrameSize, 1)]);

				for (size_t i = 0; i < l_lastChunk.size(); i++)
				{
					l_paddedLastChunk[i] = l_lastChunk[i];
				}

				auto l_zeroPaddingSize = l_FFTFrameSize - l_lastFrameSize;
				for (size_t i = 0; i < l_zeroPaddingSize; i++)
				{
					l_paddedLastChunk[i + l_lastFrameSize] = Complex(0.0, 0.0);
				}

				FFT_SingleFrame(l_paddedLastChunk);
				l_result.emplace_back(l_paddedLastChunk);
			}
		}

		return l_result;
	}

	ComplexArray Math::IFFT(const std::vector<ComplexArray> & X, WindowDesc windowDesc)
	{
		std::vector<Complex> l_vector;
		l_vector.reserve(X.size() * X[0].size());

		if (X.size() == 1)
		{
			auto X0 = X[0];
			IFFT_SingleFrame(X0);
			l_vector.insert(std::end(l_vector), std::begin(X0), std::end(X0));
		}
		else
		{
			auto l_window = GenerateWindowFunction(windowDesc);

			for (auto Xi : X)
			{
				IFFT_SingleFrame(Xi);
				Xi /= l_window;
				l_vector.insert(std::end(l_vector), std::begin(Xi), std::end(Xi));
			}
		}

		ComplexArray l_result(l_vector.data(), l_vector.size());

		return l_result;
	}

	void Math::FFT_SingleFrame(ComplexArray & x)
	{
		const size_t N = x.size();
		if (N <= 1)
		{
			return;
		}

		// Divide
		ComplexArray even = x[std::slice(0, N / 2, 2)];
		ComplexArray odd = x[std::slice(1, N / 2, 2)];

		// Conquer
		FFT_SingleFrame(even);
		FFT_SingleFrame(odd);

		// Combine
		for (size_t k = 0; k < N / 2; ++k)
		{
			Complex t = std::polar(1.0, -2 * PI<double> * k / N) * odd[k];
			x[k] = even[k] + t;
			x[k + N / 2] = even[k] - t;
		}
	}

	void Math::IFFT_SingleFrame(ComplexArray & X)
	{
		// Conjugate the complex numbers
		X = X.apply(std::conj);

		// forward FFT
		FFT_SingleFrame(X);

		// Conjugate the complex numbers again
		X = X.apply(std::conj);

		// Scale the numbers
		X /= (double)X.size();
	}

	std::vector<FreqBinData> Math::FreqDomainSeries2FreqBin(const std::vector<ComplexArray>& X, double fs)
	{
		std::vector<FreqBinData> l_result;

		for (auto& Xi : X)
		{
			auto l_XBinData = FreqDomainSeries2FreqBin_SingleFrame(Xi, fs);
			l_result.emplace_back(l_XBinData);
		}

		return l_result;
	}

	std::vector<ComplexArray> Math::FreqBin2FreqDomainSeries(const std::vector<FreqBinData>& XBinData)
	{
		std::vector<ComplexArray> l_result;

		for (auto& XBinDatai : XBinData)
		{
			auto l_X = FreqBin2FreqDomainSeries_SingleFrame(XBinDatai);
			l_result.emplace_back(l_X);
		}

		return l_result;
	}

	FreqBinData Math::FreqDomainSeries2FreqBin_SingleFrame(const ComplexArray& X, double sampleRate)
	{
		FreqBinData XBinData;
		auto N = X.size();

		// X[0] is the DC Offset
		XBinData.m_DCOffset = X[0];

		// X[i] == X[N - i], symmetry of Fourier Transform
		// Then only first half of the data in frequent domain is useful, other half are duplication
		auto l_binSize = N / 2;
		XBinData.m_FreqBinArray.reserve(l_binSize);

		for (size_t i = 1; i < l_binSize + 1; i++)
		{
			auto freq = sampleRate * (double)i / (double)N;
			//auto amp_real = linear2dBMag(std::abs(X[i].real()));
			//amp_real = X[i].real() > 0.0 ? amp_real : -amp_real;
			auto amp_real = X[i].real();
			//auto amp_imag = linear2dBMag(std::abs(X[i].imag()));
			//amp_imag = X[i].imag() > 0.0 ? amp_imag : -amp_imag;
			auto amp_imag = X[i].imag();
			FreqBin bin(freq, Complex(amp_real, amp_imag));
			XBinData.m_FreqBinArray.emplace_back(bin);
		}

		return XBinData;
	}

	ComplexArray Math::FreqBin2FreqDomainSeries_SingleFrame(const FreqBinData & XBinData)
	{
		auto N = XBinData.m_FreqBinArray.size();
		auto l_dataSize = N * 2;
		ComplexArray X(l_dataSize);

		// X[0] is the DC Offset
		X[0] = XBinData.m_DCOffset;

		// First half of data
		for (size_t i = 0; i < N; i++)
		{
			//auto amp_real = dB2LinearMag(XBinData[i].second.real());
			//amp_real = XBinData[i].second.real() > 0.0 ? amp_real : -amp_real;
			auto amp_real = XBinData.m_FreqBinArray[i].second.real();
			//auto amp_imag = dB2LinearMag(XBinData[i].second.imag());
			//amp_imag = XBinData[i].second.imag() > 0.0 ? amp_imag : -amp_imag;
			auto amp_imag = XBinData.m_FreqBinArray[i].second.imag();
			X[i + 1] = Complex(amp_real, amp_imag);
		}

		// Second half of data, without XBinData[N - 1]
		for (size_t i = 0; i < N; i++)
		{
			auto amp_real = X[N - i].real();
			auto amp_imag = X[N - i].imag();
			X[i + N] = Complex(amp_real, amp_imag);
		}

		return X;
	}

	ComplexArray Math::Synth(const std::vector<FreqBinData>& XBinData, WindowDesc windowDesc)
	{
		std::vector<Complex> l_vector;
		l_vector.reserve(XBinData.size() * XBinData[0].m_FreqBinArray.size());

		// @TODO: Merge window overlap
		for (auto XBinDatai : XBinData)
		{
			auto l_x = Synth_SingleFrame(XBinDatai);
			l_vector.insert(std::end(l_vector), std::begin(l_x), std::end(l_x));
		}

		ComplexArray l_result(l_vector.data(), l_vector.size());

		return l_result;
	}

	ComplexArray Math::Synth_SingleFrame(const FreqBinData & XBinData)
	{
		auto l_X = FreqBin2FreqDomainSeries_SingleFrame(XBinData);
		IFFT_SingleFrame(l_X);
		return l_X;
	}
}