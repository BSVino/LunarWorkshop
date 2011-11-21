#include "world.h"

#include <physics/physics.h>

REGISTER_ENTITY(CWorld);

NETVAR_TABLE_BEGIN(CWorld);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CWorld);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CWorld);
INPUTS_TABLE_END();

void CWorld::Precache()
{
	PrecacheModel("levels/test.toy");
}

void CWorld::Spawn()
{
	BaseClass::Spawn();

	SetModel("levels/test.toy");
	AddToPhysics(CT_STATIC_MESH);
}
