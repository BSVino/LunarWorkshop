#include "logicgate.h"

REGISTER_ENTITY(CLogicGate);

NETVAR_TABLE_BEGIN(CLogicGate);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLogicGate);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bLeft);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bRight);
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

	SetActive(m_bLeft && m_bRight);
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

	SetActive(m_bLeft && m_bRight);
}
