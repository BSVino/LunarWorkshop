#pragma once

#include <game/charactercamera.h>

class CGrottoCamera : public CCharacterCamera
{
	DECLARE_CLASS(CGrottoCamera, CCharacterCamera);

public:
								CGrottoCamera();

public:
	virtual void				Think();

	virtual Vector				GetCameraPosition();
	virtual Vector				GetCameraDirection();
	virtual Vector				GetCameraUp();
	virtual float				GetCameraFOV();
	virtual float				GetCameraNear() { return 0.01f; };
	virtual float				GetCameraFar() { return 500.0f; };
};
