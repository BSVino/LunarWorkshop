#ifndef REFLECTION_CAMERA_H
#define REFLECTION_CAMERA_H

#include <game/entities/charactercamera.h>

class CReflectionCamera : public CCharacterCamera
{
	DECLARE_CLASS(CReflectionCamera, CCharacterCamera);

public:
								CReflectionCamera();

public:
	virtual void				Think();

	virtual float				GetCameraFOV();
	virtual float				GetCameraNear() { return 0.01f; };
	virtual float				GetCameraFar() { return 500.0f; };
};

#endif
