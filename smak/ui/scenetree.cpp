#include "scenetree.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <tinker_platform.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/filedialog.h>

using namespace glgui;

CSceneTreePanel* CSceneTreePanel::s_pSceneTreePanel = NULL;

CSceneTreePanel::CSceneTreePanel(CConversionScene* pScene)
	: CMovablePanel("Scene Tree")
{
	m_pScene = pScene;
	m_pTree = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	AddControl(m_pTree);

	HasCloseButton(false);
	SetClearBackground(true);

	// Infinite height so that scene objects are always clickable.
	SetSize(GetWidth(), 10000);
	SetPos(50, 150);

	m_pMaterialEditor = NULL;
}

CSceneTreePanel::~CSceneTreePanel()
{
	delete m_pTree;

	if (m_pMaterialEditor)
		delete m_pMaterialEditor;
}

void CSceneTreePanel::Layout()
{
	m_pTree->SetPos(5, HEADER_HEIGHT);
	m_pTree->SetSize(GetWidth() - 5, GetHeight() - HEADER_HEIGHT - 20);

	CMovablePanel::Layout();
}

void CSceneTreePanel::UpdateTree()
{
	m_pTree->ClearTree();

	AddAllToTree();

	Layout();
}

void CSceneTreePanel::Paint(float x, float y, float w, float h)
{
	CMovablePanel::Paint(x, y, w, h);
}

void CSceneTreePanel::AddAllToTree()
{
	size_t iMaterialsNode = m_pTree->AddNode("Materials");
	CTreeNode* pMaterialsNode = m_pTree->GetNode(iMaterialsNode);
	pMaterialsNode->SetIcon(CModelWindow::Get()->GetMaterialsNodeTexture());

	size_t i;
	for (i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		size_t iMaterialNode = pMaterialsNode->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));
		CTreeNode* pMaterialNode = pMaterialsNode->GetNode(iMaterialNode);
		pMaterialNode->AddVisibilityButton();
		dynamic_cast<CTreeNodeObject<CConversionMaterial>*>(pMaterialNode)->AddEditButton(::OpenMaterialEditor);
	}

	// Don't overload the screen.
	if (pMaterialsNode->m_apNodes.size() > 10)
		pMaterialsNode->SetExpanded(false);

	size_t iMeshesNode = m_pTree->AddNode("Meshes");
	CTreeNode* pMeshesNode = m_pTree->GetNode(iMeshesNode);
	pMeshesNode->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());

	for (i = 0; i < m_pScene->GetNumMeshes(); i++)
		pMeshesNode->AddNode<CConversionMesh>(m_pScene->GetMesh(i)->GetName(), m_pScene->GetMesh(i));

	if (pMeshesNode->m_apNodes.size() > 10)
		pMeshesNode->SetExpanded(false);

	size_t iScenesNode = m_pTree->AddNode("Scenes");
	CTreeNode* pScenesNode = m_pTree->GetNode(iScenesNode);
	pScenesNode->SetIcon(CModelWindow::Get()->GetScenesNodeTexture());

	for (i = 0; i < m_pScene->GetNumScenes(); i++)
		AddNodeToTree(pScenesNode, m_pScene->GetScene(i));

	if (pScenesNode->m_apNodes.size() > 10)
		pScenesNode->SetExpanded(false);
}

