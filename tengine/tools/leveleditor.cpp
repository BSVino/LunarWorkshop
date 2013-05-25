#include "leveleditor.h"

#include <tinker_platform.h>
#include <files.h>
#include <tvector.h>

#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>
#include <glgui/checkbox.h>
#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <glgui/slidingpanel.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <tinker/profiler.h>
#include <textures/materiallibrary.h>
#include <tinker/keys.h>
#include <game/level.h>
#include <game/gameserver.h>
#include <models/models.h>
#include <toys/toy.h>
#include <tools/manipulator/manipulator.h>

#include "workbench.h"

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

	TAssert(m_ahPropertyLabels.size() == m_ahPropertyOptions.size());
	for (size_t i = 0; i < m_ahPropertyLabels.size(); i++)
	{
		RemoveControl(m_ahPropertyLabels[i]);
		RemoveControl(m_ahPropertyOptions[i]);
	}
	m_ahPropertyLabels.clear();
	m_ahPropertyOptions.clear();
	m_asPropertyHandle.clear();

	// If we're ready to create then a class has been chosen.
	const tchar* pszClassName = m_sClass.c_str();
	CEntityRegistration* pRegistration = NULL;
	tmap<tstring, bool> abHandlesSet;

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

				if (strcmp(pSaveData->m_pszHandle, "Origin") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "Angles") == 0)
					continue;
			}

			if (abHandlesSet.find(tstring(pSaveData->m_pszHandle)) != abHandlesSet.end())
				continue;

			abHandlesSet[tstring(pSaveData->m_pszHandle)] = true;

			m_asPropertyHandle.push_back(tstring(pSaveData->m_pszHandle));

			m_ahPropertyLabels.push_back(AddControl(new glgui::CLabel(tstring(pSaveData->m_pszHandle) + ": ", "sans-serif", 10), true));
			m_ahPropertyLabels.back()->SetAlign(glgui::CLabel::TA_TOPLEFT);
			m_ahPropertyLabels.back()->SetLeft(0);
			m_ahPropertyLabels.back()->SetTop(flTop);
			m_ahPropertyLabels.back()->SetWidth(10);
			m_ahPropertyLabels.back()->EnsureTextFits();

			if (strcmp(pSaveData->m_pszType, "bool") == 0)
			{
				glgui::CControl<glgui::CCheckBox> hCheckBox(AddControl(new glgui::CCheckBox(), true));
				m_ahPropertyOptions.push_back(hCheckBox.GetHandle());
				hCheckBox->SetLeft(m_ahPropertyLabels.back()->GetRight() + 10);
				hCheckBox->SetTop(flTop);
				hCheckBox->SetSize(12, 12);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
					hCheckBox->SetState(UnserializeString_bool(m_pEntity->GetParameterValue(pSaveData->m_pszHandle)));
				else if (pSaveData->m_bDefault)
					hCheckBox->SetState(!!pSaveData->m_oDefault[0], false);

				hCheckBox->SetClickedListener(this, PropertyChanged);
				hCheckBox->SetUnclickedListener(this, PropertyChanged);

				flTop += 17;
			}
			else
			{
				if (strcmp(pSaveData->m_pszType, "Vector") == 0)
					m_ahPropertyLabels.back()->AppendText(" (x y z)");
				else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					m_ahPropertyLabels.back()->AppendText(" (x y)");
				else if (strcmp(pSaveData->m_pszType, "QAngle") == 0)
					m_ahPropertyLabels.back()->AppendText(" (p y r)");
				else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					m_ahPropertyLabels.back()->AppendText(" (p y r x y z)");
				m_ahPropertyLabels.back()->SetWidth(200);

				glgui::CControl<glgui::CTextField> hTextField(AddControl(new glgui::CTextField(), true));
				m_ahPropertyOptions.push_back(hTextField.GetHandle());
				hTextField->Layout_FullWidth(0);
				hTextField->SetWidth(hTextField->GetWidth()-15);
				hTextField->SetTop(flTop+12);

				if (strcmp(pSaveData->m_pszHandle, "Model") == 0)
					hTextField->SetContentsChangedListener(this, ModelChanged, sprintf("%d", m_ahPropertyOptions.size()-1));
				else if (tstring(pSaveData->m_pszType).startswith("CEntityHandle"))
					hTextField->SetContentsChangedListener(this, TargetChanged, sprintf("%d ", m_ahPropertyOptions.size()-1) + pSaveData->m_pszType);
				else
					hTextField->SetContentsChangedListener(this, PropertyChanged);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
				{
					hTextField->SetText(m_pEntity->GetParameterValue(pSaveData->m_pszHandle));

					if (strcmp(pSaveData->m_pszType, "Vector") == 0 && CanUnserializeString_TVector(hTextField->GetText()))
					{
						Vector v = UnserializeString_TVector(hTextField->GetText());
						hTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0 && CanUnserializeString_EAngle(hTextField->GetText()))
					{
						EAngle v = UnserializeString_EAngle(hTextField->GetText());
						hTextField->SetText(pretty_float(v.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.r, 4));
					}
				}
				else if (pSaveData->m_bDefault)
				{
					if (strcmp(pSaveData->m_pszType, "size_t") == 0)
					{
						size_t i = *((size_t*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(sprintf("%d", i));
					}
					else if (strcmp(pSaveData->m_pszType, "float") == 0)
					{
						float v = *((float*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector") == 0)
					{
						Vector v = *((Vector*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					{
						Vector2D v = *((Vector2D*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0)
					{
						EAngle v = *((EAngle*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.r, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					{
						Matrix4x4 m = *((Matrix4x4*)&pSaveData->m_oDefault[0]);
						EAngle e = m.GetAngles();
						Vector v = m.GetTranslation();
						hTextField->SetText(pretty_float(e.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(e.r, 4) + " " + pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "AABB") == 0)
					{
						AABB b = *((AABB*)&pSaveData->m_oDefault[0]);
						Vector v1 = b.m_vecMins;
						Vector v2 = b.m_vecMaxs;
						hTextField->SetText(pretty_float(v1.x, 4) + " " + pretty_float(v1.y, 4) + " " + pretty_float(v1.z, 4) + " " + pretty_float(v2.x, 4) + " " + pretty_float(v2.y, 4) + " " + pretty_float(v2.z, 4));
					}
					else
					{
						TUnimplemented();
					}
				}

				flTop += 43;
			}
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	float flMaxHeight = GetParent()->GetHeight() - GetParent()->GetDefaultMargin();
	if (flTop > flMaxHeight)
		flTop = flMaxHeight;

	SetHeight(flTop);

	BaseClass::Layout();
}

void CEntityPropertiesPanel::ModelChangedCallback(const tstring& sArgs)
{
	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".mat");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_ahPropertyOptions[stoi(sArgs)].DowncastStatic<glgui::CTextField>()->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);

	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::TargetChangedCallback(const tstring& sArgs)
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	tvector<tstring> asTokens;
	tstrtok(sArgs, asTokens, "<>");

	TAssert(asTokens.size() == 2);
	if (asTokens.size() != 2)
		return;

	tvector<tstring> asTargets;

	for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
	{
		auto* pEntity = &pLevel->GetEntityData()[i];
		if (!pEntity)
			continue;

		if (!pEntity->GetName().length())
			continue;

		CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity("C"+pEntity->GetClass());
		TAssert(pRegistration);
		if (!pRegistration)
			continue;

		bool bFound = false;
		while (pRegistration)
		{
			if (asTokens[1] == pRegistration->m_pszEntityClass)
			{
				bFound = true;
				break;
			}

			pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
		}

		if (!bFound)
			continue;

		asTargets.push_back(pEntity->GetName());
	}

	m_ahPropertyOptions[stoi(sArgs)].DowncastStatic<glgui::CTextField>()->SetAutoCompleteCommands(asTargets);

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
	m_hClass = AddControl(new glgui::CMenu("Choose Class"));

	for (size_t i = 0; i < CBaseEntity::GetNumEntitiesRegistered(); i++)
	{
		CEntityRegistration* pRegistration = CBaseEntity::GetEntityRegistration(i);

		if (!pRegistration->m_bCreatableInEditor)
			continue;

		m_hClass->AddSubmenu(pRegistration->m_pszEntityClass+1, this, ChooseClass);
	}

	m_hNameLabel = AddControl(new glgui::CLabel("Name:", "sans-serif", 10));
	m_hNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hNameText = AddControl(new glgui::CTextField());

	m_hModelLabel = AddControl(new glgui::CLabel("Model:", "sans-serif", 10));
	m_hModelLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hModelText = AddControl(new glgui::CTextField());
	m_hModelText->SetContentsChangedListener(this, ModelChanged);

	m_hPropertiesPanel = AddControl(new CEntityPropertiesPanel(false));
	m_hPropertiesPanel->SetVisible(false);

	m_bReadyToCreate = false;
}

void CCreateEntityPanel::Layout()
{
	m_hClass->SetWidth(100);
	m_hClass->SetHeight(30);
	m_hClass->CenterX();
	m_hClass->SetTop(30);

	float flTop = 70;
	m_hNameLabel->SetLeft(15);
	m_hNameLabel->SetTop(flTop);
	m_hNameText->SetWidth(GetWidth()-30);
	m_hNameText->CenterX();
	m_hNameText->SetTop(flTop+12);

	flTop += 43;

	m_hModelLabel->SetLeft(15);
	m_hModelLabel->SetTop(flTop);
	m_hModelText->SetWidth(GetWidth()-30);
	m_hModelText->CenterX();
	m_hModelText->SetTop(flTop+12);

	flTop += 43;

	m_hPropertiesPanel->SetTop(flTop);
	m_hPropertiesPanel->SetLeft(10);
	m_hPropertiesPanel->SetWidth(GetWidth()-20);
	m_hPropertiesPanel->SetBackgroundColor(Color(10, 10, 10));

	if (m_bReadyToCreate)
	{
		m_hPropertiesPanel->SetClass("C" + m_hClass->GetText());
		m_hPropertiesPanel->SetVisible(true);
	}

	BaseClass::Layout();

	SetHeight(m_hPropertiesPanel->GetBottom()+15);
}

void CCreateEntityPanel::ChooseClassCallback(const tstring& sArgs)
{
	tvector<tstring> asTokens;
	strtok(sArgs, asTokens);

	m_hClass->SetText(asTokens[1]);
	m_hClass->Pop(true, true);

	m_bReadyToCreate = true;

	Layout();
}

void CCreateEntityPanel::ModelChangedCallback(const tstring& sArgs)
{
	if (!m_hModelText->GetText().length())
		return;

	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".mat");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_hModelText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

CEditorPanel::CEditorPanel()
{
	m_hEntities = AddControl(new glgui::CTree());
	m_hEntities->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hEntities->SetSelectedListener(this, EntitySelected);

	m_hObjectTitle = AddControl(new glgui::CLabel("", "sans-serif", 20));

	m_hSlider = AddControl(new glgui::CSlidingContainer());

	m_hPropertiesSlider = new glgui::CSlidingPanel(m_hSlider, "Properties");
	m_hOutputsSlider = new glgui::CSlidingPanel(m_hSlider, "Outputs");

	m_hPropertiesPanel = m_hPropertiesSlider->AddControl(new CEntityPropertiesPanel(true));
	m_hPropertiesPanel->SetBackgroundColor(Color(10, 10, 10, 50));
	m_hPropertiesPanel->SetPropertyChangedListener(this, PropertyChanged);

	m_hOutputs = m_hOutputsSlider->AddControl(new glgui::CTree());
	m_hOutputs->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hOutputs->SetSelectedListener(this, OutputSelected);

	m_hAddOutput = m_hOutputsSlider->AddControl(new glgui::CButton("Add"));
	m_hAddOutput->SetClickedListener(this, AddOutput);

	m_hRemoveOutput = m_hOutputsSlider->AddControl(new glgui::CButton("Remove"));
	m_hRemoveOutput->SetClickedListener(this, RemoveOutput);

	m_hOutput = m_hOutputsSlider->AddControl(new glgui::CMenu("Choose Output"));

	m_hOutputEntityNameLabel = m_hOutputsSlider->AddControl(new glgui::CLabel("Target Entity:", "sans-serif", 10));
	m_hOutputEntityNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hOutputEntityNameText = m_hOutputsSlider->AddControl(new glgui::CTextField());
	m_hOutputEntityNameText->SetContentsChangedListener(this, TargetEntityChanged);

	m_hInput = m_hOutputsSlider->AddControl(new glgui::CMenu("Choose Input"));

	m_hOutputArgsLabel = m_hOutputsSlider->AddControl(new glgui::CLabel("Arguments:", "sans-serif", 10));
	m_hOutputArgsLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hOutputArgsText = m_hOutputsSlider->AddControl(new glgui::CTextField());
	m_hOutputArgsText->SetContentsChangedListener(this, ArgumentsChanged);
}

void CEditorPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_hEntities->SetPos(10, 10);
	m_hEntities->SetSize(GetWidth() - 20, 200);

	m_hEntities->ClearTree();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (pLevel)
	{
		auto& aEntities = pLevel->GetEntityData();
		for (size_t i = 0; i < aEntities.size(); i++)
		{
			auto& oEntity = aEntities[i];

			tstring sName = oEntity.GetParameterValue("Name");
			tstring sModel = oEntity.GetParameterValue("Model");

			if (sName.length())
				m_hEntities->AddNode(oEntity.GetClass() + ": " + sName);
			else if (sModel.length())
				m_hEntities->AddNode(oEntity.GetClass() + " (" + GetFilename(sModel) + ")");
			else
				m_hEntities->AddNode(oEntity.GetClass());
		}
	}

	m_hObjectTitle->SetPos(0, 220);
	m_hObjectTitle->SetSize(GetWidth(), 25);

	float flTempMargin = 5;
	m_hSlider->Layout_AlignTop(m_hObjectTitle, flTempMargin);
	m_hSlider->Layout_FullWidth(flTempMargin);
	m_hSlider->SetBottom(GetHeight() - flTempMargin);

	flTempMargin = 2;
	m_hPropertiesPanel->Layout_AlignTop(nullptr, flTempMargin);
	m_hPropertiesPanel->Layout_FullWidth(flTempMargin);

	LayoutEntity();

	BaseClass::Layout();
}

void CEditorPanel::LayoutEntity()
{
	m_hObjectTitle->SetText("(No Object Selected)");
	m_hPropertiesPanel->SetVisible(false);
	m_hPropertiesPanel->SetEntity(nullptr);

	CLevelEntity* pEntity = GetCurrentEntity();

	if (pEntity)
	{
		if (pEntity->GetName().length())
			m_hObjectTitle->SetText(pEntity->GetClass() + ": " + pEntity->GetName());
		else
			m_hObjectTitle->SetText(pEntity->GetClass());

		m_hPropertiesPanel->SetClass("C" + pEntity->GetClass());
		m_hPropertiesPanel->SetEntity(pEntity);
		m_hPropertiesPanel->SetVisible(true);

		m_hOutputs->ClearTree();

		auto& aEntityOutputs = pEntity->GetOutputs();

		for (size_t i = 0; i < aEntityOutputs.size(); i++)
		{
			auto& oEntityOutput = aEntityOutputs[i];

			m_hOutputs->AddNode(oEntityOutput.m_sOutput + " -> " + oEntityOutput.m_sTargetName + ":" + oEntityOutput.m_sInput);
		}

		m_hOutputs->Layout();
	}

	LayoutOutput();
}

void CEditorPanel::LayoutOutput()
{
	m_hAddOutput->SetEnabled(false);
	m_hRemoveOutput->SetEnabled(false);
	m_hOutput->SetEnabled(false);
	m_hOutputEntityNameText->SetEnabled(false);
	m_hInput->SetEnabled(false);
	m_hOutputArgsText->SetEnabled(false);

	m_hOutputs->Layout_AlignTop();
	m_hOutputs->Layout_FullWidth();
	m_hOutputs->SetHeight(50);

	m_hAddOutput->SetHeight(15);
	m_hAddOutput->Layout_AlignTop(m_hOutputs);
	m_hAddOutput->Layout_Column(2, 0);
	m_hRemoveOutput->SetHeight(15);
	m_hRemoveOutput->Layout_AlignTop(m_hOutputs);
	m_hRemoveOutput->Layout_Column(2, 1);

	m_hOutput->Layout_AlignTop(m_hAddOutput, 10);
	m_hOutput->ClearSubmenus();
	m_hOutput->SetSize(100, 30);
	m_hOutput->CenterX();
	m_hOutput->SetText("Choose Output");

	m_hOutputEntityNameLabel->Layout_AlignTop(m_hOutput);
	m_hOutputEntityNameText->SetTop(m_hOutputEntityNameLabel->GetTop()+12);
	m_hOutputEntityNameText->SetText("");
	m_hOutputEntityNameText->Layout_FullWidth();

	m_hInput->Layout_AlignTop(m_hOutputEntityNameText, 10);
	m_hInput->ClearSubmenus();
	m_hInput->SetSize(100, 30);
	m_hInput->CenterX();
	m_hInput->SetText("Choose Input");

	m_hOutputArgsLabel->Layout_AlignTop(m_hInput);
	m_hOutputArgsText->SetTop(m_hOutputArgsLabel->GetTop()+12);
	m_hOutputArgsText->SetText("");
	m_hOutputArgsText->Layout_FullWidth();

	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	m_hAddOutput->SetEnabled(true);
	m_hRemoveOutput->SetEnabled(true);

	CLevelEntity::CLevelEntityOutput* pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	if (pOutput->m_sTargetName.length())
		m_hOutputEntityNameText->SetText(pOutput->m_sTargetName);

	if (pOutput->m_sArgs.length())
		m_hOutputArgsText->SetText(pOutput->m_sArgs);

	if (pOutput->m_sOutput.length())
		m_hOutput->SetText(pOutput->m_sOutput);

	if (pOutput->m_sInput.length())
		m_hInput->SetText(pOutput->m_sInput);

	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity("C" + pEntity->GetClass());
	TAssert(pRegistration);
	if (!pRegistration)
		return;

	m_hOutput->SetEnabled(true);
	m_hOutputEntityNameText->SetEnabled(true);
	m_hInput->SetEnabled(true);
	m_hOutputArgsText->SetEnabled(true);

	m_hOutput->ClearSubmenus();

	do {
		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			auto& oSaveData = pRegistration->m_aSaveData[i];
			if (oSaveData.m_eType != CSaveData::DATA_OUTPUT)
				continue;

			m_hOutput->AddSubmenu(oSaveData.m_pszHandle, this, ChooseOutput);
		}

		if (!pRegistration->m_pszParentClass)
			break;

		pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
	} while (pRegistration);

	LayoutInput();
}

void CEditorPanel::LayoutInput()
{
	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	CLevel* pLevel = LevelEditor()->GetLevel();
	if (!pLevel)
		return;

	CEntityRegistration* pRegistration = nullptr;
	if (m_hOutputEntityNameText->GetText()[0] == '*')
		pRegistration = CBaseEntity::GetRegisteredEntity("C" + m_hOutputEntityNameText->GetText().substr(1));
	else if (m_hOutputEntityNameText->GetText().length())
	{
		CLevelEntity* pTarget = nullptr;

		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			if (!pLevel->GetEntityData()[i].GetName().length())
				continue;

			if (pLevel->GetEntityData()[i].GetName() == m_hOutputEntityNameText->GetText())
			{
				pTarget = &pLevel->GetEntityData()[i];
				break;
			}
		}

		if (pTarget)
			pRegistration = CBaseEntity::GetRegisteredEntity("C" + pTarget->GetClass());
	}

	m_hInput->ClearSubmenus();

	if (pRegistration)
	{
		do {
			for (auto it = pRegistration->m_aInputs.begin(); it != pRegistration->m_aInputs.end(); it++)
				m_hInput->AddSubmenu(it->first, this, ChooseInput);

			if (!pRegistration->m_pszParentClass)
				break;

			pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
		} while (pRegistration);
	}
}

void CEditorPanel::Paint(float x, float y, float w, float h)
{
	Matrix4x4 mFontProjection = Matrix4x4::ProjectOrthographic(0, glgui::RootPanel()->GetWidth(), 0, glgui::RootPanel()->GetHeight(), -1, 1);

	CLevel* pLevel = LevelEditor()->GetLevel();
	if (pLevel)
	{
		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			CLevelEntity* pEnt = &pLevel->GetEntityData()[i];

			if (m_hEntities->GetSelectedNodeId() != i && !pEnt->GetName().length())
				continue;

			Vector vecCenter = pEnt->GetGlobalTransform().GetTranslation();
			if (!GameServer()->GetRenderer()->IsSphereInFrustum(vecCenter, pEnt->GetBoundingBox().Size().Length()/2))
				continue;

			if (pEnt->GetMaterialModel().IsValid() && !pEnt->ShouldDisableBackCulling())
			{
				Vector vecToCenter = vecCenter - GameServer()->GetRenderer()->GetCameraPosition();

				if (pEnt->ShouldRenderInverted())
				{
					if (vecToCenter.Dot(pEnt->GetGlobalTransform().GetForwardVector()) > 0)
						continue;
				}
				else
				{
					if (vecToCenter.Dot(pEnt->GetGlobalTransform().GetForwardVector()) < 0)
						continue;
				}
			}

			Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecCenter);

			tstring sText;
			if (pEnt->GetName().length())
				sText = pEnt->GetClass() + ": " + pEnt->GetName();
			else
				sText = pEnt->GetClass();

			float flWidth = glgui::CLabel::GetTextWidth(sText, sText.length(), "sans-serif", 10);
			float flHeight = glgui::CLabel::GetFontHeight("sans-serif", 10);
			float flAscender = glgui::CLabel::GetFontAscender("sans-serif", 10);

			if (m_hEntities->GetSelectedNodeId() == i)
				PaintRect(vecScreen.x - flWidth/2 - 3, vecScreen.y+20-flAscender - 3, flWidth + 6, flHeight + 6, Color(0, 0, 0, 150));

			CRenderingContext c(nullptr, true);

			c.SetBlend(BLEND_ALPHA);
			c.UseProgram("text");
			c.SetProjection(mFontProjection);

			if (m_hEntities->GetSelectedNodeId() == i)
				c.SetUniform("vecColor", Color(255, 0, 0, 255));
			else
				c.SetUniform("vecColor", Color(255, 255, 255, 200));

			c.Translate(Vector(vecScreen.x - flWidth/2, glgui::RootPanel()->GetBottom()-vecScreen.y-20, 0));
			c.SetUniform("bScissor", false);

			c.RenderText(sText, sText.length(), "sans-serif", 10);
		}
	}

	BaseClass::Paint(x, y, w, h);
}

CLevelEntity* CEditorPanel::GetCurrentEntity()
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return nullptr;

	auto& aEntities = pLevel->GetEntityData();

	if (m_hEntities->GetSelectedNodeId() >= aEntities.size())
		return nullptr;

	return &aEntities[m_hEntities->GetSelectedNodeId()];
}

CLevelEntity::CLevelEntityOutput* CEditorPanel::GetCurrentOutput()
{
	CLevelEntity* pEntity = GetCurrentEntity();

	auto& aEntityOutputs = pEntity->GetOutputs();

	if (m_hOutputs->GetSelectedNodeId() >= aEntityOutputs.size())
		return nullptr;

	return &aEntityOutputs[m_hOutputs->GetSelectedNodeId()];
}

void CEditorPanel::EntitySelectedCallback(const tstring& sArgs)
{
	LayoutEntity();

	LevelEditor()->EntitySelected();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	auto& aEntities = pLevel->GetEntityData();

	if (m_hEntities->GetSelectedNodeId() < aEntities.size())
	{
		Manipulator()->Activate(LevelEditor(), aEntities[m_hEntities->GetSelectedNodeId()].GetGlobalTRS(), "Entity " + sprintf("%d", m_hEntities->GetSelectedNodeId()));
	}
	else
	{
		Manipulator()->Deactivate();
	}
}

void CEditorPanel::PropertyChangedCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	if (pEntity)
	{
		CLevelEditor::PopulateLevelEntityFromPanel(pEntity, m_hPropertiesPanel);

		if (Manipulator()->IsActive())
			Manipulator()->SetTRS(pEntity->GetGlobalTRS());
	}
}

void CEditorPanel::OutputSelectedCallback(const tstring& sArgs)
{
	LayoutOutput();
}

void CEditorPanel::AddOutputCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	pEntity->GetOutputs().push_back();

	LayoutEntity();

	m_hOutputs->SetSelectedNode(pEntity->GetOutputs().size()-1);
}

void CEditorPanel::RemoveOutputCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	pEntity->GetOutputs().erase(pEntity->GetOutputs().begin()+m_hOutputs->GetSelectedNodeId());

	LayoutEntity();
}

void CEditorPanel::ChooseOutputCallback(const tstring& sArgs)
{
	m_hOutput->Pop(true, true);

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	tvector<tstring> asTokens;
	tstrtok(sArgs, asTokens);
	pOutput->m_sOutput = asTokens[1];
	m_hOutput->SetText(pOutput->m_sOutput);

	LayoutInput();
}

void CEditorPanel::TargetEntityChangedCallback(const tstring& sArgs)
{
	CLevel* pLevel = LevelEditor()->GetLevel();
	if (!pLevel)
		return;

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	pOutput->m_sTargetName = m_hOutputEntityNameText->GetText();

	tvector<tstring> asTargets;

	for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
	{
		auto* pEntity = &pLevel->GetEntityData()[i];
		if (!pEntity)
			continue;

		if (!pEntity->GetName().length())
			continue;

		CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity("C"+pEntity->GetClass());
		TAssert(pRegistration);
		if (!pRegistration)
			continue;

		asTargets.push_back(pEntity->GetName());
	}

	m_hOutputEntityNameText->SetAutoCompleteCommands(asTargets);

	LayoutInput();
}

void CEditorPanel::ChooseInputCallback(const tstring& sArgs)
{
	m_hInput->Pop(true, true);

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	tvector<tstring> asTokens;
	tstrtok(sArgs, asTokens);
	pOutput->m_sInput = asTokens[1];
	m_hInput->SetText(pOutput->m_sInput);
}

void CEditorPanel::ArgumentsChangedCallback(const tstring& sArgs)
{
	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	pOutput->m_sArgs = m_hOutputArgsText->GetText();
}

REGISTER_WORKBENCH_TOOL(LevelEditor);

CLevelEditor* CLevelEditor::s_pLevelEditor = nullptr;
	
CLevelEditor::CLevelEditor()
{
	s_pLevelEditor = this;

	m_hEditorPanel = glgui::RootPanel()->AddControl(new CEditorPanel());
	m_hEditorPanel->SetVisible(false);
	m_hEditorPanel->SetBackgroundColor(Color(0, 0, 0, 150));
	m_hEditorPanel->SetBorder(glgui::CPanel::BT_SOME);

	m_hCreateEntityButton = glgui::RootPanel()->AddControl(new glgui::CPictureButton("Create", CMaterialLibrary::AddMaterial("editor/create-entity.mat")), true);
	m_hCreateEntityButton->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_hCreateEntityButton->GetWidth()/2, 20);
	m_hCreateEntityButton->SetClickedListener(this, CreateEntity);
	m_hCreateEntityButton->SetTooltip("Create Entity Tool");

	m_hCreateEntityPanel = glgui::CControl<CCreateEntityPanel>((new CCreateEntityPanel())->GetHandle()); // Adds itself.
	m_hCreateEntityPanel->SetBackgroundColor(Color(0, 0, 0, 255));
	m_hCreateEntityPanel->SetHeaderColor(Color(100, 100, 100, 255));
	m_hCreateEntityPanel->SetBorder(glgui::CPanel::BT_SOME);
	m_hCreateEntityPanel->SetVisible(false);

	m_flCreateObjectDistance = 10;
}

CLevelEditor::~CLevelEditor()
{
	glgui::CRootPanel::Get()->RemoveControl(m_hEditorPanel);

	glgui::CRootPanel::Get()->RemoveControl(m_hCreateEntityButton);
}

void CLevelEditor::LoadLevel(const CHandle<CLevel>& pLevel)
{
	if (pLevel == m_pLevel)
		return;

	m_pLevel = pLevel;

	if (!m_pLevel.Get())
		return;

	EditorPhysics()->RemoveAllEntities();

	for (size_t i = 0; i < m_pLevel->GetEntityData().size(); i++)
	{
		CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];
		EditorPhysics()->AddEntity(pEntity, CT_STATIC_MESH);
	}
}

