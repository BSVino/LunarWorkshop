#ifndef CHAIN_PLAYER_H
#define CHAIN_PLAYER_H

#include <tengine/game/entities/player.h>

class CChainPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CChainPlayer, CPlayer);

public:
	void							MouseInput(int iButton, tinker_mouse_state_t iState);

	class CPlayerCharacter*			GetPlayerCharacter();
};

#endif
