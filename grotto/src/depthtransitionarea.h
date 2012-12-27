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

class CPlayerCharacter;

class CAutoDepthTransitionArea : public CTrigger
{
	REGISTER_ENTITY_CLASS(CAutoDepthTransitionArea, CTrigger);

public:
	virtual void		OnStartTouch(CBaseEntity* pOther);
	virtual void		OnEndTouch(CBaseEntity* pOther);

	virtual void        Think();

	bool				IsValid() const;
	Vector				GetDestination1() const;
	Vector				GetDestination2() const;

protected:
	tvector<CEntityHandle<CPlayerCharacter>> m_ahCharacters;

	CEntityHandle<CBaseEntity>	m_hTarget1;
	CEntityHandle<CBaseEntity>	m_hTarget2;
};
