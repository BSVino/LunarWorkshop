#include "modelgui.h"

#include <assert.h>
#include <algorithm>
#include <GL/freeglut.h>
#include <maths.h>
#include <vector.h>

using namespace modelgui;

Color modelgui::g_clrPanel = Color(66, 72, 82, 255);
Color modelgui::g_clrBox = Color(34, 37, 42, 255);
Color modelgui::g_clrBoxHi = Color(148, 161, 181, 255);

CBaseControl::CBaseControl(int x, int y, int w, int h)
{
	SetParent(NULL);
	m_iX = x;
	m_iY = y;
	m_iW = w;
	m_iH = h;
	m_bVisible = true;
}

CBaseControl::CBaseControl(const CRect& Rect)
{
	CBaseControl((int)Rect.x, (int)Rect.y, (int)Rect.w, (int)Rect.h);
}

void CBaseControl::Destructor()
{
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

bool CBaseControl::IsVisible()
{
	if (GetParent() && !GetParent()->IsVisible())
		return false;
	
	return m_bVisible;
}

void CBaseControl::PaintRect(int x, int y, int w, int h, Color& c)
{
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
			glNormal3f(-0.707106781f, -0.707106781f, 0);	// Give 'em normals so that the light falls on them cool-like.
			glVertex2d(x, y);
			glNormal3f(0.707106781f, -0.707106781f, 0);
			glVertex2d(x+w-1, y);
			glNormal3f(0.707106781f, 0.707106781f, 0);
			glVertex2d(x+w-1, y+h);
			glNormal3f(-0.707106781f, 0.707106781f, 0);
			glVertex2d(x, y+h);
		glEnd();
	}
	else
	{
		glBegin(GL_LINES);
			glNormal3f(1.0f, 0, 0);
			glVertex2d(x, y);
			glNormal3f(-1.0f, 0, 0);
			glVertex2d(x, y+h);
		glEnd();
	}

	if (h > 1 && w > 1)
	{
		glBegin(GL_LINES);
			glNormal3f(-0.707106781f, -0.707106781f, 0);
			glVertex2d(x, y+1);
			glNormal3f(-0.707106781f, 0.707106781f, 0);
			glVertex2d(x, y+h-1);

			glNormal3f(0.707106781f, -0.707106781f, 0);
			glVertex2d(x+w, y+1);
			glNormal3f(0.707106781f, 0.707106781f, 0);
			glVertex2d(x+w, y+h-1);
		glEnd();
	}

	glDisable(GL_BLEND);
}

CPanel::CPanel(int x, int y, int w, int h)
	: CBaseControl(x, y, w, h)
{
	SetBorder(BT_SOME);
	m_pHasCursor = NULL;
	m_bHighlight = false;
	m_bDestructing = false;
}

void CPanel::Destructor()
{
	// Protect m_apControls from accesses elsewhere.
	m_bDestructing = true;

	size_t iCount = m_apControls.size();
	size_t i;
	for (i = 0; i < iCount; i++)
	{
		// Christ.
		IControl* pControl = m_apControls[i];
		pControl->Destructor();
		pControl->Delete();
	}
	m_apControls.clear();

	m_bDestructing = false;

	CBaseControl::Destructor();
}

bool CPanel::KeyPressed(int code)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->KeyPressed(code))
			return true;
	}
	return false;
}

bool CPanel::KeyReleased(int code)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->KeyReleased(code))
			return true;
	}
	return false;
}

bool CPanel::MousePressed(int code, int mx, int my)
{
	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		int x = 0, y = 0, w = 0, h = 0;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (pControl->MousePressed(code, mx, my))
				return true;
		}
	}
	return false;
}

bool CPanel::MouseReleased(int code, int mx, int my)
{
	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		int x, y, w, h;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (pControl->MouseReleased(code, mx, my))
				return true;
		}
	}
	return false;
}

void CPanel::CursorMoved(int mx, int my)
{
	bool bFoundControlWithCursor = false;

	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible() || !pControl->IsCursorListener())
			continue;

		int x, y, w, h;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (m_pHasCursor != pControl)
			{
				if (m_pHasCursor)
				{
					m_pHasCursor->CursorOut();
				}
				m_pHasCursor = pControl;
				m_pHasCursor->CursorIn();
			}

			pControl->CursorMoved(mx, my);

			bFoundControlWithCursor = true;
			break;
		}
	}

	if (!bFoundControlWithCursor && m_pHasCursor)
	{
		m_pHasCursor->CursorOut();
		m_pHasCursor = NULL;
	}
}

void CPanel::CursorOut()
{
	if (m_pHasCursor)
	{
		m_pHasCursor->CursorOut();
		m_pHasCursor = NULL;
	}
}

void CPanel::AddControl(IControl* pControl, bool bToTail)
{
	if (!pControl)
		return;

#ifdef _DEBUG
	for (size_t i = 0; i < m_apControls.size(); i++)
		assert(m_apControls[i] != pControl);	// You're adding a control to the panel twice! Quit it!
#endif

	pControl->SetParent(this);

	if (bToTail)
		m_apControls.push_back(pControl);
	else
		m_apControls.push_back(pControl);
}

void CPanel::RemoveControl(IControl* pControl)
{
	// If we are destructing then this RemoveControl is being called from this CPanel's
	// destructor's m_apControls[i]->Destructor() so we should not delete this element
	// because it will be m_apControls.Purge()'d later.
	if (!m_bDestructing)
	{
		for (size_t i = 0; i < m_apControls.size(); i++)
		{
			if (m_apControls[i] == pControl)
				m_apControls.erase(std::remove(m_apControls.begin(), m_apControls.end(), pControl), m_apControls.end());
		}
	}

	if (m_pHasCursor == pControl)
		m_pHasCursor = NULL;
}

