#include "reflection_playercharacter.h"

#include <tinker/application.h>
#include <game/physics.h>

#include "mirror.h"

REGISTER_ENTITY(CPlayerCharacter);

NETVAR_TABLE_BEGIN(CPlayerCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayerCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CMirror>, m_hMirror);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerCharacter);
INPUTS_TABLE_END();

void CPlayerCharacter::Precache()
{
}

void CPlayerCharacter::Spawn()
{
	BaseClass::Spawn();

	SetMass(60);
	m_aabbBoundingBox = AABB(Vector(-0.35f, 0, -0.35f), Vector(0.35f, 2, 0.35f));

	AddToPhysics(CT_CHARACTER);
}

void CPlayerCharacter::PlaceMirror()
{
	if (m_hMirror != NULL)
		m_hMirror->Delete();

	CMirror* pMirror = m_hMirror = GameServer()->Create<CMirror>("CMirror");

	Vector vecForward = GetGlobalTransform().GetForwardVector();
	vecForward.y = 0;
	vecForward.Normalize();
	pMirror->SetGlobalOrigin(GetGlobalOrigin() + vecForward*1.5f);

	EAngle angGlobal = GetGlobalAngles();
	angGlobal.p = 0;
	angGlobal.r = 0;
	pMirror->SetGlobalAngles(angGlobal);
}
