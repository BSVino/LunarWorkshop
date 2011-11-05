#include "picturebutton.h"

#include <GL/glew.h>

using namespace glgui;

CPictureButton::CPictureButton(const tstring& sText, size_t iTexture, bool bToggle)
	: CButton(0, 0, 32, 32, sText, bToggle)
{
	m_iTexture = iTexture;
	m_bShowBackground = true;
	m_bSheet = false;
}

void CPictureButton::Paint(int x, int y, int w, int h)
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
		glPushAttrib(GL_ENABLE_BIT);

		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iTexture);

		glColor4f(1,1,1,(float)GetAlpha()*flHighlight/255);
		glBegin(GL_QUADS);
			glTexCoord2f((float)m_iSX/m_iTW, 1-(float)m_iSY/m_iTH);
			glVertex2d(x, y);
			glTexCoord2f((float)m_iSX/m_iTW, 1-((float)m_iSY+m_iSH)/m_iTH);
			glVertex2d(x, y+h);
			glTexCoord2f(((float)m_iSX+m_iSW)/m_iTW, 1-((float)m_iSY+m_iSH)/m_iTH);
			glVertex2d(x+w, y+h);
			glTexCoord2f(((float)m_iSX+m_iSW)/m_iTW, 1-(float)m_iSY/m_iTH);
			glVertex2d(x+w, y);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		glPopAttrib();
	}
	else if (m_iTexture)
	{
		glPushAttrib(GL_ENABLE_BIT);

		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iTexture);
		glColor4f(1,1,1,(float)GetAlpha()*flHighlight/255);
		glBegin(GL_QUADS);
			glTexCoord2f(0, 1);
			glVertex2d(x, y);
			glTexCoord2f(0, 0);
			glVertex2d(x, y+h);
			glTexCoord2f(1, 0);
			glVertex2d(x+w, y+h);
			glTexCoord2f(1, 1);
			glVertex2d(x+w, y);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		glPopAttrib();
	}
	else
	{
		// Now paint the text which appears on the button.
		CLabel::Paint(x, y, w, h);
		return;
	}

	CBaseControl::Paint(x, y, w, h);
}

void CPictureButton::SetTexture(size_t iTexture)
{
	m_bSheet = false;
	m_iTexture = iTexture;
}

void CPictureButton::SetSheetTexture(size_t iSheet, int sx, int sy, int sw, int sh, int tw, int th)
{
	m_bSheet = true;
	m_iTexture = iSheet;
	m_iSX = sx;
	m_iSY = sy;
	m_iSW = sw;
	m_iSH = sh;
	m_iTW = tw;
	m_iTH = th;
}

void CPictureButton::SetSheetTexture(size_t iSheet, const Rect& rArea, int tw, int th)
{
	m_bSheet = true;
	m_iTexture = iSheet;
	m_iSX = rArea.x;
	m_iSY = rArea.y;
	m_iSW = rArea.w;
	m_iSH = rArea.h;
	m_iTW = tw;
	m_iTH = th;
}