void CSceneTreePanel::AddNodeToTree(glgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode)
{
	size_t iNode = pTreeNode->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		AddNodeToTree(pTreeNode->GetNode(iNode), pSceneNode->GetChild(i));

	for (size_t m = 0; m < pSceneNode->GetNumMeshInstances(); m++)
	{
		size_t iMeshInstanceNode = pTreeNode->GetNode(iNode)->AddNode<CConversionMeshInstance>(pSceneNode->GetMeshInstance(m)->GetMesh()->GetName(), pSceneNode->GetMeshInstance(m));
		CTreeNode* pMeshInstanceNode = pTreeNode->GetNode(iNode)->GetNode(iMeshInstanceNode);
		pMeshInstanceNode->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
		pMeshInstanceNode->AddVisibilityButton();
		pMeshInstanceNode->SetDraggable(true);

		for (size_t s = 0; s < pSceneNode->GetMeshInstance(m)->GetMesh()->GetNumMaterialStubs(); s++)
		{
			CConversionMaterialMap* pMaterialMap = pSceneNode->GetMeshInstance(m)->GetMappedMaterial(s);

			if (!pMaterialMap)
				continue;

			if (!m_pScene->GetMaterial(pMaterialMap->m_iMaterial))
				continue;

			size_t iMapNode = pMeshInstanceNode->AddNode<CConversionMaterialMap>(m_pScene->GetMaterial(pMaterialMap->m_iMaterial)->GetName(), pMaterialMap);
			CTreeNode* pMeshMapNode = pMeshInstanceNode->GetNode(iMapNode);
			pMeshMapNode->AddVisibilityButton();
		}
	}
}

void OpenMaterialEditor(CConversionMaterial* pMaterial, const tstring& sArgs)
{
	CSceneTreePanel::Get()->OpenMaterialEditor(pMaterial);
}

void CSceneTreePanel::OpenMaterialEditor(CConversionMaterial* pMaterial)
{
	if (m_pMaterialEditor)
		delete m_pMaterialEditor;

	m_pMaterialEditor = new CMaterialEditor(pMaterial, this);

	if (!m_pMaterialEditor)
		return;

	m_pMaterialEditor->SetVisible(true);
	m_pMaterialEditor->Layout();
}

void CSceneTreePanel::Open(CConversionScene* pScene)
{
	CSceneTreePanel* pPanel = Get();

	if (!pPanel)
		pPanel = s_pSceneTreePanel = new CSceneTreePanel(pScene);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();
}

CSceneTreePanel* CSceneTreePanel::Get()
{
	return s_pSceneTreePanel;
}

