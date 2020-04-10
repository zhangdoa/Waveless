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
	auto signal_2_IFFT = Math::IFFT(signal_2_FFT, l_windowDesc);

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
	auto signal_3 = WaveParser::GenerateComplexArray(l_wavObject);
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

	auto l_wavObject_A = WaveParser::LoadFile("..//..//Asset//testA.wav");
	auto l_wavObject_B = WaveParser::LoadFile("..//..//Asset//testB.wav");

	auto l_signal = Math::GenerateSine(64.0, 440.0, 0.0, 44100.0, 4.0);
	auto l_wavHeader = WaveParser::GenerateWavHeader(1, 44100, 16, (unsigned long)l_signal.size());
	auto l_wavObject_C = WaveParser::GenerateWavObject(l_wavHeader, l_signal);
	auto l_wavObject_D = WaveParser::LoadFile("..//..//Asset//testC.wav");

	auto l_eventID_A = AudioEngine::AddEventPrototype(l_wavObject_A);
	auto l_eventID_B = AudioEngine::AddEventPrototype(l_wavObject_B);
	auto l_eventID_C = AudioEngine::AddEventPrototype(l_wavObject_C);
	auto l_eventID_D = AudioEngine::AddEventPrototype(l_wavObject_D);

	auto l_eventInstanceID_A = AudioEngine::Trigger(l_eventID_A);
	auto l_eventInstanceID_B = AudioEngine::Trigger(l_eventID_B);
	auto l_eventInstanceID_C = AudioEngine::Trigger(l_eventID_C);
	auto l_eventInstanceID_D = AudioEngine::Trigger(l_eventID_D);

	AudioEngine::Flush();

	float t = 10.0f;
	while (t < 20000.0f)
	{
		t += 0.001f;
		AudioEngine::ApplyLPF(l_eventInstanceID_B, t);
	}

	AudioEngine::Terminate();
}

#include "../Core/WsCanvasAPIExport.h"
WS_CANVAS_API void EventScript_MinimalTest(Vector& in_Position);

void testCanvasDLL()
{
	Vector in_Position;
	EventScript_MinimalTest(in_Position);
}

int main()
{
	//testOfflineFeatures();
	//testRealTimeFeatures();
	testCanvasDLL();

	return 0;
}