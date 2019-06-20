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
	ComplexArray signal_1 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
	auto signal_1_DFT = l_math.DFT(signal_1);
	auto signal_1_IDFT = l_math.IDFT(signal_1_DFT);
	auto signal_1_FFT = l_math.FFT(signal_1);
	auto signal_1_IFFT = l_math.IFFT(signal_1_FFT);

	// test case : wave file loading and parsing
	WaveParser l_parser;
	auto l_waveData = l_parser.loadFile("..//res//test.wav");
	auto l_wavHeader = l_waveData.wavHeader;
	auto l_sampleRate = reinterpret_cast<StandardWavHeader*>(l_wavHeader)->fmtChunk.nSamplesPerSec;

	// test case : sinusoid generation
	auto signal_2 = l_math.genSine(1.0, 4.0, 0.0, 20.0, 2.0);
	auto signal_2_FFT = l_math.FFT(signal_2);
	auto signal_2_bin = l_math.FreqDomainSeries2FreqBin(signal_2_FFT, 20.0);
	auto signal_2_freq = l_math.FreqBin2FreqDomainSeries(signal_2_bin);
	auto signal_2_synth = l_math.synth(signal_2_bin);

	// test case : get freq bin of wave data
	ComplexArray signal_3(l_waveData.rawData.begin(), l_waveData.rawData.end());
	auto signal_3_FFT = l_math.FFT(signal_3);
	auto signal_3_bin = l_math.FreqDomainSeries2FreqBin(signal_3_FFT, l_sampleRate);

	// test case : DSP
	auto l_rawDataProcessed = l_DSP.gain(signal_3, -4.5);
	l_rawDataProcessed = l_DSP.LPF(l_rawDataProcessed, l_sampleRate, 5000.0);
	l_rawDataProcessed = l_DSP.HPF(l_rawDataProcessed, l_sampleRate, 300.0);

	// test case : write to new wave file
	l_parser.writeFile("..//res//test_Processed.wav", l_wavHeader, l_rawDataProcessed);

	// test case : write to new wave file
	auto l_newWavHeader = l_parser.genStandardWavHeader(1, l_sampleRate, 16, (unsigned long)signal_2_synth.size());
	l_parser.writeFile("..//res//test_Sinusoid.wav", &l_newWavHeader, signal_2_synth);
	return 0;
}