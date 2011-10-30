#include "reflection_renderer.h"

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>

CReflectionRenderer::CReflectionRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
}

void CReflectionRenderer::LoadShaders()
{
	CShaderLibrary::AddShader("brightpass", "pass", "brightpass");
	CShaderLibrary::AddShader("model", "pass", "model");
	CShaderLibrary::AddShader("blur", "pass", "blur");
}

void CReflectionRenderer::StartRendering()
{
	BaseClass::StartRendering();
}
