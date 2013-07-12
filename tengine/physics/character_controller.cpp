#include "character_controller.h"

#include "LinearMath/btIDebugDraw.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btMultiSphereShape.h"
#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"
#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "LinearMath/btDefaultMotionState.h"

#include <game/entities/baseentity.h>
#include <game/entities/character.h>
#include <tinker/application.h>
#include <tinker/cvar.h>

#include "bullet_physics.h"

// Originally forked from Bullet's btKinematicCharacterController

// static helper method
static btVector3 getNormalizedVector(const btVector3& v)
{
	btVector3 n = v.normalized();
	if (n.length() < SIMD_EPSILON) {
		n.setValue(0, 0, 0);
	}
	return n;
}

class btKinematicClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
	btKinematicClosestNotMeConvexResultCallback (CCharacterController* pController, btCollisionObject* me, const btVector3& up)
		: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0)), m_me(me), m_up(up)
	{
		m_pController = pController;
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		if (!m_pController->IsColliding())
			return 1;

		if (convexResult.m_hitCollisionObject == m_me)
			return btScalar(1.0);

		if (convexResult.m_hitCollisionObject->getBroadphaseHandle()->m_collisionFilterGroup & CG_TRIGGER)
			return 1;

		if (!convexResult.m_hitCollisionObject->hasContactResponse())
			return btScalar(1.0);

		if ((size_t)convexResult.m_hitCollisionObject->getUserPointer() >= GameServer()->GetMaxEntities())
		{
			size_t iIndex = (size_t)convexResult.m_hitCollisionObject->getUserPointer() - GameServer()->GetMaxEntities();
			CBaseEntity* pControllerEntity = m_pController->GetEntity();
			TAssert(pControllerEntity);
			if (pControllerEntity)
			{
				TAssert(normalInWorldSpace);
				if (!pControllerEntity->ShouldCollideWithExtra(iIndex, Vector(convexResult.m_hitPointLocal)))
					return 1;
			}
		}
		else
		{
			CEntityHandle<CBaseEntity> hCollidedEntity((size_t)convexResult.m_hitCollisionObject->getUserPointer());
			TAssert(hCollidedEntity != NULL);
			if (hCollidedEntity.GetPointer())
			{
				CBaseEntity* pControllerEntity = m_pController->GetEntity();
				TAssert(pControllerEntity);
				if (pControllerEntity)
				{
					TAssert(normalInWorldSpace);
					if (!pControllerEntity->ShouldCollideWith(hCollidedEntity, Vector(convexResult.m_hitPointLocal)))
						return 1;
				}
			}
		}

		btVector3 hitNormalWorld;
		if (normalInWorldSpace)
		{
			hitNormalWorld = convexResult.m_hitNormalLocal;
		} else
		{
			///need to transform normal into worldspace
			hitNormalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal;
		}

		btScalar dotUp = m_up.dot(hitNormalWorld);
		if (dotUp < 0)
			return btScalar(1.0);

		return ClosestConvexResultCallback::addSingleResult (convexResult, normalInWorldSpace);
	}

protected:
	CCharacterController*	m_pController;
	btCollisionObject*		m_me;
	const btVector3			m_up;
};

CCharacterController::CCharacterController(CCharacter* pEntity, btPairCachingGhostObject* ghostObject,btConvexShape* convexShape,btScalar stepHeight)
{
	m_hEntity = pEntity;
	m_flAddedMargin = 0.02f;
	m_vecMoveVelocity.setValue(0,0,0);
	m_pGhostObject = ghostObject;
	m_flStepHeight = stepHeight;
	m_pConvexShape = convexShape;	
	m_vecGravity = btVector3(0, 0, -9.8f);
	m_vecUpVector = btVector3(0, 0, 1);
	m_vecLinearFactor = btVector3(1, 1, 1);
	m_flMaxSpeed = 55.0; // Terminal velocity of a sky diver in m/s.
	m_flJumpSpeed = 6.0;
	m_flCurrentStepOffset = m_flStepHeight;
	m_bColliding = true;
	SetMaxSlope(btRadians(45.0));
}

