#pragma once
#include "../Core/Typedef.h"
#include "../../GitSubmodules/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace Waveless
{
	namespace JSONParser
	{
		WsResult loadJsonDataFromDisk(const char* fileName, json & data);
		WsResult saveJsonDataToDisk(const char* fileName, const json & data);
	}
}
