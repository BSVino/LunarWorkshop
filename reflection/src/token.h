#ifndef REFLECTION_TOKEN_H
#define REFLECTION_TOKEN_H

#include <game/baseentity.h>

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

	void				Reflected() { m_bReflected = !m_bReflected; }
	bool				IsReflected() { return m_bReflected; }

protected:
	CEntityHandle<CReceptacle>	m_hReceptacle;
	bool				m_bReflected;
};

class CReceptacle : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CReceptacle, CBaseEntity);

public:
	void					SetToken(CToken* pToken);
	CToken*					GetToken() const { return m_hToken; }

protected:
	CEntityHandle<CToken>	m_hToken;
};

#endif
