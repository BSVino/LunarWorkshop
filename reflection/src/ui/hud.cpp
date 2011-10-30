#include "hud.h"

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>

#include "../reflection_character.h"
#include "../reflection_playercharacter.h"
#include "../reflection_game.h"

CReflectionHUD::CReflectionHUD()
	: glgui::CPanel(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight())
{
}

void CReflectionHUD::Paint(int x, int y, int w, int h)
{
}
