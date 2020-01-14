#include "Editor.h"
#include "ImGuiWrapper/ImGuiWrapper.h"
#include "NodeDescriptorManager.h"
#include "../Runtime/AudioEngine.h"

using namespace Waveless;

WsResult Editor::Setup()
{
	NodeDescriptorManager::GenerateNodeDescriptors("..//..//Asset//Nodes//");
	return ImGuiWrapper::get().Setup();
}

WsResult Editor::Initialize()
{
	if (ImGuiWrapper::get().Initialize() == WsResult::Success)
	{
		return AudioEngine::Initialize();
	}
	else
	{
		return WsResult::Fail;
	}
}

WsResult Editor::Render()
{
	while (ImGuiWrapper::get().Render() == WsResult::Success)
	{
		AudioEngine::Flush();
	}
	return WsResult::Success;
}

WsResult Editor::Terminate()
{
	if (AudioEngine::Terminate() == WsResult::Success)
	{
		return 	ImGuiWrapper::get().Terminate();
	}
	else
	{
		return WsResult::Fail;
	}
}