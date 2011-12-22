#include "kaleidobeast.h"

#include <physics/physics.h>
#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>
#include <tinker/gamewindow.h>
#include <game/entities/beam.h>

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
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CBeam>, m_hBeam);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CKaleidobeast);
INPUTS_TABLE_END();

CKaleidobeast::~CKaleidobeast()
{
	if (m_hBeam != nullptr)
		GameServer()->Delete(m_hBeam);
}

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

	m_hBeam = GameServer()->Create<CBeam>("CBeam");
}

void CKaleidobeast::Think()
{
	if (!m_bInitialPosition)
	{
		m_vecInitialPosition = GetGlobalOrigin();
		m_angInitialPosition = GetGlobalAngles();
		m_bInitialPosition = true;

		m_hBeam->SetStart(m_vecInitialPosition + GetUpVector()*EyeHeight());
		m_hBeam->SetEnd(m_vecInitialPosition + GetUpVector()*EyeHeight() + AngleVector(m_angInitialPosition)*100);
	}

	CReflectionCharacter* pPlayer = ReflectionGame()->GetLocalPlayerCharacter();

	if (!m_bSeesPlayer)
	{
		if (pPlayer && Distance(pPlayer->GetGlobalOrigin()) < DetectionDistance())
			m_bSeesPlayer = true;

		if (DistanceToLine(pPlayer->GetGlobalOrigin(), m_vecInitialPosition, m_vecInitialPosition + AngleVector(m_angInitialPosition)*100) < 2)
			m_bSeesPlayer = true;
	}

	if (m_bSeesPlayer)
	{
		if (fabs(pPlayer->GetGlobalOrigin().y - GetGlobalOrigin().y) > 2)
			m_bSeesPlayer = false;
	}

	m_hBeam->SetColor(m_bSeesPlayer?Color(255, 0, 0):Color(255, 255, 255));

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

void CKaleidobeast::PostRender(bool bTransparent) const
{
	if (!bTransparent)
		return;

	CRenderingContext c(GameServer()->GetRenderer());

	c.Translate(GetGlobalOrigin());
	c.Scale(DetectionDistance(), DetectionDistance(), DetectionDistance());
	c.SetBlend(BLEND_ALPHA);
	c.SetColor(m_bSeesPlayer?Color(255, 0, 0, 50):Color(255, 255, 255, 50));
	c.RenderSphere();
}

void CKaleidobeast::LosePlayer()
{
	m_bSeesPlayer = false;
}
