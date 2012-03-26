#include "basecontrol.h"

#include <tinker/shell.h>

#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>

#include "rootpanel.h"
#include "label.h"

using namespace glgui;

size_t CBaseControl::s_iQuad = ~0;

CBaseControl::CBaseControl(float x, float y, float w, float h)
{
	SetParent(NULL);
	SetBorder(BT_NONE);
	SetBackgroundColor(Color(0, 0, 0, 0));
	m_flX = x;
	m_flY = y;
	m_flW = w;
	m_flH = h;
	m_bVisible = true;
	SetAlpha(255);

	m_pfnCursorInCallback = NULL;
	m_pCursorInListener = NULL;
	m_pfnCursorOutCallback = NULL;
	m_pCursorOutListener = NULL;

	m_bFocus = false;

	m_flMouseInTime = 0;
}

CBaseControl::CBaseControl(const FRect& Rect)
{
	CBaseControl(Rect.x, Rect.y, Rect.w, Rect.h);
}

CBaseControl::~CBaseControl()
{
	if (HasFocus())
		CRootPanel::Get()->SetFocus(NULL);

	if (GetParent())
	{
		CPanel *pPanel = dynamic_cast<CPanel*>(GetParent());
		if (pPanel)
			pPanel->RemoveControl(this);
	}

	// Parent is IControl, which is virtual.
}

void CBaseControl::GetAbsPos(float &x, float &y)
{
	float px = 0;
	float py = 0;

	CPanel *pPanel = dynamic_cast<CPanel*>(GetParent());
	if (pPanel)
	{
		pPanel->GetAbsPos(px, py);

		if (pPanel->ShouldControlOffset(this))
		{
			px += pPanel->GetControlOffset().x;
			py += pPanel->GetControlOffset().y;
		}
	}

	x = m_flX + px;
	y = m_flY + py;
}

void CBaseControl::GetAbsDimensions(float &x, float &y, float &w, float &h)
{
	GetAbsPos(x, y);
	w = m_flW;
	h = m_flH;
}

FRect CBaseControl::GetAbsDimensions()
{
	float x, y;
	GetAbsPos(x, y);

	FRect r;
	r.x = x;
	r.y = y;
	r.w = m_flW;
	r.h = m_flH;

	return r;
}

void CBaseControl::SetWidth(float w)
{
	m_flW = w;
}

void CBaseControl::SetHeight(float h)
{
	m_flH = h;
}

void CBaseControl::SetLeft(float l)
{
	m_flX = l;
}

void CBaseControl::SetTop(float t)
{
	m_flY = t;
}

void CBaseControl::SetRight(float r)
{
	m_flW = r - m_flX;
}

void CBaseControl::SetBottom(float b)
{
	m_flH = b - m_flY;
}

void CBaseControl::Center()
{
	CenterX();
	CenterY();
}

void CBaseControl::CenterX()
{
	if (!GetParent())
		return;

	SetLeft(GetParent()->GetWidth()/2-GetWidth()/2);
}

void CBaseControl::CenterY()
{
	if (!GetParent())
		return;

	SetTop(GetParent()->GetHeight()/2-GetHeight()/2);
}

float CBaseControl::Layout_GetMargin(float flMargin)
{
	if (flMargin == g_flLayoutDefault)
	{
		TAssert(GetParent());
		if (!GetParent())
			return 0;

		return GetParent()->GetDefaultMargin();
	}

	return flMargin;
}

void CBaseControl::Layout_FullWidth(float flMargin)
{
	float flParentMargin = Layout_GetMargin(flMargin);

	TAssert(GetParent());
	if (!GetParent())
		return;

	SetLeft(flParentMargin);
	SetRight(GetParent()->GetWidth()-flParentMargin/2);
}

void CBaseControl::Layout_AlignTop(CBaseControl* pOther, float flMargin)
{
	float flParentMargin = Layout_GetMargin(flMargin);

	if (pOther)
		SetTop(pOther->GetBottom() + flParentMargin);
	else
		SetTop(flParentMargin);
}