void CPanel::Layout( void )
{
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		m_apControls[i]->Layout();
	}
}

void CPanel::Paint()
{
	int x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CPanel::Paint(int x, int y)
{
	Paint(x, y, m_iW, m_iH);
}

void CPanel::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	if (m_eBorder == BT_SOME)
		PaintBorder(x, y, w, h);

	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		if (!m_apControls[i]->IsVisible())
			continue;

		// Translate this location to the child's local space.
		int cx, cy, ax, ay;
		m_apControls[i]->GetAbsPos(cx, cy);
		GetAbsPos(ax, ay);
		m_apControls[i]->Paint(cx+x-ax, cy+y-ay);
	}
}

void CPanel::PaintBorder(int x, int y, int w, int h)
{
}

void CPanel::Think()
{
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		m_apControls[i]->Think();
	}
}

CDroppablePanel::CDroppablePanel(int x, int y, int w, int h)
	: CPanel(x, y, w, h)
{
	m_bGrabbable = true;

	CRootPanel::Get()->AddDroppable(this);
};

void CDroppablePanel::Destructor()
{
	if (m_apDraggables.size())
	{
		for (size_t i = 0; i < m_apDraggables.size(); i++)
		{
			m_apDraggables[i]->Destructor();
			m_apDraggables[i]->Delete();
		}
	}

	if (CRootPanel::Get())
		CRootPanel::Get()->RemoveDroppable(this);

	CPanel::Destructor();
}

void CDroppablePanel::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	for (size_t i = 0; i < m_apDraggables.size(); i++)
	{
		// Translate this location to the child's local space.
		int ax, ay;
		CRect c = m_apDraggables[i]->GetHoldingRect();
		GetAbsPos(ax, ay);
		m_apDraggables[i]->Paint((int)c.x+x-ax, (int)c.y+y-ay);
	}

	CPanel::Paint(x, y, w, h);
}

void CDroppablePanel::SetSize(int w, int h)
{
	CPanel::SetSize(w, h);
	for (size_t i = 0; i < m_apDraggables.size(); i++)
		m_apDraggables[i]->SetHoldingRect(GetHoldingRect());
}

void CDroppablePanel::SetPos(int x, int y)
{
	CPanel::SetPos(x, y);
	for (size_t i = 0; i < m_apDraggables.size(); i++)
		m_apDraggables[i]->SetHoldingRect(GetHoldingRect());
}

bool CDroppablePanel::MousePressed(int code, int mx, int my)
{
	if (!IsVisible())
		return false;

	if (m_bGrabbable && m_apDraggables.size() > 0)
	{
		CRect r = GetHoldingRect();
		if (code == 0 &&
			mx >= r.x &&
			my >= r.y &&
			mx < r.x + r.w &&
			my < r.y + r.h)
		{
			CRootPanel::Get()->DragonDrop(this);
			return true;
		}
	}

	return CPanel::MousePressed(code, mx, my);
}

void CDroppablePanel::SetDraggable(IDraggable* pDragged, bool bDelete)
{
	ClearDraggables(bDelete);

	AddDraggable(pDragged);
}

void CDroppablePanel::AddDraggable(IDraggable* pDragged)
{
	if (pDragged)
	{
		m_apDraggables.push_back(pDragged);
		pDragged->SetHoldingRect(GetHoldingRect());
		pDragged->SetDroppable(this);
	}
}

void CDroppablePanel::ClearDraggables(bool bDelete)
{
	if (bDelete)
	{
		for (size_t i = 0; i < m_apDraggables.size(); i++)
		{
			m_apDraggables[i]->Destructor();
			m_apDraggables[i]->Delete();
		}
	}

	m_apDraggables.clear();
}

IDraggable* CDroppablePanel::GetDraggable(int i)
{
	return m_apDraggables[i];
}

CLabel::CLabel(int x, int y, int w, int h, const char* pszText)
	: CBaseControl(x, y, w, h)
{
	m_bEnabled = true;
	m_bWrap = true;
	m_pszText = NULL;
	m_iTotalLines = 0;
	m_eAlign = TA_MIDDLECENTER;
	m_FGColor = Color(255, 255, 255, 255);

	static char szFont[1024];
	sprintf(szFont, "%s\\Fonts\\Arial.ttf", getenv("windir"));

	m_pFont = new FTGLPixmapFont(szFont);
	m_pFont->FaceSize(13);

	SetText(pszText);
}

void CLabel::Destructor()
{
	if (m_pszText)
		free(m_pszText);

	delete m_pFont;

	CBaseControl::Destructor();
}

