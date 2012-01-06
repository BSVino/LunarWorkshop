#ifndef CHAIN_CAMERA_H
#define CHAIN_CAMERA_H

#include <game/camera.h>
#include <common.h>

class CChainCamera : public CCamera
{
	DECLARE_CLASS(CChainCamera, CCamera);

public:
								CChainCamera();

public:
	virtual Vector				GetCameraPosition();
	virtual Vector				GetCameraDirection();
	virtual float				GetCameraFOV();
};

#endif
