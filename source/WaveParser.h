#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

enum class WavHeaderType
{
	Standard, NonPCM, Extensible, BWF
};

struct RIFFChunk
{
	char                ckID[4];        // RIFF Header      Magic header
	unsigned long       cksize;      // RIFF Chunk Size
	char                WAVEID[4];        // WAVE Header
};

struct factChunk
{
	char                ckID[4]; // "fact"  string
	unsigned long       cksize;  //
	unsigned long       dwSampleLength;
};

struct StandardFmtChunk
{
	char                ckID[4];         // FMT header
	unsigned long       cksize;  // Size of the fmt chunk
	unsigned short      wFormatTag;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	unsigned short      nChannels;      // Number of channels 1=Mono 2=Sterio
	unsigned long       nSamplesPerSec;  // Sampling Frequency in Hz
	unsigned long       nAvgBytesPerSec;    // bytes per second
	unsigned short      nBlockAlign;     // 2=16-bit mono, 4=16-bit stereo
	unsigned short      wBitsPerSample;  // Number of bits per sample
};

struct NonPCMFmtChunk
{
	char                ckID[4];         // FMT header
	unsigned long       cksize;  // Size of the fmt chunk
	unsigned short      wFormatTag;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	unsigned short      nChannels;      // Number of channels 1=Mono 2=Sterio
	unsigned long       nSamplesPerSec;  // Sampling Frequency in Hz
	unsigned long       nAvgBytesPerSec;    // bytes per second
	unsigned short      nBlockAlign;     // 2=16-bit mono, 4=16-bit stereo
	unsigned short      wBitsPerSample;  // Number of bits per sample
	unsigned short      cbsize; // Size of the extension
};

struct ExtensibleFmtChunk
{
	char                ckID[4];         // FMT header
	unsigned long       cksize;  // Size of the fmt chunk
	unsigned short      wFormatTag;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	unsigned short      nChannels;      // Number of channels 1=Mono 2=Sterio
	unsigned long       nSamplesPerSec;  // Sampling Frequency in Hz
	unsigned long       nAvgBytesPerSec;    // bytes per second
	unsigned short      nBlockAlign;     // 2=16-bit mono, 4=16-bit stereo
	unsigned short      wBitsPerSample;  // Number of bits per sample
	unsigned short      cbsize; // Size of the extension
	unsigned short      wValidBitsPerSample;
	unsigned long       dwChannelMask; // Speaker position mask
	char                SubFormat[16]; // GUID (first two bytes are the data format code)
};

struct bextChunk
{
	char                ckID[4]; // bext header
	unsigned long       cksize; // Size of the bext chunk
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
	unsigned short      MaxTruePeakLevel; //unsigned short : Maximum True Peak Level of the fileexpressed as dBTP(multiplied by 100)
	unsigned short      MaxMomentaryLoudness; //unsigned short : Highest value of the MomentaryLoudness Level of the file in LUFS(multipliedby 100)
	unsigned short      MaxShortTermLoudness; //unsigned short : Highest value of the Short-TermLoudness Level of the file in LUFS(multipliedby 100)
	char                Reserved[180]; //180 bytes, reserved for future use, set to ¡°NULL¡±
	//char              CodingHistory[]; //ASCII :  History coding
};

struct DataChunk
{
	char                ckID[4]; // "data"  string
	unsigned long       cksize;  // Sampled data length
};

struct IWavHeader
{
	WavHeaderType type;
};

struct  StandardWavHeader : public IWavHeader
{
	RIFFChunk           RIFFChunk;
	StandardFmtChunk    fmtChunk;
	DataChunk           DataChunk;
};

struct  NonPCMWavHeader : public IWavHeader
{
	RIFFChunk           RIFFChunk;
	NonPCMFmtChunk      fmtChunk;
	factChunk           factChunk;
	DataChunk           DataChunk;
};

struct  ExtensibleWavHeader : public IWavHeader
{
	RIFFChunk           RIFFChunk;
	ExtensibleFmtChunk  fmtChunk;
	factChunk           factChunk;
	DataChunk           DataChunk;
};

struct  BWFWavHeader : public IWavHeader
{
	RIFFChunk           RIFFChunk;
	ExtensibleFmtChunk  fmtChunk;
	bextChunk           bextChunk;
	factChunk           factChunk;
	DataChunk           DataChunk;
};

struct WaveData
{
	IWavHeader* wavHeader;
	std::vector<short> rawData;
};

class WaveParser
{
public:
	WaveParser() = default;
	~WaveParser() = default;

	WaveData loadFile(const std::string& path);
	bool writeFile(const std::string & path, const WaveData& waveData);

	// in dB
	WaveData gain(float gainLevel, const WaveData& waveData);

	void printWavHeader(IWavHeader* wavHeader);
};