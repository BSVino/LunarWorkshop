#pragma once

#include <game/entities/camera.h>

class CGrottoCamera : public CCamera
{
	REGISTER_ENTITY_CLASS(CGrottoCamera, CCamera);

public:
	virtual void				Spawn();
	virtual void				Think();
};
