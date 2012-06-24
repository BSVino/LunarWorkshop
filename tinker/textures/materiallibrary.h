#pragma once

#include <tmap.h>

#include <tstring.h>
#include <vector.h>
#include <matrix.h>

#include <tinker/renderer/shaders.h>

#include "materialhandle.h"
#include "texturehandle.h"
#include "texturelibrary.h"

class CMaterial
{
public:
	CMaterial()
	{
		m_iReferences = 0; 
	}

public:
	void Clear()
	{
		m_sFile = "";
		m_sShader = "";
		m_sBlend = "";
		m_ahTextures.clear();
		m_aParameters.clear();

		// Don't clear references, there's probably open handles to this material.
	}

	void			Save() const;
	void			Reload();

	size_t			FindParameter(const tstring& sParameterName, bool bCreate = false);
	void			SetParameter(const tstring& sParameterName, const CTextureHandle& hTexture);
	void			FillParameter(size_t iParameter, const tstring& sData, class CShader* pShader=nullptr);

public:
	tstring			m_sFile;

	size_t			m_iReferences;

	tstring			m_sShader;
	tstring			m_sBlend;

	class CParameter
	{
	public:
		tstring		m_sName;
		union
		{
			float		m_flValue;
			int			m_iValue;
			bool		m_bValue;
		};
		Vector2D	m_vec2Value;
		Vector		m_vecValue;
		Vector4D	m_vec4Value;
		tstring		m_sValue;

		tstring		m_sType;

		CShader::CParameter*	m_pShaderParameter;

	public:
		void		SetValue(const tstring& sValue, class CShader* pShader);
	};

	tvector<CParameter>		m_aParameters;
	tvector<CTextureHandle>	m_ahTextures;

	class CShader*			m_pShader;
};

class CMaterialLibrary
{
public:
							CMaterialLibrary();
							~CMaterialLibrary();

public:
	static size_t			GetNumMaterials() { return Get()->m_aMaterials.size(); }

	static CMaterialHandle	AddMaterial(const tstring& sMaterial, int iClamp = 0);
	static CMaterialHandle	AddMaterial(const class CData* pData, const tstring& sMaterial="");
	static CMaterial*		AddAsset(const tstring& sMaterial, int iClamp = 0);
	static CMaterial*		CreateMaterial(const class CData* pData, const tstring& sMaterial="");
	static CMaterialHandle	FindAsset(const tstring& sMaterial);

	static size_t			GetNumMaterialsLoaded() { return Get()->m_aMaterials.size(); };
	static bool				IsAssetLoaded(const tstring& sMaterial);

	static void				ClearUnreferenced();

	static CMaterialLibrary*	Get() { return s_pMaterialLibrary; };

protected:
	tmap<tstring, CMaterial>	m_aMaterials;

private:
	static CMaterialLibrary*	s_pMaterialLibrary;
};

#include <tinker/assethandle_functions.h>
