#include "character_controller.h"

#include "LinearMath/btIDebugDraw.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btMultiSphereShape.h"
#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"
#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "LinearMath/btDefaultMotionState.h"

#include <game/baseentity.h>

#include "bullet_physics.h"

// Originally forked from Bullet's btKinematicCharacterController

// static helper method
static btVector3
getNormalizedVector(const btVector3& v)
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
	btKinematicClosestNotMeConvexResultCallback (CCharacterController* pController, btCollisionObject* me, const btVector3& up, btScalar minSlopeDot)
	: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
	, m_me(me)
	, m_up(up)
	, m_minSlopeDot(minSlopeDot)
	{
		m_pController = pController;
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
	{
		if (convexResult.m_hitCollisionObject == m_me)
			return btScalar(1.0);

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
		if (dotUp < m_minSlopeDot) {
			return btScalar(1.0);
		}

		return ClosestConvexResultCallback::addSingleResult (convexResult, normalInWorldSpace);
	}

protected:
	CCharacterController*	m_pController;
	btCollisionObject*		m_me;
	const btVector3			m_up;
	btScalar				m_minSlopeDot;
};

/*
 * Returns the reflection direction of a ray going 'direction' hitting a surface with normal 'normal'
 *
 * from: http://www-cs-students.stanford.edu/~adityagp/final/node3.html
 */
btVector3 CCharacterController::computeReflectionDirection (const btVector3& direction, const btVector3& normal)
{
	return direction - (btScalar(2.0) * direction.dot(normal)) * normal;
}

/*
 * Returns the portion of 'direction' that is parallel to 'normal'
 */
btVector3 CCharacterController::parallelComponent (const btVector3& direction, const btVector3& normal)
{
	btScalar magnitude = direction.dot(normal);
	return normal * magnitude;
}

/*
 * Returns the portion of 'direction' that is perpindicular to 'normal'
 */
btVector3 CCharacterController::perpindicularComponent (const btVector3& direction, const btVector3& normal)
{
	return direction - parallelComponent(direction, normal);
}

CCharacterController::CCharacterController(CBaseEntity* pEntity, btPairCachingGhostObject* ghostObject,btConvexShape* convexShape,btScalar stepHeight, int upAxis)
{
	m_hEntity = pEntity;
	m_upAxis = upAxis;
	m_addedMargin = 0.02f;
	m_walkDirection.setValue(0,0,0);
	m_useGhostObjectSweepTest = true;
	m_ghostObject = ghostObject;
	m_stepHeight = stepHeight;
	m_turnAngle = btScalar(0.0);
	m_convexShape=convexShape;	
	m_useWalkDirection = true;	// use walk direction by default, legacy behavior
	m_velocityTimeInterval = 0.0;
	m_verticalVelocity = 0.0;
	m_verticalOffset = 0.0;
	m_gravity = btVector3(0, -9.8f, 0);
	m_vecUpVector = btVector3(0, 1, 0);
	m_fallSpeed = 55.0; // Terminal velocity of a sky diver in m/s.
	m_jumpSpeed = 4.0;
	m_wasOnGround = false;
	m_wasJumping = false;
	setMaxSlope(btRadians(45.0));
}

CCharacterController::~CCharacterController ()
{
}

btPairCachingGhostObject* CCharacterController::getGhostObject()
{
	return m_ghostObject;
}

