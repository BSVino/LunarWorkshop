#include "chain_window.h"

#include <time.h>

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <mtrand.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/game.h>
#include <glgui/rootpanel.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <game/camera.h>

#include "chain_player.h"
#include "chain_playercharacter.h"
#include "chain_renderer.h"
#include "ui/hud.h"

CChainWindow::CChainWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CChainWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->AddControl(m_pHUD = new CChainHUD());

	glgui::CRootPanel::Get()->SetLighting(false);
	glgui::CRootPanel::Get()->Layout();

	SetupChain();

	GameServer()->SetLoading(false);

	CApplication::Get()->SetMouseCursorEnabled(false);
}

void CChainWindow::SetupChain()
{
	CChainPlayer* pPlayer = GameServer()->Create<CChainPlayer>("CChainPlayer");
	Game()->AddPlayer(pPlayer);

	CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
	pCharacter->SetGlobalOrigin(Vector(-1, 0, -1));
	pPlayer->SetCharacter(pCharacter);
}

void CChainWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

CChainRenderer* CChainWindow::GetRenderer()
{
	return static_cast<CChainRenderer*>(GameServer()->GetRenderer());
}
