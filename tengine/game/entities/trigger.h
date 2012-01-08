#ifndef TINKER_TRIGGER_H
#define TINKER_TRIGGER_H

#include <game/baseentity.h>

// This class is a kinematic physics object that is controllable with entity I/O
class CTrigger : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTrigger, CBaseEntity);

public:
	void				Spawn();
	virtual void		OnSetModel();
	virtual void		ClientSpawn();

	virtual void		Think();

	virtual void		Touching(const CBaseEntity* pOther);
	virtual void		BeginTouchingList();
	virtual void		EndTouchingList();

	virtual void		StartTouch(const CBaseEntity* pOther);
	virtual void		EndTouch(const CBaseEntity* pOther);

	DECLARE_ENTITY_OUTPUT(OnStartTouch);
	DECLARE_ENTITY_OUTPUT(OnEndTouch);

	virtual void		StartVisible();
	virtual void		EndVisible();

	DECLARE_ENTITY_OUTPUT(OnStartVisible);
	DECLARE_ENTITY_OUTPUT(OnEndVisible);

	virtual bool		ShouldRender() const { return false; };

protected:
	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahTouching;
	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahLastTouching;

	bool				m_bVisible;
};

#endif
