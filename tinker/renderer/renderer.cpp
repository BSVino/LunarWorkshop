#include "renderer.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <maths.h>
#include <tinker_platform.h>

#include <common/worklistener.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <textures/texturelibrary.h>

#include "renderingcontext.h"

CFrameBuffer::CFrameBuffer()
{
	m_iRB = m_iMap = m_iDepth = m_iFB = 0;
}

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	TMsg("Initializing renderer\n");

	if (!HardwareSupported())
	{
		TError("Hardware not supported!");
		Alert("Your hardware does not support OpenGL 3.0. Please try updating your drivers.");
		exit(1);
	}

	m_bUseMultisampleTextures = !!GLEW_ARB_texture_multisample;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetIntegerv(GL_SAMPLES, &m_iScreenSamples);

	SetSize(iWidth, iHeight);

	m_bFrustumOverride = false;
	m_bDrawBackground = true;
}

void CRenderer::Initialize()
{
	LoadShaders();
	CShaderLibrary::CompileShaders(m_iScreenSamples);

	m_oSceneBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_DEPTH|FB_MULTISAMPLE));

	size_t iWidth = m_oSceneBuffer.m_iWidth;
	size_t iHeight = m_oSceneBuffer.m_iHeight;
	for (size_t i = 0; i < BLOOM_FILTERS; i++)
	{
		m_oBloom1Buffers[i] = CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_TEXTURE|FB_LINEAR));
		m_oBloom2Buffers[i] = CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_TEXTURE));
		iWidth /= 2;
		iHeight /= 2;
	}

	if (!CShaderLibrary::IsCompiled())
	{
		TError("Shader compilation error!");
		Alert("There was a problem compiling shaders. Please send the files shaders.txt and glinfo.txt to jorge@lunarworkshop.com");
		OpenExplorer(GetAppDataDirectory(Application()->AppDirectory()));
		exit(1);
	}
}

void CRenderer::LoadShaders()
{
	CShaderLibrary::AddShader("quad", "quad", "quad");
	CShaderLibrary::AddShader("text", "text", "text");
	CShaderLibrary::AddShader("gui", "gui", "gui");
	CShaderLibrary::AddShader("brightpass", "pass", "brightpass");
	CShaderLibrary::AddShader("blur", "pass", "blur");
}

