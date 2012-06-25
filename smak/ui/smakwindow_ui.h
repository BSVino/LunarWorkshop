#pragma once

#include <glgui/movablepanel.h>

class CHelpPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CHelpPanel, glgui::CMovablePanel);

public:
							CHelpPanel();

public:
	virtual void			CreateControls(CResource<glgui::CBaseControl> pThis);

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CControl<glgui::CLabel>			m_hInfo;

	static CResource<CBaseControl>		s_pHelpPanel;
};

class CAboutPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CAboutPanel, glgui::CMovablePanel);

public:
							CAboutPanel();

public:
	virtual void			CreateControls(CResource<glgui::CBaseControl> pThis);

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	glgui::CControl<glgui::CLabel>			m_hInfo;

	static CResource<CBaseControl>		s_pAboutPanel;
};
