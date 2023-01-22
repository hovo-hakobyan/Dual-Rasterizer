#include "pch.h"
#include "Rasterizer_Hardware.h"
#include "Camera.h"
#include "Mesh.h"

Rasterizer_Hardware::Rasterizer_Hardware(SDL_Window* pWindow, int w, int h, Camera* pCamera):
	m_pWindow{pWindow},
	m_Width{w},
	m_Height{h},
	m_pCamera{pCamera}
{
}

Rasterizer_Hardware::~Rasterizer_Hardware()
{

	if (m_pVehicleMesh)
	{
		delete m_pVehicleMesh;
		m_pVehicleMesh = nullptr;
	}
	

	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();

	if (m_pRenderTargetBuffer)
		m_pRenderTargetBuffer->Release();

	if (m_pDepthStencilView)
		m_pDepthStencilView->Release();

	if (m_pDepthStencilBuffer)
		m_pDepthStencilBuffer->Release();

	if (m_pSwapChain)
		m_pSwapChain->Release();

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	if (m_pDevice)
		m_pDevice->Release();

}

void Rasterizer_Hardware::Update(const Timer* pTimer,float rotDegree)
{
	m_pVehicleMesh->UpdateMatrices(*m_pCamera);
	m_pVehicleMesh->RotateY(rotDegree, pTimer->GetElapsed());
}

void Rasterizer_Hardware::Render(const ColorRGB& bg) const
{
	//1. Clear RTV & DSV;
	ColorRGB clearColor = bg;
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//2. Set Pipeline + invoke drawcalls
	m_pVehicleMesh->Render(m_pDeviceContext);

	//3. Present Backbuffer (swap)
	m_pSwapChain->Present(0, 0);
}

void Rasterizer_Hardware::CycleFilterMode()
{
	m_pVehicleMesh->CycleFilterMode();
}

HRESULT Rasterizer_Hardware::InitializeDirectX(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	
	//1. Create Device & DeviceContext
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION,
		&m_pDevice, nullptr, &m_pDeviceContext);

	if (FAILED(result))
		return result;

	//Create Mesh
	m_pVehicleMesh = new Mesh{ m_pDevice,vertices,indices };

	//2.Create DXGI Factory
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

	if (FAILED(result))
		return result;

	//Create swapchain Desc
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//3.Get the handle (HWND) from the SDL Backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

	if (FAILED(result))
		return result;



	//4.Create DepthStencil (DS) & DepthStencilView (DSV)
	//Resource
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;


	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;



	//5.Create RenderTarget (RT) & RenderTargetView (RTV) (buffer)

	//Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	//View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	// 6.Bind RTV & DSV(buffer) to Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//7. Set Viewport
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);


	if (pDxgiFactory)
		pDxgiFactory->Release();



	return S_OK;

}
