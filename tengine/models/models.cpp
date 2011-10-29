#include "models.h"

#include <modelconverter/modelconverter.h>
#include <renderer/renderer.h>
#include <models/texturelibrary.h>
#include <game/physics.h>

CModelLibrary* CModelLibrary::s_pModelLibrary = NULL;
static CModelLibrary g_ModelLibrary = CModelLibrary();

CModelLibrary::CModelLibrary()
{
	s_pModelLibrary = this;
}

CModelLibrary::~CModelLibrary()
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		delete m_apModels[i];
	}

	s_pModelLibrary = NULL;
}

size_t CModelLibrary::AddModel(const tstring& sModel)
{
	if (!sModel.length())
		return 0;

	size_t iModel = FindModel(sModel);
	if (iModel != ~0)
		return iModel;

	CModel* pModel = new CModel(sModel);
	m_apModels.push_back(pModel);
	pModel->Load();

	return m_apModels.size()-1;
}

CModel* CModelLibrary::GetModel(size_t i)
{
	if (i >= m_apModels.size())
		return NULL;

	return m_apModels[i];
}

size_t CModelLibrary::FindModel(const tstring& sModel)
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		if (m_apModels[i]->m_sFilename == sModel)
			return i;
	}

	return ~0;
}

CModel::CModel(const tstring& sFilename)
{
	m_sFilename = sFilename;
}

CModel::~CModel()
{
}

void CModel::Load()
{
	m_pScene = new CConversionScene();
	CModelConverter c(m_pScene);
	c.SetWantEdges(false);
	c.ReadModel(m_sFilename);

	size_t iMaterials = m_pScene->GetNumMaterials();
	LoadSceneIntoBuffer();

	m_aiTextures.resize(iMaterials);
	m_aiVertexBuffers.resize(iMaterials);
	m_aiVertexBufferSizes.resize(iMaterials);

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (m_aaVertices[i].size() == 0)
			continue;

		m_aiVertexBuffers[i] = LoadBufferIntoGL(i);
		m_aiTextures[i] = LoadTextureIntoGL(i);
		m_aiVertexBufferSizes[i] = m_aaVertices[i].size();
	}

	GamePhysics()->LoadCollisionMesh(m_sFilename, m_aaVertices);

	m_aaVertices.set_capacity(0);

	m_aabbBoundingBox = m_pScene->m_oExtends;

	delete m_pScene;
	m_pScene = NULL;
}

void CModel::LoadSceneIntoBuffer()
{
	for (size_t i = 0; i < m_pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoBuffer(m_pScene->GetScene(i));
}

void CModel::LoadSceneNodeIntoBuffer(CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	Matrix4x4 mTransformations = mParentTransformations;

	bool bTransformationsIdentity = false;
	if (pNode->m_mTransformations.IsIdentity())
		bTransformationsIdentity = true;

	if (!bTransformationsIdentity)
	{
		TAssert(!"Not entirely sure if this code works. Hasn't been tested.");
		mTransformations = mParentTransformations * pNode->m_mTransformations;
	}

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		LoadSceneNodeIntoBuffer(pNode->GetChild(i), mTransformations);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoBuffer(pNode->GetMeshInstance(m), mTransformations);
}

void CModel::LoadMeshInstanceIntoBuffer(CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations)
{
	if (!pMeshInstance->IsVisible())
		return;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	for (size_t m = 0; m < m_pScene->GetNumMaterials(); m++)
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

			while (m_aaVertices.size() <= pConversionMaterialMap->m_iMaterial)
				m_aaVertices.push_back();

			eastl::vector<Vertex_t>& aVertices = m_aaVertices[pConversionMaterialMap->m_iMaterial];

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			Vertex_t v;
			for (k = 2; k < pFace->GetNumVertices(); k++)
			{
				v.vecPosition = pMesh->GetVertex(pVertex0->v);
				v.vecNormal = pMesh->GetNormal(pVertex0->vn);
				v.vecUV = pMesh->GetUV(pVertex0->vu);
				v.clrColor = Color(255, 255, 255, 255);

				aVertices.push_back(v);

				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);

				v.vecPosition = pMesh->GetVertex(pVertex1->v);
				v.vecNormal = pMesh->GetNormal(pVertex1->vn);
				v.vecUV = pMesh->GetUV(pVertex1->vu);
				v.clrColor = Color(255, 255, 255, 255);

				aVertices.push_back(v);

				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				v.vecPosition = pMesh->GetVertex(pVertex2->v);
				v.vecNormal = pMesh->GetNormal(pVertex2->vn);
				v.vecUV = pMesh->GetUV(pVertex2->vu);
				v.clrColor = Color(255, 255, 255, 255);

				aVertices.push_back(v);
			}
		}
	}
}

size_t CModel::LoadBufferIntoGL(size_t iMaterial)
{
	return CRenderer::LoadVertexDataIntoGL(m_aaVertices[iMaterial]);
}

size_t CModel::LoadTextureIntoGL(size_t iMaterial)
{
	return CTextureLibrary::AddTextureID(m_pScene->GetMaterial(iMaterial)->GetDiffuseTexture());
}
