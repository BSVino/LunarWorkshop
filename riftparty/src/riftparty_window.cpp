#include "riftparty_window.h"

#include <GL3/gl3w.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/entities/game.h>
#include <glgui/rootpanel.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <game/entities/camera.h>

#include "riftparty_renderer.h"

CRiftPartyWindow::CRiftPartyWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CRiftPartyWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

CRenderer* CRiftPartyWindow::CreateRenderer()
{
	return new CRiftPartyRenderer();
}

CRiftPartyRenderer* CRiftPartyWindow::GetRenderer()
{
	return static_cast<CRiftPartyRenderer*>(GameServer()->GetRenderer());
}
