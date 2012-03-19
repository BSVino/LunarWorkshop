#include "textfield.h"

#include <GL3/gl3w.h>
#include <FTGL/ftgl.h>

#include <tinker_platform.h>
#include <files.h>

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

	m_iAutoComplete = -1;
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

	if (m_sText.length() && m_iAutoComplete >= 0 && m_asAutoCompleteCommands.size() && HasFocus())
	{
		int iAutoComplete = m_iAutoComplete % m_asAutoCompleteCommands.size();
		tstring sCommand = m_asAutoCompleteCommands[iAutoComplete];

		TAssertNoMsg(sCommand.compare(0, m_sText.length(), m_sText) == 0);

		sCommand = sCommand.substr(m_sText.length());

		Color clrOld = m_FGColor;
		m_FGColor /= 2;

		float flOriginalTextWidth = CLabel::GetTextWidth(m_sText, m_sText.length(), "sans-serif", m_iFontFaceSize);
		DrawLine(sCommand.c_str(), sCommand.length(), x+4+flOriginalTextWidth, y, w-8-flOriginalTextWidth, h);

		m_FGColor = clrOld;
	}
}

void CTextField::PostPaint()
{
	tstring sInput = GetText();
	if (sInput.length() && m_asAutoCompleteCommands.size() && HasFocus())
	{
		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		size_t iCommandsToShow = m_asAutoCompleteCommands.size();
		bool bAbbreviated = false;

		if (iCommandsToShow > 5)
		{
			iCommandsToShow = 5;
			bAbbreviated = true;
		}

		if (bAbbreviated)
			glgui::CRootPanel::PaintRect(x+5, y+h+2, w, (float)(iCommandsToShow+1)*13+3, Color(0, 0, 0, 200), 0, true);
		else
			glgui::CRootPanel::PaintRect(x+5, y+h+2, w, (float)iCommandsToShow*13+3, Color(0, 0, 0, 200), 0, true);

		int iCommandsToSkip = 0;
		if (m_iAutoComplete >= 0 && m_asAutoCompleteCommands.size())
		{
			int iAutoComplete = m_iAutoComplete % m_asAutoCompleteCommands.size();

			if (iAutoComplete < 5)
				glgui::CRootPanel::PaintRect(x+5, y+h+2 + 13*iAutoComplete, w, 13+3, Color(100, 100, 100, 200), 2);
			else
			{
				glgui::CRootPanel::PaintRect(x+5, y+h+2 + 13*4, w, 13+3, Color(100, 100, 100, 200), 2);
				iCommandsToSkip = iAutoComplete - 4;
			}

			if (iAutoComplete == m_asAutoCompleteCommands.size()-1)
				bAbbreviated = false;
		}

		int iCommandsPainted = 0;
		for (size_t i = iCommandsToSkip; i < iCommandsToShow+iCommandsToSkip; i++)
			glgui::CLabel::PaintText(m_asAutoCompleteCommands[i], m_asAutoCompleteCommands[i].length(), "sans-serif", 13, (float)(x + 5), (float)(y + h + iCommandsPainted++*13));

		if (bAbbreviated)
		{
			tstring sDotDotDot = "...";
			glgui::CLabel::PaintText(sDotDotDot, sDotDotDot.length(), "sans-serif", 13, (float)(x + 5), (float)(y + h + iCommandsPainted++*13));
		}
	}
}

void CTextField::DrawLine(const tchar* pszText, unsigned iLength, float x, float y, float w, float h)
{
	if (!iLength)
		return;

	FTFont* pFont = CLabel::GetFont("sans-serif", m_iFontFaceSize);

	float flMargin = (h-pFont->LineHeight())/2;
	Vector vecPosition = Vector((float)x + m_flRenderOffset, (float)CRootPanel::Get()->GetBottom() - y + flMargin - pFont->LineHeight(), 0);

	Matrix4x4 mFontProjection = Matrix4x4::ProjectOrthographic(0, CRootPanel::Get()->GetWidth(), 0, CRootPanel::Get()->GetHeight(), -1, 1);

	float cx, cy;
	GetAbsPos(cx, cy);

	FRect r(-1, -1, -1, -1);
	CPanel* pParent = dynamic_cast<CPanel*>(GetParent());
	while (pParent)
	{
		if (pParent && pParent->IsScissoring())
		{
			pParent->GetAbsPos(r.x, r.y);
			r.w = pParent->GetWidth();
			r.h = pParent->GetHeight();
			break;
		}
		pParent = dynamic_cast<CPanel*>(pParent->GetParent());
	}

	FRect r2;
	GetAbsPos(r2.x, r2.y);
	r2.w = GetWidth();
	r2.h = GetHeight();

	r2.x += 4;
	r2.y += 4;
	r2.w -= 8;
	r2.h -= 8;

	if (r.x < 0)
	{
		r = r2;
	}
	else
	{
		if (!r.Union(r2))
			r = FRect(-1, -1, -1, -1);
	}

	Vector4D vecStencil(&r.x);
	vecStencil.y = CRootPanel::Get()->GetBottom()-vecStencil.y-vecStencil.w;

	::CRenderingContext c(nullptr, true);
	c.SetBlend(BLEND_ALPHA);
	c.UseProgram("text");
	c.SetUniform("bScissor", true);
	c.SetUniform("vecScissor", vecStencil);
	c.SetUniform("vecColor", m_FGColor);
	c.SetProjection(mFontProjection);
	c.Translate(vecPosition);

	c.RenderText(pszText, iLength, "sans-serif", m_iFontFaceSize);
}

bool CTextField::TakesFocus()
{
	return IsVisible();
}

