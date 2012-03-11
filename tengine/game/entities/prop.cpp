#include "prop.h"

#include <physics/physics.h>

REGISTER_ENTITY(CProp);

NETVAR_TABLE_BEGIN(CProp);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CProp);
	SAVEDATA_EDITOR_VARIABLE("Visible");
	SAVEDATA_EDITOR_VARIABLE("Model");
	SAVEDATA_EDITOR_VARIABLE("TextureScale");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CProp);
INPUTS_TABLE_END();
