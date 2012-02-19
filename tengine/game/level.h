#ifndef LW_LEVEL_H
#define LW_LEVEL_H

#include <EASTL/map.h>

#include <tstring.h>
#include <cachedvalue.h>
#include <matrix.h>
#include <geometry.h>

// A description of an entity for use in the level editor
class CLevelEntity
{
public:
	CLevelEntity(const tstring& sClass)
	{
		InitializeCallbacks();
		m_sClass = sClass;
	}

	CLevelEntity(const CLevelEntity& o)
	{
		(*this) = o;

		InitializeCallbacks();
	}

public:
	void								InitializeCallbacks()
	{
		m_mGlobalTransform.SetCallbacks(&CalculateGlobalTransform, this);
		m_bVisible.SetCallbacks(&CalculateVisible, this);
		m_iModel.SetCallbacks(&CalculateModelID, this);
		m_aabbBounds.SetCallbacks(&CalculateBoundingBox, this);
		m_sName.SetCallbacks(&CalculateName, this);
	}

	const tstring&						GetParameterValue(const tstring& sKey) const;

	Matrix4x4							GetGlobalTransform() { return m_mGlobalTransform; }
	bool								IsVisible() { return m_bVisible; }
	size_t								GetModelID() { return m_iModel; }
	AABB								GetBoundingBox() { return m_aabbBounds; }
	tstring								GetName() { return m_sName; }

public:
	static Matrix4x4					CalculateGlobalTransform(CLevelEntity* pThis);
	static bool							CalculateVisible(CLevelEntity* pThis);
	static size_t						CalculateModelID(CLevelEntity* pThis);
	static AABB							CalculateBoundingBox(CLevelEntity* pThis);
	static tstring						CalculateName(CLevelEntity* pThis);

public:
	tstring								m_sClass;
	eastl::map<tstring, tstring>		m_asParameters;

	CCachedValue<Matrix4x4, CLevelEntity>	m_mGlobalTransform;
	CCachedValue<bool, CLevelEntity>	m_bVisible;
	CCachedValue<size_t, CLevelEntity>	m_iModel;
	CCachedValue<AABB, CLevelEntity>	m_aabbBounds;
	CCachedValue<tstring, CLevelEntity>	m_sName;

	class CLevelEntityOutput
	{
	public:
		CLevelEntityOutput()
		{
			m_bKill = false;
		}

	public:
		eastl::string					m_sOutput;
		eastl::string					m_sTargetName;
		eastl::string					m_sInput;
		eastl::string					m_sArgs;
		bool							m_bKill;
	};

	eastl::vector<CLevelEntityOutput>	m_aOutputs;
};

class CLevel
{
public:
	virtual					~CLevel() {};

public:
	void					ReadInfoFromData(const class CData* pData);
	virtual void			OnReadInfo(const class CData* pData);

	const tstring&			GetName() { return m_sName; }
	const tstring&			GetFile() { return m_sFile; }

	void					SetFile(const tstring& sFile) { m_sFile = sFile; }

	const tstring&			GetGameMode() { return m_sGameMode; }

	void					CreateEntitiesFromData(const CData* pData);
	eastl::vector<CLevelEntity>& GetEntityData() { return m_aLevelEntities; }

protected:
	tstring					m_sName;
	tstring					m_sFile;

	tstring					m_sGameMode;

	eastl::vector<CLevelEntity> m_aLevelEntities;
};

#endif
