#include "menu.h"

#include <glgui/rootpanel.h>
#include <glgui/button.h>
#include <glgui/tree.h>
#include <glgui/label.h>

#include "../chain_game.h"
#include "../story.h"

using namespace glgui;

CChainMenu::CChainMenu()
	: m_pPagesLabel(new CLabel("Pages")), m_pPages(new CTree(0, 0, 0)), m_pExit(new CButton("Quit to Desktop"))
{
	SetBorder(CPanel::BT_SOME);
	SetBackgroundColor(Color(20, 20, 20, 200));
	SetVisible(false);

	AddControl(m_pPagesLabel.get());
	AddControl(m_pPages.get());
	AddControl(m_pExit.get());

	m_pPages->SetSelectedListener(this, PageSelected);
	m_pExit->SetClickedListener(this, Exit);
}

void CChainMenu::Layout()
{
	float flRootHeight = CRootPanel::Get()->GetHeight();

	SetSize(300, flRootHeight-100);
	SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, flRootHeight/2 - GetHeight()/2);

	m_pPagesLabel->SetPos(GetWidth()/2 - m_pPagesLabel->GetWidth()/2, 10);
	m_pPages->SetWidth(250);
	m_pPages->SetHeight(GetHeight() - m_pExit->GetHeight() - m_pPagesLabel->GetHeight() - 40);
	m_pPages->SetPos(GetWidth()/2 - m_pPages->GetWidth()/2, 20 + m_pPagesLabel->GetHeight());
	m_pExit->SetPos(GetWidth()/2 - m_pExit->GetWidth()/2, GetHeight() - m_pExit->GetHeight() - 10);

	m_pPages->ClearTree();

	if (!ChainGame())
		return;

	CStory* pStory = ChainGame()->GetStory();

	if (!pStory)
		return;

	for (size_t i = 0; i < pStory->GetNumPages(); i++)
		m_pPages->AddNode(pStory->GetPageID(i));

	BaseClass::Layout();
}

void CChainMenu::PageSelectedCallback(const tstring& sArgs)
{
	int iPage = stoi(sArgs);

	if (!ChainGame())
		return;

	CStory* pStory = ChainGame()->GetStory();

	if (!pStory)
		return;

	pStory->SetPage(pStory->GetPageID(iPage));

	SetVisible(false);
}

void CChainMenu::ExitCallback(const tstring& sArgs)
{
	exit(0);
}
