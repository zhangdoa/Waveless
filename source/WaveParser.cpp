#include "WaveParser.h"
#include "Math.h"

void printLog(const std::string& rhs)
{
	std::cout << rhs << std::endl;
}

void printRIFFChunk(RIFFChunk rhs)
{
	printLog("RIFF: " + std::string(rhs.ckID));
	printLog("ChunkSize: " + std::to_string(rhs.cksize));
	printLog("WAVE: " + std::string(rhs.WAVEID));
}

void printStandardFmtChunk(StandardFmtChunk rhs)
{
	printLog("Fmt: " + std::string(rhs.ckID));
	printLog("FmtSize: " + std::to_string(rhs.cksize));
	printLog("AudioFormat: " + std::to_string(rhs.wFormatTag));
	printLog("NumOfChan: " + std::to_string(rhs.nChannels));
	printLog("SamplesPerSec: " + std::to_string(rhs.nSamplesPerSec));
	printLog("bytesPerSec: " + std::to_string(rhs.nAvgBytesPerSec));
	printLog("blockAlign: " + std::to_string(rhs.nBlockAlign));
	printLog("bitsPerSample: " + std::to_string(rhs.wBitsPerSample));
}

void printNonPCMFmtChunk(NonPCMFmtChunk rhs)
{
	printLog("Fmt: " + std::string(rhs.ckID));
	printLog("FmtSize: " + std::to_string(rhs.cksize));
	printLog("AudioFormat: " + std::to_string(rhs.wFormatTag));
	printLog("NumOfChan: " + std::to_string(rhs.nChannels));
	printLog("SamplesPerSec: " + std::to_string(rhs.nSamplesPerSec));
	printLog("bytesPerSec: " + std::to_string(rhs.nAvgBytesPerSec));
	printLog("blockAlign: " + std::to_string(rhs.nBlockAlign));
	printLog("bitsPerSample: " + std::to_string(rhs.wBitsPerSample));
	printLog("ExtensionSize: " + std::to_string(rhs.cbsize));
}

void printExtensibleFmtChunk(ExtensibleFmtChunk rhs)
{
	printLog("Fmt: " + std::string(rhs.ckID));
	printLog("FmtSize: " + std::to_string(rhs.cksize));
	printLog("AudioFormat: " + std::to_string(rhs.wFormatTag));
	printLog("NumOfChan: " + std::to_string(rhs.nChannels));
	printLog("SamplesPerSec: " + std::to_string(rhs.nSamplesPerSec));
	printLog("bytesPerSec: " + std::to_string(rhs.nAvgBytesPerSec));
	printLog("blockAlign: " + std::to_string(rhs.nBlockAlign));
	printLog("bitsPerSample: " + std::to_string(rhs.wBitsPerSample));
	printLog("ExtensionSize: " + std::to_string(rhs.cbsize));
	printLog("ValidBitsPerSample: " + std::to_string(rhs.wValidBitsPerSample));
	printLog("ChannelMask: " + std::to_string(rhs.dwChannelMask));
	printLog("SubFormat: " + std::string(rhs.SubFormat));
}

void printbextChunk(bextChunk rhs)
{
	printLog("Bext: " + std::string(rhs.ckID));
	printLog("BextSize: " + std::to_string(rhs.cksize));
	printLog("Description: " + std::string(rhs.Description));
	printLog("Originator: " + std::string(rhs.Originator));
	printLog("OriginatorReference: " + std::string(rhs.OriginatorReference));
	printLog("OriginationDate: " + std::string(rhs.OriginationDate));
	printLog("OriginationTime: " + std::string(rhs.OriginationTime));
	printLog("TimeReferenceLow: " + std::to_string(rhs.TimeReferenceLow));
	printLog("TimeReferenceHigh: " + std::to_string(rhs.TimeReferenceHigh));
	printLog("Version: " + std::to_string(rhs.Version));
	printLog("UMID: " + std::string(rhs.UMID));
	printLog("LoudnessValue: " + std::to_string(rhs.LoudnessValue));
	printLog("LoudnessRange: " + std::to_string(rhs.LoudnessRange));
	printLog("MaxTruePeakLevel: " + std::to_string(rhs.MaxTruePeakLevel));
	printLog("MaxMomentaryLoudness: " + std::to_string(rhs.MaxMomentaryLoudness));
	printLog("MaxShortTermLoudness: " + std::to_string(rhs.MaxShortTermLoudness));
}

