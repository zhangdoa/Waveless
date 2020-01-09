#include "Editor.h"
#include "ImGuiWrapper/ImGuiWrapper.h"
#include "../Runtime/AudioEngine.h"
#include "NodeDescriptorGenerator.h"

using namespace Waveless;

void WsEditor::Setup()
{
	NodeDescriptorGenerator::GenerateNodeDescriptors("..//..//Asset//Nodes//");
	ImGuiWrapper::get().Setup();
}

void WsEditor::Initialize()
{
	ImGuiWrapper::get().Initialize();
	AudioEngine::Initialize();
}

void WsEditor::Render()
{
	while (ImGuiWrapper::get().Render())
	{
		AudioEngine::Flush();
	}
}

void WsEditor::Terminate()
{
	AudioEngine::Terminate();
	ImGuiWrapper::get().Terminate();
}