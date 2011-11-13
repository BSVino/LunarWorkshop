#ifndef REFLECTION_PLAYER_H
#define REFLECTION_PLAYER_H

#include <tengine/game/entities/player.h>

#include "mirror.h"

class CReflectionPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CReflectionPlayer, CPlayer);

public:
	virtual void					Spawn();

	virtual void					MouseMotion(int x, int y);
	virtual void					MouseInput(int iButton, int iState);
	virtual void					KeyPress(int c);

	mirror_t						GetCurrentMirror() const { return m_eCurrentMirror; }

	class CPlayerCharacter*			GetPlayerCharacter();

public:
	mirror_t						m_eCurrentMirror;
};

#endif
