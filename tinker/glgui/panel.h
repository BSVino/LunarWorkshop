#ifndef TINKER_PANEL_H
#define TINKER_PANEL_H

#include "basecontrol.h"

#include <tinker_memory.h>

namespace glgui
{
	class CScrollBar;

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

		virtual CControlHandle	GetHasCursor();

		void					NextTabStop();

		virtual CControlHandle	AddControl(CBaseControl* pControl, bool bToTail = false);			// This function calls CreateControl for you.
		virtual CControlHandle	AddControl(CResource<CBaseControl> pControl, bool bToTail = false); // This function requires you to call CreateControl.
		virtual void			RemoveControl(CBaseControl* pControl);
		virtual tvector<CResource<CBaseControl>>&	GetControls() { return m_apControls; };
		virtual void			MoveToTop(CBaseControl* pControl);

		virtual void			SetHighlighted(bool bHighlight) { m_bHighlight = bHighlight; };
		virtual bool			IsHighlighted() { return m_bHighlight; };

		void					SetScissoring(bool b) { m_bScissoring = b; };
		bool					IsScissoring() const { return m_bScissoring; };

		FRect					GetControlBounds() const { return m_rControlBounds; };
		FRect					GetControlOffset() const { return m_rControlOffset; };

		void					SetVerticalScrollBarEnabled(bool b);
		void					SetHorizontalScrollBarEnabled(bool b);

		CControl<CScrollBar>	GetVerticalScrollBar() const;
		CControl<CScrollBar>	GetHorizontalScrollBar() const;

		bool					ShouldControlOffset(const CBaseControl* pControl) const;

	protected:
		tvector<CResource<CBaseControl>>	m_apControls;

		float					m_flMargin;

		// If two controls in the same panel are never layered, a single
		// pointer should suffice. Otherwise a list must be created.
		CControlHandle			m_hHasCursor;

		bool					m_bHighlight;
		bool					m_bScissoring;

		FRect					m_rControlBounds;	// Bounding box for all child controls but not children of children. Only valid after Layout()
		FRect					m_rControlOffset;	// w/h offset for children as determined by scrollbar

		CControl<CScrollBar>	m_hVerticalScrollBar;
		CControl<CScrollBar>	m_hHorizontalScrollBar;
	};
};

#endif
