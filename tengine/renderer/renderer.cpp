#include "renderer.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <maths.h>
#include <simplex.h>

#include <common/worklistener.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <models/texturelibrary.h>
#include <game/camera.h>
#include <physics/physics.h>
#include <toys/toy.h>

#include "renderingcontext.h"

CFrameBuffer::CFrameBuffer()
{
	m_iRB = m_iMap = m_iDepth = m_iFB = 0;
}

CVar r_batch("r_batch", "1");

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	TMsg("Initializing renderer\n");

#ifdef TINKER_OPTIMIZE_SOFTWARE
	m_bHardwareSupportsFramebuffers = false;
	m_bHardwareSupportsFramebuffersTestCompleted = true;
	m_bUseFramebuffers = false;

	m_bHardwareSupportsShaders = false;
	m_bHardwareSupportsShadersTestCompleted = true;
	m_bUseShaders = false;
#else
	m_bHardwareSupportsFramebuffers = false;
	m_bHardwareSupportsFramebuffersTestCompleted = false;

	m_bUseFramebuffers = true;

	if (!HardwareSupportsFramebuffers())
		m_bUseFramebuffers = false;

	m_bUseShaders = true;

	m_bHardwareSupportsShaders = false;
	m_bHardwareSupportsShadersTestCompleted = false;

	if (!HardwareSupportsShaders())
		m_bUseShaders = false;
#endif

	SetSize(iWidth, iHeight);

	m_bFrustumOverride = false;
	m_bBatching = false;

	m_bBatchThisFrame = r_batch.GetBool();

	DisableSkybox();

	m_pRendering = nullptr;
}

void CRenderer::Initialize()
{
	if (ShouldUseShaders())
	{
		LoadShaders();
		CShaderLibrary::CompileShaders();
	}

	if (ShouldUseFramebuffers())
	{
		m_oSceneBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_DEPTH));

		size_t iWidth = m_oSceneBuffer.m_iWidth;
		size_t iHeight = m_oSceneBuffer.m_iHeight;
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			m_oBloom1Buffers[i] = CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_TEXTURE|FB_LINEAR));
			m_oBloom2Buffers[i] = CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_TEXTURE));
			iWidth /= 2;
			iHeight /= 2;

			if (GameServer()->GetWorkListener())
				GameServer()->GetWorkListener()->WorkProgress(i);
		}

		if (GameServer()->GetWorkListener())
			GameServer()->GetWorkListener()->SetAction("Making noise", 0);

		m_oNoiseBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE));

		CreateNoise();
	}

	if (!CShaderLibrary::IsCompiled())
		m_bUseShaders = false;

	if (m_bUseShaders)
		TMsg("* Using shaders\n");
	if (m_bUseFramebuffers)
		TMsg("* Using framebuffers\n");
}

CFrameBuffer CRenderer::CreateFrameBuffer(size_t iWidth, size_t iHeight, fb_options_e eOptions)
{
	TAssert(ShouldUseFramebuffers());
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

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	GLint iScreenSamples;
	glGetIntegerv(GL_SAMPLES, &iScreenSamples);

	bool bMultisample = !!GLEW_EXT_framebuffer_multisample;
	bool bMultisampleTexture = false;//!!GLEW_ARB_texture_multisample;	// This isn't working and I don't need it for now.
	bool bUseMultisample = bMultisample;
	if ((eOptions&FB_TEXTURE) && !bMultisampleTexture)
		bUseMultisample = false;

	GLsizei iSamples = iScreenSamples;

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
		glGenRenderbuffersEXT(1, &oBuffer.m_iRB);
		glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB );
		if (bUseMultisample)
			glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, iSamples, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );
	}

	if (eOptions&FB_DEPTH)
	{
		glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
		glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
		if (bUseMultisample)
			glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, iSamples, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );
	}

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	if (eOptions&FB_TEXTURE)
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iTextureTarget, (GLuint)oBuffer.m_iMap, 0);
	else if (eOptions&FB_RENDERBUFFER)
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB);
	if (eOptions&FB_DEPTH)
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	TAssert(status == GL_FRAMEBUFFER_COMPLETE_EXT);

	GLint iFBSamples;
	glGetIntegerv(GL_SAMPLES, &iFBSamples);

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	TAssert(iFBSamples == iScreenSamples || iFBSamples == 0 || 0 == iScreenSamples);

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

