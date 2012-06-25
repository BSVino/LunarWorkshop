#include "checkbox.h"

#include <renderer/renderingcontext.h>

#include "rootpanel.h"

using namespace glgui;

CCheckBox::CCheckBox()
	: CButton(0, 0, 10, 10, "", true)
{
}

void CCheckBox::Paint(float x, float y, float w, float h)
{
	PaintRect(x, y, w-1, h-1, glgui::g_clrPanel, 3);

	if (m_bDown)
		CRootPanel::PaintRect(x+2, y+2, w-4, h-4, g_clrBoxHi);
}
