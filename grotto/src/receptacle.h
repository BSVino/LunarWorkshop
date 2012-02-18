#pragma once

#include <game/baseentity.h>

class CToken;

class CReceptacle : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CReceptacle, CBaseEntity);

public:
	void					Precache();
	void					Spawn();

	virtual bool			IsTokenValid(const CToken* pToken) const;

	void					SetToken(CToken* pToken);
	CToken*					GetToken() const { return m_hToken; }

	Vector					GetTokenPosition();

	DECLARE_ENTITY_OUTPUT(OnNormalToken);
	DECLARE_ENTITY_OUTPUT(OnNormalTokenRemoved);
	DECLARE_ENTITY_OUTPUT(OnReflectedToken);
	DECLARE_ENTITY_OUTPUT(OnReflectedTokenRemoved);
	DECLARE_ENTITY_OUTPUT(OnToken);
	DECLARE_ENTITY_OUTPUT(OnTokenRemoved);

protected:
	CEntityHandle<CToken>	m_hToken;
	tstring					m_sDesiredToken;		// What token does the receptacle need to fire its output?
	bool					m_bDesiredReflection;	// What reflection does the receptacle need to fire its output?
	tstring					m_sDesiredType;			// What type of token does the receptacle need to fire its output?
	Matrix4x4				m_mTokenOffset;
};
