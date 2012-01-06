#ifndef TACK_CAMERA_H
#define TACK_CAMERA_H

#include <game/camera.h>
#include <common.h>

class CTackCamera : public CCamera
{
	DECLARE_CLASS(CTackCamera, CCamera);

public:
								CTackCamera();

public:
	virtual Vector				GetCameraPosition();
	virtual Vector				GetCameraDirection();
	virtual float				GetCameraFOV();
};

#endif
