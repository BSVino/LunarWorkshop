#pragma once

#include <glgui/panel.h>

typedef enum
{
	BA_TOP,
	BA_BOTTOM,
} buttonalignment_t;

class CButtonPanel : public glgui::CPanel
{
public:
							CButtonPanel(buttonalignment_t eAlign);

public:
	virtual void			Layout();

	virtual glgui::CControl<glgui::CButton>	AddButton(glgui::CButton* pButton, const tstring& sHints, bool bNewSection, glgui::IEventListener* pListener = NULL, glgui::IEventListener::Callback pfnCallback = NULL);

	virtual void			Think();
	virtual void			Paint(float x, float y, float w, float h);

protected:
	buttonalignment_t		m_eAlign;

	tvector<float>			m_aflSpaces;
	tvector<glgui::CControl<glgui::CButton>>	m_ahButtons;
	tvector<glgui::CControl<glgui::CLabel>>		m_ahHints;
};
