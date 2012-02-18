#pragma once

#include <tstring.h>
#include <glgui/panel.h>

class CEditorPanel : public glgui::CPanel
{
	DECLARE_CLASS(CEditorPanel, glgui::CPanel);

public:
							CEditorPanel();

public:
	void					Layout();

public:
	glgui::CTree*			m_pEntities;
};

class CLevelEditor
{
public:
							CLevelEditor();

public:
	void					RenderEntity(class CLevelEntity* pEntity, bool bTransparent);

public:
	static void				Toggle();
	static bool				IsActive();

	static void				Activate();
	static void				Deactivate();

	static void				RenderEntities();
	static void				Render();

protected:
	bool					m_bActive;
	bool					m_bWasMouseActive;

	class CLevel*			m_pLevel;

	glgui::CPanel*			m_pEditorPanel;
};

CLevelEditor* LevelEditor();