void CLevelEditor::Think()
{
	BaseClass::Think();

	int x, y;
	Application()->GetMousePosition(x, y);

	if (ShouldRenderOrthographic())
	{
		Vector vecPosition1 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
		Vector vecPosition0 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 0));

		m_iHoverEntity = TraceLine(Ray(vecPosition0, (vecPosition1-vecPosition0).Normalized()));
	}
	else
	{
		Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();
		Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

		m_iHoverEntity = TraceLine(Ray(vecCamera, (vecPosition-vecCamera).Normalized()));
	}
}

void CLevelEditor::RenderEntity(size_t i)
{
	CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];

	if (m_hEditorPanel->m_hEntities->GetSelectedNodeId() == i)
		RenderEntity(pEntity, true, false, Manipulator()->GetTransform(true, false));
	else
		RenderEntity(pEntity, m_hEditorPanel->m_hEntities->GetSelectedNodeId() == i, i == m_iHoverEntity);
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bSelected, bool bHover)
{
	RenderEntity(pEntity, bSelected, bHover, pEntity->GetGlobalTransform());
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bSelected, bool bHover, const Matrix4x4& mGlobal)
{
	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	// If another context already set this, don't clobber it.
	if (!r.GetActiveFrameBuffer())
		r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

	r.Transform(mGlobal);

	if (pEntity->ShouldRenderInverted())
		r.SetWinding(!r.GetWinding());

	if (pEntity->ShouldDisableBackCulling())
		r.SetBackCulling(false);

	Vector vecScale = pEntity->GetScale();
	if (bSelected && Manipulator()->IsTransforming())
		vecScale = Manipulator()->GetNewTRS().m_vecScaling;

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
		bool bRenderMaterial = true;
		if (!pEntity->ShouldDisableBackCulling())
		{
			if (pEntity->ShouldRenderInverted())
			{
				if ((mGlobal.GetTranslation() - GetCameraPosition()).Dot(mGlobal.GetForwardVector()) > 0)
					bRenderMaterial = false;
			}
			else
			{
				if ((mGlobal.GetTranslation() - GetCameraPosition()).Dot(mGlobal.GetForwardVector()) < 0)
					bRenderMaterial = false;
			}
		}

		if (GameServer()->GetRenderer()->IsRenderingTransparent())
		{
			TPROF("CLevelEditor::RenderModel(Material)");
			r.UseProgram("model");

			Color clrEnt;
			if (bSelected)
				clrEnt = Color(255, 0, 0, (char)(255*flAlpha));
			else
				clrEnt = Color(255, 255, 255, (char)(255*flAlpha));

			Color clrEntHover = clrEnt;
			if (!bRenderMaterial)
				clrEnt.SetAlpha(clrEnt.a()/5);

			if (!bHover)
				clrEntHover.SetAlpha(clrEnt.a()/3);

			r.SetBlend(BLEND_ALPHA);
			r.SetColor(clrEntHover);
			r.SetUniform("vecColor", clrEntHover);
			r.SetUniform("bDiffuse", false);

			AABB aabbBounds = pEntity->GetBoundingBox();

			// A bit of a hack to fix up the scale of the AABB to render it properly while dragging the manipulator scale handle
			if (bSelected && Manipulator()->IsTransforming())
			{
				aabbBounds.m_vecMins = aabbBounds.m_vecMins / pEntity->GetScale() * vecScale;
				aabbBounds.m_vecMaxs = aabbBounds.m_vecMaxs / pEntity->GetScale() * vecScale;
			}

			r.RenderWireBox(aabbBounds);

			if (bRenderMaterial)
			{
				r.SetColor(clrEnt);
				r.SetUniform("vecColor", clrEnt);
				r.SetUniform("bDiffuse", true);

				r.Scale(vecScale.x, vecScale.y, vecScale.z);
				r.RenderMaterialModel(pEntity->GetMaterialModel());
			}
		}
	}
	else
	{
		r.UseProgram("model");
		if (bSelected)
			r.SetUniform("vecColor", Color(255, 0, 0, (char)(255*flAlpha)));
		else if (bHover)
			r.SetUniform("vecColor", Color(255, 255, 255, (char)(255*flAlpha)));
		else
		{
			r.SetBlend(BLEND_ALPHA);
			r.SetUniform("vecColor", Color(255, 255, 255, (char)(150*flAlpha)));
		}

		r.SetUniform("bDiffuse", false);

		AABB aabbBounds = pEntity->GetBoundingBox();

		// A bit of a hack to fix up the scale of the AABB to render it properly while dragging the manipulator scale handle
		if (bSelected && Manipulator()->IsTransforming())
		{
			aabbBounds.m_vecMins = aabbBounds.m_vecMins / pEntity->GetScale() * vecScale;
			aabbBounds.m_vecMaxs = aabbBounds.m_vecMaxs / pEntity->GetScale() * vecScale;
		}

		r.RenderWireBox(aabbBounds);
	}
}

