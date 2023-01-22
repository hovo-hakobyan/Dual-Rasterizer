#pragma once
#include "DataTypes.h"

struct SDL_Window;
class Mesh;
class Texture;
struct Camera;

class Rasterizer_Software final
{

public:
	Rasterizer_Software(SDL_Window* pWindow, int w, int h, Camera* pCamera);
	~Rasterizer_Software();

	Rasterizer_Software(const Rasterizer_Software&) = delete;
	Rasterizer_Software(Rasterizer_Software&&) noexcept = delete;
	Rasterizer_Software& operator=(const Rasterizer_Software&) = delete;
	Rasterizer_Software& operator=(Rasterizer_Software&&) noexcept = delete;

	bool Initialize(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	void Update(const Timer* pTimer,float rotDegree);
	void Render(const ColorRGB& bg);

	void CycleShadingMode();
	void ToggleDepthBuffer();
	void ToggleNormalMap();
	void ToggleBoundingBox();

private:
	enum class ShadingMode
	{
		Combined, Diffuse, ObservedArea, Specular, DepthBuffer
	};

	SDL_Window* m_pWindow{};

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	float* m_pDepthBufferPixels{};

	Mesh* m_pVehicleMesh;

	Texture* m_pVehicleDiffuse;
	Texture* m_pVehicleNormal;
	Texture* m_pVehicleSpecular;
	Texture* m_pVehicleGloss;

	

	int m_Width{};
	int m_Height{};
	Camera* m_pCamera;

	ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };
	ShadingMode m_ShadingMode{ ShadingMode::Combined };
	bool m_ShadeDepth;
	bool m_UseNormalMap;
	bool m_UseBoundingBoxVisualization;

	dae::Vector3 m_LightDirection{ .577f,-.577f,.577f };


	void RenderTriangleList(const Mesh* currentMesh);
	void RenderTriangleStrip(const Mesh* currentMesh);
	void LoopOverPixels(const Vertex_Out& ver0, const Vertex_Out& ver1, const Vertex_Out& ver2);

	void PixelShading(const Vertex_Out& v);



};
