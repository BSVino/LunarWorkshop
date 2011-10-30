#ifndef REFLECTION_PLAYER_CHARACTER_H
#define REFLECTION_PLAYER_CHARACTER_H

#include "reflection_character.h"

class CPlayerCharacter : public CReflectionCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CReflectionCharacter);

public:
	void						Precache();
	void						Spawn();

protected:
};

#endif
