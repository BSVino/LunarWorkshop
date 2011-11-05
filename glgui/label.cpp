#include "label.h"

#include <GL/glew.h>
#include <FTGL/ftgl.h>

#include "rootpanel.h"

using namespace glgui;

typedef char FTGLchar;

eastl::map<tstring, tstring> CLabel::s_apFontNames;
eastl::map<tstring, eastl::map<size_t, class ::FTFont*> > CLabel::s_apFonts;

CLabel::CLabel(int x, int y, int w, int h, const tstring& sText, const tstring& sFont, size_t iSize)
	: CBaseControl(x, y, w, h)
{
	m_bEnabled = true;
	m_bWrap = true;
	m_sText = _T("");
	m_iTotalLines = 0;
	m_eAlign = TA_MIDDLECENTER;
	m_FGColor = Color(255, 255, 255, 255);
	m_bScissor = false;

	SetFont(sFont, iSize);

	SetText(sText);

	m_iPrintChars = -1;
}

void CLabel::Destructor()
{
	CBaseControl::Destructor();
}

void CLabel::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	if (m_iAlpha == 0)
		return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	if (m_bScissor)
	{
		int cx, cy;
		GetAbsPos(cx, cy);
		glScissor(cx, glgui::CRootPanel::Get()->GetHeight()-cy-GetHeight()-3, GetWidth(), GetHeight()+3);
		glEnable(GL_SCISSOR_TEST);
	}

	Color FGColor = m_FGColor;
	if (!m_bEnabled)
		FGColor.SetColor(m_FGColor.r()/2, m_FGColor.g()/2, m_FGColor.b()/2, m_iAlpha);

	const tchar* pszSeps = _T("\n");
	tchar* pszState;
	tchar* pszText = strdup<tchar>(m_sText.c_str());
	tchar* pszTok = strtok<tchar>(pszText, pszSeps, &pszState);
	m_iLine = 0;

	m_iCharsDrawn = 0;

	while (pszTok)
	{
		glColor4ubv(FGColor);

		float tw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(pszTok).c_str());
		float t = s_apFonts[m_sFontName][m_iFontFaceSize]->LineHeight();

		if (!m_bWrap || tw < w || w == 0 || (m_iLine+1)*t > h)
		{
			if (m_iPrintChars == -1 || pszTok - pszText < m_iPrintChars)
				DrawLine(pszTok, (unsigned int)tstrlen(pszTok), x, y, w, h);

			m_iLine++;
		}
		else
		{
			tw = 0;
			unsigned int iSource = 0;
			int iLastSpace = 0, iLastBreak = 0, iLength = 0;
			while (iSource < tstrlen(pszTok))
			{
				tchar szChar[2];
				szChar[0] = pszTok[iSource];
				szChar[1] = _T('\0');
				float cw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(szChar).c_str());
				if (tw + cw < w || (tw == 0 && w < cw) || (m_iLine+1)*t > h)
				{
					iLength++;
					if (pszTok[iSource] == _T(' '))
						iLastSpace = iSource;
					tw += cw;
				}
				else
				{
					int iBackup = iSource - iLastSpace;
					if (iLastSpace == iLastBreak)
						iBackup = 0;

					iSource -= iBackup;
					iLength -= iBackup;

					if (m_iPrintChars == -1 || pszTok + iLastBreak - pszText < m_iPrintChars)
						DrawLine(pszTok + iLastBreak, iLength, x, y, w, h);

					iLength = 0;
					tw = 0;
					while (iSource < tstrlen(pszTok) && pszTok[iSource] == _T(' '))
						iSource++;
					iLastBreak = iLastSpace = iSource--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
					m_iLine++;
				}

				iSource++;
			}

			if (m_iPrintChars == -1 || pszTok + iLastBreak - pszText < m_iPrintChars)
				DrawLine(pszTok + iLastBreak, iLength, x, y, w, h);
			m_iLine++;
		}

		pszTok = strtok<tchar>(NULL, pszSeps, &pszState);
	}

	free(pszText);

	if (m_bScissor)
		glDisable(GL_SCISSOR_TEST);

	glPopAttrib();

	CBaseControl::Paint(x, y, w, h);
}

