#pragma once

#include <game/baseentity.h>

class CMomento : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CMomento, CBaseEntity);

public:
	void				Precache();
	void				Spawn();

	tstring				GetMomentoName() { return m_sMomentoName; }

protected:
	tstring				m_sMomentoName;
};
