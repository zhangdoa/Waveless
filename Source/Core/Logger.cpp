#include "Logger.h"
#include "Timer.h"
#include "Config.h"
#include <string>
#include <future>

namespace Waveless::LoggerNS
{
	inline std::ostream& GetTimestamp(std::ostream &s)
	{
		auto l_timeData = Timer::GetCurrentTime(8);
		s
			<< "["
			<< l_timeData.Year
			<< "-"
			<< l_timeData.Month
			<< "-"
			<< l_timeData.Day
			<< "-"
			<< l_timeData.Hour
			<< "-"
			<< l_timeData.Minute
			<< "-"
			<< l_timeData.Second
			<< "-"
			<< l_timeData.Millisecond
			<< "]";
		return s;
	}

#if defined WS_OS_WIN
#include <windows.h>
	inline std::ostream& redColor(std::ostream &s)
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_RED | FOREGROUND_INTENSITY);
		return s;
	}
	inline std::ostream& greenColor(std::ostream &s)
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		return s;
	}
	inline std::ostream& blueColor(std::ostream &s)
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		return s;
	}
	inline std::ostream& yellowColor(std::ostream &s)
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
		return s;
	}
	inline std::ostream& whiteColor(std::ostream &s)
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return s;
	}
#endif

	std::ofstream m_LogFile;
	std::mutex m_Mutex;
	LogLevel m_LogLevel;
}

void Waveless::Logger::SetDefaultLogLevel(LogLevel logLevel)
{
	LoggerNS::m_LogLevel = logLevel;
}

Waveless::LogLevel Waveless::Logger::GetDefaultLogLevel()
{
	return LoggerNS::m_LogLevel;
}

void Waveless::Logger::LogStartOfLine(LogLevel logLevel)
{
	LoggerNS::m_Mutex.lock();

#if defined WS_OS_WIN
	switch (logLevel)
	{
	case LogLevel::Verbose:
		std::cout << LoggerNS::blueColor; break;
	case LogLevel::Warning:
		std::cout << LoggerNS::yellowColor; break;
	case LogLevel::Error:
		std::cout << LoggerNS::redColor; break;
	case LogLevel::Success:
		std::cout << LoggerNS::greenColor; break;
	default: std::cout << LoggerNS::whiteColor; break;
	}
#endif
	std::cout << LoggerNS::GetTimestamp;
	LoggerNS::m_LogFile << LoggerNS::GetTimestamp;
}

void Waveless::Logger::LogEndOfLine()
{
	std::cout << std::endl;
	LoggerNS::m_LogFile << std::endl;
	LoggerNS::m_Mutex.unlock();
}

void Waveless::Logger::LogImpl(const void * logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(bool logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(uint8_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(uint16_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(uint32_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(uint64_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(int8_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(int16_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(int32_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(int64_t logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(float logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(double logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

void Waveless::Logger::LogImpl(const char* logMessage)
{
	std::cout << logMessage;
	LoggerNS::m_LogFile << logMessage;
}

bool Waveless::Logger::Initialize()
{
	std::stringstream ss;
	ss << LoggerNS::GetTimestamp << ".Log";
	LoggerNS::m_LogFile.open(ss.str(), std::ios::out | std::ios::trunc);

	if (LoggerNS::m_LogFile.is_open())
	{
		return true;
	}
	else
	{
		Log(LogLevel::Error, "Logger: Can't open log file!");
		return false;
	}
}

bool Waveless::Logger::Terminate()
{
	Log(LogLevel::Success, "Logger: Terminated.");
	LoggerNS::m_LogFile.close();
	return true;
}