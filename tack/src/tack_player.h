#ifndef TACK_PLAYER_H
#define TACK_PLAYER_H

#include <tengine/game/entities/player.h>

class CTackPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CTackPlayer, CPlayer);

public:
	virtual void					MouseMotion(int x, int y);
	virtual void					KeyPress(int c);

	class CPlayerCharacter*			GetPlayerCharacter();
};

#endif
