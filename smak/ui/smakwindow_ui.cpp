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
#include "progressbar.h"
#include "buttonpanel.h"
#include "combogenerator.h"
#include "aogenerator.h"
#include "normalgenerator.h"

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

