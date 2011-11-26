#include "models.h"

#include <files.h>

#include <toys/toy_util.h>
#include <toys/toy.h>
#include <renderer/renderer.h>
#include <models/texturelibrary.h>
#include <physics/physics.h>

CModelLibrary* CModelLibrary::s_pModelLibrary = NULL;
static CModelLibrary g_ModelLibrary = CModelLibrary();

CModelLibrary::CModelLibrary()
{
	s_pModelLibrary = this;
}

CModelLibrary::~CModelLibrary()
{
	for (size_t i = 0; i < m_apModels.size(); i++)
		delete m_apModels[i];

	s_pModelLibrary = NULL;
}

size_t CModelLibrary::AddModel(const tstring& sModel)
{
	if (!sModel.length())
		return 0;

	size_t iModel = FindModel(sModel);
	if (iModel != ~0)
	{
		Get()->m_apModels[iModel]->m_iReferences++;

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

	pModel->Load();

	pModel->m_iReferences++;

	return Get()->m_apModels.size()-1;
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
	CModel* pModel = GetModel(FindModel(sModel));

	if (!pModel)
		return;

	TAssert(pModel->m_iReferences > 0);
	pModel->m_iReferences--;
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
		if (!Get()->m_apModels[i])
			continue;

		if (!Get()->m_apModels[i]->m_iReferences)
		{
			delete Get()->m_apModels[i];
			Get()->m_apModels[i] = nullptr;
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

	if (m_pToy)
		delete m_pToy;
}

void CModel::Load()
{
	m_pToy = new CToy();
	CToyUtil t;
	t.Read(m_sFilename, m_pToy);

	size_t iMaterials = m_pToy->GetNumMaterials();

	m_aiTextures.resize(iMaterials);
	m_aiVertexBuffers.resize(iMaterials);
	m_aiVertexBufferSizes.resize(iMaterials);

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (m_pToy->GetMaterialNumVerts(i) == 0)
			continue;

		m_aiVertexBuffers[i] = LoadBufferIntoGL(i);
		m_aiTextures[i] = CTextureLibrary::AddTextureID(GetDirectory(m_sFilename) + "/" + m_pToy->GetMaterialTexture(i));
		m_aiVertexBufferSizes[i] = m_pToy->GetMaterialNumVerts(i);
	}

	if (m_pToy->GetPhysicsNumTris())
		GamePhysics()->LoadCollisionMesh(m_sFilename, m_pToy->GetPhysicsNumTris(), m_pToy->GetPhysicsTris(), m_pToy->GetPhysicsNumVerts(), m_pToy->GetPhysicsVerts());

	m_pToy->DeallocateMesh();

	m_aabbBoundingBox = m_pToy->GetAABB();
}

size_t CModel::LoadBufferIntoGL(size_t iMaterial)
{
	return CRenderer::LoadVertexDataIntoGL(m_pToy->GetMaterialNumVerts(iMaterial)*m_pToy->GetVertexSize(), m_pToy->GetMaterialVerts(iMaterial));
}
