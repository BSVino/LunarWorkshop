#ifndef REFLECTION_MIRROR_H
#define REFLECTION_MIRROR_H

#include <game/baseentity.h>

#include "reflection.h"

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
	void				Precache();
	void				Spawn();

	bool				IsPointInside(const Vector& vecPoint) const;

	void				SetMirrorType(mirror_t eType);
	mirror_t			GetMirrorType() const { return m_eMirrorType; }

	reflection_t		GetReflectionType() const;

	// Which side of the mirror is this point on?
	bool				GetSide(const Vector& vecPoint) const;
	Matrix4x4			GetReflection() const;

protected:
	mirror_t			m_eMirrorType;
};

#endif
