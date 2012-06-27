#pragma once

#include <glgui/panel.h>

class CProgressBar : public glgui::CPanel
{
	DECLARE_CLASS(CProgressBar, glgui::CPanel);

public:
							CProgressBar();

public:
	void					Layout();
	void					Paint(float x, float y, float w, float h);

	void					SetTotalProgress(size_t iProgress);
	void					SetProgress(size_t iProgress, const tstring& sAction = "");
	void					SetAction(const tstring& sAction);

	static CProgressBar*	Get();

protected:
	size_t					m_iTotalProgress;
	size_t					m_iCurrentProgress;

	glgui::CControl<glgui::CLabel>	m_hAction;
	tstring					m_sAction;

	static glgui::CControl<CProgressBar>	s_hProgressBar;
};
