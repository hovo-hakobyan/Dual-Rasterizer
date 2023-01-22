#include "pch.h"
#include "Rasterizer_Software.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "Utils.h"

using namespace dae;

Rasterizer_Software::Rasterizer_Software(SDL_Window* pWindow, int w, int h, Camera* pCamera) :
	m_pWindow{pWindow},
	m_Width{ w },
	m_Height{ h },
	m_pCamera{pCamera}
{
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//Initialize depthBuffer
	m_pDepthBufferPixels = new float[m_Width * m_Height] {INFINITY};

	//Load in textures
	m_pVehicleDiffuse = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
	m_pVehicleNormal = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pVehicleGloss = Texture::LoadFromFile("Resources/vehicle_gloss.png");
	m_pVehicleSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png");

}

Rasterizer_Software::~Rasterizer_Software()
{
	delete[] m_pDepthBufferPixels;
	delete m_pVehicleDiffuse;
	delete m_pVehicleNormal;
	delete m_pVehicleGloss;
	delete m_pVehicleSpecular;
	delete m_pVehicleMesh;
}

bool Rasterizer_Software::Initialize(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	m_pVehicleMesh = new Mesh{ vertices,indices };

	return m_pVehicleMesh;
}

void Rasterizer_Software::Update(const Timer* pTimer,float rotDegree)
{
	m_pVehicleMesh->RotateY(rotDegree, pTimer->GetElapsed());
}

void Rasterizer_Software::Render(const ColorRGB& bg)
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	const int pixelCount{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, pixelCount, INFINITY);

	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<UINT>(bg.r * 255) , static_cast<UINT>( bg.g * 255), static_cast<UINT>( bg.b * 255)));

	m_pVehicleMesh->TransformVertices(m_pCamera, m_Width, m_Height);

	RenderTriangleList(m_pVehicleMesh);

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Rasterizer_Software::CycleShadingMode()
{
	switch (m_ShadingMode)
	{
	case ShadingMode::Combined:
		std::cout << "**(SOFTWARE) Shading Mode = OBSERVED_AREA\n";
		m_ShadingMode = ShadingMode::ObservedArea;
		break;
	case ShadingMode::Diffuse:
		std::cout << "**(SOFTWARE) Shading Mode = SPECULAR\n";
		m_ShadingMode = ShadingMode::Specular;
		break;
	case ShadingMode::ObservedArea:
		std::cout << "**(SOFTWARE) Shading Mode = DIFFUSE\n";
		m_ShadingMode = ShadingMode::Diffuse;
		break;
	case ShadingMode::Specular:
		std::cout << "**(SOFTWARE) Shading Mode = COMBINED\n";
		m_ShadingMode = ShadingMode::Combined;
		break;
	}

	if (!m_ShadeDepth)
	{
		m_CurrentShadingMode = m_ShadingMode;
	}
}

void Rasterizer_Software::ToggleDepthBuffer()
{
	m_ShadeDepth = !m_ShadeDepth;

	if (m_ShadeDepth)
	{
		std::cout << "**(SOFTWARE) DepthBuffer Visualization ON\n";
	}
	else
	{
		std::cout << "**(SOFTWARE) DepthBuffer Visualization OFF\n";
	}
	m_CurrentShadingMode = m_ShadeDepth == true ? ShadingMode::DepthBuffer : m_ShadingMode;
}

void Rasterizer_Software::ToggleNormalMap()
{
	m_UseNormalMap = !m_UseNormalMap;
	if (m_UseNormalMap)
	{
		std::cout << "**(SOFTWARE) NormalMap ON\n";
	}
	else
	{
		std::cout << "**(SOFTWARE) NormalMap OFF\n";
	}
}



void Rasterizer_Software::RenderTriangleList(const Mesh* currentMesh)
{
	const std::vector<Vertex_Out>& verticesOut{ currentMesh->GetVerticesOut() };
	const std::vector<uint32_t>& indices{ currentMesh->GetIndices() };

	for (size_t idx = 0; idx < indices.size(); idx += 3)
	{
		LoopOverPixels(
			verticesOut[indices[idx]],
			verticesOut[indices[idx + 1]],
			verticesOut[indices[idx + 2]]);
	}
}

void Rasterizer_Software::RenderTriangleStrip(const Mesh* currentMesh)
{
	const std::vector<Vertex_Out>& verticesOut{ currentMesh->GetVerticesOut() };
	const std::vector<uint32_t>& indices{ currentMesh->GetIndices() };

	for (size_t idx = 0; idx < indices.size() - 2; ++idx)
	{

		if (idx % 2 == 0)
		{
			LoopOverPixels(
				verticesOut[indices[idx]],
				verticesOut[indices[idx + 1]],
				verticesOut[indices[idx + 2]]);
		}
		else
		{
			//Fix counterclockwise order
			LoopOverPixels(
				verticesOut[indices[idx]],
				verticesOut[indices[idx + 2]],
				verticesOut[indices[idx + 1]]);
		}

	}
}

