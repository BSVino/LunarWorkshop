#include "reflectionproxy.h"

#include "asymmetric_kinematic.h"
#include "reflected_kinematic.h"

REGISTER_ENTITY(CReflectionProxy);

NETVAR_TABLE_BEGIN(CReflectionProxy);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionProxy);
	SAVEDATA_DEFINE_OUTPUT(OnPlayerReflected);
	SAVEDATA_DEFINE_OUTPUT(OnPlayerNormal);
	SAVEDATA_DEFINE_OUTPUT(OnPlayerOnCeiling);
	SAVEDATA_DEFINE_OUTPUT(OnPlayerOnGround);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionProxy);
INPUTS_TABLE_END();

CReflectionProxy* CReflectionProxy::s_pProxy = nullptr;

CReflectionProxy::~CReflectionProxy()
{
	if (s_pProxy == this)
		s_pProxy = nullptr;
}

void CReflectionProxy::Spawn()
{
	TAssert(!s_pProxy);
	s_pProxy = this;

	BaseClass::Spawn();
}

void CReflectionProxy::OnPlayerReflection(CMirror* pMirror, bool bReflected, Matrix4x4& mNewPlayerLocal)
{
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CAsymmetricKinematic* pAsymmetricKinematic = dynamic_cast<CAsymmetricKinematic*>(pEntity);
		if (pAsymmetricKinematic)
			pAsymmetricKinematic->Reflected(mNewPlayerLocal);

		CReflectedKinematic* pReflectedKinematic = dynamic_cast<CReflectedKinematic*>(pEntity);
		if (pReflectedKinematic)
			pReflectedKinematic->Reflected(pMirror, mNewPlayerLocal);
	}

	if (!s_pProxy)
		return;

	if (bReflected)
		s_pProxy->CallOutput("OnPlayerReflected");
	else
		s_pProxy->CallOutput("OnPlayerNormal");
}

void CReflectionProxy::OnPlayerGravity(bool bCeiling)
{
	if (!s_pProxy)
		return;

	if (bCeiling)
		s_pProxy->CallOutput("OnPlayerOnCeiling");
	else
		s_pProxy->CallOutput("OnPlayerOnGround");
}
