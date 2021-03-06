#include "grotto_renderer.h"

#include <GL3/gl3w.h>

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <models/models.h>
#include <tools/workbench.h>

#include "grotto_game.h"
#include "mirror.h"
#include "grotto_playercharacter.h"
#include "kaleidobeast.h"
#include "asymmetric_kinematic.h"

CGrottoRenderer::CGrottoRenderer()
	: CGameRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_pRenderingReflection = nullptr;

	for (size_t i = 0; i < 5; i++)
	{
		auto& oBuffer = CreateFrameBuffer(sprintf(tstring("mirror%d"), i), m_iWidth, m_iHeight, (fb_options_e)(FB_TEXTURE|FB_DEPTH|FB_LINEAR));
		m_aoReflectionBuffers.push_back(oBuffer);
	}
}

void CGrottoRenderer::Initialize()
{
	BaseClass::Initialize();

	m_pRenderingReflection = nullptr;
}

void CGrottoRenderer::PreRender()
{
	TPROF("CGrottoRenderer::SetupFrame");

	if (CWorkbench::IsActive())
	{
		BaseClass::PreRender();
		return;
	}

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

		m_pRenderingReflection = pMirror;

		CRenderingContext c(this);

		StartRenderingReflection(&c, pMirror);

		// Render a reflected world to our reflection buffer.
		c.UseFrameBuffer(&m_aoReflectionBuffers[i]);

		c.ClearDepth();

		CGrottoCharacter* pPlayerCharacter = GrottoGame()->GetLocalPlayerCharacter();

		if (pPlayerCharacter)
			c.SetWinding(!(pPlayerCharacter->IsReflected(REFLECTION_LATERAL) == pPlayerCharacter->IsReflected(REFLECTION_VERTICAL)));

		RenderEverything();
		FinishRendering(&c);

		m_pRenderingReflection = nullptr;
	}

	BaseClass::PreRender();
}

void CGrottoRenderer::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (CWorkbench::IsActive())
	{
		BaseClass::ModifyContext(pContext);
		return;
	}

	if (!Game()->GetNumLocalPlayers())
		return;

	CGrottoCharacter* pPlayerCharacter = GrottoGame()->GetLocalPlayerCharacter();

	if (pPlayerCharacter)
		pContext->SetWinding(pPlayerCharacter->IsReflected(REFLECTION_LATERAL) == pPlayerCharacter->IsReflected(REFLECTION_VERTICAL));
}

void CGrottoRenderer::SetupFrame(class CRenderingContext* pContext)
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		m_bDrawBackground = true;
	else
		m_bDrawBackground = false;

	BaseClass::SetupFrame(pContext);
}

void CGrottoRenderer::StartRendering(class CRenderingContext* pContext)
{
	if (CVar::GetCVarValue("game_mode") == "menu")
	{
		BaseClass::StartRendering(pContext);
		return;
	}

	if (CWorkbench::IsActive())
	{
		BaseClass::StartRendering(pContext);
		return;
	}

	CGrottoCharacter* pPlayerCharacter = GrottoGame()->GetLocalPlayerCharacter();

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

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
}

void CGrottoRenderer::FinishRendering(class CRenderingContext* pContext)
{
	BaseClass::FinishRendering(pContext);

	if (CVar::GetCVarValue("game_mode") == "menu")
		return;
}

