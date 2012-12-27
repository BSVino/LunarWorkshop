#include "depthtransitionarea.h"

#include "grotto_character.h"
#include "grotto_playercharacter.h"

REGISTER_ENTITY(CDepthTransitionArea);

NETVAR_TABLE_BEGIN(CDepthTransitionArea);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CDepthTransitionArea);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTarget, "Target");
	SAVEDATA_EDITOR_VARIABLE("Target");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDepthTransitionArea);
INPUTS_TABLE_END();

void CDepthTransitionArea::OnStartTouch(CBaseEntity* pOther)
{
	CGrottoCharacter* pCharacter = dynamic_cast<CGrottoCharacter*>(pOther);
	if (!pCharacter)
		return;

	pCharacter->SetTouchingDepthTransitionArea(this, true);
}

void CDepthTransitionArea::OnEndTouch(CBaseEntity* pOther)
{
	CGrottoCharacter* pCharacter = dynamic_cast<CGrottoCharacter*>(pOther);
	if (!pCharacter)
		return;

	pCharacter->SetTouchingDepthTransitionArea(this, false);
}

bool CDepthTransitionArea::IsValid() const
{
	return !!m_hTarget;
}

Vector CDepthTransitionArea::GetDestination() const
{
	return m_hTarget->GetGlobalOrigin();
}

REGISTER_ENTITY(CAutoDepthTransitionArea);

NETVAR_TABLE_BEGIN(CAutoDepthTransitionArea);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CAutoDepthTransitionArea);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahCharacters);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTarget1, "Target1");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTarget2, "Target2");
	SAVEDATA_EDITOR_VARIABLE("Target1");
	SAVEDATA_EDITOR_VARIABLE("Target2");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CAutoDepthTransitionArea);
INPUTS_TABLE_END();

void CAutoDepthTransitionArea::OnStartTouch(CBaseEntity* pOther)
{
	CPlayerCharacter* pCharacter = dynamic_cast<CPlayerCharacter*>(pOther);
	if (!pCharacter)
		return;

	m_ahCharacters.push_back(pCharacter);
}

void CAutoDepthTransitionArea::OnEndTouch(CBaseEntity* pOther)
{
	CPlayerCharacter* pCharacter = dynamic_cast<CPlayerCharacter*>(pOther);
	if (!pCharacter)
		return;

	m_ahCharacters.erase_value(pCharacter);
}

void CAutoDepthTransitionArea::Think()
{
	BaseClass::Think();

	if (!IsValid())
		return;

	for (size_t i = 0; i < m_ahCharacters.size(); i++)
	{
		CPlayerCharacter* pPlayerCharacter = m_ahCharacters[i];

		float flViewYaw = AngleNormalize(pPlayerCharacter->GetViewAngles().y);

		if (fabs(flViewYaw) > 90 && fabs(pPlayerCharacter->GetGlobalOrigin().x - m_hTarget1->GetGlobalOrigin().x) > 0.1f)
		{
			Vector vecNewOrigin = pPlayerCharacter->GetGlobalOrigin();
			vecNewOrigin.x = m_hTarget1->GetGlobalOrigin().x;
			pPlayerCharacter->SetGlobalOrigin(vecNewOrigin);
		}
		else if (fabs(flViewYaw) < 90 && fabs(pPlayerCharacter->GetGlobalOrigin().x - m_hTarget2->GetGlobalOrigin().x) > 0.1f)
		{
			Vector vecNewOrigin = pPlayerCharacter->GetGlobalOrigin();
			vecNewOrigin.x = m_hTarget2->GetGlobalOrigin().x;
			pPlayerCharacter->SetGlobalOrigin(vecNewOrigin);
		}
	}
}

bool CAutoDepthTransitionArea::IsValid() const
{
	return !!m_hTarget1 && !!m_hTarget2;
}

Vector CAutoDepthTransitionArea::GetDestination1() const
{
	return m_hTarget1->GetGlobalOrigin();
}

Vector CAutoDepthTransitionArea::GetDestination2() const
{
	return m_hTarget2->GetGlobalOrigin();
}
