#ifndef CHAIN_PLAYER_H
#define CHAIN_PLAYER_H

#include <tengine/game/entities/player.h>

class CChainPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CChainPlayer, CPlayer);

public:
	class CPlayerCharacter*			GetPlayerCharacter();
};

#endif