void printfactChunk(factChunk rhs)
{
	printLog("Data: " + std::string(rhs.ckID));
	printLog("DataSize: " + std::to_string(rhs.cksize));
	printLog("SampleLength: " + std::to_string(rhs.dwSampleLength));
}

void printDataChunk(DataChunk rhs)
{
	printLog("Data: " + std::string(rhs.ckID));
	printLog("DataSize: " + std::to_string(rhs.cksize));
}

void printStandardWavHeader(StandardWavHeader* wavHeader)
{
	printRIFFChunk(wavHeader->RIFFChunk);
	printStandardFmtChunk(wavHeader->fmtChunk);
	printDataChunk(wavHeader->DataChunk);
}

void printNonPCMWavHeader(NonPCMWavHeader* wavHeader)
{
	printRIFFChunk(wavHeader->RIFFChunk);
	printNonPCMFmtChunk(wavHeader->fmtChunk);
	printfactChunk(wavHeader->factChunk);
	printDataChunk(wavHeader->DataChunk);
}

void printExtensibleWavHeader(ExtensibleWavHeader* wavHeader)
{
	printRIFFChunk(wavHeader->RIFFChunk);
	printExtensibleFmtChunk(wavHeader->fmtChunk);
	printfactChunk(wavHeader->factChunk);
	printDataChunk(wavHeader->DataChunk);
}

void printBWFWavHeader(BWFWavHeader* wavHeader)
{
	printRIFFChunk(wavHeader->RIFFChunk);
	printExtensibleFmtChunk(wavHeader->fmtChunk);
	printbextChunk(wavHeader->bextChunk);
	printfactChunk(wavHeader->factChunk);
	printDataChunk(wavHeader->DataChunk);
}

void WaveParser::printWavHeader(IWavHeader* wavHeader)
{
	switch (wavHeader->type)
	{
	case WavHeaderType::Standard: printStandardWavHeader(reinterpret_cast<StandardWavHeader*>(wavHeader)); break;
	case WavHeaderType::NonPCM: printNonPCMWavHeader(reinterpret_cast<NonPCMWavHeader*>(wavHeader)); break;
	case WavHeaderType::Extensible: printExtensibleWavHeader(reinterpret_cast<ExtensibleWavHeader*>(wavHeader)); break;
	case WavHeaderType::BWF: printBWFWavHeader(reinterpret_cast<BWFWavHeader*>(wavHeader)); break;
	default:
		break;
	}
}

