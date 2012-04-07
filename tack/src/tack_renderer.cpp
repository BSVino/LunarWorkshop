#include "tack_renderer.h"

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>

CTackRenderer::CTackRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
}

void CTackRenderer::StartRendering()
{
	BaseClass::StartRendering();
}