bool CTextField::SetFocus(bool bFocus)
{
	CBaseControl::SetFocus(bFocus);

	if (!TakesFocus())
		return false;

	m_iAutoComplete = -1;

	if (bFocus)
	{
		m_flBlinkTime = CRootPanel::Get()->GetTime();

		m_iCursor = m_sText.length();

		return true;
	}

	return false;
}

bool CTextField::MousePressed(int iButton, int mx, int my)
{
	if (!TakesFocus())
		return false;

	CRootPanel::Get()->SetFocus(this);

	float cx, cy;
	GetAbsPos(cx, cy);

	float flCursor = (float)(mx-cx);
	for (size_t i = 1; i < m_sText.length(); i++)
	{
		float flText = CLabel::GetFont("sans-serif", m_iFontFaceSize)->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str(), i);
		if (flCursor < flText)
		{
			m_iCursor = i-1;
			return true;
		}
	}

	m_iCursor = m_sText.length();

	return true;
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
	if (m_sText.length() && m_iAutoComplete >= 0 && m_asAutoCompleteCommands.size())
	{
		if (iKey == TINKER_KEY_TAB || iKey == TINKER_KEY_DOWN)
		{
			m_iAutoComplete++;
		}
		else if (iKey == TINKER_KEY_UP)
		{
			m_iAutoComplete--;
		}
		else if (iKey == TINKER_KEY_BACKSPACE || iKey == TINKER_KEY_LEFT || iKey == TINKER_KEY_RIGHT || iKey == TINKER_KEY_DEL || iKey == TINKER_KEY_HOME || iKey == TINKER_KEY_END)
		{
			// Let the text field handle it.
			m_iAutoComplete = -1;
		}
		else
		{
			tstring sInput = GetText();
			if (sInput.length())
			{
				SetText(m_asAutoCompleteCommands[m_iAutoComplete % m_asAutoCompleteCommands.size()]);
				SetCursorPosition(-1);
				UpdateContentsChangedListener();
			}

			m_iAutoComplete = -1;
		}

		return true;
	}
	else if (iKey == TINKER_KEY_ESCAPE || iKey == TINKER_KEY_ENTER)
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
	else if (m_sText.length() && m_asAutoCompleteCommands.size() && (iKey == TINKER_KEY_TAB || iKey == TINKER_KEY_DOWN))
	{
		m_iAutoComplete++;
		return true;
	}
	else if (iKey == TINKER_KEY_UP)
	{
		m_iAutoComplete--;
		return true;
	}

	m_flBlinkTime = CRootPanel::Get()->GetTime();

	FindRenderOffset();

	return iKey != TINKER_KEY_TAB;	// Let the panel handle tab.
}

void CTextField::SetContentsChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs)
{
	m_pfnContentsChangedCallback = pfnCallback;
	m_pContentsChangedListener = pListener;
	m_sContentsChangedArgs = sArgs;
}

void CTextField::UpdateContentsChangedListener()
{
	if (m_pfnContentsChangedCallback)
		m_pfnContentsChangedCallback(m_pContentsChangedListener, m_sContentsChangedArgs);
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

void CTextField::SetAutoCompleteCommands(const eastl::vector<tstring>& asCommands)
{
	m_asAutoCompleteCommands.clear();

	for (size_t i = 0; i < asCommands.size(); i++)
	{
		if (asCommands[i] == m_sText)
			continue;

		if (asCommands[i].compare(0, m_sText.length(), m_sText) == 0)
			m_asAutoCompleteCommands.push_back(asCommands[i]);
	}

	m_iAutoComplete = -1;
}

void CTextField::SetAutoCompleteFiles(const tstring& sBaseDirectory, const eastl::vector<tstring>& asExtensions, const eastl::vector<tstring>& asExtensionsExclude)
{
	tstring sSourceFolder = FindAbsolutePath(sBaseDirectory);
	tstring sInputFolder = FindAbsolutePath(sBaseDirectory + "/" + GetText());

	if (sInputFolder.compare(0, sSourceFolder.length(), sSourceFolder) != 0)
		return;

	tstring sSearchDirectory = GetDirectory(sInputFolder);

	tstring sPrefix = ToForwardSlashes(sSearchDirectory.substr(sSourceFolder.length()));
	while (sPrefix[0] == '/')
		sPrefix = sPrefix.substr(1);
	while (sPrefix.back() == '/')
		sPrefix = sPrefix.substr(0, sPrefix.length()-2);
	if (sPrefix.length())
		sPrefix = sPrefix + '/';

	eastl::vector<tstring> asFiles = ListDirectory(sSearchDirectory);
	eastl::vector<tstring> asCompletions;

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		if (!IsDirectory(sSearchDirectory + '/' + asFiles[i]))
		{
			if (asExtensions.size())
			{
				bool bFound = false;
				for (size_t j = 0; j < asExtensions.size(); j++)
				{
					if (asFiles[i].length() <= asExtensions[j].length())
						continue;

					if (asFiles[i].substr(asFiles[i].length()-asExtensions[j].length()) != asExtensions[j])
						continue;

					bFound = true;
					break;
				}

				if (!bFound)
					continue;
			}

			if (asExtensionsExclude.size())
			{
				bool bFound = false;
				for (size_t j = 0; j < asExtensionsExclude.size(); j++)
				{
					if (asFiles[i].length() <= asExtensionsExclude[j].length())
						continue;

					if (asFiles[i].substr(asFiles[i].length()-asExtensionsExclude[j].length()) != asExtensionsExclude[j])
						continue;

					bFound = true;
					break;
				}

				if (bFound)
					continue;
			}
		}

		asCompletions.push_back(sPrefix + asFiles[i]);
	}

	SetAutoCompleteCommands(asCompletions);
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
