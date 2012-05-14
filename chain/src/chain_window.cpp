#include "chain_window.h"

#include <time.h>

#include <GL3/gl3w.h>
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
#include "story.h"
#include "ui/hud.h"

CChainWindow::CChainWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CChainWindow::OpenWindow()
{
	BaseClass::OpenWindow();

	CApplication::Get()->SetMouseCursorEnabled(true);
}

void CChainWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

CRenderer* CChainWindow::CreateRenderer()
{
	return new CChainRenderer();
}

CChainRenderer* CChainWindow::GetRenderer()
{
	return static_cast<CChainRenderer*>(GameServer()->GetRenderer());
}
