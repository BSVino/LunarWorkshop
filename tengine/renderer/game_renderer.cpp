#include "game_renderer.h"

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
#include <textures/texturelibrary.h>
#include <game/camera.h>
#include <physics/physics.h>
#include <toys/toy.h>

#include "game_renderingcontext.h"

CVar r_batch("r_batch", "1");

CGameRenderer::CGameRenderer(size_t iWidth, size_t iHeight)
	: CRenderer(iWidth, iHeight)
{
	TMsg("Initializing game renderer\n");

	m_bBatching = false;
	m_bDrawBackground = false;

	m_bBatchThisFrame = r_batch.GetBool();

	DisableSkybox();

	m_pRendering = nullptr;
}

void CGameRenderer::LoadShaders()
{
	BaseClass::LoadShaders();

	CShaderLibrary::AddShader("skybox", "skybox", "skybox");
}

void CGameRenderer::SetupFrame()
{
	TPROF("CGameRenderer::SetupFrame");

	m_bBatchThisFrame = r_batch.GetBool();

	BaseClass::SetupFrame();

	if (m_iSkyboxFT != ~0)
		DrawSkybox();
}

void CGameRenderer::DrawSkybox()
{
	TPROF("CGameRenderer::DrawSkybox");

	TAssert(false);	// Hasn't been tested since the 3.0 port

	CCamera* pCamera = GameServer()->GetCamera();

	SetCameraPosition(pCamera->GetCameraPosition());
	SetCameraTarget(pCamera->GetCameraTarget());
	SetCameraUp(pCamera->GetCameraUp());
	SetCameraFOV(pCamera->GetCameraFOV());
	SetCameraNear(pCamera->GetCameraNear());
	SetCameraFar(pCamera->GetCameraFar());

	m_mProjection.SetPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		);

	m_mView.SetOrientation(m_vecCameraTarget - m_vecCameraPosition, m_vecCameraUp);
	m_mView.SetTranslation(m_vecCameraPosition);

	glEnable(GL_CULL_FACE);

	{
		CRenderingContext c(this);

		c.SetDepthTest(false);
		c.UseProgram("skybox");

		ModifySkyboxContext(&c);

		c.BeginRenderVertexArray();
		c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
		c.SetPositionBuffer(&m_avecSkyboxFT[0][0]);
		c.BindTexture(m_iSkyboxFT);
		c.EndRenderVertexArray(6);

		c.BeginRenderVertexArray();
		c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
		c.SetPositionBuffer(&m_avecSkyboxBK[0][0]);
		c.BindTexture(m_iSkyboxBK);
		c.EndRenderVertexArray(6);

		c.BeginRenderVertexArray();
		c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
		c.SetPositionBuffer(&m_avecSkyboxLF[0][0]);
		c.BindTexture(m_iSkyboxLF);
		c.EndRenderVertexArray(6);

		c.BeginRenderVertexArray();
		c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
		c.SetPositionBuffer(&m_avecSkyboxRT[0][0]);
		c.BindTexture(m_iSkyboxRT);
		c.EndRenderVertexArray(6);

		c.BeginRenderVertexArray();
		c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
		c.SetPositionBuffer(&m_avecSkyboxUP[0][0]);
		c.BindTexture(m_iSkyboxUP);
		c.EndRenderVertexArray(6);

		c.BeginRenderVertexArray();
		c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
		c.SetPositionBuffer(&m_avecSkyboxDN[0][0]);
		c.BindTexture(m_iSkyboxDN);
		c.EndRenderVertexArray(6);
	}

	glClear(GL_DEPTH_BUFFER_BIT);
}

CVar show_physics("debug_show_physics", "no");

void CGameRenderer::FinishRendering()
{
	TPROF("CGameRenderer::FinishRendering");

	BaseClass::FinishRendering();

	if (show_physics.GetBool() && ShouldRenderPhysicsDebug())
		GamePhysics()->DebugDraw(show_physics.GetInt());
}

void CGameRenderer::SetSkybox(size_t ft, size_t bk, size_t lf, size_t rt, size_t up, size_t dn)
{
	TAssert(false); // Not ported to GL3. Needs to be converted from quads to tris.

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

void CGameRenderer::DisableSkybox()
{
	m_iSkyboxFT = ~0;
}

void CGameRenderer::BeginBatching()
{
	if (!ShouldBatchThisFrame())
		return;

	m_bBatching = true;

	for (eastl::map<size_t, eastl::vector<CRenderBatch> >::iterator it = m_aBatches.begin(); it != m_aBatches.end(); it++)
		it->second.clear();
}

void CGameRenderer::AddToBatch(class CModel* pModel, const CBaseEntity* pEntity, const Matrix4x4& mTransformations, const Color& clrRender, bool bClrSwap, const Color& clrSwap, bool bReverseWinding)
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

void CGameRenderer::RenderBatches()
{
	TPROF("CGameRenderer::RenderBatches");

	m_bBatching = false;

	if (!ShouldBatchThisFrame())
		return;

	CGameRenderingContext c(this);

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

void CGameRenderer::ClassifySceneAreaPosition(CModel* pModel)
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

size_t CGameRenderer::GetSceneAreaPosition(CModel* pModel)
{
	auto it = m_aiCurrentSceneAreas.find(pModel->m_sFilename);

	if (it == m_aiCurrentSceneAreas.end())
		return ~0;

	return it->second;
}

void CGameRenderer::FindSceneAreaPosition(CModel* pModel)
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

void CGameRenderer::SetupShader(CRenderingContext* c, CModel* pModel, size_t iMaterial)
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