CMaterialEditor::CMaterialEditor(CConversionMaterial* pMaterial, CSceneTreePanel* pSceneTree)
	: CMovablePanel("Material Properties")
{
	m_pMaterial = pMaterial;
	m_pSceneTree = pSceneTree;

	m_pScene = CModelWindow::Get()->GetScene();

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		if (m_pScene->GetMaterial(i) == m_pMaterial)
		{
			m_iMaterial = i;
			break;
		}
	}

	float x, y;
	m_pSceneTree->GetAbsPos(x, y);

	SetPos(x + m_pSceneTree->GetWidth(), y);
	SetSize(500, 300);

	m_pName->AppendText(" - ");
	m_pName->AppendText(pMaterial->GetName().c_str());

	m_pDiffuseLabel = new CLabel(0, 0, 1, 1, "Diffuse map: ");
	AddControl(m_pDiffuseLabel);
	m_pDiffuseFile = new CButton(0, 0, 1, 1, "");
	m_pDiffuseFile->SetAlign(CLabel::TA_LEFTCENTER);
	m_pDiffuseFile->SetWrap(false);
	m_pDiffuseFile->SetClickedListener(this, ChooseDiffuse);
	AddControl(m_pDiffuseFile);
	m_pDiffuseRemove = new CButton(0, 0, 70, 20, "Remove");
	m_pDiffuseRemove->SetClickedListener(this, RemoveDiffuse);
	AddControl(m_pDiffuseRemove);

	m_pNormalLabel = new CLabel(0, 0, 1, 1, "Normal map: ");
	AddControl(m_pNormalLabel);
	m_pNormalFile = new CButton(0, 0, 1, 1, "");
	m_pNormalFile->SetAlign(CLabel::TA_LEFTCENTER);
	m_pNormalFile->SetWrap(false);
	m_pNormalFile->SetClickedListener(this, ChooseNormal);
	AddControl(m_pNormalFile);
	m_pNormalRemove = new CButton(0, 0, 70, 20, "Remove");
	m_pNormalRemove->SetClickedListener(this, RemoveNormal);
	AddControl(m_pNormalRemove);

	m_pAmbientLabel = new CLabel(0, 0, 1, 1, "Ambient: ");
	AddControl(m_pAmbientLabel);
	m_pAmbientRedSelector = new CScrollSelector<float>();
	SetupSelector(m_pAmbientRedSelector, 1);
	m_pAmbientRedSelector->SetSelectedListener(this, SetAmbientRed);
	AddControl(m_pAmbientRedSelector);
	m_pAmbientGreenSelector = new CScrollSelector<float>();
	SetupSelector(m_pAmbientGreenSelector, 1);
	m_pAmbientGreenSelector->SetSelectedListener(this, SetAmbientGreen);
	AddControl(m_pAmbientGreenSelector);
	m_pAmbientBlueSelector = new CScrollSelector<float>();
	SetupSelector(m_pAmbientBlueSelector, 1);
	m_pAmbientBlueSelector->SetSelectedListener(this, SetAmbientBlue);
	AddControl(m_pAmbientBlueSelector);

	m_pDiffuseSelectorLabel = new CLabel(0, 0, 1, 1, "Diffuse: ");
	AddControl(m_pDiffuseSelectorLabel);
	m_pDiffuseRedSelector = new CScrollSelector<float>();
	SetupSelector(m_pDiffuseRedSelector, 1);
	m_pDiffuseRedSelector->SetSelectedListener(this, SetDiffuseRed);
	AddControl(m_pDiffuseRedSelector);
	m_pDiffuseGreenSelector = new CScrollSelector<float>();
	SetupSelector(m_pDiffuseGreenSelector, 1);
	m_pDiffuseGreenSelector->SetSelectedListener(this, SetDiffuseGreen);
	AddControl(m_pDiffuseGreenSelector);
	m_pDiffuseBlueSelector = new CScrollSelector<float>();
	SetupSelector(m_pDiffuseBlueSelector, 1);
	m_pDiffuseBlueSelector->SetSelectedListener(this, SetDiffuseBlue);
	AddControl(m_pDiffuseBlueSelector);

	m_pSpecularLabel = new CLabel(0, 0, 1, 1, "Specular: ");
	AddControl(m_pSpecularLabel);
	m_pSpecularRedSelector = new CScrollSelector<float>();
	SetupSelector(m_pSpecularRedSelector, 1);
	m_pSpecularRedSelector->SetSelectedListener(this, SetSpecularRed);
	AddControl(m_pSpecularRedSelector);
	m_pSpecularGreenSelector = new CScrollSelector<float>();
	SetupSelector(m_pSpecularGreenSelector, 1);
	m_pSpecularGreenSelector->SetSelectedListener(this, SetSpecularGreen);
	AddControl(m_pSpecularGreenSelector);
	m_pSpecularBlueSelector = new CScrollSelector<float>();
	SetupSelector(m_pSpecularBlueSelector, 1);
	m_pSpecularBlueSelector->SetSelectedListener(this, SetSpecularBlue);
	AddControl(m_pSpecularBlueSelector);

	m_pEmissiveLabel = new CLabel(0, 0, 1, 1, "Emissive: ");
	AddControl(m_pEmissiveLabel);
	m_pEmissiveRedSelector = new CScrollSelector<float>();
	SetupSelector(m_pEmissiveRedSelector, 1);
	m_pEmissiveRedSelector->SetSelectedListener(this, SetEmissiveRed);
	AddControl(m_pEmissiveRedSelector);
	m_pEmissiveGreenSelector = new CScrollSelector<float>();
	SetupSelector(m_pEmissiveGreenSelector, 1);
	m_pEmissiveGreenSelector->SetSelectedListener(this, SetEmissiveGreen);
	AddControl(m_pEmissiveGreenSelector);
	m_pEmissiveBlueSelector = new CScrollSelector<float>();
	SetupSelector(m_pEmissiveBlueSelector, 1);
	m_pEmissiveBlueSelector->SetSelectedListener(this, SetEmissiveBlue);
	AddControl(m_pEmissiveBlueSelector);

	m_pShininessLabel = new CLabel(0, 0, 1, 1, "Shininess: ");
	AddControl(m_pShininessLabel);
	m_pShininessSelector = new CScrollSelector<float>();
	SetupSelector(m_pShininessSelector, 128);
	m_pShininessSelector->SetSelectedListener(this, SetShininess);
	AddControl(m_pShininessSelector);

	Layout();
}

