#include "panel.h"

#include <renderer/renderingcontext.h>

#include "rootpanel.h"
#include "scrollbar.h"

using namespace glgui;

CPanel::CPanel()
	: CBaseControl(0, 0, 100, 100)
{
	m_pHasCursor = NULL;
	m_bHighlight = false;
	m_bDestructing = false;
	m_bScissoring = false;

	m_pVerticalScrollBar = nullptr;
	m_pHorizontalScrollBar = nullptr;
}

CPanel::CPanel(float x, float y, float w, float h)
	: CBaseControl(x, y, w, h)
{
	SetBorder(BT_NONE);
	SetBackgroundColor(Color(0, 0, 0, 0));
	m_pHasCursor = NULL;
	m_bHighlight = false;
	m_bDestructing = false;
	m_bScissoring = false;

	m_pVerticalScrollBar = nullptr;
	m_pHorizontalScrollBar = nullptr;
}

CPanel::~CPanel()
{
	// Protect m_apControls from accesses elsewhere.
	m_bDestructing = true;

	size_t iCount = m_apControls.size();
	size_t i;
	for (i = 0; i < iCount; i++)
		delete m_apControls[i];

	m_apControls.clear();

	m_bDestructing = false;
}

bool CPanel::KeyPressed(int code, bool bCtrlDown)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->KeyPressed(code, bCtrlDown))
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

bool CPanel::CharPressed(int code)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		IControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->CharPressed(code))
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

		float x = 0, y = 0, w = 0, h = 0;
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

		float x, y, w, h;
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

		float x, y, w, h;
		pControl->GetAbsDimensions(x, y, w, h);

		if (m_pVerticalScrollBar && pControl != m_pVerticalScrollBar)
		{
			if (mx >= m_pVerticalScrollBar->GetLeft())
				continue;
		}

		if (m_pHorizontalScrollBar && pControl != m_pHorizontalScrollBar)
		{
			if (my >= m_pHorizontalScrollBar->GetTop())
				continue;
		}

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

	BaseClass::CursorOut();
}

IControl* CPanel::GetHasCursor()
{
	if (!m_pHasCursor)
		return this;

	return m_pHasCursor->GetHasCursor();
}

size_t CPanel::AddControl(IControl* pControl, bool bToTail)
{
	if (!pControl)
		return ~0;

	TAssert(pControl != this);

#ifdef _DEBUG
	for (size_t i = 0; i < m_apControls.size(); i++)
		TAssert(m_apControls[i] != pControl);	// You're adding a control to the panel twice! Quit it!
#endif

	pControl->SetParent(this);

	if (bToTail)
	{
		m_apControls.push_back(pControl);
		return m_apControls.size()-1;
	}
	else
	{
		m_apControls.insert(m_apControls.begin(), pControl);
		return 0;
	}
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
				m_apControls.erase(eastl::remove(m_apControls.begin(), m_apControls.end(), pControl), m_apControls.end());
		}
	}

	pControl->SetParent(NULL);

	if (m_pHasCursor == pControl)
		m_pHasCursor = NULL;
}

void CPanel::MoveToTop(IControl* pControl)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		if (m_apControls[i] == pControl)
		{
			m_apControls.erase(eastl::remove(m_apControls.begin(), m_apControls.end(), pControl), m_apControls.end());
			m_apControls.push_back(pControl);
			return;
		}
	}
}