void CLevelEditor::RenderCreateEntityPreview()
{
	CLevelEntity oRenderEntity;
	oRenderEntity.SetClass(m_hCreateEntityPanel->m_hClass->GetText());

	PopulateLevelEntityFromPanel(&oRenderEntity, m_hCreateEntityPanel->m_hPropertiesPanel);

	oRenderEntity.SetParameterValue("Name", m_hCreateEntityPanel->m_hNameText->GetText());
	oRenderEntity.SetParameterValue("Model", m_hCreateEntityPanel->m_hModelText->GetText());
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

size_t CLevelEditor::TraceLine(const Ray& vecTrace)
{
	CLevel* pLevel = GetLevel();
	if (!pLevel)
		return ~0;

	size_t iNearest = ~0;
	float flNearestSqr;

	float flTraceDistance = 100;
	CTraceResult tr;
	EditorPhysics()->TraceLine(tr, vecTrace.m_vecPos, vecTrace.m_vecPos + vecTrace.m_vecDir*flTraceDistance);

	if (tr.m_flFraction < 1)
	{
		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			if (pLevel->GetEntityData()[i].GetHandle() == tr.m_iHit)
			{
				iNearest = i;
				float flDistance = tr.m_flFraction * flTraceDistance;
				flNearestSqr = flDistance*flDistance;
				break;
			}
		}
	}

	for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
	{
		CLevelEntity* pEnt = &pLevel->GetEntityData()[i];

		Vector vecCenter = pEnt->GetGlobalTransform().GetTranslation();

		AABB aabbLocalBounds = pEnt->GetBoundingBox();
		aabbLocalBounds.m_vecMaxs += Vector(0.01f, 0.01f, 0.01f);	// Make sure it's not zero size.

		AABB aabbGlobalBounds = aabbLocalBounds;
		aabbGlobalBounds += vecCenter;

		if (!GameServer()->GetRenderer()->IsSphereInFrustum(vecCenter, aabbGlobalBounds.Size().Length()/2))
			continue;

		if (pEnt->GetMaterialModel().IsValid() && !pEnt->ShouldDisableBackCulling())
		{
			if (pEnt->ShouldRenderInverted())
			{
				if (vecTrace.m_vecDir.Dot(pEnt->GetGlobalTransform().GetForwardVector()) > 0)
					continue;
			}
			else
			{
				if (vecTrace.m_vecDir.Dot(pEnt->GetGlobalTransform().GetForwardVector()) < 0)
					continue;
			}
		}

		Matrix4x4 mRotation = pEnt->GetGlobalTRS().GetMatrix4x4(true, false);
		Matrix4x4 mInverseRotation = mRotation.InvertedRT();

		Ray vecLocalTrace = mInverseRotation * vecTrace;

		if (EditorPhysics()->IsEntityAdded(pEnt))
			continue;

		Vector vecIntersection;
		if (!RayIntersectsAABB(vecLocalTrace, aabbLocalBounds, vecIntersection))
			continue;

		float flDistanceSqr = (vecLocalTrace.m_vecPos-vecIntersection).LengthSqr();

		if (iNearest == ~0)
		{
			iNearest = i;
			flNearestSqr = flDistanceSqr;
			continue;
		}

		if (flDistanceSqr >= flNearestSqr)
			continue;

		iNearest = i;
		flNearestSqr = flDistanceSqr;
	}

	return iNearest;
}

