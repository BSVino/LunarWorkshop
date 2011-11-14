#include "kinematic.h"

#include <physics/physics.h>

REGISTER_ENTITY(CKinematic);

NETVAR_TABLE_BEGIN(CKinematic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CKinematic);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLerpTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLerpStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLerpEnd);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLerpGoal);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLerpGoal);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bLerping);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CKinematic);
	INPUT_DEFINE(LerpTo);
INPUTS_TABLE_END();

void CKinematic::Spawn()
{
	BaseClass::Spawn();

	m_flLerpTime = 1;
	m_flLerpStart = -1;
}

void CKinematic::OnSetModel()
{
	BaseClass::OnSetModel();

	// In case the model has changed.
	if (IsInPhysics())
		RemoveFromPhysics();

	if (GetModel() == ~0)
		return;

	AddToPhysics(CT_KINEMATIC);
}

void CKinematic::Think()
{
	BaseClass::Think();

	if (m_flLerpStart > 0)
	{
		float flTime = GameServer()->GetGameTime() - m_flLerpStart;
		float flLerp = RemapVal(flTime, 0, m_flLerpEnd - m_flLerpStart, 0, 1);
		float flRamp = SLerp(flLerp, 0.2f);

		m_bLerping = true;
		SetGlobalOrigin(m_vecLerpStart * (1-flRamp) + m_vecLerpGoal * flRamp);
		m_bLerping = false;

		if (flRamp >= 1)
			m_flLerpStart = -1;
	}
}

void CKinematic::OnSetLocalTransform(TMatrix& m)
{
	if (m_bLerping)
		return;

	m_flLerpStart = -1;
}

void CKinematic::LerpTo(const eastl::vector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 3);

	if (sArgs.size() < 3)
	{
		TMsg("Not enough arguments for LerpTo.\n");
		return;
	}

	m_flLerpStart = GameServer()->GetGameTime();
	m_flLerpEnd = m_flLerpStart + m_flLerpTime;

	m_vecLerpStart = GetGlobalOrigin();
	m_vecLerpGoal = Vector(stof(sArgs[0]), stof(sArgs[1]), stof(sArgs[2]));
}