CCharacterController::~CCharacterController ()
{
}

void CCharacterController::updateAction(btCollisionWorld* pCollisionWorld, btScalar deltaTime)
{
	if (!m_bColliding)
		return;

	TAssert(dynamic_cast<CCharacter*>(GetEntity()));

	static_cast<CCharacter*>(GetEntity())->CharacterMovement(pCollisionWorld, deltaTime);
}

void CCharacterController::CharacterMovement(btCollisionWorld* pCollisionWorld, btScalar deltaTime)
{
	// Grab the new player transform before doing movement steps in case the player has been moved,
	// such as by a platform or teleported. No need to do a physics trace for it, the penetration
	// functions should handle that.
	btTransform mCharacter;
	CPhysicsEntity* pPhysicsEntity = static_cast<CBulletPhysics*>(GamePhysics())->GetPhysicsEntity(m_hEntity);
	pPhysicsEntity->m_oMotionState.getWorldTransform(mCharacter);

	m_pGhostObject->setWorldTransform(mCharacter);

	PreStep(pCollisionWorld);

	if (m_hEntity->IsFlying())
		PlayerFly(pCollisionWorld, deltaTime);
	else if (!m_hEntity->GetGroundEntity())
		PlayerFall(pCollisionWorld, deltaTime);
	else
		PlayerWalk(pCollisionWorld, deltaTime);

	FindGround(pCollisionWorld);

	btVector3 vecOrigin = m_pGhostObject->getWorldTransform().getOrigin();

	TAssert(vecOrigin.x() < 999999);
	TAssert(vecOrigin.x() > -999999);
	TAssert(vecOrigin.y() < 999999);
	TAssert(vecOrigin.y() > -999999);
	TAssert(vecOrigin.z() < 999999);
	TAssert(vecOrigin.z() > -999999);

	if ((mCharacter.getOrigin() - vecOrigin).length2() > 0.0001f)
		pPhysicsEntity->m_oMotionState.setWorldTransform(m_pGhostObject->getWorldTransform());
}

void CCharacterController::preStep(btCollisionWorld* pCollisionWorld)
{
	PreStep(pCollisionWorld);
}

CVar phys_maxpenetrationrecover("phys_maxpenetrationrecover", "4");

void CCharacterController::PreStep(btCollisionWorld* pCollisionWorld)
{
	m_bTouchingContact = false;
	int i = 0;
	while (RecoverFromPenetration(pCollisionWorld))
	{
		i++;
		m_bTouchingContact = true;

		if (i > phys_maxpenetrationrecover.GetInt())
		{
			TMsg(sprintf("%f Character controller couldn't recover from penetration.\n", GameServer()->GetGameTime()));
			break;
		}
	}
}

void CCharacterController::playerStep(btCollisionWorld* pCollisionWorld, btScalar dt)
{
	TAssert(false);
	PlayerWalk(pCollisionWorld, dt);
}

CVar sv_friction("sv_friction", "8");

