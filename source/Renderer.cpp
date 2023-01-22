#include "pch.h"
#include "Renderer.h"
#include "Utils.h"
#include "Rasterizer_Software.h"
#include "Rasterizer_Hardware.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	
		//Parse obj
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		//Initialize Camera
		m_Camera.Initialize(static_cast<float>(m_Width) / m_Height, 45.f, { .0f,.0f,0.f });

		//Initialize Software Rasterizer
		m_pSoftwareRasterizer = new Rasterizer_Software(pWindow, m_Width, m_Height, &m_Camera);
		const bool res = m_pSoftwareRasterizer->Initialize(vertices, indices);
		if (res)
		{
			m_IsDirectXInitialized = true;
		}
		else
		{
			std::cout << "Software Rasterizer initialization failed!\n";
		}


		//Initialize Hardware Rasterizer
		m_pHardwareRasterizer = new Rasterizer_Hardware(pWindow, m_Width, m_Height, &m_Camera);
		const HRESULT result = m_pHardwareRasterizer->InitializeDirectX(vertices,indices);

		if (result == S_OK)
		{
			m_IsDirectXInitialized = true;
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
		
		//Set start render method
		m_CurrentRenderMethod = RenderMethod::Hardware;
		m_CurrentBGColor = ColorRGB{ .39f,.59f,.93f };

		PrintInfo();
	}

	Renderer::~Renderer()
	{
		if (m_pHardwareRasterizer)
		{
			delete m_pHardwareRasterizer;
			m_pHardwareRasterizer = nullptr;
		}

		if (m_pSoftwareRasterizer)
		{
			delete m_pSoftwareRasterizer;
			m_pSoftwareRasterizer = nullptr;
		}
	}

	void Renderer::Update(const Timer* pTimer)
	{
		
		const float rotDegree = m_ShouldRotate? 45.f : 0.f;
		m_Camera.Update(pTimer);

		m_pSoftwareRasterizer->Update(pTimer,rotDegree);
		m_pHardwareRasterizer->Update(pTimer,rotDegree);
	}


	void Renderer::Render() const
	{
		switch (m_CurrentRenderMethod)
		{
		case dae::Renderer::RenderMethod::Hardware:
			m_pHardwareRasterizer->Render(m_CurrentBGColor);
			break;
		case dae::Renderer::RenderMethod::Software:
			m_pSoftwareRasterizer->Render(m_CurrentBGColor);
			break;

		}

	}

	void Renderer::ToggleRenderMethod()
	{
		switch (m_CurrentRenderMethod)
		{
		case RenderMethod::Hardware:
			m_CurrentRenderMethod = RenderMethod::Software;
			if (!m_UseUniformColor)
			{
				m_CurrentBGColor = ColorRGB{ .39f,.39f,.39f };
			}
			std::cout << "**(SHARED) Rasterizer Mode = SOFTWARE\n";
			return;
		case RenderMethod::Software:
			m_CurrentRenderMethod = RenderMethod::Hardware;
			if (!m_UseUniformColor)
			{
				m_CurrentBGColor = ColorRGB{ .39f,.59f,.93f };
			}
			std::cout << "**(SHARED) Rasterizer Mode = HARDWARE\n";
			return;

		}
	}

	void Renderer::ToggleUniformClearColor()
	{
		m_UseUniformColor = !m_UseUniformColor;

		if (m_UseUniformColor)
		{
			m_CurrentBGColor = ColorRGB{ .1f,.1f,.1f };
			std::cout << "**(SHARED) Uniform ClearColor ON\n";
		}
		else
		{
			std::cout << "**(SHARED) Uniform ClearColor OFF\n";
			switch (m_CurrentRenderMethod)
			{
			case dae::Renderer::RenderMethod::Hardware:
				m_CurrentBGColor = ColorRGB{ .39f,.59f,.93f };
				break;
			case dae::Renderer::RenderMethod::Software:
				m_CurrentBGColor = ColorRGB{ .39f,.39f,.39f };
				break;

			}
		}
	}

	void Renderer::ToggleRotation()
	{
		m_ShouldRotate = !m_ShouldRotate;

		if (m_ShouldRotate)
		{
			std::cout << "**(SHARED) Vehicle Rotation ON\n";
		}
		else
		{
			std::cout << "**(SHARED) Vehicle Rotation OFF\n";
		}
	}

	bool Renderer::TogglePrintFPS()
	{
		m_ShouldPrintFPS = !m_ShouldPrintFPS;

		if (m_ShouldPrintFPS)
		{
			std::cout << "**(SHARED) Print FPS On\n";
			return true;
		}
		else
		{
			std::cout << "**(SHARED) Print FPS Off\n";
			return false;
		}
	}

	void Renderer::CycleSamplerFilter()
	{
		if (m_CurrentRenderMethod == RenderMethod::Hardware)
		{
			m_pHardwareRasterizer->CycleFilterMode();
		}
	
	}

	void Renderer::CycleShadingMode()
	{
		if (m_CurrentRenderMethod == RenderMethod::Software)
		{
			m_pSoftwareRasterizer->CycleShadingMode();
		}
	}

	void Renderer::ToggleNormalMap()
	{
		if (m_CurrentRenderMethod == RenderMethod::Software)
		{
			m_pSoftwareRasterizer->ToggleNormalMap();
		}
	}

	void Renderer::ToggleDepthBufferVisualisation()
	{
		if (m_CurrentRenderMethod == RenderMethod::Software)
		{
			m_pSoftwareRasterizer->ToggleDepthBuffer();
		}
	}

	void Renderer::PrintInfo()
	{
		std::cout << "[Key Bindings - SHARED]\n";
		std::cout << "\t[F1]\tToggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
		std::cout << "\t[F2]\tToggle Vehicle Rotation (ON/OFF)\n";
		std::cout << "\t[F10]\tToggle Uniform ClearColor (ON/OFF)\n";
		std::cout << "\t[F11]\tToggle Print FPS (ON/OFF)\n\n";

		std::cout << "[Key Bindings - HARDWARE]\n";
		std::cout << "\t[F4]\tCycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n\n";

		std::cout << "[Key Bindings - SOFTWARE]\n";
		std::cout << "\t[F5]\tCycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n";
		std::cout << "\t[F6]\tToggle Normal Map (ON/OFF)\n";
		std::cout << "\t[F7]\tToggle Depth Buffer Visualization (ON/OFF)\n";
		std::cout << "\t[F8]\tToggle BoundingBox Visualization (ON/OFF)\n";
	}

	
}
