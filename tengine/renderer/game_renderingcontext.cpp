#include "game_renderingcontext.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <maths.h>
#include <simplex.h>

#include <models/models.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <textures/texturelibrary.h>
#include <renderer/renderer.h>
#include <toys/toy.h>

#include "game_renderer.h"

CGameRenderingContext::CGameRenderingContext(CGameRenderer* pRenderer, bool bInherit)
	: CRenderingContext(pRenderer, bInherit)
{
	m_pRenderer = pRenderer;
}

void CGameRenderingContext::RenderModel(size_t iModel, const CBaseEntity* pEntity)
{
	CModel* pModel = CModelLibrary::GetModel(iModel);

	if (!pModel)
		return;

	bool bBatchThis = true;
	if (!m_pRenderer->IsBatching())
		bBatchThis = false;
	else if (m_pRenderer && GetContext().m_pFrameBuffer != m_pRenderer->GetSceneBuffer())
		bBatchThis = false;

	if (bBatchThis)
	{
		TAssert(GetContext().m_eBlend == BLEND_NONE);

		m_pRenderer->AddToBatch(pModel, pEntity, GetContext().m_mTransformations, m_clrRender, GetContext().m_bWinding);
	}
	else
	{
		m_pRenderer->m_pRendering = pEntity;

		for (size_t m = 0; m < pModel->m_aiVertexBuffers.size(); m++)
		{
			if (!pModel->m_aiVertexBufferSizes[m])
				continue;

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, (GLuint)pModel->m_aiTextures[m]);

			RenderModel(pModel, m);
		}

		m_pRenderer->m_pRendering = nullptr;
	}

	if (pModel->m_pToy->GetNumSceneAreas())
	{
		size_t iSceneArea = m_pRenderer->GetSceneAreaPosition(pModel);

		if (iSceneArea >= pModel->m_pToy->GetNumSceneAreas())
		{
			for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
			{
				AABB aabbBounds = pModel->m_pToy->GetSceneAreaAABB(i);
				if (!m_pRenderer->IsSphereInFrustum(aabbBounds.Center(), aabbBounds.Size().Length()/2))
					continue;

				RenderModel(CModelLibrary::FindModel(pModel->m_pToy->GetSceneAreaFileName(i)), pEntity);
			}
		}
		else
		{
			for (size_t i = 0; i < pModel->m_pToy->GetSceneAreaNumVisible(iSceneArea); i++)
			{
				size_t iSceneAreaToRender = pModel->m_pToy->GetSceneAreasVisible(iSceneArea, i);

				AABB aabbBounds = pModel->m_pToy->GetSceneAreaAABB(iSceneAreaToRender);
				if (!m_pRenderer->IsSphereInFrustum(aabbBounds.Center(), aabbBounds.Size().Length()/2))
					continue;

				RenderModel(CModelLibrary::FindModel(pModel->m_pToy->GetSceneAreaFileName(iSceneAreaToRender)), pEntity);
			}
		}
	}
}

#define BUFFER_OFFSET(i) (size_t)((char *)NULL + (i))

void CGameRenderingContext::RenderModel(CModel* pModel, size_t iMaterial)
{
	m_pRenderer->SetupShader(this, pModel, iMaterial);

	TAssert(m_pShader);
	if (!m_pShader)
		return;

	if (!pModel || !m_pShader)
		return;

	Vector vecCameraPosition = m_pRenderer->m_vecCameraPosition;
	Vector vecCameraDirection = m_pRenderer->m_vecCameraDirection;
	Vector vecCameraUp = m_pRenderer->m_vecCameraUp;

	BeginRenderVertexArray(pModel->m_aiVertexBuffers[iMaterial]);
	SetPositionBuffer(0u, pModel->m_pToy->GetVertexSize());
	SetTexCoordBuffer(BUFFER_OFFSET(pModel->m_pToy->GetVertexUV()), pModel->m_pToy->GetVertexSize());
	EndRenderVertexArray(pModel->m_aiVertexBufferSizes[iMaterial]);
}

void CGameRenderingContext::RenderBillboard(const tstring& sTexture, float flRadius)
{
	Vector vecUp, vecRight;
	m_pRenderer->GetCameraVectors(NULL, &vecRight, &vecUp);

	BaseClass::RenderBillboard(sTexture, flRadius, vecUp, vecRight);
}
