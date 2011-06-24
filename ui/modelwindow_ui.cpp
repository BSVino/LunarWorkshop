#include "modelwindow_ui.h"

#include <IL/il.h>
#include <maths.h>
#include <strutils.h>
#include <platform.h>
#include <GL/glfw.h>

#include <tinker/keys.h>
#include "modelwindow.h"
#include "scenetree.h"
#include "../smak_version.h"
#include "picker.h"

void CModelWindow::InitUI()
{
	CMenu* pFile = CRootPanel::Get()->AddMenu(_T("File"));
	CMenu* pView = CRootPanel::Get()->AddMenu(_T("View"));
	CMenu* pTools = CRootPanel::Get()->AddMenu(_T("Tools"));
	CMenu* pHelp = CRootPanel::Get()->AddMenu(_T("Help"));

	pFile->AddSubmenu(_T("Open..."), this, Open);
	pFile->AddSubmenu(_T("Open Into..."), this, OpenInto);
	pFile->AddSubmenu(_T("Reload"), this, Reload);
	pFile->AddSubmenu(_T("Save As..."), this, Save);
	pFile->AddSubmenu(_T("Close"), this, Close);
	pFile->AddSubmenu(_T("Exit"), this, Exit);

	pView->AddSubmenu(_T("3D view"), this, Render3D);
	pView->AddSubmenu(_T("UV view"), this, RenderUV);
	pView->AddSubmenu(_T("View wireframe"), this, Wireframe);
	pView->AddSubmenu(_T("Toggle light"), this, LightToggle);
	pView->AddSubmenu(_T("Toggle texture"), this, TextureToggle);
	pView->AddSubmenu(_T("Toggle normal map"), this, NormalToggle);
	pView->AddSubmenu(_T("Toggle AO map"), this, AOToggle);
	pView->AddSubmenu(_T("Toggle color AO map"), this, ColorAOToggle);

	pTools->AddSubmenu(_T("Generate all maps"), this, GenerateCombo);
	pTools->AddSubmenu(_T("Generate AO map"), this, GenerateAO);
	pTools->AddSubmenu(_T("Generate color AO map"), this, GenerateColorAO);
	pTools->AddSubmenu(_T("Generate normal from texture"), this, GenerateNormal);

	pHelp->AddSubmenu(_T("Help"), this, Help);
#if !defined(TINKER_NO_REGISTRATION)
	pHelp->AddSubmenu(_T("Register..."), this, Register);
#endif
	pHelp->AddSubmenu(_T("About SMAK"), this, About);

	CButtonPanel* pTopButtons = new CButtonPanel(BA_TOP);

	m_pRender3D = new CButton(0, 0, 100, 100, _T("3D"), true);
	m_pRenderUV = new CButton(0, 0, 100, 100, _T("UV"), true);

	pTopButtons->AddButton(m_pRender3D, _T("Render 3D View"), false, this, Render3D);
	pTopButtons->AddButton(m_pRenderUV, _T("Render UV View"), false, this, RenderUV);

	CRootPanel::Get()->AddControl(pTopButtons);

	CButtonPanel* pBottomButtons = new CButtonPanel(BA_BOTTOM);

	m_pWireframe = new CPictureButton(_T("Wire"), m_iWireframeTexture, true);
	m_pUVWireframe = new CPictureButton(_T("Wire"), m_iUVTexture, true);
	m_pLight = new CPictureButton(_T("Lght"), m_iLightTexture, true);
	m_pTexture = new CPictureButton(_T("Tex"), m_iTextureTexture, true);
	m_pNormal = new CPictureButton(_T("Nrml"), m_iNormalTexture, true);
	m_pAO = new CPictureButton(_T("AO"), m_iAOTexture, true);
	m_pColorAO = new CPictureButton(_T("C AO"), m_iCAOTexture, true);

	pBottomButtons->AddButton(m_pWireframe, _T("Toggle Wireframe"), true, this, Wireframe);
	pBottomButtons->AddButton(m_pUVWireframe, _T("Toggle UVs"), true, this, UVWireframe);
	pBottomButtons->AddButton(m_pLight, _T("Toggle Light"), false, this, Light);
	pBottomButtons->AddButton(m_pTexture, _T("Toggle Texture"), false, this, Texture);
	pBottomButtons->AddButton(m_pNormal, _T("Toggle Normal Map"), false, this, Normal);
	pBottomButtons->AddButton(m_pAO, _T("Toggle AO Map"), false, this, AO);
	pBottomButtons->AddButton(m_pColorAO, _T("Toggle Color AO"), false, this, ColorAO);

	CRootPanel::Get()->AddControl(pBottomButtons);

	CRootPanel::Get()->Layout();

	CSceneTreePanel::Open(&m_Scene);
}

void CModelWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

void CModelWindow::OpenCallback()
{
	if (m_bLoadingFile)
		return;

	ReadFile(OpenFileDialog(_T("All *.obj;*.sia;*.dae\0*.obj;*.sia;*.dae\0")));
}

void CModelWindow::OpenIntoCallback()
{
	if (m_bLoadingFile)
		return;

	ReadFileIntoScene(OpenFileDialog(_T("All *.obj;*.sia;*.dae\0*.obj;*.sia;*.dae\0")));
}

void CModelWindow::ReloadCallback()
{
	if (m_bLoadingFile)
		return;

	ReloadFromFile();
}

void CModelWindow::SaveCallback()
{
	SaveFile(SaveFileDialog(_T("Wavefront .obj\0*.obj\0Silo ASCII .sia\0*.sia\0Collada .dae\0*.dae\0")));
}

void CModelWindow::CloseCallback()
{
	if (m_bLoadingFile)
		return;

	DestroyAll();
}

void CModelWindow::ExitCallback()
{
	exit(0);
}

void CModelWindow::Render3DCallback()
{
	SetRenderMode(false);
}

void CModelWindow::RenderUVCallback()
{
	SetRenderMode(true);
}

void CModelWindow::SceneTreeCallback()
{
	CSceneTreePanel::Open(&m_Scene);
}

void CModelWindow::WireframeCallback()
{
	SetDisplayWireframe(m_pWireframe->GetState());
}

void CModelWindow::UVWireframeCallback()
{
	m_bDisplayUV = m_pUVWireframe->GetState();
}

void CModelWindow::LightCallback()
{
	SetDisplayLight(m_pLight->GetState());
}

void CModelWindow::TextureCallback()
{
	SetDisplayTexture(m_pTexture->GetState());
}

void CModelWindow::NormalCallback()
{
	SetDisplayNormal(m_pNormal->GetState());
}

void CModelWindow::AOCallback()
{
	SetDisplayAO(m_pAO->GetState());
}

void CModelWindow::ColorAOCallback()
{
	if (CAOPanel::Get(true) && CAOPanel::Get(true)->IsGenerating() && !CAOPanel::Get(true)->DoneGenerating())
	{
		m_pColorAO->SetState(true, false);
		return;
	}

	if (!CAOPanel::Get(true) || !CAOPanel::Get(true)->DoneGenerating())
	{
		CAOPanel::Open(true, &m_Scene, &m_aoMaterials);
		m_pColorAO->SetState(false, false);
		return;
	}

	SetDisplayColorAO(m_pColorAO->GetState());
}

void CModelWindow::LightToggleCallback()
{
	SetDisplayLight(!m_bDisplayLight);
}

void CModelWindow::TextureToggleCallback()
{
	SetDisplayTexture(!m_bDisplayTexture);
}

void CModelWindow::NormalToggleCallback()
{
	SetDisplayNormal(!m_bDisplayNormal);
}

void CModelWindow::AOToggleCallback()
{
	SetDisplayAO(!m_bDisplayAO);
}

void CModelWindow::ColorAOToggleCallback()
{
	SetDisplayColorAO(!m_bDisplayColorAO);
}

void CModelWindow::GenerateComboCallback()
{
	CComboGeneratorPanel::Open(&m_Scene, &m_aoMaterials);
}

void CModelWindow::GenerateAOCallback()
{
	CAOPanel::Open(false, &m_Scene, &m_aoMaterials);
}

void CModelWindow::GenerateColorAOCallback()
{
	CAOPanel::Open(true, &m_Scene, &m_aoMaterials);
}

void CModelWindow::GenerateNormalCallback()
{
	CNormalPanel::Open(&m_Scene, &m_aoMaterials);
}

void CModelWindow::HelpCallback()
{
	OpenHelpPanel();
}

void CModelWindow::RegisterCallback()
{
	OpenRegisterPanel();
}

void CModelWindow::AboutCallback()
{
	OpenAboutPanel();
}

void CModelWindow::OpenHelpPanel()
{
	CHelpPanel::Open();
}

void CModelWindow::OpenRegisterPanel()
{
	CRegisterPanel::Open();
}

void CModelWindow::OpenAboutPanel()
{
	CAboutPanel::Open();
}

void CModelWindow::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CModelWindow::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CModelWindow::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static float flLastTime = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && GetTime() - flLastTime < 0.3f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	CModelWindow::Get()->Render();
	CRootPanel::Get()->Think(GetTime());
	CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
	glfwSwapBuffers();

	flLastTime = GetTime();
}

void CModelWindow::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

#define BTN_HEIGHT 32
#define BTN_SPACE 8
#define BTN_SECTION 18

CButtonPanel::CButtonPanel(buttonalignment_t eAlign)
: CPanel(0, 0, BTN_HEIGHT, BTN_HEIGHT)
{
	m_eAlign = eAlign;
}

void CButtonPanel::Layout()
{
	int iX = 0;

	for (size_t i = 0; i < m_apButtons.size(); i++)
	{
		IControl* pButton = m_apButtons[i];

		if (!pButton->IsVisible())
			continue;

		pButton->SetSize(BTN_HEIGHT, BTN_HEIGHT);
		pButton->SetPos(iX, 0);

		IControl* pHint = m_apHints[i];
		pHint->SetPos(iX + BTN_HEIGHT/2 - pHint->GetWidth()/2, -18);

		iX += BTN_HEIGHT + m_aiSpaces[i];
	}

	SetSize(iX - m_aiSpaces[m_aiSpaces.size()-1], BTN_HEIGHT);
	if (m_eAlign == BA_TOP)
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, BTN_HEIGHT + BTN_SPACE);
	else
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, CRootPanel::Get()->GetHeight() - BTN_HEIGHT*2 - BTN_SPACE);

	CPanel::Layout();
}

void CButtonPanel::AddButton(CButton* pButton, const tstring& sHints, bool bNewSection, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	AddControl(pButton);
	m_apButtons.push_back(pButton);

	m_aiSpaces.push_back(bNewSection?BTN_SECTION:BTN_SPACE);

	CLabel* pHint = new CLabel(0, 0, 0, 0, sHints);
	pHint->SetAlpha(0);
	pHint->EnsureTextFits();
	AddControl(pHint);
	m_apHints.push_back(pHint);

	if (pListener)
	{
		pButton->SetClickedListener(pListener, pfnCallback);
		pButton->SetUnclickedListener(pListener, pfnCallback);
	}
}

