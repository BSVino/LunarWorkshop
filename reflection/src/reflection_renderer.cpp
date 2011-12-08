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
	for (size_t i = 0; i < 5; i++)
		m_aoReflectionBuffers.push_back(CreateFrameBuffer(m_iWidth, m_iHeight, true, true));
}

void CReflectionRenderer::Initialize()
{
	BaseClass::Initialize();
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

	eastl::vector<CEntityHandle<CMirror> > apMirrors;
	apMirrors.reserve(CMirror::GetNumMirrors());

	// None of these had better get deleted while we're doing this since they're not handles.
	for (size_t i = 0; i < CMirror::GetNumMirrors(); i++)
	{
		CMirror* pMirror = CMirror::GetMirror(i);
		if (!pMirror)
			continue;

		if (!pMirror->ShouldRender())
			continue;

		pMirror->SetBuffer(~0);

		if (!IsSphereInFrustum(pMirror->GetGlobalCenter(), (float)pMirror->GetBoundingRadius()))
			continue;

		if (apMirrors.size() >= m_aoReflectionBuffers.size())
			pMirror->SetBuffer(0);
		else
			pMirror->SetBuffer(apMirrors.size());
		apMirrors.push_back(pMirror);
	}

	for (size_t i = 0; i < apMirrors.size(); i++)
	{
		TPROF("Reflection rendering");

		if (i >= m_aoReflectionBuffers.size())
			continue;

		CMirror* pMirror = apMirrors[i];

		// Render a reflected world to our reflection buffer.
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_aoReflectionBuffers[i].m_iFB);
		glViewport(0, 0, (GLsizei)m_aoReflectionBuffers[i].m_iWidth, (GLsizei)m_aoReflectionBuffers[i].m_iHeight);

		glClear(GL_DEPTH_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);

		m_bRenderingReflection = true;

		CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

		if (pPlayerCharacter && (pPlayerCharacter->IsReflected(REFLECTION_LATERAL) == pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)))
			glFrontFace(GL_CW);

		StartRenderingReflection(pMirror);
		GameServer()->RenderEverything();
		FinishRendering();

		if (pPlayerCharacter && (pPlayerCharacter->IsReflected(REFLECTION_LATERAL) == pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)))
			glFrontFace(GL_CCW);

		m_bRenderingReflection = false;
	}

	BaseClass::SetupFrame();
}

void CReflectionRenderer::StartRendering()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
	{
		BaseClass::StartRendering();
		return;
	}

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	if (pPlayerCharacter && (pPlayerCharacter->IsReflected(REFLECTION_LATERAL) ^ pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)))
		glFrontFace(GL_CW);

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

	Vector vecCameraPosition = m_vecCameraPosition;
	Vector vecCameraTarget = m_vecCameraTarget;
	Vector vecCameraUp = m_vecCameraUp;

	CMirror* pMirror = pPlayerCharacter->GetMirrorInside();

	Vector vecMirror = pMirror?pMirror->GetGlobalOrigin():Vector();
	Matrix4x4 mReflect = pPlayerCharacter->GetReflectionMatrix();

	if (pPlayerCharacter->IsReflected(REFLECTION_ANY))
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

void CReflectionRenderer::FinishRendering()
{
	BaseClass::FinishRendering();

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	if (pPlayerCharacter && (pPlayerCharacter->IsReflected(REFLECTION_LATERAL) ^ pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)))
		glFrontFace(GL_CCW);
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

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	Matrix4x4 mReflect = pPlayerCharacter->GetReflectionMatrix();

	Matrix4x4 mTranslate, mInverseTranslate;
	mTranslate.SetTranslation(vecMirror);
	mInverseTranslate = mTranslate.InvertedRT();

	vecCameraPosition = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraPosition));
	vecCameraTarget = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraTarget));
	vecCameraUp = mReflect * m_vecCameraUp;

	mReflect *= pMirror->GetReflection();

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

	if (ShouldUseFramebuffers())
	{
		if (ShouldUseShaders())
			SetupSceneShader();

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
	bool bModel = false;
	if (pModel->m_sFilename == "models/mirror.toy")
		bModel = true;
	if (pModel->m_sFilename == "models/mirror_horizontal.toy")
		bModel = true;

	size_t iBuffer = ~0;
	if (bModel)
		iBuffer = static_cast<const CMirror*>(m_pRendering)->GetBuffer();

	if (bModel && pModel->m_aiTextures[iMaterial] == 0 && iBuffer != ~0)
	{
		c->BindTexture(GetReflectionTexture(iBuffer));

		c->UseProgram("reflection");
		c->SetUniform("bDiffuse", true);
		c->SetUniform("iDiffuse", 0);

		c->SetUniform("vecColor", c->GetColor());
		c->SetUniform("flAlpha", c->GetAlpha());

		c->SetUniform("flScreenWidth", (float)Application()->GetWindowWidth());
		c->SetUniform("flScreenHeight", (float)Application()->GetWindowHeight());

		c->SetUniform("bDiscardReflection", m_bRenderingReflection);

		return;
	}

	BaseClass::SetupShader(c, pModel, iMaterial);
}

size_t CReflectionRenderer::GetReflectionTexture(size_t i)
{
	return m_aoReflectionBuffers[i].m_iMap;
}

bool CReflectionRenderer::ShouldRenderPhysicsDebug() const
{
	return !IsRenderingReflection();
}

CReflectionRenderer* ReflectionRenderer()
{
	return static_cast<CReflectionRenderer*>(GameServer()->GetRenderer());
}
