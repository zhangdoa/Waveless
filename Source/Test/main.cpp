#pragma once
#include "../IO/WaveParser.h"
#include "../Core/Math.h"
#include "../Core/DSP.h"
#include "../Runtime/Plotter.h"
#include "../Runtime/AudioEngine.h"

using namespace Waveless;

void testOfflineFeatures()
{
	WindowDesc l_windowDesc;
	l_windowDesc.m_WindowSize = 512;
	l_windowDesc.m_WindowType = WindowType::BlackmanHarris;

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
	auto l_newWavHeader = WaveParser::GenerateWavHeader(1, 44100, 16, (unsigned long)signal_2.size());
	WaveParser::WriteFile("..//..//Asset//test_Sinusoid_Original.wav", l_newWavHeader, signal_2);
	WaveParser::WriteFile("..//..//Asset//test_Sinusoid_Resampled.wav", l_newWavHeader, signal_2_synth);

	Plotter::Show();

	// test case: wave file loading and parsing
	auto l_wavObject = WaveParser::LoadFile("..//..//Asset//test_Sinusoid_Original.wav");
	auto l_sampleRate = l_wavObject.header.fmtChunk.nSamplesPerSec;

	// test case : get freq bin of wave data
	// @TODO: Raw sample to ComplexArray
	std::vector<Complex> l_singal3Temp(l_wavObject.sample.begin(), l_wavObject.sample.end());
	ComplexArray signal_3(l_singal3Temp.data(), l_singal3Temp.size());
	auto signal_3_FFT = Math::FFT(signal_3, l_windowDesc);
	auto signal_3_bin = Math::FreqDomainSeries2FreqBin(signal_3_FFT, l_sampleRate);

	// test case : DSP
	auto l_sampleProcessed = DSP::Gain(signal_3, -4.5);
	l_sampleProcessed = DSP::LPF(l_sampleProcessed, l_sampleRate, 5000.0);
	l_sampleProcessed = DSP::HPF(l_sampleProcessed, l_sampleRate, 300.0);

	// test case : write to new wave file
	WaveParser::WriteFile("..//..//Asset//test_Sinusoid_Processed.wav", l_wavObject.header, l_sampleProcessed);
}

void testRealTimeFeatures()
{
	AudioEngine::Initialize();

	auto l_wavObjectA = WaveParser::LoadFile("..//..//Asset//testA.wav");
	auto l_wavObjectB = WaveParser::LoadFile("..//..//Asset//testB.wav");

	auto l_eventIDA = AudioEngine::AddEventPrototype(l_wavObjectA);
	auto l_eventIDB = AudioEngine::AddEventPrototype(l_wavObjectB);

	AudioEngine::Trigger(l_eventIDA);

	AudioEngine::Trigger(l_eventIDB);

	AudioEngine::Flush();

	AudioEngine::Terminate();
}

int main()
{
	//testOfflineFeatures();
	testRealTimeFeatures();

	return 0;
}