void CCharacterController::PlayerWalk(btCollisionWorld* pCollisionWorld, btScalar dt)
{
	Vector vecCurrentVelocity = m_hEntity->GetLocalVelocity();
	vecCurrentVelocity.z = 0;

	// Calculate friction first, so that the player's movement commands can overwhelm it.
	if (vecCurrentVelocity.Length2DSqr() > 0.00001f)
	{
		if (m_hEntity->GetGroundEntity())
		{
			float flSpeed2D = vecCurrentVelocity.Length2D();

			float flFriction = sv_friction.GetFloat() * flSpeed2D * dt;

			float flNewSpeed = flSpeed2D - flFriction;

			if (flNewSpeed < 0)
				flNewSpeed = 0;

			float flScale = flNewSpeed / flSpeed2D;

			vecCurrentVelocity *= flScale;
		}
	}

	// Calculate a new velocity using the player's desired direction of travel.
	btVector3 vecWishDirection = m_vecMoveVelocity;
	vecWishDirection.setZ(0);
	float flWishVelocity = m_vecMoveVelocity.length();
	if (flWishVelocity)
		vecWishDirection = vecWishDirection / flWishVelocity;
	else
		vecWishDirection = btVector3(0, 0, 0);

	if (flWishVelocity > m_hEntity->CharacterSpeed())
		flWishVelocity = m_hEntity->CharacterSpeed();

	float flVelocityInWishDirection = GetVelocity().dot(vecWishDirection);

	float flDirectionChange = flWishVelocity - flVelocityInWishDirection;

	if (flDirectionChange > 0)
	{
		float flAccelerationAmount = flWishVelocity * m_hEntity->CharacterAcceleration() * dt;
		if (flAccelerationAmount > flDirectionChange)
			flAccelerationAmount = flDirectionChange;
		vecCurrentVelocity = vecCurrentVelocity + ToTVector(vecWishDirection) * flAccelerationAmount;

		TAssert(vecCurrentVelocity.z == 0);
	}

	float flCurrentVelocity = vecCurrentVelocity.Length();

	if (flCurrentVelocity > m_hEntity->CharacterSpeed())
		vecCurrentVelocity *= m_hEntity->CharacterSpeed()/flCurrentVelocity;

	if (vecCurrentVelocity.LengthSqr() < 0.001f)
	{
		m_hEntity->SetLocalVelocity(Vector());
		return;
	}

	btTransform mWorld = m_pGhostObject->getWorldTransform();

	// Try moving the player directly, if it's possible.
	{
		btVector3 vecTargetPosition = mWorld.getOrigin() + ToBTVector(vecCurrentVelocity) * dt;

		CTraceResult tr;
		PlayerTrace(pCollisionWorld, mWorld.getOrigin(), vecTargetPosition, tr);

		if (tr.m_flFraction == 1)
		{
			mWorld.setOrigin(vecTargetPosition);
			m_pGhostObject->setWorldTransform(mWorld);
			m_hEntity->SetLocalVelocity(vecCurrentVelocity);
			return;
		}
	}

	// There was something blocking the way. Try walking up a step or along walls.

	btVector3 vecOriginalPosition = mWorld.getOrigin();

	StepForwardAndStrafe(pCollisionWorld, ToBTVector(vecCurrentVelocity) * dt);

	StepDown(pCollisionWorld, dt);

	btVector3 vecTraveled = (m_pGhostObject->getWorldTransform().getOrigin() - vecOriginalPosition) * m_vecLinearFactor;

	m_hEntity->SetLocalVelocity(ToTVector(vecTraveled)/dt);
}

CVar sv_air_movement("sv_air_movement", "0.2");

void CCharacterController::PlayerFall(btCollisionWorld* pCollisionWorld, btScalar dt)
{
	Vector vecCurrentVelocity = m_hEntity->GetLocalVelocity();
	vecCurrentVelocity += ToTVector(GetGravity()) * dt;
	m_hEntity->SetLocalVelocity(vecCurrentVelocity);

	btVector3 vecVelocity = ToBTVector(vecCurrentVelocity);

	if (m_vecMoveVelocity.length2())
	{
		btVector3 vecAllowedMoveVelocity = PerpendicularComponent(m_vecMoveVelocity, GetUpVector());
		if (vecAllowedMoveVelocity.dot(vecVelocity) < 0)
			vecVelocity = PerpendicularComponent(vecVelocity, vecAllowedMoveVelocity.normalized());

		vecVelocity += vecAllowedMoveVelocity * dt;
	}

	if (vecVelocity.length2() > m_flMaxSpeed*m_flMaxSpeed)
		vecVelocity = vecVelocity.normalized() * m_flMaxSpeed;

	if (vecVelocity.length2() < 0.001f)
		return;

	btTransform mWorld;
	mWorld = m_pGhostObject->getWorldTransform();

	btVector3 vecOriginalPosition = mWorld.getOrigin();

	StepForwardAndStrafe(pCollisionWorld, vecVelocity * dt);
}

void CCharacterController::PlayerFly(btCollisionWorld* pCollisionWorld, btScalar dt)
{
	m_hEntity->SetLocalVelocity(ToTVector(m_vecMoveVelocity));

	if (m_vecMoveVelocity.length2() < 0.001f)
		return;

	btTransform mWorld;
	mWorld = m_pGhostObject->getWorldTransform();

	btVector3 vecOriginalPosition = mWorld.getOrigin();

	StepForwardAndStrafe(pCollisionWorld, m_vecMoveVelocity * dt);
}