bool CCharacterController::recoverFromPenetration ( btCollisionWorld* collisionWorld)
{

	bool penetration = false;

	collisionWorld->getDispatcher()->dispatchAllCollisionPairs(m_ghostObject->getOverlappingPairCache(), collisionWorld->getDispatchInfo(), collisionWorld->getDispatcher());

	m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
	
	btScalar maxPen = btScalar(0.0);
	for (int i = 0; i < m_ghostObject->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
	{
		m_manifoldArray.resize(0);

		btBroadphasePair* collisionPair = &m_ghostObject->getOverlappingPairCache()->getOverlappingPairArray()[i];
		
		if (collisionPair->m_algorithm)
			collisionPair->m_algorithm->getAllContactManifolds(m_manifoldArray);

		
		for (int j=0;j<m_manifoldArray.size();j++)
		{
			btPersistentManifold* manifold = m_manifoldArray[j];

			btCollisionObject* obA = static_cast<btCollisionObject*>(manifold->getBody0());
			btCollisionObject* obB = static_cast<btCollisionObject*>(manifold->getBody1());

			btScalar directionSign;
			CEntityHandle<CBaseEntity> hOther;
			if (obA == m_ghostObject)
			{
				directionSign = btScalar(-1.0);
				hOther = CEntityHandle<CBaseEntity>((size_t)obB->getUserPointer());
			}
			else
			{
				directionSign = btScalar(1.0);
				hOther = CEntityHandle<CBaseEntity>((size_t)obA->getUserPointer());
			}

			for (int p=0;p<manifold->getNumContacts();p++)
			{
				const btManifoldPoint&pt = manifold->getContactPoint(p);

				if (obA == m_ghostObject)
				{
					if (!m_hEntity->ShouldCollideWith(hOther, Vector(pt.getPositionWorldOnB())))
						continue;
				}
				else
				{
					if (!m_hEntity->ShouldCollideWith(hOther, Vector(pt.getPositionWorldOnA())))
						continue;
				}

				btScalar dist = pt.getDistance();

				if (dist < 0.0)
				{
					if (dist < maxPen)
					{
						maxPen = dist;
						m_touchingNormal = pt.m_normalWorldOnB * directionSign;//??

					}
					m_currentPosition += pt.m_normalWorldOnB * directionSign * dist * btScalar(0.2);
					penetration = true;
				} else {
					//printf("touching %f\n", dist);
				}
			}
			
			//manifold->clearManifold();
		}
	}
	btTransform newTrans = m_ghostObject->getWorldTransform();
	newTrans.setOrigin(m_currentPosition);
	m_ghostObject->setWorldTransform(newTrans);
//	printf("m_touchingNormal = %f,%f,%f\n",m_touchingNormal[0],m_touchingNormal[1],m_touchingNormal[2]);
	return penetration;
}

void CCharacterController::stepUp ( btCollisionWorld* world)
{
	// phase 1: up
	btTransform start, end;
	m_targetPosition = m_currentPosition + getUpVector() * (m_stepHeight + (m_verticalOffset > 0.f?m_verticalOffset:0.f));

	start.setIdentity ();
	end.setIdentity ();

	/* FIXME: Handle penetration properly */
	start.setOrigin (m_currentPosition + getUpVector() * (m_convexShape->getMargin() + m_addedMargin));
	end.setOrigin (m_targetPosition);

	btKinematicClosestNotMeConvexResultCallback callback (this, m_ghostObject, -getUpVector(), btScalar(0.7071));
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;
	
	if (m_useGhostObjectSweepTest)
	{
		m_ghostObject->convexSweepTest (m_convexShape, start, end, callback, world->getDispatchInfo().m_allowedCcdPenetration);
	}
	else
	{
		world->convexSweepTest (m_convexShape, start, end, callback);
	}
	
	if (callback.hasHit())
	{
		// Only modify the position if the hit was a slope and not a wall or ceiling.
		if(callback.m_hitNormalWorld.dot(getUpVector()) > 0.0)
		{
			// we moved up only a fraction of the step height
			m_currentStepOffset = m_stepHeight * callback.m_closestHitFraction;
			m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);
		}
		m_verticalVelocity = 0.0;
		m_verticalOffset = 0.0;
	} else {
		m_currentStepOffset = m_stepHeight;
		m_currentPosition = m_targetPosition;
	}
}

