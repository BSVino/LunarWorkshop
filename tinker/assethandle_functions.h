#pragma once

template <class C, class L>
CAssetHandle<C, L>::CAssetHandle(const tstring& sName, const C* pAsset = nullptr)
{
	m_sName = sName;
	if (pAsset)
		m_pAsset = pAsset;
	else
		m_pAsset = L::AddAsset(sName).GetAsset();

	if (m_pAsset)
		const_cast<C*>(m_pAsset)->m_iReferences++;
}

template <class C, class L>
CAssetHandle<C, L>::CAssetHandle(const CAssetHandle& c)
{
	m_sName = c.m_sName;
	if (c.m_pAsset)
	{
		m_pAsset = c.m_pAsset;
		const_cast<C*>(m_pAsset)->m_iReferences++;
	}
	else
		m_pAsset = nullptr;
}

template <class C, class L>
CAssetHandle<C, L>::~CAssetHandle()
{
	L::ReleaseAsset(m_pAsset);
}

template <class C, class L>
bool CAssetHandle<C, L>::IsValid() const
{
	if (!m_pAsset)
		return false;

#ifdef _DEBUG
	bool bLoaded = L::IsAssetLoaded(m_sName);
	TAssertNoMsg(bLoaded);
	if (!bLoaded)
		return false;
#endif

	return true;
}

template <class C, class L>
void CAssetHandle<C, L>::Reset()
{
	if (!IsValid())
		return;

	L::ReleaseAsset(m_pAsset);
	m_pAsset = nullptr;
}

template <class C, class L>
const CAssetHandle<C, L>& CAssetHandle<C, L>::operator=(const CAssetHandle<C, L>& c)
{
	m_sName = c.m_sName;
	if (c.m_pAsset)
	{
		m_pAsset = c.m_pAsset;
		const_cast<C*>(m_pAsset)->m_iReferences++;
	}
	else
		m_pAsset = nullptr;

	return *this;
}
