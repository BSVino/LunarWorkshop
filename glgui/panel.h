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
								CPanel(float x, float y, float w, float h);
		virtual					~CPanel();

	public:
		virtual void			Paint();
		virtual void			Paint(float x, float y);
		virtual void			Paint(float x, float y, float w, float h);
		virtual void			Layout();
		virtual void			Think();
		virtual void			UpdateScene();

		virtual bool			KeyPressed(int code, bool bCtrlDown = false);
		virtual bool			KeyReleased(int code);
		virtual bool			CharPressed(int iKey);
		virtual bool			MousePressed(int code, int mx, int my);
		virtual bool			MouseReleased(int code, int mx, int my);
		virtual bool			IsCursorListener() {return true;};
		virtual void			CursorMoved(int mx, int my);
		virtual void			CursorOut();

		virtual IControl*		GetHasCursor();

		virtual void			AddControl(IControl* pControl, bool bToTail = false);
		virtual void			RemoveControl(IControl* pControl);
		virtual eastl::vector<IControl*>&	GetControls() { return m_apControls; };
		virtual void			MoveToTop(IControl* pControl);

		virtual void			SetHighlighted(bool bHighlight) { m_bHighlight = bHighlight; };
		virtual bool			IsHighlighted() { return m_bHighlight; };

		typedef enum
		{
			BT_NONE	= 0,
			BT_SOME = 1
		} Border;

		void					SetBorder(Border b) { m_eBorder = b; };

	protected:
		virtual void			PaintBorder(float x, float y, float w, float h);

		eastl::vector<IControl*>	m_apControls;

		// If two controls in the same panel are never layered, a single
		// pointer should suffice. Otherwise a list must be created.
		IControl*				m_pHasCursor;

		Border					m_eBorder;

		bool					m_bHighlight;
		bool					m_bDestructing;
	};
};

#endif