void CLevelEditor::EntitySelected()
{
	if (!m_pLevel)
		return;

	size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();
	auto& aEntities = m_pLevel->GetEntityData();

	if (iSelected >= aEntities.size())
		return;

	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();
	m_flCreateObjectDistance = (vecCamera - aEntities[iSelected].GetGlobalTransform().GetTranslation()).Length();
}

void CLevelEditor::CreateEntityFromPanel(const Vector& vecPosition)
{
	auto& aEntityData = m_pLevel->GetEntityData();

	size_t iNewEntity = m_pLevel->CreateEntity(m_hCreateEntityPanel->m_hClass->GetText());
	CLevelEntity* pNewEntity = &m_pLevel->GetEntityData()[iNewEntity];

	pNewEntity->SetParameterValue("Name", m_hCreateEntityPanel->m_hNameText->GetText());

	pNewEntity->SetParameterValue("Model", m_hCreateEntityPanel->m_hModelText->GetText());
	pNewEntity->SetParameterValue("Origin", sprintf("%f %f %f", vecPosition.x, vecPosition.y, vecPosition.z));

	PopulateLevelEntityFromPanel(pNewEntity, m_hCreateEntityPanel->m_hPropertiesPanel);

	EditorPhysics()->AddEntity(pNewEntity, CT_STATIC_MESH);

	m_hEditorPanel->Layout();
	m_hEditorPanel->m_hEntities->SetSelectedNode(iNewEntity);
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
			bool bValue = pPanel->m_ahPropertyOptions[i].DowncastStatic<glgui::CCheckBox>()->GetState();

			if (bValue)
				pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], "1");
			else
				pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], "0");
		}
		else
		{
			tstring sValue = pPanel->m_ahPropertyOptions[i].DowncastStatic<glgui::CTextField>()->GetText();

			if (pPanel->m_asPropertyHandle[i] == "Model")
				CModelLibrary::AddModel(sValue);

			pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], sValue);
		}
	}
}