WaveData WaveParser::loadFile(const std::string & path)
{
	std::ifstream l_file(path, std::ios::binary);

	if (!l_file.is_open())
	{
		throw std::runtime_error("std::ifstream: can't open file " + path + "!");
	}

	// get pointer to associated buffer object
	auto pbuf = l_file.rdbuf();

	// get file size using buffer's members
	std::size_t l_size = pbuf->pubseekoff(0, l_file.end, l_file.in);

	// choose header format
	pbuf->pubseekpos(16, l_file.in);
	unsigned long l_chuckSize;
	pbuf->sgetn((char*)&l_chuckSize, sizeof(l_chuckSize));

	WaveData l_result;
	size_t l_WavHeaderSize = 0;
	size_t l_rawDataSize = 0;

	if (l_chuckSize == 16)
	{
		printLog(path + " is Standard Wave format");

		// vptr
		l_WavHeaderSize = sizeof(StandardWavHeader) - 4;
		l_rawDataSize = l_size - l_WavHeaderSize;
		l_result.wavHeader = new StandardWavHeader();
		l_result.wavHeader->type = WavHeaderType::Standard;
		pbuf->pubseekpos(0, l_file.in);
		pbuf->sgetn((char*)l_result.wavHeader + 4, l_WavHeaderSize);
	}
	else if (l_chuckSize == 18)
	{
		printLog(path + " is Non-PCM Wave format");

		// vptr
		l_WavHeaderSize = sizeof(NonPCMWavHeader) - 4;
		l_rawDataSize = l_size - l_WavHeaderSize;
		l_result.wavHeader = new NonPCMWavHeader();
		l_result.wavHeader->type = WavHeaderType::NonPCM;
		pbuf->pubseekpos(0, l_file.in);
		pbuf->sgetn((char*)l_result.wavHeader + 4, l_WavHeaderSize);
	}
	else if (l_chuckSize == 40)
	{
		char isBWF[4];

		pbuf->pubseekpos(60, l_file.in);
		pbuf->sgetn((char*)&isBWF, sizeof(isBWF));

		if (isBWF[0] == *"b" || *"m")
		{
			printLog(path + " is Broadcast Wave format");

			//get RIFFChunk + ExtensibleFmtChunk + bextChunk size
			l_WavHeaderSize = 0;
			l_WavHeaderSize += sizeof(RIFFChunk);
			l_WavHeaderSize += sizeof(ExtensibleFmtChunk);
			l_WavHeaderSize += sizeof(bextChunk);

			// load temp header
			auto l_BWFHeader = new BWFWavHeader();
			pbuf->pubseekpos(0, l_file.in);
			pbuf->sgetn((char*)l_BWFHeader + 4, l_WavHeaderSize);

			// load code history
			auto l_bextSize = l_BWFHeader->bextChunk.cksize;

			auto l_codeHistorySectorPos = 60 + sizeof(bextChunk);
			auto l_codeHistorySectorLength = l_bextSize - sizeof(bextChunk);

			std::vector<char> l_codeHistory(l_codeHistorySectorLength);

			pbuf->pubseekpos(l_codeHistorySectorPos, l_file.in);
			pbuf->sgetn((char*)&l_codeHistory[0], l_codeHistorySectorLength);

			// load fact chunk and data chunk @TODO: WIP
			auto l_factChunkPos = l_codeHistorySectorPos + l_codeHistorySectorLength;
			pbuf->pubseekpos(l_factChunkPos, l_file.in);
			pbuf->sgetn((char*)&l_BWFHeader->factChunk, sizeof(factChunk));
			pbuf->sgetn((char*)&l_BWFHeader->DataChunk, sizeof(DataChunk));

			l_result.wavHeader = l_BWFHeader;
			auto l_rawDataPos = l_factChunkPos + sizeof(factChunk) + sizeof(DataChunk) + 1;
			pbuf->pubseekpos(l_rawDataPos, l_file.in);
			l_result.wavHeader->type = WavHeaderType::BWF;
		}
		else
		{
			printLog(path + " is Extensible Wave format");

			// vptr
			l_WavHeaderSize = sizeof(ExtensibleWavHeader) - 4;
			l_rawDataSize = l_size - l_WavHeaderSize;
			l_result.wavHeader = new ExtensibleWavHeader();
			l_result.wavHeader->type = WavHeaderType::Extensible;
			pbuf->pubseekpos(0, l_file.in);
			pbuf->sgetn((char*)l_result.wavHeader + 4, l_WavHeaderSize);
		}
	}

	// load raw data
	auto l_rawData = std::vector<short>(l_rawDataSize);
	pbuf->sgetn((char*)&l_rawData[0], l_rawDataSize);
	l_result.rawData = std::move(l_rawData);

	printWavHeader(l_result.wavHeader);

	return l_result;
}

bool WaveParser::writeFile(const std::string & path, const WaveData& waveData)
{
	size_t l_sizeOfWavHeader = 0;

	switch (waveData.wavHeader->type)
	{
	case WavHeaderType::Standard: l_sizeOfWavHeader = sizeof(StandardWavHeader); break;
	case WavHeaderType::NonPCM:  l_sizeOfWavHeader = sizeof(NonPCMWavHeader); break;
	case WavHeaderType::Extensible: l_sizeOfWavHeader = sizeof(ExtensibleWavHeader); break;
	case WavHeaderType::BWF: l_sizeOfWavHeader = sizeof(BWFWavHeader); break;
	default:
		break;
	}

	std::ofstream l_file(path, std::ios::binary);

	l_file.write((char*)waveData.wavHeader + 4, l_sizeOfWavHeader - 4);
	l_file.write((char*)&waveData.rawData[0], waveData.rawData.size());

	l_file.close();
	return true;
}

WaveData WaveParser::gain(float gainLevel, const WaveData& waveData)
{
	Math l_math;
	WaveData l_result;

	l_result.wavHeader = waveData.wavHeader;
	l_result.rawData.reserve(waveData.rawData.size());

	for (size_t i = 0; i < waveData.rawData.size(); i++)
	{
		l_result.rawData.emplace_back((short)((double)waveData.rawData[i] * l_math.dB2LinearMag(gainLevel)));
	}

	return l_result;
}

inline void endian_swap(unsigned short& x)
{
	x = (x >> 8) |
		(x << 8);
}