#include "masD3D11.h"
#include <assert.h>
#include <math.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static D3D11Data GD3D11Data = {};

const D3D11Data* D3D11_GetData()
{
	assert(GD3D11Data.Device5.Get() != NULL && "Failed Creating D3D11");
	return &GD3D11Data;
}

/********************************************************************************************
*
*********************************************************************************************/
bool masD3D11_Init(void* WndHandle, int WndWidth, int WndHeight)
{
	/*
	* FACTORY CREATION
	*/
	UINT CreateFactoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	CreateFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	ComPtr<IDXGIFactory2> pFactory2 = NULL;
	HRESULT hr = ::CreateDXGIFactory2(CreateFactoryFlags, IID_PPV_ARGS(&pFactory2));
	if (FAILED(hr))
		return false;

	UINT IsTearingSupported = 0;
	ComPtr<IDXGIFactory7> pFactory7 = NULL;
	hr = pFactory2.As(&pFactory7);
	if (FAILED(hr))
		return false;
	else
	{
		hr = pFactory7->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &IsTearingSupported, sizeof(UINT));
		if (FAILED(hr))
			return false;
	}


	/*
	* ADAPTER & ADAPTER DESC CREATION
	*/
	DXGI_ADAPTER_DESC3 AdapterDesc3 = {};
	ComPtr<IDXGIAdapter4> pAdapter4 = NULL;
	hr = pFactory7->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter4));
	if (FAILED(hr))
		return false;
	else
	{
		hr = pAdapter4->GetDesc3(&AdapterDesc3);
		if (FAILED(hr))
			return false;
	}

	/*
	* DEVICE & IMMEDIATE CONTEXT CREATION
	*/
	UINT DeviceCreateFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; // FOR DIRECT2D & DIRECTWRITE
#if defined(DEBUG) || defined(_DEBUG)
	DeviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif;
	D3D_FEATURE_LEVEL           Features[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	UINT                        FeaturesCount = ARRAYSIZE(Features);
	D3D_FEATURE_LEVEL           FeatureLevel;
	ComPtr<ID3D11Device>        pDevice = NULL;
	ComPtr<ID3D11DeviceContext> pImmediateContext = NULL;
	hr = ::D3D11CreateDevice(pAdapter4.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, DeviceCreateFlags, Features, FeaturesCount,
		D3D11_SDK_VERSION, &pDevice, &FeatureLevel, &pImmediateContext);
	if (FAILED(hr))
		return false;

	ComPtr<ID3D11Device5> pDevice5;
	hr = pDevice.As(&pDevice5);
	if (FAILED(hr))
		return false;

	ComPtr<ID3D11DeviceContext4> pImmediateContext4;
	hr = pImmediateContext.As(&pImmediateContext4);
	if (FAILED(hr))
		return false;


	/*
	* SWAPCHAIN CREAITON
	*/
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1;
	SwapChainDesc1.Width = WndWidth;
	SwapChainDesc1.Height = WndHeight;
	SwapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc1.Stereo = FALSE;
	SwapChainDesc1.SampleDesc = { 1, 0 };
	SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc1.BufferCount = 3;
	SwapChainDesc1.Scaling = DXGI_SCALING_NONE;
	SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	SwapChainDesc1.Flags = (IsTearingSupported) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	ComPtr<IDXGISwapChain1> pSwapChain1 = NULL;
	hr = pFactory7->CreateSwapChainForHwnd(pDevice.Get(), (HWND)WndHandle, &SwapChainDesc1, NULL, NULL, &pSwapChain1);
	if (FAILED(hr))
		return false;


	/*
	* RENDER TARGET CREATION
	*/
	ComPtr<ID3D11Texture2D> pBackBuffer = NULL;
	hr = pSwapChain1->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(hr))
		return false;

	ComPtr<ID3D11RenderTargetView> pMainRenderTarget = NULL;
	hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, &pMainRenderTarget);
	if (FAILED(hr))
		return false;


	/*
	* VIEWPORT CREATION
	*/
	D3D11_VIEWPORT MainViewport
	{
		0, 0, (float)WndWidth, (float)WndHeight, 0.f, 1.f
	};

	/*
	* SETUP OUR STRUCT
	*/
	GD3D11Data.Factory7 = pFactory7;
	GD3D11Data.IsTearingSupported = IsTearingSupported;
	GD3D11Data.Device5 = pDevice5;
	GD3D11Data.ImmediateContext4 = pImmediateContext4;
	GD3D11Data.SwapChain1 = pSwapChain1;
	GD3D11Data.MainRenderTargetView = pMainRenderTarget;
	GD3D11Data.MainViewport = MainViewport;

	return true;
}

void masD3D11_Shutdown()
{

}


void masD3D11_Clear(int R, int G, int B, int A)
{
	ID3D11DeviceContext4* ImmediateContext = GD3D11Data.ImmediateContext4.Get();

	ImmediateContext->OMSetRenderTargets(1, GD3D11Data.MainRenderTargetView.GetAddressOf(), NULL);
	ImmediateContext->RSSetViewports(1, &GD3D11Data.MainViewport);

	static float f = 1.f / 255.f;
	float ClearColor[4] = { R * f, G * f, B * f, A * f };
	ImmediateContext->ClearRenderTargetView(GD3D11Data.MainRenderTargetView.Get(), ClearColor);
}

void masD3D11_Present()
{
	UINT Sync = 0;
	UINT Flag = (GD3D11Data.IsTearingSupported) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	DXGI_PRESENT_PARAMETERS Params = {};
	HRESULT hr = GD3D11Data.SwapChain1->Present1(Sync, Flag, &Params);
	assert(hr == S_OK && "present failed");
}
