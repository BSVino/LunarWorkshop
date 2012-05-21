#include "grotto_window.h"

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/entities/game.h>
#include <glgui/rootpanel.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <game/cameramanager.h>

#include "grotto_renderer.h"

CGrottoWindow::CGrottoWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CGrottoWindow::RenderLoading()
{
}

CRenderer* CGrottoWindow::CreateRenderer()
{
	return new CGrottoRenderer();
}

CGrottoRenderer* CGrottoWindow::GetRenderer()
{
	return static_cast<CGrottoRenderer*>(GameServer()->GetRenderer());
}
