#include "story.h"

#include <glgui/label.h>
#include <renderer/renderingcontext.h>
#include <renderer/renderer.h>

#include "chain_game.h"

using namespace glgui;

REGISTER_ENTITY(CStory);

NETVAR_TABLE_BEGIN(CStory);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStory);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, glgui::CLabel*, m_pText);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iPage);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flAlpha);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flAlphaGoal);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStory);
INPUTS_TABLE_END();

eastl::vector<tstring> g_asPages;

CStory::CStory()
{
	g_asPages.push_back("[size=96]MACHINE [size=56]OF THE[/size] GODS[/size]\n\n\n\n\n\n[size=42]George T. Downing[/size]");
	g_asPages.push_back("[size=56]CHAPTER 1[/size]\n\n\n\n\n\n[size=42]The Awakening[/size]");
	g_asPages.push_back("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec et justo nunc, ut feugiat lorem.\n\nVestibulum a sem id neque condimentum posuere. Pellentesque semper vulputate lorem, id volutpat lectus bibendum id. Vivamus et enim lacus. Nam eros nisi, commodo in vulputate vel, interdum vel neque. Maecenas congue consequat felis, et ullamcorper turpis venenatis vitae. Morbi in nunc eu odio scelerisque condimentum at et elit. Integer interdum facilisis sollicitudin. Donec tempor libero vel mi vulputate vehicula. Quisque mattis nulla vel magna interdum ultrices. Nam nunc tellus, pharetra ac iaculis nec, ornare sagittis diam.");
	g_asPages.push_back("In eleifend, enim sed fermentum sagittis, nibh est sagittis nisl, aliquam tempor turpis felis a urna. In orci elit, dapibus et malesuada non, sodales ut nisl. Donec lorem purus, aliquet vitae bibendum id, rhoncus eget risus. Curabitur volutpat pharetra lectus, ac mattis risus elementum at. Suspendisse potenti. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Suspendisse ac nisi enim. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Vivamus pharetra, ipsum eu convallis aliquam, ante turpis laoreet odio, non faucibus nisi ipsum vitae lorem.\n\nNulla eget nibh eget libero blandit pulvinar at at arcu. Fusce massa nisi, ultrices id lobortis ac, mattis ac risus. Fusce non velit neque. Donec consectetur gravida odio sit amet fringilla. Duis quis sagittis ligula.");

	m_pText = new CLabel();
	m_pText->Set3D(true);
	m_pText->SetFont("sans-serif", 20);
	m_pText->SetText(g_asPages[0]);
	m_iPage = 0;
	m_flAlpha = 0;
	m_flAlphaGoal = 1;
}

CStory::~CStory()
{
	delete m_pText;
}

void CStory::Spawn()
{
	ChainGame()->SetStory(this);
}

void CStory::Think()
{
	// A bit of blackness before we start. Also helps prevent the initial frame skip.
	if (GameServer()->GetGameTime() < 1)
	{
		m_flAlpha = 0;
		return;
	}

	float flSpeed = 0.2f;
	if (m_flAlphaGoal < m_flAlpha)
		flSpeed = 1;

	m_flAlpha = Approach(m_flAlphaGoal, m_flAlpha, GameServer()->GetFrameTime()*flSpeed);

	if (m_flAlphaGoal == 0 && m_flAlpha == 0)
	{
		m_iPage = (m_iPage+1)%g_asPages.size();
		m_pText->SetText(g_asPages[m_iPage]);
		m_flAlphaGoal = 1;
	}
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
	Vector vecTopRight = FindPointAtZ(oFrustum, FRUSTUM_UP, FRUSTUM_RIGHT, 0)*flScale*4/5;
	Vector vecBottomLeft = FindPointAtZ(oFrustum, FRUSTUM_DOWN, FRUSTUM_LEFT, 0)*flScale*4/5;

	Vector vecSize = vecTopRight - vecBottomLeft;

	m_pText->SetPos(vecBottomLeft.x, vecBottomLeft.y);
	m_pText->SetSize(vecSize.x, vecSize.y);

	pContext->Scale(1/flScale, 1/flScale, 1/flScale);

	pContext->UseProgram("");
	m_pText->SetAlpha(m_flAlpha);
	m_pText->Paint();
}

void CStory::NextPage()
{
	m_flAlphaGoal = 0;
}
