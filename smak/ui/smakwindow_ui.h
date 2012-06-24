#pragma once

#include <glgui/movablepanel.h>

class CHelpPanel : public glgui::CMovablePanel
{
public:
							CHelpPanel();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CControl<glgui::CLabel>			m_hInfo;

	static glgui::CControl<CHelpPanel>		s_hHelpPanel;
};

class CAboutPanel : public glgui::CMovablePanel
{
public:
							CAboutPanel();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CControl<glgui::CLabel>			m_hInfo;

	static glgui::CControl<CAboutPanel>		s_hAboutPanel;
};