CFrameBuffer CRenderer::CreateFrameBuffer(size_t iWidth, size_t iHeight, fb_options_e eOptions)
{
	TAssert((eOptions&FB_TEXTURE) ^ (eOptions&FB_RENDERBUFFER));

	if (!(eOptions&(FB_TEXTURE|FB_RENDERBUFFER)))
		eOptions = (fb_options_e)(eOptions|FB_TEXTURE);

	if (!GLEW_ARB_texture_non_power_of_two && (eOptions&FB_TEXTURE))
	{
		// If non power of two textures are not supported, framebuffers the size of the screen will probably fuck up.
		// I don't know this for sure but I'm not taking any chances. If the extension isn't supported, roll those
		// framebuffer sizes up to the next power of two.
		iWidth--;
		iWidth |= iWidth >> 1;
		iWidth |= iWidth >> 2;
		iWidth |= iWidth >> 4;
		iWidth |= iWidth >> 8;
		iWidth |= iWidth >> 16;
		iWidth++;

		iHeight--;
		iHeight |= iHeight >> 1;
		iHeight |= iHeight >> 2;
		iHeight |= iHeight >> 4;
		iHeight |= iHeight >> 8;
		iHeight |= iHeight >> 16;
		iHeight++;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetIntegerv(GL_SAMPLES, &m_iScreenSamples);
	GLsizei iSamples = m_iScreenSamples;

	bool bUseMultisample = !!GLEW_EXT_framebuffer_multisample;
	if (iSamples == 0)
		bUseMultisample = false;
	if (!(eOptions&FB_MULTISAMPLE))
		bUseMultisample = false;

	bool bMultisampleTexture = m_bUseMultisampleTextures;

	if ((eOptions&FB_TEXTURE) && !bMultisampleTexture)
		bUseMultisample = false;

	GLuint iTextureTarget = GL_TEXTURE_2D;
	if (bUseMultisample)
		iTextureTarget = GL_TEXTURE_2D_MULTISAMPLE;

	CFrameBuffer oBuffer;
	oBuffer.m_bMultiSample = bUseMultisample;

	if (eOptions&FB_TEXTURE)
	{
		glGenTextures(1, &oBuffer.m_iMap);
		glBindTexture(iTextureTarget, (GLuint)oBuffer.m_iMap);
		if (bUseMultisample)
			glTexImage2DMultisample(iTextureTarget, iSamples, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, GL_FALSE);
		else
		{
			glTexParameteri(iTextureTarget, GL_TEXTURE_MIN_FILTER, (eOptions&FB_LINEAR)?GL_LINEAR:GL_NEAREST);
			glTexParameteri(iTextureTarget, GL_TEXTURE_MAG_FILTER, (eOptions&FB_LINEAR)?GL_LINEAR:GL_NEAREST);
			glTexParameteri(iTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(iTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexImage2D(iTextureTarget, 0, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		glBindTexture(iTextureTarget, 0);
	}
	else if (eOptions&FB_RENDERBUFFER)
	{
		glGenRenderbuffers(1, &oBuffer.m_iRB);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB );
		if (bUseMultisample)
			glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, iSamples, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}

	if (eOptions&FB_DEPTH)
	{
		glGenRenderbuffers(1, &oBuffer.m_iDepth);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
		if (bUseMultisample)
			glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, iSamples, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}

	glGenFramebuffers(1, &oBuffer.m_iFB);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	if (eOptions&FB_TEXTURE)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iTextureTarget, (GLuint)oBuffer.m_iMap, 0);
	else if (eOptions&FB_RENDERBUFFER)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB);
	if (eOptions&FB_DEPTH)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	TAssert(status == GL_FRAMEBUFFER_COMPLETE_EXT);

	GLint iFBSamples;
	glGetIntegerv(GL_SAMPLES, &iFBSamples);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	TAssert(iFBSamples == iSamples || iFBSamples == 0 || 0 == iSamples);

	oBuffer.m_iWidth = iWidth;
	oBuffer.m_iHeight = iHeight;

	oBuffer.m_vecTexCoords[0] = Vector2D(0, 1);
	oBuffer.m_vecTexCoords[1] = Vector2D(0, 0);
	oBuffer.m_vecTexCoords[2] = Vector2D(1, 0);
	oBuffer.m_vecTexCoords[3] = Vector2D(1, 1);

	oBuffer.m_vecVertices[0] = Vector2D(0, 0);
	oBuffer.m_vecVertices[1] = Vector2D(0, (float)iHeight);
	oBuffer.m_vecVertices[2] = Vector2D((float)iWidth, (float)iHeight);
	oBuffer.m_vecVertices[3] = Vector2D((float)iWidth, 0);

	return oBuffer;
}

void CRenderer::PreFrame()
{
}

void CRenderer::PostFrame()
{
}

void CRenderer::PreRender()
{
}

void CRenderer::PostRender()
{
}

void CRenderer::ModifyContext(class CRenderingContext* pContext)
{
	pContext->UseFrameBuffer(&m_oSceneBuffer);
}

void CRenderer::SetupFrame(class CRenderingContext* pContext)
{
	TPROF("CRenderer::SetupFrame");

	pContext->ClearDepth();

	if (m_bDrawBackground)
		DrawBackground(pContext);
}

void CRenderer::DrawBackground(class CRenderingContext* pContext)
{
	pContext->ClearColor();
}

void CRenderer::StartRendering(class CRenderingContext* pContext)
{
	TPROF("CRenderer::StartRendering");

	pContext->SetProjection(Matrix4x4::ProjectPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		));

	pContext->SetView(Matrix4x4::ConstructCameraView(m_vecCameraPosition, m_vecCameraDirection, m_vecCameraUp));

	for (size_t i = 0; i < 16; i++)
	{
		m_aflModelView[i] = ((float*)pContext->GetView())[i];
		m_aflProjection[i] = ((float*)pContext->GetProjection())[i];
	}

	if (m_bFrustumOverride)
	{
		Matrix4x4 mProjection = Matrix4x4::ProjectPerspective(
				m_flFrustumFOV,
				(float)m_iWidth/(float)m_iHeight,
				m_flFrustumNear,
				m_flFrustumFar
			);

		Matrix4x4 mView = Matrix4x4::ConstructCameraView(m_vecFrustumPosition, m_vecFrustumDirection, m_vecCameraUp);

		m_oFrustum.CreateFrom(mProjection * mView);
	}
	else
		m_oFrustum.CreateFrom(pContext->GetProjection() * pContext->GetView());

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);
}

CVar show_frustum("debug_show_frustum", "no");

