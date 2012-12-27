#include "grotto_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <game/entities/game.h>
#include <tinker/cvar.h>

#include "grotto_character.h"
#include "grotto_playercharacter.h"
#include "grotto_game.h"
#include "grotto_renderer.h"
#include "mirror.h"

REGISTER_ENTITY(CGrottoCamera);

NETVAR_TABLE_BEGIN(CGrottoCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CGrottoCamera);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angTarget);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angTargetGoal);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flLastTargetChange);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flDistance);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bLockToPlayerHeight, "LockToPlayerHeight", false);
	SAVEDATA_EDITOR_VARIABLE("LockToPlayerHeight");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoCamera);
INPUTS_TABLE_END();

void CGrottoCamera::Spawn()
{
	BaseClass::Spawn();

	m_flLastTargetChange = 0;

	m_flDistance = 0;
}

void CGrottoCamera::Think()
{
	if (!m_hCameraTarget)
	{
		m_hCameraTarget = GrottoGame()->GetLocalPlayerCharacter();
		m_angTarget = m_hCameraTarget->GetGlobalAngles();
		m_angTargetGoal = m_hCameraTarget->GetGlobalAngles();
	}

	if (m_flDistance == 0)
	{
		Vector vecCameraClosestPoint;
		DistanceToLineSegment(GetGlobalOrigin(), m_hTargetStart->GetGlobalOrigin(), m_hTargetEnd->GetGlobalOrigin(), &vecCameraClosestPoint);
		m_flDistance = (GetGlobalOrigin() - vecCameraClosestPoint).Length();
	}

	BaseClass::Think();
}

CVar cam_fliptime("cam_fliptime", "0.5");

void CGrottoCamera::CameraThink()
{
	BaseClass::CameraThink();

	if (IsAutoTracking())
	{
		bool bTurnedAround = false;
		if (fabs(m_hCameraTarget->GetLocalAngles().y) > 100)
			bTurnedAround = true;

		Vector vecTargetPosition = m_hCameraTarget->GetGlobalOrigin();

		Vector vecTargetStart = m_hTargetStart->GetGlobalOrigin();
		Vector vecTargetEnd = m_hTargetEnd->GetGlobalOrigin();

		EAngle angTargetStart;
		EAngle angTargetEnd;

		if (bTurnedAround)
		{
			angTargetStart = VectorAngles(Matrix4x4().AddReflection(Vector(1, 0, 0)) * AngleVector(m_hTargetStart->GetGlobalAngles()));
			angTargetEnd = VectorAngles(Matrix4x4().AddReflection(Vector(1, 0, 0)) * AngleVector(m_hTargetEnd->GetGlobalAngles()));

			angTargetEnd.y = angTargetStart.y + AngleDifference(angTargetEnd.y, angTargetStart.y);
		}
		else
		{
			angTargetStart = m_hTargetStart->GetGlobalAngles();
			angTargetEnd = m_hTargetEnd->GetGlobalAngles();
		}

		Vector vecClosestPoint;
		DistanceToLineSegment(vecTargetPosition, vecTargetStart, vecTargetEnd, &vecClosestPoint);

		float flTargetLength = (vecTargetEnd-vecTargetStart).Length();
		TAssert(flTargetLength > 0);

		float flFraction = (vecClosestPoint - vecTargetStart).Length()/flTargetLength;

		EAngle angCamera = RemapValClamped<EAngle>(flFraction, 0, 1, angTargetStart, angTargetEnd);

		if (!m_hCameraTarget->GetGlobalAngles().Equals(m_angTargetGoal, 0.1f))
		{
			m_flLastTargetChange = GameServer()->GetGameTime();
			m_angTarget = m_angTargetGoal;
			m_angTargetGoal = m_hCameraTarget->GetGlobalAngles();
		}

		if (GameServer()->GetGameTime() < m_flLastTargetChange+cam_fliptime.GetFloat())
		{
			float flGameTime = (float)GameServer()->GetGameTime();
			float flChangeStart = (float)m_flLastTargetChange;
			float flChangeEnd = (float)(m_flLastTargetChange+cam_fliptime.GetFloat());

			float flLerp = Lerp(RemapValClamped(flGameTime, flChangeStart, flChangeEnd, 0.0f, 1.0f), 0.8f);

			if (bTurnedAround)
			{
				angTargetStart = m_hTargetStart->GetGlobalAngles();
				angTargetEnd = m_hTargetEnd->GetGlobalAngles();
			}
			else
			{
				angTargetStart = VectorAngles(Matrix4x4().AddReflection(Vector(1, 0, 0)) * AngleVector(m_hTargetStart->GetGlobalAngles()));
				angTargetEnd = VectorAngles(Matrix4x4().AddReflection(Vector(1, 0, 0)) * AngleVector(m_hTargetEnd->GetGlobalAngles()));

				angTargetEnd.y = angTargetStart.y + AngleDifference(angTargetEnd.y, angTargetStart.y);
			}

			EAngle angCameraFrom = RemapValClamped<EAngle>(flGameTime, flChangeStart, flChangeEnd, angTargetStart, angTargetEnd);

			SetGlobalAngles(RemapValClamped<EAngle>(flLerp, 0, 1, angCameraFrom, angCamera));
		}
		else
		{
			m_flLastTargetChange = 0;
			m_angTarget = m_hCameraTarget->GetGlobalAngles();
			m_angTargetGoal = m_hCameraTarget->GetGlobalAngles();
			SetGlobalAngles(angCamera);
		}

		Vector vecCamera = vecClosestPoint - AngleVector(GetGlobalAngles())*m_flDistance;

		if (m_bLockToPlayerHeight)
			vecCamera.y = m_hCameraTarget->GetGlobalOrigin().y + m_hCameraTarget->GetPhysBoundingBox().Center().y;

		SetGlobalOrigin(vecCamera);
	}
}

