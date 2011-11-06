#ifndef CHAIN_PLAYER_H
#define CHAIN_PLAYER_H

#include <tengine/game/entities/player.h>

class CChainPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CChainPlayer, CPlayer);

public:
	void							MouseInput(int iButton, int iState);

	class CPlayerCharacter*			GetPlayerCharacter();
};

#endif
