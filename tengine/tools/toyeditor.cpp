#include "toyeditor.h"

#include <iostream>
#include <fstream>

#include <tinker_platform.h>
#include <files.h>

#include <glgui/rootpanel.h>
#include <glgui/movablepanel.h>
#include <glgui/textfield.h>
#include <glgui/button.h>
#include <glgui/menu.h>
#include <glgui/filedialog.h>
#include <glgui/tree.h>
#include <tinker/application.h>
#include <models/models.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <game/gameserver.h>
#include <tinker/keys.h>
#include <ui/gamewindow.h>
#include <tools/toybuilder/geppetto.h>
#include <datamanager/dataserializer.h>
#include <textures/texturelibrary.h>

#include "workbench.h"
#include "manipulator.h"

REGISTER_WORKBENCH_TOOL(ToyEditor);

CCreateToySourcePanel::CCreateToySourcePanel()
	: glgui::CMovablePanel("Create Toy Source Tool")
{
	SetBackgroundColor(Color(0, 0, 0, 255));
	SetHeaderColor(Color(100, 100, 100, 255));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pToyFileLabel = new glgui::CLabel("Toy File:", "sans-serif", 10);
	m_pToyFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pToyFileLabel);
	m_pToyFileText = new glgui::CTextField();
	m_pToyFileText->SetContentsChangedListener(this, ToyChanged);
	AddControl(m_pToyFileText);

	m_pSourceFileLabel = new glgui::CLabel("Source File:", "sans-serif", 10);
	m_pSourceFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pSourceFileLabel);
	m_pSourceFileText = new glgui::CTextField();
	m_pSourceFileText->SetContentsChangedListener(this, SourceChanged);
	AddControl(m_pSourceFileText);

	m_pWarnings = new glgui::CLabel("");
	m_pWarnings->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pWarnings);

	m_pCreate = new glgui::CButton("Create");
	m_pCreate->SetClickedListener(this, Create);
	AddControl(m_pCreate);
}

void CCreateToySourcePanel::Layout()
{
	float flTop = 20;
	m_pToyFileLabel->SetLeft(15);
	m_pToyFileLabel->SetTop(flTop);
	m_pToyFileText->SetWidth(GetWidth()-30);
	m_pToyFileText->CenterX();
	m_pToyFileText->SetTop(flTop+12);

	flTop += 43;

	m_pSourceFileLabel->SetLeft(15);
	m_pSourceFileLabel->SetTop(flTop);
	m_pSourceFileText->SetWidth(GetWidth()-30);
	m_pSourceFileText->CenterX();
	m_pSourceFileText->SetTop(flTop+12);

	flTop += 43;

	m_pWarnings->SetLeft(15);
	m_pWarnings->SetTop(flTop);
	m_pWarnings->SetWidth(GetWidth()-30);
	m_pWarnings->CenterX();
	m_pWarnings->SetWrap(true);
	m_pWarnings->SetBottom(GetBottom() - 60);

	m_pCreate->SetTop(GetHeight() - 45);
	m_pCreate->CenterX();

	BaseClass::Layout();

	FileNamesChanged();
}

