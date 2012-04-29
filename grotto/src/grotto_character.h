#pragma once

#include <tengine/game/entities/character.h>

#include "grotto.h"

class CMirror;
class CDepthTransitionArea;

// One level every two meters
#define METERS_PER_DEPTH 2

class CGrottoCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CGrottoCharacter, CCharacter);

public:
								CGrottoCharacter();

public:
	virtual void				Spawn();

	virtual const TVector		GetGoalVelocity();

	virtual void				OnSetLocalTransform(Matrix4x4& mNew);
	virtual void				TestMirror(CMirror* pMirror, Matrix4x4& mNew);
	virtual void				Reflect(const Matrix4x4& mMirror, const Matrix4x4& mReflection, reflection_t eReflectionType, Matrix4x4& mNew, CMirror* pMirror = nullptr);

	DECLARE_ENTITY_INPUT(ReflectVertical);
	DECLARE_ENTITY_INPUT(ReflectLateral);

	bool						IsReflected(reflection_t eReflectionType) const;
	CMirror*					GetMirrorInside() const;
	virtual void				Reflected(reflection_t eReflectionType) {};

	virtual const TVector		GetUpVector() const;

	// Is the point near enough to a mirror to be considered inside of it for physics purposes?
	bool						IsNearMirror(class CMirror* pMirror, const Vector& vecPoint) const;

	virtual bool				ShouldCollideWith(CBaseEntity* pOther, const TVector& vecPoint) const;

	const Matrix4x4				GetReflectionMatrix() const { return m_mLateralReflection * m_mVerticalReflection; }

	void						SetTouchingDepthTransitionArea(CDepthTransitionArea* pArea, bool bTouching);

protected:
	// A bit map of types of reflection applied currently.
	size_t						m_iReflected;	// Protected and reflected? Rejected? Projected? INFLECTED?!?
	Matrix4x4					m_mLateralReflection;	// For rendering
	Matrix4x4					m_mVerticalReflection;	// For rendering
	CEntityHandle<CMirror>		m_hMirrorInside;

	eastl::vector<CEntityHandle<CDepthTransitionArea>>	m_ahTouchingDepthTransitionArea;
};