void CLabel::DrawLine(tchar* pszText, unsigned iLength, int x, int y, int w, int h)
{
	float lw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(pszText).c_str(), iLength);
	float t = s_apFonts[m_sFontName][m_iFontFaceSize]->LineHeight();
	float th = GetTextHeight() - t;

	float flBaseline = (float)s_apFonts[m_sFontName][m_iFontFaceSize]->FaceSize()/2 + s_apFonts[m_sFontName][m_iFontFaceSize]->Descender()/2;

	Vector vecPosition;

	if (m_eAlign == TA_MIDDLECENTER)
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + flBaseline + h/2 - th/2 + m_iLine*t, 0);
	else if (m_eAlign == TA_LEFTCENTER)
		vecPosition = Vector((float)x, (float)y + flBaseline + h/2 - th/2 + m_iLine*t, 0);
	else if (m_eAlign == TA_RIGHTCENTER)
		vecPosition = Vector((float)x + (float)w - lw, y + flBaseline + h/2 - th/2 + m_iLine*t, 0);
	else if (m_eAlign == TA_TOPCENTER)
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + flBaseline + m_iLine*t, 0);
	else if (m_eAlign == TA_BOTTOMCENTER)
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + h - (m_iTotalLines-1)*t + m_iLine*t, 0);
	else if (m_eAlign == TA_BOTTOMLEFT)
		vecPosition = Vector((float)x, (float)y + h - (m_iTotalLines-1)*t + m_iLine*t, 0);
	else	// TA_TOPLEFT
		vecPosition = Vector((float)x, (float)y + flBaseline + m_iLine*t, 0);

	int iDrawChars;
	if (m_iPrintChars == -1)
		iDrawChars = iLength;
	else
	{
		if ((int)iLength > m_iPrintChars - m_iCharsDrawn)
			iDrawChars = m_iPrintChars - m_iCharsDrawn;
		else
			iDrawChars = iLength;
	}

	PaintText(pszText, iDrawChars, m_sFontName, m_iFontFaceSize, vecPosition.x, vecPosition.y);

	m_iCharsDrawn += iLength+1;
}

float CLabel::GetTextWidth(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize)
{
	if (!GetFont(sFontName, iFontFaceSize))
		AddFontSize(sFontName, iFontFaceSize);

	return s_apFonts[sFontName][iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(sText).c_str(), iLength);
}

float CLabel::GetFontHeight(const tstring& sFontName, int iFontFaceSize)
{
	if (!GetFont(sFontName, iFontFaceSize))
		AddFontSize(sFontName, iFontFaceSize);

	return s_apFonts[sFontName][iFontFaceSize]->LineHeight();
}

void CLabel::PaintText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, float x, float y)
{
	if (!GetFont(sFontName, iFontFaceSize))
		AddFontSize(sFontName, iFontFaceSize);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, CRootPanel::Get()->GetRight(), 0, CRootPanel::Get()->GetBottom(), -1, 1);

	glMatrixMode(GL_MODELVIEW);

	s_apFonts[sFontName][iFontFaceSize]->Render(convertstring<tchar, FTGLchar>(sText).c_str(), iLength, FTPoint(x, CRootPanel::Get()->GetBottom()-y));

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}

void CLabel::PaintText3D(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, Vector vecPosition)
{
	if (!GetFont(sFontName, iFontFaceSize))
		AddFontSize(sFontName, iFontFaceSize);

	s_apFonts[sFontName][iFontFaceSize]->Render(convertstring<tchar, FTGLchar>(sText).c_str(), iLength, FTPoint(vecPosition.x, vecPosition.y, vecPosition.z));
}

void CLabel::SetSize(int w, int h)
{
	CBaseControl::SetSize(w, h);
	ComputeLines();
}

void CLabel::SetText(const tstring& sText)
{
	m_sText = sText;

	ComputeLines();
}

void CLabel::AppendText(const tstring& sText)
{
	m_sText.append(sText);
}

void CLabel::SetFont(const tstring& sFontName, int iSize)
{
	m_sFontName = sFontName;
	m_iFontFaceSize = iSize;

	if (!GetFont(m_sFontName, m_iFontFaceSize))
		AddFontSize(m_sFontName, m_iFontFaceSize);
}

int CLabel::GetTextWidth()
{
	return (int)s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str());
}

float CLabel::GetTextHeight()
{
	return (s_apFonts[m_sFontName][m_iFontFaceSize]->LineHeight()) * m_iTotalLines;
}