void CLabel::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	glDisable(GL_LIGHTING);

	Color FGColor = m_FGColor;
	if (!m_bEnabled)
		FGColor.SetColor(m_FGColor.r()/2, m_FGColor.g()/2, m_FGColor.b()/2, m_iAlpha);

	wchar_t* pszSeps = L"\n";
	wchar_t* pszText = _wcsdup(m_pszText);
	wchar_t* pszTok = wcstok(pszText, pszSeps);
	m_iLine = 0;

	while (pszTok)
	{
		glColor4ubv(FGColor);

		float tw = m_pFont->Advance(pszTok);
		float t = m_pFont->LineHeight();

		if (!m_bWrap || tw < w || w == 0 || (m_iLine+1)*t > h)
		{
			DrawLine(pszTok, (unsigned int)wcslen(pszTok), x, y, w, h);

			m_iLine++;
		}
		else
		{
			tw = 0;
			unsigned int iSource = 0;
			int iLastSpace = 0, iLastBreak = 0, iLength = 0;
			while (iSource < wcslen(pszTok))
			{
				wchar_t szChar[2];
				szChar[0] = pszTok[iSource];
				szChar[1] = L'\0';
				float cw = m_pFont->Advance(szChar);
				if (tw + cw < w || (tw == 0 && w < cw) || (m_iLine+1)*t > h)
				{
					iLength++;
					if (pszTok[iSource] == L' ')
						iLastSpace = iSource;
					tw += cw;
				}
				else
				{
					int iBackup = iSource - iLastSpace;
					if (iLastSpace == iLastBreak)
						iBackup = 0;

					iSource -= iBackup;
					iLength -= iBackup;

					DrawLine(pszTok + iLastBreak, iLength, x, y, w, h);

					iLength = 0;
					tw = 0;
					while (iSource < wcslen(pszTok) && pszTok[iSource] == L' ')
						iSource++;
					iLastBreak = iLastSpace = iSource--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
					m_iLine++;
				}

				iSource++;
			}

			DrawLine(pszTok + iLastBreak, iLength, x, y, w, h);
			m_iLine++;
		}

		pszTok = wcstok(NULL, pszSeps);
	}

	free(pszText);

	glEnable(GL_LIGHTING);
}

void CLabel::DrawLine(wchar_t* pszText, unsigned iLength, int x, int y, int w, int h)
{
	float lw = m_pFont->Advance(pszText);
	float t = m_pFont->LineHeight();
	float th = GetTextHeight() - t;

	float flBaseline = (float)m_pFont->FaceSize()/2 + m_pFont->Descender()/2;

	if (m_eAlign == TA_MIDDLECENTER)
		glRasterPos2f((float)x + (float)w/2 - lw/2, (float)y + (float)h/2 - flBaseline + th/2 - m_iLine*t);
	else if (m_eAlign == TA_LEFTCENTER)
		glRasterPos2f((float)x, (float)y + (float)h/2 - flBaseline);
	else if (m_eAlign == TA_RIGHTCENTER)
		glRasterPos2f((float)x + (float)w - lw, y + h/2 - flBaseline + th/2 - m_iLine*t);
	else	// TA_TOPLEFT
		glRasterPos2f((float)x, (float)y + (float)h - lw);

	m_pFont->Render(pszText);
}

void CLabel::SetSize(int w, int h)
{
	CBaseControl::SetSize(w, h);
	ComputeLines();
}

void CLabel::SetText(const wchar_t* pszText)
{
	if (m_pszText)
		free(m_pszText);
	m_pszText = NULL;

	if (!pszText)
		m_pszText = _wcsdup(L"");
	else
		m_pszText = _wcsdup(pszText);

	ComputeLines();
}

void CLabel::SetText(const char* pszText)
{
	if (m_pszText)
		free(m_pszText);
	m_pszText = NULL;

	if (!pszText)
		SetText(L"");
	else
	{
		size_t iSize = (strlen(pszText) + 1) * sizeof(wchar_t);
		wchar_t* pszBuf = (wchar_t*)malloc(iSize);

		mbstowcs(pszBuf, pszText, strlen(pszText)+1);

		SetText(pszBuf);
		free(pszBuf);
	}
}

void CLabel::AppendText(const wchar_t* pszText)
{
	if (!pszText)
		return;

	const wchar_t* pszCurr = GetText();

	size_t iLength = wcslen(pszText) + wcslen(pszCurr) + 1;

	size_t iSize = iLength * sizeof(wchar_t);
	wchar_t* pszBuf = (wchar_t*)malloc(iSize);

	wcscpy(pszBuf, pszCurr);
	wcscat(pszBuf, pszText);

	SetText(pszBuf);
	free(pszBuf);
}

void CLabel::AppendText(const char* pszText)
{
	if (!pszText)
		return;

	size_t iSize = (strlen(pszText) + 1) * sizeof(wchar_t);
	wchar_t* pszBuf = (wchar_t*)malloc(iSize);
	mbstowcs(pszBuf, pszText, strlen(pszText)+1);
	AppendText(pszBuf);
	free(pszBuf);
}

int CLabel::GetTextWidth()
{
	return (int)m_pFont->Advance(m_pszText);
}

float CLabel::GetTextHeight()
{
	return (m_pFont->LineHeight()) * m_iTotalLines;
}

