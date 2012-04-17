#pragma once

#include <game/entities/trigger.h>

class CDepthTransitionArea : public CTrigger
{
	REGISTER_ENTITY_CLASS(CDepthTransitionArea, CTrigger);

public:
	virtual void		OnStartTouch(CBaseEntity* pOther);
	virtual void		OnEndTouch(CBaseEntity* pOther);

	bool				IsValid() const;
	Vector				GetDestination() const;

protected:
	CEntityHandle<CBaseEntity>	m_hTarget;
};
