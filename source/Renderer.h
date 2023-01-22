#pragma once
#include "Camera.h"
struct SDL_Window;
struct SDL_Surface;
class Rasterizer_Software;
class Rasterizer_Hardware;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleRenderMethod();
		void ToggleUniformClearColor();
		void ToggleRotation();
		bool TogglePrintFPS();
		void CycleSamplerFilter();
		void CycleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBufferVisualisation();
		void ToggleBoundingBoxVisualisation();

	private:
		enum class RenderMethod
		{
			Hardware, Software
		};

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsDirectXInitialized{ false };
		bool m_IsSoftwareInitialized{ false };
		bool m_UseUniformColor{ false };
		bool m_ShouldRotate{ true };
		bool m_ShouldPrintFPS{ false };

		Camera m_Camera{};
		Rasterizer_Software* m_pSoftwareRasterizer;
		Rasterizer_Hardware* m_pHardwareRasterizer;

		RenderMethod m_CurrentRenderMethod;
		ColorRGB m_CurrentBGColor;

		void PrintInfo();


	};
}
