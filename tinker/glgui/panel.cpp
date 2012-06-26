#include "panel.h"

#include <tinker/shell.h>
#include <renderer/renderingcontext.h>
#include <tinker/keys.h>

#include "rootpanel.h"
#include "scrollbar.h"

using namespace glgui;

CPanel::CPanel()
	: CBaseControl(0, 0, 100, 100)
{
	m_flMargin = 15;

	m_bHighlight = false;
	m_bScissoring = false;
}

CPanel::CPanel(float x, float y, float w, float h)
	: CBaseControl(x, y, w, h)
{
	m_flMargin = 15;

	SetBorder(BT_NONE);
	SetBackgroundColor(Color(0, 0, 0, 0));
	m_bHighlight = false;
	m_bScissoring = false;
}

CPanel::~CPanel()
{
}

bool CPanel::KeyPressed(int code, bool bCtrlDown)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->TakesFocus() && !pControl->HasFocus())
			continue;

		if (pControl->KeyPressed(code, bCtrlDown))
			return true;

		if (pControl->HasFocus() && code == TINKER_KEY_TAB)
		{
			NextTabStop();
			return true;
		}
	}
	return false;
}

bool CPanel::KeyReleased(int code)
{
	int iCount = (int)m_apControls.size();

	// Start at the end of the list so that items drawn last are tested for keyboard events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

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
		CBaseControl* pControl = m_apControls[i];

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
		CBaseControl* pControl = m_apControls[i];

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
		CBaseControl* pControl = m_apControls[i];

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

bool CPanel::MouseDoubleClicked(int code, int mx, int my)
{
	int iCount = (int)m_apControls.size();
	// Start at the end of the list so that items drawn last are tested for mouse events first.
	for (int i = iCount-1; i >= 0; i--)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		float x = 0, y = 0, w = 0, h = 0;
		pControl->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (pControl->MouseDoubleClicked(code, mx, my))
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
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible() || !pControl->IsCursorListener())
			continue;

		float x, y, w, h;
		pControl->GetAbsDimensions(x, y, w, h);

		if (m_hVerticalScrollBar && pControl != m_hVerticalScrollBar)
		{
			if (mx >= m_hVerticalScrollBar->GetLeft())
				continue;
		}

		if (m_hHorizontalScrollBar && pControl != m_hHorizontalScrollBar)
		{
			if (my >= m_hHorizontalScrollBar->GetTop())
				continue;
		}

		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			if (m_hHasCursor != pControl)
			{
				if (m_hHasCursor)
					m_hHasCursor->CursorOut();
				m_hHasCursor = pControl->GetHandle();
				m_hHasCursor->CursorIn();
			}

			pControl->CursorMoved(mx, my);

			bFoundControlWithCursor = true;
			break;
		}
	}

	if (!bFoundControlWithCursor && m_hHasCursor)
	{
		m_hHasCursor->CursorOut();
		m_hHasCursor.reset();
	}
}

void CPanel::CursorOut()
{
	if (m_hHasCursor)
	{
		m_hHasCursor->CursorOut();
		m_hHasCursor.reset();
	}

	BaseClass::CursorOut();
}

CControlHandle CPanel::GetHasCursor()
{
	if (!m_hHasCursor)
		return m_hThis;

	return m_hHasCursor->GetHasCursor();
}

void CPanel::NextTabStop()
{
	size_t iOriginalFocus = 0;

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		CBaseControl* pControl = m_apControls[i];

		if (!pControl->IsVisible())
			continue;

		if (pControl->HasFocus())
		{
			iOriginalFocus = i;
			break;
		}
	}

	for (size_t i = 0; i < m_apControls.size()-1; i++)
	{
		size_t iControl = (iOriginalFocus + i + 1)%m_apControls.size();
		CBaseControl* pControl = m_apControls[iControl];

		if (!pControl->IsVisible())
			continue;

		if (pControl->TakesFocus())
		{
			CBaseControl* pBaseControl = dynamic_cast<CBaseControl*>(pControl);
			if (!pBaseControl)
				continue;

			CRootPanel::Get()->SetFocus(pBaseControl->GetHandle());
			return;
		}
	}
}

CControlHandle CPanel::AddControl(CBaseControl* pControl, bool bToTail)
{
	if (pControl)
		TAssertNoMsg(pControl->GetHandle() == nullptr)	// If you hit this assert, don't create the control with CreateControl() this function does that.
	else
		return CControlHandle();

	return AddControl(CreateControl(pControl), bToTail);
}

