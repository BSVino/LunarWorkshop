#pragma once

#include <game/entities/baseentity.h>

#include "grotto.h"

typedef enum
{
	MIRROR_NONE = 0,
	MIRROR_VERTICAL,	// Vertical mirror that reflects things left/right
	MIRROR_HORIZONTAL,	// Horizontal mirror that reflects things up/down
} mirror_t;

class CMirror : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CMirror, CBaseEntity);

public:
	virtual				~CMirror();

public:
	void				Precache();
	void				Spawn();

	bool				IsPointInside(const Vector& vecPoint, bool bPhysics = true) const;

	void				SetMirrorType(mirror_t eType);
	mirror_t			GetMirrorType() const { return m_eMirrorType; }

	reflection_t		GetReflectionType() const;

	// Which side of the mirror is this point on?
	bool				GetSide(const Vector& vecPoint) const;
	Matrix4x4			GetReflection() const;

	void				SetBuffer(size_t i) { m_iBuffer = i; }
	size_t				GetBuffer() const { return m_iBuffer; }

	static size_t		GetNumMirrors() { return m_ahMirrors.size(); };
	static CMirror*		GetMirror(size_t i) { return m_ahMirrors[i]; };

protected:
	mirror_t			m_eMirrorType;
	size_t				m_iBuffer;

	static tvector<CEntityHandle<CMirror> >	m_ahMirrors;
};