void CRenderer::FinishRendering(class CRenderingContext* pContext)
{
	TPROF("CRenderer::FinishRendering");

	if (show_frustum.GetBool())
	{
		for (size_t i = 0; i < 6; i++)
		{
			Vector vecForward = m_oFrustum.p[i].n;
			Vector vecRight = vecForward.Cross(Vector(0, 1, 0)).Normalized();
			Vector vecUp = vecRight.Cross(vecForward).Normalized();
			Vector vecCenter = vecForward * m_oFrustum.p[i].d;

			vecForward *= 100;
			vecRight *= 100;
			vecUp *= 100;

			glBegin(GL_QUADS);
				glVertex3fv(vecCenter + vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp - vecRight);
				glVertex3fv(vecCenter + vecUp - vecRight);
			glEnd();
		}
	}
}

void CRenderer::FinishFrame(class CRenderingContext* pContext)
{
	pContext->SetProjection(Matrix4x4());
	pContext->SetView(Matrix4x4());

	RenderOffscreenBuffers(pContext);

	RenderFullscreenBuffers(pContext);
}

CVar r_bloom("r_bloom", "1");

void CRenderer::RenderOffscreenBuffers(class CRenderingContext* pContext)
{
	if (r_bloom.GetBool())
	{
		TPROF("Bloom");

		CRenderingContext c(this);

		// Use a bright-pass filter to catch only the bright areas of the image
		c.UseProgram("brightpass");

		c.SetUniform("iSource", 0);
		c.SetUniform("flScale", (float)1/BLOOM_FILTERS);

		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			c.SetUniform("flBrightness", BloomBrightnessCutoff() - 0.1f*i);
			RenderFrameBufferToBuffer(&m_oSceneBuffer, &m_oBloom1Buffers[i]);
		}

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);
	}
}

CVar r_bloom_buffer("r_bloom_buffer", "-1");

void CRenderer::RenderFullscreenBuffers(class CRenderingContext* pContext)
{
	TPROF("CRenderer::RenderFullscreenBuffers");

	RenderFrameBufferFullscreen(&m_oSceneBuffer);

	if (r_bloom.GetBool())
	{
		if (r_bloom_buffer.GetInt() >= 0 && r_bloom_buffer.GetInt() < BLOOM_FILTERS)
		{
			CRenderingContext c(this);
			RenderFrameBufferFullscreen(&m_oBloom1Buffers[r_bloom_buffer.GetInt()]);
		}
		else
		{
			CRenderingContext c(this);
			c.SetBlend(BLEND_ADDITIVE);
			for (size_t i = 0; i < BLOOM_FILTERS; i++)
				RenderFrameBufferFullscreen(&m_oBloom1Buffers[i]);
		}
	}
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	CRenderingContext c(this);

	c.UseProgram("blur");

	c.SetUniform("iSource", 0);
	c.SetUniform("aflCoefficients", KERNEL_SIZE, &aflKernel[0]);
	c.SetUniform("flOffsetX", 0.0f);
	c.SetUniform("flOffsetY", 0.0f);

    // Perform the blurring.
    for (size_t i = 0; i < BLOOM_FILTERS; i++)
    {
		if (bHorizontal)
			c.SetUniform("flOffsetX", 1.2f / apSources[i].m_iWidth);
		else
			c.SetUniform("flOffsetY", 1.2f / apSources[i].m_iWidth);

		RenderFrameBufferToBuffer(&apSources[i], &apTargets[i]);
    }
}

void CRenderer::RenderFrameBufferFullscreen(CFrameBuffer* pBuffer)
{
	if (pBuffer->m_iMap)
		RenderMapFullscreen(pBuffer->m_iMap, pBuffer->m_bMultiSample);
	else if (pBuffer->m_iRB)
		RenderRBFullscreen(pBuffer);
}

void CRenderer::RenderFrameBufferToBuffer(CFrameBuffer* pSource, CFrameBuffer* pDestination)
{
	if (pSource->m_iMap)
		RenderMapToBuffer(pSource->m_iMap, pDestination, pSource->m_bMultiSample);
	else if (pSource->m_iRB)
		RenderRBToBuffer(pSource, pDestination);
}

void CRenderer::RenderRBFullscreen(CFrameBuffer* pSource)
{
	TAssert(false);		// ATI cards don't like this at all. Never do it.

	glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, (GLuint)pSource->m_iFB);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);

	glBlitFramebuffer(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderRBToBuffer(CFrameBuffer* pSource, CFrameBuffer* pDestination)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, (GLuint)pSource->m_iFB);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, (GLuint)pDestination->m_iFB);

	glBlitFramebuffer(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, pDestination->m_iWidth, pDestination->m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderMapFullscreen(size_t iMap, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);
	c.UseFrameBuffer(0);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	TAssert(!bMapIsMultisample);	// Must implement
	c.BindTexture(iMap);

	c.EndRenderVertexArray(6);
}

