#include "Plotter.h"

namespace Waveless
{
	void Plotter::Plot(const ComplexArray & rhs)
	{
		std::vector<double> l_bin;
		l_bin.resize(rhs.size());
		std::vector<double> l_real;
		l_real.resize(rhs.size());
		std::vector<double> l_imagine;
		l_imagine.resize(rhs.size());

		for (size_t i = 0; i < rhs.size(); i++)
		{
			l_bin[i] = double(i);
		}

		for (size_t i = 0; i < rhs.size(); i++)
		{
			l_real[i] = Math::Linear2dBMag(rhs[i].real());
		}

		for (size_t i = 0; i < rhs.size(); i++)
		{
			l_imagine[i] = Math::Linear2dBMag(rhs[i].imag());
		}
	}

	void Plotter::Plot(const FreqBinArray & rhs)
	{
		std::vector<double> l_bin;
		l_bin.resize(rhs.size());
		std::vector<double> l_real;
		l_real.resize(rhs.size());
		std::vector<double> l_imagine;
		l_imagine.resize(rhs.size());

		for (size_t i = 0; i < rhs.size(); i++)
		{
			l_bin[i] = rhs[i].first;
		}

		for (size_t i = 0; i < rhs.size(); i++)
		{
			l_real[i] = rhs[i].second.real();
		}

		for (size_t i = 0; i < rhs.size(); i++)
		{
			l_imagine[i] = rhs[i].second.imag();
		}
	}

	void Plotter::Show()
	{
	}
}