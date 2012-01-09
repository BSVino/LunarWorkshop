#include "label.h"

#include <GL3/gl3w.h>
#include <FTGL/ftgl.h>

#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>
#include <renderer/shaders.h>

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
	m_sText = "";
	m_eAlign = TA_MIDDLECENTER;
	m_clrText = Color(255, 255, 255, 255);
	m_bScissor = false;

	m_pLinkClickListener = NULL;
	m_pSectionHoverListener = NULL;

	SetFont("sans-serif", 13);

	SetText("");

	m_iPrintChars = -1;
}

CLabel::CLabel(const tstring& sText, const tstring& sFont, size_t iSize)
	: CBaseControl(0, 0, 100, 20)
{
	m_bEnabled = true;
	m_bWrap = true;
	m_b3D = false;
	m_bNeedsCompute = false;
	m_sText = "";
	m_eAlign = TA_MIDDLECENTER;
	m_clrText = Color(255, 255, 255, 255);
	m_bScissor = false;

	m_pLinkClickListener = NULL;
	m_pSectionHoverListener = NULL;

	SetFont(sFont, iSize);

	SetText(sText);

	m_iPrintChars = -1;
}

CLabel::CLabel(float x, float y, float w, float h, const tstring& sText, const tstring& sFont, size_t iSize)
	: CBaseControl(x, y, w, h)
{
	m_bEnabled = true;
	m_bWrap = true;
	m_b3D = false;
	m_bNeedsCompute = false;
	m_sText = "";
	m_eAlign = TA_MIDDLECENTER;
	m_clrText = Color(255, 255, 255, 255);
	m_bScissor = false;

	m_pLinkClickListener = NULL;
	m_pSectionHoverListener = NULL;

	SetFont(sFont, iSize);

	SetText(sText);

	m_iPrintChars = -1;
}

CVar glgui_showsections("glgui_showsections", "off");

void CLabel::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	if (m_iAlpha == 0)
		return;

	if (m_bNeedsCompute)
		ComputeLines();

	m_iCharsDrawn = 0;

	float ax, ay;
	GetAbsPos(ax, ay);

	for (size_t i = 0; i < m_aLines.size(); i++)
	{
		const CLine& oLine = m_aLines[i];
		for (size_t j = 0; j < oLine.m_aSections.size(); j++)
		{
			if (m_pSectionHoverListener || glgui_showsections.GetBool())
			{
				const CLineSection& oSection = oLine.m_aSections[j];

				if (MouseIsInside(oLine, oSection))
				{
					if (m_pSectionHoverListener)
						m_pfnSectionHoverCallback(m_pSectionHoverListener, sprintf("%d %d", i, j));

					if (glgui_showsections.GetBool())
					{
						float ox, oy;
						GetAlignmentOffset(oLine.m_flLineWidth, oLine.m_flLineHeight, oSection.m_sFont, oSection.m_iFontSize, w, h, ox, oy);

						if (Is3D())
						{
							float flHeight = s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->LineHeight();
							float flDescender = s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Descender();

							float x = oSection.m_rArea.x + ax + ox;
							float y = oSection.m_rArea.h - (oSection.m_rArea.y + ay + oy) - flHeight + flDescender;
							float w = oSection.m_rArea.w;
							float h = oSection.m_rArea.h;

							PaintRect(x, y, w, h, Color(50, 50, 50, 255));
						}
						else
						{
							CBaseControl::PaintRect(oSection.m_rArea.x + ax + ox, oSection.m_rArea.y + ay + oy, oSection.m_rArea.w, oSection.m_rArea.h);
						}
					}
				}
			}

			DrawSection(oLine, oLine.m_aSections[j], x, y, w, h);
		}
	}

	CBaseControl::Paint(x, y, w, h);
}

