#include "smakwindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maths.h>

#include <tinker_platform.h>

#include <modelconverter/modelconverter.h>
#include <glgui/glgui.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <tinker/renderer/renderer.h>
#include <datamanager/data.h>
#include <textures/materiallibrary.h>

#include "smak_renderer.h"
#include "scenetree.h"
#include "models/models.h"

using namespace glgui;

//#define RAYTRACE_DEBUG
#ifdef RAYTRACE_DEBUG
#include <raytracer/raytracer.h>
#endif

// Some compilers don't need it, make sure it still exists though.
#ifndef CALLBACK
#ifdef _WIN32
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif
#endif

#ifdef OPENGL2
extern "C" {
static void CALLBACK RenderTesselateBegin(GLenum ePrim);
static void CALLBACK RenderTesselateVertex(void* pVertexData, void* pPolygonData);
static void CALLBACK RenderTesselateEnd();
}
#endif

CSMAKWindow* CSMAKWindow::s_pSMAKWindow = NULL;

CSMAKWindow::CSMAKWindow(int argc, char** argv)
	: CApplication(argc, argv)
{
	s_pSMAKWindow = this;

	m_bLoadingFile = false;

	m_pLightHalo = NULL;
	m_pLightBeam = NULL;

	m_aiObjects.clear();
	m_iObjectsCreated = 0;

	m_flCameraDistance = 100;

	m_bCameraRotating = false;
	m_bCameraDollying = false;
	m_bCameraPanning = false;
	m_bLightRotating = false;

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	m_flCameraYaw = 45;
	m_flCameraPitch = -45;

	m_flCameraUVZoom = 1;
	m_flCameraUVX = 0;
	m_flCameraUVY = 0;

	m_flLightYaw = 100;
	m_flLightPitch = -45;

	m_vecLightPositionUV = Vector(0.5f, 0.5f, 1.0f);

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	m_pLightHalo = NULL;
	m_pLightBeam = NULL;

	m_iWireframeTexture = 0;
	m_iSmoothTexture = 0;
	m_iUVTexture = 0;
	m_iLightTexture = 0;
	m_iTextureTexture = 0;
	m_iNormalTexture = 0;
	m_iAOTexture = 0;
	m_iCAOTexture = 0;
	m_iArrowTexture = 0;
	m_iEditTexture = 0;
	m_iVisibilityTexture = 0;

	m_iShaderProgram = 0;

	m_aDebugLines.set_capacity(2);
}

void CSMAKWindow::OpenWindow()
{
	SetMultisampling(true);
	BaseClass::OpenWindow(m_iWindowWidth, m_iWindowHeight, false, true);

#ifdef OPENGL3
	size_t iTexture = LoadTextureIntoGL("lighthalo.png");
	if (iTexture)
		m_pLightHalo = new CMaterial(iTexture);

	iTexture = LoadTextureIntoGL("lightbeam.png");
	if (iTexture)
		m_pLightBeam = new CMaterial(iTexture);

	m_iWireframeTexture = LoadTextureIntoGL("wireframe.png");
	m_iSmoothTexture = LoadTextureIntoGL("smooth.png");
	m_iUVTexture = LoadTextureIntoGL("uv.png");
	m_iLightTexture = LoadTextureIntoGL("light.png");
	m_iTextureTexture = LoadTextureIntoGL("texture.png");
	m_iNormalTexture = LoadTextureIntoGL("normal.png");
	m_iAOTexture = LoadTextureIntoGL("ao.png");
	m_iCAOTexture = LoadTextureIntoGL("aocolor.png");
	m_iArrowTexture = LoadTextureIntoGL("arrow.png");
	m_iEditTexture = LoadTextureIntoGL("pencil.png");
	m_iVisibilityTexture = LoadTextureIntoGL("eye.png");
#endif

	InitUI();

	SetRenderMode(false);
	SetDisplayWireframe(false);
	SetDisplayUVWireframe(true);
	SetDisplayLight(true);
	SetDisplayTexture(true);
	SetDisplayAO(false);
	SetDisplayColorAO(false);

#ifdef OPENGL2
	m_pTesselator = gluNewTess();
	gluTessCallback(m_pTesselator, GLU_TESS_BEGIN, (void(CALLBACK*)())RenderTesselateBegin);
	gluTessCallback(m_pTesselator, GLU_TESS_VERTEX_DATA, (void(CALLBACK*)())RenderTesselateVertex);
	gluTessCallback(m_pTesselator, GLU_TESS_END, (void(CALLBACK*)())RenderTesselateEnd);
	gluTessProperty(m_pTesselator, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glLineWidth(1.0);

	GLfloat flLightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat flLightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat flLightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, flLightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, flLightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, flLightSpecular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1f);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
#endif

	CSceneTreePanel::Get()->UpdateTree();

	RootPanel()->Layout();
}

