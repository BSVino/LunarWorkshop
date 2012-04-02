#include "grotto_playercharacter.h"

#include <tinker/application.h>
#include <physics/physics.h>
#include <renderer/game_renderingcontext.h>

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
	PrecacheModel("models/pedestal.toy");
}

void CPlayerCharacter::Spawn()
{
	Precache();

	SetModel("models/pedestal.toy");

	SetMass(60);
	m_aabbBoundingBox = AABB(Vector(-0.35f, 0, -0.35f), Vector(0.35f, 2, 0.35f));

	SetGlobalGravity(Vector(0, -9.8f, 0)*2);

	BaseClass::Spawn();
}

void CPlayerCharacter::PreRender(bool bTransparent) const
{
	BaseClass::PreRender(bTransparent);
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
	float flTokenRadius = 2.5f;

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

		TFloat flRadius = flTokenRadius*flTokenRadius;

		CReceptacle* pReceptacle = dynamic_cast<CReceptacle*>(pEntity);
		if (pReceptacle)
		{
			if (!pReceptacle->IsActive())
				continue;

			if ((GetGlobalCenter() - pReceptacle->GetTokenPosition()).LengthSqr() > flRadius)
				continue;

			if (pReceptacle->GetToken() && m_hToken != nullptr)
			{
				pToken = m_hToken;
				DropToken();
				CToken* pOther = pReceptacle->GetToken();
				pReceptacle->SetToken(pToken);
				PickUpToken(pOther);
				return;
			}

			if (!pReceptacle->GetToken() && m_hToken != nullptr)
			{
				pToken = m_hToken;
				DropToken();
				pReceptacle->SetToken(pToken);
				return;
			}

			if (pReceptacle->GetToken() && m_hToken == nullptr)
			{
				pToken = pReceptacle->GetToken();
				pReceptacle->SetToken(nullptr);
				PickUpToken(pToken);
				return;
			}
		}

		pToken = dynamic_cast<CToken*>(pEntity);
		if (pToken && !pToken->GetReceptacle())
		{
			if (pToken->GetReceptacle() && !pToken->GetReceptacle()->IsActive())
				continue;

			if ((GetGlobalCenter() - pToken->GetGlobalCenter()).LengthSqr() > flRadius)
				continue;

			if (m_hToken != nullptr)
				DropToken();
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

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CKaleidobeast* pKaleidobeast = dynamic_cast<CKaleidobeast*>(CBaseEntity::GetEntity(i));
		if (!pKaleidobeast)
			continue;

		pKaleidobeast->LosePlayer();
	}
}

void CPlayerCharacter::GoIntoScreen()
{
	m_iDepthLevel++;

	Vector vecPosition = GetGlobalOrigin();
	vecPosition.x = (float)(m_iDepthLevel * METERS_PER_DEPTH);
	SetGlobalOrigin(vecPosition);
}

void CPlayerCharacter::GoOutOfScreen()
{
	m_iDepthLevel--;

	Vector vecPosition = GetGlobalOrigin();
	vecPosition.x = (float)(m_iDepthLevel * METERS_PER_DEPTH);
	SetGlobalOrigin(vecPosition);
}

void CPlayerCharacter::FlipScreen()
{
	SetGlobalAngles(GetGlobalAngles() + EAngle(0, 180, 0));
	SetViewAngles(GetViewAngles() + EAngle(0, 180, 0));
}
