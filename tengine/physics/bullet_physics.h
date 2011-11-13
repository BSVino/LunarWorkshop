#ifndef TINKER_BULLET_PHYSICS_H
#define TINKER_BULLET_PHYSICS_H

#include "physics.h"

#include <btBulletDynamicsCommon.h>

#include <game/entityhandle.h>

#include "character_controller.h"

class CMotionState : public btMotionState
{
public:
	CMotionState()
	{
		m_pPhysics = NULL;
	}

public:
	virtual void getWorldTransform(btTransform& mCenterOfMass) const;

	virtual void setWorldTransform(const btTransform& mCenterOfMass);

public:
	class CBulletPhysics*			m_pPhysics;
	CEntityHandle<CBaseEntity>		m_hEntity;
};

class CPhysicsEntity
{
public:
	CPhysicsEntity()
	{
		m_pRigidBody = NULL;
		m_pGhostObject = NULL;
		m_pCharacterController = NULL;
		m_bCenterMassOffset = true;
	};

	~CPhysicsEntity()
	{
	};

public:
	btRigidBody*						m_pRigidBody;
	class btPairCachingGhostObject*		m_pGhostObject;
	CCharacterController*				m_pCharacterController;
	CMotionState						m_oMotionState;
	bool								m_bCenterMassOffset;
};

class CBulletPhysics : public CPhysicsModel
{
public:
							CBulletPhysics();
							~CBulletPhysics();

public:
	virtual void			AddEntity(class CBaseEntity* pEnt, collision_type_t eCollisionType);
	virtual void			RemoveEntity(class CBaseEntity* pEnt);

	virtual void			LoadCollisionMesh(const tstring& sModel, const eastl::vector< eastl::vector<Vertex_t> >& aTriangles);

	virtual void			Simulate();

	virtual void			DebugDraw();

	virtual void			SetEntityTransform(class CBaseEntity* pEnt, const Matrix4x4& mTransform);
	virtual void			SetEntityVelocity(class CBaseEntity* pEnt, const Vector& vecVelocity);
	virtual Vector			GetEntityVelocity(class CBaseEntity* pEnt);
	virtual void			SetControllerWalkVelocity(class CBaseEntity* pEnt, const Vector& vecVelocity);
	virtual void			SetEntityGravity(class CBaseEntity* pEnt, const Vector& vecGravity);
	virtual void			SetEntityUpVector(class CBaseEntity* pEnt, const Vector& vecUp);

	virtual void			CharacterJump(class CBaseEntity* pEnt);

	virtual CPhysicsEntity*	GetPhysicsEntity(class CBaseEntity* pEnt);

protected:
	eastl::vector<CPhysicsEntity>			m_aEntityList;

	btDefaultCollisionConfiguration*		m_pCollisionConfiguration;
	btCollisionDispatcher*					m_pDispatcher;
	btDbvtBroadphase*						m_pBroadphase;
	class btGhostPairCallback*				m_pGhostPairCallback;
	btDiscreteDynamicsWorld*				m_pDynamicsWorld;

	class CCollisionMesh
	{
	public:
		btTriangleIndexVertexArray*				m_pIndexVertexArray;
		btCollisionShape*						m_pCollisionShape;
		eastl::vector<eastl::vector<int> >		m_aiIndices;
		eastl::vector<eastl::vector<Vector> >	m_avecVertices;
	};

	eastl::map<size_t, CCollisionMesh>		m_apCollisionMeshes;
	eastl::map<tstring, btCollisionShape*>	m_apCharacterShapes;

	class CPhysicsDebugDrawer*				m_pDebugDrawer;
};

#endif
