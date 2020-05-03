#pragma once
#include "../Core/Typedef.h"

namespace Waveless
{
	class NodeCompiler
	{
	public:
		NodeCompiler() = default;
		~NodeCompiler() = default;

		static WsResult Compile(const char* inputFileName, const char* outputFileName);
	};
}

void WriteScriptSign(const char * inputFileName, std::vector<char> &l_TU);
