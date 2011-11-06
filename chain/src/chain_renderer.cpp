#include "chain_renderer.h"

#include <GL/glew.h>

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>

CChainRenderer::CChainRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	FrustumOverride(Vector(0, 0, 10), Vector(0, 0, 0), 45, 0.1f, 50);
}

void CChainRenderer::LoadShaders()
{
	CShaderLibrary::AddShader("brightpass", "pass", "brightpass");
	CShaderLibrary::AddShader("model", "pass", "model");
	CShaderLibrary::AddShader("blur", "pass", "blur");
	CShaderLibrary::AddShader("text", "pass", "text");
}

CChainRenderer* ChainRenderer()
{
	return static_cast<CChainRenderer*>(GameServer()->GetRenderer());
}
