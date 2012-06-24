#include "glgui.h"

#include "basecontrol.h"

using namespace glgui;

Color glgui::g_clrPanel = Color(37, 37, 37, 255);
Color glgui::g_clrBox = Color(34, 37, 42, 255);
Color glgui::g_clrBoxHi = Color(148, 161, 181, 255);

float glgui::g_flLayoutDefault = -FLT_MAX;

CResource<CBaseControl> glgui::CreateControl(CBaseControl* pControl)
{
	return CBaseControl::CreateControl(pControl);
}