void CLevelEditor::DuplicateSelectedEntity()
{
	if (!m_hEditorPanel->m_hEntities->GetSelectedNode())
		return;

	size_t iNewHandle = m_pLevel->CopyEntity(m_pLevel->GetEntityData()[m_hEditorPanel->m_hEntities->GetSelectedNodeId()]);

	EditorPhysics()->AddEntity(&m_pLevel->GetEntityData()[iNewHandle], CT_STATIC_MESH);

	m_hEditorPanel->Layout();
	m_hEditorPanel->m_hEntities->SetSelectedNode(iNewHandle);
}

void CLevelEditor::CreateEntityCallback(const tstring& sArgs)
{
	m_hCreateEntityPanel->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_hCreateEntityPanel->GetWidth()/2, 72);
	m_hCreateEntityPanel->SetVisible(true);
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
		size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();
		auto& aEntities = m_pLevel->GetEntityData();

		m_hEditorPanel->m_hEntities->Unselect();
		if (iSelected < aEntities.size())
		{
			EditorPhysics()->RemoveEntity(&aEntities[iSelected]);
			aEntities.erase(aEntities.begin()+iSelected);
			m_hEditorPanel->Layout();
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

	// H for the same reason, my dvorak to qwerty key mapper
	if ((c == 'D' || c == 'H') && Application()->IsCtrlDown())
	{
		DuplicateSelectedEntity();
		return true;
	}

	return BaseClass::KeyPress(c);
}

