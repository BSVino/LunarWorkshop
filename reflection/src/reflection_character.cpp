#include "reflection_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>
#include <physics/physics.h>

#include "reflection_game.h"
#include "reflection_renderer.h"
#include "reflection_playercharacter.h"
#include "mirror.h"

REGISTER_ENTITY(CReflectionCharacter);

NETVAR_TABLE_BEGIN(CReflectionCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iReflected);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Matrix4x4, m_mLateralReflection);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Matrix4x4, m_mVerticalReflection);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CMirror>, m_hMirrorInside);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionCharacter);
INPUTS_TABLE_END();

CReflectionCharacter::CReflectionCharacter()
{
}

void CReflectionCharacter::Spawn()
{
	BaseClass::Spawn();

	SetGlobalGravity(Vector(0, -9.8f, 0));
	m_flMaxStepSize = 0.1f;

	m_iReflected = 0;
}

TVector CReflectionCharacter::GetGoalVelocity()
{
	TVector vecGoalVelocity = BaseClass::GetGoalVelocity();

	if (IsReflected(REFLECTION_LATERAL) ^ IsReflected(REFLECTION_VERTICAL))
		vecGoalVelocity.z = -vecGoalVelocity.z;

	return vecGoalVelocity;
}

float CReflectionCharacter::EyeHeight() const
{
	return 1.8f;
}

void CReflectionCharacter::OnSetLocalTransform(Matrix4x4& mNew)
{
	BaseClass::OnSetLocalTransform(mNew);

	if (!ReflectionGame()->GetLocalPlayerCharacter())
		return;

	for (size_t i = 0; i < CMirror::GetNumMirrors(); i++)
	{
		CMirror* pMirror = CMirror::GetMirror(i);
		if (!pMirror)
			continue;

		TestMirror(pMirror, mNew);
	}
}