void CCreateToySourcePanel::ToyChangedCallback(const tstring& sArgs)
{
	FileNamesChanged();

	if (!m_pToyFileText->GetText().length())
		return;

	eastl::vector<tstring> asExtensions;
	eastl::vector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_pToyFileText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

void CCreateToySourcePanel::SourceChangedCallback(const tstring& sArgs)
{
	FileNamesChanged();

	if (!m_pSourceFileText->GetText().length())
		return;

	eastl::vector<tstring> asExtensions;
	asExtensions.push_back(".txt");

	m_pSourceFileText->SetAutoCompleteFiles("../sources", asExtensions);
}

tstring CCreateToySourcePanel::GetToyFileName()
{
	tstring sToyFile = m_pToyFileText->GetText();

	if (sToyFile.length() <= 4 || sToyFile.substr(sToyFile.length()-4) != ".toy")
		sToyFile.append(".toy");

	return sToyFile;
}

tstring CCreateToySourcePanel::GetSourceFileName()
{
	tstring sSourceFile = m_pSourceFileText->GetText();

	if (sSourceFile.length() <= 4 || sSourceFile.substr(sSourceFile.length()-4) != ".txt")
		sSourceFile.append(".txt");

	return "../sources/" + sSourceFile;
}

void CCreateToySourcePanel::FileNamesChanged()
{
	m_pWarnings->SetText("");

	tstring sToyFile = GetToyFileName();
	if (IsFile(sToyFile))
		m_pWarnings->SetText("WARNING: This toy file already exists. It will be overwritten when the new source file is built.");

	tstring sSourceFile = GetSourceFileName();
	if (IsFile(sSourceFile))
	{
		if (m_pWarnings->GetText().length())
			m_pWarnings->AppendText("\n\n");
		m_pWarnings->AppendText("WARNING: This source file already exists. It will be overwritten.");
	}

	m_pCreate->SetVisible(false);

	if (m_pSourceFileText->GetText().length() && m_pToyFileText->GetText().length())
		m_pCreate->SetVisible(true);
}

void CCreateToySourcePanel::CreateCallback(const tstring& sArgs)
{
	if (!m_pSourceFileText->GetText().length() || !m_pToyFileText->GetText().length())
		return;

	ToyEditor()->NewToy();
	ToyEditor()->GetToyToModify().m_sFilename = GetSourceFileName();
	ToyEditor()->GetToyToModify().m_sToyFile = GetToyFileName();

	SetVisible(false);

	ToyEditor()->Layout();
}

CSourcePanel::CSourcePanel()
{
	SetBackgroundColor(Color(0, 0, 0, 150));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pFilename = new glgui::CLabel("", "sans-serif", 16);
	AddControl(m_pFilename);

	m_pToyFileLabel = new glgui::CLabel("Toy File: ", "sans-serif", 10);
	m_pToyFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pToyFileLabel);

	m_pToyFileText = new glgui::CTextField();
	m_pToyFileText->SetContentsChangedListener(this, ToyFileChanged);
	AddControl(m_pToyFileText, true);

	m_pMeshLabel = new glgui::CLabel("Mesh: ", "sans-serif", 10);
	m_pMeshLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pMeshLabel);

	m_pMeshText = new glgui::CTextField();
	m_pMeshText->SetContentsChangedListener(this, ModelChanged, "mesh");
	AddControl(m_pMeshText, true);

	m_pPhysLabel = new glgui::CLabel("Physics: ", "sans-serif", 10);
	m_pPhysLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pPhysLabel);

	m_pPhysText = new glgui::CTextField();
	m_pPhysText->SetContentsChangedListener(this, ModelChanged, "phys");
	AddControl(m_pPhysText, true);

	m_pPhysShapesLabel = new glgui::CLabel("Physics Shapes: ", "sans-serif", 10);
	m_pPhysShapesLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pPhysShapesLabel);

	m_pPhysicsShapes = new glgui::CTree(0, 0, 0);
	m_pPhysicsShapes->SetBackgroundColor(Color(0, 0, 0, 100));
	m_pPhysicsShapes->SetSelectedListener(this, PhysicsAreaSelected);
	AddControl(m_pPhysicsShapes);

	m_pNewPhysicsShape = new glgui::CButton("New Shape", false, "sans-serif", 10);
	m_pNewPhysicsShape->SetClickedListener(this, NewPhysicsShape);
	AddControl(m_pNewPhysicsShape);

	m_pDeletePhysicsShape = new glgui::CButton("Delete Shape", false, "sans-serif", 10);
	m_pDeletePhysicsShape->SetClickedListener(this, DeletePhysicsShape);
	AddControl(m_pDeletePhysicsShape);

	m_pSave = new glgui::CButton("Save");
	m_pSave->SetClickedListener(this, Save);
	AddControl(m_pSave);

	m_pBuild = new glgui::CButton("Build");
	m_pBuild->SetClickedListener(this, Build);
	AddControl(m_pBuild);
}

