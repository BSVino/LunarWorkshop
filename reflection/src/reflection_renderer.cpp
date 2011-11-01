#include "reflection_renderer.h"

#include <GL/glew.h>

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>

#include "reflection_game.h"
#include "mirror.h"
#include "reflection_playercharacter.h"

CReflectionRenderer::CReflectionRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_oReflectionBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, true, true);
}


void CReflectionRenderer::Initialize()
{
	BaseClass::Initialize();

	CModel* pModel = CModelLibrary::Get()->GetModel(CModelLibrary::Get()->FindModel("models/mirror.obj"));
	for (size_t i = 0; i < pModel->m_aiTextures.size(); i++)
	{
		// A bit of a hack, but there's no material library yet so...
		if (pModel->m_aiTextures[i] == 0)
		{
			// Here's our reflection surface.
			pModel->m_aiTextures[i] = GetReflectionTexture();
		}
	}
}

void CReflectionRenderer::LoadShaders()
{
	CShaderLibrary::AddShader("brightpass", "pass", "brightpass");
	CShaderLibrary::AddShader("model", "pass", "model");
	CShaderLibrary::AddShader("blur", "pass", "blur");
	CShaderLibrary::AddShader("reflection", "pass", "reflection");
}

extern CVar r_batch;

void CReflectionRenderer::SetupFrame()
{
	TPROF("CReflectionRenderer::SetupFrame");

	m_bBatchThisFrame = r_batch.GetBool();

	if (m_hMirror != NULL)
	{
		TPROF("Reflection rendering");

		// Render a reflected world to our reflection buffer.
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oReflectionBuffer.m_iFB);
		glViewport(0, 0, (GLsizei)m_oReflectionBuffer.m_iWidth, (GLsizei)m_oReflectionBuffer.m_iHeight);

		glClear(GL_DEPTH_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);

		m_bRenderingReflection = true;
		glFrontFace(GL_CW);
		StartRenderingReflection(m_hMirror);
		GameServer()->RenderEverything();
		FinishRendering();
		glFrontFace(GL_CCW);
		m_bRenderingReflection = false;
	}

	BaseClass::SetupFrame();
}

void CReflectionRenderer::StartRendering()
{
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

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	Vector vecCameraPosition = m_vecCameraPosition;
	Vector vecCameraTarget = m_vecCameraTarget;
	Vector vecCameraUp = m_vecCameraUp;

	if (pPlayerCharacter->IsReflected())
	{
		CMirror* pMirror = pPlayerCharacter->GetMirrorInside();

		TAssert(pMirror);

		Vector vecMirror = pMirror?pMirror->GetGlobalOrigin():Vector();
		Vector vecMirrorForward = pMirror?pMirror->GetGlobalTransform().GetForwardVector():Vector(1, 0, 0);

		Matrix4x4 mReflect;
		mReflect.SetReflection(vecMirrorForward);

		Matrix4x4 mTranslate, mInverseTranslate;
		mTranslate.SetTranslation(vecMirror);
		mInverseTranslate = mTranslate.InvertedRT();

		vecCameraPosition = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraPosition));
		vecCameraTarget = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraTarget));
		vecCameraUp = mReflect * m_vecCameraUp;
	}

	gluLookAt(vecCameraPosition.x, vecCameraPosition.y, vecCameraPosition.z,
		vecCameraTarget.x, vecCameraTarget.y, vecCameraTarget.z,
		vecCameraUp.x, vecCameraUp.y, vecCameraUp.z);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	Matrix4x4 mModelView, mProjection;
	glGetFloatv( GL_MODELVIEW_MATRIX, mModelView );
	glGetFloatv( GL_PROJECTION_MATRIX, mProjection );

	m_oFrustum.CreateFrom(mProjection * mModelView);

	// Optimization opportunity: shrink the view frustum to the edges and surface of the mirror?

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glPopAttrib();

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

