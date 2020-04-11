#include "String.h"
#include "Math.h"

namespace StringManagerNS
{
	std::unordered_map<uint64_t, std::string> m_rawStringPool;
	std::unordered_map<uint64_t, Waveless::WsString> m_WsStringPool;
}

using namespace Waveless;
using namespace StringManagerNS;

const char* FindRawString(uint64_t UUID)
{
	auto l_result = m_rawStringPool.find(UUID);
	if (l_result != m_rawStringPool.end())
	{
		return l_result->second.c_str();
	}
	else
	{
		return nullptr;
	}
}

WsResult StringManager::Setup()
{
	m_rawStringPool.reserve(8192);
	m_WsStringPool.reserve(8192);

	return WsResult::Success;
}

WsString StringManager::SpawnString(const char * rhs)
{
	auto l_id = Math::GenerateUUID();
	m_rawStringPool.emplace(l_id, rhs);

	WsString l_instance;
	l_instance.UUID = l_id;
	l_instance.objectState = ObjectState::Active;
	l_instance.value = FindRawString(l_id);

	m_WsStringPool.emplace(l_id, l_instance);

	return FindString(l_id);
}

WsString StringManager::FindString(uint64_t UUID)
{
	auto l_result = m_WsStringPool.find(UUID);
	if (l_result != m_WsStringPool.end())
	{
		return l_result->second;
	}
	else
	{
		return WsString();
	}
}

WsResult StringManager::DeleteString(uint64_t UUID)
{
	auto l_WsString = m_WsStringPool.find(UUID);
	if (l_WsString != m_WsStringPool.end())
	{
		m_WsStringPool.erase(UUID);
		auto l_rawString = m_rawStringPool.find(UUID);
		if (l_rawString != m_rawStringPool.end())
		{
			m_rawStringPool.erase(UUID);
			return WsResult::Success;
		}
		else
		{
			return WsResult::IDNotFound;
		}
	}
	else
	{
		return WsResult::IDNotFound;
	}
}