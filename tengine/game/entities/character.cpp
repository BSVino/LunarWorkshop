#include "character.h"

#include <matrix.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <game/game.h>
#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>
#include <physics/physics.h>

#include "player.h"

REGISTER_ENTITY(CCharacter);

NETVAR_TABLE_BEGIN(CCharacter);
	NETVAR_DEFINE(CEntityHandle, m_hControllingPlayer);
	NETVAR_DEFINE(CEntityHandle, m_hGround);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_hControllingPlayer);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, EAngle, m_angView, "ViewAngles");
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_hGround);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bTransformMoveByView);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecGoalVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecMoveVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastAttack);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TFloat, m_flMaxStepSize);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCharacter);
INPUTS_TABLE_END();

CCharacter::CCharacter()
{
	m_bTransformMoveByView = true;

	SetMass(60);
}

void CCharacter::Spawn()
{
	BaseClass::Spawn();

	SetTotalHealth(100);

	m_vecMoveVelocity = Vector(0,0,0);
	m_vecGoalVelocity = Vector(0,0,0);

	m_flLastAttack = -1;

	m_flMaxStepSize = 10;

	m_bTakeDamage = true;

	m_flMaxStepSize = 0.1f;

	AddToPhysics(CT_CHARACTER);
}

void CCharacter::Think()
{
	BaseClass::Think();

	MoveThink();
}

void CCharacter::Move(movetype_t eMoveType)
{
	if (eMoveType == MOVE_FORWARD)
		m_vecGoalVelocity.x = 1;
	else if (eMoveType == MOVE_BACKWARD)
		m_vecGoalVelocity.x = -1;
	else if (eMoveType == MOVE_RIGHT)
		m_vecGoalVelocity.z = 1;
	else if (eMoveType == MOVE_LEFT)
		m_vecGoalVelocity.z = -1;
}

void CCharacter::StopMove(movetype_t eMoveType)
{
	if (eMoveType == MOVE_FORWARD && m_vecGoalVelocity.x > 0)
		m_vecGoalVelocity.x = 0;
	else if (eMoveType == MOVE_BACKWARD && m_vecGoalVelocity.x < 0)
		m_vecGoalVelocity.x = 0;
	else if (eMoveType == MOVE_LEFT && m_vecGoalVelocity.z < 0)
		m_vecGoalVelocity.z = 0;
	else if (eMoveType == MOVE_RIGHT && m_vecGoalVelocity.z > 0)
		m_vecGoalVelocity.z = 0;
}

TVector CCharacter::GetGoalVelocity()
{
	if (m_vecGoalVelocity.LengthSqr())
		m_vecGoalVelocity.Normalize();

	return m_vecGoalVelocity;
}

void CCharacter::MoveThink()
{
	float flSimulationFrameTime = 0.01f;

	TVector vecGoalVelocity = GetGoalVelocity();

	m_vecMoveVelocity.x = Approach(vecGoalVelocity.x, m_vecMoveVelocity.x, GameServer()->GetFrameTime()*CharacterAcceleration());
	m_vecMoveVelocity.y = 0;
	m_vecMoveVelocity.z = Approach(vecGoalVelocity.z, m_vecMoveVelocity.z, GameServer()->GetFrameTime()*CharacterAcceleration());

	if (m_vecMoveVelocity.LengthSqr() > 0)
	{
		TVector vecMove = m_vecMoveVelocity * CharacterSpeed();
		TVector vecLocalVelocity;

		if (m_bTransformMoveByView)
		{
			Vector vecUp = GetUpVector();
		
			if (HasMoveParent() && GetMoveParent()->TransformsChildUp())
			{
				TMatrix mGlobalToLocal = GetMoveParent()->GetGlobalToLocalTransform();
				vecUp = mGlobalToLocal.TransformVector(vecUp);
			}

			TMatrix m = GetLocalTransform();
			m.SetAngles(GetViewAngles());

			Vector vecRight = m.GetForwardVector().Cross(vecUp).Normalized();
			Vector vecForward = vecUp.Cross(vecRight).Normalized();

			m.SetForwardVector(vecForward);
			m.SetUpVector(vecUp);
			m.SetRightVector(vecRight);

			vecLocalVelocity = m.TransformVector(vecMove);
		}
		else
			vecLocalVelocity = vecMove;

		GamePhysics()->SetControllerWalkVelocity(this, vecLocalVelocity);
	}
	else
		GamePhysics()->SetControllerWalkVelocity(this, Vector(0, 0, 0));
}

