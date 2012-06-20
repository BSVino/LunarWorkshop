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
	for (tmap<tstring, CTexture>::iterator it = m_aTextures.begin(); it != m_aTextures.end(); it++)
		CRenderer::UnloadTextureFromGL(it->second.m_iGLID);

	s_pTextureLibrary = NULL;
}

CTextureHandle CTextureLibrary::AddTexture(const tstring& sTexture, int iClamp)
{
	CTextureHandle hTexture = FindAsset(sTexture);
	if (hTexture.IsValid())
		return hTexture;

	return CTextureHandle(sTexture, AddAsset(sTexture, iClamp));
}

CTextureHandle CTextureLibrary::AddTexture(Vector* vecColors, size_t iWidth, size_t iHeight)
{
	static int i = 0;
	tstring sName = sprintf("[generated vector texture %d]", i++);
	return CTextureHandle(sName, AddAsset(sName, vecColors, iWidth, iHeight));
}

CTexture* CTextureLibrary::AddAsset(const tstring& sTexture, int iClamp)
{
	if (!sTexture.length())
		return nullptr;

	int w, h;
	Color* pclrTexture = CRenderer::LoadTextureData(sTexture, w, h);
	if (!pclrTexture)
		return nullptr;

	CTexture oTex;
	
	oTex.m_iGLID = CRenderer::LoadTextureIntoGL(pclrTexture, w, h, iClamp);
	oTex.m_iWidth = w;
	oTex.m_iHeight = h;

	CRenderer::UnloadTextureData(pclrTexture);

	Get()->m_aTextures[sTexture] = oTex;

	return &Get()->m_aTextures[sTexture];
}

CTexture* CTextureLibrary::AddAsset(const tstring& sTexture, Vector* vecColors, size_t iWidth, size_t iHeight)
{
	if (!vecColors)
		return nullptr;

	CTexture oTex;
	
	oTex.m_iGLID = CRenderer::LoadTextureIntoGL(vecColors, iWidth, iHeight);
	oTex.m_iWidth = iWidth;
	oTex.m_iHeight = iHeight;

	Get()->m_aTextures[sTexture] = oTex;

	return &Get()->m_aTextures[sTexture];
}

CTextureHandle CTextureLibrary::FindAsset(const tstring& sTexture)
{
	tmap<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return CTextureHandle();

	return CTextureHandle(sTexture, &it->second);
}

size_t CTextureLibrary::FindTextureID(const tstring& sTexture)
{
	return FindAsset(sTexture)->m_iGLID;
}

size_t CTextureLibrary::GetTextureGLID(const tstring& sTexture)
{
	tmap<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return ~0;

	return it->second.m_iGLID;
}

size_t CTextureLibrary::GetTextureWidth(const tstring& sTexture)
{
	tmap<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return 0;

	return it->second.m_iWidth;
}

size_t CTextureLibrary::GetTextureHeight(const tstring& sTexture)
{
	tmap<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return 0;

	return it->second.m_iHeight;
}

bool CTextureLibrary::IsAssetLoaded(const tstring& sTexture)
{
	if (!Get())
		return false;

	tmap<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return false;

	return true;
}

void CTextureLibrary::UnloadTexture(const tstring& sTexture)
{
	tmap<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return;

	TAssert(it->second.m_iReferences == 0);

	CRenderer::UnloadTextureFromGL(it->second.m_iGLID);
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
