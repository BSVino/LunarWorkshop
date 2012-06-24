#include "smakwindow_ui.h"

#include <maths.h>
#include <strutils.h>
#include <tinker_platform.h>
#include <tinker/keys.h>
#include <glgui/menu.h>
#include <glgui/rootpanel.h>
#include <glgui/picturebutton.h>
#include <glgui/checkbox.h>
#include <glgui/tree.h>
#include <glgui/textfield.h>
#include <glgui/filedialog.h>
#include <textures/materiallibrary.h>

#include "smakwindow.h"
#include "scenetree.h"
#include "../smak_version.h"
#include "picker.h"
#include "smak_renderer.h"

using namespace glgui;

void CSMAKWindow::InitUI()
{
	CMenu* pFile = CRootPanel::Get()->AddMenu("File");
	CMenu* pView = CRootPanel::Get()->AddMenu("View");
	CMenu* pTools = CRootPanel::Get()->AddMenu("Tools");
	CMenu* pHelp = CRootPanel::Get()->AddMenu("Help");

	pFile->AddSubmenu("Open...", this, OpenDialog);
	pFile->AddSubmenu("Open Into...", this, OpenIntoDialog);
	pFile->AddSubmenu("Reload", this, Reload);
	pFile->AddSubmenu("Save As...", this, SaveDialog);
	pFile->AddSubmenu("Close", this, Close);
	pFile->AddSubmenu("Exit", this, Exit);

	pView->AddSubmenu("3D view", this, Render3D);
	pView->AddSubmenu("UV view", this, RenderUV);
	pView->AddSubmenu("View wireframe", this, Wireframe);
	pView->AddSubmenu("Toggle light", this, LightToggle);
	pView->AddSubmenu("Toggle texture", this, TextureToggle);
	pView->AddSubmenu("Toggle normal map", this, NormalToggle);
	pView->AddSubmenu("Toggle AO map", this, AOToggle);
	pView->AddSubmenu("Toggle color AO map", this, ColorAOToggle);

	pTools->AddSubmenu("Generate all maps", this, GenerateCombo);
	pTools->AddSubmenu("Generate AO map", this, GenerateAO);
	pTools->AddSubmenu("Generate color AO map", this, GenerateColorAO);
	pTools->AddSubmenu("Generate normal from texture", this, GenerateNormal);

	pHelp->AddSubmenu("Help", this, Help);
	pHelp->AddSubmenu("About SMAK", this, About);

	CResource<CBaseControl> pTopButtons = CreateControl(new CButtonPanel(BA_TOP));
	CControl<CButtonPanel> hTopButtons = pTopButtons;

	m_hRender3D = hTopButtons->AddButton(new CPictureButton("3D", GetSMAKRenderer()->GetSmoothTexture(), true), "Render 3D View", false, this, Render3D);
	m_hRenderUV = hTopButtons->AddButton(new CPictureButton("UV", GetSMAKRenderer()->GetUVTexture(), true), "Render UV View", false, this, RenderUV);

	CRootPanel::Get()->AddControl(pTopButtons);

	CResource<CBaseControl> pBottomButtons = CreateControl(new CButtonPanel(BA_BOTTOM));
	CControl<CButtonPanel> hBottomButtons = pBottomButtons;

	m_hWireframe = hBottomButtons->AddButton(new CPictureButton("Wire", GetSMAKRenderer()->GetWireframeTexture(), true), "Toggle Wireframe", true, this, Wireframe);
	m_hUVWireframe = hBottomButtons->AddButton(new CPictureButton("Wire", GetSMAKRenderer()->GetUVTexture(), true), "Toggle UVs", true, this, UVWireframe);
	m_hLight = hBottomButtons->AddButton(new CPictureButton("Lght", GetSMAKRenderer()->GetLightTexture(), true), "Toggle Light", false, this, Light);
	m_hTexture = hBottomButtons->AddButton(new CPictureButton("Tex", GetSMAKRenderer()->GetTextureTexture(), true), "Toggle Texture", false, this, Texture);
	m_hNormal = hBottomButtons->AddButton(new CPictureButton("Nrml", GetSMAKRenderer()->GetNormalTexture(), true), "Toggle Normal Map", false, this, Normal);
	m_hAO = hBottomButtons->AddButton(new CPictureButton("AO", GetSMAKRenderer()->GetAOTexture(), true), "Toggle AO Map", false, this, AO);
	m_hColorAO = hBottomButtons->AddButton(new CPictureButton("C AO", GetSMAKRenderer()->GetColorAOTexture(), true), "Toggle Color AO", false, this, ColorAO);

	CRootPanel::Get()->AddControl(pBottomButtons);

	CSceneTreePanel::Open(&m_Scene);

	CRootPanel::Get()->Layout();
}

void CSMAKWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

void CSMAKWindow::OpenDialogCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	CFileDialog::ShowOpenDialog("", ".obj;.sia;.dae", this, OpenFile);
}

void CSMAKWindow::OpenFileCallback(const tstring& sArgs)
{
	ReadFile(sArgs.c_str());
}

void CSMAKWindow::OpenIntoDialogCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	CFileDialog::ShowOpenDialog("", ".obj;.sia;.dae", this, OpenIntoFile);
}

void CSMAKWindow::OpenIntoFileCallback(const tstring& sArgs)
{
	ReadFileIntoScene(sArgs.c_str());
}

void CSMAKWindow::ReloadCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	ReloadFromFile();
}

void CSMAKWindow::SaveDialogCallback(const tstring& sArgs)
{
	CFileDialog::ShowSaveDialog("", ".obj;.sia;.dae", this, SaveFile);
}

void CSMAKWindow::SaveFileCallback(const tstring& sArgs)
{
	SaveFile(sArgs.c_str());
}

void CSMAKWindow::CloseCallback(const tstring& sArgs)
{
	if (m_bLoadingFile)
		return;

	DestroyAll();
}

void CSMAKWindow::ExitCallback(const tstring& sArgs)
{
	exit(0);
}

void CSMAKWindow::Render3DCallback(const tstring& sArgs)
{
	SetRenderMode(false);
}

void CSMAKWindow::RenderUVCallback(const tstring& sArgs)
{
	SetRenderMode(true);
}

void CSMAKWindow::SceneTreeCallback(const tstring& sArgs)
{
	CSceneTreePanel::Open(&m_Scene);
}

void CSMAKWindow::WireframeCallback(const tstring& sArgs)
{
	SetDisplayWireframe(m_hWireframe->GetState());
}

void CSMAKWindow::UVWireframeCallback(const tstring& sArgs)
{
	m_bDisplayUV = m_hUVWireframe->GetState();
}

void CSMAKWindow::LightCallback(const tstring& sArgs)
{
	SetDisplayLight(m_hLight->GetState());
}

void CSMAKWindow::TextureCallback(const tstring& sArgs)
{
	SetDisplayTexture(m_hTexture->GetState());
}

void CSMAKWindow::NormalCallback(const tstring& sArgs)
{
	SetDisplayNormal(m_hNormal->GetState());
}

void CSMAKWindow::AOCallback(const tstring& sArgs)
{
	SetDisplayAO(m_hAO->GetState());
}

void CSMAKWindow::ColorAOCallback(const tstring& sArgs)
{
#if 0
	if (CAOPanel::Get(true) && CAOPanel::Get(true)->IsGenerating() && !CAOPanel::Get(true)->DoneGenerating())
	{
		m_hColorAO->SetState(true, false);
		return;
	}

	if (!CAOPanel::Get(true) || !CAOPanel::Get(true)->DoneGenerating())
	{
		CAOPanel::Open(true, &m_Scene);
		m_hColorAO->SetState(false, false);
		return;
	}

	SetDisplayColorAO(m_hColorAO->GetState());
#endif
}

void CSMAKWindow::LightToggleCallback(const tstring& sArgs)
{
	SetDisplayLight(!m_bDisplayLight);
}

