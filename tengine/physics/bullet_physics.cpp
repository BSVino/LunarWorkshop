#include "bullet_physics.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <game/gameserver.h>
#include <game/entities/character.h>
#include <models/models.h>
#include <tinker/profiler.h>

#include "physics_debugdraw.h"

CBulletPhysics::CBulletPhysics()
{
	// Allocate all memory up front to avoid reallocations
	m_aEntityList.set_capacity(GameServer()->GetMaxEntities());

	m_pCollisionConfiguration = new btDefaultCollisionConfiguration();
	m_pDispatcher = new	btCollisionDispatcher(m_pCollisionConfiguration);
	m_pBroadphase = new btDbvtBroadphase();
	m_pBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(m_pGhostPairCallback = new btGhostPairCallback());

	m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pDispatcher, m_pBroadphase, NULL, m_pCollisionConfiguration);
	m_pDynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

	m_pDebugDrawer = NULL;
}

CBulletPhysics::~CBulletPhysics()
{
	delete m_pDynamicsWorld;
	for (eastl::map<size_t, CCollisionMesh>::iterator it = m_apCollisionMeshes.begin(); it != m_apCollisionMeshes.end(); it++)
	{
		delete it->second.m_pCollisionShape;
		delete it->second.m_pIndexVertexArray;
	}
	delete m_pBroadphase;
	delete m_pGhostPairCallback;
	delete m_pDispatcher;
	delete m_pCollisionConfiguration;

	if (m_pDebugDrawer)
		delete m_pDebugDrawer;
}

