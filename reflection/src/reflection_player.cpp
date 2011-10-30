#include "reflection_player.h"

#include <tengine/game/entities/character.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>

#include "reflection_playercharacter.h"

REGISTER_ENTITY(CReflectionPlayer);

NETVAR_TABLE_BEGIN(CReflectionPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionPlayer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionPlayer);
INPUTS_TABLE_END();

CPlayerCharacter* CReflectionPlayer::GetPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(m_hCharacter.GetPointer());
}
