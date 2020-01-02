#include "ImGuiWrapper.h"
#include "../../Core/Config.h"

#include "../../../GitSubmodules/imgui/imgui.h"

#if defined WS_OS_WIN
#include "ImGuiWindowWin.h"
#endif

#if defined WS_OS_MAC
#include "ImGuiWindowMac.h"
#endif

#if defined WS_OS_LINUX
#include "ImGuiWindowLinux.h"
#endif

#if defined WS_RENDERER_DIRECTX
#include "ImGuiRendererDX11.h"
#endif

#if defined WS_RENDERER_OPENGL
#include "ImGuiRendererGL.h"
#endif

#if defined WS_RENDERER_VULKAN
#include "ImGuiRendererVK.h"
#endif

#if defined WS_RENDERER_METAL
#include "ImGuiRendererMT.h"
#endif

namespace ImGuiWrapperNS
{
	bool m_isParity = true;

	IImGuiWindow* m_windowImpl;
	IImGuiRenderer* m_rendererImpl;
}

using namespace ImGuiWrapperNS;

bool ImGuiWrapper::Setup()
{
#if defined WS_OS_WIN
	m_windowImpl = new ImGuiWindowWin();
#if defined WS_RENDERER_DIRECTX
	m_rendererImpl = new ImGuiRendererDX11();
#elif WS_RENDERER_VULKAN
	m_rendererImpl = new ImGuiRendererVK();
#elif defined WS_RENDERER_OPENGL
	m_rendererImpl = new ImGuiRendererGL();
#endif
#endif
#if defined WS_OS_MAC
	ImGuiWrapperNS::m_isParity = false;
#if defined WS_RENDERER_METAL
	ImGuiWrapperNS::m_isParity = false;
#endif
#endif
#if defined WS_OS_LINUX
	ImGuiWrapperNS::m_isParity = false;
#endif

	if (ImGuiWrapperNS::m_isParity)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiWrapperNS::m_windowImpl->setup();
		ImGuiWrapperNS::m_rendererImpl->setup();
	}

	return true;
}

bool ImGuiWrapper::Initialize()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->initialize();
		ImGuiWrapperNS::m_rendererImpl->initialize();
		ImGui::StyleColorsDark();
		ImVec4 backgroundColor = ImColor(32, 32, 32, 255);
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);
	}

	return true;
}

bool ImGuiWrapper::Render()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_rendererImpl->newFrame();
		ImGuiWrapperNS::m_windowImpl->newFrame();

		ImGui::NewFrame();

		ImGui::Begin("Content", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus);

		ImGui::End();

		ImGui::Render();

		ImGuiWrapperNS::m_rendererImpl->render();
	}
	return true;
}

bool ImGuiWrapper::Terminate()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->terminate();
		ImGuiWrapperNS::m_rendererImpl->terminate();
		ImGui::DestroyContext();
	}
	return true;
}