#pragma once

#include <glgui/panel.h>

class CChainMenu : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CChainMenu, glgui::CPanel);

public:
					CChainMenu();

public:
	virtual void	Layout();

	EVENT_CALLBACK(CChainMenu, PageSelected);
	EVENT_CALLBACK(CChainMenu, Exit);

protected:
	std::shared_ptr<class glgui::CLabel>	m_pPagesLabel;
	std::shared_ptr<class glgui::CTree>		m_pPages;
	std::shared_ptr<class glgui::CButton>	m_pExit;
};
