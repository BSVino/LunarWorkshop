#include "riftparty_playercharacter.h"

#include <tinker/application.h>
#include <physics/physics.h>
#include <game/entities/charactercamera.h>

REGISTER_ENTITY(CPlayerCharacter);

NETVAR_TABLE_BEGIN(CPlayerCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayerCharacter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerCharacter);
INPUTS_TABLE_END();

void CPlayerCharacter::Precache()
{
}

void CPlayerCharacter::Spawn()
{
	SetMass(60);
	m_aabbPhysBoundingBox = AABB(Vector(-0.35f, -0.35f, 0), Vector(0.35f, 0.35f, 2));

	SetGlobalGravity(Vector(0, 0, -9.8f)*2);

	BaseClass::Spawn();

	m_hCamera = GameServer()->Create<CCharacterCamera>("CRiftPartyCamera");
	m_hCamera->SetCharacter(this);
}

void CPlayerCharacter::Think()
{
	BaseClass::Think();
}