void CButtonPanel::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	for (size_t i = 0; i < m_apButtons.size(); i++)
	{
		if (!m_apButtons[i]->IsVisible())
		{
			m_apHints[i]->SetAlpha((int)Approach(0.0f, (float)m_apHints[i]->GetAlpha(), 30.0f));
			continue;
		}

		int x = 0, y = 0, w = 0, h = 0;
		m_apButtons[i]->GetAbsDimensions(x, y, w, h);

		float flAlpha = (float)m_apHints[i]->GetAlpha();

		if (mx >= x && my >= y && mx < x + w && my < y + h)
			m_apHints[i]->SetAlpha((int)Approach(255.0f, flAlpha, 30.0f));
		else
			m_apHints[i]->SetAlpha((int)Approach(0.0f, flAlpha, 30.0f));
	}

	CPanel::Think();
}

void CButtonPanel::Paint(int x, int y, int w, int h)
{
	CPanel::Paint(x, y, w, h);
}

CProgressBar* CProgressBar::s_pProgressBar = NULL;

CProgressBar::CProgressBar()
	: CPanel(0, 0, 100, 100)
{
	CRootPanel::Get()->AddControl(this);
	SetVisible(false);

	m_pAction = new CLabel(0, 0, 200, BTN_HEIGHT, _T(""));
	AddControl(m_pAction);
	m_pAction->SetWrap(false);

	Layout();
}

void CProgressBar::Layout()
{
	SetSize(GetParent()->GetWidth()/2, BTN_HEIGHT);
	SetPos(GetParent()->GetWidth()/4, GetParent()->GetHeight()/5);

	m_pAction->SetSize(GetWidth(), BTN_HEIGHT);
}

void CProgressBar::Paint(int x, int y, int w, int h)
{
	float flTotalProgress = (float)m_iCurrentProgress/(float)m_iTotalProgress;

	if (flTotalProgress > 1)
		flTotalProgress = 1;

	CPanel::PaintRect(x, y, w, h);
	CPanel::PaintRect(x+10, y+10, (int)((w-20)*flTotalProgress), h-20, g_clrBoxHi);

	m_pAction->Paint();
}

void CProgressBar::SetTotalProgress(size_t iProgress)
{
	m_iTotalProgress = iProgress;
	SetProgress(0);
}

void CProgressBar::SetProgress(size_t iProgress, const tstring& sAction)
{
	m_iCurrentProgress = iProgress;

	SetAction(sAction);
}

void CProgressBar::SetAction(const tstring& sAction)
{
	if (sAction.length())
		m_sAction = sAction;

	m_pAction->SetText(m_sAction);

	if (m_iTotalProgress)
	{
		tstring sProgress;
		sProgress = sprintf(tstring(" %d%%"), m_iCurrentProgress*100/m_iTotalProgress);
		m_pAction->AppendText(sProgress);
	}
}

CProgressBar* CProgressBar::Get()
{
	if (!s_pProgressBar)
		s_pProgressBar = new CProgressBar();

	return s_pProgressBar;
}

void CCloseButton::Paint(int x, int y, int w, int h)
{
	Color c;
	
	c.SetRed((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.r() * 2, 255.0f));
	c.SetGreen((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.g(), (float)g_clrBoxHi.g()));
	c.SetBlue((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.b(), (float)g_clrBoxHi.b()));
	c.SetAlpha(255);

	CRootPanel::PaintRect(x, y, w, h, c);
}

void CMinimizeButton::Paint(int x, int y, int w, int h)
{
	Color c;
	
	c.SetRed((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.r(), (float)g_clrBox.r()));
	c.SetGreen((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.g(), (float)g_clrBoxHi.g()));
	c.SetBlue((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.b() * 2, 255.0f));
	c.SetAlpha(255);

	CRootPanel::PaintRect(x, y+h/3+1, w, h/3, c);
}

CMovablePanel::CMovablePanel(const tstring& sName)
	: CPanel(0, 0, 200, 350)
{
	m_bMoving = false;

	m_pName = new CLabel(0, GetHeight()-HEADER_HEIGHT, GetWidth(), HEADER_HEIGHT, sName);
	AddControl(m_pName);

	m_pCloseButton = new CCloseButton();
	AddControl(m_pCloseButton);

	m_pCloseButton->SetClickedListener(this, CloseWindow);

	m_pMinimizeButton = new CMinimizeButton();
	AddControl(m_pMinimizeButton);

	m_pMinimizeButton->SetClickedListener(this, MinimizeWindow);

	CRootPanel::Get()->AddControl(this, true);

	m_bHasCloseButton = true;
	m_bMinimized = false;

	m_bClearBackground = false;
}

CMovablePanel::~CMovablePanel()
{
	CRootPanel::Get()->RemoveControl(this);
	Destructor();
}

void CMovablePanel::Layout()
{
	m_pName->SetDimensions(0, 0, GetWidth(), HEADER_HEIGHT);

	m_pCloseButton->SetVisible(m_bHasCloseButton);

	int iButtonSize = HEADER_HEIGHT*2/3;
	if (m_bHasCloseButton)
	{
		m_pCloseButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - iButtonSize/2, HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);
		m_pMinimizeButton->SetDimensions(GetWidth() - HEADER_HEIGHT*3/2 - iButtonSize/2, HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);
	}
	else
		m_pMinimizeButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - iButtonSize/2, HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);

	CPanel::Layout();
}

void CMovablePanel::Think()
{
	if (m_bMoving)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		SetPos(m_iStartX + mx - m_iMouseStartX, m_iStartY + my - m_iMouseStartY);
	}

	CPanel::Think();
}

void CMovablePanel::Paint(int x, int y, int w, int h)
{
	if (!m_bMinimized && !m_bClearBackground)
		CRootPanel::PaintRect(x, y, w, h, g_clrPanel);

	Color clrBox = g_clrBoxHi;
	if (m_bClearBackground)
		clrBox.SetAlpha(clrBox.a()*3/2);

	CRootPanel::PaintRect(x, y, w, HEADER_HEIGHT, clrBox);

	if (m_bMinimized)
	{
		m_pName->Paint();
		if (m_bHasCloseButton)
			m_pCloseButton->Paint();
		m_pMinimizeButton->Paint();
	}
	else
		CPanel::Paint(x, y, w, h);
}

bool CMovablePanel::MousePressed(int iButton, int mx, int my)
{
	int x, y;
	GetAbsPos(x, y);

	if (iButton == TINKER_KEY_MOUSE_LEFT && mx > x && mx < x + GetWidth() - HEADER_HEIGHT*2 && my > y && my < y + HEADER_HEIGHT )
	{
		m_iMouseStartX = mx;
		m_iMouseStartY = my;
		m_bMoving = true;

		GetPos(x, y);
		m_iStartX = x;
		m_iStartY = y;

		return true;
	}

	return CPanel::MousePressed(iButton, mx, my);
}

bool CMovablePanel::MouseReleased(int iButton, int mx, int my)
{
	if (m_bMoving)
	{
		m_bMoving = false;
		return true;
	}

	CPanel::MouseReleased(iButton, mx, my);

	return false;
}

void CMovablePanel::Minimize()
{
	m_bMinimized = !m_bMinimized;

	if (m_bMinimized)
	{
		m_iNonMinimizedHeight = GetHeight();
		SetSize(GetWidth(), HEADER_HEIGHT);
	}
	else
	{
		SetSize(GetWidth(), m_iNonMinimizedHeight);
	}

	Layout();
}

void CMovablePanel::CloseWindowCallback()
{
	SetVisible(false);
}

void CMovablePanel::MinimizeWindowCallback()
{
	Minimize();
}

CAOPanel* CAOPanel::s_pAOPanel = NULL;
CAOPanel* CAOPanel::s_pColorAOPanel = NULL;

