#pragma once
#include "../Core/Typedef.h"

namespace Waveless
{
	class NodeCompiler
	{
	public:
		NodeCompiler() = default;
		~NodeCompiler() = default;

		static WsResult Compile(const char* inputFileName, const char* outputFileName, uint64_t& pluginUUID);
	};
}