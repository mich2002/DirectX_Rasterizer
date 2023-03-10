#include "pch.h"
#include "MeshEffect.h"
#include "Texture.h"

MeshEffect::MeshEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: Effect(pDevice, assetFile)
{
	InitMatrixVariables();
	InitShaderResourceVariables();
}

MeshEffect::~MeshEffect()
{
	if (m_pGlossinessMapVariable) m_pGlossinessMapVariable->Release();
	if (m_pSpecularMapVariable) m_pSpecularMapVariable->Release();
	if (m_pNormalMapVariable) m_pNormalMapVariable->Release();

	if (m_pMatInvViewVariable) m_pMatInvViewVariable->Release();
	if (m_pMatWorldVariable) m_pMatWorldVariable->Release();
}

void MeshEffect::SetNormalMap(const std::string& filePath, ID3D11Device* pDevice) const
{
	const Texture* normalTexture = Texture::LoadFromFile(filePath, pDevice);

	if (m_pNormalMapVariable)
		m_pNormalMapVariable->SetResource(normalTexture->GetShaderResourceView());

	delete normalTexture;
}

void MeshEffect::SetSpecularMap(const std::string& filePath, ID3D11Device* pDevice) const
{
	const Texture* specularTexture = Texture::LoadFromFile(filePath, pDevice);

	if (m_pSpecularMapVariable)
		m_pSpecularMapVariable->SetResource(specularTexture->GetShaderResourceView());

	delete specularTexture;

}

void MeshEffect::SetGlossinessMap(const std::string& filePath, ID3D11Device* pDevice) const
{
	const Texture* glossinessTexture = Texture::LoadFromFile(filePath, pDevice);

	if (m_pGlossinessMapVariable)
		m_pGlossinessMapVariable->SetResource(glossinessTexture->GetShaderResourceView());

	delete glossinessTexture;

}

void MeshEffect::InitMatrixVariables()
{
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (m_pMatWorldVariable->IsValid() == false)
		std::wcout << L"m_pMatWorldVariable not valid!\n";

	m_pMatInvViewVariable = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();
	if (m_pMatInvViewVariable->IsValid() == false)
		std::wcout << L"m_pMatInvViewVariable not valid!\n";
}

void MeshEffect::InitShaderResourceVariables()
{
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"m_pNormalMapVariable not valid!\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"m_pSpecularMapVariable not valid!\n";

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"m_pGlossinessMapVariable not valid!\n";

}
