#include "checkbox.h"

#include <GL/glew.h>

#include "rootpanel.h"

using namespace glgui;

CCheckBox::CCheckBox()
	: CButton(0, 0, 10, 10, _T(""), true)
{
}

void CCheckBox::Paint(float x, float y, float w, float h)
{
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
	glEnd();

	glDisable(GL_BLEND);

	if (m_bDown)
		CRootPanel::PaintRect(x+2, y+2, w-4, h-4, g_clrBoxHi);
}
