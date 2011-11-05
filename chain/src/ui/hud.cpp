#include "hud.h"

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>
#include <tengine/renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>

#include "../chain_playercharacter.h"
#include "../chain_game.h"

CChainHUD::CChainHUD()
	: glgui::CPanel(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight())
{
}

void CChainHUD::Paint(float x, float y, float w, float h)
{
}
