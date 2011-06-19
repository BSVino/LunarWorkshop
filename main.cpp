#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <GL/glfw.h>
#include <IL/il.h>

#include <platform.h>
#include <strutils.h>

#include <modelconverter/modelconverter.h>
#include "ui/modelwindow.h"
#include "crunch/crunch.h"

typedef enum
{
	COMMAND_NONE = 0,
	COMMAND_AO,
} command_t;

class CPrintingWorkListener : public IWorkListener
{
public:
	virtual void BeginProgress() {};
	virtual void SetAction(const tstring& sAction, size_t iTotalProgress)
	{
		printf("\n");
		puts(convertstring<tchar, char>(sAction).c_str());
		if (!iTotalProgress)
			printf("...");

		m_iTotalProgress = iTotalProgress;
		m_iProgress = 0;
	}

	virtual void WorkProgress(size_t iProgress, bool bForceDraw = false)
	{
		if (!m_iTotalProgress)
			return;

		size_t iLastProgress = m_iProgress;
		m_iProgress = iProgress;

		size_t iLastPercent = (iLastProgress * 10 / m_iTotalProgress);
		size_t iPercent = (iProgress * 10 / m_iTotalProgress);

		if (iPercent > iLastPercent)
		{
			printf("%d", iPercent);
			return;
		}

		iLastPercent = (iLastProgress * 40 / m_iTotalProgress);
		iPercent = (iProgress * 40 / m_iTotalProgress);

		if (iPercent > iLastPercent)
			printf(".");
	}

	virtual void EndProgress()
	{
		printf("\n");
	}

public:
	size_t					m_iProgress;
	size_t					m_iTotalProgress;
};

