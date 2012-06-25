#ifndef TINKER_BASECONTROL_H
#define TINKER_BASECONTROL_H

#include "glgui.h"

#include <textures/materialhandle.h>
#include <tinker_memory.h>

namespace glgui
{
	class CBaseControl;
	typedef CHandle<CBaseControl> CControlHandle;

	class CBaseControl
	{
		friend CResource<CBaseControl> CreateControl(CBaseControl* pControl);

	public:
						CBaseControl(float x, float y, float w, float h);
						CBaseControl(const FRect& Rect);
		virtual			~CBaseControl();

	public:
		virtual CControlHandle	GetParent() const { return m_hParent; };
		virtual void	SetParent(CControlHandle hParent) { m_hParent = hParent; };

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
		virtual void	GetSize(float &w, float &h) const { w = m_flW; h = m_flH; };
		virtual void	GetPos(float &x, float &y) const { x = m_flX; y = m_flY; };
		virtual void	GetAbsPos(float &x, float &y) const;
		virtual void	GetAbsDimensions(float &x, float &y, float &w, float &h) const;
		virtual FRect	GetAbsDimensions() const;
		virtual float	GetWidth() const { return m_flW; };
		virtual float	GetHeight() const { return m_flH; };
		virtual void	SetDimensions(float x, float y, float w, float h) { m_flX = x; m_flY = y; m_flW = w; m_flH = h; };	// Local space
		virtual void	SetDimensions(const FRect& Dims) { SetDimensions(Dims.x, Dims.y, Dims.w, Dims.h); };	// Local space
		virtual void	GetBR(float &x, float &y) const { x = m_flX + m_flW; y = m_flY + m_flH; };
		virtual void	SetAlpha(int a) { m_iAlpha = a; };
		virtual int		GetAlpha() const { return m_iAlpha; };
		virtual void	SetWidth(float w);
		virtual void	SetHeight(float h);
		virtual void	SetLeft(float l);
		virtual void	SetTop(float t);
		virtual void	SetRight(float r);
		virtual void	SetBottom(float b);
		virtual float	GetLeft() const { return m_flX; };
		virtual float	GetTop() const { return m_flY; };
		virtual float	GetRight() const { return m_flX + m_flW; };
		virtual float	GetBottom() const { return m_flY + m_flH; };
		virtual void	Center();
		virtual void	CenterX();
		virtual void	CenterY();

		virtual float	GetDefaultMargin() { return 0; }
		virtual float	Layout_GetMargin(float flMargin);
		virtual void	Layout_FullWidth(float flMargin=g_flLayoutDefault);
		virtual void	Layout_AlignTop(CBaseControl* pOther=nullptr, float flMargin=g_flLayoutDefault);
		virtual void	Layout_AlignBottom(CBaseControl* pOther=nullptr, float flMargin=g_flLayoutDefault);
		virtual void	Layout_Column(int iTotalColumns, int iColumn, float flMargin=g_flLayoutDefault);						// Take up all horizontal space, save for the margins
		virtual void	Layout_ColumnFixed(int iTotalColumns, int iColumn, float flWidth, float flMargin=g_flLayoutDefault);	// Use a fixed width for each column and center columns

		virtual void	SetVisible(bool bVis);
		virtual bool	IsVisible();
		virtual bool	IsChildVisible(CBaseControl* pChild) { return true; }

		virtual void	LevelShutdown( void ) { return; };
		virtual bool	KeyPressed(int iKey, bool bCtrlDown = false) { return false; };
		virtual bool	KeyReleased(int iKey) { return false; };
		virtual bool	CharPressed(int iKey) { return false; };
		virtual bool	MousePressed(int iButton, int mx, int my);
		virtual bool	MouseReleased(int iButton, int mx, int my) { return false; };
		virtual bool	MouseDoubleClicked(int iButton, int mx, int my) { return false; };
		virtual bool	IsCursorListener();
		virtual void	CursorMoved(int x, int y) {};
		virtual void	CursorIn();
		virtual void	CursorOut();

		virtual CControlHandle	GetHasCursor() const;

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

		CControlHandle	GetHandle() const { return m_hThis; }
		virtual void	OnSetHandle() {}

		virtual void	CreateControls(CResource<CBaseControl> pThis) {}

		static CResource<CBaseControl> CreateControl(CBaseControl* pControl);

		static void		PaintRect(float x, float y, float w, float h, const Color& c = g_clrBox, int iBorder = 0, bool bHighlight = false);
		static void		PaintTexture(const CMaterialHandle& hTexture, float x, float y, float w, float h, const Color& c = Color(255, 255, 255, 255));
		static void		PaintSheet(const CMaterialHandle& hTexture, float x, float y, float w, float h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c = Color(255, 255, 255, 255));
		static void		MakeQuad();

	protected:
		CControlHandle	m_hParent;

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

		CControlHandle	m_hThis;

		static size_t	s_iQuad;
	};

	template <class C>
	class CControl
	{
	public:
		CControl()
		{
		}

		CControl(CControlHandle hControl)
		{
			if (hControl.expired())
				return;

			if (dynamic_cast<C*>(hControl.lock().get()))
				m_hControl = hControl;
		}

	public:
		template <class T>
		T* Downcast() const
		{
			if (m_hControl.expired())
				return nullptr;

			C* p = static_cast<C*>(m_hControl.lock().get());
			return dynamic_cast<T*>(p);
		}

		template <class T>
		T* DowncastStatic() const
		{
			if (m_hControl.expired())
				return nullptr;

			C* p = static_cast<C*>(m_hControl.lock().get());
			return static_cast<T*>(p);
		}

		CControl<C> operator=(CControlHandle hControl)
		{
			if (hControl.expired())
				return CControl<C>();

			if (dynamic_cast<C*>(hControl.lock().get()))
				m_hControl = hControl;

			return *this;
		}

		CControlHandle GetHandle() const
		{
			return m_hControl;
		}

		operator CControlHandle() const
		{
			return m_hControl;
		}

		C* Get() const
		{
			return static_cast<C*>(m_hControl.Get());
		}

		C* operator->()
		{
			return static_cast<C*>(m_hControl.Get());
		}

		const C* operator->() const
		{
			return static_cast<C*>(m_hControl.Get());
		}

		operator C*() const
		{
			return static_cast<C*>(m_hControl.Get());
		}

		bool operator!() const
		{
			return m_hControl.expired();
		}

	protected:
		CControlHandle	m_hControl;
	};
};

#endif
