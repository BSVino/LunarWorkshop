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
#include "characters/tack_playercharacter.h"
#include "tack_renderer.h"
#include "world.h"
#include "ui/hud.h"
#include "characters/zombie.h"

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
	pCharacter->SetGlobalOrigin(Vector(-2, 0, -2));
	pPlayer->SetCharacter(pCharacter);

	CZombie* pZombie = GameServer()->Create<CZombie>("CZombie");
	pZombie->SetGlobalOrigin(Vector(5, 0, 0));

	pZombie = GameServer()->Create<CZombie>("CZombie");
	pZombie->SetGlobalOrigin(Vector(6, 0, 2));

	pZombie = GameServer()->Create<CZombie>("CZombie");
	pZombie->SetGlobalOrigin(Vector(-4, 0, -2));

	pZombie = GameServer()->Create<CZombie>("CZombie");
	pZombie->SetGlobalOrigin(Vector(3, 0, 1));

	pZombie = GameServer()->Create<CZombie>("CZombie");
	pZombie->SetGlobalOrigin(Vector(-3, 0, -3));

	pZombie = GameServer()->Create<CZombie>("CZombie");
	pZombie->SetGlobalOrigin(Vector(-6, 0, 2));
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
