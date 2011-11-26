#ifndef TACK_WORLD_H
#define TACK_WORLD_H

#include <tengine/game/baseentity.h>

class CWorld : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CWorld, CBaseEntity);

public:
	virtual void		OnSetModel();
};

#endif
