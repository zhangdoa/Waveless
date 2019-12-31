#pragma once
#include "../Core/stdafx.h"
#include "../Core/Math.h"

namespace Waveless
{
	class Plotter
	{
	public:
		Plotter() = default;
		~Plotter() = default;

		static void Plot(const ComplexArray& rhs);
		static void Plot(const FreqBinArray& rhs);
		static void Show();
	};
}