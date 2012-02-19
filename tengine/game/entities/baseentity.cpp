#include "baseentity.h"

#include <strutils.h>
#include <mtrand.h>

#include <models/models.h>
#include <renderer/renderingcontext.h>
#include <renderer/particles.h>
#include <sound/sound.h>
#include <tinker/application.h>
#include <tinker/profiler.h>
#include <network/commands.h>
#include <textures/texturelibrary.h>
#include <tinker/cvar.h>
#include <physics/physics.h>
#include <game/entities/character.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>

#include "game.h"

bool g_bAutoImporting = false;
#include "beam.h"
#include "counter.h"
#include "kinematic.h"
#include "logicgate.h"
#include "mathgate.h"
#include "playerstart.h"
#include "world.h"
#include "trigger.h"
// Use this to force import of required entities.
class CAutoImport
{
public:
	CAutoImport()
	{
		g_bAutoImporting = true;
		{
			CBeam b;
			CCounter c;
			CKinematic k;
			CLogicGate l;
			CMathGate m;
			CPlayerStart p;
			CWorld w;
			CTrigger t;
		}
		g_bAutoImporting = false;
	}
} g_AutoImport = CAutoImport();

eastl::vector<CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iEntities = 0;
size_t CBaseEntity::s_iOverrideEntityListIndex = ~0;
size_t CBaseEntity::s_iNextEntityListIndex = 0;

REGISTER_ENTITY(CBaseEntity);

NETVAR_TABLE_BEGIN(CBaseEntity);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_hMoveParent);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_ahMoveChildren);
	NETVAR_DEFINE(TVector, m_vecGlobalGravity);
	NETVAR_DEFINE_INTERVAL(TVector, m_vecLocalOrigin, 0.15f);
	NETVAR_DEFINE_INTERVAL(EAngle, m_angLocalAngles, 0.15f);
	NETVAR_DEFINE_INTERVAL(TVector, m_vecLocalVelocity, 0.15f);
	NETVAR_DEFINE(bool, m_bTakeDamage);
	NETVAR_DEFINE(float, m_flTotalHealth);
	NETVAR_DEFINE(float, m_flHealth);
	NETVAR_DEFINE(bool, m_bActive);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_hTeam);
	NETVAR_DEFINE(int, m_iCollisionGroup);
	NETVAR_DEFINE(size_t, m_iModel);
	NETVAR_DEFINE(float, m_flSpawnTime);
NETVAR_TABLE_END();

void UnserializeString_LocalOrigin(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);
void UnserializeString_LocalAngles(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);
void UnserializeString_MoveParent(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);

SAVEDATA_TABLE_BEGIN(CBaseEntity);
	SAVEDATA_DEFINE_OUTPUT(OnSpawn);
	SAVEDATA_DEFINE_OUTPUT(OnTakeDamage);
	SAVEDATA_DEFINE_OUTPUT(OnKilled);
	SAVEDATA_DEFINE_OUTPUT(OnActivated);
	SAVEDATA_DEFINE_OUTPUT(OnDeactivated);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sName, "Name");
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, tstring, m_sClassName);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, float, m_flMass, "Mass", 10);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_hMoveParent, "MoveParent", UnserializeString_MoveParent);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_ahMoveChildren);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, AABB, m_aabbBoundingBox, "BoundingBox", AABB(Vector(-0.5f, -0.5f, -0.5f), Vector(0.5f, 0.5f, 0.5f)));
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bGlobalTransformsDirty);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TMatrix, m_mGlobalTransform);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, TVector, m_vecGlobalGravity, "GlobalGravity", Vector(0, -9.8f, 0));
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TMatrix, m_mLocalTransform);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Quaternion, m_qLocalRotation);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_NETVAR, TVector, m_vecLocalOrigin, "LocalOrigin", UnserializeString_LocalOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TVector, m_vecLastLocalOrigin);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_NETVAR, EAngle, m_angLocalAngles, "LocalAngles", UnserializeString_LocalAngles);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_NETVAR, TVector, m_vecLocalVelocity, "LocalVelocity");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iHandle);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, bool, m_bTakeDamage, "TakeDamage", true);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_NETVAR, float, m_flTotalHealth, "TotalHealth");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_NETVAR, float, m_flHealth, "Health");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeKilled);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastTakeDamage);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, bool, m_bActive, "Active", true);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CTeam>, m_hTeam);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bVisible, "Visible", true);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bInPhysics);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, bool, m_bDeleted);	// Deleted entities are not saved.
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bClientSpawn);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_NETVAR, int, m_iCollisionGroup, "CollisionGroup");
	SAVEDATA_DEFINE_HANDLE_DEFAULT_FUNCTION(CSaveData::DATA_NETVAR, size_t, m_iModel, "Model", ~0, UnserializeString_ModelID);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iSpawnSeed);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flSpawnTime);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBaseEntity);
	INPUT_DEFINE(SetLocalOrigin);
	INPUT_DEFINE(SetLocalAngles);
	INPUT_DEFINE(SetVisible);
	INPUT_DEFINE(Activate);
	INPUT_DEFINE(Deactivate);
	INPUT_DEFINE(ToggleActive);
	INPUT_DEFINE(SetActive);
	INPUT_DEFINE(RemoveOutput);
	INPUT_DEFINE(Delete);
INPUTS_TABLE_END();

CBaseEntity::CBaseEntity()
{
	if (g_bAutoImporting)
		return;

	if (s_iOverrideEntityListIndex == ~0)
		m_iHandle = s_iNextEntityListIndex;
	else
		m_iHandle = s_iOverrideEntityListIndex;

	s_iNextEntityListIndex = (m_iHandle+1)%s_apEntityList.size();
	while (s_apEntityList[s_iNextEntityListIndex] != NULL)
	{
		s_iNextEntityListIndex = (s_iNextEntityListIndex+1)%s_apEntityList.size();
	}

	s_apEntityList[m_iHandle] = this;

	s_iEntities++;

	m_iCollisionGroup = 0;

	m_flMass = 50.0f;
	m_bTakeDamage = false;
	m_flTotalHealth = 1;
	m_flHealth = 1;
	m_flTimeKilled = 0;
	m_flLastTakeDamage = -1;

	m_bDeleted = false;
	m_bActive = true;
	m_bVisible = true;
	m_bInPhysics = false;

	m_iModel = ~0;

	m_iSpawnSeed = 0;

	m_bClientSpawn = false;

	m_bGlobalTransformsDirty = true;
}

CBaseEntity::~CBaseEntity()
{
	if (g_bAutoImporting)
		return;

	if (IsInPhysics())
		RemoveFromPhysics();

	s_apEntityList[m_iHandle] = NULL;

	TAssert(s_iEntities > 0);
	s_iEntities--;
}

void CBaseEntity::Spawn()
{
}

TVector CBaseEntity::GetLocalCenter() const
{
	return GetLocalTransform() * m_aabbBoundingBox.Center();
}

