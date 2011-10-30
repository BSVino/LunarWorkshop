#include "reflection_window.h"

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

#include "reflection_player.h"
#include "reflection_playercharacter.h"
#include "reflection_renderer.h"
#include "world.h"
#include "ui/hud.h"

CReflectionWindow::CReflectionWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CReflectionWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->AddControl(m_pHUD = new CReflectionHUD());

	glgui::CRootPanel::Get()->SetLighting(false);
	glgui::CRootPanel::Get()->Layout();

	SetupReflection();

	GameServer()->SetLoading(false);

	CApplication::Get()->SetMouseCursorEnabled(false);
}

void CReflectionWindow::SetupReflection()
{
	CWorld* pWorld = GameServer()->Create<CWorld>("CWorld");

	CReflectionPlayer* pPlayer = GameServer()->Create<CReflectionPlayer>("CReflectionPlayer");
	Game()->AddPlayer(pPlayer);

	CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
	pCharacter->SetGlobalOrigin(Vector(-1, 0, -1));
	pPlayer->SetCharacter(pCharacter);
}

void CReflectionWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

CReflectionRenderer* CReflectionWindow::GetRenderer()
{
	return static_cast<CReflectionRenderer*>(GameServer()->GetRenderer());
}
