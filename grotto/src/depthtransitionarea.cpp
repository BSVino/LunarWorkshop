#include "depthtransitionarea.h"

#include "grotto_character.h"

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