void CCharacterController::updateTargetPositionBasedOnCollision (const btVector3& hitNormal, btScalar tangentMag, btScalar normalMag)
{
	btVector3 movementDirection = m_targetPosition - m_currentPosition;
	btScalar movementLength = movementDirection.length();
	if (movementLength>SIMD_EPSILON)
	{
		movementDirection.normalize();

		btVector3 reflectDir = computeReflectionDirection (movementDirection, hitNormal);
		reflectDir.normalize();

		btVector3 parallelDir, perpindicularDir;

		parallelDir = parallelComponent (reflectDir, hitNormal);
		perpindicularDir = perpindicularComponent (reflectDir, hitNormal);

		m_targetPosition = m_currentPosition;
		if (0)//tangentMag != 0.0)
		{
			btVector3 parComponent = parallelDir * btScalar (tangentMag*movementLength);
//			printf("parComponent=%f,%f,%f\n",parComponent[0],parComponent[1],parComponent[2]);
			m_targetPosition +=  parComponent;
		}

		if (normalMag != 0.0)
		{
			btVector3 perpComponent = perpindicularDir * btScalar (normalMag*movementLength);
//			printf("perpComponent=%f,%f,%f\n",perpComponent[0],perpComponent[1],perpComponent[2]);
			m_targetPosition += perpComponent;
		}
	} else
	{
//		printf("movementLength don't normalize a zero vector\n");
	}
}

