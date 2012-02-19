#include "levelselector.h"

#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <glgui/tree.h>
#include <ui/gamewindow.h>
#include <tinker/cvar.h>

#include <game/gameserver.h>
#include <game/level.h>

using namespace glgui;

CLevelSelector::CLevelSelector()
{
	SetBorder(BT_SOME);
	SetBackgroundColor(Color(37, 37, 37, 255));

	SetWidth(400);

	CLabel* pLabel = new CLabel("Choose a level");
	pLabel->SetTop(5);
	pLabel->SetWidth(GetWidth());
	pLabel->SetAlign(CLabel::TA_TOPCENTER);
	AddControl(pLabel);

	m_pLevels = new CTree(0, 0, 0);
	m_pLevels->SetLeft(15);
	m_pLevels->SetTop(25);
	m_pLevels->SetWidth(250);
	m_pLevels->SetSelectedListener(this, Selected);
	AddControl(m_pLevels);
}

void CLevelSelector::Layout()
{
	SetPos(glgui::CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, 50);
	SetBottom(glgui::CRootPanel::Get()->GetHeight()-50);

	m_pLevels->SetHeight(GetHeight()-50);
	m_pLevels->SetWidth(GetWidth()-50);
	m_pLevels->SetLeft(GetWidth()/2-m_pLevels->GetWidth()/2);

	m_pLevels->ClearTree();
	for (size_t i = 0; i < GameServer()->GetNumLevels(); i++)
		m_pLevels->AddNode(GameServer()->GetLevel(i)->GetName() + " (" + GameServer()->GetLevel(i)->GetFile() + ")");

	BaseClass::Layout();
}

void CLevelSelector::Paint(float x, float y, float w, float h)
{
	BaseClass::Paint(x, y, w, h);
}

void CLevelSelector::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	if (bVisible)
		CApplication::Get()->SetMouseCursorEnabled(true);
	else
		CApplication::Get()->SetMouseCursorEnabled(false);
}

void CLevelSelector::SelectedCallback(const tstring& sArgs)
{
	size_t iLevel = m_pLevels->GetSelectedNodeId();

	CLevel* pLevel = GameServer()->GetLevel(iLevel);
	if (!pLevel)
		return;

	GameWindow()->Restart("level");
	CVar::SetCVar("game_level", pLevel->GetFile());

	m_pLevels->Unselect();
}
