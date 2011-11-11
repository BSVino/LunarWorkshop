#ifndef TINKER_PHYSICS_DEBUG_DRAWER_H
#define TINKER_PHYSICS_DEBUG_DRAWER_H

#include "LinearMath/btIDebugDraw.h"

class CPhysicsDebugDrawer : public btIDebugDraw
{
public:
					CPhysicsDebugDrawer();

public:
	virtual void	SetDrawing(bool bOn) { m_bDrawing = bOn; }

	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);

	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

	virtual void	drawSphere (const btVector3& p, btScalar radius, const btVector3& color);
	virtual void	drawBox (const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha);

	virtual void	drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha);
	
	virtual void	drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

	virtual void	reportErrorWarning(const char* warningString);

	virtual void	draw3dText(const btVector3& location, const char* textString);

	virtual void	setDebugMode(int debugMode) {};

	virtual int		getDebugMode() const { return btIDebugDraw::DBG_DrawWireframe; }

protected:
	bool			m_bDrawing;
};

#endif//GL_DEBUG_DRAWER_H
