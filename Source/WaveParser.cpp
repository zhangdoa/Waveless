#include "WaveParser.h"

namespace Waveless
{
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
		printLog("Fmt: " + std::string(rhs.standardFmtChunk.ckID));
		printLog("FmtSize: " + std::to_string(rhs.standardFmtChunk.cksize));
		printLog("AudioFormat: " + std::to_string(rhs.standardFmtChunk.wFormatTag));
		printLog("NumOfChan: " + std::to_string(rhs.standardFmtChunk.nChannels));
		printLog("SamplesPerSec: " + std::to_string(rhs.standardFmtChunk.nSamplesPerSec));
		printLog("bytesPerSec: " + std::to_string(rhs.standardFmtChunk.nAvgBytesPerSec));
		printLog("blockAlign: " + std::to_string(rhs.standardFmtChunk.nBlockAlign));
		printLog("bitsPerSample: " + std::to_string(rhs.standardFmtChunk.wBitsPerSample));
		printLog("ExtensionSize: " + std::to_string(rhs.cbsize));
	}

	void printExtensibleFmtChunk(ExtensibleFmtChunk rhs)
	{
		printLog("Fmt: " + std::string(rhs.standardFmtChunk.ckID));
		printLog("FmtSize: " + std::to_string(rhs.standardFmtChunk.cksize));
		printLog("AudioFormat: " + std::to_string(rhs.standardFmtChunk.wFormatTag));
		printLog("NumOfChan: " + std::to_string(rhs.standardFmtChunk.nChannels));
		printLog("SamplesPerSec: " + std::to_string(rhs.standardFmtChunk.nSamplesPerSec));
		printLog("bytesPerSec: " + std::to_string(rhs.standardFmtChunk.nAvgBytesPerSec));
		printLog("blockAlign: " + std::to_string(rhs.standardFmtChunk.nBlockAlign));
		printLog("bitsPerSample: " + std::to_string(rhs.standardFmtChunk.wBitsPerSample));
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

	void printStandardWavHeader(StandardWavHeader* header)
	{
		printRIFFChunk(header->RIFFChunk);
		printStandardFmtChunk(header->fmtChunk);
		printDataChunk(header->DataChunk);
	}

	void printNonPCMWavHeader(NonPCMWavHeader* header)
	{
		printRIFFChunk(header->RIFFChunk);
		printNonPCMFmtChunk(header->fmtChunk);
		printfactChunk(header->factChunk);
		printDataChunk(header->DataChunk);
	}

	void printExtensibleWavHeader(ExtensibleWavHeader* header)
	{
		printRIFFChunk(header->RIFFChunk);
		printExtensibleFmtChunk(header->fmtChunk);
		printfactChunk(header->factChunk);
		printDataChunk(header->DataChunk);
	}

	void printBWFWavHeader(BWFWavHeader* header)
	{
		printRIFFChunk(header->RIFFChunk);
		printExtensibleFmtChunk(header->fmtChunk);
		printbextChunk(header->bextChunk);
		printfactChunk(header->factChunk);
		printDataChunk(header->DataChunk);
	}

	StandardWavHeader WaveParser::GenerateStandardWavHeader(unsigned short nChannels, unsigned long nSamplesPerSec, unsigned short wBitsPerSample, unsigned long dataChuckSize)
	{
		StandardWavHeader l_header;
		l_header.type = WavHeaderType::Standard;

		std::memcpy(l_header.RIFFChunk.ckID, "RIFF", 4);
		l_header.RIFFChunk.cksize = 44 + dataChuckSize;
		std::memcpy(l_header.RIFFChunk.WAVEID, "WAVE", 4);

		std::memcpy(l_header.fmtChunk.ckID, "fmt ", 4);
		l_header.fmtChunk.cksize = 16;
		l_header.fmtChunk.wFormatTag = 1;
		l_header.fmtChunk.nChannels = nChannels;
		l_header.fmtChunk.nSamplesPerSec = nSamplesPerSec;
		l_header.fmtChunk.nAvgBytesPerSec = nSamplesPerSec * nChannels * wBitsPerSample / 8;
		l_header.fmtChunk.nBlockAlign = nChannels * wBitsPerSample / 8;
		l_header.fmtChunk.wBitsPerSample = wBitsPerSample;

		std::memcpy(l_header.DataChunk.ckID, "data", 4);
		l_header.DataChunk.cksize = dataChuckSize;

		return l_header;
	}

	void WaveParser::PrintWavHeader(IWavHeader* header)
	{
		switch (header->type)
		{
		case WavHeaderType::Standard: printStandardWavHeader(reinterpret_cast<StandardWavHeader*>(header)); break;
		case WavHeaderType::NonPCM: printNonPCMWavHeader(reinterpret_cast<NonPCMWavHeader*>(header)); break;
		case WavHeaderType::Extensible: printExtensibleWavHeader(reinterpret_cast<ExtensibleWavHeader*>(header)); break;
		case WavHeaderType::BWF: printBWFWavHeader(reinterpret_cast<BWFWavHeader*>(header)); break;
		default:
			break;
		}
	}

	WavObject WaveParser::LoadFile(const std::string & path)
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

		WavObject l_result;
		size_t l_wavHeaderSize = 0;
		size_t l_sampleCount = 0;

		if (l_chuckSize == 16)
		{
			printLog(path + " is Standard Wave format");

			// vptr
			l_wavHeaderSize = sizeof(StandardWavHeader) - 4;
			l_sampleCount = l_size - l_wavHeaderSize;
			l_result.header = new StandardWavHeader();
			l_result.header->type = WavHeaderType::Standard;
			pbuf->pubseekpos(0, l_file.in);
			pbuf->sgetn((char*)l_result.header + 4, l_wavHeaderSize);
		}
		else if (l_chuckSize == 18)
		{
			printLog(path + " is Non-PCM Wave format");

			// vptr
			l_wavHeaderSize = sizeof(NonPCMWavHeader) - 4;
			l_sampleCount = l_size - l_wavHeaderSize;
			l_result.header = new NonPCMWavHeader();
			l_result.header->type = WavHeaderType::NonPCM;
			pbuf->pubseekpos(0, l_file.in);
			pbuf->sgetn((char*)l_result.header + 4, l_wavHeaderSize);
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
				l_wavHeaderSize = 0;
				l_wavHeaderSize += sizeof(RIFFChunk);
				l_wavHeaderSize += sizeof(ExtensibleFmtChunk);
				l_wavHeaderSize += sizeof(bextChunk);

				// load temp header
				auto l_BWFHeader = new BWFWavHeader();
				pbuf->pubseekpos(0, l_file.in);
				pbuf->sgetn((char*)l_BWFHeader + 4, l_wavHeaderSize);

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

				l_result.header = l_BWFHeader;
				auto l_samplePos = l_factChunkPos + sizeof(factChunk) + sizeof(DataChunk) + 1;
				pbuf->pubseekpos(l_samplePos, l_file.in);
				l_result.header->type = WavHeaderType::BWF;
			}
			else
			{
				printLog(path + " is Extensible Wave format");

				// vptr
				l_wavHeaderSize = sizeof(ExtensibleWavHeader) - 4;
				l_sampleCount = l_size - l_wavHeaderSize;
				l_result.header = new ExtensibleWavHeader();
				l_result.header->type = WavHeaderType::Extensible;
				pbuf->pubseekpos(0, l_file.in);
				pbuf->sgetn((char*)l_result.header + 4, l_wavHeaderSize);
			}
		}

		// load sample
		auto l_sample = std::vector<short>(l_sampleCount);
		pbuf->sgetn((char*)&l_sample[0], l_sampleCount);
		l_result.sample = std::move(l_sample);

		PrintWavHeader(l_result.header);

		return l_result;
	}

	bool WaveParser::WriteFile(const std::string & path, IWavHeader * header, const ComplexArray & x)
	{
		WavObject l_wavObject;
		l_wavObject.header = header;

		std::vector<short> l_sample;
		l_sample.reserve(x.size());

		for (size_t i = 0; i < x.size(); i++)
		{
			l_sample.emplace_back((short)x[i].real());
		}

		l_wavObject.sample = std::move(l_sample);

		WriteFile(path, l_wavObject);

		return false;
	}

	bool WaveParser::WriteFile(const std::string & path, const WavObject& wavObject)
	{
		size_t l_sizeOfWavHeader = 0;

		switch (wavObject.header->type)
		{
		case WavHeaderType::Standard: l_sizeOfWavHeader = sizeof(StandardWavHeader); break;
		case WavHeaderType::NonPCM:  l_sizeOfWavHeader = sizeof(NonPCMWavHeader); break;
		case WavHeaderType::Extensible: l_sizeOfWavHeader = sizeof(ExtensibleWavHeader); break;
		case WavHeaderType::BWF: l_sizeOfWavHeader = sizeof(BWFWavHeader); break;
		default:
			break;
		}

		std::ofstream l_file(path, std::ios::binary);

		l_file.write((char*)wavObject.header + 4, l_sizeOfWavHeader - 4);
		l_file.write((char*)&wavObject.sample[0], wavObject.sample.size());

		l_file.close();
		return true;
	}

	inline void endian_swap(unsigned short& x)
	{
		x = (x >> 8) | (x << 8);
	}
}