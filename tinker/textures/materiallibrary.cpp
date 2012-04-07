#include "materiallibrary.h"

#include <files.h>

#include <tinker/shell.h>
#include <renderer/renderer.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>
#include <renderer/shaders.h>

CMaterialLibrary* CMaterialLibrary::s_pMaterialLibrary = NULL;
static CMaterialLibrary g_MaterialLibrary = CMaterialLibrary();

CMaterialLibrary::CMaterialLibrary()
{
	s_pMaterialLibrary = this;
}

CMaterialLibrary::~CMaterialLibrary()
{
	s_pMaterialLibrary = NULL;
}

CMaterialHandle CMaterialLibrary::AddAsset(const tstring& sMaterial, int iClamp)
{
	if (!sMaterial.length())
		return CMaterialHandle();

	if (sMaterial.substr(sMaterial.length()-4) != ".mat")
		return CMaterialHandle();

	CMaterialHandle hMaterial = FindMaterial(sMaterial);
	if (hMaterial.IsValid())
		return hMaterial;

	std::basic_ifstream<tchar> f(sMaterial.c_str());

	if (!f.is_open())
		return CMaterialHandle();

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	return CreateMaterial(pData.get(), sMaterial);
}

CMaterialHandle CMaterialLibrary::CreateMaterial(const CData* pData, const tstring& sMaterialOutput)
{
	tstring sMaterial = sMaterialOutput;
	if (!sMaterialOutput.length())
		sMaterial = "[from data]";

	CData* pShaderData = pData->FindChild("Shader");
	if (!pShaderData)
	{
		TError("Material file with no shader: " + sMaterial + "\n");
		return CMaterialHandle();
	}

	CShader* pShader = CShaderLibrary::GetShader(pShaderData->GetValueTString());
	TAssert(pShader);

	if (!pShader)
	{
		TError("Material file with invalid shader: " + sMaterial + "\n");
		return CMaterialHandle();
	}

	CMaterial oMat;

	oMat.m_sShader = pShaderData->GetValueTString();
	oMat.m_ahTextures.resize(pShader->m_asTextures.size());

	for (size_t i = 0; i < pShaderData->GetNumChildren(); i++)
	{
		CData* pParameter = pShaderData->GetChild(i);
		tstring sParameter = pParameter->GetKey();

		auto it = pShader->m_aParameters.find(sParameter);
		TAssert(it != pShader->m_aParameters.end());
		if (it == pShader->m_aParameters.end())
		{
			TError("Material file has a property that's not in the shader: " + sMaterial + "\n");
			continue;
		}

		CMaterial::CParameter& oPar = oMat.m_aParameters.push_back();
		oPar.m_sName = sParameter;
		oPar.m_sValue = pParameter->GetValueTString();

		tstring sType;
		for (size_t j = 0; j < it->second.m_aActions.size(); j++)
			if (it->second.m_aActions[j].m_sValue == "[value]")
				sType = pShader->m_asUniforms[it->second.m_aActions[j].m_sName];

		TAssert(sType.length());

		if (sType == "float")
			oPar.m_flValue = pParameter->GetValueFloat();
		else if (sType == "vec2")
			oPar.m_vec2Value = pParameter->GetValueVector2D();
		else if (sType == "vec3")
			oPar.m_vecValue = pParameter->GetValueVector();
		else if (sType == "vec4")
			oPar.m_vec4Value = pParameter->GetValueVector4D();
		else if (sType == "int")
			oPar.m_iValue = pParameter->GetValueInt();
		else if (sType == "bool")
			oPar.m_bValue = pParameter->GetValueBool();
		else if (sType == "mat4")
		{
			TAssert(false); // Unimplemented
		}
		else if (sType == "sampler2D")
		{
			// No op. Texture is read below.
		}
		else
			TAssert(false);

		for (size_t j = 0; j < it->second.m_aActions.size(); j++)
		{
			for (size_t k = 0; k < pShader->m_asTextures.size(); k++)
			{
				if (pShader->m_asTextures[k] == it->second.m_aActions[j].m_sName)
				{
					oMat.m_ahTextures[k] = CTextureHandle(pParameter->GetValueTString());
					if (!oMat.m_ahTextures[k].IsValid())
						oMat.m_ahTextures[k] = CTextureHandle(GetDirectory(sMaterial) + "/" + oPar.m_sValue);
					TAssert(oMat.m_ahTextures[k].IsValid());
					if (!oMat.m_ahTextures[k].IsValid())
						TError("Material file '" + sMaterial + "' has a texture that couldn't be found: " + sParameter + ": " + pParameter->GetValueTString() + "\n");
				}
			}
		}
	}

	Get()->m_aMaterials[sMaterial] = oMat;

	return CMaterialHandle(sMaterial, &Get()->m_aMaterials[sMaterial]);
}

CMaterialHandle CMaterialLibrary::FindMaterial(const tstring& sMaterial)
{
	eastl::map<tstring, CMaterial>::iterator it = Get()->m_aMaterials.find(sMaterial);
	if (it == Get()->m_aMaterials.end())
		return CMaterialHandle();

	return CMaterialHandle(sMaterial, &it->second);
}

void CMaterialLibrary::ReleaseMaterial(const tstring& sMaterial)
{
	eastl::map<tstring, CMaterial>::iterator it = Get()->m_aMaterials.find(sMaterial);
	if (it == Get()->m_aMaterials.end())
		return;

	TAssert(it->second.m_iReferences > 0);
	it->second.m_iReferences--;
}

void CMaterialLibrary::ReleaseAsset(const CMaterial* pMaterial)
{
	if (!pMaterial)
		return;

	CMaterial* pMaterialNonConst = const_cast<CMaterial*>(pMaterial);

	TAssert(pMaterialNonConst->m_iReferences > 0);
	if (pMaterialNonConst->m_iReferences > 0)
		pMaterialNonConst->m_iReferences--;
}

bool CMaterialLibrary::IsAssetLoaded(const tstring& sMaterial)
{
	eastl::map<tstring, CMaterial>::iterator it = Get()->m_aMaterials.find(sMaterial);
	if (it == Get()->m_aMaterials.end())
		return false;

	return true;
}

void CMaterialLibrary::ClearUnreferenced()
{
	for (auto it = Get()->m_aMaterials.begin(); it != Get()->m_aMaterials.end();)
	{
		if (!it->second.m_iReferences)
			Get()->m_aMaterials.erase(it++);
		else
			it++;
	}
}