TVector CBaseEntity::GetGlobalCenter() const
{
	return GetGlobalTransform() * m_aabbBoundingBox.Center();
}

TFloat CBaseEntity::GetBoundingRadius() const
{
	return m_aabbBoundingBox.Size().Length()/2;
}

void CBaseEntity::SetModel(const tstring& sModel)
{
	size_t iModel = CModelLibrary::FindModel(sModel);

	if (iModel == ~0)
	{
		TError("Couldn't find model " + sModel + "\n");
		return;
	}

	SetModel(iModel);
}

void CBaseEntity::SetModel(size_t iModel)
{
	m_iModel = iModel;

	if (m_iModel.Get() == ~0)
		return;

	CModel* pModel = CModelLibrary::GetModel(iModel);
	if (pModel)
		m_aabbBoundingBox = pModel->m_aabbBoundingBox;

	OnSetModel();
}

CModel* CBaseEntity::GetModel() const
{
	return CModelLibrary::GetModel(GetModelID());
}

void CBaseEntity::SetMoveParent(CBaseEntity* pParent)
{
	if (IsInPhysics() && pParent)
	{
		collision_type_t eCollisionType = GamePhysics()->GetEntityCollisionType(this);
		TAssert(eCollisionType == CT_KINEMATIC || eCollisionType == CT_CHARACTER);
	}

	if (m_hMoveParent.GetPointer() == pParent)
		return;

	if (m_hMoveParent != NULL)
	{
		TMatrix mPreviousGlobal = GetGlobalTransform();
		TVector vecPreviousVelocity = GetGlobalVelocity();
		TVector vecPreviousLastOrigin = mPreviousGlobal * GetLastLocalOrigin();

		for (size_t i = 0; i < m_hMoveParent->m_ahMoveChildren.size(); i++)
		{
			if (m_hMoveParent->m_ahMoveChildren[i]->GetHandle() == GetHandle())
			{
				m_hMoveParent->m_ahMoveChildren.erase(i);
				break;
			}
		}
		m_hMoveParent = NULL;

		m_vecLocalVelocity = vecPreviousVelocity;
		m_mLocalTransform = mPreviousGlobal;
		m_vecLastLocalOrigin = vecPreviousLastOrigin;
		m_vecLocalOrigin = mPreviousGlobal.GetTranslation();
		m_qLocalRotation = Quaternion(mPreviousGlobal);
		m_angLocalAngles = mPreviousGlobal.GetAngles();

		InvalidateGlobalTransforms();
	}

	TVector vecPreviousVelocity = GetLocalVelocity();
	TVector vecPreviousLastOrigin = GetLastLocalOrigin();
	TMatrix mPreviousTransform = GetLocalTransform();

	m_hMoveParent = pParent;

	if (!pParent)
		return;

	pParent->m_ahMoveChildren.push_back(this);

	TMatrix mGlobalToLocal = m_hMoveParent->GetGlobalToLocalTransform();

	m_vecLastLocalOrigin = mGlobalToLocal * vecPreviousLastOrigin;
	m_mLocalTransform = mGlobalToLocal * mPreviousTransform;
	m_vecLocalOrigin = m_mLocalTransform.GetTranslation();
	m_qLocalRotation = Quaternion(m_mLocalTransform);
	m_angLocalAngles = m_mLocalTransform.GetAngles();

	TFloat flVelocityLength = vecPreviousVelocity.Length();
	if (flVelocityLength > TFloat(0))
		m_vecLocalVelocity = mGlobalToLocal.TransformVector(vecPreviousVelocity);
	else
		m_vecLocalVelocity = TVector(0, 0, 0);

	InvalidateGlobalTransforms();
}

void CBaseEntity::InvalidateGlobalTransforms()
{
	m_bGlobalTransformsDirty = true;

	for (size_t i = 0; i < m_ahMoveChildren.size(); i++)
		m_ahMoveChildren[i]->InvalidateGlobalTransforms();
}

TMatrix CBaseEntity::GetParentGlobalTransform() const
{
	if (!HasMoveParent())
		return TMatrix();

	return GetMoveParent()->GetGlobalTransform();
}

const TMatrix& CBaseEntity::GetGlobalTransform()
{
	if (!m_bGlobalTransformsDirty)
		return m_mGlobalTransform;

	if (m_hMoveParent == NULL)
		m_mGlobalTransform = m_mLocalTransform;
	else
		m_mGlobalTransform = m_hMoveParent->GetGlobalTransform() * m_mLocalTransform;

	m_bGlobalTransformsDirty = false;

	return m_mGlobalTransform;
}

TMatrix CBaseEntity::GetGlobalTransform() const
{
	if (!m_bGlobalTransformsDirty)
		return m_mGlobalTransform;

	if (m_hMoveParent == NULL)
		return m_mLocalTransform;
	else
		return m_hMoveParent->GetGlobalTransform() * m_mLocalTransform;
}

void CBaseEntity::SetGlobalTransform(const TMatrix& m)
{
	TMatrix mNew = m;
	if (HasMoveParent())
		mNew = GetMoveParent()->GetGlobalToLocalTransform() *  m;

	if (mNew != m_mLocalTransform)
		OnSetLocalTransform(mNew);

	if (HasMoveParent())
	{
		m_mLocalTransform = mNew;
		m_bGlobalTransformsDirty = true;
	}
	else
	{
		m_mGlobalTransform = m_mLocalTransform = mNew;
		m_bGlobalTransformsDirty = false;
	}

	m_vecLocalOrigin = m_mLocalTransform.GetTranslation();
	m_angLocalAngles = m_mLocalTransform.GetAngles();
	m_qLocalRotation = Quaternion(m_mLocalTransform);

	if (IsInPhysics())
		GamePhysics()->SetEntityTransform(this, GetGlobalTransform());
}

TMatrix CBaseEntity::GetGlobalToLocalTransform()
{
	if (HasMoveParent())
		return GetMoveParent()->GetGlobalTransform().InvertedRT();
	else
		return GetGlobalTransform().InvertedRT();
}

TMatrix CBaseEntity::GetGlobalToLocalTransform() const
{
	if (HasMoveParent())
		return GetMoveParent()->GetGlobalTransform().InvertedRT();
	else
		return GetGlobalTransform().InvertedRT();
}

TVector CBaseEntity::GetGlobalOrigin()
{
	return GetGlobalTransform().GetTranslation();
}

EAngle CBaseEntity::GetGlobalAngles()
{
	return GetGlobalTransform().GetAngles();
}

TVector CBaseEntity::GetGlobalOrigin() const
{
	return GetGlobalTransform().GetTranslation();
}

EAngle CBaseEntity::GetGlobalAngles() const
{
	return GetGlobalTransform().GetAngles();
}

