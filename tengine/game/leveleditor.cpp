#include "leveleditor.h"

#include <tinker_platform.h>
#include <files.h>

#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <tinker/profiler.h>
#include <textures/texturelibrary.h>
#include <tinker/keys.h>

#include "level.h"
#include "gameserver.h"

CCreateEntityPanel::CCreateEntityPanel()
	: glgui::CMovablePanel("Create Entity Tool")
{
	m_pClass = new glgui::CMenu("Choose Class");

	for (size_t i = 0; i < CBaseEntity::GetNumEntitiesRegistered(); i++)
	{
		CEntityRegistration* pRegistration = CBaseEntity::GetEntityRegistration(i);

		if (!pRegistration->m_bCreatableInEditor)
			continue;

		m_pClass->AddSubmenu(pRegistration->m_pszEntityClass+1, this, ChooseClass);
	}

	AddControl(m_pClass);

	m_pNameLabel = new glgui::CLabel("Name:", "sans-serif", 10);
	m_pNameLabel->SetAlign(glgui::CLabel::TA_LEFTCENTER);
	AddControl(m_pNameLabel);
	m_pNameText = new glgui::CTextField();
	AddControl(m_pNameText);

	m_pModelLabel = new glgui::CLabel("Model:", "sans-serif", 10);
	m_pModelLabel->SetAlign(glgui::CLabel::TA_LEFTCENTER);
	AddControl(m_pModelLabel);
	m_pModelText = new glgui::CTextField();
	m_pModelText->SetContentsChangedListener(this, ModelChanged);
	AddControl(m_pModelText);

	m_bReadyToCreate = false;
}

void CCreateEntityPanel::Layout()
{
	m_pClass->SetWidth(100);
	m_pClass->SetHeight(30);
	m_pClass->CenterX();
	m_pClass->SetTop(30);

	m_pNameLabel->SetLeft(15);
	m_pNameLabel->SetTop(70);
	m_pNameText->SetWidth(GetWidth()-30);
	m_pNameText->CenterX();
	m_pNameText->SetTop(85);

	m_pModelLabel->SetLeft(15);
	m_pModelLabel->SetTop(110);
	m_pModelText->SetWidth(GetWidth()-30);
	m_pModelText->CenterX();
	m_pModelText->SetTop(125);

	SetHeight(170);

	BaseClass::Layout();
}

void CCreateEntityPanel::ChooseClassCallback(const tstring& sArgs)
{
	eastl::vector<tstring> asTokens;
	strtok(sArgs, asTokens);

	m_pClass->SetText(asTokens[1]);
	m_pClass->Pop(true, true);

	m_bReadyToCreate = true;
}

void CCreateEntityPanel::ModelChangedCallback(const tstring& sArgs)
{
	if (!m_pModelText->GetText().length())
		return;

	tstring sGameFolder = FindAbsolutePath(".");
	tstring sInputFolder = FindAbsolutePath(m_pModelText->GetText());

	if (sInputFolder.compare(0, sGameFolder.length(), sGameFolder) != 0)
		return;

	tstring sSearchDirectory = GetDirectory(sInputFolder);

	tstring sPrefix = ToForwardSlashes(sSearchDirectory.substr(sGameFolder.length()));
	while (sPrefix[0] == '/')
		sPrefix = sPrefix.substr(1);
	while (sPrefix.back() == '/')
		sPrefix = sPrefix.substr(0, sPrefix.length()-2);
	if (sPrefix.length())
		sPrefix = sPrefix + '/';

	eastl::vector<tstring> asFiles = ListDirectory(sSearchDirectory);

	for (size_t i = 0; i < asFiles.size(); i++)
		asFiles[i] = sPrefix + asFiles[i];

	m_pModelText->SetAutoCompleteCommands(asFiles);
}

CEditorPanel::CEditorPanel()
{
	m_pEntities = new glgui::CTree(0, 0, 0);
	m_pEntities->SetBackgroundColor(Color(0, 0, 0, 100));
	m_pEntities->SetSelectedListener(this, EntitySelected);
	AddControl(m_pEntities);

	m_pObjectTitle = new glgui::CLabel("", "sans-serif", 20);
	AddControl(m_pObjectTitle);
}

void CEditorPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flCurrLeft = 20;
	float flCurrTop = 20;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-40);

	m_pEntities->SetPos(10, 10);
	m_pEntities->SetSize(GetWidth() - 20, 200);

	m_pEntities->ClearTree();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (pLevel)
	{
		auto aEntities = pLevel->GetEntityData();
		for (size_t i = 0; i < aEntities.size(); i++)
		{
			auto oEntity = aEntities[i];

			tstring sName = oEntity.GetParameterValue("Name");

			if (sName.length())
				m_pEntities->AddNode(oEntity.m_sClass + ": " + oEntity.GetParameterValue("Name"));
			else
				m_pEntities->AddNode(oEntity.m_sClass);
		}
	}

	m_pObjectTitle->SetPos(0, 220);
	m_pObjectTitle->SetSize(GetWidth(), 25);

	LayoutEntities();

	BaseClass::Layout();
}

void CEditorPanel::LayoutEntities()
{
	m_pObjectTitle->SetText("(No Object Selected)");

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	auto aEntities = pLevel->GetEntityData();

	if (m_pEntities->GetSelectedNodeId() < aEntities.size())
	{
		CLevelEntity* pEntity = &aEntities[m_pEntities->GetSelectedNodeId()];
		if (pEntity->GetName().length())
			m_pObjectTitle->SetText(pEntity->m_sClass + ": " + pEntity->GetName());
		else
			m_pObjectTitle->SetText(pEntity->m_sClass);
	}
}

void CEditorPanel::EntitySelectedCallback(const tstring& sArgs)
{
	LayoutEntities();

	LevelEditor()->EntitySelected();
}

void CEditorCamera::Think()
{
	BaseClass::Think();

	if (m_bFreeMode)
	{
		m_vecEditCamera = m_vecFreeCamera;
		m_angEditCamera = m_angFreeCamera;
	}
}

TVector CEditorCamera::GetCameraPosition()
{
	return m_vecEditCamera;
}

TVector CEditorCamera::GetCameraDirection()
{
	return AngleVector(m_angEditCamera);
}

void CEditorCamera::SetCameraOrientation(TVector vecPosition, Vector vecDirection)
{
	m_vecEditCamera = vecPosition;
	m_angEditCamera = VectorAngles(vecDirection);
}

CLevelEditor::CLevelEditor()
{
	m_pLevel = nullptr;

	m_bActive = false;
	m_pEditorPanel = new CEditorPanel();
	m_pEditorPanel->SetVisible(false);
	m_pEditorPanel->SetBackgroundColor(Color(0, 0, 0, 150));
	m_pEditorPanel->SetBorder(glgui::CPanel::BT_SOME);
	glgui::CRootPanel::Get()->AddControl(m_pEditorPanel);

	m_pCreateEntityButton = new glgui::CPictureButton("Create", CTextureLibrary::AddTextureID("editor/create-entity.png"));
	m_pCreateEntityButton->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pCreateEntityButton->GetWidth()/2, 20);
	m_pCreateEntityButton->SetClickedListener(this, CreateEntity);
	m_pCreateEntityButton->SetTooltip("Create Entity Tool");
	glgui::CRootPanel::Get()->AddControl(m_pCreateEntityButton, true);

	m_pCreateEntityPanel = new CCreateEntityPanel();
	m_pCreateEntityPanel->SetBackgroundColor(Color(0, 0, 0, 255));
	m_pCreateEntityPanel->SetHeaderColor(Color(100, 100, 100, 255));
	m_pCreateEntityPanel->SetBorder(glgui::CPanel::BT_SOME);
	m_pCreateEntityPanel->SetVisible(false);

	m_pCamera = new CEditorCamera();

	m_flCreateObjectDistance = 10;
}

CLevelEditor::~CLevelEditor()
{
	glgui::CRootPanel::Get()->RemoveControl(m_pEditorPanel);
	delete m_pEditorPanel;

	delete m_pCamera;

	glgui::CRootPanel::Get()->RemoveControl(m_pCreateEntityButton);
	delete m_pCreateEntityButton;
	delete m_pCreateEntityPanel;
}