void CLabel::ComputeLines(int w, int h)
{
	if (w == -1)
		w = m_iW;

	if (h == -1)
		h = m_iH;

	wchar_t* pszSeps = L"\n";
	wchar_t* pszText = _wcsdup(m_pszText);

	// Cut off any ending line returns so that labels don't have hanging space below.
	if (pszText[wcslen(pszText)-1] == L'\n')
		pszText[wcslen(pszText)-1] = L'\0';

	// FIXME: All this code is technically duplicated from Paint(),
	// but I can't think of a good way to reconcile them. Some kind
	// of lineating method is required or something...? We need to
	// add up all the lines as if they were being truncated during
	// printing to get the real height of all the text.
	wchar_t* pszTok = wcstok(pszText, pszSeps);

	m_iTotalLines = 0;

	while (pszTok)
	{
		float tw = m_pFont->Advance(pszTok);
		float t = m_pFont->LineHeight();

		if (!m_bWrap || tw < w || w == 0 || (m_iTotalLines+1)*t > h)
		{
			m_iTotalLines++;
		}
		else
		{
			tw = 0;
			unsigned int iSource = 0;
			int iLastSpace = 0, iLastBreak = 0, iLength = 0;
			while (iSource < wcslen(pszTok))
			{
				wchar_t szChar[2];
				szChar[0] = pszTok[iSource];
				szChar[1] = L'\0';
				float cw = m_pFont->Advance(szChar);
				if (tw + cw < w || (tw == 0 && w < cw) || (m_iTotalLines+1)*t > h)
				{
					iLength++;
					if (pszTok[iSource] == L' ')
						iLastSpace = iSource;
					tw += cw;
				}
				else
				{
					int iBackup = iSource - iLastSpace;
					if (iLastSpace == iLastBreak)
						iBackup = 0;

					iSource -= iBackup;
					iLength -= iBackup;

					iLength = 0;
					tw = 0;
					while (iSource < wcslen(pszTok) && pszTok[iSource] == L' ')
						iSource++;
					iLastBreak = iLastSpace = iSource--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
					m_iTotalLines++;
				}

				iSource++;
			}

			m_iTotalLines++;
		}

		pszTok = wcstok(NULL, pszSeps);
	}

	free(pszText);
}

// Make the label tall enough for one line of text to fit inside.
void CLabel::EnsureTextFits()
{
	int w = GetTextWidth()+4;
	int h = (int)GetTextHeight()+4;

	if (m_iH < h)
		SetSize(m_iW, h);

	if (m_iW < w)
		SetSize(w, m_iH);
}

int CLabel::GetTextWidth( /*vgui::HFont& font,*/ const wchar_t *str, int iLength )
{
	int pixels = 0;
	wchar_t *p = (wchar_t *)str;
	while ( *p && p-str < iLength )
	{
//		pixels += vgui::surface()->GetCharacterWidth( font, *p++ );
	}
	return pixels;
}

const wchar_t* CLabel::GetText()
{
	if (!m_pszText)
		return L"";
	else
		return m_pszText;
}

Color CLabel::GetFGColor()
{
	return m_FGColor;
}

void CLabel::SetFGColor(Color FGColor)
{
	m_FGColor = FGColor;
	SetAlpha(FGColor.a());
}

void CLabel::SetAlpha(int a)
{
	CBaseControl::SetAlpha(a);
	m_FGColor.SetAlpha(a);
}

CButton::CButton(int x, int y, int w, int h, const char* pszText, bool bToggle)
	: CLabel(x, y, w, h, pszText)
{
	m_bToggle = bToggle;
	m_bToggleOn = false;
	m_bDown = false;
	m_flHighlightGoal = m_flHighlight = 0;
	m_pClickListener = NULL;
	m_pfnClickCallback = NULL;
	m_pUnclickListener = NULL;
	m_pfnUnclickCallback = NULL;
}

void CButton::Destructor()
{
	CLabel::Destructor();
}

void CButton::SetToggleState(bool bState)
{
	if (m_bDown == bState)
		return;

	m_bToggleOn = m_bDown = bState;
}

bool CButton::Push()
{
	if (!m_bEnabled)
		return false;

	if (m_bDown && !m_bToggle)
		return false;

	m_bDown = true;

	if (m_bToggle)
		m_bToggleOn = !m_bToggleOn;

	return true;
}

bool CButton::Pop(bool bRegister, bool bReverting)
{
	if (!m_bDown)
		return false;

	if (m_bToggle)
	{
		if (bReverting)
			m_bToggleOn = !m_bToggleOn;

		if (m_bToggleOn)
			SetState(true, bRegister);
		else
			SetState(false, bRegister);
	}
	else
		SetState(false, bRegister);

	return true;
}

void CButton::SetState(bool bDown, bool bRegister)
{
	m_bDown = bDown;

	if (m_bToggle)
		m_bToggleOn = bDown;

	if (m_bToggle && !m_bToggleOn)
	{
		if (bRegister && m_pUnclickListener && m_pfnUnclickCallback)
			m_pfnUnclickCallback(m_pUnclickListener);
	}
	else
	{
		if (bRegister && m_pClickListener && m_pfnClickCallback)
			m_pfnClickCallback(m_pClickListener);
	}
}

void CButton::SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	assert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pClickListener = pListener;
	m_pfnClickCallback = pfnCallback;
}

void CButton::SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	assert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pUnclickListener = pListener;
	m_pfnUnclickCallback = pfnCallback;
}

bool CButton::MousePressed(int code, int mx, int my)
{
	if (!IsVisible())
		return CLabel::MousePressed(code, mx, my);

	bool bUsed = false;
	if (code == 0)
	{
		bUsed = Push();
		CRootPanel::Get()->SetButtonDown(this);
	}
	return bUsed;
}

bool CButton::MouseReleased(int code, int mx, int my)
{
	if (!IsVisible())
		return CLabel::MouseReleased(code, mx, my);

	if (CRootPanel::Get()->GetButtonDown() != this)
		return false;

	bool bUsed = false;
	if (code == 0)
	{
		bUsed = Pop();
		CRootPanel::Get()->SetButtonDown(NULL);
	}
	return bUsed;
}

void CButton::CursorIn()
{
	CLabel::CursorIn();

	m_flHighlightGoal = 1;
}

