#include "receptacle.h"

#include <physics/physics.h>
#include <tinker/application.h>

#include "token.h"

REGISTER_ENTITY(CReceptacle);

NETVAR_TABLE_BEGIN(CReceptacle);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReceptacle);
	SAVEDATA_DEFINE_OUTPUT(OnNormalToken);
	SAVEDATA_DEFINE_OUTPUT(OnNormalTokenRemoved);
	SAVEDATA_DEFINE_OUTPUT(OnReflectedToken);
	SAVEDATA_DEFINE_OUTPUT(OnReflectedTokenRemoved);
	SAVEDATA_DEFINE_OUTPUT(OnToken);
	SAVEDATA_DEFINE_OUTPUT(OnTokenRemoved);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CToken>, m_hToken);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sDesiredToken, "DesiredToken");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, bool, m_bDesiredReflection, "DesiredReflection");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReceptacle);
INPUTS_TABLE_END();

void CReceptacle::Precache()
{
	PrecacheModel("models/pedestal.toy");
}

void CReceptacle::Spawn()
{
	Precache();

	BaseClass::Spawn();

	SetModel("models/pedestal.toy");
	AddToPhysics(CT_KINEMATIC);
}

void CReceptacle::SetToken(CToken* pToken)
{
	if (m_hToken.GetPointer())
	{
		if (!m_sDesiredToken.length() || m_hToken->GetName() == m_sDesiredToken)
		{
			if (m_hToken->IsReflected())
				CallOutput("OnReflectedTokenRemoved");
			else
				CallOutput("OnNormalTokenRemoved");
			CallOutput("OnTokenRemoved");
		}
	}

	if (!pToken)
	{
		if (m_hToken != nullptr)
			m_hToken->m_hReceptacle = nullptr;
	}

	m_hToken = pToken;

	if (m_hToken == nullptr)
		return;

	pToken->SetMoveParent(this);
	pToken->SetLocalTransform(TMatrix(EAngle(45, 180, 0), Vector(0, 0.742105f, 0)));
	pToken->m_hReceptacle = this;

	if (!m_sDesiredToken.length() || pToken->GetName() == m_sDesiredToken)
	{
		if (pToken->IsReflected())
			CallOutput("OnReflectedToken");
		else
			CallOutput("OnNormalToken");
		CallOutput("OnToken");
	}
}