CControlHandle CPanel::AddControl(CResource<CBaseControl> pControl, bool bToTail)
{
	if (pControl.get())
		TAssertNoMsg(pControl->GetHandle() != nullptr)	// If you hit this assert, you didn't create the control with CreateControl(new CYadeya)
	else
		return CControlHandle();

	TAssertNoMsg(pControl != this);

#ifdef _DEBUG
	for (size_t i = 0; i < m_apControls.size(); i++)
		TAssertNoMsg(m_apControls[i] != pControl);	// You're adding a control to the panel twice! Quit it!
#endif

	// If you hit this assert then you're adding a control to a panel that hasn't had CreateControl called on it yet.
	// Don't create child controls for a panel in its constructor, do it in CreateControls()
	TAssertNoMsg(m_hThis);

	pControl->SetParent(m_hThis);

	if (bToTail)
		m_apControls.push_back(pControl);
	else
		m_apControls.insert(m_apControls.begin(), pControl);

	return CControlHandle(pControl);
}

void CPanel::RemoveControl(CBaseControl* pControl)
{
	pControl->SetParent(CControlHandle());

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		if (m_apControls[i] == pControl)
			m_apControls.erase(m_apControls.begin()+i);
	}

	if (m_hHasCursor == pControl)
		m_hHasCursor.reset();
}

void CPanel::MoveToTop(CBaseControl* pControl)
{
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		if (m_apControls[i] == pControl)
		{
			CResource<CBaseControl> pControlResource = m_apControls[i];
			m_apControls.erase(m_apControls.begin()+i);
			m_apControls.push_back(pControlResource);
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

	if (m_hVerticalScrollBar)
	{
		m_hVerticalScrollBar->SetVisible((rAllBounds.y < rPanelBounds.y) || (rAllBounds.Bottom() > rPanelBounds.Bottom()));
	}

	if (m_hHorizontalScrollBar)
	{
		m_hHorizontalScrollBar->SetVisible((rAllBounds.x < rPanelBounds.x) || (rAllBounds.Right() > rPanelBounds.Right()));
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
		CBaseControl* pControl = m_apControls[i];
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

void CPanel::PostPaint()
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
		CBaseControl* pControl = m_apControls[i];
		if (!pControl->IsVisible())
			continue;

		if (bScissor)
		{
			CRootPanel::GetContext()->SetUniform("bScissor", true);
			CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
		}

		pControl->PostPaint();
	}

	if (bScissor)
		CRootPanel::GetContext()->SetUniform("bScissor", false);

	BaseClass::PostPaint();
}

bool CPanel::ShouldControlOffset(const CBaseControl* pControl) const
{
	if (pControl == m_hVerticalScrollBar || pControl == m_hHorizontalScrollBar)
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

	if (m_hVerticalScrollBar && m_hVerticalScrollBar->IsVisible())
	{
		float flScrollable = m_rControlBounds.h - GetHeight();
		m_rControlOffset.y = -GetVerticalScrollBar()->GetHandlePosition() * flScrollable;
	}

	if (m_hHorizontalScrollBar && m_hHorizontalScrollBar->IsVisible())
	{
		float flScrollable = m_rControlBounds.w - GetWidth();
		m_rControlOffset.x = -GetHorizontalScrollBar()->GetHandlePosition() * flScrollable;
	}
}

void CPanel::SetVerticalScrollBarEnabled(bool b)
{
	if (m_hVerticalScrollBar && b)
		return;

	if (!m_hVerticalScrollBar && !b)
		return;

	if (b)
	{
		CResource<CBaseControl> pScrollbar = CreateControl(new CScrollBar(false));
		m_hVerticalScrollBar = pScrollbar;
		AddControl(pScrollbar);
		Layout();
	}
	else
	{
		RemoveControl(m_hVerticalScrollBar);
	}
}

void CPanel::SetHorizontalScrollBarEnabled(bool b)
{
	if (m_hHorizontalScrollBar && b)
		return;

	if (!m_hHorizontalScrollBar && !b)
		return;

	if (b)
	{
		CResource<CBaseControl> pScrollBar = CreateControl(new CScrollBar(true));
		m_hHorizontalScrollBar = pScrollBar;
		AddControl(pScrollBar);
		Layout();
	}
	else
	{
		RemoveControl(m_hHorizontalScrollBar);
	}
}

CControl<CScrollBar> CPanel::GetVerticalScrollBar() const
{
	return m_hVerticalScrollBar;
}

CControl<CScrollBar> CPanel::GetHorizontalScrollBar() const
{
	return m_hHorizontalScrollBar;
}