void CBaseEntity::SetGlobalOrigin(const TVector& vecOrigin)
{
	if (m_hMoveParent == NULL)
		SetLocalOrigin(vecOrigin);
	else
	{
		TMatrix mGlobalToLocal = GetMoveParent()->GetGlobalToLocalTransform();
		SetLocalOrigin(mGlobalToLocal * vecOrigin);
	}
}

void CBaseEntity::SetGlobalAngles(const EAngle& angAngles)
{
	if (m_hMoveParent == NULL)
		SetLocalAngles(angAngles);
	else
	{
		TMatrix mGlobalToLocal = m_hMoveParent->GetGlobalToLocalTransform();
		mGlobalToLocal.SetTranslation(TVector(0,0,0));
		TMatrix mGlobalAngles;
		mGlobalAngles.SetAngles(angAngles);
		TMatrix mLocalAngles = mGlobalToLocal * mGlobalAngles;
		SetLocalAngles(mLocalAngles.GetAngles());
	}
}

TVector CBaseEntity::GetGlobalVelocity()
{
	if (IsInPhysics())
		return GamePhysics()->GetEntityVelocity(this);

	return GetParentGlobalTransform().TransformVector(GetLocalVelocity());
}

TVector CBaseEntity::GetGlobalVelocity() const
{
	return GetParentGlobalTransform().TransformVector(GetLocalVelocity());
}

void CBaseEntity::SetGlobalVelocity(const TVector& vecVelocity)
{
	if (m_hMoveParent == NULL)
		SetLocalVelocity(vecVelocity);
	else
		SetLocalVelocity(GetMoveParent()->GetGlobalToLocalTransform().TransformVector(vecVelocity));
}

void CBaseEntity::SetGlobalGravity(const TVector& vecGravity)
{
	m_vecGlobalGravity = vecGravity;

	if (IsInPhysics())
		GamePhysics()->SetEntityGravity(this, vecGravity);
}

void CBaseEntity::SetLocalTransform(const TMatrix& m)
{
	TMatrix mNew = m;
	OnSetLocalTransform(mNew);

	if (!m_vecLocalOrigin.IsInitialized())
		m_vecLocalOrigin = m.GetTranslation();

	EAngle angNew = mNew.GetAngles();
	if (!m_angLocalAngles.IsInitialized())
		m_angLocalAngles = angNew;

	if (IsInPhysics())
	{
		TAssert(!GetMoveParent());
		GamePhysics()->SetEntityTransform(this, mNew);
	}

	m_mLocalTransform = mNew;

	if ((mNew.GetTranslation() - m_vecLocalOrigin).LengthSqr() > TFloat(0))
		m_vecLocalOrigin = mNew.GetTranslation();

	EAngle angDifference = angNew - m_angLocalAngles;
	if (fabs(angDifference.p) > 0.001f || fabs(angDifference.y) > 0.001f || fabs(angDifference.r) > 0.001f)
	{
		m_angLocalAngles = mNew.GetAngles();
		m_qLocalRotation = Quaternion(mNew);
	}

	InvalidateGlobalTransforms();
}

void CBaseEntity::SetLocalRotation(const Quaternion& q)
{
	SetLocalAngles(q.GetAngles());

	InvalidateGlobalTransforms();

	if (IsInPhysics())
	{
		TAssert(!GetMoveParent());
		GamePhysics()->SetEntityTransform(this, GetGlobalTransform());
	}
}

void CBaseEntity::SetLocalOrigin(const TVector& vecOrigin)
{
	if (!m_vecLocalOrigin.IsInitialized())
		m_vecLocalOrigin = vecOrigin;

	if (IsInPhysics())
	{
		if (GetMoveParent())
		{
			collision_type_t eCollisionType = GamePhysics()->GetEntityCollisionType(this);
			TAssert(eCollisionType == CT_KINEMATIC || eCollisionType == CT_CHARACTER);
		}

		Matrix4x4 mLocal = m_mLocalTransform;
		mLocal.SetTranslation(vecOrigin);

		Matrix4x4 mGlobal = GetParentGlobalTransform() * mLocal;

		GamePhysics()->SetEntityTransform(this, mGlobal);
	}

	if ((vecOrigin - m_vecLocalOrigin).LengthSqr() == TFloat(0))
		return;

	TMatrix mNew = m_mLocalTransform;
	mNew.SetTranslation(vecOrigin);
	OnSetLocalTransform(mNew);

	m_vecLocalOrigin = mNew.GetTranslation();
	m_mLocalTransform = mNew;

	InvalidateGlobalTransforms();
};

TVector CBaseEntity::GetLastGlobalOrigin() const
{
	return GetParentGlobalTransform() * GetLastLocalOrigin();
}

void CBaseEntity::SetLocalVelocity(const TVector& vecVelocity)
{
	if (!m_vecLocalVelocity.IsInitialized())
		m_vecLocalVelocity = vecVelocity;

	if (IsInPhysics())
		GamePhysics()->SetEntityVelocity(this, GetParentGlobalTransform().TransformVector(vecVelocity));

	if ((vecVelocity - m_vecLocalVelocity).LengthSqr() == TFloat(0))
		return;

	m_vecLocalVelocity = vecVelocity;
}

void CBaseEntity::SetLocalAngles(const EAngle& angAngles)
{
	if (!m_angLocalAngles.IsInitialized())
		m_angLocalAngles = angAngles;

	if (IsInPhysics())
	{
		if (GetMoveParent())
		{
			collision_type_t eCollisionType = GamePhysics()->GetEntityCollisionType(this);
			TAssert(eCollisionType == CT_KINEMATIC || eCollisionType == CT_CHARACTER);
		}

		Matrix4x4 mLocal = m_mLocalTransform;
		mLocal.SetAngles(angAngles);

		Matrix4x4 mGlobal = GetParentGlobalTransform() * mLocal;

		GamePhysics()->SetEntityTransform(this, mGlobal);
	}

	EAngle angDifference = angAngles - m_angLocalAngles;
	if (fabs(angDifference.p) < 0.001f && fabs(angDifference.y) < 0.001f && fabs(angDifference.r) < 0.001f)
		return;

	TMatrix mNew = m_mLocalTransform;
	mNew.SetAngles(angAngles);
	OnSetLocalTransform(mNew);

	m_angLocalAngles = mNew.GetAngles();

	m_mLocalTransform = mNew;
	m_qLocalRotation = Quaternion(mNew);

	InvalidateGlobalTransforms();
}

void CBaseEntity::SetLocalOrigin(const eastl::vector<tstring>& asArgs)
{
	if (asArgs.size() != 3)
	{
		TError("CBaseEntity::SetLocalOrigin with != 3 arguments. Was expecting \"x y z\"\n");
		return;
	}

	SetLocalOrigin(Vector(stof(asArgs[0]), stof(asArgs[1]), stof(asArgs[2])));
}