void CSourcePanel::SetVisible(bool bVis)
{
	if (bVis && !IsVisible())
		UpdateFields();

	BaseClass::SetVisible(bVis);
}

void CSourcePanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_pFilename->Layout_AlignTop();
	m_pFilename->Layout_FullWidth(0);

	BaseClass::Layout();

	const CToySource* pToySource = &ToyEditor()->GetToy();

	if (!pToySource->m_sFilename.length())
		return;

	LayoutFilename();

	m_pToyFileLabel->Layout_AlignTop(m_pFilename);
	m_pToyFileLabel->SetWidth(10);
	m_pToyFileLabel->SetHeight(1);
	m_pToyFileLabel->EnsureTextFits();
	m_pToyFileLabel->Layout_FullWidth();

	m_pToyFileText->Layout_FullWidth();
	m_pToyFileText->SetTop(m_pToyFileLabel->GetTop()+12);

	float flControlMargin = 12;

	m_pMeshLabel->Layout_AlignTop(m_pToyFileText, flControlMargin);
	m_pMeshLabel->SetWidth(10);
	m_pMeshLabel->SetHeight(1);
	m_pMeshLabel->EnsureTextFits();
	m_pMeshLabel->Layout_FullWidth();

	m_pMeshText->Layout_FullWidth();
	m_pMeshText->SetTop(m_pMeshLabel->GetTop()+12);

	m_pPhysLabel->Layout_AlignTop(m_pMeshText, flControlMargin);
	m_pPhysLabel->SetWidth(10);
	m_pPhysLabel->SetHeight(1);
	m_pPhysLabel->EnsureTextFits();
	m_pPhysLabel->Layout_FullWidth();

	m_pPhysText->Layout_FullWidth();
	m_pPhysText->SetTop(m_pPhysLabel->GetTop()+12);

	m_pPhysShapesLabel->Layout_AlignTop(m_pPhysText, flControlMargin);
	m_pPhysShapesLabel->SetWidth(10);
	m_pPhysShapesLabel->SetHeight(1);
	m_pPhysShapesLabel->EnsureTextFits();
	m_pPhysShapesLabel->Layout_FullWidth();

	m_pPhysicsShapes->Layout_FullWidth();
	m_pPhysicsShapes->SetTop(m_pPhysShapesLabel->GetTop()+12);
	m_pPhysicsShapes->SetHeight(100);

	m_pPhysicsShapes->ClearTree();

	for (size_t i = 0; i < pToySource->m_aShapes.size(); i++)
		m_pPhysicsShapes->AddNode("Rectangle");

	m_pNewPhysicsShape->SetHeight(20);
	m_pNewPhysicsShape->Layout_Column(2, 0);
	m_pNewPhysicsShape->Layout_AlignTop(m_pPhysicsShapes, 5);
	m_pDeletePhysicsShape->SetHeight(20);
	m_pDeletePhysicsShape->Layout_Column(2, 1);
	m_pDeletePhysicsShape->Layout_AlignTop(m_pPhysicsShapes, 5);
	m_pDeletePhysicsShape->SetEnabled(false);

	m_pSave->Layout_Column(2, 0);
	m_pSave->Layout_AlignBottom();
	m_pBuild->Layout_Column(2, 1);
	m_pBuild->Layout_AlignBottom();

	BaseClass::Layout();
}

void CSourcePanel::LayoutFilename()
{
	const CToySource* pToySource = &ToyEditor()->GetToy();

	if (!pToySource->m_sFilename.length())
		return;

	tstring sFilename = pToySource->m_sFilename;
	tstring sAbsoluteSourcePath = FindAbsolutePath("../sources/");
	tstring sAbsoluteFilename = FindAbsolutePath(sFilename);
	if (sAbsoluteFilename.find(sAbsoluteSourcePath) == 0)
		sFilename = ToForwardSlashes(sAbsoluteFilename.substr(sAbsoluteSourcePath.length()));
	m_pFilename->SetText(sFilename);
	if (!ToyEditor()->IsSaved())
		m_pFilename->AppendText(" *");
}

