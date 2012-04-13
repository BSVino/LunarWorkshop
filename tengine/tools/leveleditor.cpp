#include "leveleditor.h"

#include <tinker_platform.h>
#include <files.h>

#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>
#include <glgui/checkbox.h>
#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <tinker/profiler.h>
#include <textures/materiallibrary.h>
#include <tinker/keys.h>
#include <game/level.h>
#include <game/gameserver.h>
#include <models/models.h>

#include "workbench.h"
#include "manipulator.h"

CEntityPropertiesPanel::CEntityPropertiesPanel(bool bCommon)
{
	SetVerticalScrollBarEnabled(true);
	SetScissoring(true);
	m_bCommonProperties = bCommon;
	m_pEntity = nullptr;
	m_pPropertyChangedListener = nullptr;
}

void CEntityPropertiesPanel::Layout()
{
	if (!m_sClass.length())
		return;

	float flTop = 5;

	TAssert(m_apPropertyLabels.size() == m_apPropertyOptions.size());
	for (size_t i = 0; i < m_apPropertyLabels.size(); i++)
	{
		RemoveControl(m_apPropertyLabels[i]);
		RemoveControl(m_apPropertyOptions[i]);

		delete m_apPropertyLabels[i];
		delete m_apPropertyOptions[i];
	}
	m_apPropertyLabels.clear();
	m_apPropertyOptions.clear();
	m_asPropertyHandle.clear();

	// If we're ready to create then a class has been chosen.
	const tchar* pszClassName = m_sClass.c_str();
	CEntityRegistration* pRegistration = NULL;
	eastl::map<tstring, bool> abHandlesSet;

	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			auto pSaveData = &pRegistration->m_aSaveData[i];

			if (!pSaveData->m_pszHandle || !pSaveData->m_pszHandle[0])
				continue;

			if (!pSaveData->m_bShowInEditor)
				continue;

			if (!m_bCommonProperties)
			{
				if (strcmp(pSaveData->m_pszHandle, "Name") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "Model") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "LocalOrigin") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "LocalAngles") == 0)
					continue;
			}

			if (abHandlesSet.find(tstring(pSaveData->m_pszHandle)) != abHandlesSet.end())
				continue;

			abHandlesSet[tstring(pSaveData->m_pszHandle)] = true;

			m_asPropertyHandle.push_back(tstring(pSaveData->m_pszHandle));

			m_apPropertyLabels.push_back(new glgui::CLabel(tstring(pSaveData->m_pszHandle) + ": ", "sans-serif", 10));
			m_apPropertyLabels.back()->SetAlign(glgui::CLabel::TA_TOPLEFT);
			AddControl(m_apPropertyLabels.back());
			m_apPropertyLabels.back()->SetLeft(15);
			m_apPropertyLabels.back()->SetTop(flTop);
			m_apPropertyLabels.back()->SetWidth(10);
			m_apPropertyLabels.back()->EnsureTextFits();

			if (strcmp(pSaveData->m_pszType, "bool") == 0)
			{
				glgui::CCheckBox* pCheckbox = new glgui::CCheckBox();
				m_apPropertyOptions.push_back(pCheckbox);
				AddControl(pCheckbox);
				pCheckbox->SetLeft(m_apPropertyLabels.back()->GetRight() + 10);
				pCheckbox->SetTop(flTop);
				pCheckbox->SetSize(12, 12);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
					pCheckbox->SetState(UnserializeString_bool(m_pEntity->GetParameterValue(pSaveData->m_pszHandle)));
				else if (pSaveData->m_bDefault)
					pCheckbox->SetState(!!pSaveData->m_oDefault[0], false);

				pCheckbox->SetClickedListener(this, PropertyChanged);
				pCheckbox->SetUnclickedListener(this, PropertyChanged);

				flTop += 17;
			}
			else
			{
				if (strcmp(pSaveData->m_pszType, "Vector") == 0)
					m_apPropertyLabels.back()->AppendText(" (x y z)");
				else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					m_apPropertyLabels.back()->AppendText(" (x y)");
				else if (strcmp(pSaveData->m_pszType, "QAngle") == 0)
					m_apPropertyLabels.back()->AppendText(" (p y r)");
				else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					m_apPropertyLabels.back()->AppendText(" (p y r x y z)");
				m_apPropertyLabels.back()->SetWidth(200);

				glgui::CTextField* pTextField = new glgui::CTextField();
				m_apPropertyOptions.push_back(pTextField);
				AddControl(pTextField);
				pTextField->SetWidth(GetWidth()-30);
				pTextField->CenterX();
				pTextField->SetTop(flTop+12);

				if (strcmp(pSaveData->m_pszHandle, "Model") == 0)
					pTextField->SetContentsChangedListener(this, ModelChanged, sprintf("%d", i));
				else
					pTextField->SetContentsChangedListener(this, PropertyChanged);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
					pTextField->SetText(m_pEntity->GetParameterValue(pSaveData->m_pszHandle));
				else if (pSaveData->m_bDefault)
				{
					if (strcmp(pSaveData->m_pszType, "size_t") == 0)
					{
						size_t i = *((size_t*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(sprintf("%d", i));
					}
					else if (strcmp(pSaveData->m_pszType, "float") == 0)
					{
						float v = *((float*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(pretty_float(v));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector") == 0)
					{
						Vector v = *((Vector*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(pretty_float(v.x) + " " + pretty_float(v.y) + " " + pretty_float(v.z));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					{
						Vector2D v = *((Vector2D*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(pretty_float(v.x) + " " + pretty_float(v.y));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0)
					{
						EAngle v = *((EAngle*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(pretty_float(v.p) + " " + pretty_float(v.y) + " " + pretty_float(v.r));
					}
					else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					{
						Matrix4x4 m = *((Matrix4x4*)&pSaveData->m_oDefault[0]);
						EAngle e = m.GetAngles();
						Vector v = m.GetTranslation();
						pTextField->SetText(pretty_float(e.p) + " " + pretty_float(v.y) + " " + pretty_float(e.r) + " " + pretty_float(v.x) + " " + pretty_float(v.y) + " " + pretty_float(v.z));
					}
					else if (strcmp(pSaveData->m_pszType, "AABB") == 0)
					{
						AABB b = *((AABB*)&pSaveData->m_oDefault[0]);
						Vector v1 = b.m_vecMins;
						Vector v2 = b.m_vecMaxs;
						pTextField->SetText(pretty_float(v1.x) + " " + pretty_float(v1.y) + " " + pretty_float(v1.z) + " " + pretty_float(v2.x) + " " + pretty_float(v2.y) + " " + pretty_float(v2.z));
					}
					else
					{
						TAssert(false);
					}
				}

				flTop += 43;
			}
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	if (flTop > m_flMaxHeight)
		flTop = m_flMaxHeight;

	SetHeight(flTop);

	BaseClass::Layout();
}

void CEntityPropertiesPanel::ModelChangedCallback(const tstring& sArgs)
{
	eastl::vector<tstring> asExtensions;
	eastl::vector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".png");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	static_cast<glgui::CTextField*>(m_apPropertyOptions[stoi(sArgs)])->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);

	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::PropertyChangedCallback(const tstring& sArgs)
{
	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::SetPropertyChangedListener(glgui::IEventListener* pListener, glgui::IEventListener::Callback pfnCallback)
{
	m_pPropertyChangedListener = pListener;
	m_pfnPropertyChangedCallback = pfnCallback;
}

void CEntityPropertiesPanel::SetEntity(class CLevelEntity* pEntity)
{
	m_pEntity = pEntity;
	Layout();
}

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
	m_pNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pNameLabel);
	m_pNameText = new glgui::CTextField();
	AddControl(m_pNameText);

	m_pModelLabel = new glgui::CLabel("Model:", "sans-serif", 10);
	m_pModelLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pModelLabel);
	m_pModelText = new glgui::CTextField();
	m_pModelText->SetContentsChangedListener(this, ModelChanged);
	AddControl(m_pModelText);

	m_pPropertiesPanel = new CEntityPropertiesPanel(false);
	m_pPropertiesPanel->SetVisible(false);
	AddControl(m_pPropertiesPanel);

	m_bReadyToCreate = false;
}

void CCreateEntityPanel::Layout()
{
	m_pClass->SetWidth(100);
	m_pClass->SetHeight(30);
	m_pClass->CenterX();
	m_pClass->SetTop(30);

	float flTop = 70;
	m_pNameLabel->SetLeft(15);
	m_pNameLabel->SetTop(flTop);
	m_pNameText->SetWidth(GetWidth()-30);
	m_pNameText->CenterX();
	m_pNameText->SetTop(flTop+12);

	flTop += 43;

	m_pModelLabel->SetLeft(15);
	m_pModelLabel->SetTop(flTop);
	m_pModelText->SetWidth(GetWidth()-30);
	m_pModelText->CenterX();
	m_pModelText->SetTop(flTop+12);

	flTop += 43;

	m_pPropertiesPanel->SetTop(flTop);
	m_pPropertiesPanel->SetLeft(10);
	m_pPropertiesPanel->SetWidth(GetWidth()-20);
	m_pPropertiesPanel->SetBackgroundColor(Color(10, 10, 10));

	if (m_bReadyToCreate)
	{
		m_pPropertiesPanel->SetClass("C" + m_pClass->GetText());
		m_pPropertiesPanel->SetMaxHeight(300);
		m_pPropertiesPanel->SetVisible(true);
	}

	BaseClass::Layout();

	SetHeight(m_pPropertiesPanel->GetBottom()+15);
}

void CCreateEntityPanel::ChooseClassCallback(const tstring& sArgs)
{
	eastl::vector<tstring> asTokens;
	strtok(sArgs, asTokens);

	m_pClass->SetText(asTokens[1]);
	m_pClass->Pop(true, true);

	m_bReadyToCreate = true;

	Layout();
}

void CCreateEntityPanel::ModelChangedCallback(const tstring& sArgs)
{
	if (!m_pModelText->GetText().length())
		return;

	eastl::vector<tstring> asExtensions;
	eastl::vector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".png");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_pModelText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

CEditorPanel::CEditorPanel()
{
	m_pEntities = new glgui::CTree();
	m_pEntities->SetBackgroundColor(Color(0, 0, 0, 100));
	m_pEntities->SetSelectedListener(this, EntitySelected);
	AddControl(m_pEntities);

	m_pObjectTitle = new glgui::CLabel("", "sans-serif", 20);
	AddControl(m_pObjectTitle);

	m_pPropertiesPanel = new CEntityPropertiesPanel(true);
	m_pPropertiesPanel->SetBackgroundColor(Color(10, 10, 10, 50));
	m_pPropertiesPanel->SetPropertyChangedListener(this, PropertyChanged);
	AddControl(m_pPropertiesPanel);
}

void CEditorPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_pEntities->SetPos(10, 10);
	m_pEntities->SetSize(GetWidth() - 20, 200);

	m_pEntities->ClearTree();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (pLevel)
	{
		auto& aEntities = pLevel->GetEntityData();
		for (size_t i = 0; i < aEntities.size(); i++)
		{
			auto& oEntity = aEntities[i];

			tstring sName = oEntity.GetParameterValue("Name");

			if (sName.length())
				m_pEntities->AddNode(oEntity.GetClass() + ": " + oEntity.GetParameterValue("Name"));
			else
				m_pEntities->AddNode(oEntity.GetClass());
		}
	}

	m_pObjectTitle->SetPos(0, 220);
	m_pObjectTitle->SetSize(GetWidth(), 25);

	LayoutEntities();

	m_pPropertiesPanel->SetTop(m_pObjectTitle->GetBottom() + 5);
	m_pPropertiesPanel->SetLeft(5);
	m_pPropertiesPanel->SetRight(GetWidth()-5);
	m_pPropertiesPanel->SetMaxHeight(GetBottom() - m_pObjectTitle->GetBottom() - 10);

	BaseClass::Layout();
}

void CEditorPanel::LayoutEntities()
{
	m_pObjectTitle->SetText("(No Object Selected)");
	m_pPropertiesPanel->SetVisible(false);
	m_pPropertiesPanel->SetEntity(nullptr);

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	auto& aEntities = pLevel->GetEntityData();

	if (m_pEntities->GetSelectedNodeId() < aEntities.size())
	{
		CLevelEntity* pEntity = &aEntities[m_pEntities->GetSelectedNodeId()];
		if (pEntity->GetName().length())
			m_pObjectTitle->SetText(pEntity->GetClass() + ": " + pEntity->GetName());
		else
			m_pObjectTitle->SetText(pEntity->GetClass());

		m_pPropertiesPanel->SetClass("C" + pEntity->GetClass());
		m_pPropertiesPanel->SetEntity(pEntity);
		m_pPropertiesPanel->SetVisible(true);
	}
}

void CEditorPanel::EntitySelectedCallback(const tstring& sArgs)
{
	LayoutEntities();

	LevelEditor()->EntitySelected();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	auto& aEntities = pLevel->GetEntityData();

	if (m_pEntities->GetSelectedNodeId() < aEntities.size())
	{
		Manipulator()->Activate(LevelEditor(), aEntities[m_pEntities->GetSelectedNodeId()].GetGlobalTRS(), "Entity " + sprintf("%d", m_pEntities->GetSelectedNodeId()));
	}
	else
	{
		Manipulator()->Deactivate();
	}
}

void CEditorPanel::PropertyChangedCallback(const tstring& sArgs)
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	auto& aEntities = pLevel->GetEntityData();

	if (m_pEntities->GetSelectedNodeId() < aEntities.size())
	{
		CLevelEntity* pEntity = &aEntities[m_pEntities->GetSelectedNodeId()];
		CLevelEditor::PopulateLevelEntityFromPanel(pEntity, m_pPropertiesPanel);

		if (Manipulator()->IsActive())
			Manipulator()->SetTRS(pEntity->GetGlobalTRS());
	}
}

REGISTER_WORKBENCH_TOOL(LevelEditor);

CLevelEditor* CLevelEditor::s_pLevelEditor = nullptr;
	
CLevelEditor::CLevelEditor()
{
	s_pLevelEditor = this;

	m_pEditorPanel = new CEditorPanel();
	m_pEditorPanel->SetVisible(false);
	m_pEditorPanel->SetBackgroundColor(Color(0, 0, 0, 150));
	m_pEditorPanel->SetBorder(glgui::CPanel::BT_SOME);
	glgui::CRootPanel::Get()->AddControl(m_pEditorPanel);

	m_pCreateEntityButton = new glgui::CPictureButton("Create", CMaterialLibrary::AddAsset("editor/create-entity.png"));
	m_pCreateEntityButton->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pCreateEntityButton->GetWidth()/2, 20);
	m_pCreateEntityButton->SetClickedListener(this, CreateEntity);
	m_pCreateEntityButton->SetTooltip("Create Entity Tool");
	glgui::CRootPanel::Get()->AddControl(m_pCreateEntityButton, true);

	m_pCreateEntityPanel = new CCreateEntityPanel();
	m_pCreateEntityPanel->SetBackgroundColor(Color(0, 0, 0, 255));
	m_pCreateEntityPanel->SetHeaderColor(Color(100, 100, 100, 255));
	m_pCreateEntityPanel->SetBorder(glgui::CPanel::BT_SOME);
	m_pCreateEntityPanel->SetVisible(false);

	m_flCreateObjectDistance = 10;
}

CLevelEditor::~CLevelEditor()
{
	glgui::CRootPanel::Get()->RemoveControl(m_pEditorPanel);
	delete m_pEditorPanel;

	glgui::CRootPanel::Get()->RemoveControl(m_pCreateEntityButton);
	delete m_pCreateEntityButton;
	delete m_pCreateEntityPanel;
}

void CLevelEditor::RenderEntity(size_t i)
{
	CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];

	if (m_pEditorPanel->m_pEntities->GetSelectedNodeId() == i)
	{
		CLevelEntity oCopy = *pEntity;
		oCopy.SetGlobalTransform(Manipulator()->GetTransform());
		RenderEntity(&oCopy, true);
	}
	else
		RenderEntity(pEntity, m_pEditorPanel->m_pEntities->GetSelectedNodeId() == i);
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bSelected)
{
	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	// If another context already set this, don't clobber it.
	if (!r.GetActiveFrameBuffer())
		r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

	r.Transform(pEntity->GetGlobalTransform());

	float flAlpha = 1;
	if (!pEntity->IsVisible())
	{
		r.SetBlend(BLEND_ALPHA);
		flAlpha = 0.4f;
	}

	if (pEntity->GetModelID() != ~0)
	{
		if (bSelected)
			r.SetColor(Color(255, 0, 0));
		else
			r.SetColor(Color(255, 255, 255));

		TPROF("CLevelEditor::RenderEntity()");
		r.RenderModel(pEntity->GetModelID(), nullptr);
	}
	else if (pEntity->GetMaterialModel().IsValid())
	{
		if (GameServer()->GetRenderer()->IsRenderingTransparent())
		{
			TPROF("CLevelEditor::RenderModel(Material)");
			r.UseProgram("model");
			r.SetUniform("bDiffuse", true);
			if (bSelected)
				r.SetUniform("vecColor", Color(255, 0, 0, (char)(255*flAlpha)));
			else
				r.SetUniform("vecColor", Color(255, 255, 255, (char)(255*flAlpha)));

			r.Scale(0, pEntity->GetMaterialModelScale().y, pEntity->GetMaterialModelScale().x);
			r.RenderMaterialModel(pEntity->GetMaterialModel());
		}
	}
	else
	{
		r.UseProgram("model");
		if (bSelected)
			r.SetUniform("vecColor", Color(255, 0, 0, (char)(255*flAlpha)));
		else
			r.SetUniform("vecColor", Color(255, 255, 255, (char)(255*flAlpha)));
		r.SetUniform("bDiffuse", false);
		r.RenderWireBox(pEntity->GetBoundingBox());
	}
}

void CLevelEditor::RenderCreateEntityPreview()
{
	CLevelEntity oRenderEntity;
	oRenderEntity.SetClass(m_pCreateEntityPanel->m_pClass->GetText());

	PopulateLevelEntityFromPanel(&oRenderEntity, m_pCreateEntityPanel->m_pPropertiesPanel);

	oRenderEntity.SetParameterValue("Name", m_pCreateEntityPanel->m_pNameText->GetText());
	oRenderEntity.SetParameterValue("Model", m_pCreateEntityPanel->m_pModelText->GetText());
	oRenderEntity.SetGlobalTransform(Matrix4x4(EAngle(0, 0, 0), PositionFromMouse()));

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

void CLevelEditor::CreateEntityFromPanel(const Vector& vecPosition)
{
	auto& aEntityData = m_pLevel->GetEntityData();
	auto& oNewEntity = aEntityData.push_back();
	oNewEntity.SetClass(m_pCreateEntityPanel->m_pClass->GetText());
	oNewEntity.SetParameterValue("Name", m_pCreateEntityPanel->m_pNameText->GetText());

	oNewEntity.SetParameterValue("Model", m_pCreateEntityPanel->m_pModelText->GetText());
	oNewEntity.SetParameterValue("LocalOrigin", sprintf("%f %f %f", vecPosition.x, vecPosition.y, vecPosition.z));

	PopulateLevelEntityFromPanel(&oNewEntity, m_pCreateEntityPanel->m_pPropertiesPanel);

	m_pEditorPanel->Layout();
	m_pEditorPanel->m_pEntities->SetSelectedNode(aEntityData.size()-1);
}

void CLevelEditor::PopulateLevelEntityFromPanel(class CLevelEntity* pEntity, CEntityPropertiesPanel* pPanel)
{
	tstring sModel;

	for (size_t i = 0; i < pPanel->m_asPropertyHandle.size(); i++)
	{
		CSaveData oSaveData;
		CSaveData* pSaveData = CBaseEntity::FindSaveDataValuesByHandle(("C" + pEntity->GetClass()).c_str(), pPanel->m_asPropertyHandle[i].c_str(), &oSaveData);
		if (strcmp(pSaveData->m_pszType, "bool") == 0)
		{
			bool bValue = static_cast<glgui::CCheckBox*>(pPanel->m_apPropertyOptions[i])->GetState();

			if (bValue)
				pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], "1");
			else
				pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], "0");
		}
		else
		{
			tstring sValue = static_cast<glgui::CTextField*>(pPanel->m_apPropertyOptions[i])->GetText();

			if (pPanel->m_asPropertyHandle[i] == "Model")
				CModelLibrary::AddModel(sValue);

			pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], sValue);
		}
	}
}