void CBaseEntity::SetLocalAngles(const eastl::vector<tstring>& asArgs)
{
	if (asArgs.size() != 3)
	{
		TError("CBaseEntity::SetLocalAngles with != 3 arguments. Was expecting \"p y r\"\n");
		return;
	}

	SetLocalAngles(EAngle(stof(asArgs[0]), stof(asArgs[1]), stof(asArgs[2])));
}

CBaseEntity* CBaseEntity::GetEntity(size_t iHandle)
{
	if (iHandle >= GameServer()->GetMaxEntities())
		return NULL;

	return s_apEntityList[iHandle];
}

size_t CBaseEntity::GetNumEntities()
{
	return s_iEntities;
}

void CBaseEntity::SetVisible(const eastl::vector<tstring>& sArgs)
{
	TAssert(sArgs.size());
	if (!sArgs.size())
	{
		TError("CBaseEntity(" + GetName() + "):SetVisible missing a value. Expecting \"On\" or \"Off\"\n");
		return;
	}

	bool bValue = (sArgs[0].comparei("yes") == 0 || sArgs[0].comparei("true") == 0 || sArgs[0].comparei("on") == 0 || stoi(sArgs[0]) != 0);

	SetVisible(bValue);
}

void CBaseEntity::AddToPhysics(collision_type_t eCollisionType)
{
	TAssert(!IsInPhysics());
	if (IsInPhysics())
		return;

	GamePhysics()->AddEntity(this, eCollisionType);
	m_bInPhysics = true;
}

void CBaseEntity::RemoveFromPhysics()
{
	TAssert(IsInPhysics());
	if (!IsInPhysics())
		return;

	m_bInPhysics = false;
	GamePhysics()->RemoveEntity(this);
}

CTeam* CBaseEntity::GetTeam() const
{
	return m_hTeam;
}

void CBaseEntity::SetTeam(class CTeam* pTeam)
{
	m_hTeam = pTeam;
	OnTeamChange();
}

void CBaseEntity::ClientUpdate(int iClient)
{
}

void CBaseEntity::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	if (!m_bTakeDamage)
		return;

	bool bWasAlive = IsAlive();

	m_flLastTakeDamage = GameServer()->GetGameTime();

	if (GameNetwork()->IsHost())
		m_flHealth -= flDamage;

	OnTakeDamage(pAttacker, pInflictor, eDamageType, flDamage, bDirectHit);
	CallOutput("OnTakeDamage");

	Game()->OnTakeDamage(this, pAttacker, pInflictor, flDamage, bDirectHit, !IsAlive() && bWasAlive);

	if (bWasAlive && m_flHealth <= 0)
		Killed(pAttacker);
}

void CBaseEntity::Kill()
{
	if (!IsAlive())
		return;

	if (GameNetwork()->IsHost())
		m_flHealth = -1;

	Killed(NULL);
}

void CBaseEntity::Killed(CBaseEntity* pKilledBy)
{
	m_flTimeKilled = GameServer()->GetGameTime();

	OnKilled(pKilledBy);
	Game()->OnKilled(this);

	CallOutput("OnKilled");
}

void CBaseEntity::SetActive(bool bActive)
{
	if (bActive && !m_bActive)
	{
		OnActivated();
		CallOutput("OnActivated");
	}

	if (m_bActive && !bActive)
	{
		OnDeactivated();
		CallOutput("OnDeactivated");
	}

	m_bActive = bActive;
}

void CBaseEntity::Activate(const eastl::vector<tstring>& sArgs)
{
	SetActive(true);
}

void CBaseEntity::Deactivate(const eastl::vector<tstring>& sArgs)
{
	SetActive(false);
}

void CBaseEntity::ToggleActive(const eastl::vector<tstring>& sArgs)
{
	SetActive(!IsActive());
}

void CBaseEntity::SetActive(const eastl::vector<tstring>& sArgs)
{
	TAssert(sArgs.size());
	if (!sArgs.size())
	{
		TError("CBaseEntity(" + GetName() + "):SetActive missing a value. Expecting \"On\" or \"Off\"\n");
		return;
	}

	bool bValue = (sArgs[0].comparei("yes") == 0 || sArgs[0].comparei("true") == 0 || sArgs[0].comparei("on") == 0 || stoi(sArgs[0]) != 0);

	SetActive(bValue);
}

CVar show_centers("debug_show_centers", "off");

void CBaseEntity::PreRender(bool bTransparent) const
{
	if (ShouldRenderModel() && CModelLibrary::GetModel(m_iModel))
		GameServer()->GetRenderer()->ClassifySceneAreaPosition(CModelLibrary::GetModel(m_iModel));
}

void CBaseEntity::Render(bool bTransparent) const
{
	TPROF("CBaseEntity::Render");

	if (!IsVisible())
		return;

	PreRender(bTransparent);

	do {
		CGameRenderingContext r(GameServer()->GetRenderer(), true);

		// If another context already set this, don't clobber it.
		if (!r.GetActiveFrameBuffer())
			r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		r.Transform(GetRenderTransform());

		ModifyContext(&r, bTransparent);

		if (r.GetAlpha() > 0)
		{
			if (ShouldRenderModel() && m_iModel != (size_t)~0)
			{
				if (r.GetBlend() == BLEND_NONE && !bTransparent)
				{
					TPROF("CRenderingContext::RenderModel(Opaque)");
					r.RenderModel(GetModelID(), this);
				}
				else if (r.GetBlend() != BLEND_NONE && bTransparent)
				{
					TPROF("CRenderingContext::RenderModel(Transparent)");
					r.RenderModel(GetModelID(), this);
				}
			}

			TPROF("CBaseEntity::OnRender");
			OnRender(&r, bTransparent);
		}
	} while (false);

	PostRender(bTransparent);

	if (show_centers.GetBool())
	{
		CRenderingContext r(GameServer()->GetRenderer(), true);
		r.UseProgram("model");
		r.Translate(GetGlobalCenter());
		r.BeginRenderDebugLines();
			r.Vertex(Vector(-1, 0, 0));
			r.Vertex(Vector(1, 0, 0));
		r.EndRender();
		r.BeginRenderDebugLines();
			r.Vertex(Vector(0, -1, 0));
			r.Vertex(Vector(0, 1, 0));
		r.EndRender();
		r.BeginRenderDebugLines();
			r.Vertex(Vector(0, 0, -1));
			r.Vertex(Vector(0, 0, 1));
		r.EndRender();
	}
}

void CBaseEntity::Delete()
{
	GameServer()->Delete(this);
}

void CBaseEntity::Delete(const eastl::vector<tstring>& sArgs)
{
	Delete();
}