bool CCharacterController::PlayerTrace(btCollisionWorld* pCollisionWorld, const btVector3& vecStart, const btVector3& vecEnd, CTraceResult& tr)
{
	btTransform mStart, mEnd;
	mStart.setIdentity();
	mEnd.setIdentity();
	mStart.setOrigin(vecStart);
	mEnd.setOrigin(vecEnd);

	btKinematicClosestNotMeConvexResultCallback callback(this, m_pGhostObject, vecStart - vecEnd);
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

	btScalar margin = m_pConvexShape->getMargin();
	m_pConvexShape->setMargin(margin + m_flAddedMargin);

	m_pGhostObject->convexSweepTest (m_pConvexShape, mStart, mEnd, callback, pCollisionWorld->getDispatchInfo().m_allowedCcdPenetration);

	m_pConvexShape->setMargin(margin);

	if (callback.m_closestHitFraction < tr.m_flFraction)
	{
		tr.m_flFraction = callback.m_closestHitFraction;
		tr.m_vecHit = ToTVector(callback.m_hitPointWorld);
		tr.m_vecNormal = ToTVector(callback.m_hitNormalWorld);
		tr.m_iHit = (size_t)callback.m_hitCollisionObject->getUserPointer();
		if ((size_t)callback.m_hitCollisionObject->getUserPointer() >= GameServer()->GetMaxEntities())
			tr.m_iHitExtra = (size_t)callback.m_hitCollisionObject->getUserPointer() - GameServer()->GetMaxEntities();
	}

	return callback.hasHit();
}

void CCharacterController::setWalkDirection(const btVector3& walkDirection)
{
	SetMoveVelocity(walkDirection);
}

void CCharacterController::setVelocityForTimeInterval(const btVector3& velocity, btScalar timeInterval)
{
	SetMoveVelocity(velocity/timeInterval);
}

void CCharacterController::warp(const btVector3& origin)
{
	btTransform mNew;
	mNew.setIdentity();
	mNew.setOrigin(origin);
	m_pGhostObject->setWorldTransform(mNew);
}

bool CCharacterController::canJump() const
{
	return !!m_hEntity->GetGroundEntity();
}

void CCharacterController::jump()
{
	if (!canJump())
		return;

	SetJumpSpeed((float)m_hEntity->JumpStrength());

	m_hEntity->SetGlobalVelocity(ToTVector(GetUpVector()) * m_flJumpSpeed + m_hEntity->GetGlobalVelocity());

	m_hEntity->SetGroundEntity(nullptr);
}

bool CCharacterController::onGround() const
{
	return GetVelocity().dot(GetUpVector()) > 0;
}

void CCharacterController::SetMoveVelocity(const btVector3& velocity)
{
	m_vecMoveVelocity = velocity;
	m_vecMoveVelocityNormalized = getNormalizedVector(velocity);
}

btVector3 CCharacterController::GetMoveVelocity() const
{
	return m_vecMoveVelocity;
}

void CCharacterController::SetMaxSpeed (btScalar flMaxSpeed)
{
	m_flMaxSpeed = flMaxSpeed;
}

void CCharacterController::SetJumpSpeed (btScalar flJumpSpeed)
{
	m_flJumpSpeed = flJumpSpeed;
}

void CCharacterController::SetGravity(const btVector3& vecGravity)
{
	m_vecGravity = vecGravity;
}

btVector3 CCharacterController::GetGravity() const
{
	return m_vecGravity;
}

btVector3 CCharacterController::GetVelocity() const
{
	return ToBTVector(m_hEntity->GetLocalVelocity());
}

void CCharacterController::SetMaxSlope(btScalar slopeRadians)
{
	m_flMaxSlopeRadians = slopeRadians;
	m_flMaxSlopeCosine = btCos(slopeRadians);
}

btScalar CCharacterController::GetMaxSlope() const
{
	return m_flMaxSlopeRadians;
}