void CLevelEditor::CreateEntityCallback(const tstring& sArgs)
{
	m_pCreateEntityPanel->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pCreateEntityPanel->GetWidth()/2, 72);
	m_pCreateEntityPanel->SetVisible(true);
}

void CLevelEditor::SaveLevelCallback(const tstring& sArgs)
{
	if (m_pLevel)
		m_pLevel->SaveToFile();

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

	// ; because my dvorak to qwerty key mapper works against me when the game is open, oh well.
	if ((c == 'S' || c == ';') && Application()->IsCtrlDown())
	{
		if (m_pLevel)
			m_pLevel->SaveToFile();

		return true;
	}

	return false;
}

bool CLevelEditor::MouseInput(int iButton, int iState)
{
	if (iState == 1 && m_pCreateEntityPanel->IsVisible() && m_pCreateEntityPanel->m_bReadyToCreate)
	{
		CreateEntityFromPanel(PositionFromMouse());
		return true;
	}

	return false;
}

void CLevelEditor::Activate()
{
	SetCameraOrientation(GameServer()->GetCamera()->GetCameraPosition(), GameServer()->GetCamera()->GetCameraDirection());

	m_pLevel = GameServer()->GetLevel(CVar::GetCVarValue("game_level"));

	m_pEditorPanel->SetVisible(true);
	m_pCreateEntityButton->SetVisible(true);

	GetFileMenu()->AddSubmenu("Save", this, SaveLevel);
}

