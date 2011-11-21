#ifndef TINKER_PHYSICS_H
#define TINKER_PHYSICS_H

#include <matrix.h>

#include <models/models.h>

typedef enum collision_type_e
{
	CT_NONE = 0,
	CT_STATIC_MESH,	// Not animated, never moves. World geometry.
	CT_KINEMATIC,	// Not simulated by the engine, but collides with dynamic objects. Animated externally by game code.
	CT_CHARACTER,	// Kinematically animated character controller.
} collision_type_t;

class CPhysicsModel
{
public:
	virtual					~CPhysicsModel() {}

public:
	virtual void			AddEntity(class CBaseEntity* pEnt, collision_type_t eCollisionType) {};
	virtual void			RemoveEntity(class CBaseEntity* pEnt) {};

	virtual void			LoadCollisionMesh(const tstring& sModel, size_t iTris, int* aiTris, size_t iVerts, float* aflVerts) {};

	virtual void			Simulate() {};

	virtual void			DebugDraw() {};

	virtual void			SetEntityTransform(class CBaseEntity* pEnt, const Matrix4x4& mTransform) {};
	virtual void			SetEntityVelocity(class CBaseEntity* pEnt, const Vector& vecVelocity) {};
	virtual Vector			GetEntityVelocity(class CBaseEntity* pEnt) { return Vector(0, 0, 0); };
	virtual void			SetControllerWalkVelocity(class CBaseEntity* pEnt, const Vector& vecVelocity) {};
	virtual void			SetEntityGravity(class CBaseEntity* pEnt, const Vector& vecGravity) {};
	virtual void			SetEntityUpVector(class CBaseEntity* pEnt, const Vector& vecUp) {};

	virtual void			CharacterJump(class CBaseEntity* pEnt) {};
};

class CPhysicsManager
{
public:
							CPhysicsManager();
							~CPhysicsManager();

public:
	CPhysicsModel*			GetModel() { return m_pModel; }

protected:
	CPhysicsModel*			m_pModel;
};

CPhysicsModel* GamePhysics();

#endif