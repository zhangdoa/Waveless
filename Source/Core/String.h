#pragma once
#include "Object.h"

namespace Waveless
{
	struct WsString : public Object
	{
		const char* value;
	};

	namespace StringManager
	{
		WsResult Setup();
		WsString SpawnString(const char* rhs);
		WsString FindString(uint64_t UUID);
		WsResult DeleteString(uint64_t UUID);
	};
}
