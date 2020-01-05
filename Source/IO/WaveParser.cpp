#include "WaveParser.h"
#include "../Core/Logger.h"

namespace Waveless
{
	void printRIFFChunk(RIFFChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
		Logger::Log(LogLevel::Verbose, "RIFFType: ", rhs.RIFFType);
	}

	void printFmtChunk(fmtChunk rhs, WavHeaderType type)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
		Logger::Log(LogLevel::Verbose, "AudioFormat: ", (uint16_t)rhs.wFormatTag);
		Logger::Log(LogLevel::Verbose, "NumOfChan: ", (uint16_t)rhs.nChannels);
		Logger::Log(LogLevel::Verbose, "SamplesPerSec: ", (uint32_t)rhs.nSamplesPerSec);
		Logger::Log(LogLevel::Verbose, "bytesPerSec: ", (uint32_t)rhs.nAvgBytesPerSec);
		Logger::Log(LogLevel::Verbose, "blockAlign: ", (uint16_t)rhs.nBlockAlign);
		Logger::Log(LogLevel::Verbose, "bitsPerSample: ", (uint16_t)rhs.wBitsPerSample);

		if (type == WavHeaderType::NonPCM || type == WavHeaderType::Extensible)
		{
			Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint16_t)rhs.cbSize);

			if (type == WavHeaderType::Extensible)
			{
				Logger::Log(LogLevel::Verbose, "ValidBitsPerSample: ", (uint16_t)rhs.wValidBitsPerSample);
				Logger::Log(LogLevel::Verbose, "ChannelMask: ", (uint32_t)rhs.dwChannelMask);
				Logger::Log(LogLevel::Verbose, "SubFormat: ", rhs.SubFormat);
			}
		}
	}

	void printBextChunk(bextChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
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

	void printFactChunk(factChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
		Logger::Log(LogLevel::Verbose, "SampleLength: ", (uint32_t)rhs.dwSampleLength);
	}

	void printDataChunk(dataChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
	}

	void WaveParser::PrintWavHeader(WavHeader* header)
	{
		printRIFFChunk(header->RIFFChunk);
		printFmtChunk(header->fmtChunk, header->type);
		if (header->type == WavHeaderType::BWF)
		{
			printBextChunk(header->bextChunk);
		}
		if (header->type != WavHeaderType::Standard)
		{
			printFactChunk(header->factChunk);
		}
		printDataChunk(header->dataChunk);
	}

	void GetData(std::filebuf* pbuf, void* rhs, size_t size)
	{
		pbuf->sgetn(reinterpret_cast<char*>(rhs), size);
		pbuf->pubseekoff(-size, std::ios_base::cur, std::ios_base::in | std::ios::binary);
	}

	bool CheckChunkID(std::filebuf* pbuf, const char* ID)
	{
		char l_ChunkID[4];

		GetData(pbuf, &l_ChunkID[0], sizeof(l_ChunkID));

		if (strncmp(l_ChunkID, ID, strlen(ID)))
		{
			Logger::Log(LogLevel::Error, "Not ", ID, "!");
			return true;
		}

		return false;
	}

	WavObject WaveParser::LoadFile(const char* path)
	{
		std::ifstream l_file(path, std::ios::binary);

		if (!l_file.is_open())
		{
			Logger::Log(LogLevel::Error, "std::ifstream: can't open file ", path, "!");
		}

		auto pbuf = l_file.rdbuf();

		std::size_t l_size = pbuf->pubseekoff(0, l_file.end, l_file.in);

		// Check file format
		pbuf->pubseekpos(0, l_file.in);
		CheckChunkID(pbuf, "RIFF");

		pbuf->pubseekpos(8, l_file.in);
		CheckChunkID(pbuf, "WAVE");

		WavHeader l_WavHeader;

		// RIFF
		pbuf->pubseekpos(0, l_file.in);
		GetData(pbuf, &l_WavHeader.RIFFChunk, sizeof(l_WavHeader.RIFFChunk));

		// JUNK without real junk data
		pbuf->pubseekoff(sizeof(l_WavHeader.RIFFChunk), std::ios_base::cur, l_file.in);
		if (!CheckChunkID(pbuf, "JUNK"))
		{
			GetData(pbuf, &l_WavHeader.JunkChunk, sizeof(l_WavHeader.JunkChunk));
			pbuf->pubseekoff(l_WavHeader.JunkChunk.chunkSize + 8, std::ios_base::cur, l_file.in);
		}

		// fmt with only effective chunk length
		if (!CheckChunkID(pbuf, "fmt"))
		{
			pbuf->pubseekoff(4, std::ios_base::cur, l_file.in);
			unsigned long l_fmtChuckSize;
			GetData(pbuf, &l_fmtChuckSize, sizeof(l_fmtChuckSize));

			if (l_fmtChuckSize == 16)
			{
				Logger::Log(LogLevel::Verbose, path, " is Standard Wave format");
				l_WavHeader.type = WavHeaderType::Standard;
			}
			else if (l_fmtChuckSize == 18)
			{
				Logger::Log(LogLevel::Verbose, path, " is Non-PCM Wave format");
				l_WavHeader.type = WavHeaderType::NonPCM;
			}
			else if (l_fmtChuckSize == 40)
			{
				Logger::Log(LogLevel::Verbose, path, " is Extensible Wave format");
				l_WavHeader.type = WavHeaderType::Extensible;
			}

			pbuf->pubseekoff(-4, std::ios_base::cur, l_file.in);
			GetData(pbuf, &l_WavHeader.fmtChunk, l_fmtChuckSize + 8);

			pbuf->pubseekoff(l_fmtChuckSize + 8, std::ios_base::cur, l_file.in);
		}

		// fact
		if (!CheckChunkID(pbuf, "fact"))
		{
			GetData(pbuf, &l_WavHeader.factChunk, sizeof(l_WavHeader.factChunk));
			pbuf->pubseekoff(sizeof(l_WavHeader.factChunk), std::ios_base::cur, l_file.in);
		}

		// bext
		if (!CheckChunkID(pbuf, "bext"))
		{
			Logger::Log(LogLevel::Verbose, path, " isBroadcast Wave Format");
			l_WavHeader.type = WavHeaderType::BWF;

			GetData(pbuf, &l_WavHeader.bextChunk, sizeof(l_WavHeader.bextChunk));
			pbuf->pubseekoff(sizeof(l_WavHeader.bextChunk), std::ios_base::cur, l_file.in);

			// load code history
			auto l_bextSize = l_WavHeader.bextChunk.ckSize;
			auto l_codeHistorySectorLength = l_bextSize + 8 - sizeof(bextChunk);

			if (l_codeHistorySectorLength)
			{
				std::vector<char> l_codeHistory(l_codeHistorySectorLength);
				pbuf->sgetn((char*)&l_codeHistory[0], l_codeHistorySectorLength);
				Logger::Log(LogLevel::Verbose, "Code History: ", &l_codeHistory[0]);
			}
		}

		if (!CheckChunkID(pbuf, "data"))
		{
			GetData(pbuf, &l_WavHeader.dataChunk, sizeof(l_WavHeader.dataChunk));
			pbuf->pubseekoff(sizeof(l_WavHeader.dataChunk), std::ios_base::cur, l_file.in);
		}

		WavObject l_result;
		l_result.header = l_WavHeader;

		// load sample
		auto l_sample = std::vector<char>(l_WavHeader.dataChunk.ckSize);
		GetData(pbuf, &l_sample[0], l_sample.size());

		l_result.sample = std::move(l_sample);

		PrintWavHeader(&l_result.header);

		return l_result;
	}

	WavHeader WaveParser::GenerateWavHeader(unsigned short channels, unsigned long sampleRate, unsigned short bitDepth, unsigned long sampleCount)
	{
		auto l_bytesPerSample = bitDepth / 8;

		WavHeader l_header;
		l_header.type = WavHeaderType::Standard;

		std::memcpy(l_header.RIFFChunk.ckID, "RIFF", 4);
		l_header.RIFFChunk.ckSize = 44 + sampleCount * l_bytesPerSample;
		std::memcpy(l_header.RIFFChunk.RIFFType, "WAVE", 4);

		std::memcpy(l_header.fmtChunk.ckID, "fmt ", 4);
		l_header.fmtChunk.ckSize = 16;
		l_header.fmtChunk.wFormatTag = 1;
		l_header.fmtChunk.nChannels = channels;
		l_header.fmtChunk.nSamplesPerSec = sampleRate * channels;
		l_header.fmtChunk.nAvgBytesPerSec = sampleRate * channels * l_bytesPerSample;
		l_header.fmtChunk.nBlockAlign = channels * l_bytesPerSample;
		l_header.fmtChunk.wBitsPerSample = bitDepth;

		std::memcpy(l_header.dataChunk.ckID, "data", 4);
		l_header.dataChunk.ckSize = sampleCount * l_bytesPerSample;

		return l_header;
	}

	WavObject WaveParser::GenerateWavObject(const WavHeader& header, const ComplexArray& x)
	{
		std::vector<char> l_samples;
		auto l_bytesPerSample = header.fmtChunk.wBitsPerSample / 8;

		l_samples.resize(x.size() * l_bytesPerSample);

		for (size_t i = 0; i < x.size(); i++)
		{
			auto l_sampleOrig = x[i].real();

			int32_t l_sample = (int32_t)l_sampleOrig;
			std::memcpy(&l_samples[i * l_bytesPerSample], &l_sample, l_bytesPerSample);
		}

		WavObject l_result;
		l_result.header = header;
		l_result.sample = std::move(l_samples);

		return l_result;
	}

	bool WaveParser::WriteFile(const char* path, const WavObject& wavObject)
	{
		std::ofstream l_file(path, std::ios::binary);

		if (!l_file.is_open())
		{
			Logger::Log(LogLevel::Error, "std::ifstream: can't open file ", path, "!");
			return false;
		}

		// RIFF
		l_file.write((char*)&wavObject.header.RIFFChunk, sizeof(wavObject.header.RIFFChunk));

		// fmt
		size_t l_fmtChunkSize = 0;

		switch (wavObject.header.type)
		{
		case WavHeaderType::Standard: l_fmtChunkSize = 16; break;
		case WavHeaderType::NonPCM:  l_fmtChunkSize = 18; break;
		case WavHeaderType::Extensible: l_fmtChunkSize = 40; break;
		case WavHeaderType::BWF: l_fmtChunkSize = 40; break;
		default:
			break;
		}

		l_file.write((char*)&wavObject.header.fmtChunk, l_fmtChunkSize + 8);

		// fact
		if (wavObject.header.type == WavHeaderType::NonPCM || wavObject.header.type == WavHeaderType::Extensible)
		{
			l_file.write((char*)&wavObject.header.factChunk, sizeof(wavObject.header.factChunk));
		}

		// bext
		if (wavObject.header.type == WavHeaderType::BWF)
		{
			l_file.write((char*)&wavObject.header.bextChunk, sizeof(wavObject.header.bextChunk));
		}

		// data
		l_file.write((char*)&wavObject.header.dataChunk, sizeof(wavObject.header.dataChunk));

		l_file.write((char*)&wavObject.sample[0], wavObject.sample.size());

		l_file.close();

		return true;
	}

	bool WaveParser::WriteFile(const char* path, const WavHeader & header, const ComplexArray & x)
	{
		return WriteFile(path, GenerateWavObject(header, x));
	}

	inline void endian_swap(unsigned short& x)
	{
		x = (x >> 8) | (x << 8);
	}
}