void CButton::CursorOut()
{
	CLabel::CursorOut();

	m_flHighlightGoal = 0;
}

void CButton::SetToggleButton(bool bToggle)
{
	if (m_bToggle == bToggle)
		return;

	m_bToggle = bToggle;

	SetState(false, false);
}

void CButton::Think()
{
	m_flHighlight = Approach(m_flHighlightGoal, m_flHighlight, CRootPanel::Get()->GetFrameTime()*3);

	CLabel::Think();
}

void CButton::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	PaintButton(x, y, w, h);

	// Now paint the text which appears on the button.
	CLabel::Paint(x, y, w, h);
}

void CButton::PaintButton(int x, int y, int w, int h)
{
	if (!m_bEnabled)
	{
	}
	else if (m_bDown)
	{
		CRootPanel::PaintRect(x, y, w, h, g_clrBoxHi);
	}
	else
	{
		Color clrBox = g_clrBox;
		clrBox.SetAlpha((int)RemapVal(m_flHighlight, 0, 1, 125, 255));
		CRootPanel::PaintRect(x, y, w, h, clrBox);
	}
}

CSlidingPanel::CInnerPanel::CInnerPanel(CSlidingContainer* pMaster)
	: CPanel(0, 0, 100, SLIDER_COLLAPSED_HEIGHT)
{
	m_pMaster = pMaster;
}

bool CSlidingPanel::CInnerPanel::IsVisible()
{
	if (!m_pMaster->IsCurrent(dynamic_cast<CSlidingPanel*>(m_pParent)))
		return false;

	return CPanel::IsVisible();
}

CSlidingPanel::CSlidingPanel(CSlidingContainer* pParent, char* pszTitle)
	: CPanel(0, 0, 100, 5)
{
	assert(pParent);

	m_bCurrent = false;

//	m_pTitle = new CLabel(0, 0, 100, SLIDER_COLLAPSED_HEIGHT, pszTitle);
//	AddControl(m_pTitle);

	m_pInnerPanel = new CInnerPanel(pParent);
	m_pInnerPanel->SetBorder(CPanel::BT_NONE);
	AddControl(m_pInnerPanel);

	// Add to tail so that panels appear in the order they are added.
	pParent->AddControl(this, true);
}

void CSlidingPanel::Layout()
{
//	m_pTitle->SetSize(m_pParent->GetWidth(), SLIDER_COLLAPSED_HEIGHT);

	m_pInnerPanel->SetPos(5, SLIDER_COLLAPSED_HEIGHT);
	m_pInnerPanel->SetSize(GetWidth() - 10, GetHeight() - 5 - SLIDER_COLLAPSED_HEIGHT);

	CPanel::Layout();
}

void CSlidingPanel::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	CPanel::Paint(x, y, w, h);
}

bool CSlidingPanel::MousePressed(int code, int mx, int my)
{
	CSlidingContainer* pParent = dynamic_cast<CSlidingContainer*>(m_pParent);

	if (pParent->IsCurrent(this))
		return CPanel::MousePressed(code, mx, my);
	else
	{
		pParent->SetCurrent(this);
		return true;
	}
}

void CSlidingPanel::AddControl(IControl* pControl, bool bToTail)
{
	// The title and inner panel should be added to this panel.
	// All other controls should be added to the inner panel.
	// This way the inner panel can be set not visible in order
	// to set all children not visible at once.

	if (/*pControl != m_pTitle && */pControl != m_pInnerPanel)
	{
		m_pInnerPanel->AddControl(pControl, bToTail);
		return;
	}

	CPanel::AddControl(pControl, bToTail);
}

void CSlidingPanel::SetCurrent(bool bCurrent)
{
	m_bCurrent = bCurrent;

	m_pInnerPanel->SetVisible(bCurrent);
}

CSlidingContainer::CSlidingContainer(int x, int y, int w, int h)
	: CPanel(x, y, w, h)
{
	SetBorder(BT_NONE);
	SetCurrent(0);
}

void CSlidingContainer::Layout()
{
	int iY = 0;
	size_t iCount = m_apControls.size();
	int iCurrentHeight = GetHeight() - CSlidingPanel::SLIDER_COLLAPSED_HEIGHT * (VisiblePanels()-1);

	for (size_t i = 0; i < iCount; i++)
	{
		if (!m_apControls[i]->IsVisible())
			continue;

		m_apControls[i]->SetPos(0, iY);
		m_apControls[i]->SetSize(GetWidth(), (i == m_iCurrent)?iCurrentHeight:CSlidingPanel::SLIDER_COLLAPSED_HEIGHT);

		iY += (i == m_iCurrent)?iCurrentHeight:CSlidingPanel::SLIDER_COLLAPSED_HEIGHT;
	}

	CPanel::Layout();
}

void CSlidingContainer::AddControl(IControl* pControl, bool bToTail)
{
	if (!pControl)
		return;

	assert(dynamic_cast<CSlidingPanel*>(pControl));

	CPanel::AddControl(pControl, bToTail);

	// Re-layout now that we've added some. Maybe this one is the current one!
	SetCurrent(m_iCurrent);
}

bool CSlidingContainer::IsCurrent(int iPanel)
{
	return iPanel == m_iCurrent;
}

