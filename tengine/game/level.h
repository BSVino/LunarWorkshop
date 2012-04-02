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

	CLevelEntity()
	{
		InitializeCallbacks();
	}

public:
	void								InitializeCallbacks()
	{
		m_mGlobalTransform.SetCallbacks(&CalculateGlobalTransform, this);
		m_bVisible.SetCallbacks(&CalculateVisible, this);
		m_iModel.SetCallbacks(&CalculateModelID, this);
		m_iTexture.SetCallbacks(&CalculateTextureID, this);
		m_vecTextureModelScale.SetCallbacks(&CalculateTextureModelScale, this);
		m_aabbBounds.SetCallbacks(&CalculateBoundingBox, this);
		m_sName.SetCallbacks(&CalculateName, this);
	}

	void								Dirtify()
	{
		m_mGlobalTransform.Dirtify();
		m_bVisible.Dirtify();
		m_iModel.Dirtify();
		m_iTexture.Dirtify();
		m_vecTextureModelScale.Dirtify();
		m_aabbBounds.Dirtify();
		m_sName.Dirtify();
	}

	tstring								GetClass() { return m_sClass; }
	void								SetClass(const tstring& sClass) { m_sClass = sClass; }

	eastl::vector<CLevelEntityOutput>&	GetOutputs() { return m_aOutputs; }

	const tstring&						GetParameterValue(const tstring& sKey) const;
	void								SetParameterValue(const tstring& sKey, const tstring& sValue);
	void								RemoveParameter(const tstring& sKey);
	bool								HasParameterValue(const tstring& sKey) const;
	eastl::map<tstring, tstring>&		GetParameters() { return m_asParameters; }

	Matrix4x4							GetGlobalTransform() { return m_mGlobalTransform; }
	void								SetGlobalTransform(const Matrix4x4& m) { m_mGlobalTransform = m; }
	bool								IsVisible() { return m_bVisible; }
	size_t								GetModelID() { return m_iModel; }
	size_t								GetTextureModelID() { return m_iTexture; }
	Vector2D							GetTextureModelScale() { return m_vecTextureModelScale; }
	AABB								GetBoundingBox() { return m_aabbBounds; }
	tstring								GetName() { return m_sName; }

public:
	static Matrix4x4					CalculateGlobalTransform(CLevelEntity* pThis);
	static bool							CalculateVisible(CLevelEntity* pThis);
	static size_t						CalculateModelID(CLevelEntity* pThis);
	static size_t						CalculateTextureID(CLevelEntity* pThis);
	static Vector2D						CalculateTextureModelScale(CLevelEntity* pThis);
	static AABB							CalculateBoundingBox(CLevelEntity* pThis);
	static tstring						CalculateName(CLevelEntity* pThis);

private:
	tstring								m_sClass;
	eastl::map<tstring, tstring>		m_asParameters;

	CCachedValue<Matrix4x4, CLevelEntity>	m_mGlobalTransform;
	CCachedValue<bool, CLevelEntity>		m_bVisible;
	CCachedValue<size_t, CLevelEntity>		m_iModel;
	CCachedValue<size_t, CLevelEntity>		m_iTexture;
	CCachedValue<Vector2D, CLevelEntity>	m_vecTextureModelScale;
	CCachedValue<AABB, CLevelEntity>		m_aabbBounds;
	CCachedValue<tstring, CLevelEntity>		m_sName;

	eastl::vector<CLevelEntityOutput>	m_aOutputs;
};

class CLevel
{
public:
	virtual					~CLevel() {};

public:
	void					ReadInfoFromData(const class CData* pData);
	virtual void			OnReadInfo(const class CData* pData);

	void					SaveToFile();

	const tstring&			GetName() { return m_sName; }
	const tstring&			GetFile() { return m_sFile; }

	void					SetFile(const tstring& sFile) { m_sFile = sFile; }

	const tstring&			GetGameMode() { return m_sGameMode; }

	void					CreateEntitiesFromData(const CData* pData);
	eastl::vector<CLevelEntity>& GetEntityData() { return m_aLevelEntities; }
	const eastl::vector<CLevelEntity>& GetEntityData() const { return m_aLevelEntities; }

protected:
	tstring					m_sName;
	tstring					m_sFile;

	tstring					m_sGameMode;

	eastl::vector<CLevelEntity> m_aLevelEntities;
};

#endif
