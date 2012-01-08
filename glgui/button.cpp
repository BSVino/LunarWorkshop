#include "button.h"

#include <tinker/keys.h>

#include "rootpanel.h"

using namespace glgui;

CButton::CButton(const tstring& sText, bool bToggle, const tstring& sFont, size_t iSize)
	: CLabel(10, 10, 100, 30, sText, sFont, iSize)
{
	m_bToggle = bToggle;
	m_bToggleOn = false;
	m_bDown = false;
	m_flHighlightGoal = m_flHighlight = 0;
	m_pClickListener = NULL;
	m_pfnClickCallback = NULL;
	m_pUnclickListener = NULL;
	m_pfnUnclickCallback = NULL;
	m_clrButton = g_clrBox;
	m_clrDown = g_clrBoxHi;
}

CButton::CButton(float x, float y, float w, float h, const tstring& sText, bool bToggle, const tstring& sFont, size_t iSize)
	: CLabel(x, y, w, h, sText, sFont, iSize)
{
	m_bToggle = bToggle;
	m_bToggleOn = false;
	m_bDown = false;
	m_flHighlightGoal = m_flHighlight = 0;
	m_pClickListener = NULL;
	m_pfnClickCallback = NULL;
	m_pUnclickListener = NULL;
	m_pfnUnclickCallback = NULL;
	m_clrButton = g_clrBox;
	m_clrDown = g_clrBoxHi;
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
			m_pfnUnclickCallback(m_pUnclickListener, "");
	}
	else
	{
		if (bRegister && m_pClickListener && m_pfnClickCallback)
			m_pfnClickCallback(m_pClickListener, "");
	}
}

void CButton::SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pClickListener = pListener;
	m_pfnClickCallback = pfnCallback;
}

void CButton::SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pUnclickListener = pListener;
	m_pfnUnclickCallback = pfnCallback;
}

bool CButton::MousePressed(int code, int mx, int my)
{
	if (!IsVisible())
		return CLabel::MousePressed(code, mx, my);

	bool bUsed = false;
	if (code == TINKER_KEY_MOUSE_LEFT)
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
	if (code == TINKER_KEY_MOUSE_LEFT)
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

void CButton::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	PaintButton(x, y, w, h);

	// Now paint the text which appears on the button.
	CLabel::Paint(x, y, w, h);
}

void CButton::PaintButton(float x, float y, float w, float h)
{
	if (m_bDown)
	{
		CRootPanel::PaintRect(x, y, w, h, m_clrDown, 3);
	}
	else
	{
		Color clrBox = m_clrButton;
		if (m_bEnabled)
			clrBox.SetAlpha((int)RemapVal(m_flHighlight, 0, 1, 125, 255));
		CRootPanel::PaintRect(x, y, w, h, clrBox, 2, m_bEnabled && m_flHighlightGoal > 1);
	}
}
