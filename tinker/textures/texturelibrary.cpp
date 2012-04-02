#include "texturelibrary.h"

#include <tinker/shell.h>
#include <renderer/renderer.h>

CTextureLibrary* CTextureLibrary::s_pTextureLibrary = NULL;
static CTextureLibrary g_TextureLibrary = CTextureLibrary();

CTextureLibrary::CTextureLibrary()
{
	s_pTextureLibrary = this;
}

CTextureLibrary::~CTextureLibrary()
{
	for (eastl::map<tstring, CTexture>::iterator it = m_aTextures.begin(); it != m_aTextures.end(); it++)
		CRenderer::UnloadTextureFromGL(it->second.m_iGLID);

	s_pTextureLibrary = NULL;
}

const CTexture* CTextureLibrary::AddTexture(const tstring& sTexture, int iClamp)
{
	if (!sTexture.length())
		return NULL;

	const CTexture* pTexture = FindTexture(sTexture);
	if (pTexture != NULL)
	{
		Get()->m_aTextures[sTexture].m_iReferences++;
		return pTexture;
	}

	int w, h;
	Color* pclrTexture = CRenderer::LoadTextureData(sTexture, w, h);
	if (!pclrTexture)
		return NULL;

	CTexture oTex;
	
	oTex.m_iGLID = CRenderer::LoadTextureIntoGL(pclrTexture, w, h, iClamp);
	oTex.m_iWidth = w;
	oTex.m_iHeight = h;

	CRenderer::UnloadTextureData(pclrTexture);

	Get()->m_aTextures[sTexture] = oTex;
	Get()->m_aTextures[sTexture].m_iReferences++;

	return &Get()->m_aTextures[sTexture];
}

size_t CTextureLibrary::AddTextureID(const tstring& sTexture, int iClamp)
{
	const CTexture* pTex = AddTexture(sTexture, iClamp);

	if (!pTex)
		return 0;

	return pTex->m_iGLID;
}

const CTexture* CTextureLibrary::FindTexture(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return NULL;

	return &it->second;
}

size_t CTextureLibrary::FindTextureID(const tstring& sTexture)
{
	const CTexture* pTex = FindTexture(sTexture);

	if (!pTex)
		return 0;

	return pTex->m_iGLID;
}

tstring CTextureLibrary::FindTextureByID(size_t iID)
{
	for (auto it = Get()->m_aTextures.begin(); it != Get()->m_aTextures.end(); it++)
	{
		if (it->second.m_iGLID == iID)
			return it->first;
	}

	return "";
}

void CTextureLibrary::ReleaseTexture(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return;

	TAssert(it->second.m_iReferences > 0);
	it->second.m_iReferences--;
}

size_t CTextureLibrary::GetTextureGLID(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return ~0;

	return it->second.m_iGLID;
}

size_t CTextureLibrary::GetTextureWidth(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return 0;

	return it->second.m_iWidth;
}

size_t CTextureLibrary::GetTextureHeight(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return 0;

	return it->second.m_iHeight;
}

void CTextureLibrary::UnloadTexture(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return;

	TAssert(it->second.m_iReferences == 0);

	CRenderer::UnloadTextureFromGL(it->second.m_iGLID);
}

void CTextureLibrary::ResetReferenceCounts()
{
	for (auto it = Get()->m_aTextures.begin(); it != Get()->m_aTextures.end(); it++)
		it->second.m_iReferences = 0;
}

void CTextureLibrary::ClearUnreferenced()
{
	for (auto it = Get()->m_aTextures.begin(); it != Get()->m_aTextures.end();)
	{
		if (!it->second.m_iReferences)
		{
			UnloadTexture(it->first);
			Get()->m_aTextures.erase(it++);
		}
		else
			it++;
	}
}

