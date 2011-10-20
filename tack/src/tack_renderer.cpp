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
	CShaderLibrary::AddShader("model", "pass", "model");
	CShaderLibrary::AddShader("blur", "pass", "blur");
}

void CTackRenderer::StartRendering()
{
	BaseClass::StartRendering();

	CRenderingContext c(this);

	c.SetColor(Color(255, 0, 0));
	c.BeginRenderDebugLines();
	c.Vertex(Vector(-1000, 0, 0));
	c.Vertex(Vector(1000, 0, 0));
	c.EndRender();

	c.BeginRenderDebugLines();
	c.Vertex(Vector(0, 0, -1000));
	c.Vertex(Vector(0, 0, 1000));
	c.EndRender();

	c.SetColor(Color(100, 100, 100));
	c.BeginRenderQuads();
	c.Vertex(Vector(-1000, 0, -1000));
	c.Vertex(Vector(-1000, 0, 1000));
	c.Vertex(Vector(1000, 0, 1000));
	c.Vertex(Vector(1000, 0, -1000));
	c.EndRender();
}
