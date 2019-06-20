#pragma once
#include "stdafx.h"
#include "Math.h"

namespace Waveless
{
	class Plotter
	{
	public:
		Plotter() = default;
		~Plotter() = default;

		static void Plot(const ComplexArray& rhs);
		static void Plot(const FrequencyBinArray& rhs);
		static void Show();
	};
}