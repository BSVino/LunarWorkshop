#include <stdio.h>

#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>

void LoadSceneIntoToyPhysics(CConversionScene* pScene, CToyUtil* pToy);

void LoadMeshInstanceIntoToy(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
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

			while (pToy->GetNumMaterials() <= pConversionMaterialMap->m_iMaterial)
				pToy->AddMaterial(pScene->GetMaterial(pConversionMaterialMap->m_iMaterial)->GetDiffuseTexture());

			size_t iMaterial = pConversionMaterialMap->m_iMaterial;

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				pToy->AddVertex(iMaterial, pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
				pToy->AddVertex(iMaterial, pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
				pToy->AddVertex(iMaterial, pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));
			}
		}
	}
}

void LoadSceneNodeIntoToy(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
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
		LoadSceneNodeIntoToy(pScene, pNode->GetChild(i), mTransformations, pToy);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoToy(pScene, pNode->GetMeshInstance(m), mTransformations, pToy);
}

void LoadSceneIntoToy(CConversionScene* pScene, CToyUtil* pToy)
{
	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoToy(pScene, pScene->GetScene(i), Matrix4x4(), pToy);
}

char* g_pszPhys = nullptr;

int main(int argc, char** args)
{
	printf("Toy Builder for Lunar Workshop's Tinker Engine\n");

	if (argc < 3)
	{
		printf("Usage: %s input.obj output.toy [--physics input.obj]\n", args[0]);
		return 1;
	}

	for (int i = 3; i < argc; i++)
	{
		if (strcmp(args[i], "--physics") == 0)
		{
			if (argc < i+1)
			{
				printf("Usage: %s input.obj output.toy [--physics input.obj]\n", args[0]);
				return 1;
			}

			g_pszPhys = args[i+1];
			i++;
		}
	}

	printf("Reading model '%s' ...", args[1]);
	CConversionScene* pScene = new CConversionScene();
	CModelConverter c(pScene);

	c.ReadModel(args[1]);
	printf(" Done.\n");

	printf("Building toy ...", args[2]);
	CToyUtil t;
	LoadSceneIntoToy(pScene, &t);
	printf(" Done.\n");

	if (g_pszPhys)
	{
		printf("Reading physics model '%s' ...", g_pszPhys);
		CConversionScene* pScene = new CConversionScene();
		CModelConverter c(pScene);

		c.ReadModel(g_pszPhys);
		printf(" Done.\n");

		printf("Building physics model ...");
		LoadSceneIntoToyPhysics(pScene, &t);
		printf(" Done.\n");

		delete pScene;
	}

	printf(" Materials: %d\n", t.GetNumMaterials());
	printf(" Vertices: %d\n", t.GetNumVerts());
	printf(" Physics tris: %d\n", t.GetNumPhysIndices()/3);

	printf("Writing '%s' ...", args[1]);

	if (!t.Write(args[2]))
	{
		printf("\nError writing to file.\n");
		return 1;
	}

	printf(" Done.\n");

	delete pScene;

	printf("Toy built successfully.\n\n");

	return 0;
}
