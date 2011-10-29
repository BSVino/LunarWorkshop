#include "physics_debugdraw.h"

#include <GL/glew.h>

#include <renderer/renderingcontext.h>
#include <game/gameserver.h>
#include <tinker/application.h>

CPhysicsDebugDrawer::CPhysicsDebugDrawer()
{
}

void CPhysicsDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& fromColor, const btVector3& toColor)
{
	CRenderingContext c(GameServer()->GetRenderer());
	c.UseProgram("model");
	c.SetUniform("bDiffuse", false);
	c.BeginRenderDebugLines();
		c.Color(Color(Vector((const float*)fromColor)));
		c.Vertex(Vector((const float*)from));
		c.Color(Color(Vector((const float*)fromColor)));
		c.Vertex(Vector((const float*)to));
	c.EndRender();
}

void CPhysicsDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
	drawLine(from,to,color,color);
}

void CPhysicsDebugDrawer::drawSphere (const btVector3& p, btScalar radius, const btVector3& color)
{
	CRenderingContext c(GameServer()->GetRenderer());
	c.UseProgram("model");
	c.SetUniform("bDiffuse", false);
	c.Translate(Vector((const float*)p));
	c.Scale(radius, radius, radius);
	c.SetColor(Color(Vector((const float*)color)));
	c.RenderSphere();
}

void CPhysicsDebugDrawer::drawBox (const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha)
{
	TAssert(!"Unimplemented");
}

void CPhysicsDebugDrawer::drawTriangle(const btVector3& a,const btVector3& b,const btVector3& c,const btVector3& color,btScalar alpha)
{
	CRenderingContext r(GameServer()->GetRenderer());
	r.UseProgram("model");
	r.SetUniform("bDiffuse", false);
	r.SetColor(Color(Vector((const float*)color)));
	if (alpha < 1)
		r.SetBlend(BLEND_ALPHA);
	r.SetAlpha(alpha);

	const btVector3	n=btCross(b-a,c-a).normalized();

	r.BeginRenderTris();
	r.Normal(Vector((const float*)n));
	r.Vertex(Vector((const float*)a));
	r.Vertex(Vector((const float*)b));
	r.Vertex(Vector((const float*)c));
	r.EndRender();
}

void CPhysicsDebugDrawer::draw3dText(const btVector3& location,const char* textString)
{
	TAssert(!"Unimplemented");
}

void CPhysicsDebugDrawer::reportErrorWarning(const char* warningString)
{
	TMsg(sprintf(tstring("CPhysicsDebugDrawer: %s\n"), warningString));
}

void CPhysicsDebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
	btVector3 to=pointOnB+normalOnB*1;//distance;
	const btVector3& from = pointOnB;

	CRenderingContext r(GameServer()->GetRenderer());
	r.UseProgram("model");
	r.SetUniform("bDiffuse", false);
	r.SetColor(Color(Vector((const float*)color)));

	r.BeginRenderDebugLines();
	r.Vertex(Vector((const float*)from));
	r.Vertex(Vector((const float*)to));
	r.EndRender();
}