void CMaterialEditor::SetupSelector(CScrollSelector<float>* pSelector, float flMaxValue)
{
	pSelector->AddSelection(CScrollSelection<float>(0*flMaxValue/20, "0%"));
	pSelector->AddSelection(CScrollSelection<float>(1*flMaxValue/20, "5%"));
	pSelector->AddSelection(CScrollSelection<float>(2*flMaxValue/20, "10%"));
	pSelector->AddSelection(CScrollSelection<float>(3*flMaxValue/20, "15%"));
	pSelector->AddSelection(CScrollSelection<float>(4*flMaxValue/20, "20%"));
	pSelector->AddSelection(CScrollSelection<float>(5*flMaxValue/20, "25%"));
	pSelector->AddSelection(CScrollSelection<float>(6*flMaxValue/20, "30%"));
	pSelector->AddSelection(CScrollSelection<float>(7*flMaxValue/20, "35%"));
	pSelector->AddSelection(CScrollSelection<float>(8*flMaxValue/20, "40%"));
	pSelector->AddSelection(CScrollSelection<float>(9*flMaxValue/20, "45%"));
	pSelector->AddSelection(CScrollSelection<float>(10*flMaxValue/20, "50%"));
	pSelector->AddSelection(CScrollSelection<float>(11*flMaxValue/20, "55%"));
	pSelector->AddSelection(CScrollSelection<float>(12*flMaxValue/20, "60%"));
	pSelector->AddSelection(CScrollSelection<float>(13*flMaxValue/20, "65%"));
	pSelector->AddSelection(CScrollSelection<float>(14*flMaxValue/20, "70%"));
	pSelector->AddSelection(CScrollSelection<float>(15*flMaxValue/20, "75%"));
	pSelector->AddSelection(CScrollSelection<float>(16*flMaxValue/20, "80%"));
	pSelector->AddSelection(CScrollSelection<float>(17*flMaxValue/20, "85%"));
	pSelector->AddSelection(CScrollSelection<float>(18*flMaxValue/20, "90%"));
	pSelector->AddSelection(CScrollSelection<float>(19*flMaxValue/20, "95%"));
	pSelector->AddSelection(CScrollSelection<float>(20*flMaxValue/20, "100%"));
}