void CBaseEntity::CallInput(const eastl::string& sName, const tstring& sArgs)
{
	CEntityInput* pInput = GetInput(sName.c_str());

	if (!pInput)
	{
		TAssert(!"Input missing.");
		TMsg(sprintf(tstring("Input %s not found in %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	eastl::vector<tstring> asArgs;
	tstrtok(sArgs, asArgs);
	pInput->m_pfnCallback(this, asArgs);
}

void CBaseEntity::CallOutput(const eastl::string& sName)
{
	CSaveData* pData = GetSaveData((eastl::string("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->SetEntity(this);
	pOutput->SetOutputName(sName);
	pOutput->Call();
}

void CBaseEntity::AddOutputTarget(const eastl::string& sName, const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs, bool bKill)
{
	CSaveData* pData = GetSaveData((eastl::string("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->AddTarget(sTargetName, sInput, sArgs, bKill);
}

void CBaseEntity::RemoveOutputs(const eastl::string& sName)
{
	CSaveData* pData = GetSaveData((eastl::string("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->Clear();
}

void CBaseEntity::RemoveOutput(const eastl::vector<tstring>& sArgs)
{
	if (sArgs.size() == 0)
	{
		TMsg("RemoveOutput called without a output name argument.\n");
		return;
	}

	RemoveOutputs(convertstring<tchar, char>(sArgs[0]));
}

CVar debug_entity_outputs("debug_entity_outputs", "off");

void CEntityOutput::Call()
{
	if (m_aTargets.size() == 0)
	{
		if (debug_entity_outputs.GetBool())
			TMsg(tstring(m_pEnt->GetClassName()) + "(" + m_pEnt->GetName() + "):" + m_sOutputName + "\n");

		return;
	}

	for (size_t j = 0; j < m_aTargets.size(); j++)
	{
		CEntityOutputTarget* pTarget = &m_aTargets[j];

		if (pTarget->m_sTargetName.length() == 0)
			continue;

		if (pTarget->m_sInput.length() == 0)
			continue;

		eastl::vector<CBaseEntity*> apEntities;
		CBaseEntity::FindEntitiesByName(pTarget->m_sTargetName, apEntities);

		for (size_t i = 0; i < apEntities.size(); i++)
		{
			tstring sFormattedArgs = FormatArgs(pTarget->m_sArgs);
			CBaseEntity* pTargetEntity = apEntities[i];

			if (debug_entity_outputs.GetBool())
				TMsg(tstring(m_pEnt->GetClassName()) + "(" + m_pEnt->GetName() + "):" + m_sOutputName + " -> " + pTargetEntity->GetClassName() + "(" + pTargetEntity->GetName() + "):" + pTarget->m_sInput + " (" + sFormattedArgs + ")\n");

			pTargetEntity->CallInput(pTarget->m_sInput, sFormattedArgs);
		}

		if (!apEntities.size())
			TError("Couldn't find any entity with name '" + pTarget->m_sTargetName + "'\n");
	}

	for (size_t i = 0; i < m_aTargets.size(); i++)
	{
		CEntityOutputTarget* pTarget = &m_aTargets[i];
		if (pTarget->m_bKill)
		{
			m_aTargets.erase(m_aTargets.begin()+i);
			i--;
		}
	}
}

void CEntityOutput::AddTarget(const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs, bool bKill)
{
	CEntityOutputTarget* pTarget = &m_aTargets.push_back();

	pTarget->m_sTargetName = sTargetName;
	pTarget->m_sInput = sInput;
	pTarget->m_sArgs = sArgs;
	pTarget->m_bKill = bKill;
}

void CEntityOutput::Clear()
{
	m_aTargets.clear();
}

tstring CEntityOutput::FormatArgs(tstring sArgs)
{
	size_t iArg = 0;

	while (true)
	{
		tstring sArg = sprintf("[%d]", iArg);
		auto i = sArgs.find(sArg);
		if (i == tstring::npos)
			return sArgs;

		sArgs.replace(i, sArg.length(), m_pEnt->GetOutputValue(m_sOutputName, iArg));

		iArg++;
	}
}

SERVER_GAME_COMMAND(EmitSound)
{
	if (pCmd->GetNumArguments() < 4)
	{
		TMsg("EmitSound with less than 4 arguments.\n");
		return;
	}

	CSoundLibrary::PlaySound(CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(0)), pCmd->Arg(3), pCmd->ArgAsFloat(1), !!pCmd->ArgAsInt(2));
}

void CBaseEntity::EmitSound(const tstring& sFilename, float flVolume, bool bLoop)
{
	::EmitSound.RunCommand(sprintf(tstring("%d %.1f %d %s"), GetHandle(), flVolume, bLoop?1:0, sFilename.c_str()));
}

SERVER_GAME_COMMAND(StopSound)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("StopSound with less than 2 arguments.\n");
		return;
	}

	CSoundLibrary::StopSound(CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(0)), pCmd->Arg(1));
}

void CBaseEntity::StopSound(const tstring& sFilename)
{
	::StopSound.RunCommand(sprintf(tstring("%d %s"), GetHandle(), sFilename.c_str()));
}

bool CBaseEntity::IsSoundPlaying(const tstring& sFilename)
{
	return CSoundLibrary::IsSoundPlaying(this, sFilename);
}

void CBaseEntity::SetSoundVolume(const tstring& sFilename, float flVolume)
{
	CSoundLibrary::SetSoundVolume(this, sFilename, flVolume);
}

TFloat CBaseEntity::Distance(const TVector& vecSpot) const
{
	TFloat flDistance = (GetGlobalCenter() - vecSpot).Length();
	if (flDistance < GetBoundingRadius())
		return 0;

	return flDistance - GetBoundingRadius();
}

void CBaseEntity::SetSpawnSeed(size_t iSpawnSeed)
{
	m_iSpawnSeed = iSpawnSeed;

	mtsrand(iSpawnSeed);
}

SERVER_GAME_COMMAND(ClientSpawn)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("ClientSpawn with no arguments.\n");
		return;
	}

	CEntityHandle<CBaseEntity> hEntity(pCmd->ArgAsUInt(0));

	if (hEntity == NULL)
	{
		TMsg("ClientSpawn with invalid entity.\n");
		return;
	}

	hEntity->ClientSpawn();
}

void CBaseEntity::IssueClientSpawn()
{
	::ClientSpawn.RunCommand(sprintf(tstring("%d"), GetHandle()));
	m_bClientSpawn = true;
}

// ClientSpawn is always guaranteed to run after the client has received all initial data about a new entity.
void CBaseEntity::ClientSpawn()
{
	TAssert(!m_bClientSpawn);
	m_bClientSpawn = true;

	CallOutput("OnSpawn");
}

CSaveData* CBaseEntity::GetSaveData(const char* pszName)
{
	return GetSaveData(GetClassName(), pszName);
}

CSaveData* CBaseEntity::GetSaveDataByHandle(const char* pszHandle)
{
	return GetSaveDataByHandle(GetClassName(), pszHandle);
}

CNetworkedVariableData* CBaseEntity::GetNetworkVariable(const char* pszName)
{
	const tchar* pszClassName = GetClassName();
	CEntityRegistration* pRegistration = NULL;
	
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aNetworkVariables.size(); i++)
		{
			CNetworkedVariableData* pVarData = &pRegistration->m_aNetworkVariables[i];

			if (strcmp(pVarData->m_pszName, pszName) == 0)
				return pVarData;
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CEntityInput* CBaseEntity::GetInput(const char* pszName)
{
	const tchar* pszClassName = GetClassName();
	CEntityRegistration* pRegistration = NULL;
	
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		eastl::map<eastl::string, CEntityInput>::iterator it = pRegistration->m_aInputs.find(pszName);

		if (it != pRegistration->m_aInputs.end())
			return &it->second;

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

void CBaseEntity::CheckSaveDataSize(CEntityRegistration* pRegistration)
{
#ifndef _DEBUG
	return;
#endif

	size_t iSaveTableSize = 0;

	size_t iFirstOffset = 0;
	if (pRegistration->m_aSaveData.size())
		iFirstOffset = pRegistration->m_aSaveData[0].m_iOffset;

	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pData = &pRegistration->m_aSaveData[i];

		if (pData->m_bOverride)
			continue;

		// If bools have non-bools after them then the extra space is padded to retain four-byte alignment.
		// So, round everything up. Might mean adding bools doesn't trigger it, oh well.
		if (pData->m_iSizeOfVariable%4 == 0 && iSaveTableSize%4 != 0)
			iSaveTableSize += 4-iSaveTableSize%4;

		// This can help you find where missing stuff is, if all of the save data is in order.
		// On GCC there's also a problem where a boolean at the end of a parent class can make the beginning address of any child classes be not a multiple of 4, which can cause this to trip. Solution: Keep your booleans near the center of your class definitions. (Really should rewrite this function but meh.)
//		TAssert(pData->m_iOffset - iFirstOffset == iSaveTableSize);

		iSaveTableSize += pData->m_iSizeOfVariable;
	}

	// In case a bool is at the end.
	if (iSaveTableSize%4)
		iSaveTableSize += 4-iSaveTableSize%4;

	size_t iSizeOfThis = SizeOfThis();

	// If you're getting this assert it probably means you forgot to add a savedata entry for some variable that you added to a class.
	if (iSaveTableSize != iSizeOfThis)
	{
		TMsg(sprintf(tstring("Save table for class '%s' doesn't match the class's size, %d != %d.\n"), convertstring<char, tchar>(GetClassName()).c_str(), iSaveTableSize, iSizeOfThis));
//		TAssert(!"Save table size doesn't match class size.\n");
	}
}

void CBaseEntity::CheckTables(const char* pszEntity)
{
#ifndef _DEBUG
	return;
#endif

	CEntityRegistration* pRegistration = GetRegisteredEntity(pszEntity);

	eastl::vector<CSaveData>& aSaveData = pRegistration->m_aSaveData;

	for (size_t i = 0; i < aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &aSaveData[i];
		CNetworkedVariableData* pVariable = GetNetworkVariable(pSaveData->m_pszVariableName);
		if (pSaveData->m_eType == CSaveData::DATA_NETVAR)
			// I better be finding this in the network tables or yer gon have some 'splainin to do!
			TAssert(pVariable)
		else
		{
			// I better NOT be finding this in the network tables or yer gon have some 'splainin to do!
			TAssert(!pVariable);
		}
	}
}

void CBaseEntity::ClientEnterGame()
{
	SetModel(m_iModel);
}

void CBaseEntity::SerializeEntity(std::ostream& o, CBaseEntity* pEntity)
{
	writetstring(o, pEntity->GetClassName());

	size_t iHandle = pEntity->GetHandle();
	o.write((char*)&iHandle, sizeof(iHandle));

	size_t iSpawnSeed = pEntity->GetSpawnSeed();
	o.write((char*)&iSpawnSeed, sizeof(iSpawnSeed));

	pEntity->Serialize(o);
	pEntity->OnSerialize(o);
}

bool CBaseEntity::UnserializeEntity(std::istream& i)
{
	tstring sClassName = readtstring(i);

	size_t iHandle;
	i.read((char*)&iHandle, sizeof(iHandle));

	size_t iSpawnSeed;
	i.read((char*)&iSpawnSeed, sizeof(iSpawnSeed));

	size_t iNewHandle = GameServer()->CreateEntity(sClassName, iHandle, iSpawnSeed);
	TAssert(iNewHandle == iHandle);

	CEntityHandle<CBaseEntity> hEntity(iNewHandle);

	if (!hEntity->Unserialize(i))
		return false;

	return hEntity->OnUnserialize(i);
}

void CBaseEntity::Serialize(std::ostream& o, const char* pszClassName, void* pEntity)
{
	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

	size_t iSaveDataSize = 0;
	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &pRegistration->m_aSaveData[i];
		if (pSaveData->m_eType != CSaveData::DATA_OMIT)
			iSaveDataSize++;
	}

	o.write((char*)&iSaveDataSize, sizeof(iSaveDataSize));

	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &pRegistration->m_aSaveData[i];

		if (pSaveData->m_eType == CSaveData::DATA_OMIT)
			continue;

		o.write((char*)&i, sizeof(i));

		char* pData = (char*)pEntity + pSaveData->m_iOffset;
		switch(pSaveData->m_eType)
		{
		case CSaveData::DATA_COPYTYPE:
			o.write(pData, pSaveData->m_iSizeOfType);
			break;

		case CSaveData::DATA_COPYARRAY:
			o.write(pData, pSaveData->m_iSizeOfVariable);
			break;

		case CSaveData::DATA_COPYVECTOR:
		{
			eastl::vector<size_t>* pVector = (eastl::vector<size_t>*)pData;
			size_t iSize = pVector->size();
			o.write((char*)&iSize, sizeof(iSize));
			if (iSize)
				o.write((char*)pVector->data(), pSaveData->m_iSizeOfType*iSize);
			break;
		}

		case CSaveData::DATA_NETVAR:
		{
			size_t iDataLength;
			CNetworkedVariableBase* pVariable = (CNetworkedVariableBase*)pData;
			char* pRealData = (char*)pVariable->Serialize(iDataLength);
			o.write((char*)&iDataLength, sizeof(iDataLength));
			o.write(pRealData, iDataLength);
			break;
		}

		case CSaveData::DATA_STRING:
			writestring(o, *(eastl::string*)pData);
			break;

		case CSaveData::DATA_STRING16:
			writetstring(o, *(tstring*)pData);
			break;

		case CSaveData::DATA_OUTPUT:
		{
			CEntityOutput* pOutput = (CEntityOutput*)pData;
			size_t iTargets = pOutput->m_aTargets.size();
			o.write((char*)&iTargets, sizeof(iTargets));
			for (size_t i = 0; i < pOutput->m_aTargets.size(); i++)
			{
				CEntityOutput::CEntityOutputTarget* pTarget = &pOutput->m_aTargets[i];
				writestring(o, pTarget->m_sTargetName);
				writestring(o, pTarget->m_sInput);
				writestring(o, pTarget->m_sArgs);
				o.write((char*)&pTarget->m_bKill, sizeof(pTarget->m_bKill));
			}
			break;
		}
		}
	}
}

bool CBaseEntity::Unserialize(std::istream& i, const char* pszClassName, void* pEntity)
{
	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

	size_t iSaveDataSize;
	i.read((char*)&iSaveDataSize, sizeof(iSaveDataSize));

	for (size_t j = 0; j < iSaveDataSize; j++)
	{
		size_t iSaveData;
		i.read((char*)&iSaveData, sizeof(iSaveData));

		CSaveData* pSaveData = &pRegistration->m_aSaveData[iSaveData];

		TAssert(pSaveData->m_eType != CSaveData::DATA_OMIT);

		char* pData = (char*)pEntity + pSaveData->m_iOffset;
		switch(pSaveData->m_eType)
		{
		case CSaveData::DATA_COPYTYPE:
			i.read(pData, pSaveData->m_iSizeOfType);
			break;

		case CSaveData::DATA_COPYARRAY:
			i.read(pData, pSaveData->m_iSizeOfVariable);
			break;

		case CSaveData::DATA_COPYVECTOR:
		{
			eastl::vector<size_t>* pVector = (eastl::vector<size_t>*)pData;
			size_t iSize;
			i.read((char*)&iSize, sizeof(iSize));
			if (iSize)
			{
				pSaveData->m_pfnResizeVector(pData, iSize);
				i.read((char*)pVector->data(), pSaveData->m_iSizeOfType*iSize);
			}
			break;
		}

		case CSaveData::DATA_NETVAR:
		{
			size_t iDataLength;
			i.read((char*)&iDataLength, sizeof(iDataLength));

			CNetworkedVariableBase* pVariable = (CNetworkedVariableBase*)pData;
			char* pRealData = new char[iDataLength];
			i.read(pRealData, iDataLength);
			pVariable->Unserialize(iDataLength, pRealData);
			delete[] pRealData;
			break;
		}

		case CSaveData::DATA_STRING:
			((eastl::string*)pData)->assign(readstring(i));
			break;

		case CSaveData::DATA_STRING16:
			((tstring*)pData)->assign(readtstring(i));
			break;

		case CSaveData::DATA_OUTPUT:
		{
			CEntityOutput* pOutput = (CEntityOutput*)pData;
			size_t iTargets = 0;
			i.read((char*)&iTargets, sizeof(iTargets));
			for (size_t j = 0; j < iTargets; j++)
			{
				bool bKill;

				eastl::string sTargetName = readstring(i);
				eastl::string sInput = readstring(i);
				eastl::string sArgs = readstring(i);
				i.read((char*)&bKill, sizeof(bool));

				pOutput->AddTarget(sTargetName, sInput, sArgs, bKill);
			}
			break;
		}
		}
	}

	return true;
}

void CBaseEntity::PrecacheModel(const tstring& sModel)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sModel)
			return;
	}

	if (CModelLibrary::AddModel(sModel) == ~0)
	{
		TError("Model \"" + sModel + "\" could not be loaded.");
		return;
	}

	pReg->m_asPrecaches.push_back(sModel);
}

void CBaseEntity::PrecacheParticleSystem(const tstring& sSystem)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sSystem)
			return;
	}

	size_t iSystem = CParticleSystemLibrary::Get()->FindParticleSystem(sSystem);
	CParticleSystemLibrary::Get()->LoadParticleSystem(iSystem);

	pReg->m_asPrecaches.push_back(sSystem);
}

