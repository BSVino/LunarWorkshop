#include "grotto_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>
#include <physics/physics.h>

#include "grotto_game.h"
#include "grotto_renderer.h"
#include "grotto_playercharacter.h"
#include "mirror.h"
#include "reflectionproxy.h"
#include "depthtransitionarea.h"

REGISTER_ENTITY(CGrottoCharacter);

NETVAR_TABLE_BEGIN(CGrottoCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrottoCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iReflected);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Matrix4x4, m_mLateralReflection);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Matrix4x4, m_mVerticalReflection);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CMirror>, m_hMirrorInside);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CDepthTransitionArea>, m_ahTouchingDepthTransitionArea);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoCharacter);
	INPUT_DEFINE(ReflectVertical);
	INPUT_DEFINE(ReflectLateral);
INPUTS_TABLE_END();

CGrottoCharacter::CGrottoCharacter()
{
}

void CGrottoCharacter::Spawn()
{
	BaseClass::Spawn();

	m_iReflected = 0;

	GamePhysics()->SetLinearFactor(this, Vector(0, 1, 1));
}

const TVector CGrottoCharacter::GetGoalVelocity()
{
	TVector vecGoalVelocity = BaseClass::GetGoalVelocity();

	if (IsReflected(REFLECTION_LATERAL) ^ IsReflected(REFLECTION_VERTICAL))
		vecGoalVelocity.z = -vecGoalVelocity.z;

	return vecGoalVelocity;
}

void CGrottoCharacter::OnSetLocalTransform(Matrix4x4& mNew)
{
	BaseClass::OnSetLocalTransform(mNew);

	if (!GrottoGame()->GetLocalPlayerCharacter())
		return;

	Vector vecGlobal = GetParentGlobalTransform() * mNew.GetTranslation();

	for (size_t i = 0; i < CMirror::GetNumMirrors(); i++)
	{
		CMirror* pMirror = CMirror::GetMirror(i);
		if (!pMirror)
			continue;

		if ((pMirror->GetGlobalOrigin() - vecGlobal).LengthSqr() > m_aabbPhysBoundingBox.Size().LengthSqr()*2)
			continue;

		TestMirror(pMirror, mNew);
	}
}

void CGrottoCharacter::TestMirror(CMirror* pMirror, Matrix4x4& mNew)
{
	Vector vecNewOrigin = mNew.GetTranslation();

	if (pMirror && vecNewOrigin != m_vecLocalOrigin)
	{
		Vector vecOldGlobalOrigin = GetGlobalOrigin();
		Vector vecNewGlobalOrigin = GetParentGlobalTransform() * vecNewOrigin;

		Matrix4x4 mMirror = pMirror->GetGlobalTransform();

		Vector vecOldView = vecOldGlobalOrigin + GetUpVector() * EyeHeight();
		Vector vecNewView = vecNewGlobalOrigin + GetUpVector() * EyeHeight();
		bool bOldSide = pMirror->GetSide(vecOldView);
		bool bNewSide = pMirror->GetSide(vecNewView);

		bool bPointInsideCheck = pMirror->IsPointInside(vecOldView, false);

		if(bOldSide != bNewSide && bPointInsideCheck)
			Reflect(mMirror, pMirror->GetReflection(), pMirror->GetReflectionType(), mNew, pMirror);
	}
}