CSMAKWindow::~CSMAKWindow()
{
	if (m_pLightHalo)
		delete m_pLightHalo;

	if (m_pLightBeam)
		delete m_pLightBeam;

#ifdef OPENGL2
	gluDeleteTess(m_pTesselator);
#endif
}

CRenderer* CSMAKWindow::CreateRenderer()
{
	return new CSMAKRenderer();
}

void CSMAKWindow::Run()
{
	while (IsOpen())
	{
		Render();
		SwapBuffers();
	}
}

void CSMAKWindow::DestroyAll()
{
	m_Scene.DestroyAll();

	m_szFileLoaded[0] = '\0';

	m_aiObjects.clear();
	m_iObjectsCreated = 0;
	m_ahMaterials.clear();

	CModelLibrary::ResetReferenceCounts();
	CModelLibrary::ClearUnreferenced();

	CRootPanel::Get()->UpdateScene();
	CSceneTreePanel::Get()->UpdateTree();

	CRootPanel::Get()->Layout();
}

void CSMAKWindow::ReadFile(const tchar* pszFile)
{
	if (!pszFile)
		return;

	if (m_bLoadingFile)
		return;

	// Save it in here in case m_szFileLoaded was passed into ReadFile, in which case it would be destroyed by DestroyAll.
	tstring sFile = pszFile;

	DestroyAll();

	ReadFileIntoScene(sFile.c_str());
}

void CSMAKWindow::ReadFileIntoScene(const tchar* pszFile)
{
	if (!pszFile)
		return;

	if (m_bLoadingFile)
		return;

	m_bLoadingFile = true;

	CModelConverter c(&m_Scene);

	c.SetWorkListener(this);

	if (!c.ReadModel(pszFile))
	{
		m_bLoadingFile = false;
		return;
	}

	tstrncpy(m_szFileLoaded, 1023, pszFile, tstrlen(pszFile));

	BeginProgress();
	SetAction("Loading into video hardware", 0);
	LoadIntoGL();
	EndProgress();

	m_flCameraDistance = m_Scene.m_oExtends.Size().Length() * 1.5f;

	CRootPanel::Get()->UpdateScene();
	CSceneTreePanel::Get()->UpdateTree();

	m_bLoadingFile = false;
}

void CSMAKWindow::ReloadFromFile()
{
	ReadFile(m_szFileLoaded);
}

void CSMAKWindow::LoadIntoGL()
{
	LoadModelsIntoGL();
	LoadMaterialsIntoGL();

	ClearDebugLines();
}

void CSMAKWindow::LoadModelsIntoGL()
{
	for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
		CModelLibrary::AddModel(&m_Scene, i);
}

void CSMAKWindow::LoadMaterialsIntoGL()
{
	m_ahMaterials.clear();
	m_ahMaterials.resize(m_Scene.GetNumMaterials());

	for (size_t i = 0; i < m_Scene.GetNumMaterials(); i++)
	{
		CData oMaterialData;
		CData* pShader = oMaterialData.AddChild("Shader", "model");
		if (m_Scene.GetMaterial(i))
		{
			pShader->AddChild("Diffuse", m_Scene.GetMaterial(i)->GetDiffuseTexture());
			m_ahMaterials[i] = CMaterialLibrary::AddMaterial(&oMaterialData);
		}

		//TAssert(m_aiMaterials[i]);
		if (!m_ahMaterials[i].IsValid())
			TError(tstring("Couldn't create material \"") + m_Scene.GetMaterial(i)->GetName() + "\"\n");
	}
}

void CSMAKWindow::SaveFile(const tchar* pszFile)
{
	if (!pszFile)
		return;

	CModelConverter c(&m_Scene);

	c.SetWorkListener(this);

	c.SaveModel(pszFile);
}

void CSMAKWindow::Render()
{
	GetSMAKRenderer()->Render();

	CRootPanel::Get()->Think(GetTime());
	CRootPanel::Get()->Paint(0, 0, (float)m_iWindowWidth, (float)m_iWindowHeight);
}

// Ew!
int g_iTangentAttrib;
int g_iBitangentAttrib;
bool g_bNormalMap;

