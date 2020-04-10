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

	void printJunkChunk(JunkChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
	}

	void printFmtChunk(fmtChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
		Logger::Log(LogLevel::Verbose, "AudioFormat: ", (uint16_t)rhs.wFormatTag);
		Logger::Log(LogLevel::Verbose, "NumOfChan: ", (uint16_t)rhs.nChannels);
		Logger::Log(LogLevel::Verbose, "SamplesPerSec: ", (uint32_t)rhs.nSamplesPerSec);
		Logger::Log(LogLevel::Verbose, "bytesPerSec: ", (uint32_t)rhs.nAvgBytesPerSec);
		Logger::Log(LogLevel::Verbose, "blockAlign: ", (uint16_t)rhs.nBlockAlign);
		Logger::Log(LogLevel::Verbose, "bitsPerSample: ", (uint16_t)rhs.wBitsPerSample);

		if (rhs.ckSize > 16)
		{
			Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint16_t)rhs.cbSize);

			if (rhs.ckSize > 18)
			{
				Logger::Log(LogLevel::Verbose, "ValidBitsPerSample: ", (uint16_t)rhs.wValidBitsPerSample);
				Logger::Log(LogLevel::Verbose, "ChannelMask: ", (uint32_t)rhs.dwChannelMask);
				Logger::Log(LogLevel::Verbose, "SubFormat: ", rhs.SubFormat);
			}
		}
	}
	void printFactChunk(factChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
		Logger::Log(LogLevel::Verbose, "SampleLength: ", (uint32_t)rhs.dwSampleLength);
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

	void printDataChunk(dataChunk rhs)
	{
		Logger::Log(LogLevel::Verbose, "ChunkID: ", rhs.ckID);
		Logger::Log(LogLevel::Verbose, "ChunkSize: ", (uint32_t)rhs.ckSize);
	}

	void WaveParser::PrintWavHeader(WavHeader* header)
	{
		if (header->ChunkValidities[0])
		{
			printRIFFChunk(header->RIFFChunk);
		}
		if (header->ChunkValidities[1])
		{
			printJunkChunk(header->JunkChunk);
		}
		if (header->ChunkValidities[2])
		{
			printFmtChunk(header->fmtChunk);
		}
		if (header->ChunkValidities[3])
		{
			printFactChunk(header->factChunk);
		}
		if (header->ChunkValidities[4])
		{
			printBextChunk(header->bextChunk);
		}
		if (header->ChunkValidities[5])
		{
			printDataChunk(header->dataChunk);
		}
	}

	void GetData(std::filebuf* pbuf, void* rhs, size_t size)
	{
		pbuf->sgetn(reinterpret_cast<char*>(rhs), size);
		pbuf->pubseekoff(-(int32_t)size, std::ios_base::cur, std::ios_base::in | std::ios::binary);
	}

	bool CheckChunkID(std::filebuf* pbuf, const char* ID)
	{
		char l_ChunkID[4];

		GetData(pbuf, &l_ChunkID[0], sizeof(l_ChunkID));

		if (strncmp(l_ChunkID, ID, strlen(ID)))
		{
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
		pbuf->pubseekoff(sizeof(l_WavHeader.RIFFChunk), std::ios_base::cur, l_file.in);
		l_WavHeader.ChunkValidities[0] = 1;

		bool l_getDataChunk = false;

		while (!l_getDataChunk)
		{
			// JUNK without real junk data
			if (!CheckChunkID(pbuf, "JUNK") || !CheckChunkID(pbuf, "junk"))
			{
				GetData(pbuf, &l_WavHeader.JunkChunk, sizeof(l_WavHeader.JunkChunk));
				pbuf->pubseekoff(l_WavHeader.JunkChunk.ckSize + 8, std::ios_base::cur, l_file.in);
				l_WavHeader.ChunkValidities[1] = 1;
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
				}
				else if (l_fmtChuckSize == 18)
				{
					Logger::Log(LogLevel::Verbose, path, " is Non-PCM Wave format");
				}
				else if (l_fmtChuckSize == 40)
				{
					Logger::Log(LogLevel::Verbose, path, " is Extensible Wave format");
				}

				pbuf->pubseekoff(-4, std::ios_base::cur, l_file.in);
				GetData(pbuf, &l_WavHeader.fmtChunk, l_fmtChuckSize + 8);

				pbuf->pubseekoff(l_fmtChuckSize + 8, std::ios_base::cur, l_file.in);
				l_WavHeader.ChunkValidities[2] = 1;
			}

			// fact
			if (!CheckChunkID(pbuf, "fact"))
			{
				GetData(pbuf, &l_WavHeader.factChunk, sizeof(l_WavHeader.factChunk));
				pbuf->pubseekoff(sizeof(l_WavHeader.factChunk), std::ios_base::cur, l_file.in);
				l_WavHeader.ChunkValidities[3] = 1;
			}

			// bext
			if (!CheckChunkID(pbuf, "bext"))
			{
				Logger::Log(LogLevel::Verbose, path, " is Broadcast Wave Format");

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
				l_WavHeader.ChunkValidities[4] = 1;
			}

			if (!CheckChunkID(pbuf, "data"))
			{
				GetData(pbuf, &l_WavHeader.dataChunk, sizeof(l_WavHeader.dataChunk));
				pbuf->pubseekoff(sizeof(l_WavHeader.dataChunk), std::ios_base::cur, l_file.in);
				l_WavHeader.ChunkValidities[5] = 1;

				l_getDataChunk = true;
			}
		}

		WavObject l_result;
		l_result.header = l_WavHeader;

		// load sample
		l_result.count = l_WavHeader.dataChunk.ckSize;
		// @TODO: Memory pool for samples
		auto l_samples = new char[l_result.count];
		GetData(pbuf, &l_samples[0], l_result.count);

		l_result.samples = l_samples;

		PrintWavHeader(&l_result.header);

		return l_result;
	}

	WavHeader WaveParser::GenerateWavHeader(unsigned short channels, unsigned long sampleRate, unsigned short bitDepth, unsigned long sampleCount)
	{
		auto l_bytesPerSample = bitDepth / 8;

		WavHeader l_header;
		std::memcpy(l_header.RIFFChunk.ckID, "RIFF", 4);
		l_header.RIFFChunk.ckSize = 44 + sampleCount * l_bytesPerSample;
		std::memcpy(l_header.RIFFChunk.RIFFType, "WAVE", 4);
		l_header.ChunkValidities[0] = 1;

		std::memcpy(l_header.fmtChunk.ckID, "fmt ", 4);
		l_header.fmtChunk.ckSize = 16;
		l_header.fmtChunk.wFormatTag = 1;
		l_header.fmtChunk.nChannels = channels;
		l_header.fmtChunk.nSamplesPerSec = sampleRate * channels;
		l_header.fmtChunk.nAvgBytesPerSec = sampleRate * channels * l_bytesPerSample;
		l_header.fmtChunk.nBlockAlign = channels * l_bytesPerSample;
		l_header.fmtChunk.wBitsPerSample = bitDepth;
		l_header.ChunkValidities[2] = 1;

		std::memcpy(l_header.dataChunk.ckID, "data", 4);
		l_header.dataChunk.ckSize = sampleCount * l_bytesPerSample;
		l_header.ChunkValidities[5] = 1;

		return l_header;
	}

	WavObject WaveParser::GenerateWavObject(const WavHeader& header, const ComplexArray& x)
	{
		WavObject l_result;

		auto l_bytesPerSample = header.fmtChunk.wBitsPerSample / 8;
		l_result.count = (int)x.size() * l_bytesPerSample;
		auto l_samples = new char[l_result.count];

		for (size_t i = 0; i < x.size(); i++)
		{
			auto l_sampleOrig = x[i].real();

			int32_t l_sample = (int32_t)l_sampleOrig;
			std::memcpy(&l_samples[i * l_bytesPerSample], &l_sample, l_bytesPerSample);
		}

		l_result.header = header;
		l_result.samples = l_samples;

		return l_result;
	}

	ComplexArray WaveParser::GenerateComplexArray(const WavObject& wavObject)
	{
		auto l_bytesPerSample = wavObject.header.fmtChunk.wBitsPerSample / 8;

		ComplexArray l_result;
		l_result.resize(wavObject.count / l_bytesPerSample);

		for (size_t i = 0; i < l_result.size(); i++)
		{
			auto l_sampleOrig = &wavObject.samples[i * l_bytesPerSample];
			Complex l_sample;

			if (l_bytesPerSample == 1)
			{
				auto l_sampleTemp = (int8_t*)l_sampleOrig;
			}
			else if (l_bytesPerSample == 2)
			{
				auto l_sampleTemp = (int16_t*)l_sampleOrig;
				l_sample = *l_sampleTemp;
			}
			else if (l_bytesPerSample == 4)
			{
				auto l_sampleTemp = (int32_t*)l_sampleOrig;
				l_sample = *l_sampleTemp;
			}

			l_result[i] = l_sample;
		}

		return l_result;
	}

	Waveless::WsResult WaveParser::WriteFile(const char* path, const WavObject& wavObject)
	{
		std::ofstream l_file(path, std::ios::out | std::ios::ate | std::ios::binary);

		if (!l_file.is_open())
		{
			Logger::Log(LogLevel::Error, "std::ofstream: can't open file ", path, "!");
			return WsResult::FileNotFound;
		}
		if (wavObject.header.ChunkValidities[0])
		{
			l_file.write((char*)&wavObject.header.RIFFChunk, sizeof(wavObject.header.RIFFChunk));
		}
		if (wavObject.header.ChunkValidities[2])
		{
			l_file.write((char*)&wavObject.header.fmtChunk, wavObject.header.fmtChunk.ckSize + 8);
		}
		if (wavObject.header.ChunkValidities[3])
		{
			l_file.write((char*)&wavObject.header.factChunk, sizeof(wavObject.header.factChunk));
		}
		if (wavObject.header.ChunkValidities[4])
		{
			l_file.write((char*)&wavObject.header.bextChunk, sizeof(wavObject.header.bextChunk));
		}
		if (wavObject.header.ChunkValidities[1])
		{
			l_file.write((char*)&wavObject.header.JunkChunk, sizeof(wavObject.header.JunkChunk));
			std::fill_n(std::ostream_iterator<char>(l_file), wavObject.header.JunkChunk.ckSize, '\0');
		}
		if (wavObject.header.ChunkValidities[5])
		{
			l_file.write((char*)&wavObject.header.dataChunk, sizeof(wavObject.header.dataChunk));
		}

		l_file.write((char*)&wavObject.samples[0], wavObject.count);

		l_file.close();

		return WsResult::Success;
	}

	Waveless::WsResult WaveParser::WriteFile(const char* path, const WavHeader & header, const ComplexArray & x)
	{
		return WriteFile(path, GenerateWavObject(header, x));
	}

	inline void endian_swap(unsigned short& x)
	{
		x = (x >> 8) | (x << 8);
	}
}