void CBaseControl::Layout_AlignBottom(CBaseControl* pOther, float flMargin)
{
	float flParentMargin = Layout_GetMargin(flMargin);

	if (pOther)
		SetTop(pOther->GetTop() - GetHeight() - flParentMargin);
	else
	{
		TAssert(GetParent());
		if (!GetParent())
			return;

		SetTop(GetParent()->GetHeight() - GetHeight() - flParentMargin);
	}
}

void CBaseControl::Layout_Column(int iTotalColumns, int iColumn, float flMargin)
{
	float flParentMargin = Layout_GetMargin(flMargin);

	TAssert(GetParent());
	if (!GetParent())
		return;

	float flAllControlsWidth = GetParent()->GetWidth() - flParentMargin*(iTotalColumns+1);
	float flControlWidth = flAllControlsWidth/iTotalColumns;
	SetLeft(flParentMargin + (flControlWidth+flParentMargin)*iColumn);
	SetWidth(flControlWidth);
}

void CBaseControl::SetVisible(bool bVis)
{
	if (bVis && !m_bVisible)
		Layout();

	m_bVisible = bVis;
}

bool CBaseControl::IsVisible()
{
	if (GetParent())
	{
		if (!GetParent()->IsVisible())
			return false;
		if (!GetParent()->IsChildVisible(this))
			return false;
	}
	
	return m_bVisible;
}

bool CBaseControl::MousePressed(int iButton, int mx, int my)
{
	return CRootPanel::Get()->SetFocus(this);
}