void CLevelEditor::Deactivate()
{
	m_pEditorPanel->SetVisible(false);
	m_pCreateEntityButton->SetVisible(false);
	m_pCreateEntityPanel->SetVisible(false);

	if (m_pLevel && m_pLevel->GetEntityData().size())
		GameServer()->RestartLevel();
}

void CLevelEditor::RenderScene()
{
	if (!m_pLevel)
		return;

	TPROF("CLevelEditor::RenderEntities()");

	GameServer()->GetRenderer()->SetRenderingTransparent(false);

	auto& aEntityData = m_pLevel->GetEntityData();
	for (size_t i = 0; i < aEntityData.size(); i++)
		RenderEntity(i);

	GameServer()->GetRenderer()->SetRenderingTransparent(true);

	for (size_t i = 0; i < aEntityData.size(); i++)
		RenderEntity(i);

	if (m_pCreateEntityPanel->IsVisible() && m_pCreateEntityPanel->m_bReadyToCreate)
		RenderCreateEntityPreview();
}

void CLevelEditor::CameraThink()
{
	if (Workbench()->GetCamera()->GetFreeMode())
	{
		m_vecEditCamera = Workbench()->GetCamera()->GetFreeCameraPosition();
		m_angEditCamera = Workbench()->GetCamera()->GetFreeCameraAngles();
	}
}