void CSlidingContainer::SetCurrent(int iPanel)
{
	if (m_iCurrent < (int)m_apControls.size())
		dynamic_cast<CSlidingPanel*>(m_apControls[m_iCurrent])->SetCurrent(false);

	m_iCurrent = iPanel;

	// iPanel may be invalid, for example if the container is empty and being initialized to 0.
	if (m_iCurrent < (int)m_apControls.size())
	{
		dynamic_cast<CSlidingPanel*>(m_apControls[m_iCurrent])->SetCurrent(true);

		Layout();
	}
}

bool CSlidingContainer::IsCurrent(CSlidingPanel* pPanel)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
		if (m_apControls[i] == pPanel)
			return IsCurrent((int)i);

	return false;
}

void CSlidingContainer::SetCurrent(CSlidingPanel* pPanel)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
		if (m_apControls[i] == pPanel)
			SetCurrent((int)i);
}

bool CSlidingContainer::IsCurrentValid()
{
	if (m_iCurrent >= (int)m_apControls.size())
		return false;

	if (!m_apControls[m_iCurrent]->IsVisible())
		return false;

	return true;
}

int CSlidingContainer::VisiblePanels()
{
	int iResult = 0;
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		if (m_apControls[i]->IsVisible())
			iResult++;
	}
	return iResult;
}

CRootPanel*	CRootPanel::s_pRootPanel = NULL;

CRootPanel::CRootPanel() :
	CPanel(0, 0, 800, 600)
{
	assert(!s_pRootPanel);

	s_pRootPanel = this;

	CPanel::SetBorder(BT_NONE);

	m_pButtonDown = NULL;

	m_pDragging = NULL;

	m_pPopup = NULL;

	m_pMenuBar = new CMenuBar();
	AddControl(m_pMenuBar);

	m_flFrameTime = 0;
}

CRootPanel::~CRootPanel( )
{
	Destructor();
}

void CRootPanel::Destructor( )
{
	m_bDestructing = true;

	size_t iCount = m_apDroppables.size();
	for (size_t i = 0; i < iCount; i++)
	{
		// Christ.
		m_apDroppables[i]->Destructor();
		m_apDroppables[i]->Delete();
	}
	m_apDroppables.clear();

	m_bDestructing = false;

	CPanel::Destructor();

	s_pRootPanel = NULL;
}

CRootPanel*	CRootPanel::Get()
{
	if (!s_pRootPanel)
		s_pRootPanel = new CRootPanel();

	return s_pRootPanel;
}

void CRootPanel::Think()
{
	static int iTime = 0;

	if (iTime == glutGet(GLUT_ELAPSED_TIME))
		return;

	m_flFrameTime = ((float)glutGet(GLUT_ELAPSED_TIME) - iTime)/1000;

	CPanel::Think();

	iTime = glutGet(GLUT_ELAPSED_TIME);
}

void CRootPanel::Paint()
{
	// Switch GL to 2d drawing model.
	int aiViewport[4];
	glGetIntegerv(GL_VIEWPORT, aiViewport);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, aiViewport[2], 0, aiViewport[3], -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	CPanel::Paint();

	if (m_pDragging)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		int iWidth = GetHeight()/12;
//		m_pDragging->GetCurrentDraggable()->Paint(mx-iWidth/2, my-iWidth/2, iWidth, iWidth, true);
	}

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void CRootPanel::Layout()
{
	// Don't layout if 
	if (m_pDragging)
		return;

	int aiViewport[4];
	glGetIntegerv(GL_VIEWPORT, aiViewport);
	SetDimensions(aiViewport[0], aiViewport[1], aiViewport[2], aiViewport[3]);

	CPanel::Layout();
}

void CRootPanel::SetButtonDown(CButton* pButton)
{
	m_pButtonDown = pButton;
}

CButton* CRootPanel::GetButtonDown()
{
	return m_pButtonDown;
}

bool CRootPanel::MousePressed(int code, int mx, int my)
{
	assert(!m_pDragging);

	if (m_pPopup)
	{
		int x = 0, y = 0, w = 0, h = 0;
		m_pPopup->GetAbsDimensions(x, y, w, h);
		if (!(mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h))
		{
			m_pPopup->Close();
			m_pPopup = NULL;
		}
	}

	return CPanel::MousePressed(code, mx, my);
}

bool CRootPanel::MouseReleased(int code, int mx, int my)
{
	if (m_pDragging)
	{
		if (DropDraggable())
			return true;
	}

	bool bUsed = CPanel::MouseReleased(code, mx, my);

	if (!bUsed)
	{
		// Nothing caught the mouse release, so lets try to pop some buttons.
//		if (m_pButtonDown)
//			return m_pButtonDown->Pop(false, true);
	}

	return bUsed;
}

void CRootPanel::CursorMoved(int x, int y)
{
	m_iMX = x;
	m_iMY = y;

	if (!m_pDragging)
	{
		CPanel::CursorMoved(x, y);
	}
}

void CRootPanel::DragonDrop(IDroppable* pDroppable)
{
	if (!pDroppable->IsVisible())
		return;

	assert(pDroppable);

	m_pDragging = pDroppable;
}

void CRootPanel::AddDroppable(IDroppable* pDroppable)
{
	assert(pDroppable);
	m_apDroppables.push_back(pDroppable);
}

void CRootPanel::RemoveDroppable(IDroppable* pDroppable)
{
	assert(pDroppable);
	if (!m_bDestructing)
	{
		for (size_t i = 0; i < m_apDroppables.size(); i++)
			if (m_apDroppables[i] == pDroppable)
				m_apDroppables.erase(std::remove(m_apDroppables.begin(), m_apDroppables.end(), pDroppable), m_apDroppables.end());
	}
}

