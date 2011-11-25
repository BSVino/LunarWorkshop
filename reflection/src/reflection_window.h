#ifndef REFLECTION_WINDOW_H
#define REFLECTION_WINDOW_H

#include <tinker/gamewindow.h>

class CReflectionWindow : public CGameWindow
{
	DECLARE_CLASS(CReflectionWindow, CGameWindow);

public:
								CReflectionWindow(int argc, char** argv);

public:
	virtual eastl::string		WindowTitle() { return "Reflection"; }
	virtual tstring				AppDirectory() { return _T("Reflection"); }

	void						SetupReflection();

	virtual void				RenderLoading();

	class CReflectionRenderer*	GetRenderer();
	class CReflectionHUD*		GetHUD() { return m_pHUD; };

protected:
	class CReflectionHUD*		m_pHUD;
};

inline CReflectionWindow* ReflectionWindow()
{
	return static_cast<CReflectionWindow*>(CApplication::Get());
}

#endif
