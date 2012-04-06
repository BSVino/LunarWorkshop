#ifndef TINKER_APPLICATION_H
#define TINKER_APPLICATION_H

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <common.h>
#include <vector.h>
#include <color.h>
#include <configfile.h>

#include "shell.h"

class CApplication : public CShell
{
public:
								CApplication(int argc, char** argv);
	virtual 					~CApplication();

public:
	void						SetMultisampling(bool bMultisampling) { m_bMultisampling = bMultisampling; }

	void						OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable);

	void						DumpGLInfo();

	virtual eastl::string		WindowTitle() { return "Tinker"; }
	virtual tstring				AppDirectory() { return "Tinker"; }

	void						SwapBuffers();
	double						GetTime();

	bool						IsOpen();
	void						Close();

	bool						HasFocus();

	static void					RenderCallback() { Get()->Render(); };
	virtual void				Render();

	static int					WindowCloseCallback(void*) { return Get()->WindowClose(); };
	virtual int					WindowClose();

	static void					WindowResizeCallback(void*, int x, int y) { Get()->WindowResize(x, y); };
	virtual void				WindowResize(int x, int y);

	static void					MouseMotionCallback(void*, int x, int y) { Get()->MouseMotion(x, y); };
	virtual void				MouseMotion(int x, int y);

	static void					MouseInputCallback(void*, int iButton, int iState);
	virtual bool				MouseInput(int iButton, int iState);

	static void					MouseWheelCallback(void*, int x, int y);
	virtual void				MouseWheel(int x, int y) {};

	static void					KeyEventCallback(void*, int c, int e) { Get()->KeyEvent(c, e); };
	void						KeyEvent(int c, int e);
	virtual bool				KeyPress(int c);
	virtual void				KeyRelease(int c);

	static void					CharEventCallback(void*, int c) { Get()->CharEvent(c); };
	void						CharEvent(int c);

	virtual bool				DoKeyPress(int c) { return false; };
	virtual void				DoKeyRelease(int c) {};

	virtual bool				DoCharPress(int c) { return false; };

	bool						IsCtrlDown();
	bool						IsAltDown();
	bool						IsShiftDown();
	bool						IsMouseLeftDown();
	bool						IsMouseRightDown();
	bool						IsMouseMiddleDown();
	void						GetMousePosition(int& x, int& y);

	void						SetMouseCursorEnabled(bool bEnabled);
	bool						IsMouseCursorEnabled();

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	bool						IsFullscreen() { return m_bFullscreen; };

	virtual void				OnClientDisconnect(int iClient) {};

	void						InitRegistrationFile();
	virtual bool				IsRegistered();
	void						ReadProductCode();
	eastl::string				GetProductCode();
	void						SetLicenseKey(eastl::string sKey);
	bool						QueryRegistrationKey(tstring sServer, tstring sURI, tstring sKey, eastl::string sProduct, tstring& sError);
	void						SaveProductCode();
	eastl::string				GenerateCode();

	static void					OpenConsole();
	static void					CloseConsole();
	static void					ToggleConsole();
	static bool					IsConsoleOpen();
	virtual void				PrintConsole(const tstring& sText);
	virtual void				PrintError(const tstring& sText);
	class CConsole*				GetConsole();

	static CApplication*		Get() { return s_pApplication; };

protected:
	size_t						m_pWindow;
	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;
	bool						m_bFullscreen;
	bool						m_bIsOpen;

	bool						m_bMultisampling;

	ConfigFile					m_oRegFile;
	eastl::string				m_sCode;
	eastl::string				m_sKey;

	bool						m_bMouseEnabled;
	bool						m_bMouseDownInGUI;

	class CConsole*				m_pConsole;

	static CApplication*		s_pApplication;
};

inline CApplication* Application()
{
	return CApplication::Get();
}

#endif
