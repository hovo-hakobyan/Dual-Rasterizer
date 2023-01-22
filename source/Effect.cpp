#include "pch.h"
#include "Effect.h"
#include "Texture.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& path)
{
	m_pEffect = LoadEffect(pDevice, path);
	if (m_pEffect)
	{
		m_pPointTechnique = m_pEffect->GetTechniqueByName("PointFilterTechnique");
		m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearFilterTechnique");
		m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicFilterTechnique");
	}

	if (!m_pPointTechnique->IsValid() || !m_pLinearTechnique->IsValid() || !m_pAnisotropicTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}

	m_pMatWorldViewProj = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProj->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
	}

	m_pDiffuseMapVar = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVar->IsValid())
	{
		std::wcout << L"Diffuse map variable not valid\n";
	}

	m_CurrentFilterMode = FilterMode::Point;

	m_pMatWorld = m_pEffect->GetVariableBySemantic("World")->AsMatrix();
	if (!m_pMatWorld->IsValid())
	{
		std::wcout << L"MatWorld not valid!\n";
	}

	m_pMatViewInv = m_pEffect->GetVariableBySemantic("ViewInverse")->AsMatrix();
	if (!m_pMatViewInv->IsValid())
	{
		std::wcout << L"InvView not valid!\n";
	}

	m_pSpecularMapVar = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVar->IsValid())
	{
		std::wcout << L"Specular map variable not valid\n";
	}

	m_pGlossMapVar = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossMapVar->IsValid())
	{
		std::wcout << L"Gloss map variable not valid\n";
	}

	m_pNormalMapVar = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVar->IsValid())
	{
		std::wcout << L"Normal map variable not valid\n";
	}
}

Effect::~Effect()
{
	if (m_pNormalMapVar)
		m_pNormalMapVar->Release();

	if (m_pGlossMapVar)
		m_pGlossMapVar->Release();

	if (m_pSpecularMapVar)
		m_pSpecularMapVar->Release();

	if (m_pMatViewInv)
		m_pMatViewInv->Release();

	if (m_pMatWorld)
		m_pMatWorld->Release();

	if (m_pDiffuseMapVar)
		m_pDiffuseMapVar->Release();

	if (m_pMatWorldViewProj)
		m_pMatWorldViewProj->Release();

	if (m_pAnisotropicTechnique)
		m_pAnisotropicTechnique->Release();

	if (m_pLinearTechnique)
		m_pLinearTechnique->Release();

	if (m_pPointTechnique)
		m_pPointTechnique->Release();

	if (m_pEffect)
		m_pEffect->Release();
}

void Effect::SetWorldViewProjMatrix(const dae::Matrix& worldViewProjection)
{
	m_pMatWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProjection));
}

void Effect::SetWorldMatrix(const dae::Matrix& worldMatrix)
{
	m_pMatWorld->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}

void Effect::SetInvViewMatrix(const dae::Matrix& worldMatrix)
{
	m_pMatViewInv->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}

void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVar)
	{
		m_pDiffuseMapVar->SetResource(pDiffuseTexture->GetRV());

	}
}

void Effect::SetNormalMap(Texture* pNormalMap)
{
	if (m_pNormalMapVar)
	{
		m_pNormalMapVar->SetResource(pNormalMap->GetRV());
	}

}

void Effect::SetGlossMap(Texture* pGloss)
{
	if (m_pGlossMapVar)
	{
		m_pGlossMapVar->SetResource(pGloss->GetRV());
	}
}

void Effect::SetSpecularMap(Texture* pSpecular)
{
	if (m_pSpecularMapVar)
	{
		m_pSpecularMapVar->SetResource(pSpecular->GetRV());
	}
}

void Effect::CycleFilterMode()
{
	switch (m_CurrentFilterMode)
	{
	case FilterMode::Point:
		std::wcout << "**(HARDWARE) Sampler Filter = Linear\n";
		m_CurrentFilterMode = FilterMode::Linear;
		break;
	case FilterMode::Linear:
		std::wcout << "**(HARDWARE) Sampler Filter = Anisotropic\n";
		m_CurrentFilterMode = FilterMode::Anisotropic;
		break;
	case FilterMode::Anisotropic:
		std::wcout << "**(HARDWARE) Sampler Filter = Point\n";
		m_CurrentFilterMode = FilterMode::Point;
		break;
	}
}

ID3DX11EffectTechnique* Effect::GetTechnique() const
{
	switch (m_CurrentFilterMode)
	{
	case FilterMode::Point:
		return m_pPointTechnique;
	case FilterMode::Linear:
		return m_pLinearTechnique;
	case FilterMode::Anisotropic:
		return m_pAnisotropicTechnique;
	}

	return nullptr;
}

static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect{ nullptr };

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << '\n';
		}
		else
		{
			std::wstringstream ss;
			ss << L"EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << '\n';
			return nullptr;
		}
	}

	return pEffect;
}
