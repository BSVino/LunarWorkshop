#include "label.h"

#include <GL/glew.h>
#include <FTGL/ftgl.h>

#include "rootpanel.h"

using namespace glgui;

typedef char FTGLchar;

eastl::map<tstring, tstring> CLabel::s_apFontNames;
eastl::map<tstring, eastl::map<size_t, class ::FTFont*> > CLabel::s_apFonts;

CLabel::CLabel()
	: CBaseControl(0, 0, 100, 30)
{
	m_bEnabled = true;
	m_bWrap = true;
	m_b3D = false;
	m_bNeedsCompute = false;
	m_sText = _T("");
	m_eAlign = TA_MIDDLECENTER;
	m_FGColor = Color(255, 255, 255, 255);
	m_bScissor = false;

	SetFont("sans-serif", 13);

	SetText("");

	m_iPrintChars = -1;
}

CLabel::CLabel(float x, float y, float w, float h, const tstring& sText, const tstring& sFont, size_t iSize)
	: CBaseControl(x, y, w, h)
{
	m_bEnabled = true;
	m_bWrap = true;
	m_b3D = false;
	m_bNeedsCompute = false;
	m_sText = _T("");
	m_eAlign = TA_MIDDLECENTER;
	m_FGColor = Color(255, 255, 255, 255);
	m_bScissor = false;

	SetFont(sFont, iSize);

	SetText(sText);

	m_iPrintChars = -1;
}

void CLabel::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	if (m_iAlpha == 0)
		return;

	if (m_bNeedsCompute)
		ComputeLines();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	if (m_bScissor)
	{
		float cx, cy;
		GetAbsPos(cx, cy);
		glScissor((int)cx, (int)(glgui::CRootPanel::Get()->GetHeight()-cy-GetHeight()-3), (int)GetWidth(), (int)GetHeight()+3);
		glEnable(GL_SCISSOR_TEST);
	}

	Color FGColor = m_FGColor;
	if (!m_bEnabled)
		FGColor.SetColor(m_FGColor.r()/2, m_FGColor.g()/2, m_FGColor.b()/2, m_iAlpha);

	m_iLine = 0;

	m_iCharsDrawn = 0;

	for (size_t i = 0; i < m_asLines.size(); i++)
	{
		glColor4ubv(FGColor);

		DrawLine(m_asLines[i].c_str(), m_asLines[i].length(), x, y, w, h);
		m_iLine++;
	}

	if (m_bScissor)
		glDisable(GL_SCISSOR_TEST);

	glPopAttrib();

	CBaseControl::Paint(x, y, w, h);
}

void CLabel::DrawLine(const tchar* pszText, unsigned iLength, float x, float y, float w, float h)
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
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + h - (m_asLines.size()-1)*t + m_iLine*t, 0);
	else if (m_eAlign == TA_BOTTOMLEFT)
		vecPosition = Vector((float)x, (float)y + h - (m_asLines.size()-1)*t + m_iLine*t, 0);
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

	if (Get3D())
	{
		vecPosition.y = -vecPosition.y;
		PaintText3D(pszText, iDrawChars, m_sFontName, m_iFontFaceSize, vecPosition);
	}
	else
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

void CLabel::SetSize(float w, float h)
{
	m_bNeedsCompute = (GetWidth() != w) || (GetHeight() != h);

	CBaseControl::SetSize(w, h);
}

void CLabel::SetText(const tstring& sText)
{
	m_sText = sText;

	m_bNeedsCompute = true;
}

void CLabel::AppendText(const tstring& sText)
{
	m_sText.append(sText);

	m_bNeedsCompute = true;
}

void CLabel::SetFont(const tstring& sFontName, int iSize)
{
	m_sFontName = sFontName;
	m_iFontFaceSize = iSize;

	if (!GetFont(m_sFontName, m_iFontFaceSize))
		AddFontSize(m_sFontName, m_iFontFaceSize);

	m_bNeedsCompute = true;
}

float CLabel::GetTextWidth()
{
	if (m_bNeedsCompute)
		ComputeLines();

	return s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(m_sText).c_str());
}

float CLabel::GetTextHeight()
{
	if (m_bNeedsCompute)
		ComputeLines();

	return (s_apFonts[m_sFontName][m_iFontFaceSize]->LineHeight()) * m_asLines.size();
}

void CLabel::ComputeLines(float w, float h)
{
	if (w < 0)
		w = m_flW;

	if (h < 0)
		h = m_flH;

	const tchar* pszSeps = _T("\n");
	tchar* pszText = strdup<tchar>(m_sText.c_str());

	// Cut off any ending line returns so that labels don't have hanging space below.
	if (pszText[tstrlen(pszText)-1] == _T('\n'))
		pszText[tstrlen(pszText)-1] = _T('\0');

	tchar* pszState;
	tchar* pszTok = strtok<tchar>(pszText, pszSeps, &pszState);

	m_asLines.clear();

	while (pszTok)
	{
		float tw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(convertstring<tchar, FTGLchar>(pszTok).c_str());
		float lh = s_apFonts[m_sFontName][m_iFontFaceSize]->LineHeight();

		if (!m_bWrap || tw < w || w == 0 || (m_asLines.size()+1)*lh > h)
		{
			m_asLines.push_back(pszTok);
		}
		else
		{
			tw = 0;
			unsigned int iSource = 0;
			int iLastSpace = 0, iLastBreak = 0, iLength = 0;
			while (iSource < tstrlen(pszTok))
			{
				FTGLchar szChar[2];
				szChar[0] = FTGLchar(pszTok[iSource]);
				szChar[1] = '\0';
				float cw = s_apFonts[m_sFontName][m_iFontFaceSize]->Advance(szChar);

				// If our total line width plus this character does not exceed the label's width
				// or if this is the first character in this line
				// or if we've exceeded the total height of the label
				if (tw + cw < w || (tw == 0 && w < cw) || (m_asLines.size()+1)*lh > h)
				{
					// Then add this letter on to our current line.
					iLength++;
					if (pszTok[iSource] == _T(' '))
						iLastSpace = iSource;
					tw += cw;
				}
				else
				{
					// Looks like we've exceeded the label width. Find the previous space, and that's our word break. Add a new line.

					int iBackup = iSource - iLastSpace;
					if (iLastSpace == iLastBreak)
						iBackup = 0;

					iSource -= iBackup;
					iLength -= iBackup;

					m_asLines.push_back(tstring(&pszTok[iLastBreak], &pszTok[iLastBreak+iLength]));

					iLength = 0;
					tw = 0;

					// Proceed to the end of any string of whitespace characters
					while (iSource < tstrlen(pszTok) && pszTok[iSource] == _T(' '))
						iSource++;

					iLastBreak = iLastSpace = iSource--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
				}

				iSource++;
			}

			m_asLines.push_back(tstring(&pszTok[iLastBreak]));
		}

		pszTok = strtok<tchar>(NULL, pszSeps, &pszState);
	}

	free(pszText);

	m_bNeedsCompute = false;
}

// Make the label tall enough for one line of text to fit inside.
void CLabel::EnsureTextFits()
{
	float w = GetTextWidth()+4;
	float h = GetTextHeight()+4;

	if (m_flH < h)
		SetSize(m_flW, h);

	if (m_flW < w)
		SetSize(w, m_flH);
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