TVector CLevelEditor::GetCameraPosition()
{
	return m_vecEditCamera;
}

TVector CLevelEditor::GetCameraDirection()
{
	return AngleVector(m_angEditCamera);
}

void CLevelEditor::SetCameraOrientation(TVector vecPosition, Vector vecDirection)
{
	m_vecEditCamera = vecPosition;
	m_angEditCamera = VectorAngles(vecDirection);
}

void CLevelEditor::ManipulatorUpdated(const tstring& sArguments)
{
	// Grab this before GetToyToModify since that does a layout and clobbers the list.
	size_t iSelected = m_pEditorPanel->m_pEntities->GetSelectedNodeId();

	eastl::vector<tstring> asTokens;
	strtok(sArguments, asTokens);
	TAssert(asTokens.size() == 2);
	TAssert(stoi(asTokens[1]) == iSelected);
	TAssert(iSelected != ~0);

	if (!GetLevel())
		return;

	if (iSelected >= GetLevel()->GetEntityData().size())
		return;

	Vector vecTranslation = Manipulator()->GetTRS().m_vecTranslation;
	EAngle angRotation = Manipulator()->GetTRS().m_angRotation;
	GetLevel()->GetEntityData()[iSelected].SetParameterValue("LocalOrigin", pretty_float(vecTranslation.x) + " " + pretty_float(vecTranslation.y) + " " + pretty_float(vecTranslation.z));
	GetLevel()->GetEntityData()[iSelected].SetParameterValue("LocalAngles", pretty_float(angRotation.p) + " " + pretty_float(angRotation.y) + " " + pretty_float(angRotation.r));

	m_pEditorPanel->LayoutEntities();
}
