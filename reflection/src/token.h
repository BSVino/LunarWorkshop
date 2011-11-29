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

class CReceptacle : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CReceptacle, CBaseEntity);

public:
	void					Precache();
	void					Spawn();

	void					SetToken(CToken* pToken);
	CToken*					GetToken() const { return m_hToken; }

	// What token does the receptacle need to fire its output?
	void					SetDesiredToken(const tstring& sToken) { m_sDesiredToken = sToken; }
	// What reflection does the receptacle need to fire its output?
	void					SetDesiredReflection(bool bReflected) { m_bDesiredReflection = bReflected; }

	DECLARE_ENTITY_OUTPUT(OnNormalToken);
	DECLARE_ENTITY_OUTPUT(OnNormalTokenRemoved);
	DECLARE_ENTITY_OUTPUT(OnReflectedToken);
	DECLARE_ENTITY_OUTPUT(OnReflectedTokenRemoved);
	DECLARE_ENTITY_OUTPUT(OnToken);
	DECLARE_ENTITY_OUTPUT(OnTokenRemoved);

protected:
	CEntityHandle<CToken>	m_hToken;
	tstring					m_sDesiredToken;
	bool					m_bDesiredReflection;
};

#endif
