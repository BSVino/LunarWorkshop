#ifndef TINKER_KINEMATIC_H
#define TINKER_KINEMATIC_H

#include <game/entities/baseentity.h>

// This class is a kinematic physics object that is controllable with entity I/O
class CKinematic : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CKinematic, CBaseEntity);

public:
	void				Spawn();
	virtual void		OnSetModel();
	virtual void		Think();

	virtual void		OnSetLocalTransform(TMatrix& m);

	virtual void		SetLerpTime(float flLerpTime) { m_flLerpTime = flLerpTime; }
	virtual void		SetAngleLerpTime(float flLerpTime) { m_flAngleLerpTime = flLerpTime; }

	DECLARE_ENTITY_INPUT(LerpTo);
	DECLARE_ENTITY_INPUT(LerpAnglesTo);

protected:
	float				m_flLerpTime;
	float				m_flLerpStart;
	float				m_flLerpEnd;

	Vector				m_vecLerpStart;
	Vector				m_vecLerpGoal;

	float				m_flAngleLerpTime;
	float				m_flAngleLerpStart;
	float				m_flAngleLerpEnd;

	EAngle				m_angLerpStart;
	EAngle				m_angLerpGoal;

	bool				m_bLerping;
};

#endif
