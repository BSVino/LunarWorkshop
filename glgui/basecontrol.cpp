#include "basecontrol.h"

#include <GL/glew.h>

#include "rootpanel.h"
#include "label.h"

using namespace glgui;

CBaseControl::CBaseControl(float x, float y, float w, float h)
{
	SetParent(NULL);
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
	m_flY = l;
}

void CBaseControl::SetTop(float t)
{
	m_flX = t;
}

void CBaseControl::SetRight(float r)
{
	m_flW = r - m_flX;
}

void CBaseControl::SetBottom(float b)
{
	m_flH = b - m_flY;
}

void CBaseControl::SetVisible(bool bVis)
{
	if (bVis && !m_bVisible)
		Layout();

	m_bVisible = bVis;
}

bool CBaseControl::IsVisible()
{
	if (GetParent() && !GetParent()->IsVisible())
		return false;
	
	return m_bVisible;
}

bool CBaseControl::MousePressed(int iButton, int mx, int my)
{
	CRootPanel::Get()->SetFocus(this);
	return false;
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

		PaintRect((float)mx-3, my-flFontHeight+1, flTextWidth+6, flFontHeight+6); 
		glColor4ubv(Color(255, 255, 255, 255));
		CLabel::PaintText(m_sTip, m_sTip.length(), "sans-serif", iFontSize, (float)mx, (float)my);
	}
}

void CBaseControl::PaintRect(float x, float y, float w, float h, const Color& c)
{
	glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ubv(c);

	glMaterialfv(GL_FRONT, GL_AMBIENT, Vector(0.0f, 0.0f, 0.0f));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Vector(1.0f, 1.0f, 1.0f));
	glMaterialfv(GL_FRONT, GL_SPECULAR, Vector(0.2f, 0.2f, 0.3f));
	glMaterialfv(GL_FRONT, GL_EMISSION, Vector(0.0f, 0.0f, 0.0f));
	glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);

	glLineWidth(1);

	glBegin(GL_QUADS);
		glNormal3f(-0.707106781f, 0.707106781f, 0);	// Give 'em normals so that the light falls on them cool-like.
		glVertex2f(x, y);
		glNormal3f(-0.707106781f, -0.707106781f, 0);
		glVertex2f(x, y+h);
		glNormal3f(0.707106781f, -0.707106781f, 0);
		glVertex2f(x+w, y+h);
		glNormal3f(0.707106781f, 0.707106781f, 0);
		glVertex2f(x+w, y);
	glEnd();

	glPopAttrib();
}

void CBaseControl::PaintTexture(size_t iTexture, float x, float y, float w, float h, const Color& c)
{
	glPushAttrib(GL_ENABLE_BIT);

	if ((w < 0) ^ (h < 0))
		glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	glColor4ubv(c);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(x, y);
		glTexCoord2f(0, 0);
		glVertex2f(x, y+h);
		glTexCoord2f(1, 0);
		glVertex2f(x+w, y+h);
		glTexCoord2f(1, 1);
		glVertex2f(x+w, y);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopAttrib();
}

void CBaseControl::PaintSheet(size_t iTexture, float x, float y, float w, float h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c)
{
	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	glColor4ubv(c);
	glBegin(GL_QUADS);
		glTexCoord2f((float)sx/tw, 1-(float)sy/th);
		glVertex2f(x, y);
		glTexCoord2f((float)sx/tw, 1-((float)sy+sh)/th);
		glVertex2f(x, y+h);
		glTexCoord2f(((float)sx+sw)/tw, 1-((float)sy+sh)/th);
		glVertex2f(x+w, y+h);
		glTexCoord2f(((float)sx+sw)/tw, 1-(float)sy/th);
		glVertex2f(x+w, y);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopAttrib();
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
