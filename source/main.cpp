#pragma once
#include "WaveParser.h"
#include "Math.h"
#include "DSP.h"

using namespace Waveless;

int main()
{
	Waveless::Math l_math;
	DSP l_DSP;

	// test case : DFT, IDFT, FFT and IFFT
	ComplexArray x = { 1.0, 2.0, 3.0, 4.0 };
	ComplexArray X_DFT = l_math.DFT(x);
	ComplexArray x_IDFT = l_math.IDFT(X_DFT);
	ComplexArray X_FFT = l_math.FFT(x);
	ComplexArray x_IFFT = l_math.IFFT(X_FFT);

	// test case : wave file loading and parsing
	WaveParser l_parser;
	auto l_waveData = l_parser.loadFile("..//res//test.wav");
	auto l_wavHeader = l_waveData.wavHeader;
	auto l_sampleRate = reinterpret_cast<StandardWavHeader*>(l_wavHeader)->fmtChunk.nSamplesPerSec;

	// test case : sinusoid generation
	ComplexArray x_Sinusoid = l_math.genSine(1.0, 4.0, 0.0, 20.0, 2.0);
	ComplexArray X_Sinusoid = l_math.FFT(x_Sinusoid);
	FrequencyBinArray SinusoidBin = l_math.FreqDomainSeries2FreqBin(X_Sinusoid, 20.0);
	ComplexArray X_SinusoidSynth = l_math.FreqBin2FreqDomainSeries(SinusoidBin);
	ComplexArray x_SinusoidSynth = l_math.synth(SinusoidBin);

	// test case : get freq bin of wave data
	ComplexArray x2(l_waveData.rawData.begin(), l_waveData.rawData.end());
	ComplexArray X2 = l_math.FFT(x2);
	FrequencyBinArray X2Bin = l_math.FreqDomainSeries2FreqBin(X2, l_sampleRate);

	// test case : DSP
	auto l_rawDataProcessed = l_DSP.gain(x2, -4.5);
	l_rawDataProcessed = l_DSP.LPF(l_rawDataProcessed, l_sampleRate, 5000.0);
	l_rawDataProcessed = l_DSP.HPF(l_rawDataProcessed, l_sampleRate, 300.0);

	// test case : write to new wave file
	l_parser.writeFile("..//res//test_Processed.wav", l_wavHeader, l_rawDataProcessed);

	// test case : write to new wave file
	auto l_newWavHeader = l_parser.genStandardWavHeader(1, l_sampleRate, 16, (unsigned long)x_SinusoidSynth.size());
	l_parser.writeFile("..//res//test_Sinusoid.wav", &l_newWavHeader, x_SinusoidSynth);
	return 0;
}