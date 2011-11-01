#ifndef REFLECTION_CHARACTER_H
#define REFLECTION_CHARACTER_H

#include <tengine/game/entities/character.h>

class CMirror;

class CReflectionCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CReflectionCharacter, CCharacter);

public:
								CReflectionCharacter();

public:
	virtual void				Spawn();

	virtual TVector				GetGoalVelocity();

	virtual TFloat				EyeHeight() const { return 1.8f; }
	virtual TFloat				CharacterSpeed() { return 7.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }

	virtual void				OnSetLocalTransform(Matrix4x4& mNew);

	bool						IsReflected() const { return m_bReflected; }
	CMirror*					GetMirrorInside() const;
	virtual void				Reflected() {};

protected:
	bool						m_bReflected;	// Protected and reflected? Rejected? Projected? INFLECTED?!?
	CEntityHandle<CMirror>		m_hMirrorInside;
};

#endif
