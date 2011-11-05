#include "basecontrol.h"

#include <GL/glew.h>

#include "rootpanel.h"
#include "label.h"

using namespace glgui;

CBaseControl::CBaseControl(int x, int y, int w, int h)
{
	SetParent(NULL);
	m_iX = x;
	m_iY = y;
	m_iW = w;
	m_iH = h;
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
	CBaseControl((int)Rect.x, (int)Rect.y, (int)Rect.w, (int)Rect.h);
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

void CBaseControl::GetAbsPos(int &x, int &y)
{
	int px = 0;
	int py = 0;
	if (GetParent())
		GetParent()->GetAbsPos(px, py);
	x = m_iX + px;
	y = m_iY + py;
}

void CBaseControl::GetAbsDimensions(int &x, int &y, int &w, int &h)
{
	GetAbsPos(x, y);
	w = m_iW;
	h = m_iH;
}

FRect CBaseControl::GetAbsDimensions()
{
	int x, y;
	GetAbsPos(x, y);

	FRect r;
	r.x = (float)x;
	r.y = (float)y;
	r.w = (float)m_iW;
	r.h = (float)m_iH;

	return r;
}

void CBaseControl::SetRight(int r)
{
	m_iW = r - m_iX;
}

void CBaseControl::SetBottom(int b)
{
	m_iH = b - m_iY;
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
	int x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CBaseControl::Paint(int x, int y)
{
	Paint(x, y, m_iW, m_iH);
}

void CBaseControl::Paint(int x, int y, int w, int h)
{
	if (m_sTip.length() > 0 && m_flMouseInTime > 0 && CRootPanel::Get()->GetTime() > m_flMouseInTime + 0.5f)
	{
		int iFontSize = 12;

		float flFontHeight = CLabel::GetFontHeight(_T("sans-serif"), iFontSize);
		float flTextWidth = CLabel::GetTextWidth(m_sTip, m_sTip.length(), _T("sans-serif"), iFontSize);

		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		int iTooltipRight = (int)(mx + flTextWidth);
		if (iTooltipRight > CRootPanel::Get()->GetWidth())
			mx -= (iTooltipRight - CRootPanel::Get()->GetWidth());

		PaintRect(mx-3, (int)(my-flFontHeight)+1, (int)flTextWidth+6, (int)flFontHeight+6); 
		glColor4ubv(Color(255, 255, 255, 255));
		CLabel::PaintText(m_sTip, m_sTip.length(), _T("sans-serif"), iFontSize, (float)mx, (float)my);
	}
}

void CBaseControl::PaintRect(int x, int y, int w, int h, const Color& c)
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

	if (w > 1)
	{
		glBegin(GL_QUADS);
			glNormal3f(-0.707106781f, 0.707106781f, 0);	// Give 'em normals so that the light falls on them cool-like.
			glVertex2d(x, y);
			glNormal3f(-0.707106781f, -0.707106781f, 0);
			glVertex2d(x, y+h);
			glNormal3f(0.707106781f, -0.707106781f, 0);
			glVertex2d(x+w-1, y+h);
			glNormal3f(0.707106781f, 0.707106781f, 0);
			glVertex2d(x+w-1, y);
		glEnd();
	}
	else
	{
		glBegin(GL_LINES);
			glNormal3f(0, 1.0, 0);
			glVertex2d(x, y);
			glNormal3f(0, -1.0, 0);
			glVertex2d(x, y+h);
		glEnd();
	}

	if (h > 1 && w > 1)
	{
		glBegin(GL_LINES);
			glNormal3f(-0.707106781f, 0.707106781f, 0);
			glVertex2d(x, y+1);
			glNormal3f(-0.707106781f, -0.707106781f, 0);
			glVertex2d(x, y+h-1);

			glNormal3f(0.707106781f, 0.707106781f, 0);
			glVertex2d(x+w, y+1);
			glNormal3f(0.707106781f, -0.707106781f, 0);
			glVertex2d(x+w, y+h-1);
		glEnd();
	}

	glPopAttrib();
}

void CBaseControl::PaintTexture(size_t iTexture, int x, int y, int w, int h, const Color& c)
{
	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	glColor4ubv(c);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2d(x, y);
		glTexCoord2f(0, 0);
		glVertex2d(x, y+h);
		glTexCoord2f(1, 0);
		glVertex2d(x+w, y+h);
		glTexCoord2f(1, 1);
		glVertex2d(x+w, y);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopAttrib();
}

void CBaseControl::PaintSheet(size_t iTexture, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c)
{
	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	glColor4ubv(c);
	glBegin(GL_QUADS);
		glTexCoord2f((float)sx/tw, 1-(float)sy/th);
		glVertex2d(x, y);
		glTexCoord2f((float)sx/tw, 1-((float)sy+sh)/th);
		glVertex2d(x, y+h);
		glTexCoord2f(((float)sx+sw)/tw, 1-((float)sy+sh)/th);
		glVertex2d(x+w, y+h);
		glTexCoord2f(((float)sx+sw)/tw, 1-(float)sy/th);
		glVertex2d(x+w, y);
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
		m_pfnCursorInCallback(m_pCursorInListener);

	m_flMouseInTime = CRootPanel::Get()->GetTime();
}

void CBaseControl::CursorOut()
{
	if (m_pfnCursorOutCallback)
		m_pfnCursorOutCallback(m_pCursorOutListener);

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