void CMaterialEditor::Layout()
{
	float flHeight = HEADER_HEIGHT+10;

	m_pDiffuseLabel->SetPos(10, flHeight);
	m_pDiffuseLabel->EnsureTextFits();

	float x, y;
	m_pDiffuseLabel->GetPos(x, y);

	float flControlHeight = y;

	float flDiffuseRight = x + m_pDiffuseLabel->GetWidth();

	m_pDiffuseFile->SetPos(flDiffuseRight, flHeight);
	if (m_pMaterial->GetDiffuseTexture().length())
		m_pDiffuseFile->SetText(m_pMaterial->GetDiffuseTexture().c_str());
	else
		m_pDiffuseFile->SetText("Choose...");
	m_pDiffuseFile->SetSize(0, 0);
	m_pDiffuseFile->EnsureTextFits();
	if (m_pDiffuseFile->GetWidth() + m_pDiffuseLabel->GetWidth() + 10 > GetWidth())
		m_pDiffuseFile->SetSize(GetWidth() - m_pDiffuseLabel->GetWidth() - 10, m_pDiffuseFile->GetHeight());

	flHeight += flControlHeight;

	m_pDiffuseRemove->SetPos(GetWidth() - m_pDiffuseRemove->GetWidth() - 10, flHeight);

	flHeight += flControlHeight;

	m_pNormalLabel->SetPos(10, flHeight);
	m_pNormalLabel->EnsureTextFits();

	m_pNormalLabel->GetPos(x, y);
	float flNormalRight = x + m_pNormalLabel->GetWidth();

	m_pNormalFile->SetPos(flNormalRight, flHeight);
	if (m_pMaterial->GetNormalTexture().length())
		m_pNormalFile->SetText(m_pMaterial->GetNormalTexture().c_str());
	else
		m_pNormalFile->SetText("Choose...");
	m_pNormalFile->SetSize(0, 0);
	m_pNormalFile->EnsureTextFits();
	if (m_pNormalFile->GetWidth() + m_pNormalLabel->GetWidth() + 10 > GetWidth())
		m_pNormalFile->SetSize(GetWidth() - m_pNormalLabel->GetWidth() - 10, m_pNormalFile->GetHeight());

	flHeight += flControlHeight;

	m_pNormalRemove->SetPos(GetWidth() - m_pNormalRemove->GetWidth() - 10, flHeight);

	flHeight += flControlHeight;

	float flAmbientHeight = flHeight;

	m_pAmbientLabel->SetPos(10, flHeight);
	m_pAmbientLabel->EnsureTextFits();

	m_pAmbientLabel->GetPos(x, y);
	float flAmbientRight = x + m_pAmbientLabel->GetWidth();

	m_pAmbientRedSelector->SetPos(flAmbientRight, flHeight);
	m_pAmbientRedSelector->SetSelection((int)(m_pMaterial->m_vecAmbient.x*20));
	m_pAmbientRedSelector->SetRight(GetWidth()/2-5);

	flHeight += 20;
	m_pAmbientGreenSelector->SetPos(flAmbientRight, flHeight);
	m_pAmbientGreenSelector->SetSelection((int)(m_pMaterial->m_vecAmbient.y*20));
	m_pAmbientGreenSelector->SetRight(GetWidth()/2-5);

	flHeight += 20;
	m_pAmbientBlueSelector->SetPos(flAmbientRight, flHeight);
	m_pAmbientBlueSelector->SetSelection((int)(m_pMaterial->m_vecAmbient.z*20));
	m_pAmbientBlueSelector->SetRight(GetWidth()/2-5);

	flHeight = flAmbientRight;

	m_pDiffuseSelectorLabel->SetPos(GetWidth()/2+5, flHeight);
	m_pDiffuseSelectorLabel->EnsureTextFits();

	m_pDiffuseSelectorLabel->GetPos(x, y);
	float flDiffuseSelectorX = x + m_pDiffuseSelectorLabel->GetWidth();

	m_pDiffuseRedSelector->SetPos(flDiffuseSelectorX, flHeight);
	m_pDiffuseRedSelector->SetSelection((int)(m_pMaterial->m_vecDiffuse.x*20));
	m_pDiffuseRedSelector->SetRight(GetWidth()-10);

	flHeight += 20;
	m_pDiffuseGreenSelector->SetPos(flDiffuseSelectorX, flHeight);
	m_pDiffuseGreenSelector->SetSelection((int)(m_pMaterial->m_vecDiffuse.y*20));
	m_pDiffuseGreenSelector->SetRight(GetWidth()-10);

	flHeight += 20;
	m_pDiffuseBlueSelector->SetPos(flDiffuseSelectorX, flHeight);
	m_pDiffuseBlueSelector->SetSelection((int)(m_pMaterial->m_vecDiffuse.z*20));
	m_pDiffuseBlueSelector->SetRight(GetWidth()-10);

	flHeight += flControlHeight;

	float flSpecularHeight = flHeight;

	m_pSpecularLabel->SetPos(10, flHeight);
	m_pSpecularLabel->EnsureTextFits();

	m_pSpecularLabel->GetPos(x, y);
	float flSpecularRight = x + m_pSpecularLabel->GetWidth();

	m_pSpecularRedSelector->SetPos(flSpecularRight, flHeight);
	m_pSpecularRedSelector->SetSelection((int)(m_pMaterial->m_vecSpecular.x*20));
	m_pSpecularRedSelector->SetRight(GetWidth()/2-5);

	flHeight += 20;
	m_pSpecularGreenSelector->SetPos(flSpecularRight, flHeight);
	m_pSpecularGreenSelector->SetSelection((int)(m_pMaterial->m_vecSpecular.y*20));
	m_pSpecularGreenSelector->SetRight(GetWidth()/2-5);

	flHeight += 20;
	m_pSpecularBlueSelector->SetPos(flSpecularRight, flHeight);
	m_pSpecularBlueSelector->SetSelection((int)(m_pMaterial->m_vecSpecular.z*20));
	m_pSpecularBlueSelector->SetRight(GetWidth()/2-5);

	flHeight = flSpecularHeight;

	m_pEmissiveLabel->SetPos(GetWidth()/2+5, flHeight);
	m_pEmissiveLabel->EnsureTextFits();

	m_pEmissiveLabel->GetPos(x, y);
	float flEmissiveRight = x + m_pEmissiveLabel->GetWidth();

	m_pEmissiveRedSelector->SetPos(flEmissiveRight, flHeight);
	m_pEmissiveRedSelector->SetSelection((int)(m_pMaterial->m_vecEmissive.x*20));
	m_pEmissiveRedSelector->SetRight(GetWidth()-10);

	flHeight += 20;
	m_pEmissiveGreenSelector->SetPos(flEmissiveRight, flHeight);
	m_pEmissiveGreenSelector->SetSelection((int)(m_pMaterial->m_vecEmissive.y*20));
	m_pEmissiveGreenSelector->SetRight(GetWidth()-10);

	flHeight += 20;
	m_pEmissiveBlueSelector->SetPos(flEmissiveRight, flHeight);
	m_pEmissiveBlueSelector->SetSelection((int)(m_pMaterial->m_vecEmissive.z*20));
	m_pEmissiveBlueSelector->SetRight(GetWidth()-10);

	flHeight += flControlHeight;

	m_pShininessLabel->SetPos(10, flHeight);
	m_pShininessLabel->EnsureTextFits();

	m_pShininessLabel->GetPos(x, y);
	float flShininessRight = x + m_pShininessLabel->GetWidth();

	m_pShininessSelector->SetPos(flShininessRight, flHeight);
	m_pShininessSelector->SetSelection((int)(m_pMaterial->m_flShininess/127*20));
	m_pShininessSelector->SetRight(GetWidth()/2-5);

	CMovablePanel::Layout();
}

