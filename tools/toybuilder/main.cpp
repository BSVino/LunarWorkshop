#include <stdio.h>

#include <tinker_platform.h>

#include <tinker/shell.h>
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

				pToy->AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
				pToy->AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
				pToy->AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));
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

	if (!pToy->IsUsingLocalTransformations())
		mTransformations = mParentTransformations * pNode->m_mTransformations;

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

time_t g_iBinaryModificationTime;

class CToyBuilder : public CShell
{
public:
	CToyBuilder(int argc, char** args) : CShell(argc, args) {};
};

int main(int argc, char** args)
{
	CToyBuilder c(argc, args);

	TMsg("Toy Builder for Lunar Workshop's Tinker Engine\n");

	if (argc <= 1)
	{
		TMsg(tstring("Usage: ") + args[0] + " input.obj output.toy [--physics input.obj]\n");
		TMsg(tstring("Usage: ") + args[0] + " input.txt\n");
		return 1;
	}

	g_iBinaryModificationTime = GetFileModificationTime(args[0]);
	if (!g_iBinaryModificationTime)
		g_iBinaryModificationTime = GetFileModificationTime((tstring(args[0]) + ".exe").c_str());

	CToyUtil t;

	tstring sInput = args[1];
	tstring sOutput;

	if (sInput.substr(sInput.length()-4) == ".txt")
	{
		if (!LoadFromInputScript(t, sInput, sOutput))
		{
			TMsg(tstring("Usage: ") + args[0] + " input.txt\n");
			TMsg("- input.txt must specify a game directory and an output file.\n");
			return 1;
		}
	}
	else
	{
		if (argc < 3)
		{
			TMsg(tstring("Usage: ") + args[0] + " input.obj output.toy [--physics input.obj]\n");
			return 1;
		}

		tstring sPhysics;

		for (int i = 3; i < argc; i++)
		{
			if (strcmp(args[i], "--physics") == 0)
			{
				if (argc < i+1)
				{
					TMsg(tstring("Usage: ") + args[0] + " input.obj output.toy [--physics input.obj]\n");
					return 1;
				}

				sPhysics = args[i+1];
				i++;
			}
			else if (strcmp(args[i], "--use-global-transforms") == 0)
				t.UseGlobalTransformations();
		}

		sOutput = FindAbsolutePath(args[2]);
		t.SetOutputDirectory(sOutput.substr(0, sOutput.rfind(DIR_SEP)));

		time_t iInputModificationTime = GetFileModificationTime(sInput.c_str());
		time_t iOutputModificationTime = GetFileModificationTime(sOutput.c_str());
		time_t iPhysicsModificationTime = GetFileModificationTime(sPhysics.c_str());

		bool bRecompile = false;
		if (iInputModificationTime > iOutputModificationTime)
			bRecompile = true;
		else if (iPhysicsModificationTime > iOutputModificationTime)
			bRecompile = true;
		else if (g_iBinaryModificationTime > iOutputModificationTime)
			bRecompile = true;

		if (!bRecompile)
		{
			if (Shell()->HasCommandLineSwitch("--force"))
			{
				TMsg("Forcing rebuild even though no changes detected.\n");
			}
			else
			{
				TMsg("No changes detected. Skipping '" + sOutput  + "'.\n\n");
				return 0;
			}
		}

		LoadFromFiles(t, sInput, sPhysics);
	}

	TMsg(sprintf(" Mesh materials: %d\n", t.GetNumMaterials()));
	TMsg(sprintf(" Mesh tris: %d\n", t.GetNumVerts()/3));
	TMsg(sprintf(" Physics tris: %d\n", t.GetNumPhysIndices()/3));
	TMsg(sprintf(" Scene areas: %d\n", t.GetNumSceneAreas()));

	TMsg(sprintf("Writing toy '" + sOutput + "' ..."));

	if (!t.Write(sOutput))
	{
		TMsg("\nError writing to file.\n");
		return 1;
	}

	TMsg(" Done.\n");

	TMsg("Toy built successfully.\n\n");

	return 0;
}

void LoadFromFiles(CToyUtil& t, const tstring& sMesh, const tstring& sPhysics)
{
	TMsg("Reading model '%s" + sMesh + "' ...");
	CConversionScene* pScene = new CConversionScene();
	CModelConverter c(pScene);

	c.ReadModel(sMesh);
	TMsg(" Done.\n");

	TMsg("Building toy mesh ...");
	LoadSceneIntoToy(pScene, &t);
	TMsg(" Done.\n");

	delete pScene;

	if (sPhysics.length())
	{
		TMsg("Reading physics model '" + sPhysics + "' ...");
		CConversionScene* pScene = new CConversionScene();
		CModelConverter c(pScene);

		c.ReadModel(sPhysics);
		TMsg(" Done.\n");

		TMsg("Building toy physics model ...");
		LoadSceneIntoToyPhysics(pScene, &t);
		TMsg(" Done.\n");

		delete pScene;
	}
}
