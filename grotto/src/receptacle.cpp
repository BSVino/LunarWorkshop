#include "receptacle.h"

#include <physics/physics.h>
#include <tinker/application.h>

#include "token.h"

REGISTER_ENTITY(CReceptacle);

NETVAR_TABLE_BEGIN(CReceptacle);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CReceptacle);
	SAVEDATA_DEFINE_OUTPUT(OnNormalToken);
	SAVEDATA_DEFINE_OUTPUT(OnNormalTokenRemoved);
	SAVEDATA_DEFINE_OUTPUT(OnReflectedToken);
	SAVEDATA_DEFINE_OUTPUT(OnReflectedTokenRemoved);
	SAVEDATA_DEFINE_OUTPUT(OnToken);
	SAVEDATA_DEFINE_OUTPUT(OnTokenRemoved);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CToken>, m_hToken);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sDesiredToken, "DesiredToken");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, bool, m_bDesiredReflection, "DesiredReflection");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sDesiredType, "DesiredTokenType");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, Matrix4x4, m_mTokenOffset, "TokenOffset");
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

	m_mTokenOffset = TMatrix(EAngle(40, 0, 0), Vector(0, 0.742105f, 0));
}

bool CReceptacle::IsTokenValid(const CToken* pToken) const
{
	if (!pToken)
		return true;

	if (m_sDesiredToken.length() && pToken->GetName() != m_sDesiredToken)
		return false;

	if ((m_sDesiredType.length() || pToken->GetType().length()) && pToken->GetType() != m_sDesiredType)
		return false;

	return true;
}

void CReceptacle::SetToken(CToken* pToken)
{
	if (!IsActive())
		return;

	if (m_hToken.GetPointer())
	{
		if (IsTokenValid(pToken))
		{
			if (m_hToken->IsReflected())
				CallOutput("OnReflectedTokenRemoved");
			else
				CallOutput("OnNormalTokenRemoved");
			CallOutput("OnTokenRemoved");
		}
	}

	if (m_hToken != nullptr)
		m_hToken->m_hReceptacle = nullptr;

	m_hToken = pToken;

	if (m_hToken == nullptr)
		return;

	pToken->SetMoveParent(this);
	pToken->SetLocalTransform(m_mTokenOffset);
	pToken->m_hReceptacle = this;

	if (IsTokenValid(pToken))
	{
		if (pToken->IsReflected())
			CallOutput("OnReflectedToken");
		else
			CallOutput("OnNormalToken");
		CallOutput("OnToken");
	}
}

Vector CReceptacle::GetTokenPosition()
{
	return (GetGlobalTransform() * m_mTokenOffset).GetTranslation();
}