void CMaterialEditor::ChooseDiffuseCallback(const tstring& sArgs)
{
	CFileDialog::ShowOpenDialog("", ".bmp;.jpg;.png;.tga;.psd;.gif;.tif", this, OpenDiffuse);
}

void CMaterialEditor::OpenDiffuseCallback(const tstring& sArgs)
{
	tstring sOpen = sArgs;

	if (!sOpen.length())
		return;

	size_t iTexture = CModelWindow::LoadTextureIntoGL(sOpen);

	if (!iTexture)
		return;

	CMaterial* pMaterial = &(*CModelWindow::Get()->GetMaterials())[m_iMaterial];

	if (pMaterial->m_iBase)
		glDeleteTextures(1, &pMaterial->m_iBase);

	pMaterial->m_iBase = iTexture;
	m_pMaterial->m_sDiffuseTexture = sOpen;

	Layout();
}

void CMaterialEditor::ChooseNormalCallback(const tstring& sArgs)
{
	CFileDialog::ShowOpenDialog("", ".bmp;.jpg;.png;.tga;.psd;.gif;.tif", this, OpenNormal);
}

void CMaterialEditor::OpenNormalCallback(const tstring& sArgs)
{
	tstring sOpen = sArgs;

	if (!sOpen.length())
		return;

	size_t iTexture = CModelWindow::LoadTextureIntoGL(sOpen);

	if (!iTexture)
		return;

	CMaterial* pMaterial = &(*CModelWindow::Get()->GetMaterials())[m_iMaterial];

	if (pMaterial->m_iNormal)
		glDeleteTextures(1, &pMaterial->m_iNormal);

	pMaterial->m_iNormal = iTexture;
	m_pMaterial->m_sNormalTexture = sOpen;

	if (pMaterial->m_iNormal2)
		glDeleteTextures(1, &pMaterial->m_iNormal2);
	pMaterial->m_iNormal2 = 0;

	if (pMaterial->m_iNormal2IL)
		ilDeleteImages(1, &pMaterial->m_iNormal2IL);
	if (pMaterial->m_iNormalIL)
		ilDeleteImages(1, &pMaterial->m_iNormalIL);

	pMaterial->m_iNormal2IL = 0;
	pMaterial->m_iNormalIL = 0;

	Layout();
}

