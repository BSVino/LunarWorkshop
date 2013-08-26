#pragma once

#include <tengine/game/entities/character.h>

#include "riftparty.h"

class CRiftPartyCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CRiftPartyCharacter, CCharacter);

public:
								CRiftPartyCharacter();

public:
	virtual TFloat				EyeHeight() const;
	virtual TFloat				BaseCharacterSpeed() { return 5.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }
};
