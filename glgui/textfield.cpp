#include "textfield.h"

#include <GL3/gl3w.h>
#include <FTGL/ftgl.h>

#include <tinker_platform.h>
#include <tinker/keys.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>

#include "label.h"
#include "rootpanel.h"

using namespace glgui;

typedef char FTGLchar;

CTextField::CTextField()
	: CBaseControl(0, 0, 140, 30)
{
	m_bEnabled = true;
	m_FGColor = Color(255, 255, 255, 255);

	SetFontFaceSize(13);

	SetSize(GetWidth(), GetTextHeight() + 8.0f);

	m_iCursor = 0;

	m_flBlinkTime = 0;

	m_flRenderOffset = 0;

	m_pfnContentsChangedCallback = NULL;
	m_pContentsChangedListener = NULL;
}

void CTextField::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	if (m_iAlpha == 0)
		return;

	glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 50), 3);

	float flCursor = CLabel::GetFont("sans-serif", m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), m_iCursor);
	if (HasFocus() && (fmod(CRootPanel::Get()->GetTime() - m_flBlinkTime, 1) < 0.5f))
		glgui::CRootPanel::PaintRect(x + 4 + flCursor + m_flRenderOffset, y+3, 1, h-6, Color(200, 200, 200, 255), 1);

	Color FGColor = m_FGColor;
	if (!m_bEnabled)
		FGColor.SetColor(m_FGColor.r()/2, m_FGColor.g()/2, m_FGColor.b()/2, m_iAlpha);

	CRootPanel::GetContext()->SetUniform("vecColor", m_FGColor);

	FTFont* pFont = CLabel::GetFont("sans-serif", m_iFontFaceSize);

	DrawLine(m_sText.c_str(), (unsigned int)m_sText.length(), x+4, y, w-8, h);
}

void CTextField::DrawLine(const tchar* pszText, unsigned iLength, float x, float y, float w, float h)
{
	FTFont* pFont = CLabel::GetFont("sans-serif", m_iFontFaceSize);

	float flMargin = (h-pFont->LineHeight())/2;
	Vector vecPosition = Vector((float)x + m_flRenderOffset, (float)CRootPanel::Get()->GetBottom() - y + flMargin - pFont->LineHeight(), 0);

	Matrix4x4 mFontProjection = Matrix4x4::ProjectOrthographic(0, CRootPanel::Get()->GetWidth(), 0, CRootPanel::Get()->GetHeight(), -1, 1);

	float cx, cy;
	GetAbsPos(cx, cy);

	::CRenderingContext c(nullptr, true);
	c.SetBlend(BLEND_ALPHA);
	c.UseProgram("text");
	c.SetUniform("bScissor", true);
	c.SetUniform("vecScissor", Vector4D(cx+4, 0, GetWidth()-8, 1000));
	c.SetUniform("vecColor", m_FGColor);
	c.SetProjection(mFontProjection);
	c.Translate(vecPosition);

	c.RenderText(pszText, iLength, "sans-serif", m_iFontFaceSize);
}

void CTextField::SetFocus(bool bFocus)
{
	CBaseControl::SetFocus(bFocus);

	if (bFocus)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		float cx, cy;
		GetAbsPos(cx, cy);

		float flCursor = (float)(mx-cx);
		for (size_t i = 1; i < m_sText.length(); i++)
		{
			float flText = CLabel::GetFont("sans-serif", m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), i);
			if (flCursor < flText)
			{
				m_iCursor = i-1;
				return;
			}
		}

		m_iCursor = m_sText.length();
	}
}

bool CTextField::CharPressed(int iKey)
{
	if (HasFocus())
	{
		if (iKey <= TINKER_KEY_FIRST)
		{
			m_sText.insert(m_iCursor++, 1, iKey);
			UpdateContentsChangedListener();
		}

		m_flBlinkTime = CRootPanel::Get()->GetTime();

		FindRenderOffset();

		return true;
	}

	return CBaseControl::CharPressed(iKey);
}

