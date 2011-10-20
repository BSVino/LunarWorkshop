#include "world.h"

REGISTER_ENTITY(CWorld);

NETVAR_TABLE_BEGIN(CWorld);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CWorld);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CWorld);
INPUTS_TABLE_END();

bool CWorld::CollideLocal(const TVector& v1, const TVector& v2, TVector& vecPoint, TVector& vecNormal)
{
	if (v1 == v2)
	{
		vecPoint = v1;
		vecNormal = Vector(0, 1, 0);
		if (v1.y < 0)
			return true;

		return false;
	}

	if (v2.y > 0 && v1.y > 0)
		return false;

	if (v1.y < 0)
	{
		vecPoint = v1;
		vecNormal = Vector(0, 1, 0);
		return true;
	}

	if (v2.y > v1.y)
		return false;

	float flFraction = RemapVal(0, v1.y, v2.y, 0, 1);
	if (flFraction >= 0 && flFraction <= 1)
	{
		vecPoint = v1 + (v2-v1)*flFraction;
		vecNormal = Vector(0, 1, 0);
		return true;
	}

	return false;
}

bool CWorld::Collide(const TVector& v1, const TVector& v2, TVector& vecPoint, TVector& vecNormal)
{
	return CollideLocal(v1, v2, vecPoint, vecNormal);
}