btPairCachingGhostObject* CCharacterController::getGhostObject()
{
	return m_pGhostObject;
}

CBaseEntity* CCharacterController::GetEntity() const
{
	return m_hEntity;
}

// Returns the reflection direction of a ray going 'direction' hitting a surface with normal 'normal'
// from: http://www-cs-students.stanford.edu/~adityagp/final/node3.html
btVector3 CCharacterController::ComputeReflectionDirection(const btVector3& direction, const btVector3& normal)
{
	return direction - (btScalar(2.0) * direction.dot(normal)) * normal;
}

// Returns the portion of 'direction' that is parallel to 'normal'
btVector3 CCharacterController::ParallelComponent(const btVector3& direction, const btVector3& normal)
{
	btScalar magnitude = direction.dot(normal);
	return normal * magnitude;
}

// Returns the portion of 'direction' that is perpindicular to 'normal'
btVector3 CCharacterController::PerpendicularComponent(const btVector3& direction, const btVector3& normal)
{
	return direction - ParallelComponent(direction, normal);
}

bool CCharacterController::RecoverFromPenetration(btCollisionWorld* pCollisionWorld)
{
	if (!IsColliding())
		return false;

	bool bPenetration = false;

	pCollisionWorld->getDispatcher()->dispatchAllCollisionPairs(m_pGhostObject->getOverlappingPairCache(), pCollisionWorld->getDispatchInfo(), pCollisionWorld->getDispatcher());

	btVector3 vecCurrentPosition = m_pGhostObject->getWorldTransform().getOrigin();
	btVector3 vecOriginalPosition = vecCurrentPosition;

	btScalar maxPen = btScalar(0.0);
	for (int i = 0; i < m_pGhostObject->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
	{
		m_aManifolds.resize(0);

		btBroadphasePair* pCollisionPair = &m_pGhostObject->getOverlappingPairCache()->getOverlappingPairArray()[i];

		btCollisionObject* pObject0 = static_cast<btCollisionObject*>(pCollisionPair->m_pProxy0->m_clientObject);
		btCollisionObject* pObject1 = static_cast<btCollisionObject*>(pCollisionPair->m_pProxy1->m_clientObject);

		if (!pObject0->hasContactResponse() || !pObject1->hasContactResponse())
			continue;

		if (pObject0->getBroadphaseHandle()->m_collisionFilterGroup == CG_TRIGGER || pObject1->getBroadphaseHandle()->m_collisionFilterGroup == CG_TRIGGER)
			continue;

		if (pCollisionPair->m_algorithm)
			pCollisionPair->m_algorithm->getAllContactManifolds(m_aManifolds);

		for (int j = 0; j < m_aManifolds.size(); j++)
		{
			btPersistentManifold* pManifold = m_aManifolds[j];

			btCollisionObject* obA = static_cast<btCollisionObject*>(pManifold->getBody0());
			btCollisionObject* obB = static_cast<btCollisionObject*>(pManifold->getBody1());

			btScalar directionSign;
			CEntityHandle<CBaseEntity> hOther;
			size_t iExtra;
			if (obA == m_pGhostObject)
			{
				if (obB->getBroadphaseHandle()->m_collisionFilterGroup & btBroadphaseProxy::SensorTrigger)
					continue;

				directionSign = btScalar(-1.0);
				hOther = CEntityHandle<CBaseEntity>((size_t)obB->getUserPointer());
				iExtra = (size_t)obB->getUserPointer()-GameServer()->GetMaxEntities();

				if (obB->getCollisionFlags()&btCollisionObject::CF_CHARACTER_OBJECT)
				{
					// If I'm heavier than he, don't let him push me around
					if (hOther->GetMass() < m_hEntity->GetMass())
						continue;
				}
			}
			else
			{
				if (obA->getBroadphaseHandle()->m_collisionFilterGroup & btBroadphaseProxy::SensorTrigger)
					continue;

				directionSign = btScalar(1.0);
				hOther = CEntityHandle<CBaseEntity>((size_t)obA->getUserPointer());
				iExtra = (size_t)obB->getUserPointer()-GameServer()->GetMaxEntities();

				if (obA->getCollisionFlags()&btCollisionObject::CF_CHARACTER_OBJECT)
				{
					// If I'm heavier than he, don't let him push me around
					if (hOther->GetMass() < m_hEntity->GetMass())
						continue;
				}
			}

			for (int p = 0; p < pManifold->getNumContacts(); p++)
			{
				const btManifoldPoint& pt = pManifold->getContactPoint(p);

				if (obA == m_pGhostObject)
				{
					if (hOther)
					{
						if (!m_hEntity->ShouldCollideWith(hOther, Vector(pt.getPositionWorldOnB())))
							continue;
					}
					else
					{
						if (!m_hEntity->ShouldCollideWithExtra(iExtra, Vector(pt.getPositionWorldOnB())))
							continue;
					}
				}
				else
				{
					if (hOther)
					{
						if (!m_hEntity->ShouldCollideWith(hOther, Vector(pt.getPositionWorldOnA())))
							continue;
					}
					else
					{
						if (!m_hEntity->ShouldCollideWithExtra(iExtra, Vector(pt.getPositionWorldOnA())))
							continue;
					}
				}

				btScalar flDistance = pt.getDistance();
				btScalar flMargin = std::max(obA->getCollisionShape()->getMargin(), obB->getCollisionShape()->getMargin());

				if (flDistance < -flMargin)
				{
					flDistance += flMargin;

					if (flDistance < maxPen)
					{
						maxPen = flDistance;
						m_vecTouchingNormal = pt.m_normalWorldOnB * directionSign;
					}

					btScalar flDot = pt.m_normalWorldOnB.dot(GetUpVector());
					btVector3 vecAdjustment;
					if (flDot > 0.707f)
						vecAdjustment = GetUpVector() * (directionSign * flDistance * 1.001f);
					else
						vecAdjustment = pt.m_normalWorldOnB * (directionSign * flDistance * 1.001f);

					if (vecAdjustment.length2() < 0.001*0.001)
						continue;

					vecCurrentPosition += vecAdjustment;

					bPenetration = true;
				} else {
					//printf("touching %f\n", dist);
				}
			}

			//pManifold->clearManifold();
		}
	}

	btTransform mNew = m_pGhostObject->getWorldTransform();
	mNew.setOrigin(mNew.getOrigin() + (vecCurrentPosition - vecOriginalPosition) * m_vecLinearFactor);
	m_pGhostObject->setWorldTransform(mNew);

	//printf("m_vecTouchingNormal = %f,%f,%f\n", m_vecTouchingNormal[0], m_vecTouchingNormal[1], m_vecTouchingNormal[2]);

	return bPenetration;
}

void CCharacterController::StepUp(btCollisionWorld* pWorld)
{
	// phase 1: up
	btTransform start, end;
	btVector3 vecTargetPosition = m_pGhostObject->getWorldTransform().getOrigin() + GetUpVector() * m_flStepHeight;

	start.setIdentity ();
	end.setIdentity ();

	/* FIXME: Handle penetration properly */
	start.setOrigin (m_pGhostObject->getWorldTransform().getOrigin() + GetUpVector() * (m_pConvexShape->getMargin() + m_flAddedMargin));
	end.setOrigin (vecTargetPosition);

	btKinematicClosestNotMeConvexResultCallback callback(this, m_pGhostObject, -GetUpVector());
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

	m_pGhostObject->convexSweepTest (m_pConvexShape, start, end, callback, pWorld->getDispatchInfo().m_allowedCcdPenetration);

	if (callback.hasHit())
	{
		// Only modify the position if the hit was a slope and not a wall or ceiling.
		if(callback.m_hitNormalWorld.dot(GetUpVector()) > 0.0)
		{
			// we moved up only a fraction of the step height
			m_flCurrentStepOffset = m_flStepHeight * callback.m_closestHitFraction;
			btVector3 vecInterpolated;
			vecInterpolated.setInterpolate3(m_pGhostObject->getWorldTransform().getOrigin(), vecTargetPosition, callback.m_closestHitFraction);
			m_pGhostObject->getWorldTransform().setOrigin(vecInterpolated);
		}
	} else {
		m_flCurrentStepOffset = m_flStepHeight;
		m_pGhostObject->getWorldTransform().setOrigin(vecTargetPosition);
	}
}

btVector3 CCharacterController::UpdateTargetPositionBasedOnCollision(const btVector3& vecMovementSpeed, const btVector3& vecHitNormal)
{
	btVector3 vecTargetPosition;

	btVector3 vecMovementDirection = vecMovementSpeed.normalized();

	btScalar flMovementLength = vecMovementDirection.length();

	if (flMovementLength > SIMD_EPSILON)
	{
		vecMovementDirection.normalize();

		btVector3 vecReflectDir = ComputeReflectionDirection(vecMovementDirection, vecHitNormal);
		vecReflectDir.normalize();

		btVector3 vecParallelDir, vecPerpendicularDir;

		vecParallelDir = ParallelComponent(vecReflectDir, vecHitNormal);
		vecPerpendicularDir = PerpendicularComponent(vecReflectDir, vecHitNormal);

		if (m_hEntity->IsFlying() || !m_hEntity->GetGroundEntity())
			vecPerpendicularDir.normalize();

		vecTargetPosition = m_pGhostObject->getWorldTransform().getOrigin();

		btVector3 vecPerpComponent = vecPerpendicularDir * flMovementLength;
		vecTargetPosition += vecPerpComponent;

		return vecTargetPosition;
	}
	else
	{
		TAssert(false); // Is this ever called?
		return m_pGhostObject->getWorldTransform().getOrigin();
	}
}

void CCharacterController::StepForwardAndStrafe(btCollisionWorld* pCollisionWorld, const btVector3& vecWalkMove)
{
	//printf("m_vecNormalizedDirection=%f,%f,%f\n", m_vecNormalizedDirection[0],m_vecNormalizedDirection[1],m_vecNormalizedDirection[2]);

	// phase 2: forward and strafe
	btTransform mStart, mEnd;
	btVector3 vecStartPosition = m_pGhostObject->getWorldTransform().getOrigin();
	btVector3 vecTargetPosition = vecStartPosition + vecWalkMove;

	mStart.setIdentity ();
	mEnd.setIdentity ();

	btScalar flFraction = 1.0;
	btScalar flDistance2 = (vecStartPosition-vecTargetPosition).length2();

	if (m_bTouchingContact)
	{
		if (m_vecMoveVelocityNormalized.dot(m_vecTouchingNormal) > btScalar(0.0))
			UpdateTargetPositionBasedOnCollision(vecTargetPosition - vecStartPosition, m_vecTouchingNormal);
	}

	int iMaxIter = 4;

	while (flFraction > btScalar(0.01) && iMaxIter-- > 0)
	{
		mStart.setOrigin(vecStartPosition);
		mEnd.setOrigin(vecTargetPosition);
		btVector3 sweepDirNegative(vecStartPosition - vecTargetPosition);

		btKinematicClosestNotMeConvexResultCallback callback(this, m_pGhostObject, sweepDirNegative);
		callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
		callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

		btScalar margin = m_pConvexShape->getMargin();
		m_pConvexShape->setMargin(margin + m_flAddedMargin);

		m_pGhostObject->convexSweepTest (m_pConvexShape, mStart, mEnd, callback, pCollisionWorld->getDispatchInfo().m_allowedCcdPenetration);

		m_pConvexShape->setMargin(margin);

		flFraction -= callback.m_closestHitFraction;

		if (callback.hasHit())
		{	
			vecStartPosition.setInterpolate3 (vecStartPosition, vecTargetPosition, callback.m_closestHitFraction);

			UpdateTargetPositionBasedOnCollision (vecTargetPosition - vecStartPosition, callback.m_hitNormalWorld);

			vecStartPosition += callback.m_hitNormalWorld * margin;
			vecTargetPosition += callback.m_hitNormalWorld * margin;

			btVector3 currentDir = vecTargetPosition - vecStartPosition;
			flDistance2 = currentDir.length2();
			if (flDistance2 > SIMD_EPSILON)
			{
				currentDir.normalize();
				/* See Quake2: "If velocity is against original velocity, stop ead to avoid tiny oscilations in sloping corners." */
				if (currentDir.dot(GetVelocity().normalized()) <= btScalar(0.0))
					break;
			} else
			{
				//printf("currentDir: don't normalize a zero vector\n");
				break;
			}

		} else {
			// we moved whole way
			vecStartPosition = vecTargetPosition;
		}
	}

	m_pGhostObject->getWorldTransform().setOrigin(vecStartPosition);
}

void CCharacterController::StepDown(btCollisionWorld* pCollisionWorld, btScalar dt)
{
	btTransform mStart, mEnd;

	btVector3 vecStepDrop = GetUpVector() * m_flStepHeight;
	btVector3 vecStartPosition = m_pGhostObject->getWorldTransform().getOrigin();
	btVector3 vecTargetPosition = vecStartPosition - vecStepDrop;

	mStart.setIdentity();
	mEnd.setIdentity();

	mStart.setOrigin(m_pGhostObject->getWorldTransform().getOrigin());
	mEnd.setOrigin(vecTargetPosition);

	btKinematicClosestNotMeConvexResultCallback callback(this, m_pGhostObject, GetUpVector());
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

	m_pGhostObject->convexSweepTest (m_pConvexShape, mStart, mEnd, callback, pCollisionWorld->getDispatchInfo().m_allowedCcdPenetration);

	if (callback.m_closestHitFraction > 0.5f)
	{
		// Don't move him all the way down, just enough to stay within step height of the ground.
		float flFraction = callback.m_closestHitFraction - 0.5f;
		m_pGhostObject->getWorldTransform().setOrigin( vecStartPosition + flFraction * (vecTargetPosition - vecStartPosition) );
	}
}

void CCharacterController::FindGround(btCollisionWorld* pCollisionWorld)
{
	if (m_hEntity->IsFlying())
	{
		m_hEntity->SetGroundEntity(nullptr);
		return;
	}

	if (GetVelocity().dot(GetUpVector()) > m_flJumpSpeed/2.0f)
	{
		m_hEntity->SetGroundEntity(nullptr);
		return;
	}

	bool bWalking = !m_hEntity->IsFlying() && m_hEntity->GetGroundEntity();

	float flDropHeight = bWalking?m_flCurrentStepOffset:m_pConvexShape->getMargin()*2;

	btVector3 vecStepDrop = GetUpVector() * flDropHeight;
	btVector3 vecGroundPosition = m_pGhostObject->getWorldTransform().getOrigin() - vecStepDrop;

	CTraceResult tr;
	PlayerTrace(pCollisionWorld, m_pGhostObject->getWorldTransform().getOrigin(), vecGroundPosition, tr);

	if (tr.m_flFraction < 1)
	{
		btScalar flDot = GetUpVector().dot(ToBTVector(tr.m_vecNormal));
		if (flDot < m_flMaxSlopeCosine)
			m_hEntity->SetGroundEntity(nullptr);
		else
		{
			CEntityHandle<CBaseEntity> hOther = CEntityHandle<CBaseEntity>(tr.m_iHit);
			if (hOther)
				m_hEntity->SetGroundEntity(hOther);
			else
			{
				m_hEntity->SetGroundEntityExtra(tr.m_iHitExtra);
			}
		}
	}
	else
		m_hEntity->SetGroundEntity(nullptr);

	if (bWalking && m_hEntity->GetGroundEntity() && tr.m_flFraction > 0.0f && tr.m_flFraction < 1.0f)
	{
		btVector3 vecNewOrigin;
		vecNewOrigin.setInterpolate3(m_pGhostObject->getWorldTransform().getOrigin(), vecGroundPosition, tr.m_flFraction);

		float flMargin = m_pConvexShape->getMargin() + 0.001f;
		if ((vecNewOrigin - m_pGhostObject->getWorldTransform().getOrigin()).length2() > flMargin*flMargin)
		{
			m_pGhostObject->getWorldTransform().setOrigin(vecNewOrigin);
			return;
		}
	}

	return;
}
