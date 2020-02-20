#include "String.h"

namespace StringManagerNS
{
	std::vector<std::string> m_stringPool;
}

using namespace StringManagerNS;

Waveless::WsResult Waveless::StringManager::Setup()
{
	m_stringPool.reserve(8192);

	return WsResult::Success;
}

const char * Waveless::StringManager::SpawnString(const char * rhs)
{
	m_stringPool.emplace_back(rhs);
	return m_stringPool.back().c_str();
}