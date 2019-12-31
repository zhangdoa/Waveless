#include "ImGuiWrapper.h"
#include "../../../GitSubmodules/imgui-node-editor/ThirdParty/imgui/imgui.h"

#if defined WS_OS_WIN
#include "ImGuiWindowWin.h"
#include "ImGuiRendererDX11.h"
#endif

#if defined WS_OS_MAC
#include "ImGuiWindowMac.h"
#include "ImGuiRendererMT.h"
#endif

#if defined WS_OS_LINUX
#include "ImGuiWindowLinux.h"
#endif

#if defined WS_RENDERER_OPENGL
#include "ImGuiRendererGL.h"
#endif

#if defined WS_RENDERER_VULKAN
#include "ImGuiRendererVK.h"
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
	m_windowImpl = new ImGuiWrapperWindowWin();
#endif

	switch (m_renderer)
	{
	case Renderer::GL:
#if !defined WS_OS_MAC && defined INNO_RENDERER_OPENGL
		m_rendererImpl = new ImGuiWrapperGL();
#endif
		break;
	case Renderer::DX11:
#if defined WS_OS_WIN
		m_rendererImpl = new ImGuiWrapperDX11();
#endif
		break;
	case Renderer::DX12:
#if defined WS_OS_WIN
		ImGuiWrapperNS::m_isParity = false;
#endif
		break;
	case Renderer::VK:
#if defined INNO_RENDERER_VULKAN
		m_rendererImpl = new ImGuiWrapperVK();
#endif
		break;
	case Renderer::MT:
#if defined WS_OS_MAC
		ImGuiWrapperNS::m_isParity = false;
#endif
		break;
	default:
		break;
	}

#if defined WS_OS_LINUX
	ImGuiWrapperNS::m_isParity = false;
#endif

	if (ImGuiWrapperNS::m_isParity)
	{
		// Setup Dear ImGui binding
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
		{
		}
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