void CSMAKWindow::TextureToggleCallback(const tstring& sArgs)
{
	SetDisplayTexture(!m_bDisplayTexture);
}

void CSMAKWindow::NormalToggleCallback(const tstring& sArgs)
{
	SetDisplayNormal(!m_bDisplayNormal);
}

void CSMAKWindow::AOToggleCallback(const tstring& sArgs)
{
	SetDisplayAO(!m_bDisplayAO);
}

void CSMAKWindow::ColorAOToggleCallback(const tstring& sArgs)
{
	SetDisplayColorAO(!m_bDisplayColorAO);
}

void CSMAKWindow::GenerateComboCallback(const tstring& sArgs)
{
	CComboGeneratorPanel::Open(&m_Scene);
}

void CSMAKWindow::GenerateAOCallback(const tstring& sArgs)
{
	CAOPanel::Open(&m_Scene);
}

void CSMAKWindow::GenerateColorAOCallback(const tstring& sArgs)
{
#if 0
	CAOPanel::Open(true, &m_Scene);
#endif
}

void CSMAKWindow::GenerateNormalCallback(const tstring& sArgs)
{
	CNormalPanel::Open(&m_Scene);
}

void CSMAKWindow::HelpCallback(const tstring& sArgs)
{
	OpenHelpPanel();
}

void CSMAKWindow::AboutCallback(const tstring& sArgs)
{
	OpenAboutPanel();
}

void CSMAKWindow::OpenHelpPanel()
{
	CHelpPanel::Open();
}

void CSMAKWindow::OpenAboutPanel()
{
	CAboutPanel::Open();
}

void CSMAKWindow::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CSMAKWindow::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(sAction);
	WorkProgress(0, true);
}

void CSMAKWindow::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static double flLastTime = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && GetTime() - flLastTime < 0.3f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	CSMAKWindow::Get()->Render();

	SwapBuffers();

	flLastTime = GetTime();
}

void CSMAKWindow::EndProgress()
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
	float flX = 0;

	for (size_t i = 0; i < m_ahButtons.size(); i++)
	{
		CBaseControl* pButton = m_ahButtons[i];

		if (!pButton->IsVisible())
			continue;

		pButton->SetSize(BTN_HEIGHT, BTN_HEIGHT);
		pButton->SetPos(flX, 0);

		CBaseControl* pHint = m_ahHints[i];
		pHint->SetPos(flX + BTN_HEIGHT/2 - pHint->GetWidth()/2, -18);

		flX += BTN_HEIGHT + m_aflSpaces[i];
	}

	SetSize(flX - m_aflSpaces[m_aflSpaces.size()-1], BTN_HEIGHT);
	if (m_eAlign == BA_TOP)
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, BTN_HEIGHT + BTN_SPACE);
	else
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, CRootPanel::Get()->GetHeight() - BTN_HEIGHT*2 - BTN_SPACE);

	CPanel::Layout();
}

glgui::CControl<glgui::CButton> CButtonPanel::AddButton(CButton* pButton, const tstring& sHints, bool bNewSection, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	CControl<glgui::CButton> hButton = AddControl(pButton);
	m_ahButtons.push_back(hButton);

	m_aflSpaces.push_back((float)(bNewSection?BTN_SECTION:BTN_SPACE));

	CControl<CLabel> hHint = AddControl(new CLabel(0, 0, 0, 0, sHints));
	hHint->SetAlpha(0);
	hHint->EnsureTextFits();
	m_ahHints.push_back(hHint);

	if (pListener)
	{
		hButton->SetClickedListener(pListener, pfnCallback);
		hButton->SetUnclickedListener(pListener, pfnCallback);
	}

	return hButton;
}

void CButtonPanel::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	for (size_t i = 0; i < m_ahButtons.size(); i++)
	{
		if (!m_ahButtons[i]->IsVisible())
		{
			m_ahHints[i]->SetAlpha((int)Approach(0.0f, (float)m_ahHints[i]->GetAlpha(), 30.0f));
			continue;
		}

		float x = 0, y = 0, w = 0, h = 0;
		m_ahButtons[i]->GetAbsDimensions(x, y, w, h);

		float flAlpha = (float)m_ahHints[i]->GetAlpha();

		if (mx >= x && my >= y && mx < x + w && my < y + h)
			m_ahHints[i]->SetAlpha((int)Approach(255.0f, flAlpha, 30.0f));
		else
			m_ahHints[i]->SetAlpha((int)Approach(0.0f, flAlpha, 30.0f));
	}

	CPanel::Think();
}

void CButtonPanel::Paint(float x, float y, float w, float h)
{
	CPanel::Paint(x, y, w, h);
}

CControl<CProgressBar> CProgressBar::s_hProgressBar;

CProgressBar::CProgressBar()
	: CPanel(0, 0, 100, 100)
{
	SetVisible(false);
}

void CProgressBar::CreateControls(CResource<CBaseControl> pThis)
{
	m_hAction = AddControl(new CLabel(0, 0, 200, BTN_HEIGHT, ""));
	m_hAction->SetWrap(false);

	BaseClass::CreateControls(pThis);

	Layout();
}

void CProgressBar::Layout()
{
	if (!GetParent())
		return;

	SetSize(GetParent()->GetWidth()/2, BTN_HEIGHT);
	SetPos(GetParent()->GetWidth()/4, GetParent()->GetHeight()/5);

	m_hAction->SetSize(GetWidth(), BTN_HEIGHT);
}

void CProgressBar::Paint(float x, float y, float w, float h)
{
	float flTotalProgress = (float)m_iCurrentProgress/(float)m_iTotalProgress;

	if (flTotalProgress > 1)
		flTotalProgress = 1;

	CPanel::PaintRect(x, y, w, h);
	CPanel::PaintRect(x+10, y+10, (w-20)*flTotalProgress, h-20, g_clrBoxHi);

	m_hAction->Paint();
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

	m_hAction->SetText(m_sAction);

	if (m_iTotalProgress)
	{
		tstring sProgress;
		sProgress = sprintf(" %d%%", m_iCurrentProgress*100/m_iTotalProgress);
		m_hAction->AppendText(sProgress);
	}
}

CProgressBar* CProgressBar::Get()
{
	if (!s_hProgressBar)
		s_hProgressBar = RootPanel()->AddControl(new CProgressBar());

	return s_hProgressBar;
}

CResource<CBaseControl> CAOPanel::s_pAOPanel;
CResource<CBaseControl> CAOPanel::s_pColorAOPanel;

CAOPanel::CAOPanel(bool bColor, CConversionScene* pScene)
	: CMovablePanel(bColor?"Color AO generator":"AO generator"), m_oGenerator(pScene)
{
	m_bColor = bColor;

	m_pScene = pScene;
}

