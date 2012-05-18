#pragma once

#include <tengine_config.h>
#include <vector.h>
#include <tvector.h>

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
	virtual float	GetCameraOrthoHeight();
	virtual float	GetCameraNear() { return 1.0f; };
	virtual float	GetCameraFar() { return 10000.0f; };
	virtual bool	ShouldRenderOrthographic();

	virtual bool	ShouldTransition();
	virtual float	GetTransitionLerp();

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

	size_t			GetNumCameras() { return m_ahCameras.size(); }
	class CCamera*	GetCamera(size_t iCamera);

public:
	bool			m_bFreeMode;
	TVector			m_vecFreeCamera;
	EAngle			m_angFreeCamera;
	TVector			m_vecFreeVelocity;
	float			m_flFreeOrthoHeight;

	int				m_iMouseLastX;
	int				m_iMouseLastY;

	tvector<CEntityHandle<CCamera>>	m_ahCameras;
	size_t			m_iCurrentCamera;

	double			m_flTransitionBegin;
	float			m_flTransitionTime;
	size_t			m_iLastCamera;
};

CCameraManager* CameraManager();