void CLabel::DrawSection(const CLine& l, const CLineSection& s, float x, float y, float w, float h)
{
	if (!s.m_sText.length())
	{
		m_iCharsDrawn += 1;
		return;
	}

	float ox, oy;
	GetAlignmentOffset(l.m_flLineWidth, l.m_flLineHeight, s.m_sFont, s.m_iFontSize, w, h, ox, oy);

	Vector vecPosition(x + ox + s.m_rArea.x, y + oy + s.m_rArea.y, 0);

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

	if (Is3D())
	{
		vecPosition.y = -vecPosition.y;
		PaintText3D(s.m_sText, iDrawChars, s.m_sFont, s.m_iFontSize, vecPosition, m_clrText);
	}
	else
	{
		Color clrText = m_clrText;
		if (!m_bEnabled)
			clrText.SetColor(m_clrText.r()/2, m_clrText.g()/2, m_clrText.b()/2, m_iAlpha);

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

		if (m_bScissor)
		{
			TAssert(false);	// Untested
			if (x < 0)
			{
				GetAbsPos(r.x, r.y);
				r.w = GetWidth();
				r.h = GetHeight();
			}
			else
			{
				FRect r2;

				GetAbsPos(r2.x, r2.y);
				r2.w = GetWidth();
				r2.h = GetHeight();

				if (!r.Union(r2))
					r = FRect(-1, -1, -1, -1);
			}
		}

		PaintText(s.m_sText, iDrawChars, s.m_sFont, s.m_iFontSize, vecPosition.x, vecPosition.y, clrText, r);
	}

	m_iCharsDrawn += s.m_sText.length()+1;
}

void CLabel::GetAlignmentOffset(float flLineWidth, float flLineHeight, const tstring& sFont, size_t iFontSize, float flAreaWidth, float flAreaHeight, float& x, float& y) const
{
	float lw = flLineWidth;
	float lh = flLineHeight;
	float w = flAreaWidth;
	float h = flAreaHeight;

	switch (m_eAlign)
	{
	case TA_MIDDLECENTER:
		x = w/2 - lw/2;
		y = h/2 - m_flTotalHeight/2;
		break;

	case TA_LEFTCENTER:
		x = 0;
		y = h/2 - m_flTotalHeight/2;
		break;

	case TA_RIGHTCENTER:
		TAssert(false);
		x = w - lw;
		y = h/2 - m_flTotalHeight/2;
		break;

	case TA_TOPCENTER:
		x = w/2 - lw/2;
		y = 0;
		break;

	case TA_BOTTOMCENTER:
		TAssert(false);
		x = w/2 - lw/2;
		y = h - m_flTotalHeight;
		break;

	case TA_BOTTOMLEFT:
		x = 0;
		y = h - m_flTotalHeight;
		break;

	default:
	case TA_TOPLEFT:
		TAssert(false);
		x = 0;
		y = 0;
		break;
	}
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

void CLabel::PaintText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, float x, float y, const Color& clrText, const FRect& rStencil)
{
	if (!GetFont(sFontName, iFontFaceSize))
		AddFontSize(sFontName, iFontFaceSize);

	Matrix4x4 mFontProjection = Matrix4x4::ProjectOrthographic(0, CRootPanel::Get()->GetWidth(), 0, CRootPanel::Get()->GetHeight(), -1, 1);

	float flBaseline = GetFont(sFontName, iFontFaceSize)->Ascender();

	::CRenderingContext c(nullptr, true);

	c.SetBlend(BLEND_ALPHA);
	c.UseProgram("text");
	c.SetProjection(mFontProjection);
	c.SetUniform("vecColor", clrText);
	c.Translate(Vector(x, CRootPanel::Get()->GetBottom()-y-flBaseline, 0));

	if (rStencil.x > 0)
	{
		c.SetUniform("bScissor", true);
		c.SetUniform("vecScissor", Vector4D(&rStencil.x));
	}
	else
		c.SetUniform("bScissor", false);

	c.RenderText(sText, iLength, sFontName, iFontFaceSize);
}

void CLabel::PaintText3D(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize, Vector vecPosition, const Color& clrText)
{
	::CRenderingContext c(nullptr, true);
	
	c.UseProgram("text");
	c.SetUniform("bScissor", false);
	c.SetUniform("vecColor", clrText);
	c.Translate(vecPosition);

	c.RenderText(sText, iLength, sFontName, iFontFaceSize);
}

