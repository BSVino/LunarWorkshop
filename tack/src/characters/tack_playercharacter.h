#ifndef TACK_PLAYER_CHARACTER_H
#define TACK_PLAYER_CHARACTER_H

#include "tack_character.h"

class CPlayerCharacter : public CTackCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CTackCharacter);

public:
	void						Precache();
	void						Spawn();

	virtual float				DamageRecoveryTime() const { return 0.3f; }
};

#endif
