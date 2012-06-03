#ifndef DT_TEXTURE_LIBRARY_H
#define DT_TEXTURE_LIBRARY_H

#include <EASTL/map.h>

#include <tstring.h>
#include <vector.h>

#include "texturehandle.h"

class CTexture
{
public:
	CTexture()
	{
		m_iReferences = 0; 
		m_iGLID = 0; 
		m_iWidth = 0; 
		m_iHeight = 0; 
	}

public:
	size_t					m_iReferences;

	size_t					m_iGLID;
	size_t					m_iWidth;
	size_t					m_iHeight;
};

class CTextureLibrary
{
public:
							CTextureLibrary();
							~CTextureLibrary();

public:
	static size_t			GetNumTextures() { return Get()->m_aTextures.size(); }

	static CTextureHandle	AddTexture(const tstring& sTexture, int iClamp = 0);
	static CTextureHandle	AddTexture(Vector* vecColors, size_t iWidth, size_t iHeight);
	static CTexture*		AddAsset(const tstring& sTexture, int iClamp = 0);
	static CTexture*		AddAsset(const tstring& sTexture, Vector* vecColors, size_t iWidth, size_t iHeight);
	static CTextureHandle	FindAsset(const tstring& sTexture);
	static size_t			FindTextureID(const tstring& sTexture);

	static size_t			GetTextureGLID(const tstring& sTexture);
	static size_t			GetTextureWidth(const tstring& sTexture);
	static size_t			GetTextureHeight(const tstring& sTexture);

	static size_t			GetNumTexturesLoaded() { return Get()->m_aTextures.size(); };
	static bool				IsAssetLoaded(const tstring& sTexture);

	static void				UnloadTexture(const tstring& sTexture);

	static void				ClearUnreferenced();

public:
	static CTextureLibrary*	Get() { return s_pTextureLibrary; };

protected:
	eastl::map<tstring, CTexture>	m_aTextures;

private:
	static CTextureLibrary*	s_pTextureLibrary;
};

#include <tinker/assethandle_functions.h>

#endif
