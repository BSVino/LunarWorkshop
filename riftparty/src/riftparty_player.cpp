#include "riftparty_player.h"

#include <tengine/game/entities/character.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>

#include "riftparty_playercharacter.h"

REGISTER_ENTITY(CRiftPartyPlayer);

NETVAR_TABLE_BEGIN(CRiftPartyPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CRiftPartyPlayer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CRiftPartyPlayer);
INPUTS_TABLE_END();

void CRiftPartyPlayer::Spawn()
{
}

CPlayerCharacter* CRiftPartyPlayer::GetPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(m_hCharacter.GetPointer());
}

