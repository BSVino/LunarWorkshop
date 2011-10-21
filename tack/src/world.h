#ifndef TACK_WORLD_H
#define TACK_WORLD_H

#include <tengine/game/baseentity.h>

class CWorld : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CWorld, CBaseEntity);

public:
	virtual void					Precache();
	virtual void					Spawn();

	virtual bool					ShouldRender() const { return true; };

	virtual bool					ShouldCollide() const { return true; }

	virtual bool					CollideLocal(const TVector& v1, const TVector& v2, TVector& vecPoint, TVector& vecNormal);
	virtual bool					Collide(const TVector& v1, const TVector& v2, TVector& vecPoint, TVector& vecNormal);
};

#endif
