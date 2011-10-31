#include "mirror.h"

#include <models/models.h>

#include "reflection_renderer.h"

REGISTER_ENTITY(CMirror);

NETVAR_TABLE_BEGIN(CMirror);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMirror);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMirror);
INPUTS_TABLE_END();

void CMirror::Precache()
{
	PrecacheModel("models/mirror.obj");
}

void CMirror::Spawn()
{
	SetModel("models/mirror.obj");

	ReflectionRenderer()->SetMirror(this);
}
