#include "panel.h"

using namespace glgui;

CPanel::CPanel(int x, int y, int w, int h)
	: CBaseControl(x, y, w, h)
{
	SetBorder(BT_SOME);
	m_pHasCursor = NULL;
	m_bHighlight = false;
	m_bDestructing = false;
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

	BaseClass::CursorOut();
}

IControl* CPanel::GetHasCursor()
{
	if (!m_pHasCursor)
		return this;

	return m_pHasCursor->GetHasCursor();
}

void CPanel::AddControl(IControl* pControl, bool bToTail)
{
	if (!pControl)
		return;

#ifdef _DEBUG
	for (size_t i = 0; i < m_apControls.size(); i++)
		TAssert(m_apControls[i] != pControl);	// You're adding a control to the panel twice! Quit it!
#endif

	pControl->SetParent(this);

	if (bToTail)
		m_apControls.push_back(pControl);
	else
		m_apControls.insert(m_apControls.begin(), pControl);
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
	size_t iCount = m_apControls.size();
	for (size_t i = 0; i < iCount; i++)
	{
		m_apControls[i]->Layout();
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
		IControl* pControl = m_apControls[i];
		if (!pControl->IsVisible())
			continue;

		// Translate this location to the child's local space.
		int cx, cy, ax, ay;
		pControl->GetAbsPos(cx, cy);
		GetAbsPos(ax, ay);
		pControl->Paint(cx+x-ax, cy+y-ay);
	}

	BaseClass::Paint(x, y, w, h);
}

void CPanel::PaintBorder(int x, int y, int w, int h)
{
}

void CPanel::Think()
{
	size_t iCount = m_apControls.size();
	for (size_t i = iCount-1; i < iCount; i--)
	{
		m_apControls[i]->Think();
	}
}
