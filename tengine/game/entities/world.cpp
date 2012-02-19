#include "world.h"

#include <physics/physics.h>

REGISTER_ENTITY(CWorld);

NETVAR_TABLE_BEGIN(CWorld);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CWorld);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CWorld);
INPUTS_TABLE_END();

void CWorld::OnSetModel()
{
	BaseClass::OnSetModel();

	// In case the model has changed.
	if (IsInPhysics())
		RemoveFromPhysics();

	if (GetModelID() == ~0)
		return;

	AddToPhysics(CT_STATIC_MESH);
}