void Rasterizer_Software::LoopOverPixels(const Vertex_Out& ver0, const Vertex_Out& ver1, const Vertex_Out& ver2)
{

	//Frustrum culling
	if (ver0.Position.z < 0.f || ver0.Position.z > 1.f)
		return;
	if (ver1.Position.z < 0.f || ver1.Position.z > 1.f)
		return;
	if (ver2.Position.z < 0.f || ver2.Position.z > 1.f)
		return;

	Vector2 v0 = ver0.Position.GetXY();
	Vector2 v1 = ver1.Position.GetXY();
	Vector2 v2 = ver2.Position.GetXY();

	Vector2 topLeft{};
	topLeft.x = std::min(std::min(v0.x, v1.x), v2.x);
	topLeft.y = std::min(std::min(v0.y, v1.y), v2.y);

	Vector2 bottomRight{};
	bottomRight.x = std::max(std::max(v0.x, v1.x), v2.x);
	bottomRight.y = std::max(std::max(v0.y, v1.y), v2.y);


	Vector3 weight{};
	for (int px{ std::max(0,static_cast<int>(topLeft.x)) }; px <= std::min(m_Width, static_cast<int>(bottomRight.x)); ++px)
	{
		for (int py{ std::max(0,static_cast<int>(topLeft.y)) }; py <= std::min(m_Height, static_cast<int>(bottomRight.y)); ++py)
		{
			Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

			if (Utils::IsInsideTriangle(pixel, v0, v1, v2, weight))
			{
				//Z interpolated non-linear
				float currentDepth = 1.f / (weight.x / ver0.Position.z + weight.y / ver1.Position.z + weight.z / ver2.Position.z);

				if (currentDepth < m_pDepthBufferPixels[px + (py * m_Width)])
				{
					//Z-interpolated, linear
					float wBuffer{ 1 / (1 / ver0.Position.w * weight.x + 1 / ver1.Position.w * weight.y + 1 / ver2.Position.w * weight.z) };
					Vector2 uv{};
					uv = (
						ver0.Uv / ver0.Position.w * weight.x +
						ver1.Uv / ver1.Position.w * weight.y +
						ver2.Uv / ver2.Position.w * weight.z) * wBuffer;

					Vector3 normal{ (
						ver0.Normal * weight.x * ver0.Position.w +
						ver1.Normal * weight.y * ver1.Position.w +
						ver2.Normal * weight.z * ver2.Position.w) * wBuffer };

					normal.Normalize();

					Vector3 tangent{ (
						ver0.Tangent * weight.x * ver0.Position.w +
						ver1.Tangent * weight.y * ver1.Position.w +
						ver2.Tangent * weight.z * ver2.Position.w) * wBuffer };
					tangent.Normalize();

					Vector3 viewDir{ (
						ver0.ViewDirection * weight.x * ver0.Position.w +
						ver1.ViewDirection * weight.y * ver1.Position.w +
						ver2.ViewDirection * weight.z * ver2.Position.w) * wBuffer };
					viewDir.Normalize();

					Vertex_Out currentPixel
					{
						Vector4{static_cast<float>(px),static_cast<float>(py),currentDepth,wBuffer},
						uv,
						normal,
						tangent,
						viewDir
					};

					m_pDepthBufferPixels[px + (py * m_Width)] = currentDepth;

					PixelShading(currentPixel);
				}

			}
		}
	}
}

void Rasterizer_Software::PixelShading(const Vertex_Out& v)
{
	ColorRGB finalColor{};
	float remapped{};
	const float intensity{ 7.f };
	const float shininess{ 25.f };

	Vector3 binormal{ Vector3::Cross(v.Normal,v.Tangent) };
	Matrix tangentSpaceAxis = Matrix{ v.Tangent,binormal,v.Normal,Vector3::Zero };

	ColorRGB normalSample{ m_pVehicleNormal->Sample(v.Uv) };
	Vector3 normalSampleVec{ normalSample.r,normalSample.g,normalSample.b };

	Vector3 normal{ 2.f * normalSampleVec - Vector3{1.f,1.f,1.f} };
	normal = m_UseNormalMap ? tangentSpaceAxis.TransformVector(normal).Normalized() : v.Normal;

	const float lambertCosine{ Vector3::Dot(normal, -m_LightDirection) };

	if (m_CurrentShadingMode == ShadingMode::DepthBuffer)
	{
		remapped = Remap(v.Position.z);
		finalColor = { remapped,remapped,remapped };
	}
	else
	{
		if (lambertCosine > 0.f)
		{
			//Phong
			Vector3 reflect = -m_LightDirection - 2 * std::max(Vector3::Dot(normal, -m_LightDirection), 0.f) * normal;
			float alpha = std::max(Vector3::Dot(reflect, v.ViewDirection), 0.f);
			ColorRGB specular = m_pVehicleSpecular->Sample(v.Uv) * powf(alpha, shininess * m_pVehicleGloss->Sample(v.Uv).r);

			specular.r = std::max(0.f, specular.r);
			specular.g = std::max(0.f, specular.g);
			specular.b = std::max(0.f, specular.b);

			ColorRGB ambient{ .025f,.025f, .025f };
			ColorRGB diffuse{ Utils::Lambert(intensity, m_pVehicleDiffuse->Sample(v.Uv)) };

			switch (m_CurrentShadingMode)
			{
			case ShadingMode::Combined:
				finalColor = (diffuse + specular + ambient) * lambertCosine;
				break;
			case ShadingMode::Diffuse:
				finalColor = diffuse * lambertCosine;
				break;
			case ShadingMode::ObservedArea:
				finalColor = ColorRGB{ lambertCosine,lambertCosine,lambertCosine };
				break;
			case ShadingMode::Specular:
				finalColor = specular * lambertCosine;
				break;
			}
		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBackBufferPixels[static_cast<int>(v.Position.x) + (static_cast<int>(v.Position.y) * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}