bool CRootPanel::DropDraggable()
{
	assert(m_pDragging);

	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	// Drop that shit like a bad habit.

	size_t iCount = m_apDroppables.size();
	for (size_t i = 0; i < iCount; i++)
	{
		IDroppable* pDroppable = m_apDroppables[i];

		assert(pDroppable);

		if (!pDroppable)
			continue;

		if (!pDroppable->IsVisible())
			continue;

		if (!pDroppable->CanDropHere(m_pDragging->GetCurrentDraggable()))
			continue;

		CRect r = pDroppable->GetHoldingRect();
		if (mx >= r.x &&
			my >= r.y &&
			mx < r.x + r.w &&
			my < r.y + r.h)
		{
			pDroppable->SetDraggable(m_pDragging->GetCurrentDraggable());

			m_pDragging = NULL;

			// Layouts during dragging are blocked. Do a Layout() here to do any updates that need doing since the thing was dropped.
			Layout();

			return true;
		}
	}

	// Layouts during dragging are blocked. Do a Layout() here to do any updates that need doing since the thing was dropped.
	Layout();

	// Couldn't find any places to drop? Whatever nobody cares about that anyways.
	m_pDragging = NULL;

	return false;
}

void CRootPanel::Popup(IPopup* pPopup)
{
	m_pPopup = pPopup;
}

void CRootPanel::GetFullscreenMousePos(int& mx, int& my)
{
	mx = Get()->m_iMX;
	my = Get()->m_iMY;
}

void CRootPanel::DrawRect(int x, int y, int x2, int y2)
{
}

CMenu* CRootPanel::AddMenu(const char* pszTitle)
{
	if (!m_pMenuBar)
		return NULL;

	CMenu* pMenu = new CMenu(pszTitle);
	pMenu->SetWrap(false);
	m_pMenuBar->AddControl(pMenu);

	return pMenu;
}

CMenuBar::CMenuBar()
	: CPanel(0, 0, 1024, MENU_HEIGHT)
{
}

void CMenuBar::Layout( void )
{
	if (GetParent())
	{
		SetSize(GetParent()->GetWidth(), MENU_HEIGHT);
		SetPos(MENU_SPACING, GetParent()->GetHeight() - GetHeight() - MENU_SPACING);
	}

	CPanel::Layout();

	int iX = 0;
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		m_apControls[i]->SetPos(iX, 0);
		iX += m_apControls[i]->GetWidth() + MENU_SPACING;
	}
}

void CMenuBar::SetActive( CMenu* pActiveMenu )
{
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		CMenu* pCurrentMenu = dynamic_cast<CMenu*>(m_apControls[i]);

		if (!pCurrentMenu)
			continue;

		if (pCurrentMenu != pActiveMenu)
			pCurrentMenu->Pop(true, true);
	}
}

CMenu::CMenu(const char* pszTitle, bool bSubmenu)
	: CButton(0, 0, 41, MENU_HEIGHT, pszTitle, true)
{
	m_bSubmenu = bSubmenu;

	m_flHighlightGoal = m_flHighlight = m_flMenuHighlightGoal = m_flMenuHighlight = m_flMenuHeightGoal = m_flMenuHeight
		= m_flMenuSelectionHighlightGoal = m_flMenuSelectionHighlight = 0;

	m_MenuSelection = CRect(0, 0, 0, 0);
	m_MenuSelectionGoal = CRect(0, 0, 0, 0);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);

	m_pfnMenuCallback = NULL;
	m_pMenuListener = NULL;

	m_pMenu = new CSubmenuPanel();
	CRootPanel::Get()->AddControl(m_pMenu);

	m_pMenu->SetVisible(false);
}

void CMenu::Think()
{
	// Make a copy so that the below logic doesn't clobber CursorOut()
	float flHightlightGoal = m_flHighlightGoal;

	// If our menu is open always stay highlighted.
	if (m_pMenu->IsVisible())
		flHightlightGoal = 1;

	m_flHighlight = Approach(flHightlightGoal, m_flHighlight, CRootPanel::Get()->GetFrameTime()*3);
	m_flMenuHighlight = Approach(m_flMenuHighlightGoal, m_flMenuHighlight, CRootPanel::Get()->GetFrameTime()*3);
	m_flMenuHeight = Approach(m_flMenuHeightGoal, m_flMenuHeight, CRootPanel::Get()->GetFrameTime()*3);
	m_pMenu->SetFakeHeight(m_flMenuHeight);

	m_pMenu->SetVisible(m_flMenuHighlight > 0 && m_flMenuHeight > 0);

	m_flMenuSelectionHighlightGoal = 0;

	for (size_t i = 0; i < m_pMenu->GetControls().size(); i++)
	{
		int cx, cy, cw, ch, mx, my;
		m_pMenu->GetControls()[i]->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			m_flMenuSelectionHighlightGoal = 1;
			m_MenuSelectionGoal = CRect((float)cx, (float)cy, (float)cw, (float)ch);
			break;
		}
	}

	if (m_flMenuSelectionHighlight < 0.01f)
		m_MenuSelection = m_MenuSelectionGoal;
	else
	{
		m_MenuSelection.x = Approach(m_MenuSelectionGoal.x, m_MenuSelection.x, CRootPanel::Get()->GetFrameTime()*400);
		m_MenuSelection.y = Approach(m_MenuSelectionGoal.y, m_MenuSelection.y, CRootPanel::Get()->GetFrameTime()*400);
		m_MenuSelection.w = Approach(m_MenuSelectionGoal.w, m_MenuSelection.w, CRootPanel::Get()->GetFrameTime()*400);
		m_MenuSelection.h = Approach(m_MenuSelectionGoal.h, m_MenuSelection.h, CRootPanel::Get()->GetFrameTime()*400);
	}

	m_flMenuSelectionHighlight = Approach(m_flMenuSelectionHighlightGoal, m_flMenuSelectionHighlight, CRootPanel::Get()->GetFrameTime()*3);
}

