#include "pch.h"
#include "Mesh.h"
#include "DataTypes.h"
#include "Camera.h"
#include "Texture.h"
#include "Effect.h"

using namespace dae;

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) :
	m_Vertices{ vertices },
	m_Indices{ indices }
{
	m_WorldMatrix = Matrix::CreateTranslation(m_WorldMatrix.GetTranslation()+ Vector3{ 0.f, 0.f, 50.f });
	m_Vertices_Out.reserve(m_Vertices.size());

	for (size_t i = 0; i < m_Vertices_Out.capacity(); i++)
	{
		m_Vertices_Out.emplace_back(Vertex_Out{});
	}

}
Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	//Create effect
	m_pEffect = new Effect{ pDevice,L"Resources/PosCol3D.fx" };
	if (m_pEffect)
	{
		m_pEffectLocalPointer = m_pEffect->GetEffect();
		m_pTechniqueLocalPointer = m_pEffect->GetTechnique();

	}
	m_WorldMatrix = Matrix::CreateTranslation(m_WorldMatrix.GetTranslation() + Vector3{ 0.f, 0.f, 50.f });
	//Create textures

	Texture* pDiffuse = new Texture{ pDevice, "Resources/vehicle_diffuse.png" };
	Texture* pGloss = new Texture{ pDevice,"Resources/vehicle_gloss.png" };
	Texture* pSpecular = new Texture{ pDevice,"Resources/vehicle_specular.png" };
	Texture* pNormal = new Texture{ pDevice,"Resources/vehicle_normal.png" };

	m_pEffect->SetDiffuseMap(pDiffuse);
	m_pEffect->SetGlossMap(pGloss);
	m_pEffect->SetSpecularMap(pSpecular);
	m_pEffect->SetNormalMap(pNormal);

	delete pDiffuse;
	pDiffuse = nullptr;
	delete pGloss;
	pGloss = nullptr;
	delete pSpecular;
	pSpecular = nullptr;
	delete pNormal;
	pNormal = nullptr;


	//Create Vertex Layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create input layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechniqueLocalPointer->GetPassByIndex(0)->GetDesc(&passDesc);

	const HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
		assert(false);

	//Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	HRESULT result1 = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result1))
		assert(false);

	//Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result1 = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result1))
		assert(false);

}
Mesh::~Mesh()
{
	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();

	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();

	if (m_pInputLayout)
		m_pInputLayout->Release();


	if (m_pEffect)
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}
}
void Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechniqueLocalPointer->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		m_pTechniqueLocalPointer->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}
void Mesh::UpdateMatrices(const Camera& camera)
{
	
	const Matrix worldViewProjection{ m_WorldMatrix * camera.invViewMatrix * camera.projectionMatrix };
	m_pEffect->SetWorldViewProjMatrix(worldViewProjection);
	m_pEffect->SetWorldMatrix(m_WorldMatrix);
	m_pEffect->SetInvViewMatrix(camera.invViewMatrix);
}
void Mesh::CycleFilterMode()
{
	m_pEffect->CycleFilterMode();
	m_pTechniqueLocalPointer = m_pEffect->GetTechnique();
}
void Mesh::TransformVertices(Camera* pCamera,int w, int h)
{
	const Matrix worldViewProjection{m_WorldMatrix * pCamera->invViewMatrix * pCamera->projectionMatrix };
	for (size_t i = 0; i < m_Vertices.size(); i++)
	{
		m_Vertices_Out[i].Position.x = m_Vertices[i].Position.x;
		m_Vertices_Out[i].Position.y = m_Vertices[i].Position.y;
		m_Vertices_Out[i].Position.z = m_Vertices[i].Position.z;
		m_Vertices_Out[i].Uv = m_Vertices[i].Uv;
		m_Vertices_Out[i].Normal = m_WorldMatrix.TransformVector(m_Vertices[i].Normal).Normalized();
		m_Vertices_Out[i].Tangent = m_WorldMatrix.TransformVector(m_Vertices[i].Tangent).Normalized();
		m_Vertices_Out[i].ViewDirection = m_WorldMatrix.TransformPoint(m_Vertices[i].Position) - pCamera->origin;


		m_Vertices_Out[i].Position = worldViewProjection.TransformPoint(m_Vertices_Out[i].Position);


		//Perspective Divide
		const float invW{ 1.f / m_Vertices_Out[i].Position.w };

		m_Vertices_Out[i].Position.x *= invW;
		m_Vertices_Out[i].Position.y *= invW;
		m_Vertices_Out[i].Position.z *= invW;


		m_Vertices_Out[i].Position.x = (m_Vertices_Out[i].Position.x + 1) / 2 * w;
		m_Vertices_Out[i].Position.y = (1 - m_Vertices_Out[i].Position.y) / 2 * h;

	}
}
void Mesh::RotateY(float angle, float deltaTime)
{
	m_WorldMatrix = Matrix::CreateRotationY(angle * TO_RADIANS * deltaTime) * m_WorldMatrix;
}