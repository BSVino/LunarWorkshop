#pragma once

#include <common.h>

#include "camera.h"
#include "entityhandle.h"

class CCharacter;

class CCharacterCamera : public CCamera
{
	DECLARE_CLASS(CCharacterCamera, CCamera);

public:
				CCharacterCamera();

public:
	virtual TVector			GetCameraPosition();
	virtual TVector			GetCameraDirection();
	virtual TVector			GetCameraUp();

	virtual TVector			GetThirdPersonCameraPosition();
	virtual TVector			GetThirdPersonCameraDirection();

	virtual bool			KeyDown(int c);

	void					ToggleThirdPerson();
	void					SetThirdPerson(bool bOn) { m_bThirdPerson = bOn; };
	bool					GetThirdPerson() { return m_bThirdPerson; };

public:
	bool						m_bThirdPerson;
	CEntityHandle<CCharacter>	m_hCharacter;
};
