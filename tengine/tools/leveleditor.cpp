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
			AddControl(m_apPropertyLabels.back(), true);
			m_apPropertyLabels.back()->SetLeft(0);
			m_apPropertyLabels.back()->SetTop(flTop);
			m_apPropertyLabels.back()->SetWidth(10);
			m_apPropertyLabels.back()->EnsureTextFits();

			if (strcmp(pSaveData->m_pszType, "bool") == 0)
			{
				glgui::CCheckBox* pCheckbox = new glgui::CCheckBox();
				m_apPropertyOptions.push_back(pCheckbox);
				AddControl(pCheckbox, true);
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
				AddControl(pTextField, true);
				pTextField->Layout_FullWidth(0);
				pTextField->SetWidth(pTextField->GetWidth()-15);
				pTextField->SetTop(flTop+12);

				if (strcmp(pSaveData->m_pszHandle, "Model") == 0)
					pTextField->SetContentsChangedListener(this, ModelChanged, sprintf("%d", i));
				else if (tstr_startswith(pSaveData->m_pszType, "CEntityHandle"))
					pTextField->SetContentsChangedListener(this, TargetChanged, sprintf("%d ", i) + pSaveData->m_pszType);
				else
					pTextField->SetContentsChangedListener(this, PropertyChanged);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
				{
					pTextField->SetText(m_pEntity->GetParameterValue(pSaveData->m_pszHandle));

					if (strcmp(pSaveData->m_pszType, "Vector") == 0 && CanUnserializeString_TVector(pTextField->GetText()))
					{
						Vector v = UnserializeString_TVector(pTextField->GetText());
						pTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0 && CanUnserializeString_EAngle(pTextField->GetText()))
					{
						EAngle v = UnserializeString_EAngle(pTextField->GetText());
						pTextField->SetText(pretty_float(v.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.r, 4));
					}
				}
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
						pTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					{
						Vector2D v = *((Vector2D*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0)
					{
						EAngle v = *((EAngle*)&pSaveData->m_oDefault[0]);
						pTextField->SetText(pretty_float(v.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.r, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					{
						Matrix4x4 m = *((Matrix4x4*)&pSaveData->m_oDefault[0]);
						EAngle e = m.GetAngles();
						Vector v = m.GetTranslation();
						pTextField->SetText(pretty_float(e.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(e.r, 4) + " " + pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "AABB") == 0)
					{
						AABB b = *((AABB*)&pSaveData->m_oDefault[0]);
						Vector v1 = b.m_vecMins;
						Vector v2 = b.m_vecMaxs;
						pTextField->SetText(pretty_float(v1.x, 4) + " " + pretty_float(v1.y, 4) + " " + pretty_float(v1.z, 4) + " " + pretty_float(v2.x, 4) + " " + pretty_float(v2.y, 4) + " " + pretty_float(v2.z, 4));
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
	eastl::vector<tstring> asExtensions;
	eastl::vector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".mat");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	static_cast<glgui::CTextField*>(m_apPropertyOptions[stoi(sArgs)])->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);

	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::TargetChangedCallback(const tstring& sArgs)
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	eastl::vector<tstring> asTokens;
	tstrtok(sArgs, asTokens, "<>");

	TAssert(asTokens.size() == 2);
	if (asTokens.size() != 2)
		return;

	eastl::vector<tstring> asTargets;

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

	static_cast<glgui::CTextField*>(m_apPropertyOptions[stoi(sArgs)])->SetAutoCompleteCommands(asTargets);

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
	asExtensions.push_back(".mat");
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

	m_pSlider = new glgui::CSlidingContainer();
	AddControl(m_pSlider);

	m_pPropertiesSlider = new glgui::CSlidingPanel(m_pSlider, "Properties");
	m_pOutputsSlider = new glgui::CSlidingPanel(m_pSlider, "Outputs");

	m_pPropertiesPanel = new CEntityPropertiesPanel(true);
	m_pPropertiesPanel->SetBackgroundColor(Color(10, 10, 10, 50));
	m_pPropertiesPanel->SetPropertyChangedListener(this, PropertyChanged);
	m_pPropertiesSlider->AddControl(m_pPropertiesPanel);

	m_pOutputs = new glgui::CTree();
	m_pOutputs->SetBackgroundColor(Color(0, 0, 0, 100));
	m_pOutputs->SetSelectedListener(this, OutputSelected);
	m_pOutputsSlider->AddControl(m_pOutputs);

	m_pAddOutput = new glgui::CButton("Add");
	m_pAddOutput->SetClickedListener(this, AddOutput);
	m_pOutputsSlider->AddControl(m_pAddOutput);

	m_pRemoveOutput = new glgui::CButton("Remove");
	m_pRemoveOutput->SetClickedListener(this, RemoveOutput);
	m_pOutputsSlider->AddControl(m_pRemoveOutput);

	m_pOutput = new glgui::CMenu("Choose Output");
	m_pOutputsSlider->AddControl(m_pOutput);

	m_pOutputEntityNameLabel = new glgui::CLabel("Target Entity:", "sans-serif", 10);
	m_pOutputEntityNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pOutputsSlider->AddControl(m_pOutputEntityNameLabel);
	m_pOutputEntityNameText = new glgui::CTextField();
	m_pOutputEntityNameText->SetContentsChangedListener(this, TargetEntityChanged);
	m_pOutputsSlider->AddControl(m_pOutputEntityNameText);

	m_pInput = new glgui::CMenu("Choose Input");
	m_pOutputsSlider->AddControl(m_pInput);

	m_pOutputArgsLabel = new glgui::CLabel("Arguments:", "sans-serif", 10);
	m_pOutputArgsLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pOutputsSlider->AddControl(m_pOutputArgsLabel);
	m_pOutputArgsText = new glgui::CTextField();
	m_pOutputArgsText->SetContentsChangedListener(this, ArgumentsChanged);
	m_pOutputsSlider->AddControl(m_pOutputArgsText);
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
			tstring sModel = oEntity.GetParameterValue("Model");

			if (sName.length())
				m_pEntities->AddNode(oEntity.GetClass() + ": " + sName);
			else if (sModel.length())
				m_pEntities->AddNode(oEntity.GetClass() + " (" + GetFilename(sModel) + ")");
			else
				m_pEntities->AddNode(oEntity.GetClass());
		}
	}

	m_pObjectTitle->SetPos(0, 220);
	m_pObjectTitle->SetSize(GetWidth(), 25);

	float flTempMargin = 5;
	m_pSlider->Layout_AlignTop(m_pObjectTitle, flTempMargin);
	m_pSlider->Layout_FullWidth(flTempMargin);
	m_pSlider->SetBottom(GetHeight() - flTempMargin);

	flTempMargin = 2;
	m_pPropertiesPanel->Layout_AlignTop(nullptr, flTempMargin);
	m_pPropertiesPanel->Layout_FullWidth(flTempMargin);

	LayoutEntity();

	BaseClass::Layout();
}

void CEditorPanel::LayoutEntity()
{
	m_pObjectTitle->SetText("(No Object Selected)");
	m_pPropertiesPanel->SetVisible(false);
	m_pPropertiesPanel->SetEntity(nullptr);

	CLevelEntity* pEntity = GetCurrentEntity();

	if (pEntity)
	{
		if (pEntity->GetName().length())
			m_pObjectTitle->SetText(pEntity->GetClass() + ": " + pEntity->GetName());
		else
			m_pObjectTitle->SetText(pEntity->GetClass());

		m_pPropertiesPanel->SetClass("C" + pEntity->GetClass());
		m_pPropertiesPanel->SetEntity(pEntity);
		m_pPropertiesPanel->SetVisible(true);

		m_pOutputs->ClearTree();

		auto& aEntityOutputs = pEntity->GetOutputs();

		for (size_t i = 0; i < aEntityOutputs.size(); i++)
		{
			auto& oEntityOutput = aEntityOutputs[i];

			m_pOutputs->AddNode(oEntityOutput.m_sOutput + " -> " + oEntityOutput.m_sTargetName + ":" + oEntityOutput.m_sInput);
		}

		m_pOutputs->Layout();
	}

	LayoutOutput();
}

void CEditorPanel::LayoutOutput()
{
	m_pAddOutput->SetEnabled(false);
	m_pRemoveOutput->SetEnabled(false);
	m_pOutput->SetEnabled(false);
	m_pOutputEntityNameText->SetEnabled(false);
	m_pInput->SetEnabled(false);
	m_pOutputArgsText->SetEnabled(false);

	m_pOutputs->Layout_AlignTop();
	m_pOutputs->Layout_FullWidth();
	m_pOutputs->SetHeight(50);

	m_pAddOutput->SetHeight(15);
	m_pAddOutput->Layout_AlignTop(m_pOutputs);
	m_pAddOutput->Layout_Column(2, 0);
	m_pRemoveOutput->SetHeight(15);
	m_pRemoveOutput->Layout_AlignTop(m_pOutputs);
	m_pRemoveOutput->Layout_Column(2, 1);

	m_pOutput->Layout_AlignTop(m_pAddOutput, 10);
	m_pOutput->ClearSubmenus();
	m_pOutput->SetSize(100, 30);
	m_pOutput->CenterX();
	m_pOutput->SetText("Choose Output");

	m_pOutputEntityNameLabel->Layout_AlignTop(m_pOutput);
	m_pOutputEntityNameText->SetTop(m_pOutputEntityNameLabel->GetTop()+12);
	m_pOutputEntityNameText->SetText("");
	m_pOutputEntityNameText->Layout_FullWidth();

	m_pInput->Layout_AlignTop(m_pOutputEntityNameText, 10);
	m_pInput->ClearSubmenus();
	m_pInput->SetSize(100, 30);
	m_pInput->CenterX();
	m_pInput->SetText("Choose Input");

	m_pOutputArgsLabel->Layout_AlignTop(m_pInput);
	m_pOutputArgsText->SetTop(m_pOutputArgsLabel->GetTop()+12);
	m_pOutputArgsText->SetText("");
	m_pOutputArgsText->Layout_FullWidth();

	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	m_pAddOutput->SetEnabled(true);
	m_pRemoveOutput->SetEnabled(true);

	CLevelEntity::CLevelEntityOutput* pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	if (pOutput->m_sTargetName.length())
		m_pOutputEntityNameText->SetText(pOutput->m_sTargetName);

	if (pOutput->m_sArgs.length())
		m_pOutputArgsText->SetText(pOutput->m_sArgs);

	if (pOutput->m_sOutput.length())
		m_pOutput->SetText(pOutput->m_sOutput);

	if (pOutput->m_sInput.length())
		m_pInput->SetText(pOutput->m_sInput);

	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity("C" + pEntity->GetClass());
	TAssert(pRegistration);
	if (!pRegistration)
		return;

	m_pOutput->SetEnabled(true);
	m_pOutputEntityNameText->SetEnabled(true);
	m_pInput->SetEnabled(true);
	m_pOutputArgsText->SetEnabled(true);

	m_pOutput->ClearSubmenus();

	do {
		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			auto& oSaveData = pRegistration->m_aSaveData[i];
			if (oSaveData.m_eType != CSaveData::DATA_OUTPUT)
				continue;

			m_pOutput->AddSubmenu(oSaveData.m_pszHandle, this, ChooseOutput);
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
	if (m_pOutputEntityNameText->GetText()[0] == '*')
		pRegistration = CBaseEntity::GetRegisteredEntity("C" + m_pOutputEntityNameText->GetText().substr(1));
	else if (m_pOutputEntityNameText->GetText().length())
	{
		CLevelEntity* pTarget = nullptr;

		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			if (!pLevel->GetEntityData()[i].GetName().length())
				continue;

			if (pLevel->GetEntityData()[i].GetName() == m_pOutputEntityNameText->GetText())
			{
				pTarget = &pLevel->GetEntityData()[i];
				break;
			}
		}

		if (pTarget)
			pRegistration = CBaseEntity::GetRegisteredEntity("C" + pTarget->GetClass());
	}

	m_pInput->ClearSubmenus();

	if (pRegistration)
	{
		do {
			for (auto it = pRegistration->m_aInputs.begin(); it != pRegistration->m_aInputs.end(); it++)
				m_pInput->AddSubmenu(it->first, this, ChooseInput);

			if (!pRegistration->m_pszParentClass)
				break;

			pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
		} while (pRegistration);
	}
}

CLevelEntity* CEditorPanel::GetCurrentEntity()
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return nullptr;

	auto& aEntities = pLevel->GetEntityData();

	if (m_pEntities->GetSelectedNodeId() >= aEntities.size())
		return nullptr;

	return &aEntities[m_pEntities->GetSelectedNodeId()];
}

CLevelEntity::CLevelEntityOutput* CEditorPanel::GetCurrentOutput()
{
	CLevelEntity* pEntity = GetCurrentEntity();

	auto& aEntityOutputs = pEntity->GetOutputs();

	if (m_pOutputs->GetSelectedNodeId() >= aEntityOutputs.size())
		return nullptr;

	return &aEntityOutputs[m_pOutputs->GetSelectedNodeId()];
}

void CEditorPanel::EntitySelectedCallback(const tstring& sArgs)
{
	LayoutEntity();

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
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	if (pEntity)
	{
		CLevelEditor::PopulateLevelEntityFromPanel(pEntity, m_pPropertiesPanel);

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

	m_pOutputs->SetSelectedNode(pEntity->GetOutputs().size()-1);
}

void CEditorPanel::RemoveOutputCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	pEntity->GetOutputs().erase(pEntity->GetOutputs().begin()+m_pOutputs->GetSelectedNodeId());
}

void CEditorPanel::ChooseOutputCallback(const tstring& sArgs)
{
	m_pOutput->Pop(true, true);

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	eastl::vector<tstring> asTokens;
	tstrtok(sArgs, asTokens);
	pOutput->m_sOutput = asTokens[1];
	m_pOutput->SetText(pOutput->m_sOutput);

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

	pOutput->m_sTargetName = m_pOutputEntityNameText->GetText();

	eastl::vector<tstring> asTargets;

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

	m_pOutputEntityNameText->SetAutoCompleteCommands(asTargets);

	LayoutInput();
}

void CEditorPanel::ChooseInputCallback(const tstring& sArgs)
{
	m_pInput->Pop(true, true);

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	eastl::vector<tstring> asTokens;
	tstrtok(sArgs, asTokens);
	pOutput->m_sInput = asTokens[1];
	m_pInput->SetText(pOutput->m_sInput);
}

void CEditorPanel::ArgumentsChangedCallback(const tstring& sArgs)
{
	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	pOutput->m_sArgs = m_pOutputArgsText->GetText();
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

	m_pCreateEntityButton = new glgui::CPictureButton("Create", CMaterialLibrary::AddAsset("editor/create-entity.mat"));
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
		oCopy.SetGlobalTransform(Manipulator()->GetTransform(true, false));	// Scaling is already done in RenderEntity()
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
		if (GameServer()->GetRenderer()->IsRenderingTransparent())
		{
			TPROF("CLevelEditor::RenderModel(Material)");
			r.UseProgram("model");
			r.SetUniform("bDiffuse", true);
			if (bSelected)
				r.SetColor(Color(255, 0, 0, (char)(255*flAlpha)));
			else
				r.SetColor(Color(255, 255, 255, (char)(255*flAlpha)));

			r.Scale(0, vecScale.y, vecScale.x);
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

		r.Scale(vecScale.x, vecScale.y, vecScale.z);

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
	if (!m_pLevel)
		return;

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

void CLevelEditor::DuplicateSelectedEntity()
{
	if (!m_pEditorPanel->m_pEntities->GetSelectedNode())
		return;

	auto& aEntityData = m_pLevel->GetEntityData();
	auto& oNewEntity = aEntityData.push_back();
	oNewEntity = aEntityData[m_pEditorPanel->m_pEntities->GetSelectedNodeId()];

	m_pEditorPanel->Layout();
	m_pEditorPanel->m_pEntities->SetSelectedNode(aEntityData.size()-1);
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

	// H for the same reason, my dvorak to qwerty key mapper
	if ((c == 'D' || c == 'H') && Application()->IsCtrlDown())
	{
		DuplicateSelectedEntity();
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
	SetCameraOrientation(GameServer()->GetCameraManager()->GetCameraPosition(), GameServer()->GetCameraManager()->GetCameraDirection());

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
	if (Workbench()->GetCameraManager()->GetFreeMode())
	{
		m_vecEditCamera = Workbench()->GetCameraManager()->GetFreeCameraPosition();
		m_angEditCamera = Workbench()->GetCameraManager()->GetFreeCameraAngles();
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
	Vector vecScaling = Manipulator()->GetTRS().m_vecScaling;
	GetLevel()->GetEntityData()[iSelected].SetParameterValue("LocalOrigin", pretty_float(vecTranslation.x) + " " + pretty_float(vecTranslation.y) + " " + pretty_float(vecTranslation.z));
	GetLevel()->GetEntityData()[iSelected].SetParameterValue("LocalAngles", pretty_float(angRotation.p) + " " + pretty_float(angRotation.y) + " " + pretty_float(angRotation.r));
	GetLevel()->GetEntityData()[iSelected].SetParameterValue("Scale", pretty_float(vecScaling.x) + " " + pretty_float(vecScaling.y) + " " + pretty_float(vecScaling.z));

	m_pEditorPanel->LayoutEntity();
}
