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
	AddControl(m_pEntities);
}

void CEditorPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flCurrLeft = 20;
	float flCurrTop = 20;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-40);

	m_pEntities->SetPos(10, GetHeight()-210);
	m_pEntities->SetSize(GetWidth() - 20, 200);

	m_pEntities->ClearTree();

	CLevel* pLevel = GameServer()->GetLevel(CVar::GetCVarValue("game_level"));

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

	BaseClass::Layout();
}

CLevelEditor::CLevelEditor()
{
	m_bActive = false;
	m_pEditorPanel = new CEditorPanel();
	m_pEditorPanel->SetVisible(false);
	m_pEditorPanel->SetBackgroundColor(Color(0, 0, 0, 150));
	m_pEditorPanel->SetBorder(glgui::CPanel::BT_SOME);
	glgui::CRootPanel::Get()->AddControl(m_pEditorPanel);
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bTransparent)
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
	if (!LevelEditor())
		return false;

	return LevelEditor()->m_bActive;
}

void CLevelEditor::Activate()
{
	if (!LevelEditor())
		return;

	LevelEditor()->m_bActive = true;

	LevelEditor()->m_pEditorPanel->SetVisible(true);

	LevelEditor()->m_bWasMouseActive = Application()->IsMouseCursorEnabled();
	Application()->SetMouseCursorEnabled(true);

	LevelEditor()->m_pLevel = GameServer()->GetLevel(CVar::GetCVarValue("game_level"));
}

void CLevelEditor::Deactivate()
{
	if (!LevelEditor())
		return;

	LevelEditor()->m_bActive = false;

	LevelEditor()->m_pEditorPanel->SetVisible(false);

	Application()->SetMouseCursorEnabled(LevelEditor()->m_bWasMouseActive);
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
		LevelEditor()->RenderEntity(&aEntityData[i], false);
	}

	for (size_t i = 0; i < aEntityData.size(); i++)
	{
		LevelEditor()->RenderEntity(&aEntityData[i], true);
	}
}

void CLevelEditor::Render()
{
	if (!LevelEditor())
		return;

	if (!IsActive())
		return;
}

CLevelEditor* LevelEditor()
{
	// This function won't work unless we're in dev mode.
	// I don't want memory wasted on the level editor for most players.
	if (!CVar::GetCVarBool("developer"))
		return nullptr;

	static CLevelEditor* pLevelEditor = new CLevelEditor();

	return pLevelEditor;
}
