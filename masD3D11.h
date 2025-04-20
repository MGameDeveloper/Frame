#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct D3D11Data
{
	ComPtr<IDXGIFactory7>          Factory7;
	ComPtr<IDXGISwapChain1>        SwapChain1;
	ComPtr<ID3D11Device5>          Device5;
	ComPtr<ID3D11DeviceContext4>   ImmediateContext4;
	ComPtr<ID3D11RenderTargetView> MainRenderTargetView;
	D3D11_VIEWPORT                 MainViewport;
	bool                           IsTearingSupported;
};

bool             masD3D11_Init(void* WndHandle, int WndWidth, int WndHeight);
void             masD3D11_Shutdown();
void             masD3D11_Clear(int R = 0, int G = 0, int B = 0, int A = 255);
void             masD3D11_Present();
const D3D11Data* masD3D11_GetData();