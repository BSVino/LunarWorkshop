#ifndef CHAIN_WINDOW_H
#define CHAIN_WINDOW_H

#include <tinker/gamewindow.h>

class CChainWindow : public CGameWindow
{
	DECLARE_CLASS(CChainWindow, CGameWindow);

public:
								CChainWindow(int argc, char** argv);

public:
	virtual eastl::string		WindowTitle() { return "Chain"; }
	virtual tstring				AppDirectory() { return "Chain"; }

	void						OpenWindow();

	void						SetupEngine();
	void						SetupChain();

	virtual void				RenderLoading();

	class CChainRenderer*		GetRenderer();
	class CChainHUD*			GetHUD() { return m_pHUD; };

protected:
	class CChainHUD*			m_pHUD;
};

inline CChainWindow* ChainWindow()
{
	return static_cast<CChainWindow*>(CApplication::Get());
}

#endif
