#ifndef TINKER_ROOTPANEL_H
#define TINKER_ROOTPANEL_H

#include "glgui.h"
#include "panel.h"

class CRenderingContext;

namespace glgui
{
	class CRootPanel : public CPanel
	{
		DECLARE_CLASS(CRootPanel, CPanel);

	public:
									CRootPanel( );
		virtual						~CRootPanel( );

	public:
		virtual void				CreateControls(CResource<CBaseControl> pThis);

		virtual void				Think(double flTime);
		virtual void				UpdateScene();
		virtual void				Paint(float x, float y, float w, float h);
		virtual void				Layout();

		virtual bool				MousePressed(int code, int mx, int my, bool bInsideControl = false);
		virtual bool				MouseReleased(int code, int mx, int my);
		virtual bool				MouseDoubleClicked(int code, int mx, int my);
		virtual void				CursorMoved(int mx, int my);

		// Dragon Drop stuff is in this class, because this is always the
		// top-level panel so all the messages go through it first.
		virtual void				DragonDrop(IDroppable* pDroppable);
		virtual void				AddDroppable(IDroppable* pDroppable);
		virtual void				RemoveDroppable(IDroppable* pDroppable);
		virtual bool				DropDraggable();
		virtual IDraggable*			GetCurrentDraggable() { return m_pDragging?m_pDragging->GetCurrentDraggable():NULL; };
		virtual IDroppable*			GetCurrentDroppable() { return m_pDragging; };

		bool						SetFocus(CControlHandle hControl);

		void						SetButtonDown(CControl<CButton> pButton);
		CControl<CButton>			GetButtonDown() const;

		CControl<CMenuBar>			GetMenuBar() { return m_hMenuBar; };
		CControl<CMenu>				AddMenu(const tstring& sText);

		void						SetLighting(bool bLighting) { m_bUseLighting = bLighting; };

		double						GetFrameTime() { return m_flFrameTime; };
		double						GetTime() { return m_flTime; };

		static CRootPanel*			Get();

		static void					GetFullscreenMousePos(int& mx, int& my);

		static ::CRenderingContext*	GetContext() { return Get()->m_pRenderingContext; }

	private:
		static CResource<CBaseControl>	s_pRootPanel;
		static bool					s_bRootPanelValid;

		tvector<IDroppable*>		m_apDroppables;
		IDroppable*					m_pDragging;

		// If the mouse is released over nothing, then try popping this button.
		CControl<CButton>			m_hButtonDown;

		CControlHandle				m_hFocus;

		CControl<CMenuBar>			m_hMenuBar;

		double						m_flFrameTime;
		double						m_flTime;

		int							m_iMX;
		int							m_iMY;

		bool						m_bUseLighting;

		::CRenderingContext*		m_pRenderingContext;
	};

	inline CRootPanel* RootPanel()
	{
		return CRootPanel::Get();
	}
};

#endif
