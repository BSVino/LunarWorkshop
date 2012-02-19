#ifndef TINKER_BASEENTITY_H
#define TINKER_BASEENTITY_H

#include <EASTL/map.h>
#include <EASTL/vector.h>

#include <vector.h>
#include <geometry.h>
#include <matrix.h>
#include <quaternion.h>
#include <common.h>

#include <tengine_config.h>

#include <network/network.h>

#include "entityhandle.h"

extern enum collision_type_e;

typedef enum
{
	DAMAGE_GENERIC = 1,
	DAMAGE_EXPLOSION,
	DAMAGE_COLLISION,
	DAMAGE_BURN,
	DAMAGE_LASER,
	DAMAGE_MELEE,
} damagetype_t;

typedef void (*EntityRegisterCallback)();
typedef void (*EntityPrecacheCallback)();
typedef size_t (*EntityCreateCallback)();

template<typename T>
size_t NewEntity()
{
	T* pT = new T();
	return pT->GetHandle();
}

template <class C>
void ResizeVectorTmpl(char* pData, size_t iVectorSize)
{
	eastl::vector<C>* pVector = (eastl::vector<C>*)pData;
	pVector->resize(iVectorSize);
}

// The last three arguments are for error reporting if the unserialization goes awry.
bool UnserializeString_bool(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
size_t UnserializeString_size_t(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
TVector UnserializeString_TVector(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
EAngle UnserializeString_EAngle(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
AABB UnserializeString_AABB(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");

void UnserializeString_bool(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_int(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_size_t(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_float(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_tstring(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_TVector(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_Vector(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_EAngle(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_Matrix4x4(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_AABB(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_ModelID(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_EntityHandle(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);

class CSaveData
{
public:
	typedef enum
	{
		DATA_OMIT = 0,
		DATA_COPYTYPE,
		DATA_COPYARRAY,
		DATA_COPYVECTOR,
		DATA_NETVAR,
		DATA_STRING,
		DATA_STRING16,
		DATA_OUTPUT,
	} datatype_t;

	typedef void (*UnserializeString)(const tstring& sData, CSaveData* pData, class CBaseEntity* pEntity);
	typedef void (*ResizeVector)(char* pData, size_t iVectorSize);

	datatype_t				m_eType;
	const char*				m_pszVariableName;
	const char*				m_pszHandle;
	size_t					m_iOffset;
	size_t					m_iSizeOfVariable;
	size_t					m_iSizeOfType;
	UnserializeString		m_pfnUnserializeString;
	ResizeVector			m_pfnResizeVector;
	char					m_oDefault[24];
	bool					m_bOverride;
};

typedef void (*EntityInputCallback)(const class CBaseEntity* pTarget, const eastl::vector<tstring>& sArgs);
class CEntityInput
{
public:
	eastl::string							m_sName;
	EntityInputCallback						m_pfnCallback;
};

#define DECLARE_ENTITY_INPUT(name) \
	virtual void name(const eastl::vector<tstring>& sArgs); \
	static void name##InputCallback(const class CBaseEntity* pTarget, const eastl::vector<tstring>& sArgs) \
	{ \
		((ThisClass*)pTarget)->name(sArgs); \
	}

class CEntityOutput
{
public:
	void									Call();
	void									AddTarget(const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs, bool bKill);
	void									Clear();

	tstring									FormatArgs(tstring sArgs);

	void									SetEntity(class CBaseEntity* pEnt) { m_pEnt = pEnt; }
	void									SetOutputName(const eastl::string& sOutputName) { m_sOutputName = sOutputName; }

public:
	class CEntityOutputTarget
	{
	public:
		eastl::string						m_sTargetName;
		eastl::string						m_sInput;
		eastl::string						m_sArgs;
		bool								m_bKill;
	};

	eastl::vector<CEntityOutputTarget>		m_aTargets;
	class CBaseEntity*						m_pEnt;
	eastl::string							m_sOutputName;
};

#define DECLARE_ENTITY_OUTPUT(name) \
	CEntityOutput			m_Output_##name; \

class CEntityRegistration
{
public:
	const char*					m_pszEntityClass;
	const char*					m_pszParentClass;
	EntityRegisterCallback		m_pfnRegisterCallback;
	EntityPrecacheCallback		m_pfnPrecacheCallback;
	EntityCreateCallback		m_pfnCreateCallback;
	eastl::vector<CSaveData>	m_aSaveData;
	eastl::vector<CNetworkedVariableData>	m_aNetworkVariables;
	eastl::map<eastl::string, CEntityInput>		m_aInputs;
	eastl::vector<tstring>		m_asPrecaches;
	bool						m_bCreatableInEditor;
};

#define REGISTER_ENTITY_CLASS_NOBASE(entity) \
DECLARE_CLASS(entity, entity); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
static void PrecacheCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::PrecacheCallback(pEntity); \
	delete pEntity; \
} \
static const char* Get##entity##ParentClass() { return NULL; } \
 \
virtual const char* GetClassName() { return #entity; } \
virtual void RegisterNetworkVariables(); \
virtual void RegisterSaveData(); \
virtual void RegisterInputData(); \
virtual size_t SizeOfThis() \
{ \
	/* -4 because the vtable is 4 bytes */ \
	return sizeof(entity) - 4; \
} \
 \
virtual void Serialize(std::ostream& o) \
{ \
	CBaseEntity::Serialize(o, #entity, this); \
} \
 \
virtual bool Unserialize(std::istream& i) \
{ \
	return CBaseEntity::Unserialize(i, #entity, this); \
} \

// Third parameter: how many interfaces does the class have?
#define REGISTER_ENTITY_CLASS_INTERFACES(entity, base, iface) \
DECLARE_CLASS(entity, base); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
static void PrecacheCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::PrecacheCallback(pEntity); \
	delete pEntity; \
} \
static const char* Get##entity##ParentClass() { return #base; } \
 \
virtual const char* GetClassName() { return #entity; } \
virtual void RegisterNetworkVariables(); \
virtual void RegisterSaveData(); \
virtual void RegisterInputData(); \
virtual size_t SizeOfThis() \
{ \
	return sizeof(entity) - sizeof(BaseClass) - iface*4; \
} \
 \
virtual void Serialize(std::ostream& o) \
{ \
	BaseClass::Serialize(o); \
 \
	CBaseEntity::Serialize(o, #entity, this); \
} \
 \
virtual bool Unserialize(std::istream& i) \
{ \
	if (!BaseClass::Unserialize(i)) \
		return false; \
 \
	return CBaseEntity::Unserialize(i, #entity, this); \
} \

#define REGISTER_ENTITY_CLASS(entity, base) \
	REGISTER_ENTITY_CLASS_INTERFACES(entity, base, 0)

#define NETVAR_TABLE_BEGIN(entity) \
void entity::RegisterNetworkVariables() \
{ \
	const char* pszEntity = #entity; \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetClassName()); \
	pRegistration->m_aNetworkVariables.clear(); \
	CGameServer* pGameServer = GameServer(); \
	CNetworkedVariableData* pVarData = NULL; \

#define NETVAR_DEFINE(type, name) \
	pRegistration->m_aNetworkVariables.push_back(CNetworkedVariableData()); \
	pVarData = &pRegistration->m_aNetworkVariables[pRegistration->m_aNetworkVariables.size()-1]; \
	TAssert(!!dynamic_cast<CNetworkedVariableBase*>(&name)); \
	pVarData->m_iOffset = (((size_t)((void*)((CNetworkedVariableBase*)&name)))) - ((size_t)((CBaseEntity*)this)); \
	pVarData->m_pszName = #name; \
	pVarData->m_pfnChanged = NULL; \
	pVarData->m_flUpdateInterval = 0; \

#define NETVAR_DEFINE_CALLBACK(type, name, callback) \
	NETVAR_DEFINE(type, name); \
	pVarData->m_pfnChanged = callback; \

#define NETVAR_DEFINE_INTERVAL(type, name, interval) \
	NETVAR_DEFINE(type, name); \
	pVarData->m_flUpdateInterval = interval; \

#define NETVAR_TABLE_END() \
	CheckTables(pszEntity); \
} \

#define SAVEDATA_TABLE_BEGIN_COMMON(entity, editor) \
void entity::RegisterSaveData() \
{ \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetClassName()); \
	pRegistration->m_aSaveData.clear(); \
	pRegistration->m_bCreatableInEditor = editor; \
	CGameServer* pGameServer = GameServer(); \
	CSaveData* pSaveData = NULL; \

#define SAVEDATA_TABLE_BEGIN(entity) \
	SAVEDATA_TABLE_BEGIN_COMMON(entity, false) \

#define SAVEDATA_TABLE_BEGIN_EDITOR(entity) \
	SAVEDATA_TABLE_BEGIN_COMMON(entity, true) \

#define SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = copy; \
	pSaveData->m_pszVariableName = #name; \
	if (copy == CSaveData::DATA_NETVAR) \
		pSaveData->m_iOffset = (((size_t)((void*)((CNetworkedVariableBase*)&name)))) - ((size_t)((void*)this)); \
	else \
		pSaveData->m_iOffset = (((size_t)((void*)&name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(name); \
	pSaveData->m_iSizeOfType = sizeof(type); \
	pSaveData->m_pfnResizeVector = &ResizeVectorTmpl<type>; \
	pSaveData->m_bOverride = false; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_DEFINE_HANDLE(copy, type, name, handle) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &UnserializeString_##type; \
	memset(pSaveData->m_oDefault, 0, sizeof(type)); \

#define SAVEDATA_DEFINE_HANDLE_DEFAULT(copy, type, name, handle, def) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &UnserializeString_##type; \
	{ \
		type iDefault = def; \
		memcpy(pSaveData->m_oDefault, &iDefault, sizeof(def)); \
	} \

#define SAVEDATA_DEFINE(copy, type, name) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = nullptr; \
	pSaveData->m_pfnUnserializeString = nullptr; \

#define SAVEDATA_DEFINE_HANDLE_FUNCTION(copy, type, name, handle, function) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &function; \
	memset(pSaveData->m_oDefault, 0, sizeof(type)); \

#define SAVEDATA_DEFINE_HANDLE_DEFAULT_FUNCTION(copy, type, name, handle, def, function) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &function; \
	{ \
		type iDefault = def; \
		memcpy(pSaveData->m_oDefault, &iDefault, sizeof(def)); \
	} \

#define SAVEDATA_DEFINE_HANDLE_ENTITY(copy, type, name, handle) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &UnserializeString_EntityHandle; \
	memset(pSaveData->m_oDefault, 0, sizeof(type)); \

#define SAVEDATA_OMIT(name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = CSaveData::DATA_OMIT; \
	pSaveData->m_pszVariableName = #name; \
	pSaveData->m_pszHandle = ""; \
	pSaveData->m_iOffset = (((size_t)((void*)&name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(name); \
	pSaveData->m_iSizeOfType = 0; \
	pSaveData->m_pfnResizeVector = NULL; \
	pSaveData->m_bOverride = false; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_DEFINE_OUTPUT(name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = CSaveData::DATA_OUTPUT; \
	pSaveData->m_pszVariableName = "m_Output_" #name; \
	pSaveData->m_pszHandle = #name; \
	pSaveData->m_iOffset = (((size_t)((void*)&m_Output_##name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(m_Output_##name); \
	pSaveData->m_iSizeOfType = sizeof(CEntityOutput); \
	pSaveData->m_pfnResizeVector = NULL; \
	pSaveData->m_bOverride = false; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_OVERRIDE_DEFAULT(copy, type, name, handle, def) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = copy; \
	pSaveData->m_pszVariableName = #name; \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_bOverride = true; \
	{ \
		type iDefault = def; \
		memcpy(pSaveData->m_oDefault, &iDefault, sizeof(def)); \
	} \

#define SAVEDATA_TABLE_END() \
	CheckSaveDataSize(pRegistration); \
} \

#define INPUTS_TABLE_BEGIN(entity) \
void entity::RegisterInputData() \
{ \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetClassName()); \
	pRegistration->m_aInputs.clear(); \

#define INPUT_DEFINE(name) \
	pRegistration->m_aInputs[#name].m_sName = #name; \
	pRegistration->m_aInputs[#name].m_pfnCallback = &name##InputCallback; \

#define INPUTS_TABLE_END() \
} \

class CTeam;

class CBaseEntity
{
	friend class CGameServer;

	REGISTER_ENTITY_CLASS_NOBASE(CBaseEntity);

public:
											CBaseEntity();
	virtual									~CBaseEntity();

public:
	virtual void							Precache() {};
	virtual void							Spawn();
	DECLARE_ENTITY_OUTPUT(OnSpawn);

	void									SetName(const eastl::string& sName) { m_sName = sName; };
	eastl::string							GetName() const { return m_sName; };

	void									SetMass(float flMass) { m_flMass = flMass; };
	float									GetMass() const { return m_flMass; };

	virtual const AABB&						GetBoundingBox() const { return m_aabbBoundingBox; }
	virtual TVector							GetLocalCenter() const;
	virtual TVector							GetGlobalCenter() const;
	virtual TFloat							GetBoundingRadius() const;

	virtual TFloat							GetRenderRadius() const { return GetBoundingRadius(); };

	void									SetModel(const tstring& sModel);
	void									SetModel(size_t iModel);
	size_t									GetModelID() const { return m_iModel; };
	class CModel*							GetModel() const;
	virtual void							OnSetModel() {};

	virtual Matrix4x4						GetRenderTransform() const { return Matrix4x4(GetGlobalTransform()); };
	virtual Vector							GetRenderOrigin() const { return GetRenderTransform().GetTranslation(); };
	EAngle									GetRenderAngles() const { return GetRenderTransform().GetAngles(); };

	void									SetMoveParent(CBaseEntity* pParent);
	CBaseEntity*							GetMoveParent() const { return m_hMoveParent; };
	bool									HasMoveParent() const { return m_hMoveParent != NULL; };
	void									InvalidateGlobalTransforms();
	TMatrix									GetParentGlobalTransform() const;

	const TMatrix&							GetGlobalTransform();
	TMatrix									GetGlobalTransform() const;
	void									SetGlobalTransform(const TMatrix& m);

	TMatrix									GetGlobalToLocalTransform();
	TMatrix									GetGlobalToLocalTransform() const;

	virtual TVector							GetGlobalOrigin();
	virtual EAngle							GetGlobalAngles();

	virtual TVector							GetGlobalOrigin() const;
	virtual EAngle							GetGlobalAngles() const;

	void									SetGlobalOrigin(const TVector& vecOrigin);
	void									SetGlobalAngles(const EAngle& angAngles);

	virtual TVector							GetGlobalVelocity();
	virtual TVector							GetGlobalVelocity() const;
	void									SetGlobalVelocity(const TVector& vecVelocity);

	virtual inline TVector					GetGlobalGravity() const { return m_vecGlobalGravity; };
	void									SetGlobalGravity(const TVector& vecGravity);

	const TMatrix&							GetLocalTransform() const { return m_mLocalTransform; }
	void									SetLocalTransform(const TMatrix& m);
	virtual void							OnSetLocalTransform(TMatrix& m) {}

	const Quaternion&						GetLocalRotation() const { return m_qLocalRotation; }
	void									SetLocalRotation(const Quaternion& q);

	virtual inline TVector					GetLocalOrigin() const { return m_vecLocalOrigin; };
	void									SetLocalOrigin(const TVector& vecOrigin);

	inline TVector							GetLastLocalOrigin() const { return m_vecLastLocalOrigin; };
	void									SetLastLocalOrigin(const TVector& vecOrigin) { m_vecLastLocalOrigin = vecOrigin; };

	inline TVector							GetLastGlobalOrigin() const;

	inline TVector							GetLocalVelocity() const { return m_vecLocalVelocity; };
	void									SetLocalVelocity(const TVector& vecVelocity);

	inline EAngle							GetLocalAngles() const { return m_angLocalAngles; };
	void									SetLocalAngles(const EAngle& angLocalAngles);

	DECLARE_ENTITY_INPUT(SetLocalOrigin);
	DECLARE_ENTITY_INPUT(SetLocalAngles);

	virtual TVector							GetUpVector() const { return TVector(0, 1, 0); };

	virtual bool							TransformsChildUp() const { return false; };

	bool									IsVisible() const { return m_bVisible; }
	void									SetVisible(bool bVisible) { m_bVisible = bVisible; }

	DECLARE_ENTITY_INPUT(SetVisible);

	bool									IsInPhysics() const { return m_bInPhysics; };
	void									AddToPhysics(enum collision_type_e eCollisionType);
	void									RemoveFromPhysics();

	size_t									GetHandle() const { return m_iHandle; }

	virtual float							GetTotalHealth() const { return m_flTotalHealth; }
	virtual void							SetTotalHealth(float flHealth) { m_flTotalHealth = m_flHealth = flHealth; }
	virtual float							GetHealth() const { return m_flHealth; }
	virtual bool							IsAlive() { return m_flHealth > 0; }

	class CTeam*							GetTeam() const;
	void									SetTeam(class CTeam* pTeam);
	virtual void							OnTeamChange() {};

	virtual void							ClientUpdate(int iClient);
	virtual void							ClientEnterGame();

	virtual void							TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true);
	virtual void							OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true) {};
	virtual bool							TakesDamage() const { return m_bTakeDamage; };
	DECLARE_ENTITY_OUTPUT(OnTakeDamage);
	void									Kill();
	void									Killed(CBaseEntity* pKilledBy);
	virtual void							OnKilled(CBaseEntity* pKilledBy) {};
	DECLARE_ENTITY_OUTPUT(OnKilled);

	void									SetActive(bool bActive);
	bool									IsActive() const { return m_bActive; };
	virtual void							OnActivated() {};
	virtual void							OnDeactivated() {};
	DECLARE_ENTITY_INPUT(Activate);
	DECLARE_ENTITY_INPUT(Deactivate);
	DECLARE_ENTITY_INPUT(ToggleActive);
	DECLARE_ENTITY_INPUT(SetActive);
	DECLARE_ENTITY_OUTPUT(OnActivated);
	DECLARE_ENTITY_OUTPUT(OnDeactivated);

	virtual bool							ShouldRender() const { return (size_t)m_iModel != ~0; };
	virtual bool							ShouldRenderModel() const { return true; };
	virtual void							PreRender(bool bTransparent) const;
	virtual void							ModifyContext(class CRenderingContext* pContext, bool bTransparent) const {};
	void									Render(bool bTransparent) const;
	virtual void							OnRender(class CRenderingContext* pContext, bool bTransparent) const {};
	virtual void							PostRender(bool bTransparent) const {};

	void									Delete();
	virtual void							OnDeleted() {};
	virtual void							OnDeleted(class CBaseEntity* pEntity) {};
	bool									IsDeleted() { return m_bDeleted; }
	void									SetDeleted() { m_bDeleted = true; }
	DECLARE_ENTITY_INPUT(Delete);

	virtual void							Think() {};

	virtual void							Touching(const CBaseEntity* pOther) {};
	virtual void							BeginTouchingList() {};
	virtual void							EndTouchingList() {};

	void									CallInput(const eastl::string& sName, const tstring& sArgs);
	void									CallOutput(const eastl::string& sName);
	void									AddOutputTarget(const eastl::string& sName, const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs = "", bool bKill = false);
	void									RemoveOutputs(const eastl::string& sName);
	virtual tstring							GetOutputValue(const tstring& sOutput, size_t iValue) { return ""; }
	DECLARE_ENTITY_INPUT(RemoveOutput);

	void									EmitSound(const tstring& sSound, float flVolume = 1.0f, bool bLoop = false);
	void									StopSound(const tstring& sModel);
	bool									IsSoundPlaying(const tstring& sModel);
	void									SetSoundVolume(const tstring& sModel, float flVolume);

	virtual TFloat							Distance(const TVector& vecSpot) const;

	// Physics callback - Should this object collide with pOther at the specified point?
	// At this point the expensive collision checks have passed and the two objects will
	// definitely collide if true is returned here. If two objects should never collide,
	// use collision groups instead to avoid the expensive collision checks.
	virtual bool							ShouldCollideWith(CBaseEntity* pOther, const TVector& vecPoint) const { return true; }

	size_t									GetSpawnSeed() const { return m_iSpawnSeed; }
	void									SetSpawnSeed(size_t iSpawnSeed);

	float									GetSpawnTime() const { return m_flSpawnTime; }
	void									SetSpawnTime(float flSpawnTime) { m_flSpawnTime = flSpawnTime; };

	bool									HasIssuedClientSpawn() { return m_bClientSpawn; }
	void									IssueClientSpawn();
	virtual void							ClientSpawn();

	CSaveData*								GetSaveData(const char* pszName);
	CSaveData*								GetSaveDataByHandle(const char* pszHandle);
	CNetworkedVariableData*					GetNetworkVariable(const char* pszName);
	CEntityInput*							GetInput(const char* pszName);

	virtual void							OnSerialize(std::ostream& o) {};
	virtual bool							OnUnserialize(std::istream& i) { return true; };
	void									CheckSaveDataSize(CEntityRegistration* pRegistration);

	void									CheckTables(const char* pszEntity);

	static CBaseEntity*						GetEntity(size_t iHandle);
	template <class T>
	static T*								GetEntityType(size_t iHandle)
	{
		CBaseEntity* pEntity = GetEntity(iHandle);
		if (!pEntity)
			return NULL;

		return dynamic_cast<T*>(pEntity);
	}

	static size_t							GetNumEntities();

	void									PrecacheModel(const tstring& sModel);
	void									PrecacheParticleSystem(const tstring& sSystem);
	void									PrecacheSound(const tstring& sSound);
	void									PrecacheTexture(const tstring& sTexture);

public:
	static void								RegisterEntity(const char* pszClassName, const char* pszParentClass, EntityRegisterCallback pfnRegisterCallback, EntityPrecacheCallback pfnPrecacheCallback, EntityCreateCallback pfnCreateCallback);
	static void								Register(CBaseEntity* pEntity);
	static CEntityRegistration*				GetRegisteredEntity(tstring sClassName);
	static void								PrecacheCallback(CBaseEntity* pEntity);

	static size_t							GetNumEntitiesRegistered();
	static CEntityRegistration*				GetEntityRegistration(size_t iEntity);

	static CSaveData*						GetSaveData(const char* pszClassName, const char* pszName);
	static CSaveData*						GetSaveDataByHandle(const char* pszClassName, const char* pszHandle);
	static CSaveData*						GetOutput(const char* pszClassName, const tstring& sOutput);

	static void								SerializeEntity(std::ostream& o, CBaseEntity* pEntity);
	static bool								UnserializeEntity(std::istream& i);

	static void								Serialize(std::ostream& o, const char* pszClassName, void* pEntity);
	static bool								Unserialize(std::istream& i, const char* pszClassName, void* pEntity);

	template <class T>
	static T*								FindClosest(const TVector& vecPoint, CBaseEntity* pFurther = NULL);

	static CBaseEntity*						GetEntityByName(const eastl::string& sName);
	static void								FindEntitiesByName(const eastl::string& sName, eastl::vector<CBaseEntity*>& apEntities);

protected:
	static eastl::map<tstring, CEntityRegistration>& GetEntityRegistration();

protected:
	tstring									m_sName;
	tstring									m_sClassName;

	float									m_flMass;

	CNetworkedHandle<CBaseEntity>			m_hMoveParent;
	CNetworkedSTLVector<CEntityHandle<CBaseEntity>>	m_ahMoveChildren;

	AABB									m_aabbBoundingBox;

	bool									m_bGlobalTransformsDirty;
	TMatrix									m_mGlobalTransform;
	CNetworkedVector						m_vecGlobalGravity;

	TMatrix									m_mLocalTransform;
	Quaternion								m_qLocalRotation;
	CNetworkedVector						m_vecLocalOrigin;
	TVector									m_vecLastLocalOrigin;
	CNetworkedEAngle						m_angLocalAngles;
	CNetworkedVector						m_vecLocalVelocity;

	size_t									m_iHandle;

	CNetworkedVariable<bool>				m_bTakeDamage;
	CNetworkedVariable<float>				m_flTotalHealth;
	CNetworkedVariable<float>				m_flHealth;
	float									m_flTimeKilled;
	float									m_flLastTakeDamage;

	CNetworkedVariable<bool>				m_bActive;

	CNetworkedHandle<CTeam>					m_hTeam;

	bool									m_bVisible;
	bool									m_bInPhysics;
	bool									m_bDeleted;
	bool									m_bClientSpawn;

	CNetworkedVariable<int>					m_iCollisionGroup;

	CNetworkedVariable<size_t>				m_iModel;

	size_t									m_iSpawnSeed;
	CNetworkedVariable<float>				m_flSpawnTime;

private:
	static eastl::vector<CBaseEntity*>		s_apEntityList;
	static size_t							s_iEntities;
	static size_t							s_iOverrideEntityListIndex;
	static size_t							s_iNextEntityListIndex;
};

#define REGISTER_ENTITY(entity) \
class CRegister##entity \
{ \
public: \
	CRegister##entity() \
	{ \
		CBaseEntity::RegisterEntity(#entity, entity::Get##entity##ParentClass(), &entity::RegisterCallback##entity, &entity::PrecacheCallback##entity, &NewEntity<entity>); \
	} \
} g_Register##entity = CRegister##entity(); \

#include "gameserver.h"
#include "template_functions.h"

template <class T>
T* CBaseEntity::FindClosest(const TVector& vecPoint, CBaseEntity* pFurther)
{
	T* pClosest = NULL;

	TFloat flFurtherDistance = 0;
	if (pFurther)
		flFurtherDistance = pFurther->Distance(vecPoint);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		T* pT = dynamic_cast<T*>(pEntity);

		if (!pT)
			continue;

		if (pT == pFurther)
			continue;

		TFloat flEntityDistance = pT->Distance(vecPoint);
		if (pFurther && (flEntityDistance <= flFurtherDistance))
			continue;

		if (!pClosest)
		{
			pClosest = pT;
			continue;
		}

		if (flEntityDistance < pClosest->Distance(vecPoint))
			pClosest = pT;
	}

	return pClosest;
}

#endif
