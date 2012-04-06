#include "texturehandle.h"

#include "texturelibrary.h"

CTextureHandle::CTextureHandle()
{
	m_pTexture = nullptr;
}

CTextureHandle::CTextureHandle(const tstring& sTexture, const CTexture* pTexture)
{
	m_sTexture = sTexture;
	if (pTexture)
		m_pTexture = pTexture;
	else
		m_pTexture = CTextureLibrary::AddTexture(sTexture).GetTexture();

	if (m_pTexture)
		const_cast<CTexture*>(m_pTexture)->m_iReferences++;
}

CTextureHandle::CTextureHandle(const CTextureHandle& c)
{
	m_sTexture = c.m_sTexture;
	if (c.m_pTexture)
	{
		m_pTexture = c.m_pTexture;
		const_cast<CTexture*>(m_pTexture)->m_iReferences++;
	}
	else
		m_pTexture = nullptr;
}

CTextureHandle::~CTextureHandle()
{
	CTextureLibrary::ReleaseTexture(m_pTexture);
}

bool CTextureHandle::IsValid() const
{
	if (!m_pTexture)
		return false;

#ifdef _DEBUG
	bool bLoaded = CTextureLibrary::IsTextureLoaded(m_sTexture);
	TAssertNoMsg(bLoaded);
	if (!bLoaded)
		return false;
#endif

	return true;
}

size_t CTextureHandle::GetID() const
{
	if (!IsValid())
		return 0;

	return m_pTexture->m_iGLID;
}

const CTexture* CTextureHandle::GetTexture() const
{
	if (!IsValid())
		return nullptr;

	return m_pTexture;
}

void CTextureHandle::Reset()
{
	if (!IsValid())
		return;

	CTextureLibrary::ReleaseTexture(m_pTexture);
	m_pTexture = nullptr;
}

const CTextureHandle& CTextureHandle::operator=(const CTextureHandle& c)
{
	m_sTexture = c.m_sTexture;
	if (c.m_pTexture)
	{
		m_pTexture = c.m_pTexture;
		const_cast<CTexture*>(m_pTexture)->m_iReferences++;
	}
	else
		m_pTexture = nullptr;

	return *this;
}