void CCharacter::Jump()
{
	GamePhysics()->CharacterJump(this);
}

bool CCharacter::CanAttack() const
{
	if (m_flLastAttack >= 0 && GameServer()->GetGameTime() - m_flLastAttack < AttackTime())
		return false;

	return true;
}

void CCharacter::Attack()
{
	if (!CanAttack())
		return;

	m_flLastAttack = GameServer()->GetGameTime();
	m_vecMoveVelocity = TVector();

	TFloat flAttackSphereRadius = AttackSphereRadius();
	TVector vecDamageSphereCenter = AttackSphereCenter();

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t j = 0; j < iMaxEntities; j++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(j);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (!pEntity->TakesDamage())
			continue;

		if (pEntity == this)
			continue;

		TFloat flRadius = pEntity->GetBoundingRadius() + flAttackSphereRadius;
		flRadius = flRadius*flRadius;
		if ((vecDamageSphereCenter - pEntity->GetGlobalCenter()).LengthSqr() > flRadius)
			continue;

		pEntity->TakeDamage(this, this, DAMAGE_GENERIC, AttackDamage());
	}
}

bool CCharacter::IsAttacking() const
{
	if (m_flLastAttack < 0)
		return false;

	return (GameServer()->GetGameTime() - m_flLastAttack < AttackTime());
}

void CCharacter::MoveToPlayerStart()
{
	CBaseEntity* pPlayerStart = CBaseEntity::GetEntityByName("*CPlayerStart");

	SetMoveParent(NULL);

	if (!pPlayerStart)
	{
		SetGlobalOrigin(Vector(0, 0, 0));
		SetGlobalAngles(EAngle(0, 0, 0));
		return;
	}

	SetGlobalOrigin(pPlayerStart->GetGlobalOrigin());
	m_angView = pPlayerStart->GetGlobalAngles();
}

CVar debug_showplayervectors("debug_showplayervectors", "off");

void CCharacter::PostRender(bool bTransparent) const
{
	if (!bTransparent && debug_showplayervectors.GetBool())
		ShowPlayerVectors();
}

void CCharacter::ShowPlayerVectors() const
{
	TMatrix m = GetGlobalTransform();
	m.SetAngles(m_angView);

	Vector vecUp = GetUpVector();
	Vector vecRight = m.GetForwardVector().Cross(vecUp).Normalized();
	Vector vecForward = vecUp.Cross(vecRight).Normalized();
	m.SetForwardVector(vecForward);
	m.SetUpVector(vecUp);
	m.SetRightVector(vecRight);

	CCharacter* pLocalCharacter = Game()->GetLocalPlayer()->GetCharacter();

	TVector vecEyeHeight = GetUpVector() * EyeHeight();

	CRenderingContext c(GameServer()->GetRenderer());

	c.UseProgram("model");
	c.Translate((GetGlobalOrigin()));
	c.SetColor(Color(255, 255, 255));
	c.BeginRenderDebugLines();
	c.Vertex(Vector(0,0,0));
	c.Vertex((float)EyeHeight() * vecUp);
	c.EndRender();

	if (!GetGlobalVelocity().IsZero())
	{
		c.BeginRenderDebugLines();
		c.Vertex(vecEyeHeight);
		c.Vertex(vecEyeHeight + GetGlobalVelocity());
		c.EndRender();
	}

	c.SetColor(Color(255, 0, 0));
	c.BeginRenderDebugLines();
	c.Vertex(vecEyeHeight);
	c.Vertex(vecEyeHeight + vecForward);
	c.EndRender();

	c.SetColor(Color(0, 255, 0));
	c.BeginRenderDebugLines();
	c.Vertex(vecEyeHeight);
	c.Vertex(vecEyeHeight + vecRight);
	c.EndRender();

	c.SetColor(Color(0, 0, 255));
	c.BeginRenderDebugLines();
	c.Vertex(vecEyeHeight);
	c.Vertex(vecEyeHeight + vecUp);
	c.EndRender();
}

void CCharacter::SetControllingPlayer(CPlayer* pCharacter)
{
	m_hControllingPlayer = pCharacter;
}

CPlayer* CCharacter::GetControllingPlayer() const
{
	return m_hControllingPlayer;
}

void CCharacter::SetGroundEntity(CBaseEntity* pEntity)
{
	if ((CBaseEntity*)m_hGround == pEntity)
		return;

	m_hGround = pEntity;
	SetMoveParent(pEntity);
}