void CRenderer::RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);
	c.UseFrameBuffer(pBuffer);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	TAssert(!bMapIsMultisample);	// Must implement
	c.BindTexture(iMap);

	c.EndRenderVertexArray(6);
}

void CRenderer::FrustumOverride(Vector vecPosition, Vector vecDirection, float flFOV, float flNear, float flFar)
{
	m_bFrustumOverride = true;
	m_vecFrustumPosition = vecPosition;
	m_vecFrustumDirection = vecDirection;
	m_flFrustumFOV = flFOV;
	m_flFrustumNear = flNear;
	m_flFrustumFar = flFar;
}

void CRenderer::CancelFrustumOverride()
{
	m_bFrustumOverride = false;
}

Vector CRenderer::GetCameraVector()
{
	return m_vecCameraDirection;
}

void CRenderer::GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp)
{
	Vector vecForward = GetCameraVector();
	Vector vecRight;

	if (pvecForward)
		(*pvecForward) = vecForward;

	if (pvecRight || pvecUp)
		vecRight = vecForward.Cross(m_vecCameraUp).Normalized();

	if (pvecRight)
		(*pvecRight) = vecRight;

	if (pvecUp)
		(*pvecUp) = vecRight.Cross(vecForward).Normalized();
}

bool CRenderer::IsSphereInFrustum(const Vector& vecCenter, float flRadius)
{
	return m_oFrustum.TouchesSphere(vecCenter, flRadius);
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;

	m_vecFullscreenTexCoords[0] = Vector2D(0, 0);
	m_vecFullscreenTexCoords[1] = Vector2D(1, 1);
	m_vecFullscreenTexCoords[2] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[3] = Vector2D(0, 0);
	m_vecFullscreenTexCoords[4] = Vector2D(1, 0);
	m_vecFullscreenTexCoords[5] = Vector2D(1, 1);

	m_vecFullscreenVertices[0] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[1] = Vector(1, 1, 0);
	m_vecFullscreenVertices[2] = Vector(-1, 1, 0);
	m_vecFullscreenVertices[3] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[4] = Vector(1, -1, 0);
	m_vecFullscreenVertices[5] = Vector(1, 1, 0);
}

Vector CRenderer::ScreenPosition(Vector vecWorld)
{
	GLdouble x, y, z;
	gluProject(
		vecWorld.x, vecWorld.y, vecWorld.z,
		(GLdouble*)m_aflModelView, (GLdouble*)m_aflProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)m_iHeight - (float)y, (float)z);
}

Vector CRenderer::WorldPosition(Vector vecScreen)
{
	GLdouble x, y, z;
	gluUnProject(
		vecScreen.x, (float)m_iHeight - vecScreen.y, vecScreen.z,
		(GLdouble*)m_aflModelView, (GLdouble*)m_aflProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)y, (float)z);
}

bool CRenderer::HardwareSupported()
{
	if (!GLEW_VERSION_3_0)
		return false;

	// Compile a test framebuffer. If it fails we don't support framebuffers.

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &oBuffer.m_iDepth);
	glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512 );
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	glGenFramebuffers(1, &oBuffer.m_iFB);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		TError("Test framebuffer compile failed.\n");
		glDeleteTextures(1, &oBuffer.m_iMap);
		glDeleteRenderbuffers(1, &oBuffer.m_iDepth);
		glDeleteFramebuffers(1, &oBuffer.m_iFB);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteTextures(1, &oBuffer.m_iMap);
	glDeleteRenderbuffers(1, &oBuffer.m_iDepth);
	glDeleteFramebuffers(1, &oBuffer.m_iFB);

	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"void main()"
		"{"
		"	gl_Position = vec4(0.0, 0.0, 0.0, 0.0);"
		"}";

	const char* pszFragmentShader =
		"void main(void)"
		"{"
		"	gl_FragColor = vec4(1.0,1.0,1.0,1.0);"
		"}";

	GLuint iVShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint iProgram = glCreateProgram();

	glShaderSource(iVShader, 1, &pszVertexShader, NULL);
	glCompileShader(iVShader);

	int iVertexCompiled;
	glGetShaderiv(iVShader, GL_COMPILE_STATUS, &iVertexCompiled);

	glShaderSource(iFShader, 1, &pszFragmentShader, NULL);
	glCompileShader(iFShader);

	int iFragmentCompiled;
	glGetShaderiv(iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);

	glAttachShader(iProgram, iVShader);
	glAttachShader(iProgram, iFShader);
	glLinkProgram(iProgram);

	int iProgramLinked;
	glGetProgramiv(iProgram, GL_LINK_STATUS, &iProgramLinked);

	if (!(iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE))
		TError("Test shader compile failed.\n");

	glDetachShader(iProgram, iVShader);
	glDetachShader(iProgram, iFShader);
	glDeleteShader(iVShader);
	glDeleteShader(iFShader);
	glDeleteProgram(iProgram);

	return iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE;
}