void CRenderer::CreateNoise()
{
	if (!WantNoise())
		return;

	CSimplexNoise n1(mtrand()+0);
	CSimplexNoise n2(mtrand()+1);
	CSimplexNoise n3(mtrand()+2);

	float flSpaceFactor1 = 0.1f;
	float flHeightFactor1 = 0.5f;
	float flSpaceFactor2 = flSpaceFactor1*3;
	float flHeightFactor2 = flHeightFactor1/3;
	float flSpaceFactor3 = flSpaceFactor2*3;
	float flHeightFactor3 = flHeightFactor2/3;

	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oNoiseBuffer.m_iFB);

	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPointSize(1);

	glPushAttrib(GL_CURRENT_BIT);

	for (size_t x = 0; x < m_iWidth; x++)
	{
		for (size_t y = 0; y < m_iHeight; y++)
		{
			float flValue = 0.5f;
			flValue += n1.Noise(x*flSpaceFactor1, y*flSpaceFactor1) * flHeightFactor1;
			flValue += n2.Noise(x*flSpaceFactor2, y*flSpaceFactor2) * flHeightFactor2;
			flValue += n3.Noise(x*flSpaceFactor3, y*flSpaceFactor3) * flHeightFactor3;

			glBegin(GL_POINTS);
				glColor3f(flValue, flValue, flValue);
				glVertex2f((float)x, (float)y);
			glEnd();
		}
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}

void CRenderer::PreFrame()
{
}

void CRenderer::PostFrame()
{
}

void CRenderer::SetupFrame()
{
	TPROF("CRenderer::SetupFrame");

	m_bBatchThisFrame = r_batch.GetBool();

	if (ShouldUseFramebuffers())
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);
		glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);
	}
	else
	{
		glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	glColor4f(1, 1, 1, 1);

	if (m_iSkyboxFT == ~0)
		DrawBackground();
	else
		DrawSkybox();
}

void CRenderer::DrawBackground()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glShadeModel(GL_FLAT);
#else
	glShadeModel(GL_SMOOTH);