void CBaseEntity::PrecacheSound(const tstring& sSound)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sSound)
			return;
	}

	CSoundLibrary::Get()->AddSound(sSound);

	pReg->m_asPrecaches.push_back(sSound);
}

void CBaseEntity::PrecacheTexture(const tstring& sTexture)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sTexture)
			return;
	}

	CTextureLibrary::AddTexture(sTexture);

	pReg->m_asPrecaches.push_back(sTexture);
}

eastl::map<tstring, CEntityRegistration>& CBaseEntity::GetEntityRegistration()
{
	static eastl::map<tstring, CEntityRegistration> aEntityRegistration;
	return aEntityRegistration;
}

void CBaseEntity::RegisterEntity(const char* pszClassName, const char* pszParentClass, EntityRegisterCallback pfnRegisterCallback, EntityPrecacheCallback pfnPrecacheCallback, EntityCreateCallback pfnCreateCallback)
{
	CEntityRegistration* pEntity = &GetEntityRegistration()[pszClassName];
	pEntity->m_pszEntityClass = pszClassName;
	pEntity->m_pszParentClass = pszParentClass;
	pEntity->m_pfnRegisterCallback = pfnRegisterCallback;
	pEntity->m_pfnPrecacheCallback = pfnPrecacheCallback;
	pEntity->m_pfnCreateCallback = pfnCreateCallback;
}

