#include "PluginManager.h"
#include "../Core/Object.h"
#include "../Core/Math.h"
#include "../IO/IOService.h"
#define CR_HOST CR_UNSAFE // try to best manage static states
#include "cr.h"

namespace Waveless
{
	struct Plugin : public Object
	{
		bool m_NeedUpdate = false;
		cr_plugin m_Context = cr_plugin{};
		std::string m_FilePath;
	};

	std::unordered_map<std::string, uint64_t> m_pluginFilePaths;
	std::unordered_map<uint64_t, Plugin> m_pluginInstances;
}

using namespace Waveless;

WsResult Waveless::PluginManager::Initialize()
{
	return WsResult::Success;
}

WsResult Waveless::PluginManager::Update()
{
	for (auto& i : m_pluginInstances)
	{
		if (i.second.m_NeedUpdate)
		{
			cr_plugin_update(i.second.m_Context);
			i.second.m_NeedUpdate = false;
		}
	}

	return WsResult::Success;
}

WsResult Waveless::PluginManager::Terminate()
{
	for (auto& i : m_pluginInstances)
	{
		cr_plugin_close(i.second.m_Context);
	}

	return WsResult::Success;
}

std::string GeneratePluginPath(const char * pluginName)
{
#ifdef _DEBUG
	auto l_pluginPath = IOService::getWorkingDirectory() + "Debug/";
#else
	auto l_pluginPath = IOService::getWorkingDirectory() + "Release/";
#endif // _DEBUG

	l_pluginPath += pluginName;

#if defined(_WIN32)
	l_pluginPath += ".dll";
#elif defined(__linux__)
	l_pluginPath += ".so";
#elif defined(__APPLE__)
	l_pluginPath += ".dylib";
#else
#endif

	return l_pluginPath;
}

WsResult Waveless::PluginManager::LoadPlugin(const char* pluginName, uint64_t& UUID)
{
	auto l_pluginPath = GeneratePluginPath(pluginName);

	auto l_result = m_pluginFilePaths.find(l_pluginPath);

	if (l_result == m_pluginFilePaths.end())
	{
		Plugin l_plugin;
		l_plugin.UUID = Math::GenerateUUID();
		l_plugin.m_FilePath = l_pluginPath;

		cr_plugin_open(l_plugin.m_Context, l_pluginPath.c_str());
		l_plugin.objectState = ObjectState::Activated;
		l_plugin.m_Context.userdata = 0;

		m_pluginFilePaths.emplace(l_pluginPath, l_plugin.UUID);
		m_pluginInstances.emplace(l_plugin.UUID, l_plugin);
		UUID = l_plugin.UUID;
	}

	return WsResult::Success;
}

WsResult Waveless::PluginManager::UploadUserData(uint64_t UUID, void* userData)
{
	auto l_result = m_pluginInstances.find(UUID);

	if (l_result != m_pluginInstances.end())
	{
		l_result->second.m_Context.userdata = userData;
		l_result->second.m_NeedUpdate = true;
		return WsResult::Success;
	}
	return WsResult::IDNotFound;
}