void CPanel::Layout( void )
{
	FRect rPanelBounds = GetAbsDimensions();
	FRect rAllBounds = GetAbsDimensions();

	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		m_apControls[i]->Layout();

		FRect rControlBounds = m_apControls[i]->GetAbsDimensions();

		if (rControlBounds.x < rAllBounds.x)
			rAllBounds.x = rControlBounds.x;

		if (rControlBounds.y < rAllBounds.y)
			rAllBounds.y = rControlBounds.y;

		if (rControlBounds.Right() > rAllBounds.Right())
			rAllBounds.w = rControlBounds.Right() - rAllBounds.x;

		if (rControlBounds.Bottom() > rAllBounds.Bottom())
			rAllBounds.h = rControlBounds.Bottom() - rAllBounds.y;
	}

	m_rControlBounds = rAllBounds;

	if (m_pVerticalScrollBar)
	{
		m_pVerticalScrollBar->SetVisible((rAllBounds.y < rPanelBounds.y) || (rAllBounds.Bottom() > rPanelBounds.Bottom()));
	}

	if (m_pHorizontalScrollBar)
	{
		m_pHorizontalScrollBar->SetVisible((rAllBounds.x < rPanelBounds.x) || (rAllBounds.Right() > rPanelBounds.Right()));
	}
}

void CPanel::UpdateScene( void )
{
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
		m_apControls[i]->UpdateScene();
}

void CPanel::Paint()
{
	float x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CPanel::Paint(float x, float y)
{
	Paint(x, y, m_flW, m_flH);
}

void CPanel::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	bool bScissor = m_bScissoring;
	float sx, sy;
	if (bScissor)
	{
		GetAbsPos(sx, sy);

		//CRootPanel::PaintRect(sx, sy, GetWidth(), GetHeight(), Color(0, 0, 100, 50));

		CRootPanel::GetContext()->SetUniform("bScissor", true);
		CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
	}

	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		IControl* pControl = m_apControls[i];
		if (!pControl->IsVisible())
			continue;

		if (bScissor)
		{
			CRootPanel::GetContext()->SetUniform("bScissor", true);
			CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
		}

		// Translate this location to the child's local space.
		float cx, cy, ax, ay;
		pControl->GetAbsPos(cx, cy);
		GetAbsPos(ax, ay);
		pControl->PaintBackground(cx+x-ax, cy+y-ay, pControl->GetWidth(), pControl->GetHeight());
		pControl->Paint(cx+x-ax, cy+y-ay);
	}

	if (bScissor)
		CRootPanel::GetContext()->SetUniform("bScissor", false);

	BaseClass::Paint(x, y, w, h);
}

bool CPanel::ShouldControlOffset(IControl* pControl) const
{
	if (pControl == m_pVerticalScrollBar || pControl == m_pHorizontalScrollBar)
		return false;

	return true;
}

void CPanel::Think()
{
	size_t iCount = m_apControls.size();
	for (size_t i = iCount-1; i < iCount; i--)
	{
		m_apControls[i]->Think();
	}

	m_rControlOffset = FRect(0, 0, 0, 0);

	if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
	{
		float flScrollable = m_rControlBounds.h - GetHeight();
		m_rControlOffset.y = -m_pVerticalScrollBar->GetHandlePosition() * flScrollable;
	}

	if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
	{
		float flScrollable = m_rControlBounds.w - GetWidth();
		m_rControlOffset.x = -m_pHorizontalScrollBar->GetHandlePosition() * flScrollable;
	}
}

void CPanel::SetVerticalScrollBarEnabled(bool b)
{
	if (m_pVerticalScrollBar && b)
		return;

	if (!m_pVerticalScrollBar && !b)
		return;

	if (b)
	{
		m_pVerticalScrollBar = new CScrollBar(false);
		AddControl(m_pVerticalScrollBar);
		Layout();
	}
	else
	{
		RemoveControl(m_pVerticalScrollBar);
		delete m_pVerticalScrollBar;
		m_pVerticalScrollBar = nullptr;
	}
}

void CPanel::SetHorizontalScrollBarEnabled(bool b)
{
	if (m_pHorizontalScrollBar && b)
		return;

	if (!m_pHorizontalScrollBar && !b)
		return;

	if (b)
	{
		m_pHorizontalScrollBar = new CScrollBar(true);
		AddControl(m_pHorizontalScrollBar);
		Layout();
	}
	else
	{
		RemoveControl(m_pHorizontalScrollBar);
		delete m_pHorizontalScrollBar;
		m_pHorizontalScrollBar = nullptr;
	}
}
