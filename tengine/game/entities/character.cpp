#include "character.h"

#include <matrix.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tengine/game/game.h>

#include <tengine/renderer/renderer.h>
#include <tengine/renderer/renderingcontext.h>

#include "player.h"

REGISTER_ENTITY(CCharacter);

NETVAR_TABLE_BEGIN(CCharacter);
	NETVAR_DEFINE(CEntityHandle, m_hControllingPlayer);
	NETVAR_DEFINE(CEntityHandle, m_hGround);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_hControllingPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_hGround);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bTransformMoveByView);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecGoalVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecMoveVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastAttack);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flMoveSimulationTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TFloat, m_flMaxStepSize);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCharacter);
INPUTS_TABLE_END();

CCharacter::CCharacter()
{
	m_bTransformMoveByView = true;
}

void CCharacter::Spawn()
{
	BaseClass::Spawn();

	SetSimulated(false);
	SetTotalHealth(100);

	m_vecMoveVelocity = Vector(0,0,0);
	m_vecGoalVelocity = Vector(0,0,0);

	m_flLastAttack = -1;

	m_flMoveSimulationTime = 0;

	m_flMaxStepSize = 10;

	m_bTakeDamage = true;
}

void CCharacter::Think()
{
	FindGroundEntity();

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

	if (!GetGroundEntity())
	{
		// Keep the sim time current
		for (; m_flMoveSimulationTime < GameServer()->GetGameTime(); m_flMoveSimulationTime += flSimulationFrameTime);

		return;
	}

	TVector vecGoalVelocity = GetGoalVelocity();

	m_vecMoveVelocity.x = Approach(vecGoalVelocity.x, m_vecMoveVelocity.x, GameServer()->GetFrameTime()*4);
	m_vecMoveVelocity.y = 0;
	m_vecMoveVelocity.z = Approach(vecGoalVelocity.z, m_vecMoveVelocity.z, GameServer()->GetFrameTime()*4);

	if (m_vecMoveVelocity.LengthSqr() > 0)
	{
		TVector vecMove = m_vecMoveVelocity * CharacterSpeed();
		TVector vecLocalVelocity;

		if (m_bTransformMoveByView)
		{
			Vector vecUp = GetUpVector();
		
			if (HasMoveParent())
			{
				TMatrix mGlobalToLocal = GetMoveParent()->GetGlobalToLocalTransform();
				vecUp = mGlobalToLocal.TransformVector(vecUp);
			}

			TMatrix m = GetLocalTransform();

			Vector vecRight = m.GetForwardVector().Cross(vecUp).Normalized();
			Vector vecForward = vecUp.Cross(vecRight).Normalized();

			m.SetForwardVector(vecForward);
			m.SetUpVector(vecUp);
			m.SetRightVector(vecRight);

			vecLocalVelocity = m.TransformVector(vecMove);
		}
		else
			vecLocalVelocity = vecMove;

		SetLocalVelocity(vecLocalVelocity);
	}
	else
		SetLocalVelocity(TVector());

	eastl::vector<CEntityHandle<CBaseEntity> > apCollisionList;

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t j = 0; j < iMaxEntities; j++)
	{
		CBaseEntity* pEntity2 = CBaseEntity::GetEntity(j);

		if (!pEntity2)
			continue;

		if (pEntity2->IsDeleted())
			continue;

		if (pEntity2 == this)
			continue;

		if (!pEntity2->ShouldCollide())
			continue;

		apCollisionList.push_back(pEntity2);
	}

	TMatrix mGlobalToLocalRotation;
	if (HasMoveParent())
	{
		mGlobalToLocalRotation = GetMoveParent()->GetGlobalToLocalTransform();
		mGlobalToLocalRotation.SetTranslation(TVector());
	}

	// Break simulations up into consistent small steps to preserve accuracy.
	for (; m_flMoveSimulationTime < GameServer()->GetGameTime(); m_flMoveSimulationTime += flSimulationFrameTime)
	{
		TVector vecVelocity = GetLocalVelocity();

		TVector vecLocalOrigin = GetLocalOrigin();
		TVector vecGlobalOrigin = GetGlobalOrigin();

		vecVelocity = vecVelocity * flSimulationFrameTime;

		TVector vecLocalDestination = vecLocalOrigin + vecVelocity;
		TVector vecGlobalDestination = vecLocalDestination;
		if (GetMoveParent())
			vecGlobalDestination = GetMoveParent()->GetGlobalTransform() * vecLocalDestination;

		TVector vecNewLocalOrigin = vecLocalDestination;

		size_t iTries = 0;
		while (true)
		{
			iTries++;

			TVector vecPoint, vecNormal;

			TVector vecLocalCollisionPoint, vecGlobalCollisionPoint;

			bool bContact = false;
			for (size_t i = 0; i < apCollisionList.size(); i++)
			{
				CBaseEntity* pEntity2 = apCollisionList[i];

				if (GetMoveParent() == pEntity2)
				{
					if (pEntity2->CollideLocal(vecLocalOrigin, vecLocalDestination, vecPoint, vecNormal))
					{
						bContact = true;
						Touching(pEntity2);
						vecLocalCollisionPoint = vecPoint;
						vecGlobalCollisionPoint = GetMoveParent()->GetGlobalTransform() * vecPoint;
					}
				}
				else
				{
					if (pEntity2->Collide(vecGlobalOrigin, vecGlobalDestination, vecPoint, vecNormal))
					{
						bContact = true;
						Touching(pEntity2);
						vecGlobalCollisionPoint = vecPoint;
						if (GetMoveParent())
						{
							vecLocalCollisionPoint = GetMoveParent()->GetGlobalToLocalTransform() * vecPoint;
							vecNormal = GetMoveParent()->GetGlobalToLocalTransform().TransformVector(vecNormal);
						}
						else
							vecLocalCollisionPoint = vecGlobalCollisionPoint;
					}
				}
			}

			if (bContact)
			{
				vecNewLocalOrigin = vecLocalCollisionPoint;
				vecVelocity -= vecLocalCollisionPoint - vecLocalOrigin;
			}

			if (!bContact)
				break;

			if (iTries > 4)
				break;

			vecLocalOrigin = vecLocalCollisionPoint;
			vecGlobalOrigin = vecGlobalCollisionPoint;

			// Clip the velocity to the surface normal of whatever we hit.
			TFloat flDistance = vecVelocity.Dot(vecNormal);

			vecVelocity = vecVelocity - vecNormal * flDistance;

			// Do it one more time just to make sure we're not headed towards the plane.
			TFloat flAdjust = vecVelocity.Dot(vecNormal);
			if (flAdjust < 0.0f)
				vecVelocity -= (vecNormal * flAdjust);

			vecLocalDestination = vecLocalOrigin + vecVelocity;

			if (GetMoveParent())
				vecGlobalDestination = GetMoveParent()->GetGlobalTransform() * vecLocalDestination;
			else
				vecGlobalDestination = vecLocalDestination;

			SetLocalVelocity(vecVelocity.Normalized() * GetLocalVelocity().Length());
		}

		SetLocalOrigin(vecNewLocalOrigin);

		// Try to keep the player on the ground.
		// Untested.
		/*TVector vecStart = GetGlobalOrigin() + GetGlobalTransform().GetUpVector()*m_flMaxStepSize;
		TVector vecEnd = GetGlobalOrigin() - GetGlobalTransform().GetUpVector()*m_flMaxStepSize;

		// First go up a bit
		TVector vecHit, vecNormal;
		Game()->TraceLine(GetGlobalOrigin(), vecStart, vecHit, vecNormal, NULL);
		vecStart = vecHit;

		// Now see if there's ground underneath us.
		bool bHit = Game()->TraceLine(vecStart, vecEnd, vecHit, vecNormal, NULL);
		if (bHit && vecNormal.y >= TFloat(0.7f))
			SetGlobalOrigin(vecHit);*/
	}
}

