#pragma once
#include "../Core/Typedef.h"

namespace Waveless
{
	class PluginManager
	{
	public:
		PluginManager() = default;
		~PluginManager() = default;

		static WsResult Initialize();
		static WsResult Update();
		static WsResult Terminate();

		static WsResult LoadPlugin(const char* pluginName, void* userData);
	};
}
