#ifndef REFLECTION_PLAYER_H
#define REFLECTION_PLAYER_H

#include <tengine/game/entities/player.h>

class CReflectionPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CReflectionPlayer, CPlayer);

public:
	class CPlayerCharacter*			GetPlayerCharacter();
};

#endif
