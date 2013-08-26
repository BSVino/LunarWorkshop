#pragma once

#include <game/entities/charactercamera.h>

class CRiftPartyCamera : public CCharacterCamera
{
	REGISTER_ENTITY_CLASS(CRiftPartyCamera, CCharacterCamera);

public:
								CRiftPartyCamera();

public:
	virtual void				Think();

	virtual float				GetCameraFOV();
	virtual float				GetCameraNear() { return 0.01f; };
	virtual float				GetCameraFar() { return 500.0f; };

	virtual const Vector        GetUpVector() const;
};
