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

	DECLARE_ENTITY_OUTPUT(OnAndTrue);
	DECLARE_ENTITY_OUTPUT(OnAndFalse);

	DECLARE_ENTITY_OUTPUT(OnOrTrue);
	DECLARE_ENTITY_OUTPUT(OnOrFalse);

	void				SetAndGate(bool bAnd);
	void				SetOrGate(bool bOr);

public:
	bool				m_bLeft;
	bool				m_bRight;

	bool				m_bAnd;
	bool				m_bOr;
};
