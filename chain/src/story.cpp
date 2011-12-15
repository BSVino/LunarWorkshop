#include "story.h"

#include <geometry.h>

#include <glgui/label.h>
#include <renderer/renderingcontext.h>
#include <renderer/renderer.h>
#include <tinker/application.h>
#include <game/camera.h>

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
	if (!g_asPages.size())
	{
		g_asPages.push_back("[size=96]MACHINE [size=56]OF THE[/size] GODS[/size]\n\n\n\n\n\n[size=42]George T. Downing[/size]");
		g_asPages.push_back("[size=56]CHAPTER 1[/size]\n\n\n\n\n\n[size=42]The Awakening[/size]");
		g_asPages.push_back("The hills here rolled across the land like waves far out at sea, gentle slopes covered in grass which reached Chain's thigh.  Wind buffeted his back, adding to the impression of motion on the land. In the twilight, Chain could make out the details of the small shrine which sat above the hill no better than he could seeing it from afar in the afternoon.  Light flickered within it, the yellow of a flame.  Even fire was different, since the Leaving.  If touched it would still burn. It still held back the dark.  But fire no longer consumed its host.  Chain wondered from time to time if this was fire's true nature.  Perhaps the destruction it caused in the past was out of the flame's will to survive, and now that it no longer needed food fire was content to stay as it was.\n \n[link=0]Click here to continue.[/link]");
		g_asPages.push_back("Regardless of reason, this flame had lit the shrine for years in wait of someone in need. Chain crested the hill, glancing over the stonework of the small sanctuary.  It was covered in what Chain referred to as Symbol of Three.  With a corner for the Body, a corner for the Spirit,  a corner for the World around, and What Gods May Be in the center of it all.  A single candle burned on an alter overflowing with the wax remains of it's predecessors, and flowers were on the floor at it's base.  Scratched into the walls were prayers of the faithful, pleas to God or fate or the universe jaggedly etched into the smooth lines of the stone walls.  So much was asked in the writings, but Chain did not find a single line of thanks.  He often wondered at this when he inspected holy sites.  Did these people forget all they had in their lives to be thankful for?  Or did they only offer devotion when their need was so great they could not even see the wealth they had?  Chain sat against the wall, resting his burdens on the floor. His sister had been the same.\n \n[link=0]Click here to continue.[/link]");
		g_asPages.push_back("Sleep came and it went, unimportant as ever these days. Chain did not grow tired, but occasionally he grew weary. Weary, and unsure. Rising, he stretched unfulfilled and ate unsatisfied.  The candle winked at him out of the corner of his eye.  Chain rummaged through his belongings and produced a glass jar he had filled with an assortment of little gemstones.  Breaking the candle free of its wax mooring, he jammed it into the jar. Gems fell twinkling to the floor, rattling in the small alcove. Once the candle was nicely anchored, he secured the jar to his belt with a bit of leather lanyard.  He ran his hand over a couple of the prayers on the wall, wondering what these people had wanted so badly.  He had come to understand that there was no such thing as need.  Desire, however strong, did not make something necessary or inevitable.  These people's desires to live.  His own, to stop.\n\n[link=0]Click here to continue.[/link]");
	}

	m_pText = new CLabel();
	m_pText->Set3D(true);
	m_pText->SetFont("sans-serif", 20);
	m_pText->SetText(g_asPages[0]);
	m_pText->SetLinkClickedListener(this, LinkClicked);
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

	int mx, my;
	Application()->GetMousePosition(mx, my);
	Vector vecWorld = GameServer()->GetRenderer()->WorldPosition(Vector((float)mx, (float)my, 0));

	Vector vecCamera = GameServer()->GetCamera()->GetCameraPosition();
	Vector v1, v2, v3;
	Vector vecSize;
	m_pText->GetSize(vecSize.x, vecSize.y);
	m_pText->GetPos(v1.x, v1.y);
	m_pText->GetPos(v2.x, v2.y);
	m_pText->GetPos(v3.x, v3.y);
	v2.x += vecSize.x;
	v3.y += vecSize.y;

	Vector vecHit;
	RayIntersectsPlane(Ray(vecCamera, (vecWorld - vecCamera).Normalized()), v1, v2, v3, &vecHit);

	m_pText->Set3DMousePosition(vecHit*LabelScale());
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
	if (!bTransparent)
		return;

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

	float flScale = LabelScale();
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

void CStory::MousePressed()
{
	if (m_iPage >= 2)
	{
		// The text isn't part of the HUD so we need to manually call its MousePressed.
		m_pText->MousePressed(0, 0, 0);
		return;
	}

	m_flAlphaGoal = 0;
}

void CStory::LinkClickedCallback(const tstring& sLink)
{
	size_t iPage = stoi(sLink);

	// For now just always go to the next page.
	m_flAlphaGoal = 0;
}
