#include "reflection_window.h"

#include <GL3/gl3w.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/game.h>
#include <glgui/rootpanel.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <game/camera.h>

#include "reflection_renderer.h"

CReflectionWindow::CReflectionWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
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
