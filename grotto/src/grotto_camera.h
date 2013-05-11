#pragma once

#include <game/entities/charactercamera.h>

class CGrottoCamera : public CCharacterCamera
{
	DECLARE_CLASS(CGrottoCamera, CCharacterCamera);

public:
								CGrottoCamera();

public:
	virtual void				Think();

	virtual float				GetCameraFOV();
	virtual float				GetCameraNear() { return 0.01f; };
	virtual float				GetCameraFar() { return 500.0f; };
};
