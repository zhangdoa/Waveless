#include "ImGuiRendererDX11.h"
#include "ImGuiWindowWin.h"

#include "../../ThirdParty/ImGui/imgui_impl_dx11.cpp"

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

namespace ImGuiRendererDX11NS
{
	ID3D11Device*            m_d3dDevice = nullptr;
	ID3D11DeviceContext*     m_d3dDeviceContext = nullptr;
	IDXGISwapChain*          m_SwapChain = nullptr;
	ID3D11RenderTargetView*  m_mainRenderTargetView = nullptr;

	void CreateRenderTarget()
	{
		DXGI_SWAP_CHAIN_DESC sd;
		m_SwapChain->GetDesc(&sd);

		// Create the render target
		ID3D11Texture2D* pBackBuffer;
		D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
		ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));
		render_target_view_desc.Format = sd.BufferDesc.Format;
		render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		m_d3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &m_mainRenderTargetView);
		m_d3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
		pBackBuffer->Release();
	}

	void CleanupRenderTarget()
	{
		if (m_mainRenderTargetView) { m_mainRenderTargetView->Release(); m_mainRenderTargetView = nullptr; }
	}

	HRESULT CreateDeviceD3D(HWND hWnd)
	{
		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC sd;
		{
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 2;
			sd.BufferDesc.Width = 0;
			sd.BufferDesc.Height = 0;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		}

		UINT createDeviceFlags = 0;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0, };
		if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 1, D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_d3dDevice, &featureLevel, &m_d3dDeviceContext) != S_OK)
			return E_FAIL;

		CreateRenderTarget();

		return S_OK;
	}

	void CleanupDeviceD3D()
	{
		CleanupRenderTarget();
		if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
		if (m_d3dDeviceContext) { m_d3dDeviceContext->Release(); m_d3dDeviceContext = nullptr; }
		if (m_d3dDevice) { m_d3dDevice->Release(); m_d3dDevice = nullptr; }
	}
}

using namespace ImGuiRendererDX11NS;

bool ImGuiRendererDX11::setup()
{
	CreateDeviceD3D(ImGuiWindowWin::getHWND());
	return true;
}

bool ImGuiRendererDX11::initialize()
{
	ImGui_ImplDX11_Init(m_d3dDevice, m_d3dDeviceContext);

	return true;
}

bool ImGuiRendererDX11::newFrame()
{
	ImGui_ImplDX11_NewFrame();
	return true;
}

bool ImGuiRendererDX11::render()
{
	g_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}

bool ImGuiRendererDX11::terminate()
{
	ImGui_ImplDX11_Shutdown();
	CleanupDeviceD3D();
	return true;
}