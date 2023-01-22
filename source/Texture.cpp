#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

using namespace dae;

Texture::Texture(SDL_Surface* pSurface) :
	m_pSurface{ pSurface },
	m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
{
}

Texture::Texture(ID3D11Device* pDevice, const std::string& path)
{
	m_pSurface = IMG_Load(path.c_str());
	m_pSurfacePixels = (uint32_t*)m_pSurface->pixels;

	/**Texture*/
	D3D11_TEXTURE2D_DESC desc{}; //Describes a 2d Texture
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1; //How many Downsized version of texture
	desc.ArraySize = 1; //How many textures in texture array
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT; //Read write access by GPU
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subResData{};
	subResData.pSysMem = m_pSurface->pixels; //Pointer to init data
	subResData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);  //Distance in bytes 
	subResData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch); //Doc says this is only for 3D textures, but whatever

	HRESULT result{ pDevice->CreateTexture2D(&desc, &subResData, &m_pTexture) };

	if (FAILED(result))
	{
		assert(false && "Couldn't create 2D texture");
	}

	/**Resource view*/
	D3D11_SHADER_RESOURCE_VIEW_DESC rvDesc{};
	rvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; //Must be the same as resource type
	rvDesc.Texture2D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pTexture, &rvDesc, &m_pResourceView);
	if (FAILED(result))
	{
		assert(false && "Couldn't create Resource view");
	}

	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}
}

Texture::~Texture()
{
	if(m_pResourceView)
		m_pResourceView->Release();

	if(m_pTexture)
		m_pTexture->Release();

	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}
}

Texture* Texture::LoadFromFile(const std::string& path)
{
	SDL_Surface* surface = IMG_Load(path.c_str());
	Texture* texture{ new Texture{surface} };

	return texture;
}

ColorRGB Texture::Sample(const Vector2& uv) const
{
	const int nrColors{ 3 };
	Uint8 rgb[nrColors];
	int idx{ static_cast<int>(uv.x * m_pSurface->w) + (static_cast<int>(uv.y * m_pSurface->h) * m_pSurface->w) };

	Uint32 pixel{ m_pSurfacePixels[idx] };
	SDL_GetRGB(pixel, m_pSurface->format, &rgb[0], &rgb[1], &rgb[2]);


	return ColorRGB{ static_cast<float>(rgb[0] / 255.0f),static_cast<float>(rgb[1] / 255.0f) ,static_cast<float>(rgb[2] / 255.0f) };
}