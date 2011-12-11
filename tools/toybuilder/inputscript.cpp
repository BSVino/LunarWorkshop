#include <tinker_platform.h>
#include <files.h>

#include <datamanager/dataserializer.h>
#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>

void LoadSceneIntoToy(CConversionScene* pScene, CToyUtil* pToy);
void LoadSceneIntoToyPhysics(CConversionScene* pScene, CToyUtil* pToy);
void LoadSceneNodeIntoToy(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy);
void LoadSceneNodeIntoToyPhysics(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy);

void LoadSceneAreas(CToyUtil& t, CData* pData)
{
	eastl::map<tstring, std::shared_ptr<CConversionScene> > asScenes;
	eastl::map<tstring, size_t> aiSceneIDs;

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pArea = pData->GetChild(i);

		if (pArea->GetKey() == "NeighborDistance")
		{
			t.SetNeighborDistance(pArea->GetValueFloat());
			continue;
		}

		TAssert(pArea->GetKey() == "Area");
		if (pArea->GetKey() != "Area")
			continue;

		tstring sFile = pArea->FindChildValueTString("File");
		TAssert(sFile.length());
		if (!sFile.length())
			continue;

		tstring sMesh = pArea->FindChildValueTString("Mesh");
		TAssert(sMesh.length());
		if (!sMesh.length())
			continue;

		tstring sPhysics = pArea->FindChildValueTString("Physics");
		TAssert(sPhysics.length());
		if (!sPhysics.length())
			continue;

		auto it = asScenes.find(sFile);
		if (it == asScenes.end())
		{
			printf("Reading model '%s' ...", sFile);
			std::shared_ptr<CConversionScene> pScene(new CConversionScene());
			CModelConverter c(pScene.get());

			c.ReadModel(sFile);
			printf(" Done.\n");

			asScenes[sFile] = pScene;
		}

		CToyUtil ts;

		CConversionSceneNode* pMeshNode = asScenes[sFile]->FindSceneNode(sMesh);
		CConversionSceneNode* pPhysicsNode = asScenes[sFile]->FindSceneNode(sPhysics);

		TAssert(pMeshNode);
		TAssert(pPhysicsNode);

		printf("Building scene area toy ...");

		if (pMeshNode)
			LoadSceneNodeIntoToy(asScenes[sFile].get(), pMeshNode, Matrix4x4(), &ts);
		else
			TError("Couldn't find a scene node in '" + sFile + "' named '" + sMesh + "'\n");

		if (pPhysicsNode)
			LoadSceneNodeIntoToyPhysics(asScenes[sFile].get(), pPhysicsNode, Matrix4x4(), &ts);
		else
			TError("Couldn't find a scene node in '" + sFile + "' named '" + sMesh + "'\n");

		printf(" Done.\n");

		tstring sGameOutput = pArea->FindChildValueTString("Output");
		if (!sGameOutput.length())
			sGameOutput = sprintf(t.GetOutputDirectory() + "/" + t.GetOutputFile() + "_sa%d_" + tolower(pArea->GetValueTString()) + ".toy", i);

		tstring sFileOutput = FindAbsolutePath(t.GetGameDirectory() + DIR_SEP + sGameOutput);

		printf(" Mesh materials: %d\n", ts.GetNumMaterials());
		printf(" Mesh tris: %d\n", ts.GetNumVerts()/3);
		printf(" Physics tris: %d\n", ts.GetNumPhysIndices()/3);

		printf("Writing scene area toy '%s' ...", sFileOutput);
		if (ts.Write(sFileOutput))
			printf(" Done.\n");
		else
			printf(" FAILED!\n");

		aiSceneIDs[pArea->GetValueTString()] = t.AddSceneArea(sGameOutput);
	}

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pArea = pData->GetChild(i);

		if (pArea->GetKey() != "Area")
			continue;

		size_t iArea = aiSceneIDs[pArea->GetValueTString()];

		for (size_t i = 0; i < pArea->GetNumChildren(); i++)
		{
			CData* pNeighbor = pArea->GetChild(i);

			if (pNeighbor->GetKey() == "Neighbor")
			{
				TAssert(aiSceneIDs.find(pNeighbor->GetValueTString()) != aiSceneIDs.end());
				if (aiSceneIDs.find(pNeighbor->GetValueTString()) == aiSceneIDs.end())
				{
					TError("Couldn't find area \"" + pNeighbor->GetValueTString() + "\"\n");
					continue;
				}

				t.AddSceneAreaNeighbor(iArea, aiSceneIDs[pNeighbor->GetValueTString()]);
				continue;
			}

			if (pNeighbor->GetKey() == "Visible")
			{
				TAssert(aiSceneIDs.find(pNeighbor->GetValueTString()) != aiSceneIDs.end());
				if (aiSceneIDs.find(pNeighbor->GetValueTString()) == aiSceneIDs.end())
				{
					TError("Couldn't find area \"" + pNeighbor->GetValueTString() + "\"\n");
					continue;
				}

				t.AddSceneAreaVisible(iArea, aiSceneIDs[pNeighbor->GetValueTString()]);
				continue;
			}
		}
	}
}

bool LoadFromInputScript(CToyUtil& t, const tstring& sScript, tstring& sOutput)
{
	std::basic_ifstream<tchar> f(sScript.c_str());
	if (!f.is_open())
		return false;

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CData* pOutput = pData->FindChild("Output");
	if (!pOutput)
	{
		delete pData;
		return false;
	}

	CData* pGame = pData->FindChild("Game");
	if (!pGame)
	{
		delete pData;
		return false;
	}

	t.SetGameDirectory(FindAbsolutePath(pGame->GetValueTString()));

	tstring sOutputDir = str_replace(pOutput->GetValueTString(), "\\", "/");
	t.SetOutputDirectory(GetDirectory(sOutputDir));
	t.SetOutputFile(GetFilename(sOutputDir));

	sOutput = FindAbsolutePath(t.GetGameDirectory() + DIR_SEP + pOutput->GetValueTString());

	CData* pMesh = pData->FindChild("Mesh");
	if (pMesh)
	{
		printf("Reading model '%s' ...", pMesh->GetValueTString());
		CConversionScene* pScene = new CConversionScene();
		CModelConverter c(pScene);

		c.ReadModel(pMesh->GetValueTString());
		printf(" Done.\n");

		printf("Building toy mesh ...");
		LoadSceneIntoToy(pScene, &t);
		printf(" Done.\n");

		delete pScene;
	}

	CData* pPhysics = pData->FindChild("Physics");
	if (pPhysics)
	{
		printf("Reading physics model '%s' ...", pPhysics->GetValueTString());
		CConversionScene* pScene = new CConversionScene();
		CModelConverter c(pScene);

		c.ReadModel(pPhysics->GetValueTString());
		printf(" Done.\n");

		printf("Building toy physics model ...");
		LoadSceneIntoToyPhysics(pScene, &t);
		printf(" Done.\n");

		delete pScene;
	}

	CData* pSceneAreas = pData->FindChild("SceneAreas");
	if (pSceneAreas)
		LoadSceneAreas(t, pSceneAreas);

	delete pData;

	return true;
}
