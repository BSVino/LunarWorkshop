#pragma once

#include <tengine/game/entities/character.h>

class CKaleidobeast : public CCharacter
{
	REGISTER_ENTITY_CLASS(CKaleidobeast, CCharacter);

public:
	void			Precache();
	void			Spawn();
	void			Think();

	bool			CanSeePlayer() const { return m_bSeesPlayer; }

	float			CharacterSpeed() { return 2.0f; }

	virtual bool	UsePhysicsModelForController() const { return true; }

protected:
	bool			m_bSeesPlayer;
	bool			m_bInitialPosition;
	Vector			m_vecInitialPosition;
	EAngle			m_angInitialPosition;
};
