#include "corpse.h"

#include "tack_game.h"

REGISTER_ENTITY(CCorpse);

NETVAR_TABLE_BEGIN(CCorpse);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCorpse);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, special_ability_t, m_eAbility);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCorpse);
INPUTS_TABLE_END();

void CCorpse::Think()
{
	BaseClass::Think();

	EAngle a = GetLocalAngles();
	a.p = AngleApproach(90, a.p, 180*GameServer()->GetFrameTime());
	SetLocalAngles(a);

	float flDistanceSqr = (TackGame()->GetLocalPlayerCharacter()->GetGlobalOrigin() - GetGlobalOrigin()).LengthSqr();
	if (flDistanceSqr > 20*20)
		Delete();
}