void CSourcePanel::UpdateFields()
{
	m_pToyFileText->SetText(ToyEditor()->GetToy().m_sToyFile);
	m_pMeshText->SetText(ToyEditor()->GetToy().m_sMesh);
	m_pPhysText->SetText(ToyEditor()->GetToy().m_sPhys);
}

void CSourcePanel::ToyFileChangedCallback(const tstring& sArgs)
{
	ToyEditor()->GetToyToModify().m_sToyFile = m_pToyFileText->GetText();

	if (!m_pToyFileText->GetText().length())
		return;

	eastl::vector<tstring> asExtensions;
	eastl::vector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_pToyFileText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

void CSourcePanel::ModelChangedCallback(const tstring& sArgs)
{
	glgui::CTextField* pField;
	if (sArgs == "mesh")
	{
		pField = m_pMeshText;
		ToyEditor()->GetToyToModify().m_sMesh = pField->GetText();
	}
	else
	{
		pField = m_pPhysText;
		ToyEditor()->GetToyToModify().m_sPhys = pField->GetText();
	}

	eastl::vector<tstring> asExtensions;
	asExtensions.push_back(".obj");
	asExtensions.push_back(".sia");
	asExtensions.push_back(".dae");
	asExtensions.push_back(".png");

	pField->SetAutoCompleteFiles(GetDirectory(ToyEditor()->GetToy().m_sFilename), asExtensions);

	ToyEditor()->Layout();
}

void CSourcePanel::PhysicsAreaSelectedCallback(const tstring& sArgs)
{
	if (m_pPhysicsShapes->GetSelectedNodeId() < ToyEditor()->GetToy().m_aShapes.size())
	{
		Manipulator()->Activate(ToyEditor(), ToyEditor()->GetToy().m_aShapes[m_pPhysicsShapes->GetSelectedNodeId()].m_trsTransform, "PhysicsShape " + sprintf("%d", m_pPhysicsShapes->GetSelectedNodeId()));
		m_pDeletePhysicsShape->SetEnabled(true);
	}
	else
	{
		Manipulator()->Deactivate();
		m_pDeletePhysicsShape->SetEnabled(false);
	}
}

void CSourcePanel::NewPhysicsShapeCallback(const tstring& sArgs)
{
	auto& oPhysicsShape = ToyEditor()->GetToyToModify().m_aShapes.push_back();
	oPhysicsShape.m_aabbBounds = AABB(-Vector(0.5f, 0.5f, 0.5f), Vector(0.5f, 0.5f, 0.5f));

	Layout();
}

void CSourcePanel::DeletePhysicsShapeCallback(const tstring& sArgs)
{
	if (m_pPhysicsShapes->GetSelectedNodeId() >= ToyEditor()->GetToy().m_aShapes.size())
		return;

	size_t iSelected = m_pPhysicsShapes->GetSelectedNodeId();	// Grab before GetToyToModify() since a Layout() is called which resecs the shapes list.
	auto pToy = &ToyEditor()->GetToyToModify();
	pToy->m_aShapes.erase(pToy->m_aShapes.begin()+iSelected);

	Layout();
}

void CSourcePanel::SaveCallback(const tstring& sArgs)
{
	ToyEditor()->GetToy().Save();
}

void CSourcePanel::BuildCallback(const tstring& sArgs)
{
	ToyEditor()->GetToy().Build();
}

CToyEditor* CToyEditor::s_pToyEditor = nullptr;

CToyEditor::CToyEditor()
{
	s_pToyEditor = this;

	m_pCreateToySourcePanel = new CCreateToySourcePanel();
	m_pCreateToySourcePanel->Layout();
	m_pCreateToySourcePanel->Center();
	m_pCreateToySourcePanel->SetVisible(false);

	m_pSourcePanel = new CSourcePanel();
	m_pSourcePanel->SetVisible(false);
	glgui::CRootPanel::Get()->AddControl(m_pSourcePanel);

	m_iMeshPreview = ~0;
	m_iPhysPreview = ~0;
	m_iTexturePreview = 0;

	m_bRotatingPreview = false;
	m_angPreview = EAngle(-20, 20, 0);

	m_bSaved = false;
}

CToyEditor::~CToyEditor()
{
	delete m_pCreateToySourcePanel;
}

void CToyEditor::Activate()
{
	Layout();
}

void CToyEditor::Deactivate()
{
	m_pCreateToySourcePanel->SetVisible(false);
	m_pSourcePanel->SetVisible(false);
}

void CToyEditor::Layout()
{
	m_pCreateToySourcePanel->SetVisible(false);
	m_pSourcePanel->SetVisible(false);

	if (!m_oToySource.m_sFilename.length())
		m_pCreateToySourcePanel->SetVisible(true);
	else
		m_pSourcePanel->SetVisible(true);

	SetupMenu();

	tstring sMesh = FindAbsolutePath(GetDirectory(GetToy().m_sFilename) + "/" + GetToy().m_sMesh);
	if (IsFile(sMesh))
	{
		if (m_iMeshPreview != ~0)
		{
			CModel* pMesh = CModelLibrary::GetModel(m_iMeshPreview);
			if (sMesh != FindAbsolutePath(pMesh->m_sFilename))
			{
				CModelLibrary::ReleaseModel(m_iMeshPreview);
				CModelLibrary::ClearUnreferenced();
				m_iMeshPreview = CModelLibrary::AddModel(sMesh);
			}
		}
		else
			m_iMeshPreview = CModelLibrary::AddModel(sMesh);
	}
	else
	{
		if (m_iMeshPreview != ~0)
		{
			CModelLibrary::ReleaseModel(m_iMeshPreview);
			CModelLibrary::ClearUnreferenced();
			m_iMeshPreview = ~0;
		}
	}

	tstring sTexture = FindAbsolutePath(GetDirectory(GetToy().m_sFilename) + "/" + GetToy().m_sMesh);
	if (IsFile(sTexture))
		// Don't bother with clearing old ones, they'll get flushed eventually.
		m_iTexturePreview = CTextureLibrary::AddTextureID(sTexture);
	else
		m_iTexturePreview = ~0;

	tstring sPhys = FindAbsolutePath(GetDirectory(GetToy().m_sFilename) + "/" + GetToy().m_sPhys);
	if (IsFile(sPhys))
	{
		if (m_iPhysPreview != ~0)
		{
			CModel* pPhys = CModelLibrary::GetModel(m_iPhysPreview);
			if (sPhys != FindAbsolutePath(pPhys->m_sFilename))
			{
				CModelLibrary::ReleaseModel(m_iPhysPreview);
				CModelLibrary::ClearUnreferenced();
				m_iPhysPreview = CModelLibrary::AddModel(sPhys);
			}
		}
		else
			m_iPhysPreview = CModelLibrary::AddModel(sPhys);
	}
	else
	{
		if (m_iPhysPreview != ~0)
		{
			CModelLibrary::ReleaseModel(m_iPhysPreview);
			CModelLibrary::ClearUnreferenced();
		}
		m_iPhysPreview = ~0;
	}
}

void CToyEditor::SetupMenu()
{
	GetFileMenu()->ClearSubmenus();

	GetFileMenu()->AddSubmenu("New", this, NewToy);
	GetFileMenu()->AddSubmenu("Open", this, ChooseToy);

	if (GetToy().m_sFilename.length())
	{
		GetFileMenu()->AddSubmenu("Save", this, SaveToy);
		GetFileMenu()->AddSubmenu("Build", this, BuildToy);
	}
}

void CToyEditor::RenderScene()
{
	if (m_iMeshPreview != ~0)
		TAssert(CModelLibrary::GetModel(m_iMeshPreview));

	if (m_iMeshPreview != ~0 && CModelLibrary::GetModel(m_iMeshPreview))
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.SetColor(Color(255, 255, 255));

		c.RenderModel(m_iMeshPreview);
	}

	if (m_iTexturePreview != 0)
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.SetColor(Color(255, 255, 255));

		c.RenderTextureModel(m_iTexturePreview);
	}

	if (m_iPhysPreview != ~0 && CModelLibrary::GetModel(m_iPhysPreview))
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.ClearDepth();

		float flAlpha = 0.3f;
		if (m_iMeshPreview == ~0 && m_iTexturePreview == 0)
			flAlpha = 1.0f;

		c.SetColor(Color(0, 100, 155, (int)(255*flAlpha)));
		c.SetAlpha(flAlpha);
		if (flAlpha < 1)
			c.SetBlend(BLEND_ALPHA);

		c.RenderModel(m_iPhysPreview);
	}

	for (size_t i = 0; i < GetToy().m_aShapes.size(); i++)
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.ClearDepth();

		c.UseProgram("model");
		c.SetUniform("bDiffuse", false);

		float flAlpha = 0.2f;
		if (m_pSourcePanel->m_pPhysicsShapes->GetSelectedNodeId() == i)
			flAlpha = 0.8f;
		if (m_iMeshPreview == ~0 && m_iTexturePreview == 0)
			flAlpha = 1.0f;
		if (flAlpha < 1)
			c.SetBlend(BLEND_ALPHA);

		if (m_pSourcePanel->m_pPhysicsShapes->GetSelectedNodeId() == i)
			c.SetUniform("vecColor", Color(255, 50, 100, (char)(255*flAlpha)));
		else
			c.SetUniform("vecColor", Color(0, 100, 200, (char)(255*flAlpha)));

		if (m_pSourcePanel->m_pPhysicsShapes->GetSelectedNodeId() == i)
			c.Transform(Manipulator()->GetTransform());
		else
			c.Transform(GetToy().m_aShapes[i].m_trsTransform.GetMatrix4x4());

		c.RenderWireBox(GetToy().m_aShapes[i].m_aabbBounds);

		// Reset the uniforms so other stuff doesn't get this ugly color.
		c.SetUniform("bDiffuse", true);
		c.SetUniform("vecColor", Color(255, 255, 255, 255));
	}
}

