#include "ImGuiRendererGL.h"

namespace ImGuiRendererGLNS
{
}

bool ImGuiRendererGL::setup()
{
	return true;
}

bool ImGuiRendererGL::initialize()
{
	return true;
}

bool ImGuiRendererGL::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	return true;
}

bool ImGuiRendererGL::render()
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return true;
}

bool ImGuiRendererGL::terminate()
{
	ImGui_ImplOpenGL3_Shutdown();

	return true;
}