CAOPanel::CAOPanel(bool bColor, CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
	: CMovablePanel(bColor?_T("Color AO generator"):_T("AO generator")), m_oGenerator(pScene, paoMaterials)
{
	m_bColor = bColor;

	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	if (m_bColor)
		SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 150);
	else
		SetPos(GetParent()->GetWidth() - GetWidth() - 200, GetParent()->GetHeight() - GetHeight() - 100);

	m_pSizeLabel = new CLabel(0, 0, 32, 32, _T("Size"));
	AddControl(m_pSizeLabel);

	m_pSizeSelector = new CScrollSelector<int>();
#ifdef _DEBUG
	m_pSizeSelector->AddSelection(CScrollSelection<int>(16, _T("16x16")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(32, _T("32x32")));
#endif
	m_pSizeSelector->AddSelection(CScrollSelection<int>(64, _T("64x64")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(128, _T("128x128")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(256, _T("256x256")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(512, _T("512x512")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(1024, _T("1024x1024")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(2048, _T("2048x2048")));
	//m_pSizeSelector->AddSelection(CScrollSelection<int>(4096, _T("4096x4096")));
	m_pSizeSelector->SetSelection(2);
	AddControl(m_pSizeSelector);

	m_pEdgeBleedLabel = new CLabel(0, 0, 32, 32, _T("Edge Bleed"));
	AddControl(m_pEdgeBleedLabel);

	m_pEdgeBleedSelector = new CScrollSelector<int>();
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(0, _T("0")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(1, _T("1")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(2, _T("2")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(3, _T("3")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(4, _T("4")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(5, _T("5")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(6, _T("6")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(7, _T("7")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(8, _T("8")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(9, _T("9")));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(10, _T("10")));
	m_pEdgeBleedSelector->SetSelection(1);
	AddControl(m_pEdgeBleedSelector);

	if (!m_bColor)
	{
		m_pAOMethodLabel = new CLabel(0, 0, 32, 32, _T("Method"));
		AddControl(m_pAOMethodLabel);

		m_pAOMethodSelector = new CScrollSelector<int>();
		m_pAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_SHADOWMAP, _T("Shadow map (fast!)")));
		m_pAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_RAYTRACE, _T("Raytraced (slow!)")));
		m_pAOMethodSelector->SetSelectedListener(this, AOMethod);
		AddControl(m_pAOMethodSelector);

		m_pRayDensityLabel = new CLabel(0, 0, 32, 32, _T("Ray Density"));
		AddControl(m_pRayDensityLabel);

		m_pRayDensitySelector = new CScrollSelector<int>();
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(5, _T("5")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(6, _T("6")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(7, _T("7")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(8, _T("8")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(9, _T("9")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(10, _T("10")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(11, _T("11")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(12, _T("12")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(13, _T("13")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(14, _T("14")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(15, _T("15")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(16, _T("16")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(17, _T("17")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(18, _T("18")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(19, _T("19")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(20, _T("20")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(21, _T("21")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(22, _T("22")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(23, _T("23")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(24, _T("24")));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(25, _T("25")));
		m_pRayDensitySelector->SetSelection(15);
		AddControl(m_pRayDensitySelector);

		m_pFalloffLabel = new CLabel(0, 0, 32, 32, _T("Ray Falloff"));
		AddControl(m_pFalloffLabel);

		m_pFalloffSelector = new CScrollSelector<float>();
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.01f, _T("0.01")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.05f, _T("0.05")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.1f, _T("0.1")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.25f, _T("0.25")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.5f, _T("0.5")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.75f, _T("0.75")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.0f, _T("1.0")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.25f, _T("1.25")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.5f, _T("1.5")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.75f, _T("1.75")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(2.0f, _T("2.0")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(2.5f, _T("2.5")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(3.0f, _T("3.0")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(4.0f, _T("4.0")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(5.0f, _T("5.0")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(6.0f, _T("6.0")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(7.5f, _T("7.5")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(10.0f, _T("10")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(25.0f, _T("25")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(50.0f, _T("50")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(100.0f, _T("100")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(250.0f, _T("250")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(500.0f, _T("500")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1000.0f, _T("1000")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(2500.0f, _T("2500")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(5000.0f, _T("5000")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(10000.0f, _T("10000")));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(-1.0f, _T("No falloff")));
		m_pFalloffSelector->SetSelection(6);
		AddControl(m_pFalloffSelector);

		m_pLightsLabel = new CLabel(0, 0, 32, 32, _T("Lights"));
		AddControl(m_pLightsLabel);

		m_pLightsSelector = new CScrollSelector<int>();
		m_pLightsSelector->AddSelection(CScrollSelection<int>(500, _T("500")));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(1000, _T("1000")));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(1500, _T("1500")));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(2000, _T("2000")));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(2500, _T("2500")));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(3000, _T("3000")));
		m_pLightsSelector->SetSelection(3);
		AddControl(m_pLightsSelector);

		m_pRandomLabel = new CLabel(0, 0, 32, 32, _T("Randomize rays"));
		AddControl(m_pRandomLabel);

		m_pRandomCheckBox = new CCheckBox();
		AddControl(m_pRandomCheckBox);

		m_pCreaseLabel = new CLabel(0, 0, 32, 32, _T("Crease edges"));
		AddControl(m_pCreaseLabel);

		m_pCreaseCheckBox = new CCheckBox();
		AddControl(m_pCreaseCheckBox);

		m_pGroundOcclusionLabel = new CLabel(0, 0, 32, 32, _T("Ground occlusion"));
		AddControl(m_pGroundOcclusionLabel);

		m_pGroundOcclusionCheckBox = new CCheckBox();
		AddControl(m_pGroundOcclusionCheckBox);
	}

	m_pGenerate = new CButton(0, 0, 100, 100, _T("Generate"));
	AddControl(m_pGenerate);

	m_pGenerate->SetClickedListener(this, Generate);

	m_pSave = new CButton(0, 0, 100, 100, _T("Save Map"));
	AddControl(m_pSave);

	m_pSave->SetClickedListener(this, SaveMap);
	m_pSave->SetVisible(false);

	Layout();
}

void CAOPanel::Layout()
{
	int iSpace = 20;

	m_pSizeLabel->EnsureTextFits();

	int iSelectorSize = m_pSizeLabel->GetHeight() - 4;

	m_pSizeSelector->SetSize(GetWidth() - m_pSizeLabel->GetWidth() - iSpace, iSelectorSize);

	int iControlY = HEADER_HEIGHT;

	m_pSizeSelector->SetPos(GetWidth() - m_pSizeSelector->GetWidth() - iSpace/2, iControlY);
	m_pSizeLabel->SetPos(5, iControlY);

	iControlY += 30;

	m_pEdgeBleedLabel->EnsureTextFits();
	m_pEdgeBleedLabel->SetPos(5, iControlY);

	m_pEdgeBleedSelector->SetSize(GetWidth() - m_pEdgeBleedLabel->GetWidth() - iSpace, iSelectorSize);
	m_pEdgeBleedSelector->SetPos(GetWidth() - m_pEdgeBleedSelector->GetWidth() - iSpace/2, iControlY);

	if (!m_bColor)
	{
		iControlY += 30;

		m_pAOMethodLabel->EnsureTextFits();
		m_pAOMethodLabel->SetPos(5, iControlY);

		m_pAOMethodSelector->SetSize(GetWidth() - m_pAOMethodLabel->GetWidth() - iSpace, iSelectorSize);
		m_pAOMethodSelector->SetPos(GetWidth() - m_pAOMethodSelector->GetWidth() - iSpace/2, iControlY);

		bool bRaytracing = (m_pAOMethodSelector->GetSelectionValue() == AOMETHOD_RAYTRACE);
		m_pRayDensityLabel->SetVisible(bRaytracing);
		m_pRayDensitySelector->SetVisible(bRaytracing);

		m_pFalloffSelector->SetVisible(bRaytracing);
		m_pFalloffLabel->SetVisible(bRaytracing);

		m_pRandomCheckBox->SetVisible(bRaytracing);
		m_pRandomLabel->SetVisible(bRaytracing);

		if (bRaytracing)
		{
			iControlY += 30;

			m_pRayDensityLabel->EnsureTextFits();
			m_pRayDensityLabel->SetPos(5, iControlY);

			m_pRayDensitySelector->SetSize(GetWidth() - m_pRayDensityLabel->GetWidth() - iSpace, iSelectorSize);
			m_pRayDensitySelector->SetPos(GetWidth() - m_pRayDensitySelector->GetWidth() - iSpace/2, iControlY);

			iControlY += 30;

			m_pFalloffLabel->EnsureTextFits();
			m_pFalloffLabel->SetPos(5, iControlY);

			m_pFalloffSelector->SetSize(GetWidth() - m_pFalloffLabel->GetWidth() - iSpace, iSelectorSize);
			m_pFalloffSelector->SetPos(GetWidth() - m_pFalloffSelector->GetWidth() - iSpace/2, iControlY);

			iControlY += 30;

			m_pRandomLabel->EnsureTextFits();
			m_pRandomLabel->SetPos(25, iControlY);

			m_pRandomCheckBox->SetPos(10, iControlY + m_pRandomLabel->GetHeight()/2 - m_pRandomCheckBox->GetHeight()/2);
		}

		bool bShadowmapping = (m_pAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP);
		m_pLightsLabel->SetVisible(bShadowmapping);
		m_pLightsSelector->SetVisible(bShadowmapping);

		if (bShadowmapping)
		{
			iControlY += 30;

			m_pLightsLabel->EnsureTextFits();
			m_pLightsLabel->SetPos(5, iControlY);

			m_pLightsSelector->SetSize(GetWidth() - m_pLightsLabel->GetWidth() - iSpace, iSelectorSize);
			m_pLightsSelector->SetPos(GetWidth() - m_pLightsSelector->GetWidth() - iSpace/2, iControlY);
		}

		m_pGroundOcclusionLabel->SetVisible(bShadowmapping || bRaytracing);
		m_pGroundOcclusionCheckBox->SetVisible(bShadowmapping || bRaytracing);

		if (bShadowmapping || bRaytracing)
		{
			// If we're on the raytracing screen there was already the Randomize rays checkbox
			// so keep the spacing smaller.
			if (bRaytracing)
				iControlY += 20;
			else
				iControlY += 30;

			m_pGroundOcclusionLabel->EnsureTextFits();
			m_pGroundOcclusionLabel->SetPos(25, iControlY);

			m_pGroundOcclusionCheckBox->SetPos(10, iControlY + m_pGroundOcclusionLabel->GetHeight()/2 - m_pGroundOcclusionCheckBox->GetHeight()/2);
		}

		if (bShadowmapping || bRaytracing)
			iControlY += 20;
		else
			iControlY += 30;

		m_pCreaseLabel->EnsureTextFits();
		m_pCreaseLabel->SetPos(25, iControlY);

		m_pCreaseCheckBox->SetPos(10, iControlY + m_pCreaseLabel->GetHeight()/2 - m_pCreaseCheckBox->GetHeight()/2);
	}

	m_pSave->SetSize(GetWidth()/2, GetWidth()/6);
	m_pSave->SetPos(GetWidth()/4, GetHeight() - (int)(m_pSave->GetHeight()*1.5f));
	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetSize(GetWidth()/2, GetWidth()/6);
	m_pGenerate->SetPos(GetWidth()/4, GetHeight() - (int)(m_pSave->GetHeight()*1.5f) - (int)(m_pGenerate->GetHeight()*1.5f));

	CMovablePanel::Layout();
}

bool CAOPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CAOPanel::GenerateCallback()
{
	if (m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_pSave->SetVisible(false);

	// Switch over to UV mode so we can see our progress.
	CModelWindow::Get()->SetRenderMode(true);

	// If the 3d model was there get rid of it.
	CModelWindow::Get()->Render();
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glfwSwapBuffers();
	CModelWindow::Get()->Render();
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glfwSwapBuffers();

	if (m_bColor)
		CModelWindow::Get()->SetDisplayColorAO(true);
	else
		CModelWindow::Get()->SetDisplayAO(true);

	m_pGenerate->SetText(_T("Cancel"));

	int iSize = m_pSizeSelector->GetSelectionValue();
	m_oGenerator.SetMethod(m_bColor?AOMETHOD_RENDER:(aomethod_t)m_pAOMethodSelector->GetSelectionValue());
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.SetBleed(m_pEdgeBleedSelector->GetSelectionValue());
	m_oGenerator.SetUseTexture(true);
	m_oGenerator.SetWorkListener(this);
	if (!m_bColor)
	{
		if (m_pAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP)
			m_oGenerator.SetSamples(m_pLightsSelector->GetSelectionValue());
		else
			m_oGenerator.SetSamples(m_pRayDensitySelector->GetSelectionValue());
		m_oGenerator.SetRandomize(m_pRandomCheckBox->GetToggleState());
		m_oGenerator.SetCreaseEdges(m_pCreaseCheckBox->GetToggleState());
		m_oGenerator.SetGroundOcclusion(m_pGroundOcclusionCheckBox->GetToggleState());
		m_oGenerator.SetRayFalloff(m_pFalloffSelector->GetSelectionValue());
	}
	m_oGenerator.Generate();

	size_t iAO;
	if (m_oGenerator.DoneGenerating())
		iAO = m_oGenerator.GenerateTexture();

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		size_t& iAOTexture = m_bColor?(*m_paoMaterials)[i].m_iColorAO:(*m_paoMaterials)[i].m_iAO;

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		if (iAOTexture)
			glDeleteTextures(1, &iAOTexture);

		if (m_oGenerator.DoneGenerating())
			iAOTexture = iAO;
		else
			iAOTexture = 0;
	}

	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetText(_T("Generate"));
}

void CAOPanel::SaveMapCallback()
{
	if (!m_oGenerator.DoneGenerating())
		return;

	m_oGenerator.SaveToFile(SaveFileDialog(_T("Portable Network Graphics (.png)\0*.png\0Bitmap (.bmp)\0*.bmp\0JPEG (.jpg)\0*.jpg\0Truevision Targa (.tga)\0*.tga\0Adobe PhotoShop (.psd)\0*.psd\0")));
}

void CAOPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CAOPanel::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CAOPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static float flLastTime = 0;
	static float flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CModelWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	// We need to correct the viewport size before we push any events, or else any Layout() commands during
	// button presses and the line will use the wrong viewport size.
	glViewport(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());

	if (m_oGenerator.IsGenerating() && flLastTime - flLastGenerate > 0.5f)
	{
		size_t iAO = m_oGenerator.GenerateTexture(true);

		for (size_t i = 0; i < m_paoMaterials->size(); i++)
		{
			size_t& iAOTexture = m_bColor?(*m_paoMaterials)[i].m_iColorAO:(*m_paoMaterials)[i].m_iAO;

			if (iAOTexture)
				glDeleteTextures(1, &iAOTexture);

			iAOTexture = iAO;
		}

		flLastGenerate = CModelWindow::Get()->GetTime();
	}

	CModelWindow::Get()->Render();
	CRootPanel::Get()->Think(CModelWindow::Get()->GetTime());
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glfwSwapBuffers();

	flLastTime = CModelWindow::Get()->GetTime();
}

void CAOPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CAOPanel::FindBestRayFalloff()
{
	if (!m_pScene->GetNumMeshes())
		return;

	if (!m_bColor)
		m_pFalloffSelector->SetSelection(m_pFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
}

void CAOPanel::Open(bool bColor, CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
{
	CAOPanel* pPanel = Get(bColor);

	// Get rid of the last one, in case we've changed the scene.
	if (pPanel)
		delete pPanel;

	if (bColor)
		s_pColorAOPanel = new CAOPanel(true, pScene, paoMaterials);
	else
		s_pAOPanel = new CAOPanel(false, pScene, paoMaterials);

	pPanel = Get(bColor);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();

	pPanel->FindBestRayFalloff();
}

CAOPanel* CAOPanel::Get(bool bColor)
{
	return bColor?s_pColorAOPanel:s_pAOPanel;
}

void CAOPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

void CAOPanel::AOMethodCallback()
{
	// So we can appear/disappear the ray density bar if the AO method has changed.
	Layout();
}

COptionsButton::COptionsButton()
	: CButton(0, 0, 100, 100, _T(""), true)
{
	m_pPanel = new COptionsPanel(this);
	m_pPanel->SetVisible(false);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);
}

void COptionsButton::OpenCallback()
{
	CRootPanel::Get()->AddControl(m_pPanel, true);

	int iPanelWidth = m_pPanel->GetWidth();
	int iButtonWidth = GetWidth();
	int x, y, w, h;
	GetAbsDimensions(x, y, w, h);
	m_pPanel->SetPos(x + iButtonWidth/2 - iPanelWidth/2, y+h+3);

	m_pPanel->Layout();
	m_pPanel->SetVisible(true);
}

void COptionsButton::CloseCallback()
{
	CRootPanel::Get()->RemoveControl(m_pPanel);
	m_pPanel->SetVisible(false);

	SetState(false, false);
}

COptionsButton::COptionsPanel::COptionsPanel(COptionsButton* pButton)
	: CPanel(0, 0, 200, 350)
{
	m_pOkay = new CButton(0, 0, 100, 100, _T("Okay"));
	m_pOkay->SetClickedListener(pButton, Close);
	AddControl(m_pOkay);
}

void COptionsButton::COptionsPanel::Layout()
{
	BaseClass::Layout();

	m_pOkay->SetSize(40, 20);
	m_pOkay->SetPos(GetWidth()/2 - 20, GetHeight() - 30);
}

void COptionsButton::COptionsPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, g_clrBox);

	BaseClass::Paint(x, y, w, h);
}

CComboGeneratorPanel* CComboGeneratorPanel::s_pComboGeneratorPanel = NULL;

CComboGeneratorPanel::CComboGeneratorPanel(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
	: CMovablePanel(_T("Combo map generator")), m_oGenerator(pScene, paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	m_pMeshInstancePicker = NULL;

	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	m_pSizeLabel = new CLabel(0, 0, 32, 32, _T("Size"));
	AddControl(m_pSizeLabel);

	m_pSizeSelector = new CScrollSelector<int>();
#ifdef _DEBUG
	m_pSizeSelector->AddSelection(CScrollSelection<int>(16, _T("16x16")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(32, _T("32x32")));
#endif
	m_pSizeSelector->AddSelection(CScrollSelection<int>(64, _T("64x64")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(128, _T("128x128")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(256, _T("256x256")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(512, _T("512x512")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(1024, _T("1024x1024")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(2048, _T("2048x2048")));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(4096, _T("4096x4096")));
	m_pSizeSelector->SetSelection(4);
	AddControl(m_pSizeSelector);

	m_pLoResLabel = new CLabel(0, 0, 32, 32, _T("Low Resolution Meshes"));
	AddControl(m_pLoResLabel);

	m_pLoRes = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	m_pLoRes->SetBackgroundColor(g_clrBox);
	m_pLoRes->SetDroppedListener(this, DroppedLoResMesh);
	AddControl(m_pLoRes);

	m_pHiResLabel = new CLabel(0, 0, 32, 32, _T("High Resolution Meshes"));
	AddControl(m_pHiResLabel);

	m_pHiRes = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	m_pHiRes->SetBackgroundColor(g_clrBox);
	m_pHiRes->SetDroppedListener(this, DroppedHiResMesh);
	AddControl(m_pHiRes);

	m_pAddLoRes = new CButton(0, 0, 100, 100, _T("Add"));
	m_pAddLoRes->SetClickedListener(this, AddLoRes);
	AddControl(m_pAddLoRes);

	m_pAddHiRes = new CButton(0, 0, 100, 100, _T("Add"));
	m_pAddHiRes->SetClickedListener(this, AddHiRes);
	AddControl(m_pAddHiRes);

	m_pRemoveLoRes = new CButton(0, 0, 100, 100, _T("Remove"));
	m_pRemoveLoRes->SetClickedListener(this, RemoveLoRes);
	AddControl(m_pRemoveLoRes);

	m_pRemoveHiRes = new CButton(0, 0, 100, 100, _T("Remove"));
	m_pRemoveHiRes->SetClickedListener(this, RemoveHiRes);
	AddControl(m_pRemoveHiRes);

	m_pDiffuseCheckBox = new CCheckBox();
	m_pDiffuseCheckBox->SetState(true, false);
	AddControl(m_pDiffuseCheckBox);

	m_pDiffuseLabel = new CLabel(0, 0, 100, 100, _T("Diffuse"));
	AddControl(m_pDiffuseLabel);

	m_pAOCheckBox = new CCheckBox();
	m_pAOCheckBox->SetState(true, false);
	AddControl(m_pAOCheckBox);

	m_pAOLabel = new CLabel(0, 0, 100, 100, _T("Ambient Occlusion"));
	AddControl(m_pAOLabel);

	m_pAOOptions = new COptionsButton();
	AddControl(m_pAOOptions);

	m_pBleedLabel = new CLabel(0, 0, 100, 100, _T("Edge Bleed"));
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pBleedLabel);

	m_pBleedSelector = new CScrollSelector<int>();
	m_pBleedSelector->AddSelection(CScrollSelection<int>(0, _T("0")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(1, _T("1")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(2, _T("2")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(3, _T("3")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(4, _T("4")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(5, _T("5")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(6, _T("6")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(7, _T("7")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(8, _T("8")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(9, _T("9")));
	m_pBleedSelector->AddSelection(CScrollSelection<int>(10, _T("10")));
	m_pBleedSelector->SetSelection(1);
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pBleedSelector);

	m_pSamplesLabel = new CLabel(0, 0, 100, 100, _T("Samples"));
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pSamplesLabel);

	m_pSamplesSelector = new CScrollSelector<int>();
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(5, _T("5")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(6, _T("6")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(7, _T("7")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(8, _T("8")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(9, _T("9")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(10, _T("10")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(11, _T("11")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(12, _T("12")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(13, _T("13")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(14, _T("14")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(15, _T("15")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(16, _T("16")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(17, _T("17")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(18, _T("18")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(19, _T("19")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(20, _T("20")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(21, _T("21")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(22, _T("22")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(23, _T("23")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(24, _T("24")));
	m_pSamplesSelector->AddSelection(CScrollSelection<int>(25, _T("25")));
	m_pSamplesSelector->SetSelection(15);
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pSamplesSelector);

	m_pFalloffLabel = new CLabel(0, 0, 100, 100, _T("Ray Falloff"));
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pFalloffLabel);

	m_pFalloffSelector = new CScrollSelector<float>();
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.01f, _T("0.01")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.05f, _T("0.05")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.1f, _T("0.1")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.25f, _T("0.25")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.5f, _T("0.5")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.75f, _T("0.75")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.0f, _T("1.0")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.25f, _T("1.25")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.5f, _T("1.5")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.75f, _T("1.75")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(2.0f, _T("2.0")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(2.5f, _T("2.5")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(3.0f, _T("3.0")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(4.0f, _T("4.0")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(5.0f, _T("5.0")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(6.0f, _T("6.0")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(7.5f, _T("7.5")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(10.0f, _T("10")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(25.0f, _T("25")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(50.0f, _T("50")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(100.0f, _T("100")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(250.0f, _T("250")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(500.0f, _T("500")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(1000.0f, _T("1000")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(2500.0f, _T("2500")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(5000.0f, _T("5000")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(10000.0f, _T("10000")));
	m_pFalloffSelector->AddSelection(CScrollSelection<float>(-1.0f, _T("No falloff")));
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pFalloffSelector);

	m_pRandomCheckBox = new CCheckBox();
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pRandomCheckBox);

	m_pRandomLabel = new CLabel(0, 0, 100, 100, _T("Randomize rays"));
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pRandomLabel);

	m_pGroundOcclusionCheckBox = new CCheckBox();
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pGroundOcclusionCheckBox);

	m_pGroundOcclusionLabel = new CLabel(0, 0, 100, 100, _T("Ground occlusion"));
	m_pAOOptions->GetOptionsPanel()->AddControl(m_pGroundOcclusionLabel);

	m_pNormalCheckBox = new CCheckBox();
	m_pNormalCheckBox->SetState(true, false);
	AddControl(m_pNormalCheckBox);

	m_pNormalLabel = new CLabel(0, 0, 100, 100, _T("Normal Map"));
	AddControl(m_pNormalLabel);

	m_pGenerate = new CButton(0, 0, 100, 100, _T("Generate"));
	m_pGenerate->SetClickedListener(this, Generate);
	AddControl(m_pGenerate);

	m_pSave = new CButton(0, 0, 100, 100, _T("Save Map"));
	AddControl(m_pSave);

	m_pSave->SetClickedListener(this, SaveMap);
	m_pSave->SetVisible(false);

	Layout();
}

void CComboGeneratorPanel::Layout()
{
	int iSpace = 20;

	m_pSizeLabel->EnsureTextFits();

	int iSelectorSize = m_pSizeLabel->GetHeight() - 4;

	m_pSizeSelector->SetSize(GetWidth() - m_pSizeLabel->GetWidth() - iSpace, iSelectorSize);

	int iControlY = HEADER_HEIGHT;

	m_pSizeSelector->SetPos(GetWidth() - m_pSizeSelector->GetWidth() - iSpace/2, iControlY);
	m_pSizeLabel->SetPos(5, iControlY);

	int iTreeWidth = GetWidth()/2-15;

	m_pLoResLabel->EnsureTextFits();
	m_pLoResLabel->SetPos(10, 40);

	m_pLoRes->SetSize(iTreeWidth, 150);
	m_pLoRes->SetPos(10, 70);

	m_pAddLoRes->SetSize(40, 20);
	m_pAddLoRes->SetPos(10, 225);

	m_pRemoveLoRes->SetSize(60, 20);
	m_pRemoveLoRes->SetPos(60, 225);

	m_pHiResLabel->EnsureTextFits();
	m_pHiResLabel->SetPos(iTreeWidth+20, 40);

	m_pHiRes->SetSize(iTreeWidth, 150);
	m_pHiRes->SetPos(iTreeWidth+20, 70);

	m_pAddHiRes->SetSize(40, 20);
	m_pAddHiRes->SetPos(iTreeWidth+20, 225);

	m_pRemoveHiRes->SetSize(60, 20);
	m_pRemoveHiRes->SetPos(iTreeWidth+70, 225);

	m_pDiffuseLabel->SetPos(35, 220);
	m_pDiffuseLabel->EnsureTextFits();
	m_pDiffuseLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pDiffuseLabel->SetWrap(false);
	m_pDiffuseCheckBox->SetPos(20, 220 + m_pDiffuseLabel->GetHeight()/2 - m_pDiffuseCheckBox->GetHeight()/2);

	m_pAOLabel->SetPos(35, 250);
	m_pAOLabel->EnsureTextFits();
	m_pAOLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pAOLabel->SetWrap(false);
	m_pAOCheckBox->SetPos(20, 250 + m_pAOLabel->GetHeight()/2 - m_pAOCheckBox->GetHeight()/2);

	m_pAOOptions->SetPos(250, 250 + m_pAOLabel->GetHeight()/2 - m_pAOOptions->GetHeight()/2);
	m_pAOOptions->SetSize(60, 20);
	m_pAOOptions->SetText(_T("Options..."));

	int iControl = 3;

	m_pAOOptions->GetOptionsPanel()->SetSize(200, 200);

	iControlY = 10;

	m_pBleedLabel->SetSize(10, 10);
	m_pBleedLabel->EnsureTextFits();
	m_pBleedLabel->SetPos(5, iControlY);

	m_pBleedSelector->SetSize(m_pAOOptions->GetOptionsPanel()->GetWidth() - m_pBleedLabel->GetWidth() - iSpace, iSelectorSize);
	m_pBleedSelector->SetPos(m_pAOOptions->GetOptionsPanel()->GetWidth() - m_pBleedSelector->GetWidth() - iSpace/2, iControlY);

	iControlY += 30;

	m_pSamplesLabel->SetSize(10, 10);
	m_pSamplesLabel->EnsureTextFits();
	m_pSamplesLabel->SetPos(5, iControlY);

	m_pSamplesSelector->SetSize(m_pAOOptions->GetOptionsPanel()->GetWidth() - m_pSamplesLabel->GetWidth() - iSpace, iSelectorSize);
	m_pSamplesSelector->SetPos(m_pAOOptions->GetOptionsPanel()->GetWidth() - m_pSamplesSelector->GetWidth() - iSpace/2, iControlY);

	iControlY += 30;

	m_pFalloffLabel->SetSize(10, 10);
	m_pFalloffLabel->EnsureTextFits();
	m_pFalloffLabel->SetPos(5, iControlY);

	m_pFalloffSelector->SetSize(m_pAOOptions->GetOptionsPanel()->GetWidth() - m_pFalloffLabel->GetWidth() - iSpace, iSelectorSize);
	m_pFalloffSelector->SetPos(m_pAOOptions->GetOptionsPanel()->GetWidth() - m_pFalloffSelector->GetWidth() - iSpace/2, iControlY);

	iControlY += 30;

	m_pRandomLabel->SetSize(10, 10);
	m_pRandomLabel->EnsureTextFits();
	m_pRandomLabel->SetPos(25, iControlY);

	m_pRandomCheckBox->SetPos(10, iControlY + m_pRandomLabel->GetHeight()/2 - m_pRandomCheckBox->GetHeight()/2);

	iControlY += 30;

	m_pGroundOcclusionLabel->SetSize(10, 10);
	m_pGroundOcclusionLabel->EnsureTextFits();
	m_pGroundOcclusionLabel->SetPos(25, iControlY);

	m_pGroundOcclusionCheckBox->SetPos(10, iControlY + m_pGroundOcclusionLabel->GetHeight()/2 - m_pGroundOcclusionCheckBox->GetHeight()/2);

	m_pNormalLabel->SetPos(35, 280);
	m_pNormalLabel->EnsureTextFits();
	m_pNormalLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pNormalLabel->SetWrap(false);
	m_pNormalCheckBox->SetPos(20, 280 + m_pNormalLabel->GetHeight()/2 - m_pNormalCheckBox->GetHeight()/2);

	m_pGenerate->SetSize(100, 33);
	m_pGenerate->SetPos(GetWidth()/2 - m_pGenerate->GetWidth()/2, GetHeight() - (int)(m_pGenerate->GetHeight()*3));

	m_pSave->SetSize(100, 33);
	m_pSave->SetPos(GetWidth()/2 - m_pSave->GetWidth()/2, GetHeight() - (int)(m_pSave->GetHeight()*1.5f));
	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	size_t i;
	m_pLoRes->ClearTree();
	if (!m_apLoResMeshes.size())
		m_pLoRes->AddNode(_T("No meshes. Click 'Add'"));
	else
	{
		for (i = 0; i < m_apLoResMeshes.size(); i++)
		{
			m_pLoRes->AddNode<CConversionMeshInstance>(m_apLoResMeshes[i]->GetMesh()->GetName(), m_apLoResMeshes[i]);
			m_pLoRes->GetNode(i)->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
		}
	}

	m_pHiRes->ClearTree();
	if (!m_apHiResMeshes.size())
		m_pHiRes->AddNode(_T("No meshes. Click 'Add'"));
	else
	{
		for (i = 0; i < m_apHiResMeshes.size(); i++)
		{
			m_pHiRes->AddNode<CConversionMeshInstance>(m_apHiResMeshes[i]->GetMesh()->GetName(), m_apHiResMeshes[i]);
			m_pHiRes->GetNode(i)->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
		}
	}

	CMovablePanel::Layout();
}

void CComboGeneratorPanel::UpdateScene()
{
	m_apLoResMeshes.clear();
	m_apHiResMeshes.clear();

	if (m_pScene->GetNumMeshes())
		m_pFalloffSelector->SetSelection(m_pFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
}

void CComboGeneratorPanel::Think()
{
	bool bFoundMaterial = false;
	for (size_t iMesh = 0; iMesh < m_apLoResMeshes.size(); iMesh++)
	{
		CConversionMeshInstance* pMeshInstance = m_apLoResMeshes[iMesh];

		for (size_t iMaterialStub = 0; iMaterialStub < pMeshInstance->GetMesh()->GetNumMaterialStubs(); iMaterialStub++)
		{
			size_t iMaterial = pMeshInstance->GetMappedMaterial(iMaterialStub)->m_iMaterial;

			// Materials not loaded yet?
			if (!m_paoMaterials->size())
				continue;

			bFoundMaterial = true;
			break;
		}
	}

	CMovablePanel::Think();
}

void CComboGeneratorPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);

	if (!m_bMinimized)
	{
		CRootPanel::PaintRect(x+10, y+250, w-20, 1, g_clrBoxHi);
		CRootPanel::PaintRect(x+10, y+h-110, w-20, 1, g_clrBoxHi);
	}
}

bool CComboGeneratorPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CComboGeneratorPanel::GenerateCallback()
{
	if (m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_pSave->SetVisible(false);

	m_pGenerate->SetText(_T("Cancel"));

	CModelWindow::Get()->SetDisplayNormal(true);

	// Disappear all of the hi-res meshes so we can see the lo res better.
	for (size_t m = 0; m < m_apHiResMeshes.size(); m++)
		m_apHiResMeshes[m]->SetVisible(false);

	for (size_t m = 0; m < m_apLoResMeshes.size(); m++)
	{
		if (m_apLoResMeshes[m]->GetMesh()->GetNumFaces() > 10000)
			continue;

		m_apLoResMeshes[m]->SetVisible(true);
	}

	int iSize = m_pSizeSelector->GetSelectionValue();
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.ClearMethods();

	if (m_pDiffuseCheckBox->GetState())
	{
		m_oGenerator.AddDiffuse();
		CModelWindow::Get()->SetDisplayTexture(true);
	}

	if (m_pAOCheckBox->GetState())
	{
		m_oGenerator.AddAO(
			m_pSamplesSelector->GetSelectionValue(),
			m_pRandomCheckBox->GetState(),
			m_pFalloffSelector->GetSelectionValue(),
			m_pGroundOcclusionCheckBox->GetState(),
			m_pBleedSelector->GetSelectionValue());
		CModelWindow::Get()->SetDisplayAO(true);
	}

	if (m_pNormalCheckBox->GetState())
	{
		m_oGenerator.AddNormal();
		CModelWindow::Get()->SetDisplayNormal(true);
	}

	m_oGenerator.SetModels(m_apHiResMeshes, m_apLoResMeshes);
	m_oGenerator.SetWorkListener(this);
	m_oGenerator.Generate();

	size_t iDiffuse = 0;
	size_t iAO = 0;
	size_t iNormalGL = 0;
	size_t iNormalIL = 0;
	if (m_oGenerator.DoneGenerating())
	{
		iDiffuse = m_oGenerator.GenerateDiffuse();
		iAO = m_oGenerator.GenerateAO();
		m_oGenerator.GenerateNormal(iNormalGL, iNormalIL);
	}

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		size_t& iDiffuseTexture = (*m_paoMaterials)[i].m_iBase;
		size_t& iAOTexture = (*m_paoMaterials)[i].m_iAO;
		size_t& iNormalGLTexture = (*m_paoMaterials)[i].m_iNormal;
		size_t& iNormalILTexture = (*m_paoMaterials)[i].m_iNormalIL;

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		if (iDiffuseTexture)
			glDeleteTextures(1, &iDiffuseTexture);

		if (m_oGenerator.DoneGenerating())
			iDiffuseTexture = iDiffuse;
		else
			iDiffuseTexture = 0;

		if (iAOTexture)
			glDeleteTextures(1, &iAOTexture);

		if (m_oGenerator.DoneGenerating())
			iAOTexture = iAO;
		else
			iAOTexture = 0;

		if (iNormalGLTexture)
			glDeleteTextures(1, &iNormalGLTexture);

		if (m_oGenerator.DoneGenerating())
			iNormalGLTexture = iNormalGL;
		else
			iNormalGLTexture = 0;

		if (iNormalILTexture)
			glDeleteTextures(1, &iNormalILTexture);

		if (m_oGenerator.DoneGenerating())
			iNormalILTexture = iNormalIL;
		else
			iNormalILTexture = 0;
	}

	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetText(_T("Generate"));
}

void CComboGeneratorPanel::SaveMapCallback()
{
	if (!m_oGenerator.DoneGenerating())
		return;

	const tchar* pszFilename = SaveFileDialog(_T("Portable Network Graphics (.png)\0*.png\0Bitmap (.bmp)\0*.bmp\0JPEG (.jpg)\0*.jpg\0Truevision Targa (.tga)\0*.tga\0Adobe PhotoShop (.psd)\0*.psd\0"));

	if (!pszFilename)
		return;

	m_oGenerator.SaveAll(pszFilename);

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		m_pScene->GetMaterial(i)->m_sNormalTexture = pszFilename;
	}

	CRootPanel::Get()->Layout();
}

void CComboGeneratorPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CComboGeneratorPanel::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CComboGeneratorPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static float flLastTime = 0;
	static float flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CModelWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	// We need to correct the viewport size before we push any events, or else any Layout() commands during
	// button presses and the line will use the wrong viewport size.
	glViewport(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());

	if (m_oGenerator.IsGenerating() && CModelWindow::Get()->GetTime() - flLastGenerate > 0.5f)
	{
		size_t iDiffuse = m_oGenerator.GenerateDiffuse(true);
		size_t iAO = m_oGenerator.GenerateAO(true);
		size_t iNormal, iNormalIL;
		m_oGenerator.GenerateNormal(iNormal, iNormalIL, true);

		for (size_t i = 0; i < m_paoMaterials->size(); i++)
		{
			size_t& iDiffuseTexture = (*m_paoMaterials)[i].m_iBase;
			size_t& iAOTexture = (*m_paoMaterials)[i].m_iAO;
			size_t& iNormalTexture = (*m_paoMaterials)[i].m_iNormal;

			if (iDiffuseTexture)
				glDeleteTextures(1, &iDiffuseTexture);

			iDiffuseTexture = iDiffuse;

			if (iAOTexture)
				glDeleteTextures(1, &iAOTexture);

			iAOTexture = iAO;

			if (iNormalTexture)
				glDeleteTextures(1, &iNormalTexture);

			iNormalTexture = iNormal;
		}

		flLastGenerate = CModelWindow::Get()->GetTime();
	}

	CModelWindow::Get()->Render();
	CRootPanel::Get()->Think(CModelWindow::Get()->GetTime());
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glfwSwapBuffers();

	flLastTime = CModelWindow::Get()->GetTime();
}

void CComboGeneratorPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CComboGeneratorPanel::AddLoResCallback()
{
	if (m_pMeshInstancePicker)
		delete m_pMeshInstancePicker;

	m_pMeshInstancePicker = new CMeshInstancePicker(this, AddLoResMesh);

	int x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_pMeshInstancePicker->GetSize(pw, ph);
	m_pMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CComboGeneratorPanel::AddHiResCallback()
{
	if (m_pMeshInstancePicker)
		delete m_pMeshInstancePicker;

	m_pMeshInstancePicker = new CMeshInstancePicker(this, AddHiResMesh);

	int x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_pMeshInstancePicker->GetSize(pw, ph);
	m_pMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CComboGeneratorPanel::AddLoResMeshCallback()
{
	CConversionMeshInstance* pMeshInstance = m_pMeshInstancePicker->GetPickedMeshInstance();
	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apLoResMeshes.push_back(pMeshInstance);

	delete m_pMeshInstancePicker;
	m_pMeshInstancePicker = NULL;

	Layout();
}

void CComboGeneratorPanel::AddHiResMeshCallback()
{
	CConversionMeshInstance* pMeshInstance = m_pMeshInstancePicker->GetPickedMeshInstance();
	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apHiResMeshes.push_back(pMeshInstance);

	delete m_pMeshInstancePicker;
	m_pMeshInstancePicker = NULL;

	Layout();
}

void CComboGeneratorPanel::RemoveLoResCallback()
{
	CTreeNode* pNode = m_pLoRes->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);
	if (!pMeshNode)
		return;

	for (size_t i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshNode->GetObject())
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	Layout();
}

void CComboGeneratorPanel::RemoveHiResCallback()
{
	CTreeNode* pNode = m_pHiRes->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);
	if (!pMeshNode)
		return;

	for (size_t i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshNode->GetObject())
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	Layout();
}

void CComboGeneratorPanel::DroppedLoResMeshCallback()
{
	IDraggable* pDraggable = CRootPanel::Get()->GetCurrentDraggable();
	CConversionMeshInstance* pMeshInstance = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pDraggable)->GetObject();

	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apLoResMeshes.push_back(pMeshInstance);

	Layout();
}

void CComboGeneratorPanel::DroppedHiResMeshCallback()
{
	IDraggable* pDraggable = CRootPanel::Get()->GetCurrentDraggable();
	CConversionMeshInstance* pMeshInstance = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pDraggable)->GetObject();

	if (!pMeshInstance)
		return;

	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	m_apHiResMeshes.push_back(pMeshInstance);

	delete m_pMeshInstancePicker;
	m_pMeshInstancePicker = NULL;

	Layout();
}

void CComboGeneratorPanel::Open(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
{
	CComboGeneratorPanel* pPanel = s_pComboGeneratorPanel;

	if (pPanel)
		delete pPanel;

	pPanel = s_pComboGeneratorPanel = new CComboGeneratorPanel(pScene, paoMaterials);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();

	if (pScene->GetNumMeshes())
		pPanel->m_pFalloffSelector->SetSelection(pPanel->m_pFalloffSelector->FindClosestSelectionValue(pScene->m_oExtends.Size().Length()/2));
}

void CComboGeneratorPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

CNormalPanel* CNormalPanel::s_pNormalPanel = NULL;

CNormalPanel::CNormalPanel(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
	: CMovablePanel(_T("Normal map generator")), m_oGenerator(pScene, paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	m_pMaterialsLabel = new CLabel(0, 0, 32, 32, _T("Choose A Material To Generate From:"));
	AddControl(m_pMaterialsLabel);

	m_pMaterials = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	m_pMaterials->SetBackgroundColor(g_clrBox);
	AddControl(m_pMaterials);

	m_pProgressLabel = new CLabel(0, 0, 100, 100, _T(""));
	AddControl(m_pProgressLabel);

	m_pDepthLabel = new CLabel(0, 0, 32, 32, _T("Overall Depth"));
	AddControl(m_pDepthLabel);

	m_pDepthSelector = new CScrollSelector<float>();
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.0f, _T("0%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.025f, _T("2.5%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.05f, _T("5%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.075f, _T("7.5%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.1f, _T("10%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.2f, _T("20%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.3f, _T("30%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.4f, _T("40%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.5f, _T("50%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.6f, _T("60%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.7f, _T("70%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.8f, _T("80%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(0.9f, _T("90%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.0f, _T("100%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.1f, _T("110%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.2f, _T("120%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.3f, _T("130%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.4f, _T("140%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.5f, _T("150%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.6f, _T("160%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.7f, _T("170%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.8f, _T("180%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(1.9f, _T("190%")));
	m_pDepthSelector->AddSelection(CScrollSelection<float>(2.0f, _T("200%")));
	m_pDepthSelector->SetSelection(13);
	m_pDepthSelector->SetSelectedListener(this, UpdateNormal2);
	AddControl(m_pDepthSelector);

	m_pHiDepthLabel = new CLabel(0, 0, 32, 32, _T("Texture Hi-Freq Depth"));
	AddControl(m_pHiDepthLabel);

	m_pHiDepthSelector = new CScrollSelector<float>();
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.0f, _T("0%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.025f, _T("2.5%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.05f, _T("5%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.075f, _T("7.5%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.1f, _T("10%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.2f, _T("20%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.3f, _T("30%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.4f, _T("40%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.5f, _T("50%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.6f, _T("60%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.7f, _T("70%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.8f, _T("80%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.9f, _T("90%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.0f, _T("100%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.1f, _T("110%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.2f, _T("120%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.3f, _T("130%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.4f, _T("140%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.5f, _T("150%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.6f, _T("160%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.7f, _T("170%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.8f, _T("180%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.9f, _T("190%")));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(2.0f, _T("200%")));
	m_pHiDepthSelector->SetSelection(13);
	m_pHiDepthSelector->SetSelectedListener(this, UpdateNormal2);
	AddControl(m_pHiDepthSelector);

	m_pMidDepthLabel = new CLabel(0, 0, 32, 32, _T("Texture Mid-Freq Depth"));
	AddControl(m_pMidDepthLabel);

	m_pMidDepthSelector = new CScrollSelector<float>();
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.0f, _T("0%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.025f, _T("2.5%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.05f, _T("5%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.075f, _T("7.5%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.1f, _T("10%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.2f, _T("20%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.3f, _T("30%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.4f, _T("40%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.5f, _T("50%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.6f, _T("60%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.7f, _T("70%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.8f, _T("80%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(0.9f, _T("90%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.0f, _T("100%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.1f, _T("110%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.2f, _T("120%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.3f, _T("130%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.4f, _T("140%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.5f, _T("150%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.6f, _T("160%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.7f, _T("170%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.8f, _T("180%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(1.9f, _T("190%")));
	m_pMidDepthSelector->AddSelection(CScrollSelection<float>(2.0f, _T("200%")));
	m_pMidDepthSelector->SetSelection(13);
	m_pMidDepthSelector->SetSelectedListener(this, UpdateNormal2);
	AddControl(m_pMidDepthSelector);

	m_pLoDepthLabel = new CLabel(0, 0, 32, 32, _T("Texture Lo-Freq Depth"));
	AddControl(m_pLoDepthLabel);

	m_pLoDepthSelector = new CScrollSelector<float>();
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.0f, _T("0%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.025f, _T("2.5%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.05f, _T("5%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.075f, _T("7.5%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.1f, _T("10%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.2f, _T("20%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.3f, _T("30%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.4f, _T("40%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.5f, _T("50%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.6f, _T("60%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.7f, _T("70%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.8f, _T("80%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.9f, _T("90%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.0f, _T("100%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.1f, _T("110%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.2f, _T("120%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.3f, _T("130%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.4f, _T("140%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.5f, _T("150%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.6f, _T("160%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.7f, _T("170%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.8f, _T("180%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.9f, _T("190%")));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(2.0f, _T("200%")));
	m_pLoDepthSelector->SetSelection(13);
	m_pLoDepthSelector->SetSelectedListener(this, UpdateNormal2);
	AddControl(m_pLoDepthSelector);

	m_pSave = new CButton(0, 0, 100, 100, _T("Save Map"));
	AddControl(m_pSave);

	m_pSave->SetClickedListener(this, SaveMap);
	m_pSave->SetVisible(false);

	Layout();
}

void CNormalPanel::Layout()
{
	int iSpace = 20;

	m_pMaterialsLabel->EnsureTextFits();

	int iSelectorSize = m_pMaterialsLabel->GetHeight() - 4;
	int iControlY = HEADER_HEIGHT;

	m_pMaterialsLabel->SetPos(5, iControlY);

	int iTreeWidth = GetWidth()/2-15;

	m_pMaterials->ClearTree();
	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		m_pMaterials->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));

		if (m_paoMaterials->size() > i && !m_paoMaterials->at(i).m_iBase)
			m_pMaterials->GetNode(i)->m_pLabel->SetAlpha(100);

		m_pMaterials->SetSelectedListener(this, SetupNormal2);
	}

	m_pMaterials->SetSize(iTreeWidth, 150);
	m_pMaterials->SetPos(GetWidth()/2-iTreeWidth/2, 50);

	m_pProgressLabel->SetSize(1, 1);
	m_pProgressLabel->SetPos(35, 220);
	m_pProgressLabel->EnsureTextFits();
	m_pProgressLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pProgressLabel->SetWrap(false);

	m_pDepthLabel->SetPos(10, 290);
	m_pDepthLabel->EnsureTextFits();
	m_pDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pDepthLabel->SetWrap(false);
	m_pDepthSelector->SetPos(m_pDepthLabel->GetRight() + 10, 290 + m_pDepthLabel->GetHeight()/2 - m_pDepthSelector->GetHeight()/2);
	m_pDepthSelector->SetRight(GetWidth() - 10);

	m_pHiDepthLabel->SetPos(10, 310);
	m_pHiDepthLabel->EnsureTextFits();
	m_pHiDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pHiDepthLabel->SetWrap(false);
	m_pHiDepthSelector->SetPos(m_pHiDepthLabel->GetRight() + 10, 310 + m_pHiDepthLabel->GetHeight()/2 - m_pHiDepthSelector->GetHeight()/2);
	m_pHiDepthSelector->SetRight(GetWidth() - 10);

	m_pMidDepthLabel->SetPos(10, 330);
	m_pMidDepthLabel->EnsureTextFits();
	m_pMidDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pMidDepthLabel->SetWrap(false);
	m_pMidDepthSelector->SetPos(m_pMidDepthLabel->GetRight() + 10, 330 + m_pMidDepthLabel->GetHeight()/2 - m_pMidDepthSelector->GetHeight()/2);
	m_pMidDepthSelector->SetRight(GetWidth() - 10);

	m_pLoDepthLabel->SetPos(10, 350);
	m_pLoDepthLabel->EnsureTextFits();
	m_pLoDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pLoDepthLabel->SetWrap(false);
	m_pLoDepthSelector->SetPos(m_pLoDepthLabel->GetRight() + 10, 350 + m_pLoDepthLabel->GetHeight()/2 - m_pLoDepthSelector->GetHeight()/2);
	m_pLoDepthSelector->SetRight(GetWidth() - 10);

	m_pSave->SetSize(100, 33);
	m_pSave->SetPos(GetWidth()/2 - m_pSave->GetWidth()/2, GetHeight() - (int)(m_pSave->GetHeight()*1.5f));
	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	CMovablePanel::Layout();
}

void CNormalPanel::UpdateScene()
{
	Layout();
}

void CNormalPanel::Think()
{
	if (m_oGenerator.IsNewNormal2Available())
	{
		size_t iNormal2, iNormal2IL;
		m_oGenerator.GetNormalMap2(iNormal2, iNormal2IL);

		for (size_t i = 0; i < m_paoMaterials->size(); i++)
		{
			size_t& iNormalTexture = (*m_paoMaterials)[i].m_iNormal2;
			size_t& iNormalIL = (*m_paoMaterials)[i].m_iNormal2IL;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			if (iNormalTexture)
				glDeleteTextures(1, &iNormalTexture);

			if (iNormalIL)
				ilDeleteImages(1, &iNormalIL);

			iNormalTexture = iNormal2;
			iNormalIL = iNormal2IL;
			break;
		}

		m_pSave->SetVisible(!!iNormal2);

		if (!!iNormal2)
			CModelWindow::Get()->SetDisplayNormal(true);
	}

	if (m_oGenerator.IsSettingUp())
	{
		tstring s;
		s = sprintf(tstring("Setting up... %d%%"), (int)(m_oGenerator.GetSetupProgress()*100));
		m_pProgressLabel->SetText(s);
		m_pSave->SetVisible(false);
	}
	else if (m_oGenerator.IsGeneratingNewNormal2())
	{
		tstring s;
		s = sprintf(tstring("Generating... %d%%"), (int)(m_oGenerator.GetNormal2GenerationProgress()*100));
		m_pProgressLabel->SetText(s);
		m_pSave->SetVisible(false);
	}
	else
		m_pProgressLabel->SetText(_T(""));

	m_oGenerator.Think();

	CMovablePanel::Think();
}

void CNormalPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);

	if (!m_bMinimized)
	{
		CRootPanel::PaintRect(x+10, y+295, w-20, 1, g_clrBoxHi);
		CRootPanel::PaintRect(x+10, y+385, w-20, 1, g_clrBoxHi);
	}
}

bool CNormalPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CNormalPanel::SaveMapCallback()
{
	if (!m_oGenerator.DoneGenerating())
		return;

	const tchar* pszFilename = SaveFileDialog(_T("Portable Network Graphics (.png)\0*.png\0Bitmap (.bmp)\0*.bmp\0JPEG (.jpg)\0*.jpg\0Truevision Targa (.tga)\0*.tga\0Adobe PhotoShop (.psd)\0*.psd\0"));

	if (!pszFilename)
		return;

	ModelWindow()->SaveNormal(m_oGenerator.GetGenerationMaterial(), pszFilename);

	CRootPanel::Get()->Layout();
}

void CNormalPanel::SetupNormal2Callback()
{
	m_oGenerator.SetNormalTextureDepth(m_pDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureHiDepth(m_pHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureMidDepth(m_pMidDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_pLoDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTexture(true, m_pMaterials->GetSelectedNodeId());

	CModelWindow::Get()->SetDisplayNormal(true);
}

void CNormalPanel::UpdateNormal2Callback()
{
	m_oGenerator.SetNormalTextureDepth(m_pDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureHiDepth(m_pHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureMidDepth(m_pMidDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_pLoDepthSelector->GetSelectionValue());
	m_oGenerator.UpdateNormal2();
}

void CNormalPanel::Open(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
{
	CNormalPanel* pPanel = s_pNormalPanel;

	if (pPanel)
		delete pPanel;

	pPanel = s_pNormalPanel = new CNormalPanel(pScene, paoMaterials);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();
}

void CNormalPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

CHelpPanel* CHelpPanel::s_pHelpPanel = NULL;

CHelpPanel::CHelpPanel()
	: CMovablePanel(_T("Help"))
{
	m_pInfo = new CLabel(0, 0, 100, 100, _T(""));
	AddControl(m_pInfo);
	Layout();
}

void CHelpPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 200);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pInfo->SetSize(GetWidth(), GetHeight());
	m_pInfo->SetPos(0, 30);

	m_pInfo->SetText(_T("CONTROLS:\n"));
	m_pInfo->AppendText(_T("Left Mouse Button - Move the camera\n"));
	m_pInfo->AppendText(_T("Right Mouse Button - Zoom in and out\n"));
	m_pInfo->AppendText(_T("Ctrl-LMB - Rotate the light\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("For in-depth help information please visit our website, http://www.getsmak.net/\n"));

	CMovablePanel::Layout();
}

void CHelpPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CHelpPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CHelpPanel::Open()
{
	if (!s_pHelpPanel)
		s_pHelpPanel = new CHelpPanel();

	s_pHelpPanel->SetVisible(true);
	s_pHelpPanel->Layout();
}

void CHelpPanel::Close()
{
	if (!s_pHelpPanel)
		return;

	s_pHelpPanel->SetVisible(false);
}

CAboutPanel* CAboutPanel::s_pAboutPanel = NULL;

CAboutPanel::CAboutPanel()
	: CMovablePanel(_T("About SMAK"))
{
	m_pInfo = new CLabel(0, 0, 100, 100, _T(""));
	AddControl(m_pInfo);
	Layout();
}

void CAboutPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 250);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pInfo->SetSize(GetWidth(), GetHeight());
	m_pInfo->SetPos(0, 30);

	m_pInfo->SetText(_T("SMAK - The Super Model Army Knife\n"));
	m_pInfo->AppendText(_T("Version ") SMAK_VERSION _T("\n"));
	m_pInfo->AppendText(_T("Copyright © 2010, Jorge Rodriguez <jorge@lunarworkshop.com>\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("FCollada copyright © 2006, Feeling Software\n"));
	m_pInfo->AppendText(_T("DevIL copyright © 2001-2009, Denton Woods\n"));
	m_pInfo->AppendText(_T("FTGL copyright © 2001-2003, Henry Maddocks\n"));
	m_pInfo->AppendText(_T("GLFW copyright © 2002-2007, Camilla Berglund\n"));
	m_pInfo->AppendText(_T("pthreads-win32 copyright © 2001, 2006 Ross P. Johnson\n"));
	m_pInfo->AppendText(_T("GLEW copyright © 2002-2007, Milan Ikits, Marcelo E. Magallon, Lev Povalahev\n"));
	m_pInfo->AppendText(_T("Freetype copyright © 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg\n"));

	CMovablePanel::Layout();
}

void CAboutPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CAboutPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CAboutPanel::Open()
{
	if (!s_pAboutPanel)
		s_pAboutPanel = new CAboutPanel();

	s_pAboutPanel->SetVisible(true);
	s_pAboutPanel->Layout();
}

void CAboutPanel::Close()
{
	if (!s_pAboutPanel)
		return;

	s_pAboutPanel->SetVisible(false);
}

CRegisterPanel* CRegisterPanel::s_pRegisterPanel = NULL;

CRegisterPanel::CRegisterPanel()
	: CMovablePanel(_T("Register SMAK"))
{
	m_pWebsiteButton = new CButton(0, 0, 100, 100, _T("Visit www.getsmak.net"));
	m_pWebsiteButton->SetClickedListener(this, Website);
	AddControl(m_pWebsiteButton);

	m_pInfo = new CLabel(0, 0, 100, 100, _T(""));
	AddControl(m_pInfo);

	m_pPirates = new CButton(0, 0, 1, 1, _T("A message to pirates"));
	m_pPirates->SetClickedListener(this, Pirates);
	AddControl(m_pPirates);

	m_pRegistrationKey = new CTextField();
	AddControl(m_pRegistrationKey);

	m_pRegister = new CButton(0, 0, 100, 100, _T("Register"));
	m_pRegister->SetClickedListener(this, Register);
	AddControl(m_pRegister);

	m_pRegisterResult = new CLabel(0, 0, 100, 100, _T(""));
	AddControl(m_pRegisterResult);

	m_pRegisterOffline = new CButton(0, 0, 100, 100, _T("Register Offline"));
	m_pRegisterOffline->SetFont(_T("sans-serif"), 11);
	m_pRegisterOffline->SetClickedListener(this, RegisterOffline);
	AddControl(m_pRegisterOffline);

	m_pProductCode = NULL;

	Layout();
}

void CRegisterPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 400);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pWebsiteButton->SetSize(120, 40);
	m_pWebsiteButton->SetPos(GetWidth()/2 - m_pWebsiteButton->GetWidth()/2, 40);

	if (m_pProductCode)
		m_pProductCode->SetVisible(false);

	m_pInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pInfo->SetSize(GetWidth(), GetHeight()-100);
	m_pInfo->SetPos(0, 100);
	m_pInfo->SetVisible(true);

	if (!ModelWindow()->IsRegistered())
	{
		m_pInfo->SetText(_T("Steps to Register SMAK\n"));
		m_pInfo->AppendText(_T(" \n"));
		m_pInfo->AppendText(_T("1. Depress the above button to visit http://www.getsmak.net/ and purchase SMAK in the onsite store.\n"));
		m_pInfo->AppendText(_T("2. You will be sent an registration code in your email. Select it and use the COPY command (Ctrl-c)\n"));
		m_pInfo->AppendText(_T("3. Paste your registration code into the box below (Ctrl-v)\n"));
		m_pInfo->AppendText(_T("4. ...?\n"));
		m_pInfo->AppendText(_T("5. Profit!\n"));

		m_pInfo->AppendText(_T(" \n"));
		m_pInfo->AppendText(_T(" \n"));
	}
	else
	{
		m_pInfo->SetText(_T("Your installation of SMAK is now fully registered. Thanks!\n"));
		m_pRegistrationKey->SetVisible(false);
		m_pRegister->SetVisible(false);
		m_pRegisterOffline->SetVisible(false);
	}

	m_pRegistrationKey->SetPos(GetWidth()/2 - m_pRegistrationKey->GetWidth()/2, 250);

	m_pRegister->SetSize(100, 30);
	m_pRegister->SetPos(GetWidth()/2 - m_pRegister->GetWidth()/2, 290);
	m_pRegister->SetClickedListener(this, Register);

	m_pRegisterResult->SetPos(10, 330);
	m_pRegisterResult->SetSize(GetWidth()-20, GetHeight());
	m_pRegisterResult->SetAlign(CLabel::TA_TOPCENTER);

	m_pRegisterOffline->SetText(_T("Register Offline"));
	m_pRegisterOffline->SetSize(60, 40);
	m_pRegisterOffline->SetPos(GetWidth()-80, 40);
	m_pRegisterOffline->SetClickedListener(this, RegisterOffline);

	m_pPirates->EnsureTextFits();
	m_pPirates->SetDimensions(GetWidth() - m_pPirates->GetWidth() - 5, GetHeight() - m_pPirates->GetHeight() - 5, m_pPirates->GetWidth(), m_pPirates->GetHeight());

	CMovablePanel::Layout();
}

void CRegisterPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CRegisterPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	return false;
}

bool CRegisterPanel::KeyPressed(int iKey)
{
	if (!ModelWindow()->IsRegistered() && (iKey == 'c' || CModelWindow::Get()->IsCtrlDown() && iKey == 'c'))
	{
		SetClipboard(ModelWindow()->GetProductCode());
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CRegisterPanel::WebsiteCallback()
{
	OpenBrowser(_T("http://getsmak.net/"));
	exit(0);
}

void CRegisterPanel::RegisterCallback()
{
	tstring sError;
	bool bSucceeded = ModelWindow()->QueryRegistrationKey(_T("reg.lunarworkshop.com"), _T("/reg/reg.php"), m_pRegistrationKey->GetText(), "smak", sError);
	m_pRegisterResult->SetText(sError.c_str());

	if (bSucceeded)
	{
		m_pRegister->SetVisible(false);
		m_pRegisterOffline->SetVisible(false);
		m_pRegistrationKey->SetVisible(false);
	}

	Layout();
}

void CRegisterPanel::RegisterOfflineCallback()
{
	m_pInfo->SetVisible(false);

	m_pRegistrationKey->SetSize(400, m_pRegister->GetHeight());
	m_pRegistrationKey->SetPos(GetWidth()/2 - m_pRegistrationKey->GetWidth()/2, 140);
	m_pRegister->SetPos(GetWidth()/2 - m_pRegister->GetWidth()/2, 180);
	m_pRegister->SetClickedListener(this, SetKey);
	m_pRegisterResult->SetPos(10, 220);

	m_pRegisterOffline->SetSize(60, 20);
	m_pRegisterOffline->SetText(_T("Copy"));
	m_pRegisterOffline->SetPos(GetWidth()-80, 100);
	m_pRegisterOffline->SetClickedListener(this, CopyProductCode);

	if (!m_pProductCode)
	{
		m_pProductCode = new CLabel(0, 110, GetWidth(), GetHeight(), _T(""));
		AddControl(m_pProductCode);
	}

	m_pProductCode->SetVisible(true);
	m_pProductCode->SetAlign(CLabel::TA_TOPCENTER);
	m_pProductCode->SetText(_T("Product Code: "));
	m_pProductCode->AppendText(ModelWindow()->GetProductCode().c_str());
}

void CRegisterPanel::CopyProductCodeCallback()
{
	SetClipboard(ModelWindow()->GetProductCode());
}

void CRegisterPanel::SetKeyCallback()
{
	eastl::string sKey = convertstring<tchar, char>(m_pRegistrationKey->GetText());
	// eastl::string has some kind of bug that needs working around.
	eastl::string sBugKey = sKey.substr(0, 40);

	ModelWindow()->SetLicenseKey(sBugKey);

	if (ModelWindow()->IsRegistered())
	{
		m_pRegisterResult->SetText(_T("Thank you for registering SMAK!"));
		m_pRegister->SetVisible(false);
		m_pRegisterOffline->SetVisible(false);
		m_pRegistrationKey->SetVisible(false);
		m_pProductCode->SetVisible(false);
	}
	else
		m_pRegisterResult->SetText(_T("Sorry, that key didn't seem to work. Try again!"));
}

void CRegisterPanel::PiratesCallback()
{
	CPiratesPanel::Open();
	Close();
}

void CRegisterPanel::Open()
{
	if (!s_pRegisterPanel)
		s_pRegisterPanel = new CRegisterPanel();

	s_pRegisterPanel->SetVisible(true);
	s_pRegisterPanel->Layout();
}

void CRegisterPanel::Close()
{
	if (!s_pRegisterPanel)
		return;

	s_pRegisterPanel->SetVisible(false);
}

CPiratesPanel* CPiratesPanel::s_pPiratesPanel = NULL;

CPiratesPanel::CPiratesPanel()
	: CMovablePanel(_T("A Message to Software Pirates"))
{
	m_pInfo = new CLabel(0, 0, 100, 100, _T(""));
	AddControl(m_pInfo, true);
	Layout();
}

void CPiratesPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(800, 400);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetSize(GetWidth()-275, GetHeight()-10);
	m_pInfo->SetPos(5, 25);
	m_pInfo->SetAlign(CLabel::TA_TOPLEFT);

	m_pInfo->SetText(_T("Dear pirates,\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("I know who you are. I know where you live. I'm coming for you.\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("Just kidding.\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("Obviously there's nothing I can do to keep a determined person from pirating my software. I know how these things work, I've pirated plenty of things myself. All I can do is ask you sincerely, if you pirate this software and you are able to use it and enjoy it, please pay for it. It's not a very expensive tool. I worked very hard on this thing (Almost as hard as you did to pirate it!) and all I'm trying to do is exchange a hard day's work for a couple of well-earned bones.\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("I'm not some big corporation trying to screw its customers. I'm not an industry assocation who will come after you if you download one thing. I'm not the man trying to hold you down, bro. I'm just a guy who's trying to make a living doing something that he loves. I don't have a BMW or a Rolex. I'm just a dude trying to feed his dog.\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("So please, when you pirate this software, think of the dog.\n"));
	m_pInfo->AppendText(_T(" \n"));
	m_pInfo->AppendText(_T("Jorge Rodriguez <jorge@lunarworkshop.com>\n"));

	CMovablePanel::Layout();
}

void CPiratesPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);

	// ACTIVATE OPERATION GUILT TRIP!!!
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)CModelWindow::Get()->GetBarretTexture());
	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2d(x+w-266, y+h/2-128);
		glTexCoord2f(0, 0);
		glVertex2d(x+w-266, y+h/2+128);
		glTexCoord2f(1, 0);
		glVertex2d(x+w-10, y+h/2+128);
		glTexCoord2f(1, 1);
		glVertex2d(x+w-10, y+h/2-128);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool CPiratesPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CPiratesPanel::Open()
{
	if (!s_pPiratesPanel)
		s_pPiratesPanel = new CPiratesPanel();

	s_pPiratesPanel->SetVisible(true);
	s_pPiratesPanel->Layout();
}

void CPiratesPanel::Close()
{
	if (!s_pPiratesPanel)
		return;

	s_pPiratesPanel->SetVisible(false);
}