void CLevelEditor::RenderEntity(size_t i, bool bTransparent)
{
	CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];
	RenderEntity(pEntity, bTransparent, m_pEditorPanel->m_pEntities->GetSelectedNodeId() == i);
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bTransparent, bool bSelected)
{
	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	// If another context already set this, don't clobber it.
	if (!r.GetActiveFrameBuffer())
		r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

	r.Transform(pEntity->GetGlobalTransform());

	if (!pEntity->IsVisible())
	{
		r.SetBlend(BLEND_ALPHA);
		r.SetAlpha(0.5f);
	}

	if (pEntity->GetModelID() != ~0)
	{
		if (bSelected)
			r.SetColor(Color(255, 0, 0));
		else
			r.SetColor(Color(255, 255, 255));

		if (r.GetBlend() == BLEND_NONE && !bTransparent)
		{
			TPROF("CLevelEditor::RenderEntity(Opaque)");
			r.RenderModel(pEntity->GetModelID(), nullptr);
		}
		else if (r.GetBlend() != BLEND_NONE && bTransparent)
		{
			TPROF("CLevelEditor::RenderEntity(Transparent)");
			r.RenderModel(pEntity->GetModelID(), nullptr);
		}
	}
	else if (pEntity->GetTextureModelID() != (size_t)0)
	{
		if (bTransparent)
		{
			TPROF("CLevelEditor::RenderModel(Texture)");
			r.UseProgram("model");
			r.SetUniform("bDiffuse", true);
			if (bSelected)
				r.SetUniform("vecColor", Color(255, 0, 0));
			else
				r.SetUniform("vecColor", Color(255, 255, 255));

			r.SetBlend(BLEND_ALPHA);
			r.Scale(0, pEntity->m_vecTextureModelScale.Get().y, pEntity->m_vecTextureModelScale.Get().x);
			r.RenderTextureModel(pEntity->GetTextureModelID());
		}
	}
	else
	{
		r.UseProgram("model");
		if (bSelected)
			r.SetUniform("vecColor", Color(255, 0, 0));
		else
			r.SetUniform("vecColor", Color(255, 255, 255));
		r.SetUniform("bDiffuse", false);
		r.RenderWireBox(pEntity->GetBoundingBox());
	}
}

void CLevelEditor::RenderCreateEntityPreview()
{
	CLevelEntity oRenderEntity;
	oRenderEntity.m_sClass = m_pCreateEntityPanel->m_pClass->GetText();
	oRenderEntity.m_sName = m_pCreateEntityPanel->m_pNameText->GetText();
	oRenderEntity.m_asParameters["Model"] = m_pCreateEntityPanel->m_pModelText->GetText();

	oRenderEntity.m_mGlobalTransform = Matrix4x4(EAngle(0, 0, 0), PositionFromMouse());
	RenderEntity(&oRenderEntity, true);
}

Vector CLevelEditor::PositionFromMouse()
{
	int x, y;
	Application()->GetMousePosition(x, y);
	Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

	Vector vecCameraDirection = GameServer()->GetRenderer()->GetCameraDirection();
	if (vecCameraDirection.Dot(vecPosition-vecCamera) < 0)
		vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, -1));

	return vecCamera + (vecPosition - vecCamera).Normalized() * m_flCreateObjectDistance;
}

void CLevelEditor::EntitySelected()
{
	size_t iSelected = m_pEditorPanel->m_pEntities->GetSelectedNodeId();
	auto& aEntities = m_pLevel->GetEntityData();

	if (iSelected >= aEntities.size())
		return;

	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();
	m_flCreateObjectDistance = (vecCamera - aEntities[iSelected].GetGlobalTransform().GetTranslation()).Length();
}

void CLevelEditor::CreateEntityCallback(const tstring& sArgs)
{
	m_pCreateEntityPanel->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pCreateEntityPanel->GetWidth()/2, 72);
	m_pCreateEntityPanel->SetVisible(true);
}

bool CLevelEditor::KeyPress(int c)
{
	if (c == TINKER_KEY_DEL)
	{
		size_t iSelected = m_pEditorPanel->m_pEntities->GetSelectedNodeId();
		auto& aEntities = m_pLevel->GetEntityData();

		m_pEditorPanel->m_pEntities->Unselect();
		if (iSelected < aEntities.size())
		{
			aEntities.erase(aEntities.begin()+iSelected);
			m_pEditorPanel->Layout();
			return true;
		}
	}

	return false;
}