#endif

	glBegin(GL_QUADS);
		glColor3ub(0, 0, 0);
		glVertex2f(-1.0f, 1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(-1.0f, -1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(1.0f, -1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(1.0f, 1.0f);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CRenderer::DrawSkybox()
{
	TPROF("CRenderer::RenderSkybox");

	CCamera* pCamera = GameServer()->GetCamera();

	SetCameraPosition(pCamera->GetCameraPosition());
	SetCameraTarget(pCamera->GetCameraTarget());
	SetCameraUp(pCamera->GetCameraUp());
	SetCameraFOV(pCamera->GetCameraFOV());
	SetCameraNear(pCamera->GetCameraNear());
	SetCameraFar(pCamera->GetCameraFar());

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		m_vecCameraUp.x, m_vecCameraUp.y, m_vecCameraUp.z);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	if (true)
	{
		CRenderingContext c(this);

		glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);
		glPushMatrix();
		glTranslatef(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		if (GLEW_ARB_multitexture || GLEW_VERSION_1_3)
			glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		ModifySkyboxContext(&c);

		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

		glEnableClientState(GL_VERTEX_ARRAY);

		glClientActiveTexture(GL_TEXTURE0);
		glTexCoordPointer(2, GL_FLOAT, 0, m_avecSkyboxTexCoords);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glVertexPointer(3, GL_FLOAT, 0, m_avecSkyboxFT);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxFT);
		glDrawArrays(GL_QUADS, 0, 4);

		glVertexPointer(3, GL_FLOAT, 0, m_avecSkyboxBK);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxBK);
		glDrawArrays(GL_QUADS, 0, 4);

		glVertexPointer(3, GL_FLOAT, 0, m_avecSkyboxLF);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxLF);
		glDrawArrays(GL_QUADS, 0, 4);

		glVertexPointer(3, GL_FLOAT, 0, m_avecSkyboxRT);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxRT);
		glDrawArrays(GL_QUADS, 0, 4);

		glVertexPointer(3, GL_FLOAT, 0, m_avecSkyboxUP);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxUP);
		glDrawArrays(GL_QUADS, 0, 4);

		glVertexPointer(3, GL_FLOAT, 0, m_avecSkyboxDN);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxDN);
		glDrawArrays(GL_QUADS, 0, 4);

		glPopClientAttrib();

		glPopMatrix();
		glPopAttrib();
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CRenderer::StartRendering()
{
	TPROF("CRenderer::StartRendering");

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		m_vecCameraUp.x, m_vecCameraUp.y, m_vecCameraUp.z);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	if (m_bFrustumOverride)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		gluPerspective(
				m_flFrustumFOV,
				(float)m_iWidth/(float)m_iHeight,
				m_flFrustumNear,
				m_flFrustumFar
			);

		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();
		glLoadIdentity();

		gluLookAt(m_vecFrustumPosition.x, m_vecFrustumPosition.y, m_vecFrustumPosition.z,
			m_vecFrustumTarget.x, m_vecFrustumTarget.y, m_vecFrustumTarget.z,
			m_vecCameraUp.x, m_vecCameraUp.y, m_vecCameraUp.z);
	}

	Matrix4x4 mModelView, mProjection;
	glGetFloatv( GL_MODELVIEW_MATRIX, mModelView );
	glGetFloatv( GL_PROJECTION_MATRIX, mProjection );

	if (m_bFrustumOverride)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	m_oFrustum.CreateFrom(mProjection * mModelView);

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glPopAttrib();

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

CVar show_frustum("debug_show_frustum", "no");
CVar show_physics("debug_show_physics", "no");

void CRenderer::FinishRendering()
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

	if (show_physics.GetBool() && ShouldRenderPhysicsDebug())
		GamePhysics()->DebugDraw(show_physics.GetInt());

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CRenderer::FinishFrame()
{
	if (ShouldUseFramebuffers())
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	RenderOffscreenBuffers();

	RenderFullscreenBuffers();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

CVar r_bloom("r_bloom", "1");

void CRenderer::RenderOffscreenBuffers()
{
	if (ShouldUseFramebuffers() && ShouldUseShaders() && r_bloom.GetBool())
	{
		TPROF("Bloom");

		// Use a bright-pass filter to catch only the bright areas of the image
		GLuint iBrightPass = (GLuint)CShaderLibrary::GetProgram("brightpass");
		UseProgram(iBrightPass);

		GLint iSource = glGetUniformLocation(iBrightPass, "iSource");
		glUniform1i(iSource, 0);

		GLint flScale = glGetUniformLocation(iBrightPass, "flScale");
		glUniform1f(flScale, (float)1/BLOOM_FILTERS);

		GLint flBrightness = glGetUniformLocation(iBrightPass, "flBrightness");

		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			glUniform1f(flBrightness, BloomBrightnessCutoff() - 0.1f*i);
			RenderFrameBufferToBuffer(&m_oSceneBuffer, &m_oBloom1Buffers[i]);
		}

		ClearProgram();

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);
	}
}

void CRenderer::RenderFullscreenBuffers()
{
	TPROF("CRenderer::RenderFullscreenBuffers");

	if (ShouldUseFramebuffers())
	{
		if (ShouldUseShaders())
			SetupSceneShader();

		RenderFrameBufferFullscreen(&m_oSceneBuffer);

		if (ShouldUseShaders())
			ClearProgram();
	}

	glEnable(GL_BLEND);

	if (ShouldUseFramebuffers() && r_bloom.GetBool())
	{
		glBlendFunc(GL_ONE, GL_ONE);
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
			RenderFrameBufferFullscreen(&m_oBloom1Buffers[i]);
	}

	glDisable(GL_BLEND);
}

