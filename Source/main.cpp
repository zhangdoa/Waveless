#pragma once
#include "WaveParser.h"
#include "Math.h"
#include "DSP.h"
#include "Plotter.h"
#include "AudioEngine.h"

using namespace Waveless;

int main()
{
	WindowDesc l_windowDesc;
	l_windowDesc.m_WindowSize = 512;
	l_windowDesc.m_WindowType = WindowType::BlackmanHarris;

	AudioEngine::Initialize();

	// test case : DFT, IDFT, FFT and IFFT
	ComplexArray signal_1 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
	auto signal_1_DFT = Math::DFT(signal_1);
	auto signal_1_IDFT = Math::IDFT(signal_1_DFT);
	auto signal_1_FFT = Math::FFT(signal_1, l_windowDesc);
	auto signal_1_IFFT = Math::IFFT(signal_1_FFT, l_windowDesc);
	Plotter::Plot(signal_1);

	// test case : sinusoid generation
	auto signal_2 = Math::GenerateSine(64.0, 440.0, 0.0, 44100.0, 4.0);
	auto signal_2_FFT = Math::FFT(signal_2, l_windowDesc);
	auto signal_2_bin = Math::FreqDomainSeries2FreqBin(signal_2_FFT, 44100.0);
	auto signal_2_freq = Math::FreqBin2FreqDomainSeries(signal_2_bin);
	auto signal_2_synth = Math::Synth(signal_2_bin, l_windowDesc);
	Plotter::Plot(signal_2_synth);

	// test case : write to new wave file
	auto l_newWavHeader = WaveParser::GenerateStandardWavHeader(1, 44100, 16, (unsigned long)signal_2_synth.size());
	WaveParser::WriteFile("..//Asset//test_Sinusoid.wav", &l_newWavHeader, signal_2_synth);
	Plotter::Show();
	// test case : wave file loading and parsing
	auto l_waveData = WaveParser::LoadFile("..//Asset//test.wav");
	auto l_wavHeader = reinterpret_cast<StandardWavHeader*>(l_waveData.wavHeader);
	auto l_sampleRate = l_wavHeader->fmtChunk.nSamplesPerSec;
	AudioEngine::Play(l_waveData);
	AudioEngine::Run();

	// test case : get freq bin of wave data
	std::vector<Complex> l_singal3Temp(l_waveData.rawData.begin(), l_waveData.rawData.end());
	ComplexArray signal_3(l_singal3Temp.data(), l_singal3Temp.size());
	auto signal_3_FFT = Math::FFT(signal_3, l_windowDesc);
	auto signal_3_bin = Math::FreqDomainSeries2FreqBin(signal_3_FFT, l_sampleRate);

	// test case : DSP
	auto l_rawDataProcessed = DSP::Gain(signal_3, -4.5);
	//l_rawDataProcessed = DSP::LPF(l_rawDataProcessed, l_sampleRate, 5000.0);
	l_rawDataProcessed = DSP::HPF(l_rawDataProcessed, l_sampleRate, 300.0);

	// test case : write to new wave file
	WaveParser::WriteFile("..//Asset//test_Processed.wav", l_wavHeader, l_rawDataProcessed);

	AudioEngine::Terminate();

	return 0;
}