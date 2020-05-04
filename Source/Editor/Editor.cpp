#include "Editor.h"
#include "ImGuiWrapper/ImGuiWrapper.h"
#include "NodeDescriptorManager.h"
#include "../Runtime/PluginManager.h"
#include "../Runtime/AudioEngine.h"
#include "../Core/String.h"

using namespace Waveless;

WsResult Editor::Setup()
{
	StringManager::Setup();
	NodeDescriptorManager::LoadAllNodeDescriptors("..//..//Asset//Nodes//");
	return ImGuiWrapper::get().Setup();
}

WsResult Editor::Initialize()
{
	if (ImGuiWrapper::get().Initialize() != WsResult::Success)
	{
		return WsResult::Fail;
	}
	if (PluginManager::Initialize() != WsResult::Success)
	{
		return WsResult::Fail;
	}
	if (AudioEngine::Initialize() != WsResult::Success)
	{
		return WsResult::Fail;
	}
	return WsResult::Success;
}

WsResult Editor::Render()
{
	while (ImGuiWrapper::get().Render() == WsResult::Success)
	{
		PluginManager::Update();
		AudioEngine::Flush();
	}
	return WsResult::Success;
}

WsResult Editor::Terminate()
{
	if (AudioEngine::Terminate() != WsResult::Success)
	{
		return WsResult::Fail;
	}
	if (PluginManager::Terminate() != WsResult::Success)
	{
		return WsResult::Fail;
	}
	if (ImGuiWrapper::get().Terminate() != WsResult::Success)
	{
		return WsResult::Fail;
	}

	return WsResult::Success;
}