void CAOPanel::CreateControls(CResource<CBaseControl> pThis)
{
	m_hSizeLabel = AddControl(new CLabel(0, 0, 32, 32, "Size"));

	m_hSizeSelector = AddControl(new CScrollSelector<int>());
#ifdef _DEBUG
	m_hSizeSelector->AddSelection(CScrollSelection<int>(16, "16x16"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(32, "32x32"));
#endif
	m_hSizeSelector->AddSelection(CScrollSelection<int>(64, "64x64"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(128, "128x128"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(256, "256x256"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(512, "512x512"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(1024, "1024x1024"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(2048, "2048x2048"));
	//m_hSizeSelector->AddSelection(CScrollSelection<int>(4096, "4096x4096"));
	m_hSizeSelector->SetSelection(2);

	m_hEdgeBleedLabel = AddControl(new CLabel(0, 0, 32, 32, "Edge Bleed"));

	m_hEdgeBleedSelector = AddControl(new CScrollSelector<int>());
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(0, "0"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(1, "1"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(2, "2"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(3, "3"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(4, "4"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(5, "5"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(6, "6"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(7, "7"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(8, "8"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(9, "9"));
	m_hEdgeBleedSelector->AddSelection(CScrollSelection<int>(10, "10"));
	m_hEdgeBleedSelector->SetSelection(1);

	if (!m_bColor)
	{
		m_hAOMethodLabel = AddControl(new CLabel(0, 0, 32, 32, "Method"));

		m_hAOMethodSelector = AddControl(new CScrollSelector<int>());
		m_hAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_SHADOWMAP, "Shadow map (fast!)"));
		m_hAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_RAYTRACE, "Raytraced (slow!)"));
		m_hAOMethodSelector->SetSelectedListener(this, AOMethod);

		m_hRayDensityLabel = AddControl(new CLabel(0, 0, 32, 32, "Ray Density"));

		m_hRayDensitySelector = AddControl(new CScrollSelector<int>());
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(5, "5"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(6, "6"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(7, "7"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(8, "8"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(9, "9"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(10, "10"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(11, "11"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(12, "12"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(13, "13"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(14, "14"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(15, "15"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(16, "16"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(17, "17"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(18, "18"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(19, "19"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(20, "20"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(21, "21"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(22, "22"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(23, "23"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(24, "24"));
		m_hRayDensitySelector->AddSelection(CScrollSelection<int>(25, "25"));
		m_hRayDensitySelector->SetSelection(15);

		m_hFalloffLabel = AddControl(new CLabel(0, 0, 32, 32, "Ray Falloff"));

		m_hFalloffSelector = AddControl(new CScrollSelector<float>());
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.01f, "0.01"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.05f, "0.05"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.1f, "0.1"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.25f, "0.25"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.5f, "0.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.75f, "0.75"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.0f, "1.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.25f, "1.25"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.5f, "1.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.75f, "1.75"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(2.0f, "2.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(2.5f, "2.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(3.0f, "3.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(4.0f, "4.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(5.0f, "5.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(6.0f, "6.0"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(7.5f, "7.5"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(10.0f, "10"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(25.0f, "25"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(50.0f, "50"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(100.0f, "100"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(250.0f, "250"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(500.0f, "500"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(1000.0f, "1000"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(2500.0f, "2500"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(5000.0f, "5000"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(10000.0f, "10000"));
		m_hFalloffSelector->AddSelection(CScrollSelection<float>(-1.0f, "No falloff"));
		m_hFalloffSelector->SetSelection(6);

		m_hLightsLabel = AddControl(new CLabel(0, 0, 32, 32, "Lights"));

		m_hLightsSelector = AddControl(new CScrollSelector<int>());
		m_hLightsSelector->AddSelection(CScrollSelection<int>(500, "500"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(1000, "1000"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(1500, "1500"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(2000, "2000"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(2500, "2500"));
		m_hLightsSelector->AddSelection(CScrollSelection<int>(3000, "3000"));
		m_hLightsSelector->SetSelection(3);

		m_hRandomLabel = AddControl(new CLabel(0, 0, 32, 32, "Randomize rays"));

		m_hRandomCheckBox = AddControl(new CCheckBox());

		m_hCreaseLabel = AddControl(new CLabel(0, 0, 32, 32, "Crease edges"));

		m_hCreaseCheckBox = AddControl(new CCheckBox());

		m_hGroundOcclusionLabel = AddControl(new CLabel(0, 0, 32, 32, "Ground occlusion"));

		m_hGroundOcclusionCheckBox = AddControl(new CCheckBox());
	}

	m_hGenerate = AddControl(new CButton(0, 0, 100, 100, "Generate"));

	m_hGenerate->SetClickedListener(this, Generate);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	BaseClass::CreateControls(pThis);

	Layout();
}

void CAOPanel::Layout()
{
	if (m_bColor)
		SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 150);
	else
		SetPos(GetParent()->GetWidth() - GetWidth() - 200, GetParent()->GetHeight() - GetHeight() - 100);

	float flSpace = 20;

	m_hSizeLabel->EnsureTextFits();

	float flSelectorSize = m_hSizeLabel->GetHeight() - 4;

	m_hSizeSelector->SetSize(GetWidth() - m_hSizeLabel->GetWidth() - flSpace, flSelectorSize);

	float flControlY = HEADER_HEIGHT;

	m_hSizeSelector->SetPos(GetWidth() - m_hSizeSelector->GetWidth() - flSpace/2, flControlY);
	m_hSizeLabel->SetPos(5, flControlY);

	flControlY += 30;

	m_hEdgeBleedLabel->EnsureTextFits();
	m_hEdgeBleedLabel->SetPos(5, flControlY);

	m_hEdgeBleedSelector->SetSize(GetWidth() - m_hEdgeBleedLabel->GetWidth() - flSpace, flSelectorSize);
	m_hEdgeBleedSelector->SetPos(GetWidth() - m_hEdgeBleedSelector->GetWidth() - flSpace/2, flControlY);

	if (!m_bColor)
	{
		flControlY += 30;

		m_hAOMethodLabel->EnsureTextFits();
		m_hAOMethodLabel->SetPos(5, flControlY);

		m_hAOMethodSelector->SetSize(GetWidth() - m_hAOMethodLabel->GetWidth() - flSpace, flSelectorSize);
		m_hAOMethodSelector->SetPos(GetWidth() - m_hAOMethodSelector->GetWidth() - flSpace/2, flControlY);

		bool bRaytracing = (m_hAOMethodSelector->GetSelectionValue() == AOMETHOD_RAYTRACE);
		m_hRayDensityLabel->SetVisible(bRaytracing);
		m_hRayDensitySelector->SetVisible(bRaytracing);

		m_hFalloffSelector->SetVisible(bRaytracing);
		m_hFalloffLabel->SetVisible(bRaytracing);

		m_hRandomCheckBox->SetVisible(bRaytracing);
		m_hRandomLabel->SetVisible(bRaytracing);

		if (bRaytracing)
		{
			flControlY += 30;

			m_hRayDensityLabel->EnsureTextFits();
			m_hRayDensityLabel->SetPos(5, flControlY);

			m_hRayDensitySelector->SetSize(GetWidth() - m_hRayDensityLabel->GetWidth() - flSpace, flSelectorSize);
			m_hRayDensitySelector->SetPos(GetWidth() - m_hRayDensitySelector->GetWidth() - flSpace/2, flControlY);

			flControlY += 30;

			m_hFalloffLabel->EnsureTextFits();
			m_hFalloffLabel->SetPos(5, flControlY);

			m_hFalloffSelector->SetSize(GetWidth() - m_hFalloffLabel->GetWidth() - flSpace, flSelectorSize);
			m_hFalloffSelector->SetPos(GetWidth() - m_hFalloffSelector->GetWidth() - flSpace/2, flControlY);

			flControlY += 30;

			m_hRandomLabel->EnsureTextFits();
			m_hRandomLabel->SetPos(25, flControlY);

			m_hRandomCheckBox->SetPos(10, flControlY + m_hRandomLabel->GetHeight()/2 - m_hRandomCheckBox->GetHeight()/2);
		}

		bool bShadowmapping = (m_hAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP);
		m_hLightsLabel->SetVisible(bShadowmapping);
		m_hLightsSelector->SetVisible(bShadowmapping);

		if (bShadowmapping)
		{
			flControlY += 30;

			m_hLightsLabel->EnsureTextFits();
			m_hLightsLabel->SetPos(5, flControlY);

			m_hLightsSelector->SetSize(GetWidth() - m_hLightsLabel->GetWidth() - flSpace, flSelectorSize);
			m_hLightsSelector->SetPos(GetWidth() - m_hLightsSelector->GetWidth() - flSpace/2, flControlY);
		}

		m_hGroundOcclusionLabel->SetVisible(bShadowmapping || bRaytracing);
		m_hGroundOcclusionCheckBox->SetVisible(bShadowmapping || bRaytracing);

		if (bShadowmapping || bRaytracing)
		{
			// If we're on the raytracing screen there was already the Randomize rays checkbox
			// so keep the spacing smaller.
			if (bRaytracing)
				flControlY += 20;
			else
				flControlY += 30;

			m_hGroundOcclusionLabel->EnsureTextFits();
			m_hGroundOcclusionLabel->SetPos(25, flControlY);

			m_hGroundOcclusionCheckBox->SetPos(10, flControlY + m_hGroundOcclusionLabel->GetHeight()/2 - m_hGroundOcclusionCheckBox->GetHeight()/2);
		}

		if (bShadowmapping || bRaytracing)
			flControlY += 20;
		else
			flControlY += 30;

		m_hCreaseLabel->EnsureTextFits();
		m_hCreaseLabel->SetPos(25, flControlY);

		m_hCreaseCheckBox->SetPos(10, flControlY + m_hCreaseLabel->GetHeight()/2 - m_hCreaseCheckBox->GetHeight()/2);
	}

	m_hSave->SetSize(GetWidth()/2, GetWidth()/6);
	m_hSave->SetPos(GetWidth()/4, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetSize(GetWidth()/2, GetWidth()/6);
	m_hGenerate->SetPos(GetWidth()/4, GetHeight() - (int)(m_hSave->GetHeight()*1.5f) - (int)(m_hGenerate->GetHeight()*1.5f));

	CMovablePanel::Layout();
}

bool CAOPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CAOPanel::GenerateCallback(const tstring& sArgs)
{
	if (m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_hSave->SetVisible(false);

	// Switch over to UV mode so we can see our progress.
	CSMAKWindow::Get()->SetRenderMode(true);

	// If the 3d model was there get rid of it.
	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	if (m_bColor)
		CSMAKWindow::Get()->SetDisplayColorAO(true);
	else
		CSMAKWindow::Get()->SetDisplayAO(true);

	m_hGenerate->SetText("Cancel");

	int iSize = m_hSizeSelector->GetSelectionValue();
	m_oGenerator.SetMethod(m_bColor?AOMETHOD_RENDER:(aomethod_t)m_hAOMethodSelector->GetSelectionValue());
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.SetBleed(m_hEdgeBleedSelector->GetSelectionValue());
	m_oGenerator.SetUseTexture(true);
	m_oGenerator.SetWorkListener(this);
	if (!m_bColor)
	{
		if (m_hAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP)
			m_oGenerator.SetSamples(m_hLightsSelector->GetSelectionValue());
		else
			m_oGenerator.SetSamples(m_hRayDensitySelector->GetSelectionValue());
		m_oGenerator.SetRandomize(m_hRandomCheckBox->GetToggleState());
		m_oGenerator.SetCreaseEdges(m_hCreaseCheckBox->GetToggleState());
		m_oGenerator.SetGroundOcclusion(m_hGroundOcclusionCheckBox->GetToggleState());
		m_oGenerator.SetRayFalloff(m_hFalloffSelector->GetSelectionValue());
	}
	m_oGenerator.Generate();

	CTextureHandle hAO;
	if (m_oGenerator.DoneGenerating())
		hAO = m_oGenerator.GenerateTexture();

	for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
	{
		CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
		if (!hMaterial.IsValid())
			continue;

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		hMaterial->SetParameter(m_bColor?"ColorAmbientOcclusion":"AmbientOcclusion", hAO);
	}

	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetText("Generate");
}

void CAOPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CAOPanel::SaveMapFileCallback(const tstring& sArgs)
{
	m_oGenerator.SaveToFile(sArgs.c_str());
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
	static double flLastTime = 0;
	static double flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CSMAKWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	if (m_oGenerator.IsGenerating() && flLastTime - flLastGenerate > 0.5f)
	{
		CTextureHandle hAO = m_oGenerator.GenerateTexture(true);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
			if (!hMaterial.IsValid())
				continue;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			hMaterial->SetParameter(m_bColor?"ColorAmbientOcclusion":"AmbientOcclusion", hAO);
		}

		flLastGenerate = CSMAKWindow::Get()->GetTime();
	}

	CSMAKWindow::Get()->Render();
	Application()->SwapBuffers();

	flLastTime = CSMAKWindow::Get()->GetTime();
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
		m_hFalloffSelector->SetSelection(m_hFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
}

void CAOPanel::Open(CConversionScene* pScene)
{
	CControl<CAOPanel> hPanel = Get();
	CResource<CBaseControl> pPanel = hPanel.GetHandle().lock();

	// Get rid of the last one, in case we've changed the scene.
	if (pPanel.get())
		hPanel->Close();

#if 0
	if (bColor)
		s_pColorAOPanel = CControl<CAOPanel>(CreateControl(new CAOPanel(true, pScene))->GetHandle());
	else
#endif
		s_pAOPanel = CreateControl(new CAOPanel(false, pScene));

	hPanel = Get();

	if (!hPanel)
		return;

	hPanel->SetVisible(true);
	hPanel->Layout();

	hPanel->FindBestRayFalloff();
}

CControl<CAOPanel> CAOPanel::Get()
{
	glgui::CControl<CAOPanel> hControl = s_pAOPanel;
	if (hControl)
		return hControl;

	return CControl<CAOPanel>();
}

void CAOPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

void CAOPanel::AOMethodCallback(const tstring& sArgs)
{
	// So we can appear/disappear the ray density bar if the AO method has changed.
	Layout();
}

COptionsButton::COptionsButton()
	: CButton(0, 0, 100, 100, "", true)
{
	m_hPanel = m_hPanelResource = CreateControl(new COptionsPanel(this));
	m_hPanel->SetVisible(false);

	SetClickedListener(this, Open);
	SetUnclickedListener(this, Close);
}

void COptionsButton::OpenCallback(const tstring& sArgs)
{
	RootPanel()->AddControl(m_hPanelResource, true);

	float flPanelWidth = m_hPanel->GetWidth();
	float flButtonWidth = GetWidth();
	float x, y, w, h;
	GetAbsDimensions(x, y, w, h);
	m_hPanel->SetPos(x + flButtonWidth/2 - flPanelWidth/2, y+h+3);

	m_hPanel->Layout();
	m_hPanel->SetVisible(true);
}

void COptionsButton::CloseCallback(const tstring& sArgs)
{
	RootPanel()->RemoveControl(m_hPanel);
	m_hPanel->SetVisible(false);

	SetState(false, false);
}

COptionsButton::COptionsPanel::COptionsPanel(COptionsButton* pButton)
	: CPanel(0, 0, 200, 350)
{
	m_pButton = pButton;
}

void COptionsButton::COptionsPanel::CreateControls(CResource<CBaseControl> pThis)
{
	m_hOkay = AddControl(new CButton(0, 0, 100, 100, "Okay"));
	m_hOkay->SetClickedListener(m_pButton, Close);

	BaseClass::CreateControls(pThis);
}

void COptionsButton::COptionsPanel::Layout()
{
	BaseClass::Layout();

	m_hOkay->SetSize(40, 20);
	m_hOkay->SetPos(GetWidth()/2 - 20, GetHeight() - 30);
}

void COptionsButton::COptionsPanel::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, h, g_clrBox);

	BaseClass::Paint(x, y, w, h);
}

CResource<CBaseControl> CComboGeneratorPanel::s_pComboGeneratorPanel;

CComboGeneratorPanel::CComboGeneratorPanel(CConversionScene* pScene)
	: CMovablePanel("Combo map generator"), m_oGenerator(pScene)
{
	m_pScene = pScene;
}

void CComboGeneratorPanel::CreateControls(CResource<CBaseControl> pThis)
{
	m_hSizeLabel = AddControl(new CLabel(0, 0, 32, 32, "Size"));

	m_hSizeSelector = AddControl(new CScrollSelector<int>());
#ifdef _DEBUG
	m_hSizeSelector->AddSelection(CScrollSelection<int>(16, "16x16"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(32, "32x32"));
#endif
	m_hSizeSelector->AddSelection(CScrollSelection<int>(64, "64x64"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(128, "128x128"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(256, "256x256"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(512, "512x512"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(1024, "1024x1024"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(2048, "2048x2048"));
	m_hSizeSelector->AddSelection(CScrollSelection<int>(4096, "4096x4096"));
	m_hSizeSelector->SetSelection(4);

	m_hLoResLabel = AddControl(new CLabel(0, 0, 32, 32, "Low Resolution Meshes"));

	m_hLoRes = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hLoRes->SetBackgroundColor(g_clrBox);
	m_hLoRes->SetDroppedListener(this, DroppedLoResMesh);

	m_hHiResLabel = AddControl(new CLabel(0, 0, 32, 32, "High Resolution Meshes"));

	m_hHiRes = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hHiRes->SetBackgroundColor(g_clrBox);
	m_hHiRes->SetDroppedListener(this, DroppedHiResMesh);

	m_hAddLoRes = AddControl(new CButton(0, 0, 100, 100, "Add"));
	m_hAddLoRes->SetClickedListener(this, AddLoRes);

	m_hAddHiRes = AddControl(new CButton(0, 0, 100, 100, "Add"));
	m_hAddHiRes->SetClickedListener(this, AddHiRes);

	m_hRemoveLoRes = AddControl(new CButton(0, 0, 100, 100, "Remove"));
	m_hRemoveLoRes->SetClickedListener(this, RemoveLoRes);

	m_hRemoveHiRes = AddControl(new CButton(0, 0, 100, 100, "Remove"));
	m_hRemoveHiRes->SetClickedListener(this, RemoveHiRes);

	m_hDiffuseCheckBox = AddControl(new CCheckBox());
	m_hDiffuseCheckBox->SetState(true, false);

	m_hDiffuseLabel = AddControl(new CLabel(0, 0, 100, 100, "Diffuse"));

	m_hAOCheckBox = AddControl(new CCheckBox());
	m_hAOCheckBox->SetState(true, false);

	m_hAOLabel = AddControl(new CLabel(0, 0, 100, 100, "Ambient Occlusion"));

	m_hAOOptions = AddControl(new COptionsButton());

	m_hBleedLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Edge Bleed"));

	m_hBleedSelector = m_hAOOptions->GetOptionsPanel()->AddControl(new CScrollSelector<int>());
	m_hBleedSelector->AddSelection(CScrollSelection<int>(0, "0"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(1, "1"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(2, "2"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(3, "3"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(4, "4"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(5, "5"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(6, "6"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(7, "7"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(8, "8"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(9, "9"));
	m_hBleedSelector->AddSelection(CScrollSelection<int>(10, "10"));
	m_hBleedSelector->SetSelection(1);

	m_hSamplesLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Samples"));

	m_hSamplesSelector = m_hAOOptions->GetOptionsPanel()->AddControl(new CScrollSelector<int>());
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(5, "5"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(6, "6"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(7, "7"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(8, "8"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(9, "9"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(10, "10"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(11, "11"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(12, "12"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(13, "13"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(14, "14"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(15, "15"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(16, "16"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(17, "17"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(18, "18"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(19, "19"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(20, "20"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(21, "21"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(22, "22"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(23, "23"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(24, "24"));
	m_hSamplesSelector->AddSelection(CScrollSelection<int>(25, "25"));
	m_hSamplesSelector->SetSelection(15);

	m_hFalloffLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Ray Falloff"));

	m_hFalloffSelector = m_hAOOptions->GetOptionsPanel()->AddControl(new CScrollSelector<float>());
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.01f, "0.01"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.05f, "0.05"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.1f, "0.1"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.25f, "0.25"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.5f, "0.5"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(0.75f, "0.75"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.0f, "1.0"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.25f, "1.25"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.5f, "1.5"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(1.75f, "1.75"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(2.0f, "2.0"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(2.5f, "2.5"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(3.0f, "3.0"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(4.0f, "4.0"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(5.0f, "5.0"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(6.0f, "6.0"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(7.5f, "7.5"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(10.0f, "10"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(25.0f, "25"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(50.0f, "50"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(100.0f, "100"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(250.0f, "250"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(500.0f, "500"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(1000.0f, "1000"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(2500.0f, "2500"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(5000.0f, "5000"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(10000.0f, "10000"));
	m_hFalloffSelector->AddSelection(CScrollSelection<float>(-1.0f, "No falloff"));

	m_hRandomCheckBox = m_hAOOptions->GetOptionsPanel()->AddControl(new CCheckBox());

	m_hRandomLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Randomize rays"));

	m_hGroundOcclusionCheckBox = m_hAOOptions->GetOptionsPanel()->AddControl(new CCheckBox());

	m_hGroundOcclusionLabel = m_hAOOptions->GetOptionsPanel()->AddControl(new CLabel(0, 0, 100, 100, "Ground occlusion"));

	m_hNormalCheckBox = AddControl(new CCheckBox());
	m_hNormalCheckBox->SetState(true, false);

	m_hNormalLabel = AddControl(new CLabel(0, 0, 100, 100, "Normal Map"));

	m_hGenerate = AddControl(new CButton(0, 0, 100, 100, "Generate"));
	m_hGenerate->SetClickedListener(this, Generate);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	BaseClass::CreateControls(pThis);

	Layout();
}

void CComboGeneratorPanel::Layout()
{
	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	float flSpace = 20;

	m_hSizeLabel->EnsureTextFits();

	float flSelectorSize = m_hSizeLabel->GetHeight() - 4;

	m_hSizeSelector->SetSize(GetWidth() - m_hSizeLabel->GetWidth() - flSpace, flSelectorSize);

	float flControlY = HEADER_HEIGHT;

	m_hSizeSelector->SetPos(GetWidth() - m_hSizeSelector->GetWidth() - flSpace/2, flControlY);
	m_hSizeLabel->SetPos(5, flControlY);

	float flTreeWidth = GetWidth()/2-15;

	m_hLoResLabel->EnsureTextFits();
	m_hLoResLabel->SetPos(10, 40);

	m_hLoRes->SetSize(flTreeWidth, 150);
	m_hLoRes->SetPos(10, 70);

	m_hAddLoRes->SetSize(40, 20);
	m_hAddLoRes->SetPos(10, 225);

	m_hRemoveLoRes->SetSize(60, 20);
	m_hRemoveLoRes->SetPos(60, 225);

	m_hHiResLabel->EnsureTextFits();
	m_hHiResLabel->SetPos(flTreeWidth+20, 40);

	m_hHiRes->SetSize(flTreeWidth, 150);
	m_hHiRes->SetPos(flTreeWidth+20, 70);

	m_hAddHiRes->SetSize(40, 20);
	m_hAddHiRes->SetPos(flTreeWidth+20, 225);

	m_hRemoveHiRes->SetSize(60, 20);
	m_hRemoveHiRes->SetPos(flTreeWidth+70, 225);

	m_hDiffuseLabel->SetPos(35, 220);
	m_hDiffuseLabel->EnsureTextFits();
	m_hDiffuseLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hDiffuseLabel->SetWrap(false);
	m_hDiffuseCheckBox->SetPos(20, 220 + m_hDiffuseLabel->GetHeight()/2 - m_hDiffuseCheckBox->GetHeight()/2);

	m_hAOLabel->SetPos(35, 250);
	m_hAOLabel->EnsureTextFits();
	m_hAOLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hAOLabel->SetWrap(false);
	m_hAOCheckBox->SetPos(20, 250 + m_hAOLabel->GetHeight()/2 - m_hAOCheckBox->GetHeight()/2);

	m_hAOOptions->SetPos(250, 250 + m_hAOLabel->GetHeight()/2 - m_hAOOptions->GetHeight()/2);
	m_hAOOptions->SetSize(60, 20);
	m_hAOOptions->SetText("Options...");

	float flControl = 3;

	m_hAOOptions->GetOptionsPanel()->SetSize(200, 200);

	flControlY = 10;

	m_hBleedLabel->SetSize(10, 10);
	m_hBleedLabel->EnsureTextFits();
	m_hBleedLabel->SetPos(5, flControlY);

	m_hBleedSelector->SetSize(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hBleedLabel->GetWidth() - flSpace, flSelectorSize);
	m_hBleedSelector->SetPos(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hBleedSelector->GetWidth() - flSpace/2, flControlY);

	flControlY += 30;

	m_hSamplesLabel->SetSize(10, 10);
	m_hSamplesLabel->EnsureTextFits();
	m_hSamplesLabel->SetPos(5, flControlY);

	m_hSamplesSelector->SetSize(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hSamplesLabel->GetWidth() - flSpace, flSelectorSize);
	m_hSamplesSelector->SetPos(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hSamplesSelector->GetWidth() - flSpace/2, flControlY);

	flControlY += 30;

	m_hFalloffLabel->SetSize(10, 10);
	m_hFalloffLabel->EnsureTextFits();
	m_hFalloffLabel->SetPos(5, flControlY);

	m_hFalloffSelector->SetSize(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hFalloffLabel->GetWidth() - flSpace, flSelectorSize);
	m_hFalloffSelector->SetPos(m_hAOOptions->GetOptionsPanel()->GetWidth() - m_hFalloffSelector->GetWidth() - flSpace/2, flControlY);

	flControlY += 30;

	m_hRandomLabel->SetSize(10, 10);
	m_hRandomLabel->EnsureTextFits();
	m_hRandomLabel->SetPos(25, flControlY);

	m_hRandomCheckBox->SetPos(10, flControlY + m_hRandomLabel->GetHeight()/2 - m_hRandomCheckBox->GetHeight()/2);

	flControlY += 30;

	m_hGroundOcclusionLabel->SetSize(10, 10);
	m_hGroundOcclusionLabel->EnsureTextFits();
	m_hGroundOcclusionLabel->SetPos(25, flControlY);

	m_hGroundOcclusionCheckBox->SetPos(10, flControlY + m_hGroundOcclusionLabel->GetHeight()/2 - m_hGroundOcclusionCheckBox->GetHeight()/2);

	m_hNormalLabel->SetPos(35, 280);
	m_hNormalLabel->EnsureTextFits();
	m_hNormalLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hNormalLabel->SetWrap(false);
	m_hNormalCheckBox->SetPos(20, 280 + m_hNormalLabel->GetHeight()/2 - m_hNormalCheckBox->GetHeight()/2);

	m_hGenerate->SetSize(100, 33);
	m_hGenerate->SetPos(GetWidth()/2 - m_hGenerate->GetWidth()/2, GetHeight() - (int)(m_hGenerate->GetHeight()*3));

	m_hSave->SetSize(100, 33);
	m_hSave->SetPos(GetWidth()/2 - m_hSave->GetWidth()/2, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	size_t i;
	m_hLoRes->ClearTree();
	if (!m_apLoResMeshes.size())
		m_hLoRes->AddNode("No meshes. Click 'Add'");
	else
	{
		for (i = 0; i < m_apLoResMeshes.size(); i++)
		{
			m_hLoRes->AddNode<CConversionMeshInstance>(m_apLoResMeshes[i]->GetMesh()->GetName(), m_apLoResMeshes[i]);
			m_hLoRes->GetNode(i)->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());
		}
	}

	m_hHiRes->ClearTree();
	if (!m_apHiResMeshes.size())
		m_hHiRes->AddNode("No meshes. Click 'Add'");
	else
	{
		for (i = 0; i < m_apHiResMeshes.size(); i++)
		{
			m_hHiRes->AddNode<CConversionMeshInstance>(m_apHiResMeshes[i]->GetMesh()->GetName(), m_apHiResMeshes[i]);
			m_hHiRes->GetNode(i)->SetIcon(SMAKWindow()->GetSMAKRenderer()->GetMeshesNodeTexture());
		}
	}

	CMovablePanel::Layout();
}

void CComboGeneratorPanel::UpdateScene()
{
	m_apLoResMeshes.clear();
	m_apHiResMeshes.clear();

	if (m_pScene->GetNumMeshes())
		m_hFalloffSelector->SetSelection(m_hFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
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

			if (iMaterial >= SMAKWindow()->GetMaterials().size())
				continue;

			bFoundMaterial = true;
			break;
		}
	}

	CMovablePanel::Think();
}

void CComboGeneratorPanel::Paint(float x, float y, float w, float h)
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
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CComboGeneratorPanel::GenerateCallback(const tstring& sArgs)
{
	if (m_oGenerator.IsGenerating())
	{
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_hSave->SetVisible(false);

	m_hGenerate->SetText("Cancel");

	CSMAKWindow::Get()->SetDisplayNormal(true);

	// Disappear all of the hi-res meshes so we can see the lo res better.
	for (size_t m = 0; m < m_apHiResMeshes.size(); m++)
		m_apHiResMeshes[m]->SetVisible(false);

	for (size_t m = 0; m < m_apLoResMeshes.size(); m++)
		m_apLoResMeshes[m]->SetVisible(true);

	int iSize = m_hSizeSelector->GetSelectionValue();
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.ClearMethods();

	if (m_hDiffuseCheckBox->GetState())
	{
		m_oGenerator.AddDiffuse();
		CSMAKWindow::Get()->SetDisplayTexture(true);
	}

	if (m_hAOCheckBox->GetState())
	{
		m_oGenerator.AddAO(
			m_hSamplesSelector->GetSelectionValue(),
			m_hRandomCheckBox->GetState(),
			m_hFalloffSelector->GetSelectionValue(),
			m_hGroundOcclusionCheckBox->GetState(),
			m_hBleedSelector->GetSelectionValue());
		CSMAKWindow::Get()->SetDisplayAO(true);
	}

	if (m_hNormalCheckBox->GetState())
	{
		m_oGenerator.AddNormal();
		CSMAKWindow::Get()->SetDisplayNormal(true);
	}

	m_oGenerator.SetModels(m_apHiResMeshes, m_apLoResMeshes);
	m_oGenerator.SetWorkListener(this);
	m_oGenerator.Generate();

	CTextureHandle hDiffuse;
	CTextureHandle hAO;
	CTextureHandle hNormal;
	if (m_oGenerator.DoneGenerating())
	{
		hDiffuse = m_oGenerator.GenerateDiffuse();
		hAO = m_oGenerator.GenerateAO();
		hNormal = m_oGenerator.GenerateNormal();
	}

	for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
	{
		CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		hMaterial->SetParameter("DiffuseTexture", hDiffuse);
		hMaterial->SetParameter("AmbientOcclusion", hAO);
		hMaterial->SetParameter("Normal", hNormal);
	}

	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

	m_hGenerate->SetText("Generate");
}

void CComboGeneratorPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CComboGeneratorPanel::SaveMapFileCallback(const tstring& sArgs)
{
	tstring sFilename = sArgs;

	if (!sFilename.length())
		return;

	m_oGenerator.SaveAll(sFilename);

	for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
	{
		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		m_pScene->GetMaterial(i)->m_sNormalTexture = sFilename;
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
	static double flLastTime = 0;
	static double flLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && CSMAKWindow::Get()->GetTime() - flLastTime < 0.01f)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	if (m_oGenerator.IsGenerating() && CSMAKWindow::Get()->GetTime() - flLastGenerate > 0.5f)
	{
		CTextureHandle hDiffuse = m_oGenerator.GenerateDiffuse(true);
		CTextureHandle hAO = m_oGenerator.GenerateAO(true);
		CTextureHandle hNormal = m_oGenerator.GenerateNormal(true);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			CMaterialHandle hMaterial = SMAKWindow()->GetMaterials()[i];
			if (!hMaterial.IsValid())
				continue;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			hMaterial->SetParameter("DiffuseTexture", hDiffuse);
			hMaterial->SetParameter("AmbientOcclusion", hAO);
			hMaterial->SetParameter("Normal", hNormal);
		}

		flLastGenerate = CSMAKWindow::Get()->GetTime();
	}

	CSMAKWindow::Get()->Render();
	SMAKWindow()->SwapBuffers();

	flLastTime = CSMAKWindow::Get()->GetTime();
}

void CComboGeneratorPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CComboGeneratorPanel::AddLoResCallback(const tstring& sArgs)
{
	if (m_hMeshInstancePicker)
		m_hMeshInstancePicker->Close();

	m_hMeshInstancePicker = (new CMeshInstancePicker(this, AddLoResMesh))->GetHandle();

	float x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_hMeshInstancePicker->GetSize(pw, ph);
	m_hMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CComboGeneratorPanel::AddHiResCallback(const tstring& sArgs)
{
	if (m_hMeshInstancePicker)
		m_hMeshInstancePicker->Close();

	m_hMeshInstancePicker = (new CMeshInstancePicker(this, AddHiResMesh))->GetHandle();

	float x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_hMeshInstancePicker->GetSize(pw, ph);
	m_hMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CComboGeneratorPanel::AddLoResMeshCallback(const tstring& sArgs)
{
	CConversionMeshInstance* pMeshInstance = m_hMeshInstancePicker->GetPickedMeshInstance();
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

	m_hMeshInstancePicker->Close();

	Layout();
}

void CComboGeneratorPanel::AddHiResMeshCallback(const tstring& sArgs)
{
	CConversionMeshInstance* pMeshInstance = m_hMeshInstancePicker->GetPickedMeshInstance();
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

	m_hMeshInstancePicker->Close();

	Layout();
}

void CComboGeneratorPanel::RemoveLoResCallback(const tstring& sArgs)
{
	CTreeNode* pNode = m_hLoRes->GetSelectedNode();
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

void CComboGeneratorPanel::RemoveHiResCallback(const tstring& sArgs)
{
	CTreeNode* pNode = m_hHiRes->GetSelectedNode();
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

void CComboGeneratorPanel::DroppedLoResMeshCallback(const tstring& sArgs)
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

void CComboGeneratorPanel::DroppedHiResMeshCallback(const tstring& sArgs)
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

	m_hMeshInstancePicker->Close();

	Layout();
}

void CComboGeneratorPanel::Open(CConversionScene* pScene)
{
	CComboGeneratorPanel* pPanel = s_pComboGeneratorPanel.DowncastStatic<CComboGeneratorPanel>();

	if (pPanel)
		pPanel->Close();

	s_pComboGeneratorPanel = CreateControl(new CComboGeneratorPanel(pScene));
	pPanel = s_pComboGeneratorPanel.DowncastStatic<CComboGeneratorPanel>();

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();

	if (pScene->GetNumMeshes())
		pPanel->m_hFalloffSelector->SetSelection(pPanel->m_hFalloffSelector->FindClosestSelectionValue(pScene->m_oExtends.Size().Length()/2));
}

void CComboGeneratorPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

CResource<CBaseControl> CNormalPanel::s_pNormalPanel;

CNormalPanel::CNormalPanel(CConversionScene* pScene)
	: CMovablePanel("Normal map generator"), m_oGenerator(pScene)
{
	m_pScene = pScene;
}

void CNormalPanel::CreateControls(CResource<glgui::CBaseControl> pThis)
{
	m_hMaterialsLabel = AddControl(new CLabel(0, 0, 32, 32, "Choose A Material To Generate From:"));

	m_hMaterials = AddControl(new CTree(SMAKWindow()->GetSMAKRenderer()->GetArrowTexture(), SMAKWindow()->GetSMAKRenderer()->GetEditTexture(), SMAKWindow()->GetSMAKRenderer()->GetVisibilityTexture()));
	m_hMaterials->SetBackgroundColor(g_clrBox);

	m_hProgressLabel = AddControl(new CLabel(0, 0, 100, 100, ""));

	m_hDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Overall Depth"));

	m_hDepthSelector = AddControl(new CScrollSelector<float>());
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hDepthSelector->SetSelection(13);
	m_hDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hHiDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Texture Hi-Freq Depth"));

	m_hHiDepthSelector = AddControl(new CScrollSelector<float>());
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hHiDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hHiDepthSelector->SetSelection(13);
	m_hHiDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hMidDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Texture Mid-Freq Depth"));

	m_hMidDepthSelector = AddControl(new CScrollSelector<float>());
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hMidDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hMidDepthSelector->SetSelection(13);
	m_hMidDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hLoDepthLabel = AddControl(new CLabel(0, 0, 32, 32, "Texture Lo-Freq Depth"));

	m_hLoDepthSelector = AddControl(new CScrollSelector<float>());
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.0f, "0%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.025f, "2.5%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.05f, "5%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.075f, "7.5%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.1f, "10%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.2f, "20%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.3f, "30%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.4f, "40%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.5f, "50%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.6f, "60%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.7f, "70%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.8f, "80%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(0.9f, "90%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.0f, "100%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.1f, "110%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.2f, "120%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.3f, "130%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.4f, "140%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.5f, "150%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.6f, "160%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.7f, "170%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.8f, "180%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(1.9f, "190%"));
	m_hLoDepthSelector->AddSelection(CScrollSelection<float>(2.0f, "200%"));
	m_hLoDepthSelector->SetSelection(13);
	m_hLoDepthSelector->SetSelectedListener(this, UpdateNormal2);

	m_hSave = AddControl(new CButton(0, 0, 100, 100, "Save Map"));

	m_hSave->SetClickedListener(this, SaveMapDialog);
	m_hSave->SetVisible(false);

	BaseClass::CreateControls(pThis);

	Layout();
}

void CNormalPanel::Layout()
{
	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	float flSpace = 20;

	m_hMaterialsLabel->EnsureTextFits();

	float flSelectorSize = m_hMaterialsLabel->GetHeight() - 4;
	float flControlY = HEADER_HEIGHT;

	m_hMaterialsLabel->SetPos(5, flControlY);

	float flTreeWidth = GetWidth()/2-15;

	m_hMaterials->ClearTree();
	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		m_hMaterials->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));

		if (SMAKWindow()->GetMaterials().size() > i && SMAKWindow()->GetMaterials()[i]->FindParameter("DiffuseTexture") == ~0)
			m_hMaterials->GetNode(i)->m_hLabel->SetAlpha(100);

		m_hMaterials->SetSelectedListener(this, SetupNormal2);
	}

	m_hMaterials->SetSize(flTreeWidth, 150);
	m_hMaterials->SetPos(GetWidth()/2-flTreeWidth/2, 50);

	m_hProgressLabel->SetSize(1, 1);
	m_hProgressLabel->SetPos(35, 220);
	m_hProgressLabel->EnsureTextFits();
	m_hProgressLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hProgressLabel->SetWrap(false);

	m_hDepthLabel->SetPos(10, 290);
	m_hDepthLabel->EnsureTextFits();
	m_hDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hDepthLabel->SetWrap(false);
	m_hDepthSelector->SetPos(m_hDepthLabel->GetRight() + 10, 290 + m_hDepthLabel->GetHeight()/2 - m_hDepthSelector->GetHeight()/2);
	m_hDepthSelector->SetRight(GetWidth() - 10);

	m_hHiDepthLabel->SetPos(10, 310);
	m_hHiDepthLabel->EnsureTextFits();
	m_hHiDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hHiDepthLabel->SetWrap(false);
	m_hHiDepthSelector->SetPos(m_hHiDepthLabel->GetRight() + 10, 310 + m_hHiDepthLabel->GetHeight()/2 - m_hHiDepthSelector->GetHeight()/2);
	m_hHiDepthSelector->SetRight(GetWidth() - 10);

	m_hMidDepthLabel->SetPos(10, 330);
	m_hMidDepthLabel->EnsureTextFits();
	m_hMidDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hMidDepthLabel->SetWrap(false);
	m_hMidDepthSelector->SetPos(m_hMidDepthLabel->GetRight() + 10, 330 + m_hMidDepthLabel->GetHeight()/2 - m_hMidDepthSelector->GetHeight()/2);
	m_hMidDepthSelector->SetRight(GetWidth() - 10);

	m_hLoDepthLabel->SetPos(10, 350);
	m_hLoDepthLabel->EnsureTextFits();
	m_hLoDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_hLoDepthLabel->SetWrap(false);
	m_hLoDepthSelector->SetPos(m_hLoDepthLabel->GetRight() + 10, 350 + m_hLoDepthLabel->GetHeight()/2 - m_hLoDepthSelector->GetHeight()/2);
	m_hLoDepthSelector->SetRight(GetWidth() - 10);

	m_hSave->SetSize(100, 33);
	m_hSave->SetPos(GetWidth()/2 - m_hSave->GetWidth()/2, GetHeight() - (int)(m_hSave->GetHeight()*1.5f));
	m_hSave->SetVisible(m_oGenerator.DoneGenerating());

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
		CTextureHandle hNewNormal2;
		m_oGenerator.GetNormalMap2(hNewNormal2);

		for (size_t i = 0; i < SMAKWindow()->GetMaterials().size(); i++)
		{
			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			SMAKWindow()->GetMaterials()[i]->SetParameter("Normal2", hNewNormal2);
			break;
		}

		m_hSave->SetVisible(hNewNormal2.IsValid());

		if (hNewNormal2.IsValid())
			CSMAKWindow::Get()->SetDisplayNormal(true);
	}

	if (m_oGenerator.IsSettingUp())
	{
		tstring s;
		s = sprintf("Setting up... %d%%", (int)(m_oGenerator.GetSetupProgress()*100));
		m_hProgressLabel->SetText(s);
		m_hSave->SetVisible(false);
	}
	else if (m_oGenerator.IsGeneratingNewNormal2())
	{
		tstring s;
		s = sprintf("Generating... %d%%", (int)(m_oGenerator.GetNormal2GenerationProgress()*100));
		m_hProgressLabel->SetText(s);
		m_hSave->SetVisible(false);
	}
	else
		m_hProgressLabel->SetText("");

	m_oGenerator.Think();

	CMovablePanel::Think();
}

void CNormalPanel::Paint(float x, float y, float w, float h)
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
		m_hSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CNormalPanel::SaveMapDialogCallback(const tstring& sArgs)
{
	if (!m_oGenerator.DoneGenerating())
		return;

	CFileDialog::ShowSaveDialog("", ".png;.bmp;.tga", this, SaveMapFile);
}

void CNormalPanel::SaveMapFileCallback(const tstring& sArgs)
{
	tstring sFilename = sArgs;

	if (!sFilename.length())
		return;

	SMAKWindow()->SaveNormal(m_oGenerator.GetGenerationMaterial(), sFilename);

	CRootPanel::Get()->Layout();
}

void CNormalPanel::SetupNormal2Callback(const tstring& sArgs)
{
	m_oGenerator.SetNormalTextureDepth(m_hDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureHiDepth(m_hHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureMidDepth(m_hMidDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_hLoDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTexture(m_hMaterials->GetSelectedNodeId());

	CSMAKWindow::Get()->SetDisplayNormal(true);
}

void CNormalPanel::UpdateNormal2Callback(const tstring& sArgs)
{
	m_oGenerator.SetNormalTextureDepth(m_hDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureHiDepth(m_hHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureMidDepth(m_hMidDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_hLoDepthSelector->GetSelectionValue());
	m_oGenerator.UpdateNormal2();
}

void CNormalPanel::Open(CConversionScene* pScene)
{
	CNormalPanel* pPanel = s_pNormalPanel.DowncastStatic<CNormalPanel>();

	if (pPanel)
		pPanel->Close();

	s_pNormalPanel = CreateControl(new CNormalPanel(pScene));
	pPanel = s_pNormalPanel.DowncastStatic<CNormalPanel>();

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

CControl<CHelpPanel> CHelpPanel::s_hHelpPanel;

CHelpPanel::CHelpPanel()
	: CMovablePanel("Help")
{
	m_hInfo = AddControl(new CLabel(0, 0, 100, 100, ""));
	Layout();
}

void CHelpPanel::Layout()
{
	if (GetParent())
	{
		float px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 200);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_hInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_hInfo->SetSize(GetWidth(), GetHeight());
	m_hInfo->SetPos(0, 30);

	m_hInfo->SetText("CONTROLS:\n");
	m_hInfo->AppendText("Left Mouse Button - Move the camera\n");
	m_hInfo->AppendText("Right Mouse Button - Zoom in and out\n");
	m_hInfo->AppendText("Ctrl-LMB - Rotate the light\n");
	m_hInfo->AppendText(" \n");
	m_hInfo->AppendText("For in-depth help information please visit our website, http://www.getsmak.net/\n");

	CMovablePanel::Layout();
}

void CHelpPanel::Paint(float x, float y, float w, float h)
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
	if (!s_hHelpPanel)
		s_hHelpPanel = (new CHelpPanel())->GetHandle();

	s_hHelpPanel->SetVisible(true);
	s_hHelpPanel->Layout();
}

void CHelpPanel::Close()
{
	if (!s_hHelpPanel)
		return;

	s_hHelpPanel->SetVisible(false);
}

CControl<CAboutPanel> CAboutPanel::s_hAboutPanel;

CAboutPanel::CAboutPanel()
	: CMovablePanel("About SMAK")
{
	m_hInfo = AddControl(new CLabel(0, 0, 100, 100, ""));
	Layout();
}

void CAboutPanel::Layout()
{
	if (GetParent())
	{
		float px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 250);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_hInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_hInfo->SetSize(GetWidth(), GetHeight());
	m_hInfo->SetPos(0, 30);

	m_hInfo->SetText("SMAK - The Super Model Army Knife\n");
	m_hInfo->AppendText("Version " SMAK_VERSION "\n");
	m_hInfo->AppendText("Copyright © 2010, Jorge Rodriguez <jorge@lunarworkshop.com>\n");
	m_hInfo->AppendText(" \n");
	m_hInfo->AppendText("FCollada copyright © 2006, Feeling Software\n");
	m_hInfo->AppendText("DevIL copyright © 2001-2009, Denton Woods\n");
	m_hInfo->AppendText("FTGL copyright © 2001-2003, Henry Maddocks\n");
	m_hInfo->AppendText("GLFW copyright © 2002-2007, Camilla Berglund\n");
	m_hInfo->AppendText("pthreads-win32 copyright © 2001, 2006 Ross P. Johnson\n");
	m_hInfo->AppendText("GLEW copyright © 2002-2007, Milan Ikits, Marcelo E. Magallon, Lev Povalahev\n");
	m_hInfo->AppendText("Freetype copyright © 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg\n");

	CMovablePanel::Layout();
}

void CAboutPanel::Paint(float x, float y, float w, float h)
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
	if (!s_hAboutPanel)
		s_hAboutPanel = (new CAboutPanel())->GetHandle();

	s_hAboutPanel->SetVisible(true);
	s_hAboutPanel->Layout();
}

void CAboutPanel::Close()
{
	if (!s_hAboutPanel)
		return;

	s_hAboutPanel->SetVisible(false);
}

