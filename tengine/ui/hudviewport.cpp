#include "hudviewport.h"

#include <glgui/rootpanel.h>

CHUDViewport::CHUDViewport()
	: glgui::CPanel(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight())
{
}
