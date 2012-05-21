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

CMaterialHandle CMaterialLibrary::AddMaterial(const tstring& sMaterial, int iClamp)
{
	return CMaterialHandle(sMaterial, AddAsset(sMaterial, iClamp));
}

CMaterialHandle CMaterialLibrary::AddMaterial(const class CData* pData, const tstring& sMaterial)
{
	CMaterialHandle hMaterial = FindAsset(sMaterial);
	if (hMaterial.IsValid())
		return hMaterial;

	CMaterial* pMaterial = CreateMaterial(pData, sMaterial);
	return CMaterialHandle(pMaterial->m_sFile, pMaterial);
}

CMaterial* CMaterialLibrary::AddAsset(const tstring& sMaterial, int iClamp)
{
	if (!sMaterial.length())
		return nullptr;

	if (!tstr_endswith(sMaterial, ".mat"))
		return nullptr;

	std::basic_ifstream<tchar> f(sMaterial.c_str());

	if (!f.is_open())
		return nullptr;

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	return CreateMaterial(pData.get(), sMaterial);
}

CMaterial* CMaterialLibrary::CreateMaterial(const CData* pData, const tstring& sMaterialOutput)
{
	tstring sMaterial = ToForwardSlashes(sMaterialOutput);
	if (!sMaterialOutput.length())
	{
		static int i = 0;
		sMaterial = sprintf("[from data %x]", i++);
	}

	CData* pShaderData = pData->FindChild("Shader");
	if (!pShaderData)
	{
		TError("Material file with no shader: " + sMaterial + "\n");
		return nullptr;
	}

	CShader* pShader = CShaderLibrary::GetShader(pShaderData->GetValueTString());
	TAssert(pShader);

	if (!pShader)
	{
		TError("Material file with invalid shader: " + sMaterial + "\n");
		return nullptr;
	}

	CMaterial& oMat = Get()->m_aMaterials[sMaterial];
	oMat.Clear();

	oMat.m_sFile = sMaterial;

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

		if (it->second.m_sBlend.length())
		{
			oMat.m_sBlend = it->second.m_sBlend;
			if (oMat.m_sBlend == "[value]")
				oMat.m_sBlend = pParameter->GetValueTString();
			continue;
		}

		CMaterial::CParameter& oPar = oMat.m_aParameters.push_back();
		oPar.m_sName = sParameter;

		FillParameter(oMat, oMat.m_aParameters.size()-1, pShader, pParameter);
	}

	for (size_t i = 0; i < oMat.m_ahTextures.size(); i++)
	{
		if (!oMat.m_ahTextures[i].IsValid())
			TError("Material file '" + oMat.m_sFile + "' has a texture that couldn't be found: " + pShader->m_asTextures[i] + "\n");
	}

	return &oMat;
}

CMaterialHandle CMaterialLibrary::FindAsset(const tstring& sMaterial)
{
	tstring sMaterialForward = ToForwardSlashes(sMaterial);
	eastl::map<tstring, CMaterial>::iterator it = Get()->m_aMaterials.find(sMaterialForward);
	if (it == Get()->m_aMaterials.end())
		return CMaterialHandle();

	return CMaterialHandle(sMaterialForward, &it->second);
}

bool CMaterialLibrary::IsAssetLoaded(const tstring& sMaterial)
{
	if (!Get())
		return false;

	tstring sMaterialForward = ToForwardSlashes(sMaterial);
	eastl::map<tstring, CMaterial>::iterator it = Get()->m_aMaterials.find(sMaterialForward);
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

void CMaterialLibrary::FillParameter(CMaterial& oMat, size_t iPar, CShader* pShader, const class CData* pData)
{
	CMaterial::CParameter& oPar = oMat.m_aParameters[iPar];

	auto it = pShader->m_aParameters.find(oPar.m_sName);
	TAssert(it != pShader->m_aParameters.end());
	if (it == pShader->m_aParameters.end())
	{
		TError("Material file has a property that's not in the shader: " + oMat.m_sFile + "\n");
		return;
	}

	CShader::CParameter* pShaderPar = &it->second;

	oPar.m_sValue = pData->GetValueTString();

	tstring sType;
	for (size_t j = 0; j < pShaderPar->m_aActions.size(); j++)
		if (pShaderPar->m_aActions[j].m_sValue == "[value]")
			sType = pShader->m_asUniforms[pShaderPar->m_aActions[j].m_sName];

	TAssert(sType.length());

	if (sType == "float")
		oPar.m_flValue = pData->GetValueFloat();
	else if (sType == "vec2")
		oPar.m_vec2Value = pData->GetValueVector2D();
	else if (sType == "vec3")
		oPar.m_vecValue = pData->GetValueVector();
	else if (sType == "vec4")
		oPar.m_vec4Value = pData->GetValueVector4D();
	else if (sType == "int")
		oPar.m_iValue = pData->GetValueInt();
	else if (sType == "bool")
		oPar.m_bValue = pData->GetValueBool();
	else if (sType == "mat4")
	{
		TUnimplemented();
	}
	else if (sType == "sampler2D")
	{
		// No op. Texture is read below.
	}
	else
		TUnimplemented();

	for (size_t j = 0; j < pShaderPar->m_aActions.size(); j++)
	{
		for (size_t k = 0; k < pShader->m_asTextures.size(); k++)
		{
			if (pShader->m_asTextures[k] == pShaderPar->m_aActions[j].m_sName)
			{
				oMat.m_ahTextures[k] = CTextureHandle(pData->GetValueTString());
				if (!oMat.m_ahTextures[k].IsValid())
					oMat.m_ahTextures[k] = CTextureHandle(GetDirectory(oMat.m_sFile) + "/" + oPar.m_sValue);
			}
		}
	}
}

void CMaterial::Save() const
{
	if (m_sFile.substr(strlen("[from data ")) == "[from data ")
	{
		TAssert(m_sFile.substr(strlen("[from data ")) != "[from data ");
		return;
	}

	std::basic_ofstream<tchar> f(m_sFile.c_str());

	TAssert(f.is_open());
	if (!f.is_open())
		return;

	tstring sShader = "Shader: " + m_sShader + "\n{\n";
	f.write(sShader.data(), sShader.length());

	if (m_sBlend.length())
	{
		tstring sOut = "\tBlend: " + m_sBlend + "\n";
		f.write(sOut.data(), sOut.length());
	}

	for (size_t i = 0; i < m_aParameters.size(); i++)
	{
		tstring sOut = "\t" + m_aParameters[i].m_sName + ": " + m_aParameters[i].m_sValue + "\n";
		f.write(sOut.data(), sOut.length());
	}

	tstring sEnd = "}\n";
	f.write(sEnd.data(), sEnd.length());
}

void CMaterial::Reload()
{
	std::basic_ifstream<tchar> f(m_sFile.c_str());

	TAssert(f.is_open());
	if (!f.is_open())
		return;

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CMaterialLibrary::CreateMaterial(pData.get(), m_sFile);
}
