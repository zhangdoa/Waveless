#include "IOService.h"
#include "../Core/Logger.h"

#ifdef __GNUC__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace Waveless::IOService
{
	std::string m_workingDir;
}

Waveless::WsResult Waveless::IOService::loadFile(const char* filePath, std::vector<char>& content, IOMode openMode)
{
	std::ios_base::openmode l_mode = std::ios::in;
	switch (openMode)
	{
	case IOMode::Text:
		l_mode = std::ios::in;
		break;
	case IOMode::Binary:
		l_mode = std::ios::in | std::ios::ate | std::ios::binary;
		break;
	default:
		break;
	}

	std::ifstream l_file;

	l_file.open((m_workingDir + filePath).c_str(), l_mode);

	if (!l_file.is_open())
	{
		Logger::Log(LogLevel::Error, "IOService: Can't open file : ", filePath, "!");
		return WsResult::FileNotFound;
	}

	auto pbuf = l_file.rdbuf();
	std::size_t l_size = pbuf->pubseekoff(0, l_file.end, l_file.in);
	pbuf->pubseekpos(0, l_file.in);

	content.reserve(l_size);
	pbuf->sgetn(&content[0], l_size);

	l_file.close();

	return WsResult::Success;
}

Waveless::WsResult Waveless::IOService::saveFile(const char* filePath, const std::vector<char>& content, IOMode saveMode)
{
	std::ios_base::openmode l_mode = std::ios::out;
	switch (saveMode)
	{
	case IOMode::Text:
		l_mode = std::ios::out;
		break;
	case IOMode::Binary:
		l_mode = std::ios::out | std::ios::ate | std::ios::binary;
		break;
	default:
		break;
	}

	std::ofstream l_file;

	l_file.open((m_workingDir + filePath).c_str(), l_mode);

	if (!l_file.is_open())
	{
		Logger::Log(LogLevel::Error, "IOService: Can't open file : ", filePath, "!");
		return WsResult::FileNotFound;
	}

	auto l_result = serializeVector(l_file, content);

	l_file.close();

	return WsResult::Success;
}

bool Waveless::IOService::isFileExist(const char* filePath)
{
	if (fs::exists(fs::path(m_workingDir + filePath)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::string Waveless::IOService::getFilePath(const char* filePath)
{
	return fs::path(filePath).remove_filename().generic_string();
}

std::string Waveless::IOService::getFileExtension(const char* filePath)
{
	return fs::path(filePath).extension().generic_string();
}

std::string Waveless::IOService::getFileName(const char* filePath)
{
	return fs::path(filePath).stem().generic_string();
}

const std::string& Waveless::IOService::getWorkingDirectory()
{
	if (!m_workingDir.size())
	{
		m_workingDir = fs::current_path().generic_string();
		m_workingDir = m_workingDir + "//";
	}
	return m_workingDir;
}

std::vector<std::string> Waveless::IOService::getAllFilePaths(const char * dirctoryPath)
{
	auto l_fullPath = getWorkingDirectory() + dirctoryPath;

	std::vector<std::string> l_result;
	l_result.reserve(8192);

	for (auto& p : fs::recursive_directory_iterator(l_fullPath))
	{
		if (!p.is_directory())
		{
			auto l_relativePath = std::filesystem::relative(p, l_fullPath);
			l_result.emplace_back(l_relativePath.generic_string());
		}
	}

	l_result.shrink_to_fit();

	return l_result;
}