void CRenderer::SetSkybox(size_t ft, size_t bk, size_t lf, size_t rt, size_t up, size_t dn)
{
	m_iSkyboxFT = ft;
	m_iSkyboxLF = lf;
	m_iSkyboxBK = bk;
	m_iSkyboxRT = rt;
	m_iSkyboxDN = dn;
	m_iSkyboxUP = up;

	m_avecSkyboxTexCoords[0] = Vector2D(0, 1);
	m_avecSkyboxTexCoords[1] = Vector2D(0, 0);
	m_avecSkyboxTexCoords[2] = Vector2D(1, 0);
	m_avecSkyboxTexCoords[3] = Vector2D(1, 1);

	m_avecSkyboxFT[0] = Vector(100, 100, -100);
	m_avecSkyboxFT[1] = Vector(100, -100, -100);
	m_avecSkyboxFT[2] = Vector(100, -100, 100);
	m_avecSkyboxFT[3] = Vector(100, 100, 100);

	m_avecSkyboxBK[0] = Vector(-100, 100, 100);
	m_avecSkyboxBK[1] = Vector(-100, -100, 100);
	m_avecSkyboxBK[2] = Vector(-100, -100, -100);
	m_avecSkyboxBK[3] = Vector(-100, 100, -100);

	m_avecSkyboxLF[0] = Vector(-100, 100, -100);
	m_avecSkyboxLF[1] = Vector(-100, -100, -100);
	m_avecSkyboxLF[2] = Vector(100, -100, -100);
	m_avecSkyboxLF[3] = Vector(100, 100, -100);

	m_avecSkyboxRT[0] = Vector(100, 100, 100);
	m_avecSkyboxRT[1] = Vector(100, -100, 100);
	m_avecSkyboxRT[2] = Vector(-100, -100, 100);
	m_avecSkyboxRT[3] = Vector(-100, 100, 100);

	m_avecSkyboxUP[0] = Vector(-100, 100, -100);
	m_avecSkyboxUP[1] = Vector(100, 100, -100);
	m_avecSkyboxUP[2] = Vector(100, 100, 100);
	m_avecSkyboxUP[3] = Vector(-100, 100, 100);

	m_avecSkyboxDN[0] = Vector(100, -100, -100);
	m_avecSkyboxDN[1] = Vector(-100, -100, -100);
	m_avecSkyboxDN[2] = Vector(-100, -100, 100);
	m_avecSkyboxDN[3] = Vector(100, -100, 100);
}

void CRenderer::DisableSkybox()
{
	m_iSkyboxFT = ~0;
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	GLuint iBlur = (GLuint)CShaderLibrary::GetProgram("blur");
	UseProgram(iBlur);

	GLint iSource = glGetUniformLocation(iBlur, "iSource");
    glUniform1i(iSource, 0);

	// Can't I get rid of this and hard code it into the shader?
	GLint aflCoefficients = glGetUniformLocation(iBlur, "aflCoefficients");
    glUniform1fv(aflCoefficients, KERNEL_SIZE, aflKernel);

    GLint flOffsetX = glGetUniformLocation(iBlur, "flOffsetX");
    glUniform1f(flOffsetX, 0);

	GLint flOffset = glGetUniformLocation(iBlur, "flOffsetY");
    glUniform1f(flOffset, 0);
    if (bHorizontal)
        flOffset = glGetUniformLocation(iBlur, "flOffsetX");

    // Perform the blurring.
    for (size_t i = 0; i < BLOOM_FILTERS; i++)
    {
		glUniform1f(flOffset, 1.2f / apSources[i].m_iWidth);
		RenderFrameBufferToBuffer(&apSources[i], &apTargets[i]);
    }

	ClearProgram();
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
	TAssert(ShouldUseFramebuffers());

	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, (GLuint)pSource->m_iFB);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);

	glBlitFramebufferEXT(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderRBToBuffer(CFrameBuffer* pSource, CFrameBuffer* pDestination)
{
	TAssert(ShouldUseFramebuffers());

	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, (GLuint)pSource->m_iFB);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, (GLuint)pDestination->m_iFB);

	glBlitFramebufferEXT(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, pDestination->m_iWidth, pDestination->m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderMapFullscreen(size_t iMap, bool bMapIsMultisample)
{
	if (GLEW_ARB_multitexture || GLEW_VERSION_1_3)
		glActiveTexture(GL_TEXTURE0);

	glEnable(GL_TEXTURE_2D);

	if (ShouldUseFramebuffers())
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glEnableClientState(GL_VERTEX_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);

	int iProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &iProgram);
	if (iProgram == 0)
	{
		glTexCoordPointer(2, GL_FLOAT, 0, m_vecFullscreenTexCoords);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		int iTexCoordAttribute = glGetAttribLocation(iProgram, "vecTexCoord0");

		glEnableVertexAttribArray(iTexCoordAttribute);
		glVertexAttribPointer(iTexCoordAttribute, 2, GL_FLOAT, false, 0, m_vecFullscreenTexCoords);
	}

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glVertexPointer(2, GL_FLOAT, 0, m_vecFullscreenVertices);

	if (bMapIsMultisample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)iMap);
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	glDrawArrays(GL_QUADS, 0, 4);

	glPopClientAttrib();
}

