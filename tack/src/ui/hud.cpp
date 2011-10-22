#include "hud.h"

#include <tengine/game/gameserver.h>
#include <tengine/renderer/renderer.h>

#include "../characters/tack_character.h"
#include "../tack_game.h"

CTackHUD::CTackHUD()
	: glgui::CPanel(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight())
{
}

void CTackHUD::Paint(int x, int y, int w, int h)
{
	glgui::CBaseControl::PaintRect(95, 15, 50, 55, Color(50, 50, 50, 150));

	glgui::CLabel::PaintText("Tack", 4, "sans-serif", 18, 100, 40);

	CTackCharacter* pTack = TackGame()->GetLocalPlayerCharacter();
	tstring sHealth = sprintf("%d%%", (int)(pTack->GetHealth()*100/pTack->GetTotalHealth()));

	glgui::CLabel::PaintText(sHealth, sHealth.length(), "sans-serif", 18, 100, 60);
}
