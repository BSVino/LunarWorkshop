#include "tack_renderer.h"

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>

CTackRenderer::CTackRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
}

void CTackRenderer::LoadShaders()
{
	CShaderLibrary::AddShader("brightpass", "pass", "brightpass");
	CShaderLibrary::AddShader("model", "model", "model");
	CShaderLibrary::AddShader("blur", "pass", "blur");
}

void CTackRenderer::StartRendering()
{
	BaseClass::StartRendering();
}