void CMaterialEditor::RemoveDiffuseCallback(const tstring& sArgs)
{
	CMaterial* pMaterial = &(*CModelWindow::Get()->GetMaterials())[m_iMaterial];

	if (pMaterial->m_iBase)
		glDeleteTextures(1, &pMaterial->m_iBase);

	pMaterial->m_iBase = 0;
	m_pMaterial->m_sDiffuseTexture = "";

	Layout();
}

void CMaterialEditor::RemoveNormalCallback(const tstring& sArgs)
{
	CMaterial* pMaterial = &(*CModelWindow::Get()->GetMaterials())[m_iMaterial];

	if (pMaterial->m_iNormal)
		glDeleteTextures(1, &pMaterial->m_iNormal);

	pMaterial->m_iNormal = 0;
	m_pMaterial->m_sNormalTexture = "";

	if (pMaterial->m_iNormal2)
		glDeleteTextures(1, &pMaterial->m_iNormal2);
	pMaterial->m_iNormal2 = 0;

	if (pMaterial->m_iNormal2IL)
		ilDeleteImages(1, &pMaterial->m_iNormal2IL);
	if (pMaterial->m_iNormalIL)
		ilDeleteImages(1, &pMaterial->m_iNormalIL);

	pMaterial->m_iNormal2IL = 0;
	pMaterial->m_iNormalIL = 0;

	Layout();
}

void CMaterialEditor::SetAmbientRedCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecAmbient.x = m_pAmbientRedSelector->GetSelectionValue();
}

void CMaterialEditor::SetAmbientGreenCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecAmbient.y = m_pAmbientGreenSelector->GetSelectionValue();
}

void CMaterialEditor::SetAmbientBlueCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecAmbient.z = m_pAmbientBlueSelector->GetSelectionValue();
}

void CMaterialEditor::SetDiffuseRedCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecDiffuse.x = m_pDiffuseRedSelector->GetSelectionValue();
}

void CMaterialEditor::SetDiffuseGreenCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecDiffuse.y = m_pDiffuseGreenSelector->GetSelectionValue();
}

void CMaterialEditor::SetDiffuseBlueCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecDiffuse.z = m_pDiffuseBlueSelector->GetSelectionValue();
}

void CMaterialEditor::SetSpecularRedCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecSpecular.x = m_pSpecularRedSelector->GetSelectionValue();
}

void CMaterialEditor::SetSpecularGreenCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecSpecular.y = m_pSpecularGreenSelector->GetSelectionValue();
}

void CMaterialEditor::SetSpecularBlueCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecSpecular.z = m_pSpecularBlueSelector->GetSelectionValue();
}

void CMaterialEditor::SetEmissiveRedCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecEmissive.x = m_pEmissiveRedSelector->GetSelectionValue();
}

void CMaterialEditor::SetEmissiveGreenCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecEmissive.y = m_pEmissiveGreenSelector->GetSelectionValue();
}

void CMaterialEditor::SetEmissiveBlueCallback(const tstring& sArgs)
{
	m_pMaterial->m_vecEmissive.z = m_pEmissiveBlueSelector->GetSelectionValue();
}

void CMaterialEditor::SetShininessCallback(const tstring& sArgs)
{
	m_pMaterial->m_flShininess = m_pShininessSelector->GetSelectionValue();
}
