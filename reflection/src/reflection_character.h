#ifndef REFLECTION_CHARACTER_H
#define REFLECTION_CHARACTER_H

#include <tengine/game/entities/character.h>

class CReflectionCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CReflectionCharacter, CCharacter);

public:
								CReflectionCharacter();

public:
	virtual void				Spawn();

	virtual TFloat				EyeHeight() const { return 1.8f; }
	virtual TFloat				CharacterSpeed() { return 7.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }

protected:
};

#endif
