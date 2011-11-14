#ifndef TINKER_KINEMATIC_H
#define TINKER_KINEMATIC_H

#include <game/baseentity.h>

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

	DECLARE_ENTITY_INPUT(LerpTo);

protected:
	float				m_flLerpTime;
	float				m_flLerpStart;
	float				m_flLerpEnd;

	Vector				m_vecLerpStart;
	Vector				m_vecLerpGoal;
	bool				m_bLerping;
};

#endif
