#ifndef REFLECTION_CAMERA_H
#define REFLECTION_CAMERA_H

#include <game/camera.h>
#include <common.h>

class CReflectionCamera : public CCamera
{
	DECLARE_CLASS(CReflectionCamera, CCamera);

public:
								CReflectionCamera();

public:
	virtual Vector				GetCameraPosition();
	virtual Vector				GetCameraTarget();
	virtual float				GetCameraFOV();
	virtual float				GetCameraNear() { return 0.01f; };
	virtual float				GetCameraFar() { return 100.0f; };
};

#endif