void CMenu::Layout()
{
	int iHeight = 0;
	int iWidth = 0;
	std::vector<IControl*> apControls = m_pMenu->GetControls();
	for (size_t i = 0; i < apControls.size(); i++)
	{
		apControls[i]->SetPos(5, (int)(apControls.size()*MENU_HEIGHT-(i+1)*MENU_HEIGHT));
		iHeight += MENU_HEIGHT;
		if (apControls[i]->GetWidth()+10 > iWidth)
			iWidth = apControls[i]->GetWidth()+10;
	}

	int x, y;
	GetAbsPos(x, y);

	m_pMenu->SetSize(iWidth, iHeight);
	m_pMenu->SetPos(x, y - 5 - m_pMenu->GetHeight());

	m_pMenu->Layout();
}

void CMenu::Paint(int x, int y, int w, int h)
{
	if (!m_bSubmenu)
	{
		Color clrBox = g_clrBox;
		clrBox.SetAlpha((int)RemapVal(m_flHighlight, 0, 1, 125, 255));
		CRootPanel::PaintRect(x, y, w, h, clrBox);
	}

	if (m_pMenu->IsVisible())
	{
		int mx, my, mw, mh;
		m_pMenu->GetAbsDimensions(mx, my, mw, mh);

		float flMenuHeight = Lerp(m_flMenuHeight, 0.6f);
		if (flMenuHeight > 0.99f)
			flMenuHeight = 0.99f;	// When it hits 1 it jerks.

		Color clrBox = g_clrBox;
		clrBox.SetAlpha((int)RemapVal(m_flMenuHighlight, 0, 1, 0, 255));
		CRootPanel::PaintRect(mx, (int)(my + mh - mh*flMenuHeight), mw, (int)(mh*flMenuHeight), clrBox);

		if (m_flMenuSelectionHighlight > 0)
		{
			clrBox = g_clrBoxHi;
			clrBox.SetAlpha((int)(255 * m_flMenuSelectionHighlight * flMenuHeight));
			CRootPanel::PaintRect((int)m_MenuSelection.x, (int)m_MenuSelection.y+1, (int)m_MenuSelection.w, (int)m_MenuSelection.h-2, clrBox);
		}
	}

	CLabel::Paint(x, y, w, h);
}

void CMenu::CursorIn()
{
	m_flHighlightGoal = 1;
}

void CMenu::CursorOut()
{
	m_flHighlightGoal = 0;
}

void CMenu::SetMenuListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnMenuCallback = pfnCallback;
	m_pMenuListener = pListener;
}

void CMenu::OpenCallback()
{
	CRootPanel::Get()->GetMenuBar()->SetActive(this);

	if (m_pMenu->GetControls().size())
	{
		m_flMenuHeightGoal = 1;
		m_flMenuHighlightGoal = 1;
		Layout();
	}
}

void CMenu::CloseCallback()
{
	if (m_pMenu->GetControls().size())
	{
		m_flMenuHeightGoal = 0;
		m_flMenuHighlightGoal = 0;
	}
}

void CMenu::ClickedCallback()
{
	if (m_pMenuListener)
		m_pfnMenuCallback(m_pMenuListener);

	CRootPanel::Get()->GetMenuBar()->SetActive(NULL);
}

void CMenu::AddSubmenu(const char* pszTitle, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	CMenu* pMenu = new CMenu(pszTitle, true);
	pMenu->SetAlign(TA_LEFTCENTER);
	pMenu->SetWrap(false);
	pMenu->EnsureTextFits();
	pMenu->SetToggleButton(false);

	pMenu->SetClickedListener(pMenu, Clicked);

	if (pListener)
		pMenu->SetMenuListener(pListener, pfnCallback);

	m_pMenu->AddControl(pMenu);
}

CMenu::CSubmenuPanel::CSubmenuPanel()
	: CPanel(0, 0, 100, 100)
{
}

void CMenu::CSubmenuPanel::Think()
{
	if (m_apControls.size() != m_aflControlHighlightGoal.size() || m_apControls.size() != m_aflControlHighlight.size())
	{
		m_aflControlHighlightGoal.clear();
		m_aflControlHighlight.clear();

		for (size_t i = 0; i < m_apControls.size(); i++)
		{
			m_aflControlHighlightGoal.push_back(0);
			m_aflControlHighlight.push_back(0);
		}
	}

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		IControl* pControl = m_apControls[i];

		int x, y;
		pControl->GetPos(x, y);

		if (y < GetHeight()-m_flFakeHeight*GetHeight())
			m_aflControlHighlightGoal[i] = 0.0f;
		else
			m_aflControlHighlightGoal[i] = 1.0f;

		m_aflControlHighlight[i] = Approach(m_aflControlHighlightGoal[i], m_aflControlHighlight[i], CRootPanel::Get()->GetFrameTime()*3);

		pControl->SetAlpha((int)(m_aflControlHighlight[i] * 255));
	}

	CPanel::Think();
}
