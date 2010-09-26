#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <string>
#include <vector>
#include <vector.h>
#include <color.h>

#include <game/digitanks/digitanksgame.h>

class CDigitanksWindow
{
public:
								CDigitanksWindow(int argc, char** argv);
								~CDigitanksWindow();

public:
	void						InitUI();

	void						CreateGame(gametype_t eGameType);
	void						DestroyGame();

	void						Run();	// Doesn't return

	void						Layout();

	static void					RenderCallback() { Get()->Render(); };
	void						Render();

	static void					WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	void						WindowResize(int x, int y);

	static void					MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	void						MouseMotion(int x, int y);

	static void					MouseInputCallback(int iButton, int iState) { Get()->MouseInput(iButton, iState); };
	void						MouseInput(int iButton, int iState);

	static void					MouseWheelCallback(int iState) { Get()->MouseWheel(iState); };
	void						MouseWheel(int iState);

	static void					KeyEventCallback(int c, int e) { Get()->KeyEvent(c, e); };
	void						KeyEvent(int c, int e);

	void						KeyPress(int c);
	void						KeyRelease(int c);

	static void					SpecialCallback(int k, int x, int y) { Get()->Special(k, x, y); };
	void						Special(int k, int x, int y);

	bool						IsCtrlDown();
	bool						IsAltDown();
	bool						IsShiftDown();

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	bool						GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit = NULL);

	void						GameOver(bool bPlayerWon);

	void						CloseApplication();

	class CDigitanksMenu*		GetMenu() { return m_pMenu; };
	class CDigitanksGame*		GetGame() { return m_pDigitanksGame; };
	class CHUD*					GetHUD() { return m_pHUD; };
	class CInstructor*			GetInstructor();
	class CVictoryPanel*		GetVictoryPanel() { return m_pVictory; };
	class CStoryPanel*			GetStoryPanel() { return m_pStory; };

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	static CDigitanksWindow*	Get() { return s_pDigitanksWindow; };

protected:
	int							m_iMouseStartX;
	int							m_iMouseStartY;

	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;

	class CDigitanksMenu*		m_pMenu;
	class CVictoryPanel*		m_pVictory;
	class CDonatePanel*			m_pDonate;
	class CStoryPanel*			m_pStory;

	class CDigitanksGame*		m_pDigitanksGame;

	class CHUD*					m_pHUD;

	class CInstructor*			m_pInstructor;

	static CDigitanksWindow*	s_pDigitanksWindow;

	int							m_iMouseMoved;

	std::vector<const char*>	m_apszCommandLine;
};

#endif