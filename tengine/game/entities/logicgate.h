#pragma once

#include <game/baseentity.h>

// This class is a logic gate for entity I/O
class CLogicGate : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CLogicGate, CBaseEntity);

public:
	void				Spawn();

	DECLARE_ENTITY_INPUT(InputLeft);
	DECLARE_ENTITY_INPUT(InputRight);

public:
	bool				m_bLeft;
	bool				m_bRight;
};
