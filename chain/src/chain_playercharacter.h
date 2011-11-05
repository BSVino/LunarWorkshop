#ifndef CHAIN_PLAYER_CHARACTER_H
#define CHAIN_PLAYER_CHARACTER_H

#include <game/entities/character.h>

class CPlayerCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CCharacter);

public:
	void						Precache();
	void						Spawn();
};

#endif
