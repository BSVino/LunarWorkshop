#pragma once

#include <tengine/game/entities/character.h>

class CBeam;

class CKaleidobeast : public CCharacter
{
	REGISTER_ENTITY_CLASS(CKaleidobeast, CCharacter);

public:
					~CKaleidobeast();

public:
	void			Precache();
	void			Spawn();
	void			Think();

	void			PostRender(bool bTransparent) const;

	bool			CanSeePlayer() const { return m_bSeesPlayer; }
	void			LosePlayer();

	float			EyeHeight() const { return 1.0f; }
	float			CharacterSpeed() { return 5.0f; }
	float			CharacterAcceleration() { return 40.0f; }

	float			DetectionDistance() const { return 4.0f; }

	virtual bool	UsePhysicsModelForController() const { return true; }

protected:
	bool			m_bSeesPlayer;
	bool			m_bInitialPosition;
	Vector			m_vecInitialPosition;
	EAngle			m_angInitialPosition;

	CEntityHandle<CBeam>	m_hBeam;
};