bool CLevelEditor::MouseInput(int iButton, int iState)
{
	if (iState == 1 && m_pCreateEntityPanel->IsVisible() && m_pCreateEntityPanel->m_bReadyToCreate)
	{
		auto& aEntityData = m_pLevel->GetEntityData();
		auto& oNewEntity = aEntityData.push_back();
		oNewEntity.m_sClass = m_pCreateEntityPanel->m_pClass->GetText();
		oNewEntity.m_sName = m_pCreateEntityPanel->m_pNameText->GetText();

		oNewEntity.m_asParameters["Model"] = m_pCreateEntityPanel->m_pModelText->GetText();

		Vector vecOrigin = PositionFromMouse();
		oNewEntity.m_asParameters["LocalOrigin"] = sprintf("%f %f %f", vecOrigin.x, vecOrigin.y, vecOrigin.z);

		m_pEditorPanel->Layout();
		m_pEditorPanel->m_pEntities->SetSelectedNode(aEntityData.size()-1);
		return true;
	}

	return false;
}

void CLevelEditor::Toggle()
{
	if (!LevelEditor())
		return;

	if (IsActive())
		LevelEditor()->Deactivate();
	else
		LevelEditor()->Activate();
}

bool CLevelEditor::IsActive()
{
	if (!LevelEditor(false))
		return false;

	return LevelEditor()->m_bActive;
}

void CLevelEditor::Activate()
{
	if (!LevelEditor())
		return;

	LevelEditor()->m_pCamera->SetCameraOrientation(GameServer()->GetCamera()->GetCameraPosition(), GameServer()->GetCamera()->GetCameraDirection());

	LevelEditor()->m_bActive = true;

	LevelEditor()->m_pLevel = GameServer()->GetLevel(CVar::GetCVarValue("game_level"));

	LevelEditor()->m_pEditorPanel->SetVisible(true);
	LevelEditor()->m_pCreateEntityButton->SetVisible(true);

	LevelEditor()->m_bWasMouseActive = Application()->IsMouseCursorEnabled();
	Application()->SetMouseCursorEnabled(true);
}

void CLevelEditor::Deactivate()
{
	if (!LevelEditor())
		return;

	LevelEditor()->m_bActive = false;

	LevelEditor()->m_pEditorPanel->SetVisible(false);
	LevelEditor()->m_pCreateEntityButton->SetVisible(false);
	LevelEditor()->m_pCreateEntityPanel->SetVisible(false);

	Application()->SetMouseCursorEnabled(LevelEditor()->m_bWasMouseActive);

	if (LevelEditor()->m_pLevel && LevelEditor()->m_pLevel->GetEntityData().size())
		GameServer()->RestartLevel();
}

void CLevelEditor::RenderEntities()
{
	if (!LevelEditor())
		return;

	if (!IsActive())
		return;

	if (!LevelEditor()->m_pLevel)
		return;

	TPROF("CLevelEditor::RenderEntities()");

	auto aEntityData = LevelEditor()->m_pLevel->GetEntityData();
	for (size_t i = 0; i < aEntityData.size(); i++)
	{
		LevelEditor()->RenderEntity(i, false);
	}

	for (size_t i = 0; i < aEntityData.size(); i++)
	{
		LevelEditor()->RenderEntity(i, true);
	}

	if (LevelEditor()->m_pCreateEntityPanel->IsVisible() && LevelEditor()->m_pCreateEntityPanel->m_bReadyToCreate)
	{
		LevelEditor()->RenderCreateEntityPreview();
	}
}

void CLevelEditor::Render()
{
	if (!IsActive())
		return;
}

CCamera* CLevelEditor::GetCamera()
{
	return LevelEditor()->m_pCamera;
}

CLevelEditor* LevelEditor(bool bCreate)
{
	// This function won't work unless we're in dev mode.
	// I don't want memory wasted on the level editor for most players.
	if (!CVar::GetCVarBool("developer"))
		return nullptr;

	static bool bCreated = false;

	if (!bCreated && !bCreate)
		return nullptr;

	static CLevelEditor* pLevelEditor = new CLevelEditor();
	bCreated = true;

	return pLevelEditor;
}
