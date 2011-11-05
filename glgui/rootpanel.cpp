#include "rootpanel.h"

#include <GL/glew.h>

#include "menu.h"

using namespace glgui;

CRootPanel*	CRootPanel::s_pRootPanel = NULL;

CRootPanel::CRootPanel() :
	CPanel(0, 0, 800, 600)
{
	TAssert(!s_pRootPanel);

	s_pRootPanel = this;

	CPanel::SetBorder(BT_NONE);

	m_pButtonDown = NULL;
	m_pFocus = NULL;
	m_pDragging = NULL;
	m_pPopup = NULL;

	m_pMenuBar = new CMenuBar();
	AddControl(m_pMenuBar, true);

	m_flFrameTime = 0;
	m_flTime = 0;

	m_bUseLighting = true;
}

CRootPanel::~CRootPanel( )
{
	m_bDestructing = true;

	size_t iCount = m_apDroppables.size();
	for (size_t i = 0; i < iCount; i++)
		delete m_apDroppables[i];

	m_apDroppables.clear();

	m_bDestructing = false;

	s_pRootPanel = NULL;
}

CRootPanel*	CRootPanel::Get()
{
	if (!s_pRootPanel)
		s_pRootPanel = new CRootPanel();

	return s_pRootPanel;
}

void CRootPanel::Think(float flNewTime)
{
	if (m_flTime == flNewTime)
		return;

	m_flFrameTime = (flNewTime - m_flTime);

	// Time running backwards? Maybe the server restarted.
	if (m_flFrameTime < 0)
		m_flFrameTime = 0;

	CPanel::Think();

	m_flTime = flNewTime;
}

void CRootPanel::UpdateScene()
{
	m_pDragging = NULL;

	CPanel::UpdateScene();
}

void CRootPanel::Paint(int x, int y, int w, int h)
{
	SetSize(w, h);

	// Switch GL to 2d drawing mode.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(x, x+w, y+h, y, -1000, 1000);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	if (m_bUseLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glShadeModel(GL_FLAT);
#else
	glShadeModel(GL_SMOOTH);
#endif

	CPanel::Paint(x, y, w, h);

	if (m_pDragging)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		int iWidth = m_pDragging->GetCurrentDraggable()->GetWidth();
		int iHeight = m_pDragging->GetCurrentDraggable()->GetHeight();
		m_pDragging->GetCurrentDraggable()->Paint(mx-iWidth/2, my-iHeight/2, iWidth, iHeight, true);
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

bool CRootPanel::MousePressed(int code, int mx, int my, bool bInsideControl)
{
	TAssert(!m_pDragging);

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

	if (CPanel::MousePressed(code, mx, my))
		return true;

	if (!bInsideControl)
		return false;

	int iCount = (int)m_apControls.size();
	for (int i = 0; i < iCount; i++)
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
			// If we were inside any visible elements, don't rotate the screen.
			return true;
		}
	}

	return false;
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

	if (!pDroppable->GetCurrentDraggable()->IsDraggable())
		return;

	TAssert(pDroppable);

	m_pDragging = pDroppable;
}

void CRootPanel::AddDroppable(IDroppable* pDroppable)
{
	TAssert(pDroppable);
	m_apDroppables.push_back(pDroppable);
}

void CRootPanel::RemoveDroppable(IDroppable* pDroppable)
{
	TAssert(pDroppable);
	if (!m_bDestructing)
	{
		for (size_t i = 0; i < m_apDroppables.size(); i++)
			if (m_apDroppables[i] == pDroppable)
				m_apDroppables.erase(eastl::remove(m_apDroppables.begin(), m_apDroppables.end(), pDroppable), m_apDroppables.end());
	}
}

bool CRootPanel::DropDraggable()
{
	TAssert(m_pDragging);

	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	// Drop that shit like a bad habit.

	size_t iCount = m_apDroppables.size();
	for (size_t i = 0; i < iCount; i++)
	{
		IDroppable* pDroppable = m_apDroppables[i];

		TAssert(pDroppable);

		if (!pDroppable)
			continue;

		if (!pDroppable->IsVisible())
			continue;

		if (!pDroppable->CanDropHere(m_pDragging->GetCurrentDraggable()))
			continue;

		FRect r = pDroppable->GetHoldingRect();
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

void CRootPanel::SetFocus(CBaseControl* pFocus)
{
	if (m_pFocus)
		m_pFocus->SetFocus(false);

	if (pFocus)
		pFocus->SetFocus(true);

	m_pFocus = pFocus;
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