void CBaseEntity::Register(CBaseEntity* pEntity)
{
	pEntity->RegisterSaveData();
	pEntity->RegisterNetworkVariables();
	pEntity->RegisterInputData();
}

void CBaseEntity::PrecacheCallback(CBaseEntity* pEntity)
{
	pEntity->Precache();
}

size_t CBaseEntity::GetNumEntitiesRegistered()
{
	return GetEntityRegistration().size();
}

CEntityRegistration* CBaseEntity::GetEntityRegistration(size_t iEntity)
{
	if (iEntity >= GetNumEntitiesRegistered())
		return nullptr;

	// Not the fastest implementation but I don't think it needs to be.
	size_t i = 0;
	for (auto it = GetEntityRegistration().begin(); it != GetEntityRegistration().end(); it++, i++)
	{
		if (i == iEntity)
			return &it->second;
	}

	TAssert(false);	 // Dunno how this could happen.
	return nullptr;
}

CSaveData* CBaseEntity::GetSaveData(const char* pszClassName, const char* pszName)
{
	CEntityRegistration* pRegistration;
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			CSaveData* pVarData = &pRegistration->m_aSaveData[i];

			if (strcmp(pVarData->m_pszVariableName, pszName) == 0)
				return pVarData;
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CSaveData* CBaseEntity::GetSaveDataByHandle(const char* pszClassName, const char* pszHandle)
{
	CEntityRegistration* pRegistration;
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			CSaveData* pVarData = &pRegistration->m_aSaveData[i];

			if (!pVarData->m_pszHandle)
				continue;

			if (strcmp(pVarData->m_pszHandle, pszHandle) == 0)
				return pVarData;
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CSaveData* CBaseEntity::GetOutput(const char* pszClassName, const tstring& sOutput)
{
	return GetSaveData(pszClassName, ("m_Output_" + sOutput).c_str());
}

CEntityRegistration* CBaseEntity::GetRegisteredEntity(tstring sClassName)
{
	if (GetEntityRegistration().find(sClassName) == GetEntityRegistration().end())
		return NULL;

	return &GetEntityRegistration()[sClassName];
}

CBaseEntity* CBaseEntity::GetEntityByName(const eastl::string& sName)
{
	if (sName.length() == 0)
		return NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (sName[0] == '*')
		{
			if (eastl::string(pEntity->GetClassName()+1) == sName.c_str()+1)
				return pEntity;
		}
		else
		{
			if (pEntity->GetName() == sName)
				return pEntity;
		}
	}

	return NULL;
}

void CBaseEntity::FindEntitiesByName(const eastl::string& sName, eastl::vector<CBaseEntity*>& apEntities)
{
	if (sName.length() == 0)
		return;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (sName[0] == '*')
		{
			if (eastl::string(pEntity->GetClassName()+1) != sName.c_str()+1)
				continue;
		}
		else
		{
			if (pEntity->GetName() != sName)
				continue;
		}

		apEntities.push_back(pEntity);
	}
}

