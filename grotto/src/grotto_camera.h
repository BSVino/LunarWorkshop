#pragma once

#include <game/entities/camera.h>

class CGrottoCamera : public CCamera
{
	REGISTER_ENTITY_CLASS(CGrottoCamera, CCamera);

public:
	virtual void				Spawn();
	virtual void				Think();
	virtual void				CameraThink();

	bool						IsAutoTracking();

	bool						ShouldRenderOrthographic() { return true; }

protected:
	EAngle						m_angTarget;
	EAngle						m_angTargetGoal;
	double						m_flLastTargetChange;
};