void CCharacterController::stepForwardAndStrafe ( btCollisionWorld* collisionWorld, const btVector3& walkMove)
{
	// printf("m_normalizedDirection=%f,%f,%f\n",
	// 	m_normalizedDirection[0],m_normalizedDirection[1],m_normalizedDirection[2]);
	// phase 2: forward and strafe
	btTransform start, end;
	m_targetPosition = m_currentPosition + walkMove;

	start.setIdentity ();
	end.setIdentity ();
	
	btScalar fraction = 1.0;
	btScalar distance2 = (m_currentPosition-m_targetPosition).length2();
//	printf("distance2=%f\n",distance2);

	if (m_touchingContact)
	{
		if (m_normalizedDirection.dot(m_touchingNormal) > btScalar(0.0))
		{
			updateTargetPositionBasedOnCollision (m_touchingNormal);
		}
	}

	int maxIter = 10;

	while (fraction > btScalar(0.01) && maxIter-- > 0)
	{
		start.setOrigin (m_currentPosition);
		end.setOrigin (m_targetPosition);
		btVector3 sweepDirNegative(m_currentPosition - m_targetPosition);

		btKinematicClosestNotMeConvexResultCallback callback (this, m_ghostObject, sweepDirNegative, btScalar(0.0));
		callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
		callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;


		btScalar margin = m_convexShape->getMargin();
		m_convexShape->setMargin(margin + m_addedMargin);


		if (m_useGhostObjectSweepTest)
		{
			m_ghostObject->convexSweepTest (m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
		} else
		{
			collisionWorld->convexSweepTest (m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
		}
		
		m_convexShape->setMargin(margin);

		
		fraction -= callback.m_closestHitFraction;

		if (callback.hasHit())
		{	
			// we moved only a fraction
			btScalar hitDistance;
			hitDistance = (callback.m_hitPointWorld - m_currentPosition).length();

//			m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);

			updateTargetPositionBasedOnCollision (callback.m_hitNormalWorld);
			btVector3 currentDir = m_targetPosition - m_currentPosition;
			distance2 = currentDir.length2();
			if (distance2 > SIMD_EPSILON)
			{
				currentDir.normalize();
				/* See Quake2: "If velocity is against original velocity, stop ead to avoid tiny oscilations in sloping corners." */
				if (currentDir.dot(m_normalizedDirection) <= btScalar(0.0))
				{
					break;
				}
			} else
			{
//				printf("currentDir: don't normalize a zero vector\n");
				break;
			}

		} else {
			// we moved whole way
			m_currentPosition = m_targetPosition;
		}

	//	if (callback.m_closestHitFraction == 0.f)
	//		break;

	}
}

void CCharacterController::stepDown ( btCollisionWorld* collisionWorld, btScalar dt)
{
	btTransform start, end;

	// phase 3: down
	/*btScalar additionalDownStep = (m_wasOnGround && !onGround()) ? m_stepHeight : 0.0;
	btVector3 step_drop = getUpVector() * (m_currentStepOffset + additionalDownStep);
	btScalar downVelocity = (additionalDownStep == 0.0 && m_verticalVelocity<0.0?-m_verticalVelocity:0.0) * dt;
	btVector3 gravity_drop = getUpVector() * downVelocity; 
	m_targetPosition -= (step_drop + gravity_drop);*/

	btScalar downVelocity;
	if (m_vecUpVector.y() < 0)
		downVelocity = (m_verticalVelocity>0.f?m_verticalVelocity:0.f) * dt;
	else
		downVelocity = (m_verticalVelocity<0.f?-m_verticalVelocity:0.f) * dt;

	if(downVelocity > 0.0 && downVelocity < m_stepHeight
		&& (m_wasOnGround || !m_wasJumping))
	{
		downVelocity = m_stepHeight;
	}

	btVector3 step_drop = getUpVector() * (m_currentStepOffset + downVelocity);
	m_targetPosition -= step_drop;

	start.setIdentity ();
	end.setIdentity ();

	start.setOrigin (m_currentPosition);
	end.setOrigin (m_targetPosition);

	btKinematicClosestNotMeConvexResultCallback callback (this, m_ghostObject, getUpVector(), m_maxSlopeCosine);
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;
	
	if (m_useGhostObjectSweepTest)
	{
		m_ghostObject->convexSweepTest (m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
	} else
	{
		collisionWorld->convexSweepTest (m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
	}

	if (callback.hasHit())
	{
		// we dropped a fraction of the height -> hit floor
		m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);
		m_verticalVelocity = 0.0;
		m_verticalOffset = 0.0;
		m_wasJumping = false;
	} else {
		// we dropped the full height
		
		m_currentPosition = m_targetPosition;
	}
}



void CCharacterController::setWalkDirection
(
const btVector3& walkDirection
)
{
	m_useWalkDirection = true;
	m_walkDirection = walkDirection;
	m_normalizedDirection = getNormalizedVector(m_walkDirection);
}



void CCharacterController::setVelocityForTimeInterval
(
const btVector3& velocity,
btScalar timeInterval
)
{
//	printf("setVelocity!\n");
//	printf("  interval: %f\n", timeInterval);
//	printf("  velocity: (%f, %f, %f)\n",
//		 velocity.x(), velocity.y(), velocity.z());

	m_useWalkDirection = false;
	m_walkDirection = velocity;
	m_normalizedDirection = getNormalizedVector(m_walkDirection);
	m_velocityTimeInterval = timeInterval;
}



void CCharacterController::reset ()
{
}

void CCharacterController::warp (const btVector3& origin)
{
	btTransform xform;
	xform.setIdentity();
	xform.setOrigin (origin);
	m_ghostObject->setWorldTransform (xform);
}


void CCharacterController::preStep (  btCollisionWorld* collisionWorld)
{
	m_touchingContact = false;
	if (recoverFromPenetration (collisionWorld))
		m_touchingContact = true;

	m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
	m_targetPosition = m_currentPosition;
}

#include <stdio.h>

void CCharacterController::playerStep (  btCollisionWorld* collisionWorld, btScalar dt)
{
//	printf("playerStep(): ");
//	printf("  dt = %f", dt);

	// quick check...
	if (!m_useWalkDirection && m_velocityTimeInterval <= 0.0) {
//		printf("\n");
		return;		// no motion
	}

	m_wasOnGround = onGround();

	// Update fall velocity.
	m_verticalVelocity += m_gravity.y() * dt;

	if (m_vecUpVector.y() > 0)
	{
		if(m_verticalVelocity > 0.0 && m_verticalVelocity > m_jumpSpeed)
		{
			m_verticalVelocity = m_jumpSpeed;
		}
		if(m_verticalVelocity < 0.0 && btFabs(m_verticalVelocity) > btFabs(m_fallSpeed))
		{
			m_verticalVelocity = -btFabs(m_fallSpeed);
		}

		m_verticalOffset = m_verticalVelocity * dt;
	}
	else
	{
		if(m_verticalVelocity < 0.0 && m_verticalVelocity < -m_jumpSpeed)
		{
			m_verticalVelocity = -m_jumpSpeed;
		}
		if(m_verticalVelocity > 0.0 && btFabs(m_verticalVelocity) > btFabs(m_fallSpeed))
		{
			m_verticalVelocity = btFabs(m_fallSpeed);
		}

		m_verticalOffset = -m_verticalVelocity * dt;
	}


	btTransform xform;
	xform = m_ghostObject->getWorldTransform ();

//	printf("walkDirection(%f,%f,%f)\n",walkDirection[0],walkDirection[1],walkDirection[2]);
//	printf("walkSpeed=%f\n",walkSpeed);

	stepUp (collisionWorld);
	if (m_useWalkDirection) {
		stepForwardAndStrafe (collisionWorld, m_walkDirection);
	} else {
		//printf("  time: %f", m_velocityTimeInterval);
		// still have some time left for moving!
		btScalar dtMoving =
			(dt < m_velocityTimeInterval) ? dt : m_velocityTimeInterval;
		m_velocityTimeInterval -= dt;

		// how far will we move while we are moving?
		btVector3 move = m_walkDirection * dtMoving;

		//printf("  dtMoving: %f", dtMoving);

		// okay, step
		stepForwardAndStrafe(collisionWorld, move);
	}
	stepDown (collisionWorld, dt);

	// printf("\n");

	xform.setOrigin (m_currentPosition);
	m_ghostObject->setWorldTransform (xform);
}

void CCharacterController::setFallSpeed (btScalar fallSpeed)
{
	m_fallSpeed = fallSpeed;
}

void CCharacterController::setJumpSpeed (btScalar jumpSpeed)
{
	m_jumpSpeed = jumpSpeed;
}

void CCharacterController::setMaxJumpHeight (btScalar maxJumpHeight)
{
	m_maxJumpHeight = maxJumpHeight;
}

bool CCharacterController::canJump () const
{
	return onGround();
}

void CCharacterController::jump ()
{
	if (!canJump())
		return;

	if (m_vecUpVector.y() > 0)
		m_verticalVelocity = m_jumpSpeed;
	else
		m_verticalVelocity = -m_jumpSpeed;

	m_wasJumping = true;

#if 0
	currently no jumping.
	btTransform xform;
	m_rigidBody->getMotionState()->getWorldTransform (xform);
	btVector3 up = xform.getBasis()[1];
	up.normalize ();
	btScalar magnitude = (btScalar(1.0)/m_rigidBody->getInvMass()) * btScalar(8.0);
	m_rigidBody->applyCentralImpulse (up * magnitude);
#endif
}

void CCharacterController::setGravity(const btVector3& gravity)
{
	m_gravity = gravity;
}

btVector3 CCharacterController::getGravity() const
{
	return m_gravity;
}

void CCharacterController::setVerticalVelocity(btScalar flVerticalVelocity)
{
	m_verticalVelocity = flVerticalVelocity;
}

btVector3 CCharacterController::getVelocity() const
{
	return btVector3(m_walkDirection.x(), m_verticalVelocity, m_walkDirection.z());
}

void CCharacterController::setMaxSlope(btScalar slopeRadians)
{
	m_maxSlopeRadians = slopeRadians;
	m_maxSlopeCosine = btCos(slopeRadians);
}

btScalar CCharacterController::getMaxSlope() const
{
	return m_maxSlopeRadians;
}

bool CCharacterController::onGround () const
{
	return m_verticalVelocity == 0.0 && m_verticalOffset == 0.0;
}

void CCharacterController::debugDraw(btIDebugDraw* debugDrawer)
{
}

CBaseEntity* CCharacterController::GetEntity() const
{
	return m_hEntity;
}