void CRenderer::RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer, bool bMapIsMultisample)
{
	TAssert(ShouldUseFramebuffers());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, pBuffer->m_iWidth, pBuffer->m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);

	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glEnableClientState(GL_VERTEX_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);

	int iProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &iProgram);
	if (iProgram == 0)
	{
		glTexCoordPointer(2, GL_FLOAT, 0, pBuffer->m_vecTexCoords);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		int iTexCoordAttribute = glGetAttribLocation(iProgram, "vecTexCoord0");

		glEnableVertexAttribArray(iTexCoordAttribute);
		glVertexAttribPointer(iTexCoordAttribute, 2, GL_FLOAT, false, 0, m_vecFullscreenTexCoords);
	}

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glVertexPointer(2, GL_FLOAT, 0, pBuffer->m_vecVertices);

	if (bMapIsMultisample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)iMap);
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	glDrawArrays(GL_QUADS, 0, 4);

	glPopClientAttrib();

	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CRenderer::FrustumOverride(Vector vecPosition, Vector vecTarget, float flFOV, float flNear, float flFar)
{
	m_bFrustumOverride = true;
	m_vecFrustumPosition = vecPosition;
	m_vecFrustumTarget = vecTarget;
	m_flFrustumFOV = flFOV;
	m_flFrustumNear = flNear;
	m_flFrustumFar = flFar;
}

void CRenderer::CancelFrustumOverride()
{
	m_bFrustumOverride = false;
}

void CRenderer::BeginBatching()
{
	if (!ShouldBatchThisFrame())
		return;

	m_bBatching = true;

	for (eastl::map<size_t, eastl::vector<CRenderBatch> >::iterator it = m_aBatches.begin(); it != m_aBatches.end(); it++)
		it->second.clear();
}

void CRenderer::AddToBatch(class CModel* pModel, const CBaseEntity* pEntity, const Matrix4x4& mTransformations, const Color& clrRender, bool bClrSwap, const Color& clrSwap, bool bReverseWinding)
{
	TAssert(pModel);
	if (!pModel)
		return;

	for (size_t i = 0; i < pModel->m_aiTextures.size(); i++)
	{
		CRenderBatch* pBatch = &m_aBatches[pModel->m_aiTextures[i]].push_back();

		pBatch->pEntity = pEntity;
		pBatch->pModel = pModel;
		pBatch->mTransformation = mTransformations;
		pBatch->bSwap = bClrSwap;
		pBatch->bReverseWinding = bReverseWinding;
		pBatch->clrSwap = clrSwap;
		pBatch->clrRender = clrRender;
		pBatch->iMaterial = i;
	}
}