bool CTextField::KeyPressed(int iKey, bool bCtrlDown)
{
	if (HasFocus())
	{
		if (iKey == TINKER_KEY_ESCAPE || iKey == TINKER_KEY_ENTER)
			CRootPanel::Get()->SetFocus(NULL);
		else if (iKey == TINKER_KEY_LEFT)
		{
			if (m_iCursor > 0)
				m_iCursor--;
		}
		else if (iKey == TINKER_KEY_RIGHT)
		{
			if (m_iCursor < m_sText.length())
				m_iCursor++;
		}
		else if (iKey == TINKER_KEY_BACKSPACE)
		{
			if (m_iCursor > 0)
			{
				m_sText.erase(m_iCursor-1, 1);
				m_iCursor--;
				UpdateContentsChangedListener();
			}
		}
		else if (iKey == TINKER_KEY_DEL)
		{
			if (m_iCursor < m_sText.length())
			{
				m_sText.erase(m_iCursor, 1);
				UpdateContentsChangedListener();
			}
		}
		else if (iKey == TINKER_KEY_HOME)
		{
			m_iCursor = 0;
		}
		else if (iKey == TINKER_KEY_END)
		{
			m_iCursor = m_sText.length();
		}
		else if ((iKey == 'v' || iKey == 'V') && bCtrlDown)
		{
			tstring sClipboard = convertstring<char, tchar>(GetClipboard());
			m_sText.insert(m_sText.begin()+m_iCursor, sClipboard.begin(), sClipboard.end());
			m_iCursor += sClipboard.length();
			UpdateContentsChangedListener();
		}

		m_flBlinkTime = CRootPanel::Get()->GetTime();

		FindRenderOffset();

		return true;
	}

	return CBaseControl::KeyPressed(iKey, bCtrlDown);
}

void CTextField::SetContentsChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnContentsChangedCallback = pfnCallback;
	m_pContentsChangedListener = pListener;
}

void CTextField::UpdateContentsChangedListener()
{
	if (m_pfnContentsChangedCallback)
		m_pfnContentsChangedCallback(m_pContentsChangedListener, "");
}

void CTextField::FindRenderOffset()
{
	float cx, cy;
	GetAbsPos(cx, cy);

	float flTextWidth = CLabel::GetFont("sans-serif", m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str());
	float flCursorOffset = CLabel::GetFont("sans-serif", m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), m_iCursor);

	float flTextLeft = (cx + 4) + m_flRenderOffset;
	float flTextRight = flTextLeft + flTextWidth + m_flRenderOffset;
	float flCursorPosition = flTextLeft + flCursorOffset;

	float flLeftOverrun = (cx + 4) - flCursorPosition;
	float flRightOverrun = flCursorPosition - (cx + GetWidth() - 4);

	if (flLeftOverrun > 0)
	{
		m_flRenderOffset += (flLeftOverrun+25);
		if (m_flRenderOffset > 0)
			m_flRenderOffset = 0;
	}
	else if (flRightOverrun > 0)
		m_flRenderOffset -= flRightOverrun;
}

void CTextField::SetText(const tstring& sText)
{
	m_sText = sText;

	if (m_iCursor > m_sText.length())
		m_iCursor = m_sText.length();
}

void CTextField::AppendText(const tchar* pszText)
{
	if (!pszText)
		return;

	m_sText.append(pszText);
}

void CTextField::SetCursorPosition(size_t iPosition)
{
	m_iCursor = iPosition;

	if (m_iCursor > m_sText.length())
		m_iCursor = m_sText.length();
}

void CTextField::SetFontFaceSize(int iSize)
{
	if (!CLabel::GetFont("sans-serif", iSize))
		CLabel::AddFontSize("sans-serif", iSize);

	m_iFontFaceSize = iSize;
}

float CTextField::GetTextWidth()
{
	return CLabel::GetFont("sans-serif", m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str());
}

float CTextField::GetTextHeight()
{
	return CLabel::GetFont("sans-serif", m_iFontFaceSize)->LineHeight();
}

// Make the label tall enough for one line of text to fit inside.
void CTextField::EnsureTextFits()
{
	float w = GetTextWidth()+4;
	float h = GetTextHeight()+4;

	if (m_flH < h)
		SetSize(m_flW, h);

	if (m_flW < w)
		SetSize(w, m_flH);
}

tstring CTextField::GetText()
{
	return m_sText;
}

Color CTextField::GetFGColor()
{
	return m_FGColor;
}

void CTextField::SetFGColor(Color FGColor)
{
	m_FGColor = FGColor;
	SetAlpha(FGColor.a());
}

void CTextField::SetAlpha(int a)
{
	CBaseControl::SetAlpha(a);
	m_FGColor.SetAlpha(a);
}
