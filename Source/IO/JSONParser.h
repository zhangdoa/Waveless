#pragma once
#include "../../GitSubmodules/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace Waveless
{
	namespace JSONParser
	{
		bool loadJsonDataFromDisk(const char* fileName, json & data);
		bool saveJsonDataToDisk(const char* fileName, const json & data);
	}
}
