#pragma once
#include "../Core/Typedef.h"

namespace Waveless
{
	class Editor
	{
	public:
		Editor() = default;
		~Editor() = default;

		WsResult Setup();
		WsResult Initialize();
		WsResult Render();
		WsResult Terminate();
	};
}