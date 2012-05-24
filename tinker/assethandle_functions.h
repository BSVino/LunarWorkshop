#pragma once

template <class C, class L>
CAssetHandle<C, L>::CAssetHandle(const tstring& sName, const C* pAsset = nullptr)
{
	m_sName = sName;
	if (pAsset)
		m_pAsset = pAsset;
	else
	{
		CAssetHandle<C, L> hAsset = L::FindAsset(sName);
		if (hAsset.IsValid())
			m_pAsset = hAsset.m_pAsset;
		else
			m_pAsset = L::AddAsset(sName);
	}

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
	Reset();
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

	TAssertNoMsg(m_pAsset->m_iReferences);
	const_cast<C*>(m_pAsset)->m_iReferences--;

	m_pAsset = nullptr;
}

template <class C, class L>
const CAssetHandle<C, L>& CAssetHandle<C, L>::operator=(const CAssetHandle<C, L>& c)
{
	if (c == *this)
		return *this;

	Reset();

	// If c == *this, c.m_pAsset will be clobbered in Reset()
	const C* pAsset = c.m_pAsset;

	m_sName = c.m_sName;
	if (pAsset)
	{
		m_pAsset = pAsset;
		const_cast<C*>(m_pAsset)->m_iReferences++;
	}
	else
		m_pAsset = nullptr;

	return *this;
}

template <class C, class L>
CAssetHandle<C, L>& CAssetHandle<C, L>::operator=(CAssetHandle<C, L>&& c)
{
	if (c == *this)
		return *this;

	Reset();

	m_pAsset = c.m_pAsset;
	swap(m_sName, c.m_sName);

	c.m_pAsset = nullptr;

	return *this;
}
