#ifndef TINKER_GAME_WINDOW_H
#define TINKER_GAME_WINDOW_H

#include <tinker/application.h>

#include <common.h>

class CGameWindow : public CApplication
{
	DECLARE_CLASS(CGameWindow, CApplication);

public:
								CGameWindow(int argc, char** argv);
	virtual						~CGameWindow();

public:
	void						OpenWindow();

	void						CreateGame(const tstring& sRequestedGameType);
	void						DestroyGame();
	void						Restart(tstring sGameType);
	void						ReloadLevel();

	void						Run();
	virtual void				PreFrame();
	virtual void				PostFrame();
	virtual void				RenderLoading() {};

	virtual void				Render();

	virtual bool				KeyPress(int c);
	virtual void				KeyRelease(int c);

	virtual void				MouseMotion(int x, int y);
	virtual bool				MouseInput(int iButton, int iState);

	class CGameServer*			GetGameServer() { return m_pGameServer; };
	class CGameRenderer*		GetRenderer() { return m_pRenderer; };

	void						OpenChat();
	void						CloseChat();
	void						ToggleChat();
	bool						IsChatOpen();
	void						PrintChat(tstring sText);
	virtual class CChatBox*		GetChatBox();

protected:
	class CGameServer*			m_pGameServer;
	class CGameRenderer*		m_pRenderer;

	class CChatBox*				m_pChatBox;

	bool						m_bHaveLastMouse;
	int							m_iLastMouseX;
	int							m_iLastMouseY;

	class CHUDViewport*			m_pHUD;

	tstring						m_sRestartGameMode;
};

inline CGameWindow* GameWindow()
{
	return dynamic_cast<CGameWindow*>(CApplication::Get());
}

#endif
