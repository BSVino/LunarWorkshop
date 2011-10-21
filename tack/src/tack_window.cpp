#include "tack_window.h"

#include <time.h>

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <mtrand.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/game.h>
#include <glgui/glgui.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <game/camera.h>

#include "tack_player.h"
#include "tack_playercharacter.h"
#include "tack_renderer.h"
#include "world.h"
#include "ui/hud.h"

CTackWindow::CTackWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CTackWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->AddControl(m_pHUD = new CTackHUD());

	glgui::CRootPanel::Get()->SetLighting(false);
	glgui::CRootPanel::Get()->Layout();

	SetupTack();

	GameServer()->SetLoading(false);
}

void CTackWindow::SetupTack()
{
	CWorld* pWorld = GameServer()->Create<CWorld>("CWorld");

	CTackPlayer* pPlayer = GameServer()->Create<CTackPlayer>("CTackPlayer");
	Game()->AddPlayer(pPlayer);

	CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
	pCharacter->SetGlobalOrigin(Vector());
	pPlayer->SetCharacter(pCharacter);
}

void CTackWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

CTackRenderer* CTackWindow::GetRenderer()
{
	return static_cast<CTackRenderer*>(GameServer()->GetRenderer());
}