void CToyEditor::NewToy()
{
	m_oToySource = CToySource();
	MarkUnsaved();
}

CToySource& CToyEditor::GetToyToModify()
{
	MarkUnsaved();
	return m_oToySource;
}

void CToyEditor::MarkUnsaved()
{
	m_bSaved = false;

	// Don't call Layout, it will invalidate all sorts of important stuff.
	m_pSourcePanel->LayoutFilename();
}

void CToyEditor::MarkSaved()
{
	m_bSaved = true;
	m_pSourcePanel->Layout();
}

void CToyEditor::NewToyCallback(const tstring& sArgs)
{
	m_pCreateToySourcePanel->SetVisible(true);
}

void CToyEditor::SaveToyCallback(const tstring& sArgs)
{
	m_oToySource.Save();
}

void CToyEditor::ChooseToyCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowOpenDialog("../sources", ".txt", this, OpenToy);
}

void CToyEditor::OpenToyCallback(const tstring& sArgs)
{
	m_oToySource.Open(sArgs);

	m_pSourcePanel->Layout();
	m_pSourcePanel->UpdateFields();
	Layout();
}

void CToyEditor::BuildToyCallback(const tstring& sArgs)
{
	m_oToySource.Build();
}

bool CToyEditor::KeyPress(int c)
{
	// ; because my dvorak to qwerty key mapper works against me when the game is open, oh well.
	if ((c == 'S' || c == ';') && Application()->IsCtrlDown())
	{
		m_oToySource.Save();

		return true;
	}

	return false;
}

