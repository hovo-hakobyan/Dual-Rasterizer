#pragma once
#include "DataTypes.h"

struct SDL_Window;
class Mesh;
class Texture;
struct Camera;

class Rasterizer_Hardware final
{
public:
	Rasterizer_Hardware(SDL_Window* pWindow, int w, int h, Camera* pCamera);
	~Rasterizer_Hardware();

	Rasterizer_Hardware(const Rasterizer_Hardware&) = delete;
	Rasterizer_Hardware(Rasterizer_Hardware&&) noexcept = delete;
	Rasterizer_Hardware& operator=(const Rasterizer_Hardware&) = delete;
	Rasterizer_Hardware& operator=(Rasterizer_Hardware&&) noexcept = delete;

	HRESULT InitializeDirectX(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	void Update(const Timer* pTimer,float rotDegree);
	void Render(const ColorRGB& bg) const;


	void CycleFilterMode();

private:
	SDL_Window* m_pWindow{};

	int m_Width{};
	int m_Height{};

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	IDXGISwapChain* m_pSwapChain;

	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;


	ID3D11Resource* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;

	Mesh* m_pVehicleMesh;

	Camera* m_pCamera{};

};

