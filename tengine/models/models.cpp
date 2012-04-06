#include "models.h"

#include <files.h>

#include <toys/toy_util.h>
#include <toys/toy.h>
#include <renderer/renderer.h>
#include <textures/texturelibrary.h>
#include <physics/physics.h>
#include <tinker/application.h>

CModelLibrary* CModelLibrary::s_pModelLibrary = NULL;
static CModelLibrary g_ModelLibrary = CModelLibrary();

CModelLibrary::CModelLibrary()
{
	m_iModelsLoaded = 0;
	s_pModelLibrary = this;
}

CModelLibrary::~CModelLibrary()
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		if (m_apModels[i])
		{
			m_apModels[i]->m_iReferences = 0;
			delete m_apModels[i];
		}
	}

	s_pModelLibrary = NULL;
}

size_t CModelLibrary::AddModel(const tstring& sModel)
{
	if (!sModel.length())
		return ~0;

	size_t iModel = FindModel(sModel);
	if (iModel != ~0)
	{
		CModel* pModel = Get()->m_apModels[iModel];
		pModel->m_iReferences++;

		if (pModel->m_pToy)
		{
			for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
				CModelLibrary::AddModel(pModel->m_pToy->GetSceneAreaFileName(i));
		}

		return iModel;
	}

	CModel* pModel = new CModel(sModel);

	size_t iLocation = ~0;
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		if (!Get()->m_apModels[i])
		{
			iLocation = i;
			break;
		}
	}

	if (iLocation == ~0)
	{
		iLocation = Get()->m_apModels.size();
		Get()->m_apModels.push_back();
	}

	Get()->m_apModels[iLocation] = pModel;

	if (!pModel->Load())
	{
		Get()->m_apModels[iLocation] = nullptr;
		delete pModel;
		return ~0;
	}

	pModel->m_iReferences++;

	Get()->m_iModelsLoaded++;

	if (pModel->m_pToy)
	{
		for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
		{
			if (CModelLibrary::AddModel(pModel->m_pToy->GetSceneAreaFileName(i)) == ~0)
				TError(tstring("Area \"") + pModel->m_pToy->GetSceneAreaFileName(i) + "\" for model \"" + sModel + "\" could not be loaded.");
		}
	}

	return iLocation;
}

CModel* CModelLibrary::GetModel(size_t i)
{
	if (i >= Get()->m_apModels.size())
		return NULL;

	return Get()->m_apModels[i];
}

size_t CModelLibrary::FindModel(const tstring& sModel)
{
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		if (!Get()->m_apModels[i])
			continue;

		if (Get()->m_apModels[i]->m_sFilename == sModel)
			return i;
	}

	return ~0;
}

void CModelLibrary::ReleaseModel(const tstring& sModel)
{
	ReleaseModel(FindModel(sModel));
}

void CModelLibrary::ReleaseModel(size_t i)
{
	CModel* pModel = GetModel(i);

	if (!pModel)
		return;

	TAssert(pModel->m_iReferences > 0);
	if (pModel->m_iReferences)
		pModel->m_iReferences--;
}

void CModelLibrary::UnloadModel(size_t i)
{
	CModel* pModel = GetModel(i);

	if (!pModel)
		return;

	pModel->m_iReferences = 0;

	delete pModel;
	Get()->m_apModels[i] = nullptr;
	Get()->m_iModelsLoaded--;
}

void CModelLibrary::ResetReferenceCounts()
{
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		if (!Get()->m_apModels[i])
			continue;

		Get()->m_apModels[i]->m_iReferences = 0;
	}
}

void CModelLibrary::ClearUnreferenced()
{
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		CModel* pModel = Get()->m_apModels[i];
		if (!pModel)
			continue;

		if (!pModel->m_iReferences)
		{
			delete pModel;
			Get()->m_apModels[i] = nullptr;
			Get()->m_iModelsLoaded--;
		}
	}
}

void CModelLibrary::LoadAllIntoPhysics()
{
	for (size_t i = 0; i < Get()->m_apModels.size(); i++)
	{
		CModel* pModel = Get()->m_apModels[i];
		if (!pModel)
			continue;

		if (pModel->m_pToy)
		{
			if (pModel->m_pToy->GetPhysicsNumTris())
				GamePhysics()->LoadCollisionMesh(pModel->m_sFilename, pModel->m_pToy->GetPhysicsNumTris(), pModel->m_pToy->GetPhysicsTris(), pModel->m_pToy->GetPhysicsNumVerts(), pModel->m_pToy->GetPhysicsVerts());
		}
	}
}

CModel::CModel(const tstring& sFilename)
{
	m_iReferences = 0;
	m_sFilename = sFilename;
	m_pToy = nullptr;
}

CModel::~CModel()
{
	TAssert(m_iReferences == 0);

	if (m_pToy && m_pToy->GetPhysicsNumTris())
		GamePhysics()->UnloadCollisionMesh(m_sFilename);

	for (size_t i = 0; i < m_aiVertexBuffers.size(); i++)
	{
		if (m_aiVertexBufferSizes[i] == 0)
			continue;

		UnloadBufferFromGL(m_aiVertexBuffers[i]);
	}

	if (m_pToy)
		delete m_pToy;
}

bool CModel::Load()
{
	m_pToy = new CToy();
	CToyUtil t;
	if (!t.Read(m_sFilename, m_pToy))
	{
		delete m_pToy;
		m_pToy = nullptr;

		return LoadSourceFile();
	}

	size_t iMaterials = m_pToy->GetNumMaterials();

	m_ahTextures.resize(iMaterials);
	m_aiVertexBuffers.resize(iMaterials);
	m_aiVertexBufferSizes.resize(iMaterials);

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (m_pToy->GetMaterialNumVerts(i) == 0)
			continue;

		m_aiVertexBuffers[i] = LoadBufferIntoGL(i);
		m_aiVertexBufferSizes[i] = m_pToy->GetMaterialNumVerts(i);
		m_ahTextures[i] = CTextureLibrary::AddTexture(m_pToy->GetMaterialTexture(i));

		if (!m_ahTextures[i].IsValid())
			m_ahTextures[i] = CTextureLibrary::AddTexture(GetDirectory(m_sFilename) + "/" + m_pToy->GetMaterialTexture(i));

		//TAssert(m_aiTextures[i]);
		if (!m_ahTextures[i].IsValid())
			TError(tstring("Couldn't find texture \"") + m_pToy->GetMaterialTexture(i) + "\"\n");
	}

	if (m_pToy->GetPhysicsNumTris())
		GamePhysics()->LoadCollisionMesh(m_sFilename, m_pToy->GetPhysicsNumTris(), m_pToy->GetPhysicsTris(), m_pToy->GetPhysicsNumVerts(), m_pToy->GetPhysicsVerts());

	m_pToy->DeallocateMesh();

	m_aabbBoundingBox = m_pToy->GetAABB();

	return true;
}

size_t CModel::LoadBufferIntoGL(size_t iMaterial)
{
	return CRenderer::LoadVertexDataIntoGL(m_pToy->GetMaterialNumVerts(iMaterial)*m_pToy->GetVertexSize(), m_pToy->GetMaterialVerts(iMaterial));
}

void CModel::UnloadBufferFromGL(size_t iBuffer)
{
	CRenderer::UnloadVertexDataFromGL(iBuffer);
}
