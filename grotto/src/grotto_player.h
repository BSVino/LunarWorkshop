#pragma once

#include <tengine/game/entities/player.h>

#include "mirror.h"

class CGrottoPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CGrottoPlayer, CPlayer);

public:
	virtual void					Spawn();

	virtual void					MouseMotion(int x, int y);
	virtual void					MouseInput(int iButton, int iState);
	virtual void					KeyPress(int c);
	virtual void                    JoystickButtonPress(int iJoystick, int c);
	virtual void                    JoystickAxis(int iJoystick, int iAxis, float flValue, float flChange);

	mirror_t						GetCurrentMirror() const { return m_eCurrentMirror; }

	class CPlayerCharacter*			GetPlayerCharacter();

public:
	mirror_t						m_eCurrentMirror;
};
