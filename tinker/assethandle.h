#pragma once

#include <tstring.h>

// Handy way of accessing an asset
// Increments the ref count of an asset when it's created, decrements when it's destroyed
// Asset C must have a member m_iReferences. Library L must have AddAsset(tstring) ReleaseAsset(C) and IsAssetLoaded(tstring).
template <class C, class L>
class CAssetHandle
{
public:
	CAssetHandle()
	{
		m_pAsset = nullptr;
	}

	CAssetHandle(const tstring& sName, const C* pAsset = nullptr);
	CAssetHandle(const CAssetHandle& c);
	~CAssetHandle();

public:
	bool IsValid() const;

	tstring GetName() const
	{
		return m_sName;
	}

	const C* GetAsset() const
	{
		if (!IsValid())
			return nullptr;

		return m_pAsset;
	}

	void Reset();
	const CAssetHandle& operator=(const CAssetHandle& c);
	CAssetHandle& operator=(CAssetHandle&& c);

	inline operator const C*() const
	{
		return GetAsset();
	}

	inline const C* operator->() const
	{
		return GetAsset();
	}

protected:
	tstring				m_sName;
	const C*			m_pAsset;
};