#ifdef OPENGL2
extern "C" {
static void CALLBACK RenderTesselateBegin(GLenum ePrim)
{
	glBegin(ePrim);
}

static void CALLBACK RenderTesselateVertex(void* pVertexData, void* pPolygonData)
{
	CConversionMesh* pMesh = (CConversionMesh*)pPolygonData;
	CConversionVertex* pVertex = (CConversionVertex*)pVertexData;

	Vector vecVertex = pMesh->GetVertex(pVertex->v);
	Vector vecUV = pMesh->GetUV(pVertex->vu);

	if (GLEW_VERSION_1_3)
	{
		glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
		glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
		glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
		glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
		glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
	}
	else
		glTexCoord2fv(vecUV);

	if (g_bNormalMap)
	{
		glVertexAttrib3fv(g_iTangentAttrib, pMesh->GetTangent(pVertex->vt));
		glVertexAttrib3fv(g_iBitangentAttrib, pMesh->GetBitangent(pVertex->vb));
		glNormal3fv(pMesh->GetNormal(pVertex->vn));
	}
	else
		glNormal3fv(pMesh->GetNormal(pVertex->vn));

	glVertex3fv(vecVertex);
}

static void CALLBACK RenderTesselateEnd()
{
	glEnd();
}
}
#endif

void CSMAKWindow::WindowResize(int w, int h)
{
	float flSceneSize = m_Scene.m_oExtends.Size().Length()/2;
	if (flSceneSize < 150)
		flSceneSize = 150;

#ifdef OPENGL2
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			44.0,						// FOV
			(float)w/(float)h,			// Aspect ratio
			1.0,						// Z near
			m_flCameraDistance + flSceneSize
		);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif

	if (!IsOpen())
		return;

	CRootPanel::Get()->SetSize((float)w, (float)h);
	CRootPanel::Get()->Layout();
	BaseClass::WindowResize(w, h);
}

size_t CSMAKWindow::GetNextObjectId()
{
	return (m_iObjectsCreated++)+1;
}

void CSMAKWindow::SetRenderMode(bool bUV)
{
	m_pRender3D->SetState(false, false);
	m_pRenderUV->SetState(false, false);

	if (bUV)
		m_pRenderUV->SetState(true, false);
	else
		m_pRender3D->SetState(true, false);

	m_bRenderUV = bUV;

	m_pWireframe->SetVisible(!m_bRenderUV);
	m_pUVWireframe->SetVisible(m_bRenderUV);

	Layout();
}

void CSMAKWindow::SetDisplayWireframe(bool bWire)
{
	m_bDisplayWireframe = bWire;
	m_pWireframe->SetState(bWire, false);
}

void CSMAKWindow::SetDisplayUVWireframe(bool bWire)
{
	m_bDisplayUV = bWire;
	m_pUVWireframe->SetState(bWire, false);
}

void CSMAKWindow::SetDisplayLight(bool bLight)
{
	m_bDisplayLight = bLight;
	m_pLight->SetState(bLight, false);
}

void CSMAKWindow::SetDisplayTexture(bool bTexture)
{
	m_bDisplayTexture = bTexture;
	m_pTexture->SetState(bTexture, false);
}

void CSMAKWindow::SetDisplayNormal(bool bNormal)
{
	m_bDisplayNormal = bNormal;
	m_pNormal->SetState(bNormal, false);
}

void CSMAKWindow::SetDisplayAO(bool bAO)
{
	m_bDisplayAO = bAO;
	m_pAO->SetState(bAO, false);
}

void CSMAKWindow::SetDisplayColorAO(bool bColorAO)
{
	m_bDisplayColorAO = bColorAO;
	m_pColorAO->SetState(bColorAO, false);
}