bool CToyEditor::MouseInput(int iButton, int iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		m_bRotatingPreview = (iState == 1);
		return true;
	}

	return false;
}

void CToyEditor::MouseMotion(int x, int y)
{
	if (m_bRotatingPreview)
	{
		int lx, ly;
		if (GameWindow()->GetLastMouse(lx, ly))
		{
			m_angPreview.y += (float)(x-lx);
			m_angPreview.p -= (float)(y-ly);
		}
	}
}

TVector CToyEditor::GetCameraPosition()
{
	CModel* pMesh = CModelLibrary::GetModel(m_iMeshPreview);

	Vector vecPreviewAngle = AngleVector(m_angPreview)*10;
	if (!pMesh)
	{
		CModel* pPhys = CModelLibrary::GetModel(m_iPhysPreview);
		if (!pPhys)
			return TVector(0, 0, 0) - vecPreviewAngle;

		return pPhys->m_aabbBoundingBox.Center() - vecPreviewAngle;
	}

	return pMesh->m_aabbBoundingBox.Center() - vecPreviewAngle;
}

TVector CToyEditor::GetCameraDirection()
{
	return AngleVector(m_angPreview);
}

void CToyEditor::ManipulatorUpdated(const tstring& sArguments)
{
	// Grab this before GetToyToModify since that does a layout and clobbers the list.
	size_t iSelected = m_pSourcePanel->m_pPhysicsShapes->GetSelectedNodeId();

	eastl::vector<tstring> asTokens;
	strtok(sArguments, asTokens);
	TAssert(asTokens.size() == 2);
	TAssert(stoi(asTokens[1]) == iSelected);
	TAssert(iSelected != ~0);

	ToyEditor()->GetToyToModify().m_aShapes[iSelected].m_trsTransform = Manipulator()->GetTRS();
}

