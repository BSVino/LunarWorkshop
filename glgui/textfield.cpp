#include "textfield.h"

#include <GL/glew.h>
#include <FTGL/ftgl.h>

#include <platform.h>
#include <tinker/keys.h>

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

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	Color FGColor = m_FGColor;
	if (!m_bEnabled)
		FGColor.SetColor(m_FGColor.r()/2, m_FGColor.g()/2, m_FGColor.b()/2, m_iAlpha);

	glColor4ubv(FGColor);

	FTFont* pFont = CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize);

	DrawLine(m_sText.c_str(), (unsigned int)m_sText.length(), x+4, y, w-8, h);

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ubv(Color(200, 200, 200, 255));

	glMaterialfv(GL_FRONT, GL_AMBIENT, Vector(0.0f, 0.0f, 0.0f));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Vector(1.0f, 1.0f, 1.0f));
	glMaterialfv(GL_FRONT, GL_SPECULAR, Vector(0.2f, 0.2f, 0.3f));
	glMaterialfv(GL_FRONT, GL_EMISSION, Vector(0.0f, 0.0f, 0.0f));
	glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);

	glLineWidth(1);

	glBegin(GL_LINES);
		// Bottom line
		glNormal3f(-0.707106781f, 0.707106781f, 0);
		glVertex2f(x, y);
		glNormal3f(0.707106781f, 0.707106781f, 0);
		glVertex2f(x+w-1, y);

		// Top line
		glNormal3f(-0.707106781f, -0.707106781f, 0);
		glVertex2f(x, y+h-1);
		glNormal3f(0.707106781f, -0.707106781f, 0);
		glVertex2f(x+w-1, y+h-1);

		// Left line
		glNormal3f(-0.707106781f, 0.707106781f, 0);
		glVertex2f(x, y+1);
		glNormal3f(-0.707106781f, -0.707106781f, 0);
		glVertex2f(x, y+h-1);

		// Right line
		glNormal3f(0.707106781f, 0.707106781f, 0);
		glVertex2f(x+w, y+1);
		glNormal3f(0.707106781f, -0.707106781f, 0);
		glVertex2f(x+w, y+h-1);

		float flCursor = CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), m_iCursor);
		if (HasFocus() && (fmod(CRootPanel::Get()->GetTime() - m_flBlinkTime, 1) < 0.5f))
		{
			glNormal3f(0.707106781f, 0.707106781f, 0);
			glVertex2f(x + 4 + flCursor + m_flRenderOffset, y+5);
			glNormal3f(0.707106781f, -0.707106781f, 0);
			glVertex2f(x + 4 + flCursor + m_flRenderOffset, y+h-5);
		}

	glEnd();

	glDisable(GL_BLEND);

	glPopAttrib();
}

void CTextField::DrawLine(const tchar* pszText, unsigned iLength, float x, float y, float w, float h)
{
	FTFont* pFont = CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize);

	float lw = pFont->Advance(convertstring<tchar, FTGLchar>(pszText).c_str(), iLength);
	float t = pFont->LineHeight();

	float th = GetTextHeight() - t;

	float flBaseline = (float)pFont->FaceSize()/2 + pFont->Descender()/2;

	Vector vecPosition = Vector((float)x + m_flRenderOffset, (float)y + flBaseline + h/2 - th/2, 0);

	// FWTextureFont code. Spurious calls to CreateTexture (deep in FW) during Advance() cause unacceptable slowdowns
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, CRootPanel::Get()->GetRight(), 0, CRootPanel::Get()->GetBottom(), -1, 1);

	glMatrixMode(GL_MODELVIEW);

	float cx, cy;
	GetAbsPos(cx, cy);
	glScissor((int)cx+4, 0, (int)GetWidth()-8, 1000);
	glEnable(GL_SCISSOR_TEST);
	pFont->Render(convertstring<tchar, FTGLchar>(pszText).c_str(), iLength, FTPoint(vecPosition.x, CRootPanel::Get()->GetBottom()-vecPosition.y));
	glDisable(GL_SCISSOR_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
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
			float flText = CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), i);
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
		m_pfnContentsChangedCallback(m_pContentsChangedListener);
}

void CTextField::FindRenderOffset()
{
	float cx, cy;
	GetAbsPos(cx, cy);

	float flTextWidth = CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str());
	float flCursorOffset = CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), m_iCursor);

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
	if (!CLabel::GetFont(_T("sans-serif"), iSize))
		CLabel::AddFontSize(_T("sans-serif"), iSize);

	m_iFontFaceSize = iSize;
}

float CTextField::GetTextWidth()
{
	return CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str());
}

float CTextField::GetTextHeight()
{
	return CLabel::GetFont(_T("sans-serif"), m_iFontFaceSize)->LineHeight();
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