bool CLevelEditor::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iState == TINKER_MOUSE_PRESSED && m_hCreateEntityPanel->IsVisible() && m_hCreateEntityPanel->m_bReadyToCreate)
	{
		CreateEntityFromPanel(PositionFromMouse());
		return true;
	}

	if (iState == TINKER_MOUSE_PRESSED && iButton == TINKER_KEY_MOUSE_LEFT)
	{
		size_t iSelected;
		if (ShouldRenderOrthographic())
		{
			int x, y;
			Application()->GetMousePosition(x, y);
			Vector vecPosition1 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
			Vector vecPosition0 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 0));

			iSelected = TraceLine(Ray(vecPosition0, (vecPosition1-vecPosition0).Normalized()));
		}
		else
		{
			Vector vecPosition = PositionFromMouse();
			Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

			iSelected = TraceLine(Ray(vecCamera, (vecPosition-vecCamera).Normalized()));
		}

		m_hEditorPanel->m_hEntities->SetSelectedNode(iSelected);

		return true;
	}

	return false;
}

void CLevelEditor::Activate()
{
	SetCameraOrientation(GameServer()->GetCameraManager()->GetCameraPosition(), GameServer()->GetCameraManager()->GetCameraDirection());

	LoadLevel(GameServer()->GetLevel(CVar::GetCVarValue("game_level")));

	m_hEditorPanel->SetVisible(true);
	m_hCreateEntityButton->SetVisible(true);

	GetFileMenu()->AddSubmenu("Save", this, SaveLevel);

	BaseClass::Activate();
}

