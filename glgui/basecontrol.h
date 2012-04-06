#ifndef TINKER_BASECONTROL_H
#define TINKER_BASECONTROL_H

#include "glgui.h"

#include <textures/texturehandle.h>

namespace glgui
{
	class CBaseControl : public IControl
	{
	public:
						CBaseControl(float x, float y, float w, float h);
						CBaseControl(const FRect& Rect);
		virtual			~CBaseControl();

	public:
		virtual IControl*	GetParent() { return m_pParent; };
		virtual void	SetParent(IControl* pParent) { m_pParent = pParent; };

		virtual void	LoadTextures() {};

		virtual void	Paint();
		virtual void	Paint(float x, float y);
		virtual void	Paint(float x, float y, float w, float h);
		virtual void	PaintBackground(float x, float y, float w, float h);
		virtual void	PostPaint() {};
		virtual void	Layout() {};
		virtual void	Think() {};
		virtual void	UpdateScene() {};

		virtual void	SetSize(float w, float h) { m_flW = w; m_flH = h; };
		virtual void	SetPos(float x, float y) { m_flX = x; m_flY = y; };
		virtual void	GetSize(float &w, float &h) { w = m_flW; h = m_flH; };
		virtual void	GetPos(float &x, float &y) { x = m_flX; y = m_flY; };
		virtual void	GetAbsPos(float &x, float &y);
		virtual void	GetAbsDimensions(float &x, float &y, float &w, float &h);
		virtual FRect	GetAbsDimensions();
		virtual float	GetWidth() { return m_flW; };
		virtual float	GetHeight() { return m_flH; };
		virtual void	SetDimensions(float x, float y, float w, float h) { m_flX = x; m_flY = y; m_flW = w; m_flH = h; };	// Local space
		virtual void	SetDimensions(const FRect& Dims) { SetDimensions(Dims.x, Dims.y, Dims.w, Dims.h); };	// Local space
		virtual void	GetBR(float &x, float &y) { x = m_flX + m_flW; y = m_flY + m_flH; };
		virtual void	SetAlpha(int a) { m_iAlpha = a; };
		virtual int		GetAlpha() { return m_iAlpha; };
		virtual void	SetWidth(float w);
		virtual void	SetHeight(float h);
		virtual void	SetLeft(float l);
		virtual void	SetTop(float t);
		virtual void	SetRight(float r);
		virtual void	SetBottom(float b);
		virtual float	GetLeft() { return m_flX; };
		virtual float	GetTop() { return m_flY; };
		virtual float	GetRight() { return m_flX + m_flW; };
		virtual float	GetBottom() { return m_flY + m_flH; };
		virtual void	Center();
		virtual void	CenterX();
		virtual void	CenterY();

		virtual float	GetDefaultMargin() { return 0; }
		virtual float	Layout_GetMargin(float flMargin);
		virtual void	Layout_FullWidth(float flMargin=g_flLayoutDefault);
		virtual void	Layout_AlignTop(CBaseControl* pOther=NULL, float flMargin=g_flLayoutDefault);
		virtual void	Layout_AlignBottom(CBaseControl* pOther=NULL, float flMargin=g_flLayoutDefault);
		virtual void	Layout_Column(int iTotalColumns, int iColumn, float flMargin=g_flLayoutDefault);						// Take up all horizontal space, save for the margins
		virtual void	Layout_ColumnFixed(int iTotalColumns, int iColumn, float flWidth, float flMargin=g_flLayoutDefault);	// Use a fixed width for each column and center columns

		virtual void	SetVisible(bool bVis);
		virtual bool	IsVisible();
		virtual bool	IsChildVisible(IControl* pChild) { return true; }

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

		virtual bool	TakesFocus() { return false; }
		virtual bool	SetFocus(bool bFocus) { m_bFocus = bFocus; return TakesFocus(); };
		virtual bool	HasFocus() { return m_bFocus; };

		virtual void	SetCursorInListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual void	SetCursorOutListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual void	SetTooltip(const tstring& sTip);
		virtual tstring	GetTooltip() { return m_sTip; };

		void			SetBackgroundColor(Color c) { m_clrBackground = c; };

		typedef enum
		{
			BT_NONE	= 0,
			BT_SOME = 1
		} Border;

		void			SetBorder(Border b) { m_eBorder = b; };

		static void		PaintRect(float x, float y, float w, float h, const Color& c = g_clrBox, int iBorder = 0, bool bHighlight = false);
		static void		PaintTexture(const CTextureHandle& hTexture, float x, float y, float w, float h, const Color& c = Color(255, 255, 255, 255));
		static void		PaintSheet(const CTextureHandle& hTexture, float x, float y, float w, float h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c = Color(255, 255, 255, 255));
		static void		MakeQuad();

	protected:
		IControl*		m_pParent;

		float			m_flX;
		float			m_flY;
		float			m_flW;
		float			m_flH;

		int				m_iAlpha;

		bool			m_bVisible;

		double			m_flMouseInTime;

		IEventListener::Callback m_pfnCursorInCallback;
		IEventListener*	m_pCursorInListener;

		IEventListener::Callback m_pfnCursorOutCallback;
		IEventListener*	m_pCursorOutListener;

		bool			m_bFocus;

		tstring			m_sTip;

		Color			m_clrBackground;
		Border			m_eBorder;

		static size_t	s_iQuad;
	};
};

#endif