void CLabel::ComputeLines(int w, int h)
{
	if (w == -1)
		w = m_iW;

	if (h == -1)
		h = m_iH;

	const tchar* pszSeps = _T("\n");
	tchar* pszText = strdup<tchar>(m_sText.c_str());

	// Cut off any ending line returns so that labels don't have hanging space below.
	if (pszText[tstrlen(pszText)-1] == _T('\n'))
		pszText[tstrlen(pszText)-1] = _T('\0');

	// FIXME: All this code is technically duplicated from Paint(),
	// but I can't think of a good way to reconcile them. Some kind
	// of lineating method is required or something...? We need to
	// add up all the lines as if they were being truncated during
	// printing to get the real height of all the text.
	tchar* pszState;
	tchar* pszTok = strtok<tchar>(pszText, pszSeps, &pszState);

	m_iTotalLines = 0;

	while (pszTok)
	{
		float tw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(pszTok).c_str());
		float t = s_apFonts[m_sFontName][m_iFontFaceSize]->LineHeight();

		if (!m_bWrap || tw < w || w == 0 || (m_iTotalLines+1)*t > h)
		{
			m_iTotalLines++;
		}
		else
		{
			tw = 0;
			unsigned int iSource = 0;
			int iLastSpace = 0, iLastBreak = 0, iLength = 0;
			while (iSource < tstrlen(pszTok))
			{
				tchar szChar[2];
				szChar[0] = pszTok[iSource];
				szChar[1] = _T('\0');
				float cw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(szChar).c_str());
				if (tw + cw < w || (tw == 0 && w < cw) || (m_iTotalLines+1)*t > h)
				{
					iLength++;
					if (pszTok[iSource] == _T(' '))
						iLastSpace = iSource;
					tw += cw;
				}
				else
				{
					int iBackup = iSource - iLastSpace;
					if (iLastSpace == iLastBreak)
						iBackup = 0;

					iSource -= iBackup;
					iLength -= iBackup;

					iLength = 0;
					tw = 0;
					while (iSource < tstrlen(pszTok) && pszTok[iSource] == _T(' '))
						iSource++;
					iLastBreak = iLastSpace = iSource--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
					m_iTotalLines++;
				}

				iSource++;
			}

			m_iTotalLines++;
		}

		pszTok = strtok<tchar>(NULL, pszSeps, &pszState);
	}

	free(pszText);
}

// Make the label tall enough for one line of text to fit inside.
void CLabel::EnsureTextFits()
{
	int w = GetTextWidth()+4;
	int h = (int)GetTextHeight()+4;

	if (m_iH < h)
		SetSize(m_iW, h);

	if (m_iW < w)
		SetSize(w, m_iH);
}

tstring CLabel::GetText()
{
	return m_sText;
}

Color CLabel::GetFGColor()
{
	return m_FGColor;
}

void CLabel::SetFGColor(Color FGColor)
{
	m_FGColor = FGColor;
	SetAlpha(FGColor.a());
}

void CLabel::SetAlpha(int a)
{
	CBaseControl::SetAlpha(a);
	m_FGColor.SetAlpha(a);
}

void CLabel::SetAlpha(float a)
{
	CBaseControl::SetAlpha((int)(255*a));
	m_FGColor.SetAlpha((int)(255*a));
}

void CLabel::SetScissor(bool bScissor)
{
	m_bScissor = bScissor;
}

::FTFont* CLabel::GetFont(const tstring& sName, size_t iSize)
{
	tstring sRealName = sName;
	if (s_apFontNames.find(sName) == s_apFontNames.end())
		sRealName = _T("sans-serif");

	if (s_apFontNames.find(sRealName) == s_apFontNames.end())
	{
		tstring sFont;

#ifdef _WIN32
		sFont = sprintf(tstring("%s\\Fonts\\Arial.ttf"), convertstring<char, tchar>(getenv("windir")));
#else
		sFont = _T("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
#endif

		AddFont(_T("sans-serif"), sFont);
	}

	return s_apFonts[sRealName][iSize];
}

void CLabel::AddFont(const tstring& sName, const tstring& sFile)
{
	s_apFontNames[sName] = sFile;
}

void CLabel::AddFontSize(const tstring& sName, size_t iSize)
{
	if (s_apFontNames.find(sName) == s_apFontNames.end())
		return;

	FTTextureFont* pFont = new FTTextureFont(convertstring<tchar, char>(s_apFontNames[sName]).c_str());
	pFont->FaceSize(iSize);
	s_apFonts[sName][iSize] = pFont;
}
