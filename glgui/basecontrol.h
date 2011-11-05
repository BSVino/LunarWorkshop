#ifndef TINKER_BASECONTROL_H
#define TINKER_BASECONTROL_H

#include "glgui.h"

namespace glgui
{
	class CBaseControl : public IControl
	{
	public:
						CBaseControl(int x, int y, int w, int h);
						CBaseControl(const FRect& Rect);
		virtual			~CBaseControl() {};

	public:
		virtual void	Destructor();
		virtual void	Delete() { delete this; };

		virtual IControl*	GetParent() { return m_pParent; };
		virtual void	SetParent(IControl* pParent) { m_pParent = pParent; };

		virtual void	LoadTextures() {};

		virtual void	Paint();
		virtual void	Paint(int x, int y);
		virtual void	Paint(int x, int y, int w, int h);
		virtual void	Layout() {};
		virtual void	Think() {};
		virtual void	UpdateScene() {};

		virtual void	SetSize(int w, int h) { m_iW = w; m_iH = h; };
		virtual void	SetPos(int x, int y) { m_iX = x; m_iY = y; };
		virtual void	GetSize(int &w, int &h) { w = m_iW; h = m_iH; };
		virtual void	GetPos(int &x, int &y) { x = m_iX; y = m_iY; };
		virtual void	GetAbsPos(int &x, int &y);
		virtual void	GetAbsDimensions(int &x, int &y, int &w, int &h);
		virtual FRect	GetAbsDimensions();
		virtual int		GetWidth() { return m_iW; };
		virtual int		GetHeight() { return m_iH; };
		virtual void	SetDimensions(int x, int y, int w, int h) { m_iX = x; m_iY = y; m_iW = w; m_iH = h; };	// Local space
		virtual void	SetDimensions(const FRect& Dims) { SetDimensions((int)Dims.x, (int)Dims.y, (int)Dims.w, (int)Dims.h); };	// Local space
		virtual void	GetBR(int &x, int &y) { x = m_iX + m_iW; y = m_iY + m_iH; };
		virtual void	SetAlpha(int a) { m_iAlpha = a; };
		virtual int		GetAlpha() { return m_iAlpha; };
		virtual void	SetRight(int r);
		virtual void	SetBottom(int b);
		virtual int		GetLeft() { return m_iX; };
		virtual int		GetTop() { return m_iY; };
		virtual int		GetRight() { return m_iX + m_iW; };
		virtual int		GetBottom() { return m_iY + m_iH; };

		virtual void	SetVisible(bool bVis);
		virtual bool	IsVisible();

		virtual void	LevelShutdown( void ) { return; };
		virtual bool	KeyPressed(int iKey, bool bCtrlDown = false) { return false; };
		virtual bool	KeyReleased(int iKey) { return false; };
		virtual bool	CharPressed(int iKey) { return false; };
		virtual bool	MousePressed(int iButton, int mx, int my);
		virtual bool	MouseReleased(int iButton, int mx, int my) { return false; };
		virtual bool	IsCursorListener();
		virtual void	CursorMoved(int x, int y) {};
		virtual void	CursorIn();
		virtual void	CursorOut();

		virtual IControl*	GetHasCursor();

		virtual void	SetFocus(bool bFocus) { m_bFocus = bFocus; };
		virtual bool	HasFocus() { return m_bFocus; };

		virtual void	SetCursorInListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual void	SetCursorOutListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual void	SetTooltip(const tstring& sTip);
		virtual tstring	GetTooltip() { return m_sTip; };

		static void		PaintRect(int x, int y, int w, int h, const Color& c = g_clrBox);
		static void		PaintTexture(size_t iTexture, int x, int y, int w, int h, const Color& c = Color(255, 255, 255, 255));
		static void		PaintSheet(size_t iTexture, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c = Color(255, 255, 255, 255));

	protected:
		IControl*		m_pParent;

		int				m_iX;
		int				m_iY;
		int				m_iW;
		int				m_iH;

		int				m_iAlpha;

		bool			m_bVisible;

		float			m_flMouseInTime;

		IEventListener::Callback m_pfnCursorInCallback;
		IEventListener*	m_pCursorInListener;

		IEventListener::Callback m_pfnCursorOutCallback;
		IEventListener*	m_pCursorOutListener;

		bool			m_bFocus;

		tstring	m_sTip;
	};
};

#endif