void CCharacter::Jump()
{
	if (!GetGroundEntity())
		return;

	SetGroundEntity(NULL);

	Vector vecLocalUp = GetUpVector();
	if (HasMoveParent())
	{
		TMatrix mGlobalToLocal = GetMoveParent()->GetGlobalToLocalTransform();
		vecLocalUp = mGlobalToLocal.TransformVector(vecLocalUp);
	}

	SetLocalVelocity(GetLocalVelocity() + vecLocalUp * JumpStrength());
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

CVar debug_showplayervectors("debug_showplayervectors", "off");

void CCharacter::PostRender(bool bTransparent) const
{
	if (!bTransparent && debug_showplayervectors.GetBool())
		ShowPlayerVectors();
}

void CCharacter::ShowPlayerVectors() const
{
	TMatrix m = GetGlobalTransform();

	Vector vecUp = GetUpVector();
	Vector vecRight = m.GetForwardVector().Cross(vecUp).Normalized();
	Vector vecForward = vecUp.Cross(vecRight).Normalized();
	m.SetForwardVector(vecForward);
	m.SetUpVector(vecUp);
	m.SetRightVector(vecRight);

	CCharacter* pLocalCharacter = Game()->GetLocalPlayer()->GetCharacter();

	TVector vecEyeHeight = GetUpVector() * EyeHeight();

	CRenderingContext c(GameServer()->GetRenderer());

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

	TVector vecPoint, vecNormal;
	if (Game()->TraceLine(GetGlobalOrigin(), GetGlobalOrigin() - GetUpVector()*100, vecPoint, vecNormal, NULL))
	{
		c.Translate(vecPoint - GetGlobalOrigin());
		c.Scale(0.1f, 0.1f, 0.1f);
		c.SetColor(Color(255, 255, 255));
		c.RenderSphere();
	}
}

void CCharacter::SetControllingPlayer(CPlayer* pCharacter)
{
	m_hControllingPlayer = pCharacter;
}

CPlayer* CCharacter::GetControllingPlayer() const
{
	return m_hControllingPlayer;
}

TVector CCharacter::GetGlobalGravity() const
{
	if (GetGroundEntity())
		return TVector();

	return BaseClass::GetGlobalGravity();
}

void CCharacter::FindGroundEntity()
{
	TVector vecVelocity = GetGlobalVelocity();

	if (vecVelocity.Dot(GetUpVector()) > JumpStrength()/2.0f)
	{
		SetGroundEntity(NULL);
		SetSimulated(true);
		return;
	}

	TVector vecUp = GetUpVector() * m_flMaxStepSize;

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t j = 0; j < iMaxEntities; j++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(j);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (!pEntity->ShouldCollide())
			continue;

		if (pEntity == this)
			continue;

		TVector vecPoint, vecNormal;
		if (GetMoveParent() == pEntity)
		{
			TMatrix mGlobalToLocal = GetMoveParent()->GetGlobalToLocalTransform();
			Vector vecUpLocal = mGlobalToLocal.TransformVector(GetUpVector()) * m_flMaxStepSize;

			if (pEntity->CollideLocal(GetLocalOrigin(), GetLocalOrigin() - vecUpLocal, vecPoint, vecNormal))
			{
				SetGroundEntity(pEntity);
				SetSimulated(false);

				return;
			}
		}
		else
		{
			if (pEntity->Collide(GetGlobalOrigin(), GetGlobalOrigin() - vecUp, vecPoint, vecNormal))
			{
				SetGroundEntity(pEntity);
				SetSimulated(false);

				return;
			}
		}
	}

	SetGroundEntity(NULL);
	SetSimulated(true);
}
