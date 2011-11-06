#include "chain_player.h"

#include <tengine/game/entities/character.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>

#include "chain_playercharacter.h"
#include "chain_game.h"
#include "story.h"

REGISTER_ENTITY(CChainPlayer);

NETVAR_TABLE_BEGIN(CChainPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CChainPlayer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CChainPlayer);
INPUTS_TABLE_END();

void CChainPlayer::MouseInput(int iButton, int iState)
{
	BaseClass::MouseInput(iButton, iState);

	CStory* pStory = ChainGame()->GetStory();
	if (!pStory)
		return;

	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == 1)
		pStory->NextPage();
}

CPlayerCharacter* CChainPlayer::GetPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(m_hCharacter.GetPointer());
}
