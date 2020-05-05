#pragma once
#include "Object.h"
#include "Math.h"

namespace Waveless
{
	struct WsVector : public Object
	{
		Vector* value;
	};

	namespace VectorManager
	{
		WsResult Setup();
		WsVector SpawnVector(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f);
		WsVector FindVector(uint64_t UUID);
		WsResult DeleteVector(uint64_t UUID);
	};
}
