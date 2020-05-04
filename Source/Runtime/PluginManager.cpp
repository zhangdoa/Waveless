#include "PluginManager.h"
#include "../IO/IOService.h"
#define CR_HOST CR_UNSAFE // try to best manage static states
#include "cr.h"

namespace Waveless
{
	std::unordered_map<std::string, cr_plugin> m_pluginInstances;
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
		cr_plugin_update(i.second);
	}

	return WsResult::Success;
}

WsResult Waveless::PluginManager::Terminate()
{
	for (auto& i : m_pluginInstances)
	{
		cr_plugin_close(i.second);
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

WsResult Waveless::PluginManager::LoadPlugin(const char * pluginName, void* userData)
{
	auto l_pluginPath = GeneratePluginPath(pluginName);

	auto l_result = m_pluginInstances.find(l_pluginPath);

	if (l_result == m_pluginInstances.end())
	{
		cr_plugin ctx;
		ctx.userdata = userData;

		cr_plugin_open(ctx, l_pluginPath.c_str());

		m_pluginInstances.emplace(l_pluginPath, ctx);
	}

	return WsResult::Success;
}