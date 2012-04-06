#include "picturebutton.h"

#include <GL3/gl3w.h>

using namespace glgui;

CPictureButton::CPictureButton(const tstring& sText, const CTextureHandle& hTexture, bool bToggle)
	: CButton(0, 0, 32, 32, sText, bToggle)
{
	m_hTexture = hTexture;
	m_bShowBackground = true;
	m_bSheet = false;
}

void CPictureButton::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	if (m_bShowBackground)
		PaintButton(x, y, w, h);

	float flHighlight = 1;
	if (m_bEnabled)
		flHighlight = RemapVal(m_flHighlight, 0, 1, 0.8f, 1);

	if (m_bSheet)
	{
		PaintSheet(m_hTexture, x, y, w, h, m_iSX, m_iSY, m_iSW, m_iSH, m_iTW, m_iTH, Color(255,255,255,(unsigned char)(GetAlpha()*flHighlight)));
	}
	else if (m_hTexture.IsValid())
	{
		glEnablei(GL_BLEND, 0);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		PaintTexture(m_hTexture, x, y, w, h, Color(255,255,255,(unsigned char)(GetAlpha()*flHighlight)));
	}
	else
	{
		// Now paint the text which appears on the button.
		CLabel::Paint(x, y, w, h);
		return;
	}

	CBaseControl::Paint(x, y, w, h);
}

void CPictureButton::SetTexture(const CTextureHandle& hTexture)
{
	m_bSheet = false;
	m_hTexture = hTexture;
}

void CPictureButton::SetSheetTexture(const CTextureHandle& hTexture, int sx, int sy, int sw, int sh, int tw, int th)
{
	m_bSheet = true;
	m_hTexture = hTexture;
	m_iSX = sx;
	m_iSY = sy;
	m_iSW = sw;
	m_iSH = sh;
	m_iTW = tw;
	m_iTH = th;
}

void CPictureButton::SetSheetTexture(const CTextureHandle& hTexture, const Rect& rArea, int tw, int th)
{
	m_bSheet = true;
	m_hTexture = hTexture;
	m_iSX = rArea.x;
	m_iSY = rArea.y;
	m_iSW = rArea.w;
	m_iSH = rArea.h;
	m_iTW = tw;
	m_iTH = th;
}
