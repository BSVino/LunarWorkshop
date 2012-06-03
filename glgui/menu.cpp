#include "menu.h"

#include "rootpanel.h"

using namespace glgui;

CMenu* CRootPanel::AddMenu(const tstring& sText)
{
	if (!m_pMenuBar)
		return NULL;

	if (m_pMenuBar->GetControls().size() == 0)
		m_pMenuBar->SetVisible(true);

	CMenu* pMenu = new CMenu(sText);
	pMenu->SetWrap(false);
	m_pMenuBar->AddControl(pMenu, true);

	return pMenu;
}

CMenuBar::CMenuBar()
	: CPanel(0, 0, 1024, MENU_HEIGHT)
{
	SetVisible(false);
}

void CMenuBar::Layout( void )
{
	if (GetParent())
	{
		SetSize(GetParent()->GetWidth(), MENU_HEIGHT);
		SetPos(MENU_SPACING, MENU_SPACING);
	}

	CPanel::Layout();

	float x = 0;
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		m_apControls[i]->SetPos(x, 0);
		x += m_apControls[i]->GetWidth() + MENU_SPACING;
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

CMenu::CMenu(const tstring& sText, bool bSubmenu)
	: CButton(0, 0, 41, MENU_HEIGHT, sText, true)
{
	m_bSubmenu = bSubmenu;

	m_flHighlightGoal = m_flHighlight = m_flMenuHighlightGoal = m_flMenuHighlight = m_flMenuHeightGoal = m_flMenuHeight
		= m_flMenuSelectionHighlightGoal = m_flMenuSelectionHighlight = 0;

	m_MenuSelection = FRect(0, 0, 0, 0);
	m_MenuSelectionGoal = FRect(0, 0, 0, 0);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);

	m_pfnMenuCallback = NULL;
	m_pMenuListener = NULL;

	m_pMenu = new CSubmenuPanel(this);
	CRootPanel::Get()->AddControl(m_pMenu, true);

	m_pMenu->SetVisible(false);
}

CMenu::~CMenu()
{
	CRootPanel::Get()->RemoveControl(m_pMenu);
	delete m_pMenu;
}

void CMenu::Think()
{
	// Make a copy so that the below logic doesn't clobber CursorOut()
	float flHightlightGoal = m_flHighlightGoal;

	// If our menu is open always stay highlighted.
	if (m_pMenu->IsVisible())
		flHightlightGoal = 1;

	if (!IsVisible())
		CloseMenu();

	m_flHighlight = Approach(flHightlightGoal, m_flHighlight, (float)CRootPanel::Get()->GetFrameTime()*3);
	m_flMenuHighlight = Approach(m_flMenuHighlightGoal, m_flMenuHighlight, (float)CRootPanel::Get()->GetFrameTime()*3);
	m_flMenuHeight = Approach(m_flMenuHeightGoal, m_flMenuHeight, (float)CRootPanel::Get()->GetFrameTime()*3);
	m_pMenu->SetFakeHeight(m_flMenuHeight);

	m_pMenu->SetVisible(m_flMenuHighlight > 0 && m_flMenuHeight > 0);

	m_flMenuSelectionHighlightGoal = 0;

	for (size_t i = 0; i < m_pMenu->GetControls().size(); i++)
	{
		float cx, cy, cw, ch;
		int mx, my;
		m_pMenu->GetControls()[i]->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			m_flMenuSelectionHighlightGoal = 1;
			m_MenuSelectionGoal = FRect((float)cx, (float)cy, (float)cw, (float)ch);
			break;
		}
	}

	if (m_flMenuSelectionHighlight < 0.01f)
		m_MenuSelection = m_MenuSelectionGoal;
	else
	{
		m_MenuSelection.x = Approach(m_MenuSelectionGoal.x, m_MenuSelection.x, (float)CRootPanel::Get()->GetFrameTime()*800);
		m_MenuSelection.y = Approach(m_MenuSelectionGoal.y, m_MenuSelection.y, (float)CRootPanel::Get()->GetFrameTime()*800);
		m_MenuSelection.w = Approach(m_MenuSelectionGoal.w, m_MenuSelection.w, (float)CRootPanel::Get()->GetFrameTime()*800);
		m_MenuSelection.h = Approach(m_MenuSelectionGoal.h, m_MenuSelection.h, (float)CRootPanel::Get()->GetFrameTime()*800);
	}

	m_flMenuSelectionHighlight = Approach(m_flMenuSelectionHighlightGoal, m_flMenuSelectionHighlight, (float)CRootPanel::Get()->GetFrameTime()*3);
}

void CMenu::Layout()
{
	float iHeight = 0;
	float iWidth = 0;
	tvector<IControl*> apControls = m_pMenu->GetControls();
	for (size_t i = 0; i < apControls.size(); i++)
	{
		apControls[i]->SetPos(5, (float)(i*MENU_HEIGHT));
		iHeight += MENU_HEIGHT;
		if (apControls[i]->GetWidth()+10 > iWidth)
			iWidth = apControls[i]->GetWidth()+10;
	}

	float x, y;
	GetAbsPos(x, y);

	m_pMenu->SetSize(iWidth, iHeight);

	if (y + GetHeight() + 5 + iHeight > RootPanel()->GetHeight())
		m_pMenu->SetPos(x, y - 5 - iHeight);
	else
		m_pMenu->SetPos(x, y + 5 + GetHeight());

	m_pMenu->Layout();
}