void CToySource::Save() const
{
	if (!m_sFilename.length())
		return;

	CreateDirectory(GetDirectory(m_sFilename));

	std::basic_ofstream<tchar> f(m_sFilename.c_str());

	tstring sMessage = "// Generated by the Tinker Engine\n// Feel free to modify\n\n";
	f.write(sMessage.data(), sMessage.length());

	tstring sGame = "Game: " + GetRelativePath(".", GetDirectory(m_sFilename)) + "\n";
	f.write(sGame.data(), sGame.length());

	tstring sOutput = "Output: " + m_sToyFile + "\n\n";
	f.write(sOutput.data(), sOutput.length());

	if (m_sMesh.length())
	{
		tstring sGame = "Mesh: " + m_sMesh + "\n";
		f.write(sGame.data(), sGame.length());
	}

	if (m_sPhys.length())
	{
		tstring sGame = "Physics: " + m_sPhys + "\n";
		f.write(sGame.data(), sGame.length());
	}

	ToyEditor()->MarkSaved();
}

void CToySource::Build() const
{
	Save();

	CGeppetto g(true, FindAbsolutePath(GetDirectory(m_sFilename)));

	g.BuildFromInputScript(FindAbsolutePath(m_sFilename));
}

void CToySource::Open(const tstring& sFile)
{
	std::basic_ifstream<tchar> f((sFile).c_str());
	if (!f.is_open())
	{
		TError("Could not read input script '" + sFile + "'\n");
		return;
	}

	m_sFilename = sFile;

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CData* pOutput = pData->FindChild("Output");
	CData* pSceneAreas = pData->FindChild("SceneAreas");
	CData* pMesh = pData->FindChild("Mesh");
	CData* pPhysics = pData->FindChild("Physics");

	TAssert(!pSceneAreas);	// This is unimplemented.

	if (pOutput)
		m_sToyFile = pOutput->GetValueTString();
	else
		m_sToyFile = "";

	if (pMesh)
		m_sMesh = pMesh->GetValueTString();
	else
		m_sMesh = "";

	if (pPhysics)
		m_sPhys = pPhysics->GetValueTString();
	else
		m_sPhys = "";

	ToyEditor()->MarkSaved();
}