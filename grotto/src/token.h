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

	CReceptacle*		GetReceptacle() const;

	virtual void		ModifyContext(class CRenderingContext* pContext) const;

	void				Reflected(reflection_t eReflectionType);
	void				SetReflected(bool bReflected) { m_bReflected = bReflected; }
	bool				IsReflected() const;

	tstring				GetType() const { return m_sType; }

protected:
	CEntityHandle<CReceptacle>	m_hReceptacle;
	bool				m_bReflected;
	tstring				m_sType;
};
