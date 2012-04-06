#include "geppetto.h"

#include <tinker_platform.h>
#include <files.h>
#include <stb_image.h>

#include <tinker/shell.h>
#include <datamanager/dataserializer.h>
#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>

bool CGeppetto::LoadSceneAreas(CData* pData)
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

		if (pArea->GetKey() == "UseGlobalTransforms")
		{
			t.UseGlobalTransformations();
			continue;
		}

		if (pArea->GetKey() == "UseLocalTransforms")
		{
			t.UseLocalTransformations();
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
			TMsg("Reading model '" + GetPath(sFile) + "' ...");
			std::shared_ptr<CConversionScene> pScene(new CConversionScene());
			CModelConverter c(pScene.get());

			if (!c.ReadModel(GetPath(sFile)))
			{
				TError("Couldn't read '" + GetPath(sFile) + "'.\n");
				return false;
			}
			TMsg(" Done.\n");

			asScenes[sFile] = pScene;
		}

		CToyUtil ts;

		ts.SetGameDirectory(t.GetGameDirectory());
		ts.SetOutputDirectory(t.GetOutputDirectory());
		ts.SetOutputFile(sprintf(t.GetOutputFile() + "_sa%d_" + tolower(pArea->GetValueTString()), i));
		ts.UseLocalTransformations(t.IsUsingLocalTransformations());

		CConversionSceneNode* pMeshNode = asScenes[sFile]->FindSceneNode(sMesh);
		CConversionSceneNode* pPhysicsNode = asScenes[sFile]->FindSceneNode(sPhysics);

		TAssert(pMeshNode);
		TAssert(pPhysicsNode);

		TMsg("Building scene area toy ...");

		if (pMeshNode)
			LoadSceneNodeIntoToy(asScenes[sFile].get(), pMeshNode, Matrix4x4(), &ts);
		else
			TError("Couldn't find a scene node in '" + sFile + "' named '" + sMesh + "'\n");

		if (pPhysicsNode)
			LoadSceneNodeIntoToyPhysics(asScenes[sFile].get(), pPhysicsNode, Matrix4x4(), &ts);
		else
			TError("Couldn't find a scene node in '" + sFile + "' named '" + sMesh + "'\n");

		TMsg(" Done.\n");

		tstring sGameOutput = pArea->FindChildValueTString("Output");
		if (!sGameOutput.length())
			sGameOutput = t.GetOutputDirectory() + "/" + ts.GetOutputFile() + ".toy";

		tstring sFileOutput = FindAbsolutePath(t.GetGameDirectory() + DIR_SEP + sGameOutput);

		TMsg(sprintf(" Mesh materials: %d\n", ts.GetNumMaterials()));
		TMsg(sprintf(" Mesh tris: %d\n", ts.GetNumVerts()/3));
		TMsg(sprintf(" Physics tris: %d\n", ts.GetNumPhysIndices()/3));

		TMsg("Writing scene area toy '" + sFileOutput + "' ...");
		if (ts.Write(sFileOutput))
			TMsg(" Done.\n");
		else
			TMsg(" FAILED!\n");

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

	return true;
}

