#pragma once
#include "DataTypes.h"
#include "Matrix.h"

struct Camera;
class Effect;

class Mesh final
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	void Render(ID3D11DeviceContext* pDeviceContext);
	void UpdateMatrices(const Camera& camera);

	void CycleFilterMode();
	void TransformVertices(Camera* pCamera,int w, int h);
	void RotateY(float angle, float deltaTime);

	Matrix GetWorldMatrix()const { return m_WorldMatrix; }
	const std::vector<Vertex_Out>& GetVerticesOut() const { return m_Vertices_Out; }
	const std::vector<uint32_t>& GetIndices()const { return m_Indices; }

private:
	std::vector<Vertex>		m_Vertices;
	std::vector<Vertex_Out>	m_Vertices_Out;
	std::vector<uint32_t>	m_Indices;

	Matrix					m_WorldMatrix;

	Effect* m_pEffect{nullptr};

	ID3DX11Effect* m_pEffectLocalPointer{nullptr};
	ID3DX11EffectTechnique* m_pTechniqueLocalPointer{nullptr};

	ID3D11InputLayout* m_pInputLayout{nullptr};
	ID3D11Buffer* m_pVertexBuffer{nullptr};
	ID3D11Buffer* m_pIndexBuffer{nullptr};
	uint32_t m_NumIndices;
	
};