#include "Vector.h"

namespace VectorManagerNS
{
	std::unordered_map<uint64_t, Waveless::Vector> m_rawVectorPool;
	std::unordered_map<uint64_t, Waveless::WsVector> m_WsVectorPool;
}

using namespace Waveless;
using namespace VectorManagerNS;

Vector* FindRawVector(uint64_t UUID)
{
	auto l_result = m_rawVectorPool.find(UUID);
	if (l_result != m_rawVectorPool.end())
	{
		return &l_result->second;
	}
	else
	{
		return nullptr;
	}
}

WsResult VectorManager::Setup()
{
	m_rawVectorPool.reserve(8192);
	m_WsVectorPool.reserve(8192);

	return WsResult::Success;
}

WsVector VectorManager::SpawnVector(float x, float y, float z, float w)
{
	auto l_id = Math::GenerateUUID();
	m_rawVectorPool.emplace(l_id, Vector(x, y, z, w));

	WsVector l_instance;
	l_instance.UUID = l_id;
	l_instance.objectState = ObjectState::Activated;
	l_instance.value = FindRawVector(l_id);

	m_WsVectorPool.emplace(l_id, l_instance);

	return FindVector(l_id);
}

WsVector VectorManager::FindVector(uint64_t UUID)
{
	auto l_result = m_WsVectorPool.find(UUID);
	if (l_result != m_WsVectorPool.end())
	{
		return l_result->second;
	}
	else
	{
		return WsVector();
	}
}

WsResult VectorManager::DeleteVector(uint64_t UUID)
{
	auto l_WsVector = m_WsVectorPool.find(UUID);
	if (l_WsVector != m_WsVectorPool.end())
	{
		m_WsVectorPool.erase(UUID);
		auto l_rawVector = m_rawVectorPool.find(UUID);
		if (l_rawVector != m_rawVectorPool.end())
		{
			m_rawVectorPool.erase(UUID);
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