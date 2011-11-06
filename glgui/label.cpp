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

	float flLineHeight = 0;;

	m_iCharsDrawn = 0;

	glColor4ubv(FGColor);

	for (size_t i = 0; i < m_aLines.size(); i++)
	{
		const CLine& oLine = m_aLines[i];
		for (size_t j = 0; j < oLine.m_aSections.size(); j++)
			DrawSection(oLine, oLine.m_aSections[j], x, y, w, h, flLineHeight);

		flLineHeight += oLine.m_flLineHeight;
	}

	if (m_bScissor)
		glDisable(GL_SCISSOR_TEST);

	glPopAttrib();

	CBaseControl::Paint(x, y, w, h);
}

void CLabel::DrawSection(const CLine& l, const CLineSection& s, float x, float y, float w, float h, float flLineHeight)
{
	if (!s.m_sText.length())
	{
		m_iCharsDrawn += 1;
		return;
	}

	float lw = l.m_flLineWidth;
	float lh = l.m_flLineHeight;
	float th = m_flTotalHeight - lh;

	float flBaseline = (float)s_apFonts[s.m_sFont][s.m_iFontSize]->FaceSize()/2 + s_apFonts[s.m_sFont][s.m_iFontSize]->Descender()/2;

	Vector vecPosition;

	if (m_eAlign == TA_MIDDLECENTER)
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + flBaseline + h/2 - th/2 + flLineHeight, 0);
	else if (m_eAlign == TA_LEFTCENTER)
		vecPosition = Vector((float)x, (float)y + flBaseline + h/2 - th/2 + flLineHeight, 0);
	else if (m_eAlign == TA_RIGHTCENTER)
		vecPosition = Vector((float)x + (float)w - lw, y + flBaseline + h/2 - th/2 + flLineHeight, 0);
	else if (m_eAlign == TA_TOPCENTER)
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + flBaseline + flLineHeight, 0);
	else if (m_eAlign == TA_BOTTOMCENTER)
		vecPosition = Vector((float)x + (float)w/2 - lw/2, (float)y + h - m_flTotalHeight + flLineHeight, 0);
	else if (m_eAlign == TA_BOTTOMLEFT)
		vecPosition = Vector((float)x, (float)y + h - m_flTotalHeight + flLineHeight, 0);
	else	// TA_TOPLEFT
		vecPosition = Vector((float)x, (float)y + flBaseline + flLineHeight, 0);

	vecPosition.x += s.m_flStart;

	int iDrawChars;
	if (m_iPrintChars == -1)
		iDrawChars = s.m_sText.length();
	else
	{
		if ((int)s.m_sText.length() > m_iPrintChars - m_iCharsDrawn)
			iDrawChars = m_iPrintChars - m_iCharsDrawn;
		else
			iDrawChars = s.m_sText.length();
	}

	if (Get3D())
	{
		vecPosition.y = -vecPosition.y;
		PaintText3D(s.m_sText, iDrawChars, s.m_sFont, s.m_iFontSize, vecPosition);
	}
	else
		PaintText(s.m_sText, iDrawChars, s.m_sFont, s.m_iFontSize, vecPosition.x, vecPosition.y);

	m_iCharsDrawn += s.m_sText.length()+1;
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
	m_bNeedsCompute |= (GetWidth() != w) || (GetHeight() != h);

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

	return m_flTotalHeight;
}

