#include "physics.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <game/gameserver.h>
#include <game/entities/character.h>
#include <models/models.h>
#include <tinker/profiler.h>

#include "physics_debugdraw.h"
#include "character_controller.h"

class CBulletPhysics : public CPhysicsModel
{
protected:
	class CMotionState : public btMotionState
	{
	public:
		virtual void getWorldTransform(btTransform& mCenterOfMass) const 
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

		virtual void setWorldTransform(const btTransform& mCenterOfMass)
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

	public:
		CBulletPhysics*					m_pPhysics;
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
		};

		~CPhysicsEntity()
		{
		};

	public:
		btRigidBody*						m_pRigidBody;
		btPairCachingGhostObject*			m_pGhostObject;
		CCharacterController*				m_pCharacterController;
		CMotionState						m_oMotionState;
		bool								m_bCenterMassOffset;
	};

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

	virtual void			CharacterJump(class CBaseEntity* pEnt);

	virtual CPhysicsEntity*	GetPhysicsEntity(class CBaseEntity* pEnt);

protected:
	eastl::vector<CPhysicsEntity>		m_aEntityList;

	btDefaultCollisionConfiguration*	m_pCollisionConfiguration;
	btCollisionDispatcher*				m_pDispatcher;
	btDbvtBroadphase*					m_pBroadphase;
	btGhostPairCallback*				m_pGhostPairCallback;
	btDiscreteDynamicsWorld*			m_pDynamicsWorld;

	class CCollisionMesh
	{
	public:
		btTriangleIndexVertexArray*		m_pIndexVertexArray;
		btCollisionShape*				m_pCollisionShape;
		eastl::vector<eastl::vector<int> >		m_aiIndices;
		eastl::vector<eastl::vector<Vector> >	m_avecVertices;
	};

	eastl::map<size_t, CCollisionMesh>	m_apCollisionMeshes;
	eastl::map<tstring, btCollisionShape*>	m_apCharacterShapes;

	CPhysicsDebugDrawer*				m_pDebugDrawer;
};

CBulletPhysics::CBulletPhysics()
{
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

		float flStepHeight = 0.2f;
		CCharacter* pCharacter = dynamic_cast<CCharacter*>(pEntity);
		if (pCharacter)
			flStepHeight = pCharacter->GetMaxStepHeight();

		pPhysicsEntity->m_pCharacterController = new CCharacterController(pPhysicsEntity->m_pGhostObject, pCapsuleShape, flStepHeight);

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

void CBulletPhysics::LoadCollisionMesh(const tstring& sModel, const eastl::vector< eastl::vector<Vertex_t> >& aTriangles)
{
	size_t iModel = CModelLibrary::Get()->FindModel(sModel);

	TAssert(iModel != ~0);

	eastl::vector<eastl::vector<int> >& aiIndices = m_apCollisionMeshes[iModel].m_aiIndices;
	eastl::vector<eastl::vector<Vector> >& avecVertices = m_apCollisionMeshes[iModel].m_avecVertices;

	for (size_t i = 0; i < aTriangles.size(); i++)
	{
		aiIndices.push_back();
		avecVertices.push_back();

		if (!aTriangles[i].size())
			continue;

		aiIndices.set_capacity(aTriangles[i].size());
		avecVertices.set_capacity(aTriangles[i].size());

		for (size_t j = 0; j < aTriangles[i].size(); j += 3)
		{
			aiIndices[i].push_back(j);
			aiIndices[i].push_back(j+1);
			aiIndices[i].push_back(j+2);

			avecVertices[i].push_back(aTriangles[i][j].vecPosition);
			avecVertices[i].push_back(aTriangles[i][j+1].vecPosition);
			avecVertices[i].push_back(aTriangles[i][j+2].vecPosition);
		}
	}

	m_apCollisionMeshes[iModel].m_pIndexVertexArray = new btTriangleIndexVertexArray();
	for (size_t i = 0; i < aiIndices.size(); i++)
	{
		if (!aiIndices[i].size())
			continue;

		btIndexedMesh m;
		m.m_numTriangles = aTriangles[i].size()/3;
		m.m_triangleIndexBase = (const unsigned char *)&aiIndices[i][0];
		m.m_triangleIndexStride = sizeof(int)*3;
		m.m_numVertices = avecVertices[i].size();
		m.m_vertexBase = (const unsigned char *)&avecVertices[i][0].x;
		m.m_vertexStride = sizeof(Vector);
		m_apCollisionMeshes[iModel].m_pIndexVertexArray->addIndexedMesh(m, PHY_INTEGER);
	}

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
		pPhysicsEntity->m_pCharacterController->setWalkDirection(v * (1.0f/60));	// 1/60 being bullet's fixed time step
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

CBulletPhysics::CPhysicsEntity* CBulletPhysics::GetPhysicsEntity(class CBaseEntity* pEnt)
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

CPhysicsModel* GamePhysics()
{
	return GameServer()->GetPhysics()->GetModel();
}
