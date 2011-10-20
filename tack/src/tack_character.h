#ifndef TACK_CHARACTER_H
#define TACK_CHARACTER_H

#include <tengine/game/entities/character.h>

class CTackCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CTackCharacter, CCharacter);

public:
	virtual void				Spawn();

	virtual TFloat				GetBoundingRadius() const { return 2.0f; };
	virtual TFloat				EyeHeight() const { return 1.8f; }
	virtual TFloat				CharacterSpeed() { return 10.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }

	virtual void				PostRender(bool bTransparent) const;
};

#endif