void CGrottoCharacter::Reflect(const Matrix4x4& mMirror, const Matrix4x4& mReflection, reflection_t eReflectionType, Matrix4x4& mNew, CMirror* pMirror)
{
	Vector vecNewOrigin = mNew.GetTranslation();
	Vector vecNewGlobalOrigin = GetParentGlobalTransform() * vecNewOrigin;
	Vector vecOldLocalOrigin = GetLocalOrigin();

	// Write out reflected origin.
	Vector vecNewReflectedGlobalOrigin = mReflection * (vecNewGlobalOrigin - mMirror.GetTranslation()) + mMirror.GetTranslation();

	if (HasMoveParent())
		mNew.SetTranslation(GetMoveParent()->GetGlobalToLocalTransform() * vecNewReflectedGlobalOrigin);
	else
		mNew.SetTranslation(vecNewReflectedGlobalOrigin);

#ifdef _DEBUG
	if (pMirror)
	{
		Matrix4x4 mLocalToGlobal;
		if (HasMoveParent())
			mLocalToGlobal = GetMoveParent()->GetGlobalTransform();

		bool bOldOriginalSide = pMirror->GetSide((mLocalToGlobal * vecOldLocalOrigin) + GetUpVector() * EyeHeight());
		bool bNewOriginalSide = pMirror->GetSide((mLocalToGlobal * vecNewOrigin) + GetUpVector() * EyeHeight());
		bool bNewReflectedSide = pMirror->GetSide((mLocalToGlobal * mNew.GetTranslation()) + GetUpVector() * EyeHeight());

		if (bOldOriginalSide == bNewOriginalSide)
			TAssert(bOldOriginalSide != bNewReflectedSide)
		else
			// Should be on the same side as the old side, since it was reflected.
			TAssert(bOldOriginalSide == bNewReflectedSide);
	}
#endif

	// Reflect the velocity
	Vector vecVelocity = GetGlobalVelocity();
	Vector vecReflectedVelocity = mReflection.TransformVector(vecVelocity);
	SetGlobalVelocity(vecReflectedVelocity);

	// Reflect the character's orientation
	Vector vecForward = GetGlobalTransform().GetForwardVector();
	Vector vecReflectedForward = mReflection.TransformVector(vecForward);
	if (HasMoveParent())
		mNew.SetOrientation(GetMoveParent()->GetGlobalToLocalTransform().TransformVector(vecReflectedForward));
	else
		mNew.SetOrientation(vecReflectedForward);

	// Reflect the character's viewing vector
	Vector vecView = AngleVector(GetViewAngles());
	SetViewAngles(VectorAngles(mReflection.TransformVector(vecView)));

	// Reflect the character's gravity
	Vector vecGravity = GetGlobalGravity();
	Vector vecReflectedGravity = mReflection.TransformVector(vecGravity);
	SetGlobalGravity(vecReflectedGravity);

	// Reflect the bounding box
	Vector vecMaxs = GetPhysBoundingBox().m_vecMaxs;
	Vector vecMins = GetPhysBoundingBox().m_vecMins;
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

	m_aabbPhysBoundingBox = AABB(vecReflectedMins, vecReflectedMaxs);

	vecMaxs = GetVisBoundingBox().m_vecMaxs;
	vecMins = GetVisBoundingBox().m_vecMins;
	vecReflectedMaxs = mReflection.TransformVector(vecMaxs);
	vecReflectedMins = mReflection.TransformVector(vecMins);

	swap_if_greater(vecReflectedMins.x, vecReflectedMaxs.x);
	swap_if_greater(vecReflectedMins.y, vecReflectedMaxs.y);
	swap_if_greater(vecReflectedMins.z, vecReflectedMaxs.z);

	m_aabbVisBoundingBox = AABB(vecReflectedMins, vecReflectedMaxs);

	if (eReflectionType == REFLECTION_LATERAL)
		m_vecMoveVelocity.z = -m_vecMoveVelocity.z;

	bool bWasReflected = !!(m_iReflected&(1<<eReflectionType));
	if (bWasReflected)
		m_iReflected &= ~(1<<eReflectionType);
	else
		m_iReflected |= (1<<eReflectionType);

	if (IsReflected(REFLECTION_ANY))
		m_hMirrorInside = pMirror;
	else
		m_hMirrorInside = NULL;

	if (eReflectionType == REFLECTION_VERTICAL)
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
	else if (eReflectionType == REFLECTION_LATERAL)
	{
		if (IsReflected(REFLECTION_LATERAL))
			m_mLateralReflection.SetReflection(mMirror.GetForwardVector());
		else
			m_mLateralReflection.Identity();
	}

	Reflected(eReflectionType);

	CReflectionProxy::OnPlayerReflection(IsReflected(REFLECTION_LATERAL) ^ IsReflected(REFLECTION_VERTICAL));
	if (eReflectionType == REFLECTION_LATERAL)
		CReflectionProxy::OnPlayerGravity(IsReflected(REFLECTION_VERTICAL));
}

void CGrottoCharacter::ReflectVertical(const tvector<tstring>& asArgs)
{
	Matrix4x4 mReflection, mTransform;
	mReflection.SetReflection(Vector(0, 1, 0));
	mTransform = GetGlobalTransform();

	Reflect(GetGlobalTransform(), mReflection, REFLECTION_VERTICAL, mTransform);

	SetGlobalTransform(mTransform);
}

void CGrottoCharacter::ReflectLateral(const tvector<tstring>& asArgs)
{
	Matrix4x4 mReflection, mTransform;
	mReflection.SetReflection(Vector(1, 0, 0));
	mTransform = GetGlobalTransform();

	Reflect(GetGlobalTransform(), mReflection, REFLECTION_LATERAL, mTransform);

	SetGlobalTransform(mTransform);
}

bool CGrottoCharacter::IsReflected(reflection_t eReflectionType) const
{
	if (eReflectionType == REFLECTION_ANY)
		return !!m_iReflected;

	return !!(m_iReflected&(1<<eReflectionType));
}

CMirror* CGrottoCharacter::GetMirrorInside() const
{
	return m_hMirrorInside;
}

const TVector CGrottoCharacter::GetUpVector() const
{
	if (IsReflected(REFLECTION_VERTICAL))
		return Vector(0, -1, 0);
	else
		return Vector(0, 1, 0);
}

bool CGrottoCharacter::IsNearMirror(class CMirror* pMirror, const Vector& vecPoint) const
{
	if (!pMirror)
		return false;

	return pMirror->IsPointInside(vecPoint);
}

bool CGrottoCharacter::ShouldCollideWith(CBaseEntity* pOther, const TVector& vecPoint) const
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

void CGrottoCharacter::SetTouchingDepthTransitionArea(CDepthTransitionArea* pArea, bool bTouching)
{
	for (size_t i = 0; i < m_ahTouchingDepthTransitionArea.size(); i++)
	{
		if (m_ahTouchingDepthTransitionArea[i] == (const CDepthTransitionArea*)pArea)
		{
			if (!bTouching)
				m_ahTouchingDepthTransitionArea.erase(m_ahTouchingDepthTransitionArea.begin()+i);
			return;
		}
	}

	m_ahTouchingDepthTransitionArea.push_back(pArea);
}
