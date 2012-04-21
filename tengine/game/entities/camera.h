#pragma once

#include <game/entities/baseentity.h>

class CCamera : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCamera, CBaseEntity);

public:
					~CCamera();

public:
	void			Spawn();
	virtual void	CameraThink();

	bool			IsTracking();

	virtual float	GetFOV() { return m_flFOV; }

protected:
	CEntityHandle<CBaseEntity>	m_hCameraTarget;

	CEntityHandle<CBaseEntity>	m_hTrackStart;
	CEntityHandle<CBaseEntity>	m_hTrackEnd;

	CEntityHandle<CBaseEntity>	m_hTargetStart;
	CEntityHandle<CBaseEntity>	m_hTargetEnd;

	float			m_flFOV;
};
