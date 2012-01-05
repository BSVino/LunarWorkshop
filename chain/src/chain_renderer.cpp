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
	FrustumOverride(Vector(0, 0, 10), Vector(0, 0, 0), 45, 0.1f, 50);

	m_oMouseoverBuffer1 = CreateFrameBuffer(m_iWidth, m_iHeight, FB_TEXTURE);
	m_oMouseoverBuffer2 = CreateFrameBuffer(m_iWidth, m_iHeight, FB_TEXTURE);
}

void CChainRenderer::LoadShaders()
{
	BaseClass::LoadShaders();

	CShaderLibrary::AddShader("brightpass", "pass", "brightpass");
	CShaderLibrary::AddShader("model", "pass", "model");
	CShaderLibrary::AddShader("blur", "pass", "blur");
	CShaderLibrary::AddShader("text", "text", "text");
	CShaderLibrary::AddShader("mouseover", "pass", "mouseover");
}

void CChainRenderer::SetupFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_oMouseoverBuffer1.m_iFB);
	glColor4f(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_oMouseoverBuffer2.m_iFB);
	glClear(GL_COLOR_BUFFER_BIT);

	BaseClass::SetupFrame();

	glColor4f(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4f(1, 1, 1, 1);
}

void CChainRenderer::RenderFullscreenBuffers()
{
	BaseClass::RenderFullscreenBuffers();

	CRenderingContext c(this);
	c.UseProgram("blur");
	glColor3f(1, 1, 0);
	glEnablei(GL_BLEND, 0);
	glBlendFunc(GL_ONE, GL_ONE);

	for (size_t i = 0; i < 5; i++)
	{
		c.SetUniform("flOffsetY", 0.0f);
		c.SetUniform("flOffsetX", 1.2f / m_oMouseoverBuffer1.m_iWidth);
		RenderFrameBufferToBuffer(&m_oMouseoverBuffer1, &m_oMouseoverBuffer2);

		glBindFramebuffer(GL_FRAMEBUFFER, m_oMouseoverBuffer1.m_iFB);
		glColor4f(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);

		c.SetUniform("flOffsetX", 0.0f);
		c.SetUniform("flOffsetY", 1.2f / m_oMouseoverBuffer1.m_iWidth);
		RenderFrameBufferToBuffer(&m_oMouseoverBuffer2, &m_oMouseoverBuffer1);

		glBindFramebuffer(GL_FRAMEBUFFER, m_oMouseoverBuffer2.m_iFB);
		glColor4f(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);
	}

	c.UseProgram("mouseover");
	RenderFrameBufferFullscreen(&m_oMouseoverBuffer1);
	glDisablei(GL_BLEND, 0);
}

CChainRenderer* ChainRenderer()
{
	return static_cast<CChainRenderer*>(GameServer()->GetRenderer());
}
