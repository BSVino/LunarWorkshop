#include "models.h"

#include <stdio.h>

#include <tinker_platform.h>
#include <files.h>

#include <modelconverter/modelconverter.h>
#include <renderer/renderer.h>
#include <textures/texturelibrary.h>
#include <tinker/application.h>

eastl::vector<tstring>					g_asTextures;
eastl::vector<eastl::vector<float> >	g_aaflData;
AABB									g_aabbBounds;

void AddVertex(size_t iMaterial, const Vector& v, const Vector2D& vt)
{
	g_aaflData[iMaterial].push_back(v.x);
	g_aaflData[iMaterial].push_back(v.y);
	g_aaflData[iMaterial].push_back(v.z);
	g_aaflData[iMaterial].push_back(vt.x);
	g_aaflData[iMaterial].push_back(vt.y);

	for (int i = 0; i < 3; i++)
	{
		if (v[i] < g_aabbBounds.m_vecMins[i])
			g_aabbBounds.m_vecMins[i] = v[i];
		if (v[i] > g_aabbBounds.m_vecMaxs[i])
			g_aabbBounds.m_vecMaxs[i] = v[i];
	}
}

void LoadMeshInstanceIntoToy(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations)
{
	if (!pMeshInstance->IsVisible())
		return;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	for (size_t m = 0; m < pScene->GetNumMaterials(); m++)
	{
		for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
		{
			size_t k;
			CConversionFace* pFace = pMesh->GetFace(j);

			if (pFace->m == ~0)
				continue;

			CConversionMaterial* pMaterial = NULL;
			CConversionMaterialMap* pConversionMaterialMap = pMeshInstance->GetMappedMaterial(pFace->m);

			if (!pConversionMaterialMap)
				continue;

			if (!pConversionMaterialMap->IsVisible())
				continue;

			if (pConversionMaterialMap->m_iMaterial != m)
				continue;

			while (g_asTextures.size() <= pConversionMaterialMap->m_iMaterial)
			{
				g_asTextures.push_back(pScene->GetMaterial(pConversionMaterialMap->m_iMaterial)->GetDiffuseTexture());
				g_aaflData.push_back();
			}

			size_t iMaterial = pConversionMaterialMap->m_iMaterial;

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
				AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
				AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));
			}
		}
	}
}

void LoadSceneNodeIntoToy(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	Matrix4x4 mTransformations = mParentTransformations * pNode->m_mTransformations;

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		LoadSceneNodeIntoToy(pScene, pNode->GetChild(i), mTransformations);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoToy(pScene, pNode->GetMeshInstance(m), mTransformations);
}

void LoadSceneIntoToy(CConversionScene* pScene)
{
	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoToy(pScene, pScene->GetScene(i), Matrix4x4());
}

bool CModel::LoadSourceFile()
{
	CConversionScene* pScene = new CConversionScene();
	CModelConverter c(pScene);

	if (!c.ReadModel(m_sFilename))
	{
		delete pScene;
		return false;
	}

	g_asTextures.clear();
	g_aaflData.clear();
	g_aabbBounds = AABB(Vector(999, 999, 999), Vector(-999, -999, -999));

	LoadSceneIntoToy(pScene);

	size_t iMaterials = g_asTextures.size();

	m_aiTextures.resize(iMaterials);
	m_aiVertexBuffers.resize(iMaterials);
	m_aiVertexBufferSizes.resize(iMaterials);

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (g_aaflData[i].size() == 0)
			continue;

		m_aiVertexBuffers[i] = CRenderer::LoadVertexDataIntoGL(g_aaflData[i].size()*4, &g_aaflData[i][0]);
		m_aiVertexBufferSizes[i] = g_aaflData[i].size()/5;
		m_aiTextures[i] = CTextureLibrary::AddTextureID(g_asTextures[i]);

		if (!m_aiTextures[i])
			m_aiTextures[i] = CTextureLibrary::AddTextureID(GetDirectory(m_sFilename) + "/" + g_asTextures[i]);

		//TAssert(m_aiTextures[i]);
		if (!m_aiTextures[i])
			TError(tstring("Couldn't find texture \"") + g_asTextures[i] + "\"\n");
	}

	m_aabbBoundingBox = g_aabbBounds;

	delete pScene;

	return true;
}
