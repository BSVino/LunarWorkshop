#include "movablepanel.h"

#include <maths.h>
#include <strutils.h>
#include <tinker_platform.h>
#include <tinker/keys.h>
#include <glgui/menu.h>
#include <glgui/rootpanel.h>
#include <glgui/picturebutton.h>
#include <glgui/checkbox.h>
#include <glgui/tree.h>
#include <glgui/textfield.h>
#include <glgui/filedialog.h>

using namespace glgui;

void CCloseButton::Paint(float x, float y, float w, float h)
{
	Color c;
	
	c.SetRed((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.r() * 2, 255.0f));
	c.SetGreen((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.g(), (float)g_clrBoxHi.g()));
	c.SetBlue((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.b(), (float)g_clrBoxHi.b()));
	c.SetAlpha(255);

	CRootPanel::PaintRect(x, y, w, h, c);
}

void CMinimizeButton::Paint(float x, float y, float w, float h)
{
	Color c;
	
	c.SetRed((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.r(), (float)g_clrBox.r()));
	c.SetGreen((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.g(), (float)g_clrBoxHi.g()));
	c.SetBlue((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.b() * 2, 255.0f));
	c.SetAlpha(255);

	CRootPanel::PaintRect(x, y+h/3+1, w, h/3, c);
}

CMovablePanel::CMovablePanel(const tstring& sName)
	: CPanel(0, 0, 200, 350)
{
	m_sName = sName;

	m_bMoving = false;

	m_bHasCloseButton = true;
	m_bMinimized = false;
	m_flMinimizeTransitionLerp = 1.0;
	m_flNonMinimizedHeight = GetHeight();

	SetBackgroundColor(g_clrPanel);
	SetBorder(glgui::CPanel::BT_SOME);
	SetClearBackground(false);
}

CMovablePanel::~CMovablePanel()
{
}

void CMovablePanel::CreateControls(CResource<CBaseControl> pThis)
{
	m_hName = AddControl(new CLabel(0, GetHeight()-HEADER_HEIGHT, GetWidth(), HEADER_HEIGHT, m_sName));
	m_sName.set_capacity(0);

	m_hCloseButton = AddControl(new CCloseButton());
	m_hCloseButton->SetClickedListener(this, CloseWindow);

	m_hMinimizeButton = AddControl(new CMinimizeButton());
	m_hMinimizeButton->SetClickedListener(this, MinimizeWindow);

	RootPanel()->AddControl(pThis, true);

	BaseClass::CreateControls(pThis);
}

void CMovablePanel::Layout()
{
	float flButtonSize = HEADER_HEIGHT*2/3;

	m_hName->SetDimensions(flButtonSize*4, 0, GetWidth()-flButtonSize*8, HEADER_HEIGHT);

	m_hCloseButton->SetVisible(m_bHasCloseButton);

	if (m_bHasCloseButton)
	{
		m_hCloseButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - flButtonSize/2, HEADER_HEIGHT/2 - flButtonSize/2, flButtonSize, flButtonSize);
		m_hMinimizeButton->SetDimensions(GetWidth() - HEADER_HEIGHT*3/2 - flButtonSize/2, HEADER_HEIGHT/2 - flButtonSize/2, flButtonSize, flButtonSize);
	}
	else
		m_hMinimizeButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - flButtonSize/2, HEADER_HEIGHT/2 - flButtonSize/2, flButtonSize, flButtonSize);

	CPanel::Layout();

	m_flNonMinimizedHeight = GetHeight();
}

void CMovablePanel::Think()
{
	if (m_bMoving)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		SetPos(m_flStartX + mx - m_iMouseStartX, m_flStartY + my - m_iMouseStartY);
	}

	float flMinimizeTransitionGoal = 1;
	if (m_bMinimized)
		flMinimizeTransitionGoal = 0;

	m_flMinimizeTransitionLerp = Approach(flMinimizeTransitionGoal, m_flMinimizeTransitionLerp, (float)CRootPanel::Get()->GetFrameTime()*3);

	if (!m_bMinimized)
		m_flNonMinimizedHeight = GetHeight();

	CPanel::Think();
}

void CMovablePanel::PaintBackground(float x, float y, float w, float h)
{
	float flMinimizedHeight = HEADER_HEIGHT;
	float flMaximumHeight = m_flNonMinimizedHeight;
	float flHeight = RemapVal(Lerp(m_flMinimizeTransitionLerp, 0.7f), 0, 1, flMinimizedHeight, flMaximumHeight);

	BaseClass::PaintBackground(x, y, w, flHeight);
}

void CMovablePanel::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, HEADER_HEIGHT, m_clrHeader);

	CPanel::Paint(x, y, w, h);
}

bool CMovablePanel::MousePressed(int iButton, int mx, int my)
{
	float x, y;
	GetAbsPos(x, y);

	if (iButton == TINKER_KEY_MOUSE_LEFT && mx > x && mx < x + GetWidth() - HEADER_HEIGHT*2 && my > y && my < y + HEADER_HEIGHT )
	{
		m_iMouseStartX = mx;
		m_iMouseStartY = my;
		m_bMoving = true;

		GetPos(x, y);
		m_flStartX = x;
		m_flStartY = y;

		return true;
	}

	return CPanel::MousePressed(iButton, mx, my);
}

bool CMovablePanel::MouseReleased(int iButton, int mx, int my)
{
	if (m_bMoving)
	{
		m_bMoving = false;
		return true;
	}

	CPanel::MouseReleased(iButton, mx, my);

	return false;
}

void CMovablePanel::Minimize()
{
	m_bMinimized = !m_bMinimized;

	if (m_bMinimized)
	{
		m_flNonMinimizedHeight = GetHeight();
		SetSize(GetWidth(), HEADER_HEIGHT);
	}
	else
	{
		SetSize(GetWidth(), m_flNonMinimizedHeight);
	}

	if (!m_bMinimized)
		Layout();
}

bool CMovablePanel::IsChildVisible(CBaseControl* pChild)
{
	if (!m_bMinimized)
		return true;

	if (pChild == m_hMinimizeButton)
		return true;

	if (pChild == m_hCloseButton)
		return true;

	if (pChild == m_hName)
		return true;

	return false;
}

void CMovablePanel::SetClearBackground(bool bClearBackground)
{
	m_bClearBackground = bClearBackground;

	if (m_bClearBackground)
	{
		m_clrHeader.SetAlpha(0.6f);
		SetBackgroundColor(Color(m_clrBackground.r(), m_clrBackground.g(), m_clrBackground.b(), 0));
	}
	else
	{
		m_clrHeader.SetAlpha(1.0f);
		SetBackgroundColor(Color(m_clrBackground.r(), m_clrBackground.g(), m_clrBackground.b(), 255));
	}
}

void CMovablePanel::CloseWindowCallback(const tstring& sArgs)
{
	SetVisible(false);
}

void CMovablePanel::MinimizeWindowCallback(const tstring& sArgs)
{
	Minimize();
}

void CMovablePanel::Close()
{
	SetVisible(false);
	RootPanel()->RemoveControl(this);
}