void CRenderer::RenderBatches()
{
	TPROF("CRenderer::RenderBatches");

	m_bBatching = false;

	if (!ShouldBatchThisFrame())
		return;

	CRenderingContext c(this);

	for (eastl::map<size_t, eastl::vector<CRenderBatch> >::iterator it = m_aBatches.begin(); it != m_aBatches.end(); it++)
	{
		c.BindTexture(it->first);

		for (size_t i = 0; i < it->second.size(); i++)
		{
			CRenderBatch* pBatch = &it->second[i];

			c.SetReverseWinding(pBatch->bReverseWinding);

			c.ResetTransformations();
			c.LoadTransform(pBatch->mTransformation);

			m_pRendering = pBatch->pEntity;
			c.RenderModel(pBatch->pModel, pBatch->iMaterial);
			m_pRendering = nullptr;
		}
	}
}

void CRenderer::ClassifySceneAreaPosition(CModel* pModel)
{
	if (!pModel->m_pToy->GetNumSceneAreas())
		return;

	auto it = m_aiCurrentSceneAreas.find(pModel->m_sFilename);
	if (it == m_aiCurrentSceneAreas.end())
	{
		// No entry? 
		FindSceneAreaPosition(pModel);
		return;
	}

	if (it->second >= pModel->m_pToy->GetNumSceneAreas())
	{
		FindSceneAreaPosition(pModel);
		return;
	}

	if (pModel->m_pToy->GetSceneAreaAABB(it->second).Inside(m_vecCameraPosition))
		return;

	FindSceneAreaPosition(pModel);
}

size_t CRenderer::GetSceneAreaPosition(CModel* pModel)
{
	auto it = m_aiCurrentSceneAreas.find(pModel->m_sFilename);

	if (it == m_aiCurrentSceneAreas.end())
		return ~0;

	return it->second;
}

void CRenderer::FindSceneAreaPosition(CModel* pModel)
{
	for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
	{
		if (pModel->m_pToy->GetSceneAreaAABB(i).Inside(m_vecCameraPosition))
		{
			m_aiCurrentSceneAreas[pModel->m_sFilename] = i;
			return;
		}
	}

	// If there's no entry for this model yet, find the closest.
	if (m_aiCurrentSceneAreas.find(pModel->m_sFilename) == m_aiCurrentSceneAreas.end())
	{
		size_t iClosest = 0;
		for (size_t i = 1; i < pModel->m_pToy->GetNumSceneAreas(); i++)
		{
			if (pModel->m_pToy->GetSceneAreaAABB(i).Center().DistanceSqr(m_vecCameraPosition) < pModel->m_pToy->GetSceneAreaAABB(iClosest).Center().DistanceSqr(m_vecCameraPosition))
				iClosest = i;
		}

		m_aiCurrentSceneAreas[pModel->m_sFilename] = iClosest;
		return;
	}

	// Otherwise if we don't find one don't fuck with it. We'll consider ourselves to still be in the previous one.
}

Vector CRenderer::GetCameraVector()
{
	return (m_vecCameraTarget - m_vecCameraPosition).Normalized();
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

	m_vecFullscreenTexCoords[0] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[1] = Vector2D(0, 0);
	m_vecFullscreenTexCoords[2] = Vector2D(1, 0);
	m_vecFullscreenTexCoords[3] = Vector2D(1, 1);

	m_vecFullscreenVertices[0] = Vector2D(0, 0);
	m_vecFullscreenVertices[1] = Vector2D(0, (float)m_iHeight);
	m_vecFullscreenVertices[2] = Vector2D((float)m_iWidth, (float)m_iHeight);
	m_vecFullscreenVertices[3] = Vector2D((float)m_iWidth, 0);
}

void CRenderer::ClearProgram()
{
	TAssert(ShouldUseShaders());

	if (!ShouldUseShaders())
		return;

	glUseProgram(0);
}

void CRenderer::UseProgram(size_t i)
{
	TAssert(ShouldUseShaders());

	if (!ShouldUseShaders())
		return;

	glUseProgram(i);
}

void CRenderer::SetupShader(CRenderingContext* c, CModel* pModel, size_t iMaterial)
{
	c->UseProgram("model");
	c->SetUniform("bDiffuse", true);
	c->SetUniform("iDiffuse", 0);

	c->SetUniform("vecColor", c->GetColor());
	c->SetUniform("flAlpha", c->GetAlpha());
	c->SetUniform("bColorSwapInAlpha", c->IsColorSwapActive());
	if (c->IsColorSwapActive())
		c->SetUniform("vecColorSwap", c->GetColorSwap());
}

