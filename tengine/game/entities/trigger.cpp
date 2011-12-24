#include "trigger.h"

#include <physics/physics.h>

REGISTER_ENTITY(CTrigger);

NETVAR_TABLE_BEGIN(CTrigger);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTrigger);
	SAVEDATA_DEFINE_OUTPUT(OnStartTouch);
	SAVEDATA_DEFINE_OUTPUT(OnEndTouch);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahTouching);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahLastTouching);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTrigger);
INPUTS_TABLE_END();

void CTrigger::Spawn()
{
	BaseClass::Spawn();

	AddToPhysics(CT_TRIGGER);
}

void CTrigger::OnSetModel()
{
	BaseClass::OnSetModel();

	// In case the model has changed.
	if (IsInPhysics())
		RemoveFromPhysics();

	AddToPhysics(CT_TRIGGER);
}

void CTrigger::Think()
{
	BaseClass::Think();
}

void CTrigger::Touching(const CBaseEntity* pOther)
{
	for (size_t i = 0; i < m_ahLastTouching.size(); i++)
	{
		if (m_ahLastTouching[i] == pOther)
		{
			// We were touching before and we still are. Great.
			m_ahTouching.push_back(pOther);
			m_ahLastTouching.erase(m_ahLastTouching.begin()+i);
			return;
		}
	}

	// Not in the LostTouching list, so it must be a new touch.
	StartTouch(pOther);
	m_ahTouching.push_back(pOther);
}

void CTrigger::BeginTouchingList()
{
	m_ahLastTouching.clear();
	m_ahTouching.swap(m_ahLastTouching);
}

void CTrigger::EndTouchingList()
{
	// Anybody still in the LastTouching list is no longer touching.
	for (size_t i = 0; i < m_ahLastTouching.size(); i++)
	{
		if (!m_ahLastTouching[i])
			continue;

		EndTouch(m_ahLastTouching[i]);
	}
}

void CTrigger::StartTouch(const CBaseEntity* pOther)
{
	CallOutput("OnStartTouch");
}

void CTrigger::EndTouch(const CBaseEntity* pOther)
{
	CallOutput("OnEndTouch");
}
