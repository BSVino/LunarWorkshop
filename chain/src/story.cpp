#include "story.h"

#include <geometry.h>
#include <files.h>
#include <tinker_platform.h>

#include <glgui/label.h>
#include <renderer/renderingcontext.h>
#include <renderer/game_renderer.h>
#include <tinker/application.h>
#include <game/camera.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>

#include "chain_game.h"
#include "chain_renderer.h"

using namespace glgui;

REGISTER_ENTITY(CStory);

NETVAR_TABLE_BEGIN(CStory);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStory);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, glgui::CLabel*, m_pText);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flAlpha);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flAlphaGoal);
	SAVEDATA_OMIT(m_asPages);
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, tstring, m_sCurrentPage);
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, tstring, m_sNextPage);
	SAVEDATA_OMIT(m_aflHighlightedSections);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStory);
INPUTS_TABLE_END();

CStory::CStory()
{
	m_pText = new CLabel();
	m_pText->Set3D(true);
	m_pText->SetFont("sans-serif", 20);
	m_pText->SetLinkClickedListener(this, LinkClicked);
	m_pText->SetSectionHoverListener(this, SectionHovered);
	m_flAlpha = 0;
	m_flAlphaGoal = 1;
}

CStory::~CStory()
{
	delete m_pText;
}

void CStory::Load(const tstring& sFile)
{
	std::basic_ifstream<tchar> f(sFile.c_str());

	tstring sDirectory = GetDirectory(sFile);

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	TAssert(!f.bad());
	if (f.bad())
	{
		TError("Story index file " + sFile + " doesn't exist!\n");
		return;
	}

	CData* pPages = pData->FindChild("Pages");
	TAssert(pPages);
	if (!pPages)
	{
		TError("Story index file " + sFile + " has no pages!\n");
		return;
	}

	CData* pStartPage = pData->FindChild("StartPage");
	TAssert(pStartPage);
	if (!pStartPage)
	{
		TError("Story index file " + sFile + " has no start page!\n");
		return;
	}

	for (size_t i = 0; i < pPages->GetNumChildren(); i++)
	{
		CData* pPage = pPages->GetChild(i);

		std::basic_ifstream<tchar> f2((sDirectory + DIR_SEP + pPage->GetValueTString()).c_str());

		bool b2 = f2.bad();
		TAssert(!f2.bad());
		if (f2.bad())
		{
			TError("Couldn't find page file " + sDirectory + DIR_SEP + pPage->GetValueTString() + "\n");
			continue;
		}

		std::shared_ptr<CData> pPageData(new CData());
		CDataSerializer::Read(f2, pPageData.get());

		for (size_t j = 0; j < pPageData->GetNumChildren(); j++)
		{
			CData* pLine = pPageData->GetChild(j);

			if (pLine->GetKey() == "Line")
				m_asPages[pPage->GetKey()].m_sLines = m_asPages[pPage->GetKey()].m_sLines + str_replace(pLine->GetValueTString(), "\\n", "\n") + "\n";
			else if (pLine->GetKey() == "NextPage")
				m_asPages[pPage->GetKey()].m_sNextPage = pLine->GetValueTString();
			else if (pLine->GetKey() == "PrevPage")
				m_asPages[pPage->GetKey()].m_sPrevPage = pLine->GetValueTString();
		}
	}

	SetPage(pStartPage->GetValueTString());
}

void CStory::Precache()
{
	PrecacheTexture("textures/arrow.png");
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
		SetPage(m_sNextPage);
		m_sNextPage.clear();

		m_pText->SetText(m_asPages[m_sCurrentPage].m_sLines);
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

	// Reset all highlighted sections
	for (auto it = m_aflHighlightedSections.begin(); it != m_aflHighlightedSections.end(); it++)
	{
		it->second.m_flValue = Approach(it->second.m_flGoal, it->second.m_flValue, GameServer()->GetFrameTime()*5);
		it->second.m_flGoal = 0;
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

	for (auto it = m_aflHighlightedSections.begin(); it != m_aflHighlightedSections.end(); it++)
	{
		eastl::vector<tstring> asTokens;
		strtok(it->first, asTokens);

		TAssert(asTokens.size() == 2);
		if (asTokens.size() < 2)
			continue;

		int iLine = stoi(asTokens[0]);
		int iSection = stoi(asTokens[1]);

		const CLabel::CLine& oLine = m_pText->GetLine(iLine);
		const CLabel::CLineSection& oSection = m_pText->GetSection(iLine, iSection);

		CRenderingContext c(GameServer()->GetRenderer());
		c.UseFrameBuffer(&ChainRenderer()->GetMouseoverBuffer());
		c.UseProgram("");
		c.SetBlend(BLEND_ALPHA);

		m_pText->SetTextColor(Color(1.0f, 1.0f, 0.0f, m_flAlpha*it->second.m_flValue));

		m_pText->DrawSection(oLine, oSection, m_pText->GetLeft(), m_pText->GetTop(), m_pText->GetWidth(), m_pText->GetHeight());
	}

	m_pText->SetTextColor(Color(1.0f, 1.0f, 1.0f, m_flAlpha));
}

void CStory::MousePressed()
{
	// The text isn't part of the HUD so we need to manually call its MousePressed.
	m_pText->MousePressed(0, 0, 0);
}

const tstring& CStory::GetPageID(size_t i) const
{
	auto it = m_asPages.begin();
	for (size_t j = 0; it != m_asPages.end() && j != i; it++, j++);

	return it->first;
}

void CStory::SetPage(const tstring& sPage)
{
	m_sCurrentPage = sPage;

	TAssert(m_asPages.find(m_sCurrentPage) != m_asPages.end());
	if (m_asPages.find(m_sCurrentPage) == m_asPages.end())
	{
		TError("Page " + m_sCurrentPage + " doesn't exist!\n");
		return;
	}

	m_pText->SetText(m_asPages[m_sCurrentPage].m_sLines);

	m_aflHighlightedSections.clear();
	m_flAlpha = 0;
	m_flAlphaGoal = 1;
}

void CStory::GoToNextPage()
{
	m_sNextPage = m_asPages[m_sCurrentPage].m_sNextPage;
	m_flAlphaGoal = 0;
}

void CStory::GoToPrevPage()
{
	m_sNextPage = m_asPages[m_sCurrentPage].m_sPrevPage;
	m_flAlphaGoal = 0;
}

void CStory::LinkClickedCallback(const tstring& sLink)
{
	if (sLink.find("Page:") == 0)
	{
		m_sNextPage = trim(sLink.substr(5));
		m_flAlphaGoal = 0;
	}
}

void CStory::SectionHoveredCallback(const tstring& sSection)
{
	eastl::vector<tstring> asTokens;
	strtok(sSection, asTokens);

	TAssert(asTokens.size() == 2);
	if (asTokens.size() < 2)
		return;

	int iLine = stoi(asTokens[0]);
	int iSection = stoi(asTokens[1]);

	const CLabel::CLineSection& oSection = m_pText->GetSection(iLine, iSection);

	if (!oSection.m_sLink.length())
		return;

	m_aflHighlightedSections[sSection].m_flGoal = 1;
}
