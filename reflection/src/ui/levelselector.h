#pragma once

#include <glgui/panel.h>

class glgui::CTree;

class CLevelSelector : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CLevelSelector, glgui::CPanel);

public:
					CLevelSelector();

public:
	virtual void	Layout();

	virtual void	Paint(float x, float y, float w, float h);

	EVENT_CALLBACK(CLevelSelector, Selected);

protected:
	glgui::CTree*	m_pLevels;
};
