#pragma once
class Texture;

static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& path);
	~Effect();

	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	void SetWorldViewProjMatrix(const dae::Matrix& worldViewProjection);
	void SetWorldMatrix(const dae::Matrix& worldMatrix);
	void SetInvViewMatrix(const dae::Matrix& worldMatrix);


	void SetDiffuseMap(Texture* pDiffuseTexture);
	void SetNormalMap(Texture* pNormalMap);
	void SetGlossMap(Texture* pGloss);
	void SetSpecularMap(Texture* pSpecular);

	void CycleFilterMode();


	ID3DX11Effect* GetEffect()		const { return m_pEffect; };
	ID3DX11EffectTechnique* GetTechnique()	const;


private:
	enum class FilterMode
	{
		Point, Linear, Anisotropic
	};


	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pPointTechnique;
	ID3DX11EffectTechnique* m_pLinearTechnique;
	ID3DX11EffectTechnique* m_pAnisotropicTechnique;


	ID3DX11EffectMatrixVariable* m_pMatWorldViewProj;
	ID3DX11EffectMatrixVariable* m_pMatWorld;
	ID3DX11EffectMatrixVariable* m_pMatViewInv;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVar;
	ID3DX11EffectShaderResourceVariable* m_pGlossMapVar;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVar;

	FilterMode m_CurrentFilterMode;

};