void CBulletPhysics::AddEntity(CBaseEntity* pEntity, collision_type_t eCollisionType)
{
	TAssert(eCollisionType != CT_NONE);
	if (eCollisionType == CT_NONE)
		return;

	TAssert(pEntity);
	if (!pEntity)
		return;

	size_t iHandle = pEntity->GetHandle();
	if (m_aEntityList.size() <= iHandle)
		m_aEntityList.resize(iHandle+1);

	CPhysicsEntity* pPhysicsEntity = &m_aEntityList[iHandle];
	pPhysicsEntity->m_oMotionState.m_hEntity = pEntity;
	pPhysicsEntity->m_oMotionState.m_pPhysics = this;

	if (eCollisionType == CT_CHARACTER)
	{
		pPhysicsEntity->m_bCenterMassOffset = true;

		AABB r = pEntity->GetBoundingBox();
		float flRadiusX = r.m_vecMaxs.x - r.Center().x;
		float flRadiusZ = r.m_vecMaxs.z - r.Center().z;
		float flRadius = flRadiusX;
		if (flRadiusZ > flRadiusX)
			flRadius = flRadiusZ;
		float flHeight = r.m_vecMaxs.y - r.m_vecMins.y;

		tstring sIdentifier;
		if (pEntity->GetModel() != ~0)
			sIdentifier = CModelLibrary::Get()->GetModel(pEntity->GetModel())->m_sFilename;
		else
			sIdentifier = pEntity->GetClassName();

		auto it = m_apCharacterShapes.find(sIdentifier);
		if (it == m_apCharacterShapes.end())
		{
			TAssert(flHeight > flRadius);	// Couldn't very well make a capsule this way could we?

			m_apCharacterShapes[sIdentifier] = new btCapsuleShape(flRadius, flHeight - flRadius);
		}

		btCapsuleShape* pCapsuleShape = dynamic_cast<btCapsuleShape*>(m_apCharacterShapes[sIdentifier]);

#ifdef _DEBUG
		TAssert(pCapsuleShape);
		if (pCapsuleShape)
		{
			// Varying character sizes not yet supported!
			TAssert(pCapsuleShape->getRadius() == flRadius);
			TAssert(fabs(pCapsuleShape->getHalfHeight()*2 - flRadius) - flHeight < 0.001f);
		}
#endif

		btTransform mTransform;
		mTransform.setIdentity();
		mTransform.setFromOpenGLMatrix(&pEntity->GetGlobalTransform().m[0][0]);

		pPhysicsEntity->m_pGhostObject = new btPairCachingGhostObject();
		pPhysicsEntity->m_pGhostObject->setWorldTransform(mTransform);
		pPhysicsEntity->m_pGhostObject->setCollisionShape(pCapsuleShape);
		pPhysicsEntity->m_pGhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
		pPhysicsEntity->m_pGhostObject->setUserPointer((void*)iHandle);

		float flStepHeight = 0.2f;
		CCharacter* pCharacter = dynamic_cast<CCharacter*>(pEntity);
		if (pCharacter)
			flStepHeight = pCharacter->GetMaxStepHeight();

		pPhysicsEntity->m_pCharacterController = new CCharacterController(pEntity, pPhysicsEntity->m_pGhostObject, pCapsuleShape, flStepHeight);

		m_pDynamicsWorld->addCollisionObject(pPhysicsEntity->m_pGhostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
		m_pDynamicsWorld->addAction(pPhysicsEntity->m_pCharacterController);
	}
	else
	{
		btCollisionShape* pCollisionShape;
		float flMass;

		if (eCollisionType == CT_STATIC_MESH)
		{
			pPhysicsEntity->m_bCenterMassOffset = false;

			flMass = 0;
			pCollisionShape = m_apCollisionMeshes[pEntity->GetModel()].m_pCollisionShape;
		}
		else if (eCollisionType == CT_KINEMATIC)
		{
			pPhysicsEntity->m_bCenterMassOffset = false;

			flMass = 0;
			pCollisionShape = m_apCollisionMeshes[pEntity->GetModel()].m_pCollisionShape;
		}
		else
		{
			TAssert(!"Unimplemented collision type");
		}

		btTransform mTransform;
		mTransform.setIdentity();
		mTransform.setFromOpenGLMatrix(&pEntity->GetGlobalTransform().m[0][0]);

		bool bDynamic = (flMass != 0.f);

		btVector3 vecLocalInertia(0, 0, 0);
		if (bDynamic)
			pCollisionShape->calculateLocalInertia(flMass, vecLocalInertia);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(flMass, &pPhysicsEntity->m_oMotionState, pCollisionShape, vecLocalInertia);
		pPhysicsEntity->m_pRigidBody = new btRigidBody(rbInfo);
		pPhysicsEntity->m_pRigidBody->setUserPointer((void*)iHandle);

		if (eCollisionType == CT_KINEMATIC)
		{
			pPhysicsEntity->m_pRigidBody->setCollisionFlags(pPhysicsEntity->m_pRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			pPhysicsEntity->m_pRigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		m_pDynamicsWorld->addRigidBody(pPhysicsEntity->m_pRigidBody);
	}
}

void CBulletPhysics::RemoveEntity(CBaseEntity* pEntity)
{
	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEntity);
	if (!pPhysicsEntity)
		return;

	if (pPhysicsEntity->m_pRigidBody)
		m_pDynamicsWorld->removeRigidBody(pPhysicsEntity->m_pRigidBody);
	delete pPhysicsEntity->m_pRigidBody;
	pPhysicsEntity->m_pRigidBody = NULL;

	if (pPhysicsEntity->m_pGhostObject)
		m_pDynamicsWorld->removeCollisionObject(pPhysicsEntity->m_pGhostObject);
	delete pPhysicsEntity->m_pGhostObject;
	pPhysicsEntity->m_pGhostObject = NULL;

	if (pPhysicsEntity->m_pCharacterController)
		m_pDynamicsWorld->removeAction(pPhysicsEntity->m_pCharacterController);
	delete pPhysicsEntity->m_pCharacterController;
	pPhysicsEntity->m_pCharacterController = NULL;
}

void CBulletPhysics::LoadCollisionMesh(const tstring& sModel, size_t iTris, int* aiTris, size_t iVerts, float* aflVerts)
{
	size_t iModel = CModelLibrary::Get()->FindModel(sModel);

	TAssert(iModel != ~0);

	m_apCollisionMeshes[iModel].m_pIndexVertexArray = new btTriangleIndexVertexArray();

	btIndexedMesh m;
	m.m_numTriangles = iTris;
	m.m_triangleIndexBase = (const unsigned char *)aiTris;
	m.m_triangleIndexStride = sizeof(int)*3;
	m.m_numVertices = iVerts;
	m.m_vertexBase = (const unsigned char *)aflVerts;
	m.m_vertexStride = sizeof(Vector);
	m_apCollisionMeshes[iModel].m_pIndexVertexArray->addIndexedMesh(m, PHY_INTEGER);

	m_apCollisionMeshes[iModel].m_pCollisionShape = new btBvhTriangleMeshShape(m_apCollisionMeshes[iModel].m_pIndexVertexArray, true);
}

void CBulletPhysics::Simulate()
{
	TPROF("CBulletPhysics::Simulate");

	m_pDynamicsWorld->stepSimulation(GameServer()->GetFrameTime(), 10);

	// Non-rigid bodies don't use motion states and so we must manually update.
	for (size_t i = 0; i < m_aEntityList.size(); i++)
	{
		CPhysicsEntity* pObj = &m_aEntityList[i];
		if (pObj->m_pGhostObject)
			pObj->m_oMotionState.setWorldTransform(pObj->m_pGhostObject->getWorldTransform());
	}
}

void CBulletPhysics::DebugDraw()
{
	if (!m_pDebugDrawer)
	{
		m_pDebugDrawer = new CPhysicsDebugDrawer();
		m_pDynamicsWorld->setDebugDrawer(m_pDebugDrawer);
	}

	m_pDebugDrawer->SetDrawing(true);
	m_pDynamicsWorld->debugDrawWorld();
	m_pDebugDrawer->SetDrawing(false);
}

void CBulletPhysics::SetEntityTransform(class CBaseEntity* pEnt, const Matrix4x4& mTransform)
{
	TPROF("CBulletPhysics::SetEntityTransform");

	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return;

	if (pPhysicsEntity->m_bCenterMassOffset)
	{
		Matrix4x4 mCenter;
		mCenter.SetTranslation(pEnt->GetBoundingBox().Center());

		btTransform m;
		m.setFromOpenGLMatrix(mCenter * mTransform);

		if (pPhysicsEntity->m_pRigidBody)
			pPhysicsEntity->m_pRigidBody->setCenterOfMassTransform(m);
		else if (pPhysicsEntity->m_pGhostObject)
			pPhysicsEntity->m_pGhostObject->setWorldTransform(m);
	}
	else
	{
		btTransform m;
		m.setFromOpenGLMatrix(mTransform);

		if (pPhysicsEntity->m_pRigidBody)
			pPhysicsEntity->m_pRigidBody->setCenterOfMassTransform(m);
		else if (pPhysicsEntity->m_pGhostObject)
			pPhysicsEntity->m_pGhostObject->setWorldTransform(m);
	}
}

void CBulletPhysics::SetEntityVelocity(class CBaseEntity* pEnt, const Vector& vecVelocity)
{
	TPROF("CBulletPhysics::SetEntityVelocity");

	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return;

	btVector3 v(vecVelocity.x, vecVelocity.y, vecVelocity.z);

	if (pPhysicsEntity->m_pRigidBody)
		pPhysicsEntity->m_pRigidBody->setLinearVelocity(v);
	else if (pPhysicsEntity->m_pCharacterController)
	{
		pPhysicsEntity->m_pCharacterController->setWalkDirection(v * (1.0f/60));	// 1/60 being bullet's fixed time step
		pPhysicsEntity->m_pCharacterController->setVerticalVelocity(v.y());
	}
}

Vector CBulletPhysics::GetEntityVelocity(class CBaseEntity* pEnt)
{
	TPROF("CBulletPhysics::GetEntityVelocity");

	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return Vector();

	if (pPhysicsEntity->m_pRigidBody)
		return Vector(pPhysicsEntity->m_pRigidBody->getLinearVelocity());
	else if (pPhysicsEntity->m_pCharacterController)
		return Vector(pPhysicsEntity->m_pCharacterController->getVelocity());

	return Vector();
}

void CBulletPhysics::SetControllerWalkVelocity(class CBaseEntity* pEnt, const Vector& vecVelocity)
{
	TPROF("CBulletPhysics::SetEntityVelocity");

	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return;

	btVector3 v(vecVelocity.x, vecVelocity.y, vecVelocity.z);

	TAssert(pPhysicsEntity->m_pCharacterController);
	if (pPhysicsEntity->m_pCharacterController)
		pPhysicsEntity->m_pCharacterController->setWalkDirection(v * (1.0f/60));	// 1/60 being bullet's fixed time step
}

void CBulletPhysics::SetEntityGravity(class CBaseEntity* pEnt, const Vector& vecGravity)
{
	TPROF("CBulletPhysics::SetEntityGravity");

	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return;

	btVector3 v(vecGravity.x, vecGravity.y, vecGravity.z);

	if (pPhysicsEntity->m_pRigidBody)
		pPhysicsEntity->m_pRigidBody->setGravity(v);
	else if (pPhysicsEntity->m_pCharacterController)
		pPhysicsEntity->m_pCharacterController->setGravity(v);
}

void CBulletPhysics::SetEntityUpVector(class CBaseEntity* pEnt, const Vector& vecUp)
{
	TPROF("CBulletPhysics::SetEntityUpVector");

	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return;

	btVector3 v(vecUp.x, vecUp.y, vecUp.z);

	TAssert(pPhysicsEntity->m_pCharacterController);
	if (pPhysicsEntity->m_pCharacterController)
		pPhysicsEntity->m_pCharacterController->setUpVector(v);
}

void CBulletPhysics::CharacterJump(class CBaseEntity* pEnt)
{
	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEnt);
	if (!pPhysicsEntity)
		return;

	TAssert(pPhysicsEntity->m_pCharacterController);
	if (!pPhysicsEntity->m_pCharacterController)
		return;

	pPhysicsEntity->m_pCharacterController->jump();
}

CPhysicsEntity* CBulletPhysics::GetPhysicsEntity(class CBaseEntity* pEnt)
{
	TPROF("CBulletPhysics::GetPhysicsEntity");

	TAssert(pEnt);
	if (!pEnt)
		return NULL;

	size_t iHandle = pEnt->GetHandle();
	TAssert(m_aEntityList.size() > iHandle);
	if (m_aEntityList.size() <= iHandle)
		return NULL;

	CPhysicsEntity* pPhysicsEntity = &m_aEntityList[iHandle];
	TAssert(pPhysicsEntity);

	return pPhysicsEntity;
}

CPhysicsManager::CPhysicsManager()
{
	m_pModel = new CBulletPhysics();
}

CPhysicsManager::~CPhysicsManager()
{
	delete m_pModel;
}

void CMotionState::getWorldTransform(btTransform& mCenterOfMass) const
{
	TPROF("CMotionState::getWorldTransform");

	if (m_pPhysics->GetPhysicsEntity(m_hEntity)->m_bCenterMassOffset)
	{
		Matrix4x4 mCenter;
		mCenter.SetTranslation(m_hEntity->GetBoundingBox().Center());

		mCenterOfMass.setFromOpenGLMatrix(mCenter * m_hEntity->GetGlobalTransform());
	}
	else
		mCenterOfMass.setFromOpenGLMatrix(m_hEntity->GetGlobalTransform());
}

void CMotionState::setWorldTransform(const btTransform& mCenterOfMass)
{
	TPROF("CMotionState::setWorldTransform");

	Matrix4x4 mGlobal;
	mCenterOfMass.getOpenGLMatrix(mGlobal);

	if (m_pPhysics->GetPhysicsEntity(m_hEntity)->m_bCenterMassOffset)
	{
		Matrix4x4 mCenter;
		mCenter.SetTranslation(m_hEntity->GetBoundingBox().Center());

		m_hEntity->SetGlobalTransform(mCenter.InvertedRT() * mGlobal);
	}
	else
		m_hEntity->SetGlobalTransform(mGlobal);
}

CPhysicsModel* GamePhysics()
{
	return GameServer()->GetPhysics()->GetModel();
}
