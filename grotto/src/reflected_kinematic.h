#pragma once

#include <game/entities/kinematic.h>

class CReflectedKinematic : public CKinematic
{
	REGISTER_ENTITY_CLASS(CReflectedKinematic, CKinematic);

public:
	CReflectedKinematic();

public:
	void      PostLoad();

	void      Think();

	const Matrix4x4 GetRenderTransform() const;
	void            ModifyContext(class CRenderingContext* pContext) const;
	const TFloat    GetBoundingRadius() const;

	void            Reflected(class CMirror* pMirror, Matrix4x4& mNewPlayerLocal);
	const Matrix4x4 ReflectProperly() const;

	class CMirror* GetMirror() const;
	bool           IsValid() const;

public:
	bool      m_bReflected;
	CEntityHandle<CBaseEntity> m_hMirror;
	bool      m_bReflectionAffectsPlayer;
	bool      m_bOnlyAffectedByTarget;
};
