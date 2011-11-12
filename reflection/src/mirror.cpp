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

bool CMirror::IsPointInside(const Vector& vecPoint) const
{
	if (vecPoint.y < GetGlobalOrigin().y + 0.05f)
		return false;

	if (vecPoint.y > GetGlobalOrigin().y + GetBoundingBox().Size().y)
		return false;

	return (vecPoint - GetGlobalOrigin()).Length2DSqr() < GetBoundingBox().Size().Length2DSqr()/2;
}