void CSMAKWindow::SaveNormal(size_t iMaterial, const tstring& sFilename)
{
#ifdef OPENGL2
	CMaterial* pMaterial = &m_aoMaterials[iMaterial];

	if (pMaterial->m_iNormalIL == 0 && pMaterial->m_iNormal2IL == 0)
		return;

	ilEnable(IL_FILE_OVERWRITE);

	if (pMaterial->m_iNormalIL == 0 || pMaterial->m_iNormal2IL == 0)
	{
		ILuint iSaveId;

		ilGenImages(1, &iSaveId);
		ilBindImage(iSaveId);

		// Heh. Nice hack
		ILuint iNormalId = pMaterial->m_iNormalIL + pMaterial->m_iNormal2IL;
		ilCopyImage(iNormalId);

		ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

		if (!IsRegistered() && (ilGetInteger(IL_IMAGE_WIDTH) > 128 || ilGetInteger(IL_IMAGE_HEIGHT) > 128))
		{
			iluImageParameter(ILU_FILTER, ILU_BILINEAR);
			iluScale(128, 128, 1);
		}

		ilSaveImage(convertstring<tchar, ILchar>(sFilename).c_str());

		ilDeleteImage(iSaveId);
		ilBindImage(0);

		return;
	}

	ILuint iNormalId;
	ilGenImages(1, &iNormalId);
	ilBindImage(iNormalId);
	ilCopyImage(pMaterial->m_iNormalIL);
	ilConvertImage(IL_RGB, IL_FLOAT);

	int iWidth = ilGetInteger(IL_IMAGE_WIDTH);
	int iHeight = ilGetInteger(IL_IMAGE_HEIGHT);

	ILuint iNormal2Id;
	ilGenImages(1, &iNormal2Id);
	ilBindImage(iNormal2Id);
	ilCopyImage(pMaterial->m_iNormal2IL);
	ilConvertImage(IL_RGB, IL_FLOAT);

	int iWidth2 = ilGetInteger(IL_IMAGE_WIDTH);
	int iHeight2 = ilGetInteger(IL_IMAGE_HEIGHT);

	size_t iTotalWidth = iWidth > iWidth2 ? iWidth : iWidth2;
	size_t iTotalHeight = iHeight > iHeight2 ? iHeight : iHeight2;

	Vector* avecResizedNormals = new Vector[iTotalWidth*iTotalHeight];
	Vector* avecResizedNormals2 = new Vector[iTotalWidth*iTotalHeight];

	ilBindImage(iNormalId);
	iluImageParameter(ILU_FILTER, ILU_BILINEAR);
	iluScale((ILint)iTotalWidth, (ILint)iTotalHeight, 1);
	ilCopyPixels(0, 0, 0, (ILint)iTotalWidth, (ILint)iTotalHeight, 1, IL_RGB, IL_FLOAT, &avecResizedNormals[0].x);
	ilDeleteImage(iNormalId);

	ilBindImage(iNormal2Id);
	iluImageParameter(ILU_FILTER, ILU_BILINEAR);
	iluScale((ILint)iTotalWidth, (ILint)iTotalHeight, 1);
	ilCopyPixels(0, 0, 0, (ILint)iTotalWidth, (ILint)iTotalHeight, 1, IL_RGB, IL_FLOAT, &avecResizedNormals2[0].x);
	ilDeleteImage(iNormal2Id);

	Vector* avecMergedNormalValues = new Vector[iTotalWidth*iTotalHeight];

	for (size_t i = 0; i < iTotalWidth; i++)
	{
		for (size_t j = 0; j < iTotalHeight; j++)
		{
			size_t iTexel = iTotalHeight*j + i;

			Vector vecNormal = (avecResizedNormals[iTexel]*2 - Vector(1.0f, 1.0f, 1.0f));
			Vector vecNormal2 = (avecResizedNormals2[iTexel]*2 - Vector(1.0f, 1.0f, 1.0f));

			Vector vecBitangent = vecNormal.Cross(Vector(1, 0, 0)).Normalized();
			Vector vecTangent = vecBitangent.Cross(vecNormal).Normalized();

			Matrix4x4 mTBN;
			mTBN.SetForwardVector(vecTangent);
			mTBN.SetUpVector(vecBitangent);
			mTBN.SetRightVector(vecNormal);

			avecMergedNormalValues[iTexel] = (mTBN * vecNormal2)*0.99f/2 + Vector(0.5f, 0.5f, 0.5f);
		}
	}

	delete[] avecResizedNormals;
	delete[] avecResizedNormals2;

	ilGenImages(1, &iNormalId);
	ilBindImage(iNormalId);
	ilTexImage((ILint)iTotalWidth, (ILint)iTotalHeight, 1, 3, IL_RGB, IL_FLOAT, &avecMergedNormalValues[0].x);
	ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

	if (!IsRegistered() && (iTotalWidth > 128 || iTotalHeight > 128))
	{
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(128, 128, 1);
	}

	ilSaveImage(convertstring<tchar, ILchar>(sFilename).c_str());

	ilDeleteImages(1, &iNormalId);

	delete[] avecMergedNormalValues;
#endif
}

CSMAKRenderer* CSMAKWindow::GetSMAKRenderer()
{
	return static_cast<CSMAKRenderer*>(m_pRenderer);
};

void CSMAKWindow::ClearDebugLines()
{
	m_aDebugLines.clear();
}

void CSMAKWindow::AddDebugLine(Vector vecStart, Vector vecEnd, Color clrLine)
{
	DebugLine l;
	l.vecStart = vecStart;
	l.vecEnd = vecEnd;
	l.clrLine = clrLine;

	m_aDebugLines.push_back(l);
}