void CReflectionRenderer::StartRenderingReflection(CMirror* pMirror)
{
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

	Vector vecMirror = pMirror->GetGlobalOrigin();
	Vector vecMirrorForward = pMirror->GetGlobalTransform().GetForwardVector();

	Vector vecCameraPosition = m_vecCameraPosition;
	Vector vecCameraTarget = m_vecCameraTarget;
	Vector vecCameraUp = m_vecCameraUp;

	Matrix4x4 mReflect;
	mReflect.SetReflection(vecMirrorForward);

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();
	if (pPlayerCharacter->IsReflected())
	{
		Matrix4x4 mTranslate, mInverseTranslate;
		mTranslate.SetTranslation(vecMirror);
		mInverseTranslate = mTranslate.InvertedRT();

		vecCameraPosition = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraPosition));
		vecCameraTarget = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraTarget));
		vecCameraUp = mReflect * m_vecCameraUp;
	}

	gluLookAt(vecCameraPosition.x, vecCameraPosition.y, vecCameraPosition.z,
		vecCameraTarget.x, vecCameraTarget.y, vecCameraTarget.z,
		vecCameraUp.x, vecCameraUp.y, vecCameraUp.z);

	// Transform back to global space
	glTranslatef(vecMirror.x, vecMirror.y, vecMirror.z);

	// Do the reflection
	glMultMatrixf(mReflect);

	// Transform to mirror's space
	glTranslatef(-vecMirror.x, -vecMirror.y, -vecMirror.z);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	Matrix4x4 mModelView, mProjection;
	glGetFloatv( GL_MODELVIEW_MATRIX, mModelView );
	glGetFloatv( GL_PROJECTION_MATRIX, mProjection );

	m_oFrustum.CreateFrom(mProjection * mModelView);

	// Optimization opportunity: shrink the view frustum to the edges and surface of the mirror?

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glPopAttrib();

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

extern CVar r_bloom;

void CReflectionRenderer::RenderFullscreenBuffers()
{
	TPROF("CReflectionRenderer::RenderFullscreenBuffers");

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	if (ShouldUseFramebuffers())
	{
		if (ShouldUseShaders())
			SetupSceneShader();

		if (pPlayerCharacter->IsReflected())
			RenderMapFullscreen(m_oReflectionBuffer.m_iMap);
		else
			RenderMapFullscreen(m_oSceneBuffer.m_iMap);

		if (ShouldUseShaders())
			ClearProgram();
	}

	glEnable(GL_BLEND);

	if (ShouldUseFramebuffers() && r_bloom.GetBool())
	{
		glBlendFunc(GL_ONE, GL_ONE);
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
			RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);
	}

	glDisable(GL_BLEND);
}

void CReflectionRenderer::SetupShader(CRenderingContext* c, CModel* pModel, size_t iMaterial)
{
	if (pModel->m_sFilename == "models/mirror.obj" && pModel->m_aiTextures[iMaterial] == GetReflectionTexture())
	{
		c->UseProgram("reflection");
		c->SetUniform("bDiffuse", true);
		c->SetUniform("iDiffuse", 0);

		c->SetUniform("vecColor", c->GetColor());
		c->SetUniform("flAlpha", c->GetAlpha());

		c->SetUniform("flScreenWidth", (float)Application()->GetWindowWidth());
		c->SetUniform("flScreenHeight", (float)Application()->GetWindowHeight());

		CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();
		if (pPlayerCharacter->IsReflected())
			c->SetUniform("bDiscardReflection", !m_bRenderingReflection);
		else
			c->SetUniform("bDiscardReflection", m_bRenderingReflection);

		if (pPlayerCharacter->IsReflected())
			c->BindTexture(m_oSceneBuffer.m_iMap);

		return;
	}

	BaseClass::SetupShader(c, pModel, iMaterial);
}

size_t CReflectionRenderer::GetReflectionTexture()
{
	return m_oReflectionBuffer.m_iMap;
}

void CReflectionRenderer::SetMirror(CMirror* pMirror)
{
	m_hMirror = pMirror;
}

CReflectionRenderer* ReflectionRenderer()
{
	return static_cast<CReflectionRenderer*>(GameServer()->GetRenderer());
}
