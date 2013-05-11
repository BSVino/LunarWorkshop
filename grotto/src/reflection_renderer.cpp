#include "reflection_renderer.h"

#include <GL3/gl3w.h>

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <models/models.h>

#include "reflection_game.h"
#include "mirror.h"
#include "reflection_playercharacter.h"
#include "kaleidobeast.h"

CReflectionRenderer::CReflectionRenderer()
	: CGameRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	for (size_t i = 0; i < 5; i++)
		m_aoReflectionBuffers.push_back(CreateFrameBuffer(m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_DEPTH|FB_LINEAR)));
}

void CReflectionRenderer::Initialize()
{
	BaseClass::Initialize();

	m_bRenderingReflection = false;
}

void CReflectionRenderer::PreRender()
{
	TPROF("CReflectionRenderer::SetupFrame");

	tvector<CEntityHandle<CMirror> > apMirrors;
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

		CRenderingContext c(this);

		StartRenderingReflection(&c, pMirror);

		// Render a reflected world to our reflection buffer.
		c.UseFrameBuffer(&m_aoReflectionBuffers[i]);

		c.ClearDepth();

		m_bRenderingReflection = true;

		CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

		if (pPlayerCharacter)
			c.SetWinding(!(pPlayerCharacter->IsReflected(REFLECTION_LATERAL) == pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)));

		RenderEverything();
		FinishRendering(&c);

		m_bRenderingReflection = false;
	}

	BaseClass::PreRender();
}

void CReflectionRenderer::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (!Game()->GetNumLocalPlayers())
		return;

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	if (pPlayerCharacter)
		pContext->SetWinding(pPlayerCharacter->IsReflected(REFLECTION_LATERAL) == pPlayerCharacter->IsReflected(REFLECTION_VERTICAL));
}

void CReflectionRenderer::SetupFrame(class CRenderingContext* pContext)
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		m_bDrawBackground = true;
	else
		m_bDrawBackground = false;

	BaseClass::SetupFrame(pContext);
}

void CReflectionRenderer::StartRendering(class CRenderingContext* pContext)
{
	if (CVar::GetCVarValue("game_mode") == "menu")
	{
		BaseClass::StartRendering(pContext);
		return;
	}

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	pContext->SetProjection(Matrix4x4::ProjectPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		));

	Vector vecCameraPosition = m_vecCameraPosition;
	Vector vecCameraDirection = m_vecCameraDirection;
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
		vecCameraDirection = mTranslate.TransformVector(mReflect.TransformVector(mInverseTranslate.TransformVector(m_vecCameraDirection)));
		vecCameraUp = mReflect * m_vecCameraUp;
	}

	Matrix4x4 mView = Matrix4x4::ConstructCameraView(vecCameraPosition, vecCameraDirection, vecCameraUp);

	// Transform to mirror's space
	mView.AddTranslation(vecMirror);

	// Do the reflection
	mView *= mReflect;

	// Transform back to global space
	mView.AddTranslation(-vecMirror);

	pContext->SetView(mView);

	for (size_t i = 0; i < 16; i++)
	{
		m_aflModelView[i] = ((float*)pContext->GetView())[i];
		m_aflProjection[i] = ((float*)pContext->GetProjection())[i];
	}

	m_oFrustum.CreateFrom(pContext->GetProjection() * pContext->GetView());

	// Optimization opportunity: shrink the view frustum to the edges and surface of the mirror?

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
}

void CReflectionRenderer::FinishRendering(class CRenderingContext* pContext)
{
	BaseClass::FinishRendering(pContext);

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;
}

void CReflectionRenderer::StartRenderingReflection(class CRenderingContext* pContext, CMirror* pMirror)
{
	pContext->SetProjection(Matrix4x4::ProjectPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		));

	Vector vecMirror = pMirror->GetGlobalOrigin();
	Vector vecMirrorForward = pMirror->GetGlobalTransform().GetForwardVector();

	Vector vecCameraPosition = m_vecCameraPosition;
	Vector vecCameraDirection = m_vecCameraDirection;
	Vector vecCameraUp = m_vecCameraUp;

	CReflectionCharacter* pPlayerCharacter = ReflectionGame()->GetLocalPlayerCharacter();

	Matrix4x4 mReflect = pPlayerCharacter->GetReflectionMatrix();

	Matrix4x4 mTranslate, mInverseTranslate;
	mTranslate.SetTranslation(vecMirror);
	mInverseTranslate = mTranslate.InvertedRT();

	vecCameraPosition = mTranslate * (mReflect * (mInverseTranslate * m_vecCameraPosition));
	vecCameraDirection = mTranslate.TransformVector(mReflect.TransformVector(mInverseTranslate.TransformVector(m_vecCameraDirection)));
	vecCameraUp = mReflect * m_vecCameraUp;

	mReflect *= pMirror->GetReflection();

	Matrix4x4 mView = Matrix4x4::ConstructCameraView(vecCameraPosition, vecCameraDirection, vecCameraUp);

	// Transform to mirror's space
	mView.AddTranslation(vecMirror);

	// Do the reflection
	mView *= mReflect;

	// Transform back to global space
	mView.AddTranslation(-vecMirror);

	pContext->SetView(mView);

	for (size_t i = 0; i < 16; i++)
	{
		m_aflModelView[i] = ((float*)pContext->GetView())[i];
		m_aflProjection[i] = ((float*)pContext->GetProjection())[i];
	}

	m_oFrustum.CreateFrom(pContext->GetProjection() * pContext->GetView());

	// TODO: Optimization opportunity: shrink the view frustum to the edges and surface of the mirror

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);
}

CFrameBuffer& CReflectionRenderer::GetReflectionBuffer(size_t i)
{
	return m_aoReflectionBuffers[i];
}

bool CReflectionRenderer::ShouldRenderPhysicsDebug() const
{
	return !IsRenderingReflection();
}

CReflectionRenderer* ReflectionRenderer()
{
	return static_cast<CReflectionRenderer*>(GameServer()->GetRenderer());
}
