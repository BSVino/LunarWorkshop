#pragma once

#include <game/entities/kinematic.h>

class CAsymmetricKinematic : public CKinematic
{
	REGISTER_ENTITY_CLASS(CAsymmetricKinematic, CKinematic);

public:
	CAsymmetricKinematic();

public:
	void      PostLoad();

	void      Think();

	const Matrix4x4 GetRenderTransform() const;

	void      Reflected(Matrix4x4& mNewPlayerLocal);

public:
	bool      m_bReflected;
	CEntityHandle<CBaseEntity> m_hNormalPosition;
	CEntityHandle<CBaseEntity> m_hReflectedPosition;
	bool      m_bReflectionAffectsPlayer;
};
