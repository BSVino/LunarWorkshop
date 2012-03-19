#ifndef LW_TINKER_CONSOLE
#define LW_TINKER_CONSOLE

#include <glgui/panel.h>

class CConsole : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CConsole, glgui::CPanel);

public:
							CConsole();
	virtual					~CConsole();

public:
	virtual bool			IsVisible();
	virtual bool			IsChildVisible(IControl* pChild);
	virtual void			SetVisible(bool bVisible);
	virtual bool			IsOpen();

	virtual bool			IsCursorListener();

	virtual void			Layout();
	virtual void			Paint() { Paint(GetLeft(), GetTop(), GetWidth(), GetHeight()); };
	virtual void			Paint(float x, float y, float w, float h);

	void					PrintConsole(const tstring& sText);

	virtual bool			KeyPressed(int code, bool bCtrlDown = false);
	virtual bool			CharPressed(int iKey);

	EVENT_CALLBACK(CConsole, CommandChanged);

	virtual void			SetRenderBackground(bool bBackground) { m_bBackground = bBackground; }

protected:
	glgui::CLabel*			m_pOutput;
	glgui::CTextField*		m_pInput;

	bool					m_bBackground;

	eastl::vector<tstring>	m_asHistory;
	int						m_iHistory;
};

#endif
