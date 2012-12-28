#include "mirror.h"

#include <models/models.h>
#include <physics/physics.h>

#include "grotto_renderer.h"
#include "grotto_game.h"
#include "grotto_character.h"

REGISTER_ENTITY(CMirror);

NETVAR_TABLE_BEGIN(CMirror);
NETVAR_TABLE_END();

void UnserializeString_MirrorType(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);

SAVEDATA_TABLE_BEGIN_EDITOR(CMirror);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_COPYTYPE, mirror_t, m_eMirrorType, "MirrorType", UnserializeString_MirrorType);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iBuffer);
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bDisableBackCulling, "DisableBackCulling", true);
	SAVEDATA_EDITOR_VARIABLE("DisableBackCulling");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMirror);
INPUTS_TABLE_END();

tvector<CEntityHandle<CMirror> > CMirror::s_ahMirrors;

CMirror::~CMirror()
{
	for (size_t i = 0; i < s_ahMirrors.size(); i++)
	{
		if (s_ahMirrors[i] == (const CMirror*)this)
		{
			s_ahMirrors.erase(s_ahMirrors.begin()+i);
			break;
		}
	}
}

void CMirror::Precache()
{
}

void CMirror::Spawn()
{
	m_iBuffer = ~0;

	SetMirrorType(MIRROR_VERTICAL);

	s_ahMirrors.push_back(this);
}

bool CMirror::IsPointInside(const Vector& vecPoint, bool bPhysics) const
{
	switch(m_eMirrorType)
	{
	case MIRROR_VERTICAL:
		if (GetGlobalTransform().GetUpVector().y > 0)
		{
			if (vecPoint.y < GetGlobalOrigin().y + 0.05f)
				return false;

			if (vecPoint.y > GetGlobalOrigin().y + GetPhysBoundingBox().Size().y)
				return false;
		}
		else
		{
			if (vecPoint.y > GetGlobalOrigin().y - 0.05f)
				return false;

			if (vecPoint.y < GetGlobalOrigin().y - GetPhysBoundingBox().Size().y)
				return false;
		}

		return (vecPoint - GetGlobalOrigin()).Length2D() < GetPhysBoundingBox().Size().Length2D()/2;

	case MIRROR_HORIZONTAL:
		if (bPhysics)
		{
			Vector vecUp = Matrix4x4(GetGlobalAngles(), Vector()).GetUpVector();

			if (vecUp.y > 0)
			{
				if (vecPoint.y < GetGlobalOrigin().y - vecUp.y * 2.0f)
					return false;

				if (vecPoint.y > GetGlobalOrigin().y + vecUp.y * 0.15f)
					return false;
			}
			else
			{
				if (vecPoint.y > GetGlobalOrigin().y - vecUp.y * 2.0f)
					return false;

				if (vecPoint.y < GetGlobalOrigin().y + vecUp.y * 0.15f)
					return false;
			}
		}

		CGrottoCharacter* pPlayerCharacter = GrottoGame()->GetLocalPlayerCharacter();
		AABB aabb = GetPhysBoundingBox();
		aabb.m_vecMins += pPlayerCharacter->GetPhysBoundingBox().m_vecMins;
		aabb.m_vecMaxs += pPlayerCharacter->GetPhysBoundingBox().m_vecMaxs;

		// Use a tighter area for physics to make sure we never fall outside the level
		if (bPhysics)
			return aabb.Inside2D(vecPoint - GetGlobalOrigin());
		else
			return (aabb*1.2f).Inside2D(vecPoint - GetGlobalOrigin());
	}

	return false;
}

void CMirror::SetMirrorType(mirror_t eType)
{
	Precache();

	m_eMirrorType = eType;

	if (IsInPhysics())
		RemoveFromPhysics();
	AddToPhysics(CT_KINEMATIC);
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

void UnserializeString_MirrorType(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CMirror* pMirror = static_cast<CMirror*>(pEntity);

	if (sData == "horizontal")
		pMirror->SetMirrorType(MIRROR_HORIZONTAL);
	else
		pMirror->SetMirrorType(MIRROR_VERTICAL);
}