void CBaseControl::Paint()
{
	float x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CBaseControl::Paint(float x, float y)
{
	Paint(x, y, m_flW, m_flH);
}

void CBaseControl::Paint(float x, float y, float w, float h)
{
	if (m_sTip.length() > 0 && m_flMouseInTime > 0 && CRootPanel::Get()->GetTime() > m_flMouseInTime + 0.5f)
	{
		int iFontSize = 12;

		float flFontHeight = CLabel::GetFontHeight("sans-serif", iFontSize);
		float flTextWidth = CLabel::GetTextWidth(m_sTip, m_sTip.length(), "sans-serif", iFontSize);

		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		float iTooltipRight = mx + flTextWidth;
		if (iTooltipRight > CRootPanel::Get()->GetWidth())
			mx -= (int)(iTooltipRight - CRootPanel::Get()->GetWidth());

		PaintRect((float)mx-3, (float)my-3, flTextWidth+6, flFontHeight+6, g_clrBox, 3); 
		CLabel::PaintText(m_sTip, m_sTip.length(), "sans-serif", iFontSize, (float)mx, (float)my);
	}
}

void CBaseControl::PaintBackground(float x, float y, float w, float h)
{
	if (m_eBorder == BT_NONE && m_clrBackground.a() == 0)
		return;

	PaintRect(x, y, w, h, m_clrBackground, (m_eBorder == BT_SOME)?5:0, true);
}

void CBaseControl::PaintRect(float x, float y, float w, float h, const Color& c, int iBorder, bool bHighlight)
{
	MakeQuad();

	::CRenderingContext r(nullptr, true);

	r.SetBlend(BLEND_ALPHA);
	r.SetUniform("iBorder", iBorder);
	r.SetUniform("bHighlight", bHighlight);
	r.SetUniform("vecColor", c);
	r.SetUniform("bDiffuse", false);
	r.SetUniform("bTexCoords", false);

	r.SetUniform("vecDimensions", Vector4D(x, y, w, h));

	r.BeginRenderVertexArray(s_iQuad);
	r.SetPositionBuffer(0u, 24);
	r.SetTexCoordBuffer(12, 24);
	r.SetCustomIntBuffer("iVertex", 1, 20, 24);
	r.EndRenderVertexArray(6);
}

void CBaseControl::PaintTexture(size_t iTexture, float x, float y, float w, float h, const Color& c)
{
	MakeQuad();

	::CRenderingContext r(nullptr, true);

	if ((w < 0) ^ (h < 0))
		r.SetBackCulling(false);

	r.SetBlend(BLEND_ALPHA);
	r.SetUniform("iBorder", 0);
	r.SetUniform("bHighlight", false);
	r.SetUniform("vecColor", c);
	r.SetUniform("bDiffuse", true);
	r.SetUniform("bTexCoords", false);

	r.SetUniform("vecDimensions", Vector4D(x, y, w, h));

	r.BindTexture(iTexture);

	r.BeginRenderVertexArray(s_iQuad);
	r.SetPositionBuffer(0u, 24);
	r.SetTexCoordBuffer(12, 24);
	r.SetCustomIntBuffer("iVertex", 1, 20, 24);
	r.EndRenderVertexArray(6);

	r.SetBackCulling(true);
}

void CBaseControl::PaintSheet(size_t iTexture, float x, float y, float w, float h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c)
{
	MakeQuad();

	::CRenderingContext r(nullptr, true);

	if ((w < 0) ^ (h < 0))
		r.SetBackCulling(false);

	r.SetBlend(BLEND_ALPHA);
	r.SetUniform("iBorder", 0);
	r.SetUniform("bHighlight", false);
	r.SetUniform("vecColor", c);
	r.SetUniform("bDiffuse", true);
	r.SetUniform("bTexCoords", true);

	r.SetUniform("vecDimensions", Vector4D(x, y, w, h));
	r.SetUniform("vecTexCoords", Vector4D((float)sx/(float)tw, (float)sy/(float)th, (float)sw/(float)tw, (float)sh/(float)th));

	r.BindTexture(iTexture);

	r.BeginRenderVertexArray(s_iQuad);
	r.SetPositionBuffer(0u, 24);
	r.SetTexCoordBuffer(12, 24);
	r.SetCustomIntBuffer("iVertex", 1, 20, 24);
	r.EndRenderVertexArray(6);

	r.SetBackCulling(true);
}

void CBaseControl::MakeQuad()
{
	if (s_iQuad != ~0)
		return;

	struct {
		Vector vecPosition;
		Vector2D vecTexCoord;
		int iIndex;
	} avecData[] =
	{
		{ Vector(0, 0, 0),		Vector2D(0, 1),		0 },
		{ Vector(0, 1, 0),		Vector2D(0, 0),		1 },
		{ Vector(1, 1, 0),		Vector2D(1, 0),		2 },
		{ Vector(0, 0, 0),		Vector2D(0, 1),		0 },
		{ Vector(1, 1, 0),		Vector2D(1, 0),		2 },
		{ Vector(1, 0, 0),		Vector2D(1, 1),		3 },
	};

	s_iQuad = CRenderer::LoadVertexDataIntoGL(sizeof(avecData), (float*)&avecData[0]);
}

bool CBaseControl::IsCursorListener()
{
	if (m_pfnCursorInCallback || m_pfnCursorOutCallback)
		return true;

	return false;
}

void CBaseControl::CursorIn()
{
	if (m_pfnCursorInCallback)
		m_pfnCursorInCallback(m_pCursorInListener, "");

	m_flMouseInTime = CRootPanel::Get()->GetTime();
}

void CBaseControl::CursorOut()
{
	if (m_pfnCursorOutCallback)
		m_pfnCursorOutCallback(m_pCursorOutListener, "");

	m_flMouseInTime = 0;
}

void CBaseControl::SetCursorInListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pCursorInListener = pListener;
	m_pfnCursorInCallback = pfnCallback;
}

void CBaseControl::SetCursorOutListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pCursorOutListener = pListener;
	m_pfnCursorOutCallback = pfnCallback;
}

void CBaseControl::SetTooltip(const tstring& sTip)
{
	if (sTip == m_sTip)
		return;

	m_sTip = sTip;

	if (m_flMouseInTime > 0)
		m_flMouseInTime = CRootPanel::Get()->GetTime();
}

IControl* CBaseControl::GetHasCursor()
{
	if (m_flMouseInTime > 0)
		return this;

	return NULL;
}
