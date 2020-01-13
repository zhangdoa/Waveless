#pragma once
#include "../Core/stdafx.h"
#include "../Core/Typedef.h"
#include "../Core/Math.h"

namespace Waveless
{
	enum class WavHeaderType
	{
		Standard, NonPCM, Extensible, BWF
	};

#pragma pack (push, 1)
	struct RIFFChunk
	{
		char                ckID[4]; // "RIFF" string
		unsigned long       ckSize = 0; // RIFF Chunk Size
		char                RIFFType[4]; // "WAVE" string
	};
#pragma pack(pop)

#pragma pack (push, 1)
	struct JunkChunk
	{
		char ckID[4]; // "JUNK" or "junk" string
		unsigned long ckSize = 0; // This must be at least 28 if the chunk is intended as a place-holder for a "ds64" chunk.
		//char chunkData[] // dummy bytes
	};
#pragma pack(pop)

#pragma pack (push, 1)
	struct fmtChunk
	{
		// Standard
		char                ckID[4];         // "fmt" string
		unsigned long       ckSize = 0;  // Size of the fmt chunk
		unsigned short      wFormatTag;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
		unsigned short      nChannels;      // Number of channels 1=Mono 2=Stereo
		unsigned long       nSamplesPerSec;  // Sampling Frequency in Hz
		unsigned long       nAvgBytesPerSec;    // bytes per second
		unsigned short      nBlockAlign;     // 2=16-bit mono, 4=16-bit stereo
		unsigned short      wBitsPerSample;  // Number of bits per sample

		// Non-PCM
		unsigned short      cbSize = 0; // Size of the extension

		// Extensible
		unsigned short      wValidBitsPerSample;
		unsigned long       dwChannelMask; // Speaker position mask
		char                SubFormat[16]; // GUID (first two bytes are the data format code)
	};
#pragma pack(pop)

#pragma pack (push, 1)
	struct factChunk
	{
		char                ckID[4]; // "fact" string
		unsigned long       ckSize;  //
		unsigned long       dwSampleLength;
	};
#pragma pack(pop)

#pragma pack (push, 1)
	struct bextChunk
	{
		char                ckID[4]; // "bext" string
		unsigned long       ckSize = 0; // Size of the bext chunk
		char                Description[256]; //ASCII : Description of the sound sequence
		char                Originator[32]; //ASCII : Name of the originator
		char                OriginatorReference[32]; //ASCII : Reference of the originator
		char                OriginationDate[10]; //ASCII : yyyy:mm:dd
		char                OriginationTime[8]; //ASCII : hh:mm:ss
		unsigned long       TimeReferenceLow; //First sample count since midnight, low word
		unsigned long       TimeReferenceHigh; //First sample count since midnight, high word
		unsigned short      Version; //Version of the BWF; unsigned binary number
		char                UMID[64]; // Binary byte of SMPTE UMID
		unsigned short      LoudnessValue; //unsigned short : Integrated Loudness Value of the file in LUFS (multiplied by 100)
		unsigned short      LoudnessRange; //unsigned short : Loudness Range of the file in LU(multiplied by 100)
		unsigned short      MaxTruePeakLevel; //unsigned short : Maximum True Peak Level of the file expressed as dBTP(multiplied by 100)
		unsigned short      MaxMomentaryLoudness; //unsigned short : Highest value of the MomentaryLoudness Level of the file in LUFS(multiplied by 100)
		unsigned short      MaxShortTermLoudness; //unsigned short : Highest value of the Short-TermLoudness Level of the file in LUFS(multiplied by 100)
		char                Reserved[180]; //180 bytes, reserved for future use, set to ¡°NULL¡±
		//char              CodingHistory[]; //ASCII : History coding
	};
#pragma pack(pop)

#pragma pack (push, 1)
	struct dataChunk
	{
		char                ckID[4]; // "data" string
		unsigned long       ckSize = 0;  // Sampled data length
	};
#pragma pack(pop)

	struct WavHeader
	{
		RIFFChunk RIFFChunk;
		JunkChunk JunkChunk;
		fmtChunk fmtChunk;
		factChunk factChunk;
		bextChunk bextChunk;
		dataChunk dataChunk;
		int ChunkValidities[6] = { 0 };
	};

	struct WavObject
	{
		WavHeader header;
		std::vector<char> sample;
	};

	class WaveParser
	{
	public:
		WaveParser() = default;
		~WaveParser() = default;

		static WavObject LoadFile(const char* path);

		static WavHeader GenerateWavHeader(unsigned short channels, unsigned long sampleRate, unsigned short bitDepth, unsigned long sampleCount);
		static WavObject GenerateWavObject(const WavHeader& header, const ComplexArray& x);

		static WsResult WriteFile(const char* path, const WavObject& wavObject);
		static WsResult WriteFile(const char* path, const WavHeader& header, const ComplexArray& x);

		static void PrintWavHeader(WavHeader* header);
	};
}