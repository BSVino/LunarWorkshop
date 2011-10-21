#include "tack_playercharacter.h"

#include <tinker/application.h>

REGISTER_ENTITY(CPlayerCharacter);

NETVAR_TABLE_BEGIN(CPlayerCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayerCharacter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerCharacter);
INPUTS_TABLE_END();

void CPlayerCharacter::Precache()
{
	PrecacheModel("models/characters/tack.obj", false);
}

void CPlayerCharacter::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/characters/tack.obj");
}