size_t CRenderer::LoadVertexDataIntoGL(size_t iSizeInBytes, float* aflVertices)
{
	GLuint iVBO;
	glGenBuffersARB(1, &iVBO);
	glBindBufferARB(GL_ARRAY_BUFFER, iVBO);

	glBufferDataARB(GL_ARRAY_BUFFER, iSizeInBytes, aflVertices, GL_STATIC_DRAW);

	return iVBO;
}

void CRenderer::UnloadVertexDataFromGL(size_t iBuffer)
{
	glDeleteBuffersARB(1, &iBuffer);
}

size_t CRenderer::LoadTextureIntoGL(tstring sFilename, int iClamp)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Width & (ImageInfo.Width-1))
	{
		//TAssert(!"Image width is not power of 2.");
		ilDeleteImages(1, &iDevILId);
		return 0;
	}

	if (ImageInfo.Height & (ImageInfo.Height-1))
	{
		//TAssert(!"Image height is not power of 2.");
		ilDeleteImages(1, &iDevILId);
		return 0;
	}

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	size_t iGLId = LoadTextureIntoGL(iDevILId, iClamp);

	ilBindImage(0);
	ilDeleteImages(1, &iDevILId);

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(size_t iImageID, int iClamp)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp && !GLEW_EXT_texture_edge_clamp)
		iClamp = 1;

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if (iClamp == 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	ilBindImage(iImageID);

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	ilBindImage(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Color* pclrData, int iClamp)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp && !GLEW_EXT_texture_edge_clamp)
		iClamp = 1;

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if (iClamp == 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		4,
		256,
		256,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pclrData);

	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

void CRenderer::UnloadTextureFromGL(size_t iGLId)
{
	glDeleteTextures(1, &iGLId);
	s_iTexturesLoaded--;
}

size_t CRenderer::s_iTexturesLoaded = 0;

size_t CRenderer::LoadTextureData(tstring sFilename)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	ilBindImage(0);

	return iDevILId;
}

Color* CRenderer::GetTextureData(size_t iTexture)
{
	if (!iTexture)
		return NULL;

	ilBindImage(iTexture);
	Color* pclr = (Color*)ilGetData();
	ilBindImage(0);
	return pclr;
}

size_t CRenderer::GetTextureWidth(size_t iTexture)
{
	if (!iTexture)
		return 0;

	ilBindImage(iTexture);
	size_t iWidth = ilGetInteger(IL_IMAGE_WIDTH);
	ilBindImage(0);
	return iWidth;
}

size_t CRenderer::GetTextureHeight(size_t iTexture)
{
	if (!iTexture)
		return 0;

	ilBindImage(iTexture);
	size_t iHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	ilBindImage(0);
	return iHeight;
}

void CRenderer::UnloadTextureData(unsigned int iTexture)
{
	ilBindImage(0);
	ilDeleteImages(1, &iTexture);
}

void R_ReadPixels(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	size_t iFBO = 0;
	if (asTokens.size() > 1)
		iFBO = stoi(asTokens[1]);

	int aiViewport[4];
	glGetIntegerv( GL_VIEWPORT, aiViewport );

	int iWidth = aiViewport[2];
	int iHeight = aiViewport[3];

	unsigned char* pixels = new unsigned char[iWidth*iHeight*4];

	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)iFBO);
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);
	glReadPixels(0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ilTexImage((ILint)iWidth, (ILint)iHeight, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pixels);

	// Formats like PNG and VTF don't work unless it's in integer format.
	ilConvertImage(IL_RGBA, IL_UNSIGNED_INT);

	ilEnable(IL_FILE_OVERWRITE);

	tstring sFilename = sprintf("readpixels-%d.png", iFBO);
	ilSaveImage(convertstring<tchar, ILchar>(GetAppDataDirectory(Application()->AppDirectory(), sFilename)).c_str());

	ilDeleteImages(1,&iDevILId);

	delete pixels;
}

CCommand r_readpixels(tstring("r_readpixels"), ::R_ReadPixels);