bool UnserializeString_bool(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	return (sData.comparei("yes") == 0 || sData.comparei("true") == 0 || sData.comparei("on") == 0 || stoi(sData) != 0);
}

size_t UnserializeString_size_t(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	return stoi(sData);
}

TVector UnserializeString_TVector(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 3);
	if (asTokens.size() != 3)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for a vector (Format: \"x y z\")\n");
		return TVector();
	}

	return Vector(stof(asTokens[0]), stof(asTokens[1]), stof(asTokens[2]));
}

EAngle UnserializeString_EAngle(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 3);
	if (asTokens.size() != 3)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for an angle (Format: \"p y r\")\n");
		return EAngle();
	}

	return EAngle(stof(asTokens[0]), stof(asTokens[1]), stof(asTokens[2]));
}

AABB UnserializeString_AABB(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 6);
	if (asTokens.size() != 6)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for an AABB (Format: \"x y z x y z\")\n");
		return AABB();
	}

	return AABB(Vector(stof(asTokens[0]), stof(asTokens[1]), stof(asTokens[2])), Vector(stof(asTokens[3]), stof(asTokens[4]), stof(asTokens[5])));
}

void UnserializeString_bool(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	bool bValue = UnserializeString_bool(sData);

	bool* pData = (bool*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = bValue;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedVariable<bool>* pVariable = (CNetworkedVariable<bool>*)pData;
		(*pVariable) = bValue;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_int(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	TAssert(false);
}

void UnserializeString_size_t(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	TAssert(false);

	size_t i = UnserializeString_size_t(sData);

	size_t* pData = (size_t*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = i;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedVariable<size_t>* pVariable = (CNetworkedVariable<size_t>*)pData;
		(*pVariable) = i;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_float(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	float f = stof(sData);

	float* pData = (float*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = f;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TAssert(false);
		CNetworkedVariable<float>* pVariable = (CNetworkedVariable<float>*)pData;
		(*pVariable) = f;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_tstring(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	tstring* psData = (tstring*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_STRING:
		*psData = sData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TAssert(false);
		CNetworkedString* pVariable = (CNetworkedString*)psData;
		(*pVariable) = sData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_COPYTYPE:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_TVector(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	Vector vecData = UnserializeString_TVector(sData, pEntity->GetName(), pEntity->GetClassName(), pSaveData->m_pszHandle);

	Vector* pData = (Vector*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		TAssert(false);
		*pData = vecData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TAssert(false);
		CNetworkedVector* pVariable = (CNetworkedVector*)pData;
		(*pVariable) = vecData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_Vector(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	UnserializeString_TVector(sData, pSaveData, pEntity);
}

void UnserializeString_EAngle(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	TAssert(false);
}

void UnserializeString_Matrix4x4(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 6);
	if (asTokens.size() != 6)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") wrong number of arguments for a matrix (Format: \"x y z p y r\")\n");
		return;
	}

	Vector vecData(stof(asTokens[0]), stof(asTokens[1]), stof(asTokens[2]));
	EAngle angData(stof(asTokens[3]), stof(asTokens[4]), stof(asTokens[5]));
	Matrix4x4 mData(angData, vecData);

	Matrix4x4* pData = (Matrix4x4*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = mData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TAssert(false);
		CNetworkedVariable<Matrix4x4>* pVariable = (CNetworkedVariable<Matrix4x4>*)pData;
		(*pVariable) = mData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_AABB(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 6);
	if (asTokens.size() != 6)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") wrong number of arguments for an AABB (Format: \"x y z x y z\")\n");
		return;
	}

	AABB aabbData(Vector(stof(asTokens[0]), stof(asTokens[1]), stof(asTokens[2])), Vector(stof(asTokens[3]), stof(asTokens[4]), stof(asTokens[5])));

	AABB* pData = (AABB*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = aabbData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TAssert(false);
		CNetworkedVariable<AABB>* pVariable = (CNetworkedVariable<AABB>*)pData;
		(*pVariable) = aabbData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_ModelID(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	size_t iID = CModelLibrary::AddModel(sData);

	TAssert(iID != ~0);
	if (iID == ~0)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find or load model '" + sData + "'\n");
		return;
	}

	pEntity->SetModel(iID);
}

void UnserializeString_EntityHandle(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CBaseEntity* pNamedEntity = CBaseEntity::GetEntityByName(sData);

	TAssert(pNamedEntity);
	if (!pNamedEntity)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find entity named '" + sData + "'\n");
		return;
	}

	CEntityHandle<CBaseEntity> hData(pNamedEntity);

	CEntityHandle<CBaseEntity>* pData = (CEntityHandle<CBaseEntity>*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		TAssert(false);
		*pData = hData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedHandle<CBaseEntity>* pVariable = (CNetworkedHandle<CBaseEntity>*)pData;
		(*pVariable) = hData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_STRING16:
	case CSaveData::DATA_OUTPUT:
		TAssert(false);
		break;
	}
}

void UnserializeString_LocalOrigin(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	Vector vecData = UnserializeString_TVector(sData, pEntity->GetName(), pEntity->GetClassName(), pSaveData->m_pszHandle);
	pEntity->SetLocalOrigin(vecData);
}

void UnserializeString_LocalAngles(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 3);
	if (asTokens.size() != 3)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") wrong number of arguments for a vector\n");
		return;
	}

	EAngle angData(stof(asTokens[0]), stof(asTokens[1]), stof(asTokens[2]));
	pEntity->SetLocalAngles(angData);
}

void UnserializeString_MoveParent(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CBaseEntity* pNamedEntity = CBaseEntity::GetEntityByName(sData);

	TAssert(pNamedEntity);
	if (!pNamedEntity)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find entity named '" + sData + "'\n");
		return;
	}

	pEntity->SetMoveParent(pNamedEntity);
}