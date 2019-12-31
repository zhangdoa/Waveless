#include "ImGuiRendererDX11.h"

namespace ImGuiRendererDX11NS
{
}

bool ImGuiRendererDX11::setup()
{
	return true;
}

bool ImGuiRendererDX11::initialize()
{
	ImGui_ImplDX11_Init(l_device, l_deviceContext);

	return true;
}

bool ImGuiRendererDX11::newFrame()
{
	ImGui_ImplDX11_NewFrame();
	return true;
}

bool ImGuiRendererDX11::render()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}

bool ImGuiRendererDX11::terminate()
{
	ImGui_ImplDX11_Shutdown();

	return true;
}