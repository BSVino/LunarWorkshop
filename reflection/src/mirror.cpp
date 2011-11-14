#include "mirror.h"

#include <models/models.h>

#include "reflection_renderer.h"

REGISTER_ENTITY(CMirror);

NETVAR_TABLE_BEGIN(CMirror);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMirror);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, mirror_t, m_eMirrorType);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMirror);
INPUTS_TABLE_END();

void CMirror::Precache()
{
	PrecacheModel("models/mirror.obj");
	PrecacheModel("models/mirror_horizontal.obj");
}

void CMirror::Spawn()
{
	SetMirrorType(MIRROR_HORIZONTAL);

	ReflectionRenderer()->SetMirror(this);
}

bool CMirror::IsPointInside(const Vector& vecPoint) const
{
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		if (GetGlobalTransform().GetUpVector().y > 0)
		{
			if (vecPoint.y < GetGlobalOrigin().y + 0.05f)
				return false;

			if (vecPoint.y > GetGlobalOrigin().y + GetBoundingBox().Size().y)
				return false;
		}
		else
		{
			if (vecPoint.y > GetGlobalOrigin().y - 0.05f)
				return false;

			if (vecPoint.y < GetGlobalOrigin().y - GetBoundingBox().Size().y)
				return false;
		}

		return (vecPoint - GetGlobalOrigin()).Length2D() < GetBoundingBox().Size().Length2D()/2;

	case MIRROR_HORIZONTAL:
		if (vecPoint.y < GetGlobalOrigin().y - 0.15f)
			return false;

		if (vecPoint.y > GetGlobalOrigin().y + 0.15f)
			return false;

		return (vecPoint - GetGlobalOrigin()).Length2D() < GetBoundingBox().Size().Length2D()/4;
	}

	return false;
}

void CMirror::SetMirrorType(mirror_t eType)
{
	m_eMirrorType = eType;
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		SetModel("models/mirror.obj");
		break;

	case MIRROR_HORIZONTAL:
		SetModel("models/mirror_horizontal.obj");
		break;
	}
}

reflection_t CMirror::GetReflectionType() const
{
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		return REFLECTION_LATERAL;

	case MIRROR_HORIZONTAL:
		return REFLECTION_VERTICAL;
	}

	return REFLECTION_NONE;
}

bool CMirror::GetSide(const Vector& vecPoint) const
{
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		return (vecPoint - GetGlobalOrigin()).Normalized().Dot(GetGlobalTransform().GetForwardVector()) > 0;

	case MIRROR_HORIZONTAL:
		return (vecPoint - GetGlobalOrigin()).Normalized().Dot(Vector(0, 1, 0)) > 0;
	}

	return false;
}

Matrix4x4 CMirror::GetReflection() const
{
	if (m_eMirrorType == REFLECTION_LATERAL)
		return Matrix4x4().AddReflection(GetGlobalTransform().GetForwardVector());
	else if (m_eMirrorType == REFLECTION_VERTICAL)
		return Matrix4x4().AddReflection(Vector(0, 1, 0));

	return Matrix4x4();
}
