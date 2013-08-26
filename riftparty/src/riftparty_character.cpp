#include "riftparty_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>
#include <physics/physics.h>

#include "riftparty_game.h"
#include "riftparty_renderer.h"
#include "riftparty_playercharacter.h"

REGISTER_ENTITY(CRiftPartyCharacter);

NETVAR_TABLE_BEGIN(CRiftPartyCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CRiftPartyCharacter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CRiftPartyCharacter);
INPUTS_TABLE_END();

CRiftPartyCharacter::CRiftPartyCharacter()
{
}

float CRiftPartyCharacter::EyeHeight() const
{
	return 1.65f;
}
