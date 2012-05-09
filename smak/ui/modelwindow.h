#ifndef SMAK_MODELWINDOW_H
#define SMAK_MODELWINDOW_H

#include <modelconverter/convmesh.h>
#include <glgui/glgui.h>
#include <worklistener.h>
#include <tinker/application.h>
#include <common.h>

class CMaterial
{
public:
	CMaterial(size_t iBase)
	{
		m_iBase = iBase;
		m_iNormal = 0;
		m_iNormalIL = 0;
		m_iNormal2 = 0;
		m_iNormal2IL = 0;
		m_iAO = 0;
		m_iColorAO = 0;
	}

	size_t		m_iBase;
	size_t		m_iNormal;
	size_t		m_iNormalIL;
	size_t		m_iNormal2;
	size_t		m_iNormal2IL;
	size_t		m_iAO;
	size_t		m_iColorAO;
};

class CModelWindow : public CApplication, glgui::IEventListener, IWorkListener
{
	DECLARE_CLASS(CModelWindow, CApplication);

public:
							CModelWindow(int argc, char** argv);
							~CModelWindow();

public:
	virtual eastl::string	WindowTitle() { return "SMAK - Super Model Army Knife"; }
	virtual tstring	AppDirectory() { return "SMAK"; }

	void					OpenWindow();

	void					InitUI();

	void					CompileShaders();

	void					Run();	// Doesn't return

	void					DestroyAll();
	void					ReadFile(const tchar* pszFile);
	void					ReadFileIntoScene(const tchar* pszFile);
	void					ReloadFromFile();

	void					LoadIntoGL();
	static size_t			LoadTexture(tstring sFilename);
	static size_t			LoadTextureIntoGL(tstring sFilename);
	void					LoadTexturesIntoGL();

	void					SaveFile(const tchar* pszFile);

	void					Layout();

	virtual void			Render();
	void					Render3D();
	void					RenderGround();
	void					RenderObjects();
	void					RenderSceneNode(CConversionSceneNode* pNode);
	void					RenderMeshInstance(CConversionMeshInstance* pMeshInstance);
	void					RenderLightSource();
	void					RenderUV();

	virtual void			WindowResize(int x, int y);
	virtual void			MouseMotion(int x, int y);
	virtual void			MouseInput(int iButton, int iState);
	virtual void			MouseWheel(int x, int y);
	virtual bool			DoCharPress(int c);
	virtual bool			DoKeyPress(int c);

	EVENT_CALLBACK(CModelWindow, OpenDialog);
	EVENT_CALLBACK(CModelWindow, OpenFile);
	EVENT_CALLBACK(CModelWindow, OpenIntoDialog);
	EVENT_CALLBACK(CModelWindow, OpenIntoFile);
	EVENT_CALLBACK(CModelWindow, Reload);
	EVENT_CALLBACK(CModelWindow, SaveDialog);
	EVENT_CALLBACK(CModelWindow, SaveFile);
	EVENT_CALLBACK(CModelWindow, Close);
	EVENT_CALLBACK(CModelWindow, Exit);
	EVENT_CALLBACK(CModelWindow, Render3D);
	EVENT_CALLBACK(CModelWindow, RenderUV);
	EVENT_CALLBACK(CModelWindow, SceneTree);
	EVENT_CALLBACK(CModelWindow, Wireframe);
	EVENT_CALLBACK(CModelWindow, Flat);
	EVENT_CALLBACK(CModelWindow, Smooth);
	EVENT_CALLBACK(CModelWindow, UVWireframe);
	EVENT_CALLBACK(CModelWindow, Light);
	EVENT_CALLBACK(CModelWindow, Texture);
	EVENT_CALLBACK(CModelWindow, Combo);
	EVENT_CALLBACK(CModelWindow, Normal);
	EVENT_CALLBACK(CModelWindow, AO);
	EVENT_CALLBACK(CModelWindow, ColorAO);
	EVENT_CALLBACK(CModelWindow, LightToggle);
	EVENT_CALLBACK(CModelWindow, TextureToggle);
	EVENT_CALLBACK(CModelWindow, NormalToggle);
	EVENT_CALLBACK(CModelWindow, AOToggle);
	EVENT_CALLBACK(CModelWindow, ColorAOToggle);
	EVENT_CALLBACK(CModelWindow, GenerateCombo);
	EVENT_CALLBACK(CModelWindow, GenerateAO);
	EVENT_CALLBACK(CModelWindow, GenerateColorAO);
	EVENT_CALLBACK(CModelWindow, GenerateNormal);
	EVENT_CALLBACK(CModelWindow, Help);
	EVENT_CALLBACK(CModelWindow, Register);
	EVENT_CALLBACK(CModelWindow, About);

