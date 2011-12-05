#pragma once

#include <game/baseentity.h>

class CToken;

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