void CReflectionCharacter::TestMirror(CMirror* pMirror, Matrix4x4& mNew)
{
	Vector vecNewOrigin = mNew.GetTranslation();

	if (pMirror && vecNewOrigin != m_vecLocalOrigin)
	{
		Vector vecOldGlobalOrigin = GetGlobalOrigin();
		Vector vecNewGlobalOrigin;
		if (HasMoveParent())
			vecNewGlobalOrigin = GetMoveParent()->GetGlobalTransform() * vecNewOrigin;
		else
			vecNewGlobalOrigin = vecNewOrigin;

		Matrix4x4 mMirror = pMirror->GetGlobalTransform();
		bool bOldSide = pMirror->GetSide(vecOldGlobalOrigin + GetUpVector() * EyeHeight());
		bool bNewSide = pMirror->GetSide(vecNewGlobalOrigin + GetUpVector() * EyeHeight());

		bool bPointInsideCheck = pMirror->IsPointInside(vecOldGlobalOrigin + GetUpVector() * EyeHeight(), false);

		if(bOldSide != bNewSide && bPointInsideCheck)
		{
			Matrix4x4 mReflection = pMirror->GetReflection();

			// Write out reflected origin.
			mNew.SetTranslation(mReflection * (vecNewGlobalOrigin - mMirror.GetTranslation()) + mMirror.GetTranslation());

#ifdef _DEBUG
			// Should be on the same side as the old side, since it was reflected.
			bool bOldReflectedSide = pMirror->GetSide(vecOldGlobalOrigin + GetUpVector() * EyeHeight());
			bool bNewReflectedSide = pMirror->GetSide(mNew.GetTranslation());
			TAssert(bOldReflectedSide == bNewReflectedSide);
#endif

			// Reflect the velocity
			Vector vecVelocity = GetGlobalVelocity();
			Vector vecReflectedVelocity = mReflection.TransformVector(vecVelocity);
			SetGlobalVelocity(vecReflectedVelocity);

			// Reflect the character's orientation
			Vector vecForward = GetGlobalTransform().GetForwardVector();
			Vector vecReflectedForward = mReflection.TransformVector(vecForward);
			mNew.SetOrientation(vecReflectedForward);

			// Reflect the character's viewing vector
			Vector vecView = AngleVector(GetViewAngles());
			SetViewAngles(VectorAngles(mReflection.TransformVector(vecView)));

			// Reflect the character's gravity
			Vector vecGravity = GetGlobalGravity();
			Vector vecReflectedGravity = mReflection.TransformVector(vecGravity);
			SetGlobalGravity(vecReflectedGravity);

			// Reflect the bounding box
			Vector vecMaxs = GetBoundingBox().m_vecMaxs;
			Vector vecMins = GetBoundingBox().m_vecMins;
			Vector vecReflectedMaxs = mReflection.TransformVector(vecMaxs);
			Vector vecReflectedMins = mReflection.TransformVector(vecMins);

#define swap_if_greater(x, y) \
	if ((x) > (y)) \
	{ \
		float f = (x); \
		(x) = (y); \
		(y) = f; \
	} \

			swap_if_greater(vecReflectedMins.x, vecReflectedMaxs.x);
			swap_if_greater(vecReflectedMins.y, vecReflectedMaxs.y);
			swap_if_greater(vecReflectedMins.z, vecReflectedMaxs.z);

			m_aabbBoundingBox = AABB(vecReflectedMins, vecReflectedMaxs);

			if (pMirror->GetReflectionType() == REFLECTION_LATERAL)
				m_vecMoveVelocity.z = -m_vecMoveVelocity.z;

			bool bWasReflected = !!(m_iReflected&(1<<pMirror->GetReflectionType()));
			if (bWasReflected)
				m_iReflected &= ~(1<<pMirror->GetReflectionType());
			else
				m_iReflected |= (1<<pMirror->GetReflectionType());

			if (IsReflected(REFLECTION_ANY))
				m_hMirrorInside = pMirror;
			else
				m_hMirrorInside = NULL;

			if (pMirror->GetReflectionType() == REFLECTION_VERTICAL)
			{
				if (IsReflected(REFLECTION_VERTICAL))
					GamePhysics()->SetEntityUpVector(this, Vector(0, -1, 0));
				else
					GamePhysics()->SetEntityUpVector(this, Vector(0, 1, 0));

				if (IsReflected(REFLECTION_VERTICAL))
					m_mVerticalReflection.SetReflection(Vector(0, 1, 0));
				else
					m_mVerticalReflection.Identity();
			}
			else if (pMirror->GetReflectionType() == REFLECTION_LATERAL)
			{
				if (IsReflected(REFLECTION_LATERAL))
					m_mLateralReflection.SetReflection(pMirror->GetGlobalTransform().GetForwardVector());
				else
					m_mLateralReflection.Identity();
			}

			Reflected(pMirror->GetReflectionType());
		}
	}
}

bool CReflectionCharacter::IsReflected(reflection_t eReflectionType) const
{
	if (eReflectionType == REFLECTION_ANY)
		return !!m_iReflected;

	return !!(m_iReflected&(1<<eReflectionType));
}

CMirror* CReflectionCharacter::GetMirrorInside() const
{
	return m_hMirrorInside;
}

TVector CReflectionCharacter::GetUpVector() const
{
	if (IsReflected(REFLECTION_VERTICAL))
		return Vector(0, -1, 0);
	else
		return Vector(0, 1, 0);
}

bool CReflectionCharacter::IsNearMirror(class CMirror* pMirror, const Vector& vecPoint) const
{
	if (!pMirror)
		return false;

	return pMirror->IsPointInside(vecPoint);
}

bool CReflectionCharacter::ShouldCollideWith(CBaseEntity* pOther, const TVector& vecPoint) const
{
	if (tstring(pOther->GetClassName()) == "CWorld")
	{
		for (size_t i = 0; i < CMirror::GetNumMirrors(); i++)
		{
			CMirror* pMirror = CMirror::GetMirror(i);
			if (!pMirror)
				continue;

			if (IsNearMirror(pMirror, vecPoint))
				return false;
		}
	}

	return true;
}
