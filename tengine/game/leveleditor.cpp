#include "leveleditor.h"

#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <tinker/profiler.h>

#include "level.h"
#include "gameserver.h"

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

	m_pCamera = new CEditorCamera();
}

CLevelEditor::~CLevelEditor()
{
	glgui::CRootPanel::Get()->RemoveControl(m_pEditorPanel);
	delete m_pEditorPanel;

	delete m_pCamera;
}

void CLevelEditor::RenderEntity(size_t i, bool bTransparent)
{
	CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];

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
		if (m_pEditorPanel->m_pEntities->GetSelectedNodeId() == i)
			r.SetColor(Color(255, 0, 0));

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
	else
	{
		r.UseProgram("model");
		if (m_pEditorPanel->m_pEntities->GetSelectedNodeId() == i)
			r.SetUniform("vecColor", Color(255, 0, 0));
		else
			r.SetUniform("vecColor", Color(255, 255, 255));
		r.SetUniform("bDiffuse", false);
		r.RenderWireBox(pEntity->GetBoundingBox());
	}
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

	LevelEditor()->m_bWasMouseActive = Application()->IsMouseCursorEnabled();
	Application()->SetMouseCursorEnabled(true);
}

void CLevelEditor::Deactivate()
{
	if (!LevelEditor())
		return;

	LevelEditor()->m_bActive = false;

	LevelEditor()->m_pEditorPanel->SetVisible(false);

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
