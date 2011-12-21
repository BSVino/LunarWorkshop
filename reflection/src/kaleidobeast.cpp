#include "kaleidobeast.h"

#include <physics/physics.h>
#include <renderer/renderer.h>
#include <tinker/gamewindow.h>

#include "reflection_game.h"
#include "reflection_character.h"

REGISTER_ENTITY(CKaleidobeast);

NETVAR_TABLE_BEGIN(CKaleidobeast);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CKaleidobeast);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bSeesPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bInitialPosition);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecInitialPosition);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angInitialPosition);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CKaleidobeast);
INPUTS_TABLE_END();

void CKaleidobeast::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/characters/kaleidobeast.toy");
}

void CKaleidobeast::Spawn()
{
	Precache();

	SetMass(600);

	SetModel("models/characters/kaleidobeast.toy");

	BaseClass::Spawn();

	m_bSeesPlayer = false;
	m_bInitialPosition = false;
}

void CKaleidobeast::Think()
{
	if (!m_bInitialPosition)
	{
		m_vecInitialPosition = GetGlobalOrigin();
		m_angInitialPosition = GetGlobalAngles();
		m_bInitialPosition = true;
	}

	m_bSeesPlayer = false;

	CReflectionCharacter* pPlayer = ReflectionGame()->GetLocalPlayerCharacter();
	if (pPlayer)
	{
		if (Distance(pPlayer->GetGlobalOrigin()) < 4)
			m_bSeesPlayer = true;
	}

	bool bPlayerSees = GameServer()->GetRenderer()->IsSphereInFrustum(GetGlobalCenter(), GetBoundingRadius());

	StopMove(MOVE_FORWARD);
	StopMove(MOVE_BACKWARD);
	StopMove(MOVE_LEFT);
	StopMove(MOVE_RIGHT);

	if (bPlayerSees)
	{
		if (pPlayer && CanSeePlayer())
		{
			SetViewAngles(VectorAngles(pPlayer->GetGlobalOrigin() - GetGlobalOrigin()));
			SetGlobalAngles(EAngle(0, GetViewAngles().y, 0));
		}
	}
	else
	{
		if (pPlayer && CanSeePlayer())
		{
			SetViewAngles(VectorAngles(pPlayer->GetGlobalOrigin() - GetGlobalOrigin()));
			SetGlobalAngles(EAngle(0, GetViewAngles().y, 0));
			m_bTransformMoveByView = true;
			Move(MOVE_FORWARD);
		}
		else
		{
			if ((GetGlobalOrigin() - m_vecInitialPosition).Length2DSqr() > 0.01f)
			{
				// Tiny hack to avoid the transforming here.
				m_bTransformMoveByView = false;
				m_vecGoalVelocity = (m_vecInitialPosition - GetGlobalOrigin()).Normalized();
			}

			SetViewAngles(m_angInitialPosition);
			SetGlobalAngles(m_angInitialPosition);
		}
	}

	if (pPlayer && (pPlayer->GetGlobalOrigin() - GetGlobalOrigin()).LengthSqr() < 1.9f*1.9f)
	{
		// Basically what he's doing here is committing suicide.
		GameWindow()->Restart("level");
	}

	BaseClass::Think();
}