bool CGrottoCamera::IsAutoTracking()
{
	if (m_hCameraTarget && m_hTargetStart && m_hTargetEnd)
		return true;

	return false;
}

void CGrottoCamera::Reflect(CMirror* pMirror)
{
	TAssert(false);	 // Tested but not currently use, warrants a re-test if you want to use it.

	TAssert(pMirror);
	if (!pMirror)
		return;

	Matrix4x4 mNew = GetGlobalTransform();

	Vector vecOrigin = GetGlobalOrigin();
	Vector vecGlobalOrigin = GetParentGlobalTransform() * vecOrigin;
	Vector vecOldLocalOrigin = GetLocalOrigin();

	// Write out reflected origin.
	Vector vecNewReflectedGlobalOrigin = pMirror->GetReflection() * (vecGlobalOrigin - pMirror->GetGlobalOrigin()) + pMirror->GetGlobalOrigin();

	if (HasMoveParent())
		mNew.SetTranslation(GetMoveParent()->GetGlobalToLocalTransform() * vecNewReflectedGlobalOrigin);
	else
		mNew.SetTranslation(vecNewReflectedGlobalOrigin);

	// Reflect orientation.
	Vector vecForward = GetGlobalTransform().GetForwardVector();
	Vector vecReflectedForward = pMirror->GetReflection().TransformVector(vecForward);
	if (HasMoveParent())
		mNew.SetOrientation(GetMoveParent()->GetGlobalToLocalTransform().TransformVector(vecReflectedForward));
	else
		mNew.SetOrientation(vecReflectedForward);

	SetGlobalTransform(mNew);

	Vector vecTarget = AngleVector(m_angTarget);
	Vector vecReflectedTarget = pMirror->GetReflection().TransformVector(vecTarget);
	m_angTarget = VectorAngles(vecReflectedTarget);

	Vector vecTargetGoal = AngleVector(m_angTargetGoal);
	Vector vecReflectedTargetGoal = pMirror->GetReflection().TransformVector(vecTargetGoal);
	m_angTargetGoal = VectorAngles(vecReflectedTargetGoal);
}
