#pragma once

#include <tengine/game/entities/character.h>

class CBeam;

class CRobot : public CCharacter
{
	REGISTER_ENTITY_CLASS(CRobot, CCharacter);

public:
					~CRobot();

public:
	void			Precache();
	void			Spawn();
	void			Think();

	void			PostRender(bool bTransparent) const;

	float			EyeHeight() const { return 1.0f; }
	float			BaseCharacterSpeed() { return 4.0f; }
	float			CharacterAcceleration() { return 40.0f; }

	float			DetectionDistance() const { return 4.0f; }

	virtual bool	UsePhysicsModelForController() const { return true; }
};
