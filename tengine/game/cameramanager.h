#pragma once

#include <EASTL/vector.h>

#include <tengine_config.h>
#include <vector.h>

#include <game/entityhandle.h>

class CCamera;

class CCameraManager
{
public:
					CCameraManager();

public:
	virtual void	Think();

	virtual TVector	GetCameraPosition();
	virtual TVector	GetCameraDirection();
	virtual TVector	GetCameraUp();
	virtual float	GetCameraFOV();
	virtual float	GetCameraNear() { return 1.0f; };
	virtual float	GetCameraFar() { return 10000.0f; };

	bool			GetFreeMode() { return m_bFreeMode; };
	TVector			GetFreeCameraPosition() const { return m_vecFreeCamera; };
	EAngle			GetFreeCameraAngles() const { return m_angFreeCamera; };

	virtual void	MouseInput(int x, int y);
	virtual bool	MouseButton(int iButton, int iState) { return false; };
	virtual bool	KeyDown(int c);
	virtual bool	KeyUp(int c);

	void			AddCamera(CCamera* pCamera);
	void			RemoveCamera(CCamera* pCamera);

	void			SetActiveCamera(CCamera* pCamera);
	CCamera*		GetActiveCamera();

public:
	bool			m_bFreeMode;
	TVector			m_vecFreeCamera;
	EAngle			m_angFreeCamera;
	TVector			m_vecFreeVelocity;

	int				m_iMouseLastX;
	int				m_iMouseLastY;

	eastl::vector<CEntityHandle<CCamera>>	m_ahCameras;
	size_t			m_iCurrentCamera;
};

CCameraManager* CameraManager();
