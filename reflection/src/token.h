#ifndef REFLECTION_TOKEN_H
#define REFLECTION_TOKEN_H

#include <game/baseentity.h>

#include "reflection.h"

class CReceptacle;

class CToken : public CBaseEntity
{
	friend class CReceptacle;

	REGISTER_ENTITY_CLASS(CToken, CBaseEntity);

public:
	void				Precache();
	void				Spawn();

	CReceptacle*		GetReceptacle() const;

	virtual void		ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;

	void				Reflected(reflection_t eReflectionType);
	void				SetReflected(bool bReflected) { m_bReflected = bReflected; }
	bool				IsReflected() const;

protected:
	CEntityHandle<CReceptacle>	m_hReceptacle;
	bool				m_bReflected;
};

#endif