void CLabel::ComputeLines(float w, float h)
{
	if (w < 0)
		w = m_flW;

	if (h < 0)
		h = m_flH;

	tstring sDelimiter = "\n";

	eastl::vector<tstring> aTokens;
	explode(m_sText, aTokens, sDelimiter);

	m_aLines.clear();

	m_flTotalHeight = 0;

	CLineSection oSection;
	oSection.m_sFont = m_sFontName;
	oSection.m_iFontSize = m_iFontFaceSize;

	// This stack is so that markups can be nested.
	// ie [size=20]big[size=20]bigger[/size][/size]
	// A section is pushed on the section stack on every [size] and popped on every [/size]
	// Then it's used to push onto m_aLines.m_aSections
	eastl::vector<CLineSection> aSectionStack;
	aSectionStack.push_back(oSection);

	for (size_t i = 0; i < aTokens.size(); i++)
	{
		tstring sLine = aTokens[i];

		CLine oLine;
		oLine.m_flLineHeight = 0;
		oLine.m_flLineWidth = 0;
		m_aLines.push_back(oLine);

		// Default the line height to whatever's on the top of the section stack.
		m_aLines.back().m_flLineHeight = s_apFonts[aSectionStack.back().m_sFont][aSectionStack.back().m_iFontSize]->LineHeight();

		float lw = 0;
		unsigned int iChar = 0;
		int iLastSpace = 0, iLastBreak = 0, iLength = 0;
		while (iChar < sLine.length())
		{
			if (tstrncmp(&sLine[iChar], "[size=", 6) == 0)
			{
				// We're ending a section, push our line.
				oSection = aSectionStack.back();
				oSection.m_sText = tstring(&sLine[iLastBreak], &sLine[iChar]);
				oSection.m_flStart = m_aLines.back().m_flLineWidth;
				if (oSection.m_sText.length())
				{
					m_aLines.back().m_aSections.push_back(oSection);
					m_aLines.back().m_flLineWidth += s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Advance(oSection.m_sText.c_str());
				}
				iLength = 0;
				lw = 0;

				iChar += 6;

				int iSize = atoi(&sLine[iChar]);

				// Fast forward past the number
				while (sLine[iChar] >= '0' && sLine[iChar] <= '9')
					iChar++;

				while (sLine[iChar] != ']')
					iChar++;

				iChar++;

				oSection.m_iFontSize = iSize;
				oSection.m_sText.clear();
				aSectionStack.push_back(oSection);
				AddFontSize(oSection.m_sFont, oSection.m_iFontSize);

				iLastBreak = iChar;
			}
			else if (tstrncmp(&sLine[iChar], "[/size]", 7) == 0)
			{
				// We're ending a section, push our line.
				oSection = aSectionStack.back();
				oSection.m_sText = tstring(&sLine[iLastBreak], &sLine[iChar]);
				oSection.m_flStart = m_aLines.back().m_flLineWidth;
				m_aLines.back().m_aSections.push_back(oSection);
				m_aLines.back().m_flLineWidth += s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Advance(oSection.m_sText.c_str());
				iLength = 0;
				lw = 0;

				iChar += 7;

				aSectionStack.pop_back();
				oSection = aSectionStack.back();

				iLastBreak = iChar;
			}

			if (iChar >= sLine.length())
				break;

			if (sLine[iChar] == '\n')
			{
				m_flTotalHeight += m_aLines.back().m_flLineHeight;
				m_aLines.push_back(oLine);

				iChar++;

				continue;
			}

			CLineSection& oTopSection = aSectionStack.back();

			float lh = s_apFonts[oTopSection.m_sFont][oTopSection.m_iFontSize]->LineHeight();

			FTGLchar szChar[2];
			szChar[0] = FTGLchar(sLine[iChar]);
			szChar[1] = '\0';
			float cw = s_apFonts[oTopSection.m_sFont][oTopSection.m_iFontSize]->Advance(szChar);

			// If we make it this far then we are now adding to a block.
			if (m_aLines.back().m_flLineHeight < lh)
				m_aLines.back().m_flLineHeight = lh;

			float flTotalHeightSoFar = m_flTotalHeight + m_aLines.back().m_flLineHeight;

			// If our total line width plus this character does not exceed the label's width
			// or if this is the first character in this line
			// or if we've exceeded the total height of the label
			bool bNoWrap = (lw + cw < w || (lw == 0 && w < cw) || flTotalHeightSoFar > h);
			if (!m_bWrap)
				bNoWrap = true;
			if (w == 0)
				bNoWrap = true;

			if (bNoWrap)
			{
				// Then add this letter on to our current line.
				iLength++;
				if (sLine[iChar] == _T(' '))
					iLastSpace = iChar;
				lw += cw;
			}
			else
			{
				// Looks like we've exceeded the label width. Find the previous space, and that's our word break. Add a new line.

				int iBackup = iChar - iLastSpace;
				if (iLastSpace == iLastBreak)
					iBackup = 0;

				iChar -= iBackup;
				iLength -= iBackup;

				// We're ending a section, push our line.
				oSection = aSectionStack.back();
				oSection.m_sText = tstring(&sLine[iLastBreak], &sLine[iLastBreak+iLength]);
				oSection.m_flStart = m_aLines.back().m_flLineWidth;
				if (oSection.m_sText.length())
				{
					m_aLines.back().m_aSections.push_back(oSection);
					m_aLines.back().m_flLineWidth += s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Advance(oSection.m_sText.c_str());
				}
				m_flTotalHeight += m_aLines.back().m_flLineHeight;
				m_aLines.push_back(oLine);

				iLength = 0;
				lw = 0;

				// Proceed to the end of any string of whitespace characters
				while (iChar < sLine.length() && sLine[iChar] == _T(' '))
					iChar++;

				iLastBreak = iLastSpace = iChar--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
			}

			iChar++;
		}

		// Push the remainder.
		oSection = aSectionStack.back();
		oSection.m_sText = tstring(&sLine[iLastBreak], &sLine[iLastBreak+iLength]);
		oSection.m_flStart = m_aLines.back().m_flLineWidth;
		if (oSection.m_sText.length() || !m_aLines.back().m_aSections.size())
		{
			m_aLines.back().m_aSections.push_back(oSection);
			m_aLines.back().m_flLineWidth += s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Advance(oSection.m_sText.c_str());
		}
		m_flTotalHeight += m_aLines.back().m_flLineHeight;
	}

	TAssert(aSectionStack.size() == 1);

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
