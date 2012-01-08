#include "chain_renderer.h"

#include <GL/glew.h>

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>

CChainRenderer::CChainRenderer()
	: CGameRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_oMouseoverBuffer1 = CreateFrameBuffer(m_iWidth, m_iHeight, FB_TEXTURE);
	m_oMouseoverBuffer2 = CreateFrameBuffer(m_iWidth, m_iHeight, FB_TEXTURE);
}

void CChainRenderer::LoadShaders()
{
	BaseClass::LoadShaders();

	CShaderLibrary::AddShader("model", "pass", "model");
	CShaderLibrary::AddShader("mouseover", "pass", "mouseover");
}

void CChainRenderer::SetupFrame(class CRenderingContext* pContext)
{
	CRenderingContext c;
	c.UseFrameBuffer(&m_oMouseoverBuffer1);
	c.ClearColor();

	c.UseFrameBuffer(&m_oMouseoverBuffer2);
	c.ClearColor();

	m_bDrawBackground = true;

	c.UseFrameBuffer(&m_oSceneBuffer);

	FrustumOverride(m_vecCameraPosition, m_vecCameraDirection, m_flCameraFOV, m_flCameraNear, m_flCameraFar);

	BaseClass::SetupFrame(pContext);
}

void CChainRenderer::RenderFullscreenBuffers(class CRenderingContext* pContext)
{
	BaseClass::RenderFullscreenBuffers(pContext);

	CRenderingContext c(this);
	c.UseProgram("blur");
	c.SetUniform("vecColor", Color(255, 255, 0));
	c.SetBlend(BLEND_ADDITIVE);

	for (size_t i = 0; i < 5; i++)
	{
		c.SetUniform("flOffsetY", 0.0f);
		c.SetUniform("flOffsetX", 1.2f / m_oMouseoverBuffer1.m_iWidth);
		RenderFrameBufferToBuffer(&m_oMouseoverBuffer1, &m_oMouseoverBuffer2);

		c.UseFrameBuffer(&m_oMouseoverBuffer1);
		c.ClearColor();

		c.SetUniform("flOffsetX", 0.0f);
		c.SetUniform("flOffsetY", 1.2f / m_oMouseoverBuffer1.m_iWidth);
		RenderFrameBufferToBuffer(&m_oMouseoverBuffer2, &m_oMouseoverBuffer1);

		c.UseFrameBuffer(&m_oMouseoverBuffer2);
		c.ClearColor();
	}

	c.UseProgram("mouseover");
	RenderFrameBufferFullscreen(&m_oMouseoverBuffer1);
}

CChainRenderer* ChainRenderer()
{
	return static_cast<CChainRenderer*>(GameServer()->GetRenderer());
}
