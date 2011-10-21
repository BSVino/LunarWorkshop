#include "zombie.h"

#include "../tack_game.h"

REGISTER_ENTITY(CZombie);

NETVAR_TABLE_BEGIN(CZombie);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CZombie);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flGoalYaw);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRenderYaw);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CZombie);
INPUTS_TABLE_END();

void CZombie::Precache()
{
	PrecacheModel("models/characters/zombie.obj", false);
}

void CZombie::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/characters/zombie.obj");
}

void CZombie::ZombieThink()
{
	m_vecGoalVelocity = Vector();

	CTackCharacter* pPlayerCharacter = TackGame()->GetLocalPlayerCharacter();

	if (!pPlayerCharacter)
		return;

	float flDistanceSqr = (pPlayerCharacter->GetGlobalOrigin() - GetGlobalOrigin()).LengthSqr();
	if (flDistanceSqr > 5*5)
		return;

	m_vecGoalVelocity = (pPlayerCharacter->GetGlobalOrigin() - GetGlobalOrigin()).Normalized();

	if (flDistanceSqr < 0.3f*0.3f)
		Attack();
}

void CZombie::Think()
{
	ZombieThink();

	BaseClass::Think();
}