void CGrottoRenderer::StartRenderingReflection(class CRenderingContext* pContext, CMirror* pMirror)
{
	float flNear = (pMirror->GetRenderOrigin() - m_vecCameraPosition).Length() - pMirror->GetBoundingRadius();

	flNear = max(m_flCameraNear, flNear);

	pContext->SetProjection(Matrix4x4::ProjectPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			flNear,
			m_flCameraFar
		));

	Vector vecMirror = pMirror->GetGlobalOrigin();

	Vector vecMirrorForward = pMirror->GetGlobalTransform().GetForwardVector();

	Vector vecCameraPosition = m_vecCameraPosition;
	Vector vecCameraDirection = m_vecCameraDirection;
	Vector vecCameraUp = m_vecCameraUp;

	if (pMirror->HasMoveParent())
	{
		CAsymmetricKinematic* pKinematic = dynamic_cast<CAsymmetricKinematic*>(pMirror->GetMoveParent());
		if (pKinematic)
		{
			Matrix4x4 mKinematicRenderTransform = pKinematic->GetRenderTransform();
			Vector vecMirrorLocal = pMirror->GetLocalOrigin();

			vecMirror = mKinematicRenderTransform * vecMirrorLocal;

			Matrix4x4 mKinematicTransformGlobalToLocal = pKinematic->GetGlobalTransform().InvertedRT();

			Vector vecCameraPositionLocal = mKinematicTransformGlobalToLocal * vecCameraPosition;
			vecCameraPosition = mKinematicRenderTransform * vecCameraPositionLocal;

			Vector vecCameraDirectionLocal = mKinematicTransformGlobalToLocal.TransformVector(vecCameraDirection);
			vecCameraDirection = mKinematicRenderTransform.TransformVector(vecCameraDirectionLocal);

			Vector vecCameraUpLocal = mKinematicTransformGlobalToLocal.TransformVector(vecCameraUp);
			vecCameraUp = mKinematicRenderTransform.TransformVector(vecCameraUpLocal);
		}
	}

	CGrottoCharacter* pPlayerCharacter = GrottoGame()->GetLocalPlayerCharacter();

	Matrix4x4 mReflect = pPlayerCharacter->GetReflectionMatrix();

	Matrix4x4 mTranslate, mInverseTranslate;
	mTranslate.SetTranslation(vecMirror);
	mInverseTranslate = mTranslate.InvertedRT();

	vecCameraPosition = mTranslate * (mReflect * (mInverseTranslate * vecCameraPosition));
	vecCameraDirection = mTranslate.TransformVector(mReflect.TransformVector(mInverseTranslate.TransformVector(vecCameraDirection)));
	vecCameraUp = mReflect * vecCameraUp;

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

	// Shrink the view frustum to the edges and surface of the mirror
	Vector vecMirrorFace = pMirror->GetMirrorFace();
	Vector vecOldNearNormal = m_oFrustum.p[FRUSTUM_NEAR].n;

	if (vecOldNearNormal.Dot(vecMirrorFace) < 0)
		vecMirrorFace = -vecMirrorFace;

	// Move up the near plane so it matches the surface of the mirror.
	m_oFrustum.p[FRUSTUM_NEAR] = Plane(pMirror->GetGlobalOrigin(), vecMirrorFace);

	// TODO: Optimization opportunity: shrink the view frustum to the edges of the mirror

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);
}

bool CGrottoRenderer::ModifyShader(const CBaseEntity* pEntity, class CRenderingContext* c)
{
	if (!c->GetActiveShader())
		return BaseClass::ModifyShader(pEntity, c);

	if (c->GetActiveShader()->m_sName != "model")
		return BaseClass::ModifyShader(pEntity, c);

	c->SetUniform("bRenderingReflection", false);

	if (!IsRenderingReflection())
		return BaseClass::ModifyShader(pEntity, c);

	c->SetUniform("bRenderingReflection", true);

	Vector vecMirrorFace = GetRenderingReflectionMirror()->GetMirrorFace();
	if (vecMirrorFace.Dot(c->GetView().InvertedRT().GetTranslation() - GetRenderingReflectionMirror()->GetGlobalOrigin()) < 0)
		vecMirrorFace = -vecMirrorFace;

	Plane plSurface(GetRenderingReflectionMirror()->GetGlobalOrigin(), vecMirrorFace);
	c->SetUniform("vecMirrorPlane", Vector4D(plSurface.n.x, plSurface.n.y, plSurface.n.z, plSurface.d));

	return BaseClass::ModifyShader(pEntity, c);
}

CFrameBuffer& CGrottoRenderer::GetReflectionBuffer(size_t i)
{
	return m_aoReflectionBuffers[i];
}

bool CGrottoRenderer::ShouldRenderPhysicsDebug() const
{
	return !IsRenderingReflection();
}

CGrottoRenderer* GrottoRenderer()
{
	return static_cast<CGrottoRenderer*>(GameServer()->GetRenderer());
}
