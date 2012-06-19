#pragma once

#include <ui/gamewindow.h>

class CGrottoWindow : public CGameWindow
{
	DECLARE_CLASS(CGrottoWindow, CGameWindow);

public:
								CGrottoWindow(int argc, char** argv);

public:
	virtual tstring				WindowTitle() { return "Grotto"; }
	virtual tstring				AppDirectory() { return "Grotto"; }

	class CRenderer*			CreateRenderer();

	void						SetupReflection();

	virtual void				RenderLoading();

	class CGrottoRenderer*		GetRenderer();
	class CGrottoHUD*			GetHUD() { return m_pHUD; };

protected:
	class CGrottoHUD*			m_pHUD;
};

inline CGrottoWindow* GrottoWindow()
{
	return static_cast<CGrottoWindow*>(CApplication::Get());
}