bool CGeppetto::BuildFromInputScript(const tstring& sScript)
{
	std::basic_ifstream<tchar> f((GetPath(sScript)).c_str());
	if (!f.is_open())
	{
		TError("Could not read input script '" + sScript + "'\n");
		return false;
	}

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CData* pOutput = pData->FindChild("Output");
	if (!pOutput)
	{
		TError("Could not find Output section in input script '" + sScript + "'\n");
		return false;
	}

	CData* pGame = pData->FindChild("Game");
	if (!pGame)
	{
		TError("Could not find Game section in input script '" + sScript + "'\n");
		return false;
	}

	t.SetGameDirectory(FindAbsolutePath(GetPath(pGame->GetValueTString())));

	tstring sOutputDir = ToForwardSlashes(pOutput->GetValueTString());
	t.SetOutputDirectory(GetDirectory(sOutputDir));
	t.SetOutputFile(GetFilename(sOutputDir));
	t.SetScriptDirectory(GetDirectory((GetPath(sScript))));

	m_sOutput = FindAbsolutePath(t.GetGameDirectory() + DIR_SEP + pOutput->GetValueTString());

	CData* pSceneAreas = pData->FindChild("SceneAreas");
	CData* pMesh = pData->FindChild("Mesh");
	CData* pPhysics = pData->FindChild("Physics");
	CData* pPhysicsShapes = pData->FindChild("PhysicsShapes");

	// Find all file modification times.
	time_t iScriptModificationTime = GetFileModificationTime(sScript.c_str());
	time_t iOutputModificationTime = GetFileModificationTime(m_sOutput.c_str());

	eastl::map<tstring, time_t> aiSceneModificationTimes;

	if (pSceneAreas)
	{
		for (size_t i = 0; i < pSceneAreas->GetNumChildren(); i++)
		{
			CData* pArea = pSceneAreas->GetChild(i);

			if (pArea->GetKey() != "Area")
				continue;

			tstring sFile = pArea->FindChildValueTString("File");
			TAssert(sFile.length());
			if (!sFile.length())
				continue;

			auto it = aiSceneModificationTimes.find(sFile);
			if (it == aiSceneModificationTimes.end())
				aiSceneModificationTimes[sFile] = GetFileModificationTime(sFile.c_str());
		}
	}

	time_t iInputModificationTime = 0;
	if (pMesh)
		iInputModificationTime = GetFileModificationTime(pMesh->GetValueTString().c_str());
	time_t iPhysicsModificationTime = 0;
	if (pPhysics)
		iPhysicsModificationTime = GetFileModificationTime(pPhysics->GetValueTString().c_str());

	bool bRecompile = false;
	if (iScriptModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (iInputModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (iPhysicsModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (m_iBinaryModificationTime > iOutputModificationTime)
		bRecompile = true;
	else
	{
		for (auto it = aiSceneModificationTimes.begin(); it != aiSceneModificationTimes.end(); it++)
		{
			if (it->second > iOutputModificationTime)
			{
				bRecompile = true;
				break;
			}
		}
	}

	if (!bRecompile)
	{
		if (m_bForceCompile)
		{
			TMsg("Forcing rebuild even though no changes detected.\n");
		}
		else
		{
			TMsg("No changes detected. Skipping '" + m_sOutput + "'.\n\n");
			return true;
		}
	}

	if (pMesh)
	{
		tstring sExtension = pMesh->GetValueTString().substr(pMesh->GetValueTString().length()-4);
		if (sExtension == ".png")
		{
			int x, y, n;
			unsigned char* pData = stbi_load((GetPath(pMesh->GetValueTString())).c_str(), &x, &y, &n, 0);

			if (!pData)
			{
				TError("Input image  '" + pMesh->GetValueTString() + "' does not exist.\n");
				return false;
			}

			stbi_image_free(pData); // Don't need it, just need the dimensions.

			Vector vecUp = Vector(0, 0.5f, 0) * (float)y/100;
			Vector vecRight = Vector(0, 0, 0.5f) * (float)x/100;

			if (IsAbsolutePath(pMesh->GetValueTString()))
				t.AddMaterial(GetPath(pMesh->GetValueTString()));
			else
				t.AddMaterial(t.GetOutputDirectory() + "/" + pMesh->GetValueTString(), GetPath(pMesh->GetValueTString()));
			t.AddVertex(0, -vecRight + vecUp, Vector2D(0.0f, 1.0f));
			t.AddVertex(0, -vecRight - vecUp, Vector2D(0.0f, 0.0f));
			t.AddVertex(0, vecRight - vecUp, Vector2D(1.0f, 0.0f));

			t.AddVertex(0, -vecRight + vecUp, Vector2D(0.0f, 1.0f));
			t.AddVertex(0, vecRight - vecUp, Vector2D(1.0f, 0.0f));
			t.AddVertex(0, vecRight + vecUp, Vector2D(1.0f, 1.0f));
		}
		else
		{
			TMsg("Reading model '" + GetPath(pMesh->GetValueTString()) + "' ...");
			std::shared_ptr<CConversionScene> pScene(new CConversionScene());
			CModelConverter c(pScene.get());

			if (!c.ReadModel(GetPath(pMesh->GetValueTString())))
			{
				TError("Couldn't read '" + GetPath(pMesh->GetValueTString()) + "'.\n");
				return false;
			}
			TMsg(" Done.\n");

			TMsg("Building toy mesh ...");
			LoadSceneIntoToy(pScene.get(), &t);
			TMsg(" Done.\n");
		}
	}

	if (pPhysics)
	{
		TMsg("Reading physics model '" + GetPath(pPhysics->GetValueTString()) + "' ...");
		std::shared_ptr<CConversionScene> pScene(new CConversionScene());
		CModelConverter c(pScene.get());

		if (!c.ReadModel(GetPath(pPhysics->GetValueTString())))
		{
			TError("Couldn't read '" + GetPath(pPhysics->GetValueTString()) + "'.\n");
			return false;
		}
		TMsg(" Done.\n");

		TMsg("Building toy physics model ...");
		LoadSceneIntoToyPhysics(pScene.get(), &t);
		TMsg(" Done.\n");
	}

	if (pPhysicsShapes)
	{
		for (size_t i = 0; i < pPhysicsShapes->GetNumChildren(); i++)
		{
			CData* pShape = pPhysicsShapes->GetChild(i);

			TAssert(pShape->GetKey() == "Box");
			if (pShape->GetKey() != "Box")
				continue;

			TRS trs = pShape->GetValueTRS();

			t.AddPhysBox(trs);
		}
	}

	if (pSceneAreas)
		LoadSceneAreas(pSceneAreas);

	return Compile();
}
