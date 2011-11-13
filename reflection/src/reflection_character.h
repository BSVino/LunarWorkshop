#ifndef REFLECTION_CHARACTER_H
#define REFLECTION_CHARACTER_H

#include <tengine/game/entities/character.h>

#include "reflection.h"

class CMirror;

class CReflectionCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CReflectionCharacter, CCharacter);

public:
								CReflectionCharacter();

public:
	virtual void				Spawn();

	virtual TVector				GetGoalVelocity();

	virtual TFloat				EyeHeight() const;
	virtual TFloat				CharacterSpeed() { return 7.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }

	virtual void				OnSetLocalTransform(Matrix4x4& mNew);

	bool						IsReflected(reflection_t eReflectionType) const;
	CMirror*					GetMirrorInside() const;
	virtual void				Reflected(reflection_t eReflectionType) {};

	virtual TVector				GetUpVector() const;

	// Is the point near enough to a mirror to be considered inside of it for physics purposes?
	bool						IsNearMirror(class CMirror* pMirror, const Vector& vecPoint) const;

	virtual bool				ShouldCollideWith(CBaseEntity* pOther, const TVector& vecPoint) const;

	Matrix4x4					GetReflectionMatrix() const { return m_mLateralReflection * m_mVerticalReflection; }

protected:
	// A bit map of types of reflection applied currently.
	size_t						m_iReflected;	// Protected and reflected? Rejected? Projected? INFLECTED?!?
	Matrix4x4					m_mLateralReflection;	// For rendering
	Matrix4x4					m_mVerticalReflection;	// For rendering
	CEntityHandle<CMirror>		m_hMirrorInside;
};

#endif
