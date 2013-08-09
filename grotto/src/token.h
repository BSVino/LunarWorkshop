#pragma once

#include <game/entities/baseentity.h>

#include "grotto.h"

class CReceptacle;

class CToken : public CBaseEntity
{
	friend class CReceptacle;

	REGISTER_ENTITY_CLASS(CToken, CBaseEntity);

public:
	void				Precache();
	void				Spawn();
	void                PostLoad();

	bool                IsUsable(const CBaseEntity* pUser) const;
	void                OnUse(CBaseEntity* pUser);

	CReceptacle*		GetReceptacle() const;

	virtual void		ModifyContext(class CRenderingContext* pContext) const;

	void				Reflected(reflection_t eReflectionType);
	void				SetReflected(bool bReflected) { m_bReflected = bReflected; }
	bool				IsReflected() const;

	void                PostPlaceInReceptacle(CReceptacle* pReceptacle);

	tstring				GetType() const { return m_sType; }

protected:
	CEntityHandle<CReceptacle>	m_hReceptacle;
	CEntityHandle<CReceptacle>  m_hPostPlaceInReceptacle;
	bool				m_bReflected;
	tstring				m_sType;
	bool                m_bMustMatchUpVector;
};
