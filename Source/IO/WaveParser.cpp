#include "WaveParser.h"
#include "../Core/Logger.h"

namespace Waveless
{
	void printRIFFChunk(RIFFChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "RIFF: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.cksize);
		Logger::Log(LogLevel::Verbose, "WAVE: ", rhs.WAVEID);
	}

	void printStandardFmtChunk(StandardFmtChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "Fmt: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "FmtSize: ", (uint32_t)rhs.cksize);
		Logger::Log(LogLevel::Verbose, "AudioFormat: ", (uint16_t)rhs.wFormatTag);
		Logger::Log(LogLevel::Verbose, "NumOfChan: ", (uint16_t)rhs.nChannels);
		Logger::Log(LogLevel::Verbose, "SamplesPerSec: ", (uint32_t)rhs.nSamplesPerSec);
		Logger::Log(LogLevel::Verbose, "bytesPerSec: ", (uint32_t)rhs.nAvgBytesPerSec);
		Logger::Log(LogLevel::Verbose, "blockAlign: ", (uint16_t)rhs.nBlockAlign);
		Logger::Log(LogLevel::Verbose, "bitsPerSample: ", (uint16_t)rhs.wBitsPerSample);
	}

	void printNonPCMFmtChunk(NonPCMFmtChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "Fmt: ", rhs.standardFmtChunk.ckID);
		Logger::Log(LogLevel::Verbose, "FmtSize: ", (uint32_t)rhs.standardFmtChunk.cksize);
		Logger::Log(LogLevel::Verbose, "AudioFormat: ", (uint16_t)rhs.standardFmtChunk.wFormatTag);
		Logger::Log(LogLevel::Verbose, "NumOfChan: ", (uint16_t)rhs.standardFmtChunk.nChannels);
		Logger::Log(LogLevel::Verbose, "SamplesPerSec: ", (uint32_t)rhs.standardFmtChunk.nSamplesPerSec);
		Logger::Log(LogLevel::Verbose, "bytesPerSec: ", (uint32_t)rhs.standardFmtChunk.nAvgBytesPerSec);
		Logger::Log(LogLevel::Verbose, "blockAlign: ", (uint16_t)rhs.standardFmtChunk.nBlockAlign);
		Logger::Log(LogLevel::Verbose, "bitsPerSample: ", (uint16_t)rhs.standardFmtChunk.wBitsPerSample);
		Logger::Log(LogLevel::Verbose, "ExtensionSize: ", (uint16_t)rhs.cbsize);
	}

	void printExtensibleFmtChunk(ExtensibleFmtChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "Fmt: ", rhs.standardFmtChunk.ckID);
		Logger::Log(LogLevel::Verbose, "FmtSize: ", (uint32_t)rhs.standardFmtChunk.cksize);
		Logger::Log(LogLevel::Verbose, "AudioFormat: ", (uint16_t)rhs.standardFmtChunk.wFormatTag);
		Logger::Log(LogLevel::Verbose, "NumOfChan: ", (uint16_t)rhs.standardFmtChunk.nChannels);
		Logger::Log(LogLevel::Verbose, "SamplesPerSec: ", (uint32_t)rhs.standardFmtChunk.nSamplesPerSec);
		Logger::Log(LogLevel::Verbose, "bytesPerSec: ", (uint32_t)rhs.standardFmtChunk.nAvgBytesPerSec);
		Logger::Log(LogLevel::Verbose, "blockAlign: ", (uint16_t)rhs.standardFmtChunk.nBlockAlign);
		Logger::Log(LogLevel::Verbose, "bitsPerSample: ", (uint16_t)rhs.standardFmtChunk.wBitsPerSample);
		Logger::Log(LogLevel::Verbose, "ExtensionSize: ", (uint16_t)rhs.cbsize);
		Logger::Log(LogLevel::Verbose, "ValidBitsPerSample: ", (uint16_t)rhs.wValidBitsPerSample);
		Logger::Log(LogLevel::Verbose, "ChannelMask: ", (uint32_t)rhs.dwChannelMask);
		Logger::Log(LogLevel::Verbose, "SubFormat: ", rhs.SubFormat);
	}

	void printbextChunk(bextChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "Bext: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "BextSize: ", (uint32_t)rhs.cksize);
		Logger::Log(LogLevel::Verbose, "Description: ", rhs.Description);
		Logger::Log(LogLevel::Verbose, "Originator: ", rhs.Originator);
		Logger::Log(LogLevel::Verbose, "OriginatorReference: ", rhs.OriginatorReference);
		Logger::Log(LogLevel::Verbose, "OriginationDate: ", rhs.OriginationDate);
		Logger::Log(LogLevel::Verbose, "OriginationTime: ", rhs.OriginationTime);
		Logger::Log(LogLevel::Verbose, "TimeReferenceLow: ", (uint32_t)rhs.TimeReferenceLow);
		Logger::Log(LogLevel::Verbose, "TimeReferenceHigh: ", (uint32_t)rhs.TimeReferenceHigh);
		Logger::Log(LogLevel::Verbose, "Version: ", (uint16_t)rhs.Version);
		Logger::Log(LogLevel::Verbose, "UMID: ", rhs.UMID);
		Logger::Log(LogLevel::Verbose, "LoudnessValue: ", (uint16_t)rhs.LoudnessValue);
		Logger::Log(LogLevel::Verbose, "LoudnessRange: ", (uint16_t)rhs.LoudnessRange);
		Logger::Log(LogLevel::Verbose, "MaxTruePeakLevel: ", (uint16_t)rhs.MaxTruePeakLevel);
		Logger::Log(LogLevel::Verbose, "MaxMomentaryLoudness: ", (uint16_t)rhs.MaxMomentaryLoudness);
		Logger::Log(LogLevel::Verbose, "MaxShortTermLoudness: ", (uint16_t)rhs.MaxShortTermLoudness);
	}

	void printfactChunk(factChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "Data: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "DataSize: ", (uint32_t)rhs.cksize);
		Logger::Log(LogLevel::Verbose, "SampleLength: ", (uint32_t)rhs.dwSampleLength);
	}

	void printDataChunk(DataChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "Data: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "DataSize: ", (uint32_t)rhs.cksize);
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

	WavObject WaveParser::LoadFile(const char* path)
	{
		std::ifstream l_file(path, std::ios::binary);

		if (!l_file.is_open())
		{
			Logger::Log(LogLevel::Error, "std::ifstream: can't open file ", path, "!");
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
			Logger::Log(LogLevel::Verbose, path, " is Standard Wave format");

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
			Logger::Log(LogLevel::Verbose, path, " is Non-PCM Wave format");

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
				Logger::Log(LogLevel::Verbose, path, " is Broadcast Wave format");

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
				Logger::Log(LogLevel::Verbose, path, " is Extensible Wave format");

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

	bool WaveParser::WriteFile(const char* path, IWavHeader * header, const ComplexArray & x)
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

	bool WaveParser::WriteFile(const char* path, const WavObject& wavObject)
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