void CLevelEditor::Deactivate()
{
	BaseClass::Deactivate();

	m_hEditorPanel->SetVisible(false);
	m_hCreateEntityButton->SetVisible(false);
	m_hCreateEntityPanel->SetVisible(false);

	if (m_pLevel && m_pLevel->GetEntityData().size())
		GameServer()->RestartLevel();
}

void CLevelEditor::RenderScene()
{
	if (!m_pLevel)
		return;

	TPROF("CLevelEditor::RenderEntities()");

	{
		CRenderingContext c(GameServer()->GetRenderer(), true);

		if (!GameServer()->GetRenderer()->IsDrawingBackground())
			GameServer()->GetRenderer()->DrawBackground(&c);

		c.UseProgram("model");

		c.SetUniform("bDiffuse", false);
		c.SetBlend(BLEND_ALPHA);

		c.SetUniform("vecColor", Vector4D(0.7f, 0.2f, 0.2f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(-10000, 0, 0));
			c.Vertex(Vector(10000, 0, 0));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(0.2f, 0.7f, 0.2f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(0, -10000, 0));
			c.Vertex(Vector(0, 10000, 0));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(0.2f, 0.2f, 0.7f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(0, 0, -10000));
			c.Vertex(Vector(0, 0, 10000));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(1.0f, 1.0f, 1.0f, 0.2f));

		int i;

		Vector vecStartX(-10, -10, 0);
		Vector vecEndX(-10, 10, 0);
		Vector vecStartZ(-10, -10, 0);
		Vector vecEndZ(10, -10, 0);

		c.BeginRenderLines();
		for (i = 0; i <= 20; i++)
		{
			if (i != 10)
			{
				c.Vertex(vecStartX);
				c.Vertex(vecEndX);
				c.Vertex(vecStartZ);
				c.Vertex(vecEndZ);
			}

			vecStartX.x += 1;
			vecEndX.x += 1;
			vecStartZ.y += 1;
			vecEndZ.y += 1;
		}
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
	}

	GameServer()->GetRenderer()->SetRenderingTransparent(false);

	auto& aEntityData = m_pLevel->GetEntityData();
	for (size_t i = 0; i < aEntityData.size(); i++)
		RenderEntity(i);

	GameServer()->GetRenderer()->SetRenderingTransparent(true);

	for (size_t i = 0; i < aEntityData.size(); i++)
		RenderEntity(i);

	if (m_hCreateEntityPanel->IsVisible() && m_hCreateEntityPanel->m_bReadyToCreate)
		RenderCreateEntityPreview();
}

