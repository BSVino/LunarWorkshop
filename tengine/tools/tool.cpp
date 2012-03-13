#include "tool.h"

#include <glgui/menu.h>

#include "workbench.h"

glgui::CMenu* CWorkbenchTool::GetFileMenu()
{
	return Workbench()->m_pFileMenu;
}