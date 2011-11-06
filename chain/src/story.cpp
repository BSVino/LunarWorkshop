#include "story.h"

#include <glgui/label.h>
#include <renderer/renderingcontext.h>
#include <renderer/renderer.h>

using namespace glgui;

REGISTER_ENTITY(CStory);

NETVAR_TABLE_BEGIN(CStory);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStory);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, glgui::CLabel*, m_pText);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStory);
INPUTS_TABLE_END();

CStory::CStory()
{
	m_pText = new CLabel();
	m_pText->Set3D(true);
	m_pText->SetText("MACHINE OF THE GODS");
	m_pText->SetFont("sans-serif", 96);
}

CStory::~CStory()
{
	delete m_pText;
}

Vector FindPointAtZ(const Frustum& oFrustum, int iF1, int iF2, float Z)
{
	const Plane& p1 = oFrustum.p[iF1];
	const Plane& p2 = oFrustum.p[iF2];

	float K = Z;

	// Goal is to find the corner of the frustum for a point at (x, y, K)
	// Equation of a plane: ax + by + cz + d = 0
	// Since we know z is -10 we can do
	//  ax + by + Kc + d = 0
	// We have two of these for each corner
	//  Ax + By + KC + D = 0
	// x y and z are the same since we're solving for that. Solve one of these for x
	//  x = (-D - By - KC)/A
	// Subtitute it for the other
	//  a(-D - By - KC)/A + by + Kc + d = 0
	// Solve for y
	//  -aD/A - Bay/A - KaC/A + by + Kc + d = 0
	//  -Bay/A + by = aD/A + KaC/A - Kc - d
	//  y(-Ba/A + b) = aD/A + KaC/A - Kc - d
	//  y = (aD/A + KaC/A - Kc - d) / (b - Ba/A)
	float y = ((p1.n.x * p2.d / p2.n.x) + (K * p1.n.x * p2.n.z / p2.n.x) - K * p1.n.z - p1.d) / (p1.n.y - p2.n.y * p1.n.x/p2.n.x);

	// Now use y to solve for x
	//  x = (-D - By - KC)/A
	float x = (-p2.d - p2.n.y * y - K * p2.n.z)/p2.n.x;

	return Vector(x, y, K);
}

void CStory::OnRender(CRenderingContext* pContext, bool bTransparent) const
{
	const Frustum& oFrustum = GameServer()->GetRenderer()->GetFrustum();

/*	Vector vecTopLeft = FindPointAtZ(oFrustum, FRUSTUM_UP, FRUSTUM_LEFT, -10);
	Vector vecTopRight = FindPointAtZ(oFrustum, FRUSTUM_UP, FRUSTUM_RIGHT, -10);
	Vector vecBottomLeft = FindPointAtZ(oFrustum, FRUSTUM_DOWN, FRUSTUM_LEFT, -10);
	Vector vecBottomRight = FindPointAtZ(oFrustum, FRUSTUM_DOWN, FRUSTUM_RIGHT, -10);

	pContext->UseProgram("text");
	pContext->BeginRenderQuads();
	pContext->Color(Vector(0.5f, 0.5f, 0.5f));
	pContext->Vertex(vecTopLeft/2);
	pContext->Vertex(vecBottomLeft/2);
	pContext->Vertex(vecBottomRight/2);
	pContext->Vertex(vecTopRight/2);
	pContext->EndRender();*/

	float flScale = 100;
	Vector vecTopRight = FindPointAtZ(oFrustum, FRUSTUM_UP, FRUSTUM_RIGHT, 0)*flScale;
	Vector vecBottomLeft = FindPointAtZ(oFrustum, FRUSTUM_DOWN, FRUSTUM_LEFT, 0)*flScale;

	Vector vecSize = vecTopRight - vecBottomLeft;

	m_pText->SetPos(vecBottomLeft.x, vecBottomLeft.y);
	m_pText->SetSize(vecSize.x, vecSize.y);

	pContext->Scale(1/flScale, 1/flScale, 1/flScale);

	pContext->UseProgram("");
	m_pText->SetFGColor(Color(255, 255, 255, 255));
	m_pText->Paint();
}
