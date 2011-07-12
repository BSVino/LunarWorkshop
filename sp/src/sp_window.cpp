#include "sp_window.h"

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

#include "sp_player.h"
#include "sp_character.h"
#include "sp_renderer.h"
#include "planet.h"

CSPWindow::CSPWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
	m_pGeneralWindow = NULL;
}

void CSPWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->SetLighting(false);
	glgui::CRootPanel::Get()->Layout();

	SetupSP();

	GameServer()->SetLoading(false);

	CApplication::Get()->SetMouseCursorEnabled(false);
}

void CSPWindow::SetupSP()
{
	CSPPlayer* pPlayer = GameServer()->Create<CSPPlayer>("CSPPlayer");
	Game()->AddPlayer(pPlayer);

	CSPCharacter* pCharacter = GameServer()->Create<CSPCharacter>("CSPCharacter");
	pCharacter->SetOrigin(Vector(1, 100, 0));
	pCharacter->SetAngles(EAngle(-65, 0, 0));
	pPlayer->SetCharacter(pCharacter);

	CPlanet* pPlanet = GameServer()->Create<CPlanet>("CPlanet");
	pPlanet->SetRadius(63.78100f);	// Radius of Earth, 6378.1 km
	pPlanet->SetOrigin(Vector(0, 0, 0));
}

void CSPWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

void CSPWindow::DoKeyPress(int c)
{
	BaseClass::DoKeyPress(c);
}

CSPRenderer* CSPWindow::GetRenderer()
{
	return static_cast<CSPRenderer*>(GameServer()->GetRenderer());
}
