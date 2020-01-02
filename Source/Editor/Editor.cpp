#include "Editor.h"
#include "ImGuiWrapper/ImGuiWrapper.h"

void WsEditor::Setup()
{
	ImGuiWrapper::get().Setup();
}

void WsEditor::Initialize()
{
	ImGuiWrapper::get().Initialize();
}

void WsEditor::Render()
{
	while (1)
	{
		ImGuiWrapper::get().Render();
	}
}

void WsEditor::Terminate()
{
	ImGuiWrapper::get().Terminate();
}