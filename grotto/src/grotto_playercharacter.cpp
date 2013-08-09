#include "grotto_playercharacter.h"

#include <tinker/application.h>
#include <physics/physics.h>
#include <game/entities/charactercamera.h>

#include "mirror.h"
#include "token.h"
#include "receptacle.h"
#include "kaleidobeast.h"

REGISTER_ENTITY(CPlayerCharacter);

NETVAR_TABLE_BEGIN(CPlayerCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayerCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CMirror>, m_hMirror);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CToken>, m_hToken);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerCharacter);
INPUTS_TABLE_END();

void CPlayerCharacter::Precache()
{
}

void CPlayerCharacter::Spawn()
{
	SetMass(60);
	m_aabbPhysBoundingBox = AABB(Vector(-0.35f, -0.35f, 0), Vector(0.35f, 0.35f, 2));

	SetGlobalGravity(Vector(0, 0, -9.8f)*2);

	BaseClass::Spawn();

	m_hCamera = GameServer()->Create<CCharacterCamera>("CGrottoCamera");
	m_hCamera->SetCharacter(this);
}

void CPlayerCharacter::PlaceMirror(mirror_t eMirror)
{
	if (m_hMirror != NULL)
		m_hMirror->Delete();

	CMirror* pMirror = m_hMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetMirrorType(eMirror);

	Vector vecDistance = AngleVector(GetViewAngles()).Flattened();
	if (pMirror->GetMirrorType() == MIRROR_HORIZONTAL)
		vecDistance *= 2.5f;
	else
		vecDistance *= 1.5f;

	pMirror->SetGlobalOrigin(GetGlobalOrigin() + vecDistance);
	pMirror->SetGlobalAngles(EAngle(0, GetViewAngles().y, (IsReflected(REFLECTION_VERTICAL)?180.0f:0.0f)));

	if (IsReflected(REFLECTION_ANY))
		m_hMirrorInside = pMirror;

	if (IsReflected(REFLECTION_LATERAL))
		m_mLateralReflection.SetReflection(pMirror->GetGlobalTransform().GetForwardVector());
}

CMirror* CPlayerCharacter::GetMirror() const
{
	return m_hMirror;
}

CBaseEntity* CPlayerCharacter::Use()
{
	CBaseEntity* pUseItem = BaseClass::Use();

	if (!pUseItem)
		DropToken();

	return pUseItem;
}

void CPlayerCharacter::DropToken()
{
	if (!m_hToken)
		return;

	m_hToken->SetMoveParent(nullptr);
	m_hToken->SetGlobalOrigin(GetGlobalOrigin() + AngleVector(GetViewAngles()).Flattened().Normalized() + GetUpVector() * 0.1f);
	m_hToken->SetGlobalAngles(EAngle(0, GetViewAngles().y, (IsReflected(REFLECTION_VERTICAL)?180.0f:0.0f)));
	m_hToken->SetVisible(true);
	m_hToken->SetUsable(true);
	m_hToken = nullptr;
}

void CPlayerCharacter::PickUpToken(CToken* pToken)
{
	if (m_hToken != nullptr)
		return;

	m_hToken = pToken;

	pToken->SetVisible(false);
	pToken->SetMoveParent(this);
	pToken->SetLocalOrigin(Vector());
	pToken->SetUsable(false);
}

CToken* CPlayerCharacter::GetToken() const
{
	return m_hToken;
}

void CPlayerCharacter::Reflected(reflection_t eReflectionType)
{
	BaseClass::Reflected(eReflectionType);

	if (m_hToken != nullptr)
		m_hToken->Reflected(eReflectionType);

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CKaleidobeast* pKaleidobeast = dynamic_cast<CKaleidobeast*>(CBaseEntity::GetEntity(i));
		if (!pKaleidobeast)
			continue;

		pKaleidobeast->LosePlayer();
	}
}