Vector CRenderer::ScreenPosition(Vector vecWorld)
{
	GLdouble x, y, z;
	gluProject(
		vecWorld.x, vecWorld.y, vecWorld.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)m_iHeight - (float)y, (float)z);
}

Vector CRenderer::WorldPosition(Vector vecScreen)
{
	GLdouble x, y, z;
	gluUnProject(
		vecScreen.x, (float)m_iHeight - vecScreen.y, vecScreen.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)y, (float)z);
}

bool CRenderer::HardwareSupportsFramebuffers()
{
	if (m_bHardwareSupportsFramebuffersTestCompleted)
		return m_bHardwareSupportsFramebuffers;

	m_bHardwareSupportsFramebuffersTestCompleted = true;

	if (!GLEW_EXT_framebuffer_object)
	{
		TMsg("EXT_framebuffer_object not supported.\n");
		m_bHardwareSupportsFramebuffers = false;
		return false;
	}

	// Compile a test framebuffer. If it fails we don't support framebuffers.

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512 );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		TMsg("Test framebuffer compile failed.\n");
		glDeleteTextures(1, &oBuffer.m_iMap);
		glDeleteRenderbuffersEXT(1, &oBuffer.m_iDepth);
		glDeleteFramebuffersEXT(1, &oBuffer.m_iFB);
		m_bHardwareSupportsFramebuffers = false;
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glDeleteTextures(1, &oBuffer.m_iMap);
	glDeleteRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glDeleteFramebuffersEXT(1, &oBuffer.m_iFB);

	m_bHardwareSupportsFramebuffers = true;
	return true;
}

bool CRenderer::HardwareSupportsShaders()
{
	if (m_bHardwareSupportsShadersTestCompleted)
		return m_bHardwareSupportsShaders;

	m_bHardwareSupportsShadersTestCompleted = true;

	if (!GLEW_ARB_fragment_program)
	{
		TMsg("ARB_fragment_program not supported.\n");
		m_bHardwareSupportsShaders = false;
		return false;
	}

	if (!GLEW_VERSION_2_0)
	{
		TMsg("GL_VERSION_2_0 not supported.\n");
		m_bHardwareSupportsShaders = false;
		return false;
	}

	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
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


	if (iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE)
		m_bHardwareSupportsShaders = true;
	else
		TMsg("Test shader compile failed.\n");

	glDetachShader(iProgram, iVShader);
	glDetachShader(iProgram, iFShader);
	glDeleteShader(iVShader);
	glDeleteShader(iFShader);
	glDeleteProgram(iProgram);

	return m_bHardwareSupportsShaders;
}

size_t CRenderer::LoadVertexDataIntoGL(size_t iVerts, float* aflVertices)
{
	GLuint iVBO;
	glGenBuffersARB(1, &iVBO);
	glBindBufferARB(GL_ARRAY_BUFFER, iVBO);

	glBufferDataARB(GL_ARRAY_BUFFER, iVerts, aflVertices, GL_STATIC_DRAW);

	return iVBO;
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

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

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

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

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
	int iWidth = GameServer()->GetRenderer()->GetSceneBuffer()->m_iWidth;
	int iHeight = GameServer()->GetRenderer()->GetSceneBuffer()->m_iHeight;
	unsigned char* pixels = new unsigned char[iWidth*iHeight*4];

	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)GameServer()->GetRenderer()->GetSceneBuffer()->m_iFB);
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);
	glReadPixels(0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ilTexImage((ILint)iWidth, (ILint)iHeight, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pixels);

	// Formats like PNG and VTF don't work unless it's in integer format.
	ilConvertImage(IL_RGBA, IL_UNSIGNED_INT);

	ilSaveImage(convertstring<char, ILchar>("readpixels.png").c_str());

	ilDeleteImages(1,&iDevILId);

	delete pixels;
}

CCommand r_readpixels(tstring("r_readpixels"), ::R_ReadPixels);
