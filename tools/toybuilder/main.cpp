#include <stdio.h>

#include <tinker_platform.h>

#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>

void LoadSceneIntoToyPhysics(CConversionScene* pScene, CToyUtil* pToy);
bool LoadFromInputScript(CToyUtil& t, const tstring& sScript, tstring& sOutput);
void LoadFromFiles(CToyUtil& t, const tstring& sMesh, const tstring& sPhysics);

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

int main(int argc, char** args)
{
	printf("Toy Builder for Lunar Workshop's Tinker Engine\n");

	if (argc < 1)
	{
		printf("Usage: %s input.obj output.toy [--physics input.obj]\n", args[0]);
		printf("Usage: %s input.txt\n", args[0]);
		return 1;
	}

	CToyUtil t;

	tstring sInput = args[1];
	tstring sOutput;

	if (sInput.substr(sInput.length()-4) == ".txt")
	{
		if (!LoadFromInputScript(t, sInput, sOutput))
		{
			printf("Usage: %s input.txt\n", args[0]);
			printf("- input.txt must specify a game directory and an output file.\n");
			return 1;
		}
	}
	else
	{
		if (argc < 2)
		{
			printf("Usage: %s input.obj output.toy [--physics input.obj]\n", args[0]);
			return 1;
		}

		tstring sPhysics;

		for (int i = 3; i < argc; i++)
		{
			if (strcmp(args[i], "--physics") == 0)
			{
				if (argc < i+1)
				{
					printf("Usage: %s input.obj output.toy [--physics input.obj]\n", args[0]);
					return 1;
				}

				sPhysics = args[i+1];
				i++;
			}
		}

		sOutput = FindAbsolutePath(args[2]);
		t.SetOutputDirectory(sOutput.substr(0, sOutput.rfind(DIR_SEP)));

		LoadFromFiles(t, sInput, sPhysics);
	}

	printf(" Mesh materials: %d\n", t.GetNumMaterials());
	printf(" Mesh tris: %d\n", t.GetNumVerts()/3);
	printf(" Physics tris: %d\n", t.GetNumPhysIndices()/3);
	printf(" Scene areas: %d\n", t.GetNumSceneAreas());

	printf("Writing toy '%s' ...", sOutput);

	if (!t.Write(sOutput))
	{
		printf("\nError writing to file.\n");
		return 1;
	}

	printf(" Done.\n");

	printf("Toy built successfully.\n\n");

	return 0;
}

void LoadFromFiles(CToyUtil& t, const tstring& sMesh, const tstring& sPhysics)
{
	printf("Reading model '%s' ...", sMesh);
	CConversionScene* pScene = new CConversionScene();
	CModelConverter c(pScene);

	c.ReadModel(sMesh);
	printf(" Done.\n");

	printf("Building toy mesh ...");
	LoadSceneIntoToy(pScene, &t);
	printf(" Done.\n");

	delete pScene;

	if (sPhysics.length())
	{
		printf("Reading physics model '%s' ...", sPhysics);
		CConversionScene* pScene = new CConversionScene();
		CModelConverter c(pScene);

		c.ReadModel(sPhysics);
		printf(" Done.\n");

		printf("Building toy physics model ...");
		LoadSceneIntoToyPhysics(pScene, &t);
		printf(" Done.\n");

		delete pScene;
	}
}
