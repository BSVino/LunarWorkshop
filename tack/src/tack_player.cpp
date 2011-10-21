#include "tack_player.h"

#include <tengine/game/entities/character.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>

#include "characters/tack_playercharacter.h"

REGISTER_ENTITY(CTackPlayer);

NETVAR_TABLE_BEGIN(CTackPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTackPlayer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTackPlayer);
INPUTS_TABLE_END();

void CTackPlayer::MouseMotion(int x, int y)
{
}

void CTackPlayer::KeyPress(int c)
{
	if (c == ' ')
		GetPlayerCharacter()->Attack();
	else
		BaseClass::KeyPress(c);
}

CPlayerCharacter* CTackPlayer::GetPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(m_hCharacter.GetPointer());
}
