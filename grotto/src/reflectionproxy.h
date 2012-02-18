#pragma once

#include <game/baseentity.h>

class CReflectionProxy : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CReflectionProxy, CBaseEntity);

public:
							~CReflectionProxy();

public:
	void					Spawn();

	DECLARE_ENTITY_OUTPUT(OnPlayerReflected);
	DECLARE_ENTITY_OUTPUT(OnPlayerNormal);

	DECLARE_ENTITY_OUTPUT(OnPlayerOnCeiling);
	DECLARE_ENTITY_OUTPUT(OnPlayerOnGround);

public:
	static void				OnPlayerReflection(bool bReflected);
	static void				OnPlayerGravity(bool bCeiling);

protected:
	static CReflectionProxy*	s_pProxy;
};
