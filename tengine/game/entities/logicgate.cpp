#include "logicgate.h"

REGISTER_ENTITY(CLogicGate);

NETVAR_TABLE_BEGIN(CLogicGate);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLogicGate);
	SAVEDATA_DEFINE_OUTPUT(OnAndTrue);
	SAVEDATA_DEFINE_OUTPUT(OnAndFalse);
	SAVEDATA_DEFINE_OUTPUT(OnOrTrue);
	SAVEDATA_DEFINE_OUTPUT(OnOrFalse);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, bool, m_bLeft, "Left");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, bool, m_bRight, "Right");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bAnd);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bOr);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CLogicGate);
	INPUT_DEFINE(InputLeft);
	INPUT_DEFINE(InputRight);
INPUTS_TABLE_END();

void CLogicGate::Spawn()
{
	BaseClass::Spawn();

	m_bLeft = false;
	m_bRight = false;

	SetActive(false);
}

void CLogicGate::InputLeft(const eastl::vector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 1);

	if (sArgs.size() < 1)
	{
		TMsg("Not enough arguments for InputLeft.\n");
		return;
	}

	bool bNewLeft = !!stoi(sArgs[0]);
	if (bNewLeft == m_bLeft)
		return;

	m_bLeft = bNewLeft;

	SetAndGate(m_bLeft && m_bRight);
	SetOrGate(m_bLeft || m_bRight);
}

void CLogicGate::InputRight(const eastl::vector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 1);

	if (sArgs.size() < 1)
	{
		TMsg("Not enough arguments for InputRight.\n");
		return;
	}

	bool bNewRight = !!stoi(sArgs[0]);
	if (bNewRight == m_bRight)
		return;

	m_bRight = bNewRight;

	SetAndGate(m_bLeft && m_bRight);
	SetOrGate(m_bLeft || m_bRight);
}

void CLogicGate::SetAndGate(bool bAnd)
{
	if (bAnd && !m_bAnd)
		CallOutput("OnAndTrue");

	if (m_bAnd && !bAnd)
		CallOutput("OnAndFalse");

	m_bAnd = bAnd;
}

void CLogicGate::SetOrGate(bool bOr)
{
	if (bOr && !m_bOr)
		CallOutput("OnOrTrue");

	if (m_bOr && !bOr)
		CallOutput("OnOrFalse");

	m_bOr = bOr;
}