void CLabel::SetSize(float w, float h)
{
	m_bNeedsCompute |= (GetWidth() != w) || (GetHeight() != h);

	CBaseControl::SetSize(w, h);
}

void CLabel::GetRealMousePosition(float& x, float& y)
{
	if (Is3D())
	{
		x = m_vec3DMouse.x;
		y = RemapVal(m_vec3DMouse.y, GetTop(), GetTop()+GetHeight(), GetTop()+GetHeight(), GetTop());
	}
	else
	{
		int fsmx, fsmy;
		CRootPanel::GetFullscreenMousePos(fsmx, fsmy);

		x = (float)fsmx;
		y = (float)fsmy;
	}
}

bool CLabel::MouseIsInside(const CLine& oLine, const CLineSection& oSection)
{
	float mx, my;
	GetRealMousePosition(mx, my);

	float ax, ay;
	GetAbsPos(ax, ay);

	float ox, oy;
	GetAlignmentOffset(oLine.m_flLineWidth, oLine.m_flLineHeight, oSection.m_sFont, oSection.m_iFontSize, GetWidth(), GetHeight(), ox, oy);

	float flLeft = oSection.m_rArea.x + ax + ox;
	float flTop = oSection.m_rArea.y + ay + oy;

	if (Is3D())
	{
		float flHeight = s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->LineHeight();
		float flDescender = s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Descender();
		flTop -= flHeight;
		flTop -= flDescender;

		if (flLeft < mx && flTop < my && flLeft + oSection.m_rArea.w > mx && flTop + oSection.m_rArea.h > my)
			return true;
	}
	else
	{
		if (flLeft < mx && flTop < my && flLeft + oSection.m_rArea.w > mx && flTop + oSection.m_rArea.h > my)
			return true;
	}

	return false;
}

bool CLabel::MousePressed(int code, int mx, int my)
{
	if (!m_pLinkClickListener)
		return false;

	for (size_t i = 0; i < m_aLines.size(); i++)
	{
		const CLine& oLine = m_aLines[i];
		for (size_t j = 0; j < oLine.m_aSections.size(); j++)
		{
			const CLineSection& oSection = oLine.m_aSections[j];

			if (oSection.m_sLink.length() == 0)
				continue;

			if (MouseIsInside(oLine, oSection))
			{
				m_pfnLinkClickCallback(m_pLinkClickListener, oSection.m_sLink);
				return true;
			}
		}
	}

	return false;
}

