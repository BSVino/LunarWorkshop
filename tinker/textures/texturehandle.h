#pragma once

#include <tstring.h>

class CTexture;

// Handy way of accessing a texture
// Increments the ref count of a texture when it's created, decrements when it's destroyed
class CTextureHandle
{
public:
						CTextureHandle();
						CTextureHandle(const tstring& sTexture, const CTexture* pTexture = nullptr);
						CTextureHandle(const CTextureHandle& c);
						~CTextureHandle();

public:
	bool				IsValid() const;
	tstring				GetTextureName() const { return m_sTexture; }
	size_t				GetID() const;
	const CTexture*		GetTexture() const;

	void				Reset();

	const CTextureHandle& operator=(const CTextureHandle& c);

	inline operator const CTexture*() const
	{
		return GetTexture();
	}

	inline const CTexture* operator->() const
	{
		return GetTexture();
	}

protected:
	tstring				m_sTexture;
	const CTexture*		m_pTexture;
};
