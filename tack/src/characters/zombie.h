#ifndef TACK_ZOMBIE_H
#define TACK_ZOMBIE_H

#include "tack_character.h"

class CZombie : public CTackCharacter
{
	REGISTER_ENTITY_CLASS(CZombie, CTackCharacter);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				ZombieThink();
	virtual void				Think();

	virtual TFloat				CharacterSpeed() { return 1.0f; }

protected:
	float						m_flGoalYaw;
	float						m_flRenderYaw;
};

#endif
