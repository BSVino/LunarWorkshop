#ifndef TINKER_PANEL_H
#define TINKER_PANEL_H

#include "basecontrol.h"

namespace glgui
{
	// A panel is a container for other controls. It is for organization
	// purposes only; it does not currently keep its children from drawing
	// outside of it.
	class CPanel : public CBaseControl
	{
		DECLARE_CLASS(CPanel, CBaseControl);

#ifdef _DEBUG
		// Just so CBaseControl can get at CPanel's textures for the purpose of debug paint methods.
		friend class CBaseControl;
#endif

	public:
								CPanel();
								CPanel(float x, float y, float w, float h);
		virtual					~CPanel();

	public:
		virtual void			Paint();
		virtual void			Paint(float x, float y);
		virtual void			Paint(float x, float y, float w, float h);
		virtual void			PostPaint();
		virtual void			Layout();
		virtual void			Think();
		virtual void			UpdateScene();

		virtual void			SetDefaultMargin(float flMargin) { m_flMargin = flMargin; }
		virtual float			GetDefaultMargin() { return m_flMargin; }

		virtual bool			KeyPressed(int code, bool bCtrlDown = false);
		virtual bool			KeyReleased(int code);
		virtual bool			CharPressed(int iKey);
		virtual bool			MousePressed(int code, int mx, int my);
		virtual bool			MouseReleased(int code, int mx, int my);
		virtual bool			MouseDoubleClicked(int code, int mx, int my);
		virtual bool			IsCursorListener() {return true;};
		virtual void			CursorMoved(int mx, int my);
		virtual void			CursorOut();

		virtual CBaseControl*	GetHasCursor();

		void					NextTabStop();

		virtual size_t			AddControl(CBaseControl* pControl, bool bToTail = false);
		virtual void			RemoveControl(CBaseControl* pControl);
		virtual tvector<CBaseControl*>&	GetControls() { return m_apControls; };
		virtual void			MoveToTop(CBaseControl* pControl);

		virtual void			SetHighlighted(bool bHighlight) { m_bHighlight = bHighlight; };
		virtual bool			IsHighlighted() { return m_bHighlight; };

		void					SetScissoring(bool b) { m_bScissoring = b; };
		bool					IsScissoring() const { return m_bScissoring; };

		FRect					GetControlBounds() const { return m_rControlBounds; };
		FRect					GetControlOffset() const { return m_rControlOffset; };

		void					SetVerticalScrollBarEnabled(bool b);
		void					SetHorizontalScrollBarEnabled(bool b);

		bool					ShouldControlOffset(CBaseControl* pControl) const;

	protected:
		tvector<CBaseControl*>	m_apControls;

		float					m_flMargin;

		// If two controls in the same panel are never layered, a single
		// pointer should suffice. Otherwise a list must be created.
		CBaseControl*			m_pHasCursor;

		bool					m_bHighlight;
		bool					m_bScissoring;
		bool					m_bDestructing;

		FRect					m_rControlBounds;	// Bounding box for all child controls but not children of children. Only valid after Layout()
		FRect					m_rControlOffset;	// w/h offset for children as determined by scrollbar

		class CScrollBar*		m_pVerticalScrollBar;
		class CScrollBar*		m_pHorizontalScrollBar;
	};
};

#endif