void CLevelEditor::ManipulatorUpdated(const tstring& sArguments)
{
	// Grab this before GetToyToModify since that does a layout and clobbers the list.
	size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();

	tvector<tstring> asTokens;
	strtok(sArguments, asTokens);
	TAssert(asTokens.size() == 2);
	TAssert(stoi(asTokens[1]) == iSelected);
	TAssert(iSelected != ~0);

	if (!GetLevel())
		return;

	if (iSelected >= GetLevel()->GetEntityData().size())
		return;

	CLevelEntity* pEntity = &GetLevel()->GetEntityData()[iSelected];

	Vector vecTranslation = Manipulator()->GetTRS().m_vecTranslation;
	EAngle angRotation = Manipulator()->GetTRS().m_angRotation;
	Vector vecScaling = Manipulator()->GetTRS().m_vecScaling;

	pEntity->SetParameterValue("Origin", pretty_float(vecTranslation.x) + " " + pretty_float(vecTranslation.y) + " " + pretty_float(vecTranslation.z));
	pEntity->SetParameterValue("Angles", pretty_float(angRotation.p) + " " + pretty_float(angRotation.y) + " " + pretty_float(angRotation.r));
	pEntity->SetParameterValue("Scale", pretty_float(vecScaling.x) + " " + pretty_float(vecScaling.y) + " " + pretty_float(vecScaling.z));

	EditorPhysics()->SetEntityTransform(pEntity, pEntity->GetPhysicsTransform());

	m_hEditorPanel->LayoutEntity();
}

void CLevelEditor::DuplicateMove(const tstring& sArguments)
{
	auto& aEntityData = m_pLevel->GetEntityData();

	size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();

	if (iSelected >= aEntityData.size())
		return;

	Vector vecTranslation = Manipulator()->GetNewTRS().m_vecTranslation;
	EAngle angRotation = Manipulator()->GetNewTRS().m_angRotation;
	Vector vecScaling = Manipulator()->GetNewTRS().m_vecScaling;

	size_t iNewObject = m_pLevel->CopyEntity(aEntityData[iSelected]);

	m_hEditorPanel->Layout();

	CLevelEntity* pEntity = &GetLevel()->GetEntityData()[iNewObject];

	pEntity->SetParameterValue("Origin", pretty_float(vecTranslation.x) + " " + pretty_float(vecTranslation.y) + " " + pretty_float(vecTranslation.z));
	pEntity->SetParameterValue("Angles", pretty_float(angRotation.p) + " " + pretty_float(angRotation.y) + " " + pretty_float(angRotation.r));
	pEntity->SetParameterValue("Scale", pretty_float(vecScaling.x) + " " + pretty_float(vecScaling.y) + " " + pretty_float(vecScaling.z));

	EditorPhysics()->AddEntity(pEntity, CT_STATIC_MESH);
	EditorPhysics()->SetEntityTransform(pEntity, pEntity->GetPhysicsTransform());

	m_hEditorPanel->m_hEntities->SetSelectedNode(aEntityData.size()-1);

	m_hEditorPanel->LayoutEntity();
}
