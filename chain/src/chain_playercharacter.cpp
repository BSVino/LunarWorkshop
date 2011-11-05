#include "chain_playercharacter.h"

#include <tinker/application.h>
#include <game/physics.h>

REGISTER_ENTITY(CPlayerCharacter);

NETVAR_TABLE_BEGIN(CPlayerCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayerCharacter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerCharacter);
INPUTS_TABLE_END();

void CPlayerCharacter::Precache()
{
}

void CPlayerCharacter::Spawn()
{
	BaseClass::Spawn();
}
