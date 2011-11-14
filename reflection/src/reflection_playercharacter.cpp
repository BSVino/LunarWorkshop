#include "reflection_playercharacter.h"

#include <tinker/application.h>
#include <physics/physics.h>

#include "mirror.h"
#include "token.h"

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
	// REMOVE ME
	// Here I precache entities that I know I'll need but can't precache elsewhere.
	PrecacheModel("models/door.obj");
	PrecacheModel("models/vaultdoor.obj");
}

void CPlayerCharacter::Spawn()
{
	BaseClass::Spawn();

	SetMass(60);
	m_aabbBoundingBox = AABB(Vector(-0.35f, 0, -0.35f), Vector(0.35f, 2, 0.35f));

	AddToPhysics(CT_CHARACTER);
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

void CPlayerCharacter::FindItems()
{
	CToken* pToken = NULL;
	float flTokenRadius = 1.5f;

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t j = 0; j < iMaxEntities; j++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(j);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (!pEntity->IsVisible())
			continue;

		if (pEntity == this)
			continue;

		if (pEntity == m_hToken)
			continue;

		TFloat flRadius = pEntity->GetBoundingRadius() + flTokenRadius;
		flRadius = flRadius*flRadius;
		if ((GetGlobalCenter() - pEntity->GetGlobalCenter()).LengthSqr() > flRadius)
			continue;

		CReceptacle* pReceptacle = dynamic_cast<CReceptacle*>(pEntity);
		if (pReceptacle && !pReceptacle->GetToken() && m_hToken != nullptr)
		{
			pToken = m_hToken;
			DropToken();
			pReceptacle->SetToken(pToken);
			return;
		}

		if (pReceptacle && pReceptacle->GetToken() && m_hToken == nullptr)
		{
			pToken = pReceptacle->GetToken();
			pReceptacle->SetToken(nullptr);
			PickUpToken(pToken);
			return;
		}

		pToken = dynamic_cast<CToken*>(pEntity);

		if (pToken && !pToken->GetReceptacle())
		{
			PickUpToken(pToken);
			return;
		}
	}

	// If we couldn't find any tokens or receptacles, just drop our token.
	DropToken();
}

void CPlayerCharacter::DropToken()
{
	if (m_hToken == nullptr)
		return;

	m_hToken->SetMoveParent(nullptr);
	m_hToken->SetGlobalOrigin(GetGlobalOrigin() + AngleVector(GetViewAngles()).Flattened().Normalized());
	m_hToken->SetGlobalAngles(EAngle(0, GetViewAngles().y, (IsReflected(REFLECTION_VERTICAL)?180.0f:0.0f)));
	m_hToken->SetVisible(true);
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
}
