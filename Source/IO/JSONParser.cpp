#include "JSONParser.h"
#include "IOService.h"
#include "../Core/Logger.h"
#include "../Core/stdafx.h"

bool Waveless::JSONParser::loadJsonDataFromDisk(const char* fileName, json & data)
{
	std::ifstream i;

	i.open(IOService::getWorkingDirectory() + fileName);

	if (!i.is_open())
	{
		Logger::Log(LogLevel::Error, "JSONParser: Can't open JSON file : ", fileName, "!");
		return false;
	}

	i >> data;
	i.close();

	return true;
}

bool Waveless::JSONParser::saveJsonDataToDisk(const char* fileName, const json & data)
{
	std::ofstream o;

	o.open(IOService::getWorkingDirectory() + fileName, std::ios::out | std::ios::trunc);

	if (!o.is_open())
	{
		Logger::Log(LogLevel::Error, "JSONParser: Can't open JSON file : ", fileName, "!");
		return false;
	}

	o << std::setw(4) << data << std::endl;
	o.close();

	Logger::Log(LogLevel::Verbose, "JSONParser: JSON file : ", fileName, " has been saved.");

	return true;
}