void CMenu::Paint(float x, float y, float w, float h)
{
	if (!m_bSubmenu)
	{
		Color clrBox = m_clrButton;
		clrBox.SetAlpha((int)RemapVal(m_flHighlight, 0, 1, 125, 255));
		CRootPanel::PaintRect(x, y, w, h, clrBox, 1);
	}

	CLabel::Paint(x, y, w, h);
}

void CMenu::PostPaint()
{
	if (m_pMenu->IsVisible())
	{
		float mx, my, mw, mh;
		m_pMenu->GetAbsDimensions(mx, my, mw, mh);

		float flMenuHeight = Lerp(m_flMenuHeight, 0.6f);
		if (flMenuHeight > 0.99f)
			flMenuHeight = 0.99f;	// When it hits 1 it jerks.

		Color clrBox = g_clrBox;
		clrBox.SetAlpha((int)RemapVal(m_flMenuHighlight, 0, 1, 0, 255));
		CRootPanel::PaintRect(mx, (float)(my), mw, (float)(mh*flMenuHeight), clrBox);

		if (m_flMenuSelectionHighlight > 0)
		{
			clrBox = g_clrBoxHi;
			clrBox.SetAlpha((int)(255 * m_flMenuSelectionHighlight * flMenuHeight));
			CRootPanel::PaintRect((float)m_MenuSelection.x, (float)m_MenuSelection.y+1, (float)m_MenuSelection.w, (float)m_MenuSelection.h-2, clrBox);
		}
	}
}

void CMenu::CursorIn()
{
	m_flHighlightGoal = 1;

	CButton::CursorIn();
}

void CMenu::CursorOut()
{
	m_flHighlightGoal = 0;

	CButton::CursorOut();
}

void CMenu::SetMenuListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnMenuCallback = pfnCallback;
	m_pMenuListener = pListener;
}

void CMenu::OpenCallback(const tstring& sArgs)
{
	CRootPanel::Get()->GetMenuBar()->SetActive(this);

	if (m_pMenu->GetControls().size())
	{
		OpenMenu();
		Layout();
	}
}

void CMenu::CloseCallback(const tstring& sArgs)
{
	if (m_pMenu->GetControls().size())
		CloseMenu();
}

void CMenu::ClickedCallback(const tstring& sArgs)
{
	CRootPanel::Get()->GetMenuBar()->SetActive(NULL);

	if (m_pMenuListener)
		m_pfnMenuCallback(m_pMenuListener, sArgs);
}

void CMenu::OpenMenu()
{
	m_flMenuHighlightGoal = 1;
	m_flMenuHeightGoal = 1;
}

void CMenu::CloseMenu()
{
	m_flMenuHighlightGoal = 0;
	m_flMenuHeightGoal = 0;
	SetState(false, false);
}

void CMenu::AddSubmenu(const tstring& sTitle, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	CMenu* pMenu = new CMenu(sTitle, true);
	pMenu->SetAlign(TA_LEFTCENTER);
	pMenu->SetWrap(false);
	pMenu->EnsureTextFits();
	pMenu->SetToggleButton(false);

	if (pListener)
		pMenu->SetMenuListener(pListener, pfnCallback);

	size_t iControl = m_pMenu->AddControl(pMenu, true);

	pMenu->SetClickedListener(pMenu, Clicked, sprintf("%d " + sTitle, iControl));

	m_apEntries.push_back(pMenu);
}

void CMenu::ClearSubmenus()
{
	for (size_t i = 0; i < m_apEntries.size(); i++)
	{
		m_pMenu->RemoveControl(m_apEntries[i]);
		delete m_apEntries[i];
	}

	m_apEntries.clear();
}

size_t CMenu::GetSelectedMenu()
{
	for (size_t i = 0; i < m_apEntries.size(); i++)
	{
		float cx, cy, cw, ch;
		int mx, my;
		m_apEntries[i]->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			return i;
		}
	}

	return ~0;
}

CMenu::CSubmenuPanel::CSubmenuPanel(CMenu* pMenu)
	: CPanel(0, 0, 100, 100)
{
	m_pMenu = pMenu;
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

		float x, y;
		pControl->GetPos(x, y);

		if (y < m_flFakeHeight*GetHeight())
			m_aflControlHighlightGoal[i] = 1.0f;
		else
			m_aflControlHighlightGoal[i] = 0.0f;

		m_aflControlHighlight[i] = Approach(m_aflControlHighlightGoal[i], m_aflControlHighlight[i], (float)CRootPanel::Get()->GetFrameTime()*3);

		pControl->SetAlpha((int)(m_aflControlHighlight[i] * 255));
	}

	CPanel::Think();
}

void CMenu::CSubmenuPanel::Paint(float x, float y, float w, float h)
{
}

void CMenu::CSubmenuPanel::PostPaint()
{
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);
	BaseClass::Paint(x, y, w, h);
}

bool CMenu::CSubmenuPanel::IsVisible()
{
	return m_pMenu->IsVisible() && BaseClass::IsVisible();
}
