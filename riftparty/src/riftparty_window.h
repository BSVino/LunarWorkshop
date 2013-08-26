#pragma once

#include <ui/gamewindow.h>

class CRiftPartyWindow : public CGameWindow
{
	DECLARE_CLASS(CRiftPartyWindow, CGameWindow);

public:
								CRiftPartyWindow(int argc, char** argv);

public:
	virtual tstring				WindowTitle() { return "Rift Party"; }
	virtual tstring				AppDirectory() { return "RiftParty"; }

	virtual void				RenderLoading();

	virtual class CRenderer*	CreateRenderer();
	class CRiftPartyRenderer*   GetRenderer();
	class CRiftPartyHUD*        GetHUD() { return m_pHUD; };

protected:
	class CRiftPartyHUD*        m_pHUD;
};

inline CRiftPartyWindow* RiftPartyWindow()
{
	return static_cast<CRiftPartyWindow*>(CApplication::Get());
}
