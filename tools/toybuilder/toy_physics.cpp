#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>

void LoadMeshInstanceIntoToyPhysics(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
{
	if (!pMeshInstance->IsVisible())
		return;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	for (size_t v = 0; v < pMesh->GetNumVertices(); v++)
		pToy->AddPhysVertex(pMesh->GetVertex(v));

	for (size_t m = 0; m < pScene->GetNumMaterials(); m++)
	{
		for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
		{
			size_t k;
			CConversionFace* pFace = pMesh->GetFace(j);

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				pToy->AddPhysTriangle(pVertex0->v, pVertex1->v, pVertex2->v);
			}
		}
	}
}

void LoadSceneNodeIntoToyPhysics(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
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
		LoadSceneNodeIntoToyPhysics(pScene, pNode->GetChild(i), mTransformations, pToy);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoToyPhysics(pScene, pNode->GetMeshInstance(m), mTransformations, pToy);
}

void LoadSceneIntoToyPhysics(CConversionScene* pScene, CToyUtil* pToy)
{
	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoToyPhysics(pScene, pScene->GetScene(i), Matrix4x4(), pToy);
}

