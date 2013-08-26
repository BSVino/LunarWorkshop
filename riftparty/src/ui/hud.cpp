#include "hud.h"

#include <tinker_platform.h>

#include <tengine/game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <tinker/cvar.h>
#include <tinker/keys.h>

#include "../riftparty_character.h"
#include "../riftparty_playercharacter.h"
#include "../riftparty_game.h"
#include "../riftparty_player.h"

CRiftPartyHUD::CRiftPartyHUD()
{
}

void CRiftPartyHUD::Paint(float x, float y, float w, float h)
{
	BaseClass::Paint(x, y, w, h);

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;

}