bool CLabel::MouseReleased(int code, int mx, int my)
{
	return false;
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

void CLabel::ComputeLines(float w, float h)
{
	if (w < 0)
		w = m_flW;

	if (h < 0)
		h = m_flH;

	tstring sDelimiter = "\n";

	eastl::vector<tstring> aTokens;
	explode(m_sText, aTokens, sDelimiter);

	// Remove extra empty stuff at the end, caused by newlines at the end of the input text.
	while (aTokens.size() && aTokens.back().length() == 0)
		aTokens.erase(aTokens.end()-1);

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
				PushSection(aSectionStack.back(), tstring(&sLine[iLastBreak], &sLine[iChar]));
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
				PushSection(aSectionStack.back(), tstring(&sLine[iLastBreak], &sLine[iChar]));
				iLength = 0;
				lw = 0;

				iChar += 7;

				aSectionStack.pop_back();
				oSection = aSectionStack.back();

				iLastBreak = iChar;
			}
			else if (tstrncmp(&sLine[iChar], "[link=", 6) == 0)
			{
				// We're ending a section, push our line.
				PushSection(aSectionStack.back(), tstring(&sLine[iLastBreak], &sLine[iChar]));
				iLength = 0;
				lw = 0;

				iChar += 6;

				int iLink = iChar;

				while (sLine[iChar] != ']')
					iChar++;

				tstring sLink = sLine.substr(iLink, iChar-iLink);

				iChar++;

				oSection.m_sLink = sLink;
				oSection.m_sText.clear();
				aSectionStack.push_back(oSection);
				AddFontSize(oSection.m_sFont, oSection.m_iFontSize);

				iLastBreak = iChar;
			}
			else if (tstrncmp(&sLine[iChar], "[/link]", 7) == 0)
			{
				// We're ending a section, push our line.
				PushSection(aSectionStack.back(), sLine.substr(iLastBreak, iChar-iLastBreak));
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
				if (sLine[iChar] == ' ')
					iLastSpace = iChar;
				lw += cw;
			}
			else
			{
				// Looks like we've exceeded the label width. Find the previous space, and that's our word break. Add a new line.

				int iBackup = iChar - iLastSpace;
				if (iLastSpace == iLastBreak || iLastSpace == 0 || iLength < iBackup)
					iBackup = 0;

				iChar -= iBackup;
				iLength -= iBackup;

				// We're ending a section, push our line.
				PushSection(aSectionStack.back(), tstring(&sLine[iLastBreak], &sLine[iLastBreak+iLength]));

				m_flTotalHeight += m_aLines.back().m_flLineHeight;
				m_aLines.push_back(oLine);

				iLength = 0;
				lw = 0;

				// Proceed to the end of any string of whitespace characters
				while (iChar < sLine.length() && sLine[iChar] == ' ')
					iChar++;

				iLastBreak = iLastSpace = iChar--;	// Skip over any following spaces, but leave iSource at the space 'cause it's incremented again below.
			}

			iChar++;
		}

		// Push the remainder.
		PushSection(aSectionStack.back(), tstring(&sLine[iLastBreak], &sLine[iLastBreak+iLength]));
		m_flTotalHeight += m_aLines.back().m_flLineHeight;
	}

	TAssert(aSectionStack.size() == 1);

	m_bNeedsCompute = false;
}

void CLabel::PushSection(const CLineSection& oSection, const tstring& sLine)
{
	if (sLine.length() == 0)
		return;

	CLineSection s = oSection;

	float flSectionWidth = s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->Advance(sLine.c_str());
	float flSectionHeight = s_apFonts[oSection.m_sFont][oSection.m_iFontSize]->LineHeight();

	s.m_sText = sLine;
	s.m_rArea.x = m_aLines.back().m_flLineWidth;
	s.m_rArea.y = m_flTotalHeight;
	s.m_rArea.w = flSectionWidth;
	s.m_rArea.h = flSectionHeight;

	m_aLines.back().m_aSections.push_back(s);
	m_aLines.back().m_flLineWidth += flSectionWidth;
}

tstring CLabel::GetText()
{
	return m_sText;
}

Color CLabel::GetTextColor()
{
	return m_clrText;
}

void CLabel::SetTextColor(const Color& clrText)
{
	m_clrText = clrText;
	SetAlpha(clrText.a());
}

void CLabel::SetAlpha(int a)
{
	CBaseControl::SetAlpha(a);
	m_clrText.SetAlpha(a);
}

void CLabel::SetAlpha(float a)
{
	CBaseControl::SetAlpha((int)(255*a));
	m_clrText.SetAlpha((int)(255*a));
}

void CLabel::SetScissor(bool bScissor)
{
	m_bScissor = bScissor;
}

void CLabel::SetLinkClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnLinkClickCallback = pfnCallback;
	m_pLinkClickListener = pListener;
}

void CLabel::SetSectionHoverListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	m_pfnSectionHoverCallback = pfnCallback;
	m_pSectionHoverListener = pListener;
}

::FTFont* CLabel::GetFont(const tstring& sName, size_t iSize)
{
	tstring sRealName = sName;
	if (s_apFontNames.find(sName) == s_apFontNames.end())
		sRealName = "sans-serif";

	if (s_apFontNames.find(sRealName) == s_apFontNames.end())
	{
		tstring sFont;

#ifdef _WIN32
		sFont = sprintf(tstring("%s\\Fonts\\Arial.ttf"), convertstring<char, tchar>(getenv("windir")));
#else
		sFont = "/usr/share/fonts/truetype/freefont/FreeSans.ttf");
#endif

		AddFont("sans-serif", sFont);
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
