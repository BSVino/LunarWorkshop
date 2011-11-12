#ifndef REFLECTION_MIRROR_H
#define REFLECTION_MIRROR_H

#include <game/baseentity.h>

class CMirror : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CMirror, CBaseEntity);

public:
	void				Precache();
	void				Spawn();

	bool				IsPointInside(const Vector& vecPoint) const;
};

#endif