	size_t					GetNextObjectId();

	void					OpenHelpPanel();
	void					OpenRegisterPanel();
	void					OpenAboutPanel();

	// UI
	bool					GetRenderMode() { return m_bRenderUV; };
	void					SetRenderMode(bool bUV);
	void					SetDisplayWireframe(bool bWire);
	void					SetDisplayUVWireframe(bool bWire);
	void					SetDisplayLight(bool bLight);
	void					SetDisplayTexture(bool bTexture);
	void					SetDisplayNormal(bool bNormal);
	void					SetDisplayAO(bool bAO);
	void					SetDisplayColorAO(bool bAO);

	void					SaveNormal(size_t iMaterial, const tstring& sFilename);

	int						GetWindowWidth() { return (int)m_iWindowWidth; };
	int						GetWindowHeight() { return (int)m_iWindowHeight; };

	size_t					GetArrowTexture() { return m_iArrowTexture; };
	size_t					GetEditTexture() { return m_iEditTexture; };
	size_t					GetVisibilityTexture() { return m_iVisibilityTexture; };
	size_t					GetMaterialsNodeTexture() { return m_iTextureTexture; };
	size_t					GetMeshesNodeTexture() { return m_iWireframeTexture; };
	size_t					GetScenesNodeTexture() { return m_iAOTexture; };
	size_t					GetBarretTexture() { return m_iBarretTexture; };

	static CModelWindow*	Get() { return s_pModelWindow; };

	CConversionScene*		GetScene() { return &m_Scene; };
	tvector<CMaterial>*		GetMaterials() { return &m_aoMaterials; };

	void					ClearDebugLines();
	void					AddDebugLine(Vector vecStart, Vector vecEnd, Color clrLine = Color(150, 150, 150));

	void					BeginProgress();
	void					SetAction(const tstring& sAction, size_t iTotalProgress);
	void					WorkProgress(size_t iProgress, bool bForceDraw = false);
	void					EndProgress();

protected:
	CConversionScene		m_Scene;
	bool					m_bLoadingFile;
	tchar					m_szFileLoaded[1024];

	tvector<size_t>			m_aiObjects;
	size_t					m_iObjectsCreated;

	tvector<CMaterial>		m_aoMaterials;

	CMaterial*				m_pLightHalo;
	CMaterial*				m_pLightBeam;

	size_t					m_iWireframeTexture;
	size_t					m_iFlatTexture;
	size_t					m_iSmoothTexture;
	size_t					m_iUVTexture;
	size_t					m_iLightTexture;
	size_t					m_iTextureTexture;
	size_t					m_iNormalTexture;
	size_t					m_iAOTexture;
	size_t					m_iCAOTexture;
	size_t					m_iArrowTexture;
	size_t					m_iVisibilityTexture;
	size_t					m_iEditTexture;
	size_t					m_iBarretTexture;

	float					m_flCameraDistance;

	bool					m_bCameraRotating;
	bool					m_bCameraDollying;
	bool					m_bCameraPanning;
	bool					m_bLightRotating;

	int						m_iMouseStartX;
	int						m_iMouseStartY;

	float					m_flCameraYaw;
	float					m_flCameraPitch;

	float					m_flLightYaw;
	float					m_flLightPitch;
	Vector					m_vecLightPosition;

	float					m_flCameraUVX;
	float					m_flCameraUVY;
	float					m_flCameraUVZoom;

	Vector					m_vecLightPositionUV;

	typedef struct
	{
		Vector vecStart;
		Vector vecEnd;
		Color clrLine;
	} DebugLine;
	tvector<DebugLine>		m_aDebugLines;

	// Options
	bool					m_bRenderUV;
	bool					m_bDisplayWireframe;
	bool					m_bDisplayUV;
	bool					m_bDisplayLight;
	bool					m_bDisplayTexture;
	bool					m_bDisplayNormal;
	bool					m_bDisplayAO;
	bool					m_bDisplayColorAO;

	class GLUtesselator*	m_pTesselator;

	size_t					m_iShaderProgram;

	// Controls
	glgui::CButton*			m_pRender3D;
	glgui::CButton*			m_pRenderUV;

	glgui::CButton*			m_pWireframe;
	glgui::CButton*			m_pFlat;
	glgui::CButton*			m_pSmooth;
	glgui::CButton*			m_pUVWireframe;
	glgui::CButton*			m_pLight;
	glgui::CButton*			m_pTexture;
	glgui::CButton*			m_pNormal;
	glgui::CButton*			m_pAO;
	glgui::CButton*			m_pColorAO;

	static CModelWindow*	s_pModelWindow;
};

inline CModelWindow* ModelWindow()
{
	return CModelWindow::Get();
}

#endif