int CreateApplication(int argc, char** argv)
{
	tstring sFile;
	command_t eCommand = COMMAND_NONE;
	aomethod_t eMethod = AOMETHOD_SHADOWMAP;
	size_t iSize = 1024;
	size_t iBleed = 1;
	size_t iLights = 3000;
	size_t iSamples = 20;
	float flRayFalloff = 1.0f;
	bool bRandomize = false;
	bool bCrease = false;
	bool bGroundOcclusion = false;
	tstring sOutput;

	if (argc >= 2)
	{
		for (int i = 1; i < argc; i++)
		{
			tstring sToken = convertstring<char, tchar>(argv[i]);

			if (sToken[0] == L'-')
			{
				// It's an argument
				if (sToken == _T("--command"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					if (sToken == _T("ao"))
						eCommand = COMMAND_AO;
				}
				else if (sToken == _T("--method"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					if (sToken == _T("shadowmap"))
						eMethod = AOMETHOD_SHADOWMAP;
					else if (sToken == _T("raytrace"))
						eMethod = AOMETHOD_RAYTRACE;
					else if (sToken == _T("tridistance"))
						eMethod = AOMETHOD_TRIDISTANCE;
					else if (sToken == _T("color"))
						eMethod = AOMETHOD_RENDER;
					else
						printf("ERROR: Unrecognized method.\n");
				}
				else if (sToken == _T("--size"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					iSize = stoi(sToken);
					if (iSize < 64)
						iSize = 64;
					else if (iSize > 2048)
						iSize = 2048;
				}
				else if (sToken == _T("--bleed"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					iBleed = stoi(sToken);
					if (iBleed < 0)
						iBleed = 0;
					else if (iBleed > 10)
						iBleed = 10;
				}
				else if (sToken == _T("--lights"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					iLights = stoi(sToken);
					if (iLights < 500)
						iLights = 500;
					else if (iSize > 3000)
						iLights = 3000;
				}
				else if (sToken == _T("--samples"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					iSamples = stoi(sToken);
					if (iSamples < 5)
						iSamples = 5;
					else if (iSamples > 25)
						iSamples = 25;
				}
				else if (sToken == _T("--falloff"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					if (sToken == _T("none"))
						flRayFalloff = -1.0f;
					else
					{
						flRayFalloff = stof(sToken);
						if (flRayFalloff < 0.0001f)
							flRayFalloff = 0.0001f;
					}
				}
				else if (sToken == _T("--randomize"))
				{
					bRandomize = true;
				}
				else if (sToken == _T("--crease"))
				{
					bCrease = true;
				}
				else if (sToken == _T("--groundocclusion"))
				{
					bGroundOcclusion = true;
				}
				else if (sToken == _T("--output"))
				{
					i++;
					tstring sToken = convertstring<char, tchar>(argv[i]);
					sOutput = sToken;
				}
			}
			else
			{
				// It's a file
				sFile = sToken;
			}
		}

		switch (eCommand)
		{
		case COMMAND_AO:
		{
			puts(convertstring<tchar, char>(tstring(_T("Generating ambient occlusion map for ")) + sFile + _T("\n")).c_str());
			switch (eMethod)
			{
			case AOMETHOD_RENDER:
				printf("Method: Color AO\n");
				break;

			case AOMETHOD_TRIDISTANCE:
				printf("Method: Triangle distance\n");
				break;

			case AOMETHOD_RAYTRACE:
				printf("Method: Raytrace\n");
				break;

			case AOMETHOD_SHADOWMAP:
				printf("Method: Shadow mapping\n");
				break;
			}
			printf("Size: %dx%d\n", iSize, iSize);
			printf("Bleed: %d\n", iBleed);
			if (eMethod == AOMETHOD_SHADOWMAP)
				printf("Lights: %d\n", iLights);
			else if (eMethod == AOMETHOD_RAYTRACE)
				printf("Samples: %d\n", iSamples);
			puts(convertstring<tchar, char>(tstring(_T("Output file: ")) + sOutput + _T("\n")).c_str());

			CConversionScene s;
			CModelConverter c(&s);

			CPrintingWorkListener l;

			c.SetWorkListener(&l);

			if (!c.ReadModel(sFile.c_str()))
			{
				printf("Unsupported model format.\n");
				return 1;
			}

			if (eMethod == AOMETHOD_SHADOWMAP || eMethod == AOMETHOD_RENDER)
			{
				glfwInit();

				// The easy way to get a "windowless" context.
				glfwOpenWindow(100, 100, 0, 0, 0, 0, 16, 0, GLFW_WINDOW);
				glfwSetWindowTitle("SMAK a Batch");
				glfwIconifyWindow();

				glewInit();
			}

			ilInit();

			// If this is the color AO method, we need to load the textures.
			eastl::vector<CMaterial> aMaterials;
			if (eMethod == AOMETHOD_RENDER)
			{
				for (size_t i = 0; i < s.GetNumMaterials(); i++)
				{
					CConversionMaterial* pMaterial = s.GetMaterial(i);

					aMaterials.push_back(CMaterial(0));

					TAssert(aMaterials.size()-1 == i);

					size_t iTexture = CModelWindow::LoadTextureIntoGL(pMaterial->GetDiffuseTexture());

					if (iTexture)
						aMaterials[i].m_iBase = iTexture;
				}

				if (!aMaterials.size())
				{
					aMaterials.push_back(CMaterial(0));
				}
			}

			CAOGenerator ao(&s, &aMaterials);

			ao.SetMethod(eMethod);
			ao.SetSize(iSize, iSize);
			ao.SetBleed(iBleed);
			ao.SetUseTexture(true);
			if (eMethod == AOMETHOD_SHADOWMAP)
				ao.SetSamples(iLights);
			else if (eMethod == AOMETHOD_RAYTRACE)
				ao.SetSamples(iSamples);
			ao.SetRandomize(bRandomize);
			ao.SetCreaseEdges(bCrease);
			ao.SetGroundOcclusion(bGroundOcclusion);

			ao.SetWorkListener(&l);

			ao.Generate();

			printf("Saving results...\n");
			if (sOutput.length())
				ao.SaveToFile(sOutput.c_str());
			else
				ao.SaveToFile(_T("ao-output.png"));

			printf("Done.\n");
			return 0;
		}
		}
	}

	CModelWindow oWindow(argc, argv);

	if (sFile.length())
		oWindow.ReadFile(sFile.c_str());

	oWindow.OpenWindow();
	oWindow.Run();

	return 0;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	// Make sure we open up an assert messagebox window instead of just aborting like it does for console apps.
	_set_error_mode(_OUT_TO_MSGBOX);

#ifndef _DEBUG
	__try
	{
#endif
#endif

	return CreateApplication(argc, argv);

#if defined(_WIN32) && !defined(_DEBUG)
	}
	__except (CreateMinidump(GetExceptionInformation(), _T("SMAK")), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}
