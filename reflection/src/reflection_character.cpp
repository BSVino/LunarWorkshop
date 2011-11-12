#include "reflection_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>

#include "reflection_game.h"
#include "reflection_renderer.h"
#include "reflection_playercharacter.h"
#include "mirror.h"

REGISTER_ENTITY(CReflectionCharacter);

NETVAR_TABLE_BEGIN(CReflectionCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bReflected);
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

	m_bReflected = false;
}

TVector CReflectionCharacter::GetGoalVelocity()
{
	TVector vecGoalVelocity = BaseClass::GetGoalVelocity();

	if (IsReflected())
		vecGoalVelocity.z = -vecGoalVelocity.z;

	return vecGoalVelocity;
}

void CReflectionCharacter::OnSetLocalTransform(Matrix4x4& mNew)
{
	if (!ReflectionGame()->GetLocalPlayerCharacter())
		return;

	Vector vecNewOrigin = mNew.GetTranslation();

	CMirror* pMirror = static_cast<CPlayerCharacter*>(ReflectionGame()->GetLocalPlayerCharacter())->GetMirror();
	if (pMirror && vecNewOrigin != m_vecLocalOrigin)
	{
		Vector vecOldGlobalOrigin = GetGlobalOrigin();
		Vector vecNewGlobalOrigin;
		TAssert(!HasMoveParent());	// Hasn't been tested with move parents.
		if (HasMoveParent())
			vecNewGlobalOrigin = GetMoveParent()->GetGlobalTransform() * vecNewOrigin;
		else
			vecNewGlobalOrigin = vecNewOrigin;

		Matrix4x4 mMirror = pMirror->GetGlobalTransform();
		bool bOldSide = (vecOldGlobalOrigin - mMirror.GetTranslation()).Normalized().Dot(mMirror.GetForwardVector()) > 0;
		bool bNewSide = (vecNewGlobalOrigin - mMirror.GetTranslation()).Normalized().Dot(mMirror.GetForwardVector()) > 0;

		if(bOldSide != bNewSide && pMirror->IsPointInside(GetGlobalCenter()))
		{
			Matrix4x4 mReflection;
			mReflection.AddReflection(mMirror.GetForwardVector());

			// Write out reflected origin.
			mNew.SetTranslation(mReflection * (vecNewGlobalOrigin - mMirror.GetTranslation()) + mMirror.GetTranslation());

			// Should be on the same side as the old side, since it was reflected.
			bool bReflectedSide = (mNew.GetTranslation() - mMirror.GetTranslation()).Normalized().Dot(mMirror.GetForwardVector()) > 0;
			TAssert(bReflectedSide == bOldSide);

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

			m_vecMoveVelocity.z = -m_vecMoveVelocity.z;

			m_bReflected = !m_bReflected;

			if (m_bReflected)
				m_hMirrorInside = pMirror;
			else
				m_hMirrorInside = NULL;

			Reflected();
		}
	}
}

CMirror* CReflectionCharacter::GetMirrorInside() const
{
	return m_hMirrorInside;
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
		return !IsNearMirror(static_cast<CPlayerCharacter*>(ReflectionGame()->GetLocalPlayerCharacter())->GetMirror(), vecPoint);

	return true;
}
