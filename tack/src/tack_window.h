#ifndef TACK_WINDOW_H
#define TACK_WINDOW_H

#include <tinker/gamewindow.h>

class CTackWindow : public CGameWindow
{
	DECLARE_CLASS(CTackWindow, CGameWindow);

public:
								CTackWindow(int argc, char** argv);

public:
	virtual tstring				WindowTitle() { return "Tack Johnson: Attorney At Law!"; }
	virtual tstring				AppDirectory() { return "TackJohnson"; }

	void						SetupEngine();
	void						SetupTack();

	virtual void				RenderLoading();

	class CTackRenderer*		GetRenderer();
	class CTackHUD*				GetHUD() { return m_pHUD; };

protected:
	class CTackHUD*				m_pHUD;
};

inline CTackWindow* TackWindow()
{
	return static_cast<CTackWindow*>(CApplication::Get());
}

#endif
