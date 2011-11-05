#ifndef TINKER_ROOTPANEL_H
#define TINKER_ROOTPANEL_H

#include "panel.h"

namespace glgui
{
	class CRootPanel : public CPanel
	{
	public:
									CRootPanel( );
		virtual						~CRootPanel( );

	public:
		virtual void				Think(float flTime);
		virtual void				UpdateScene();
		virtual void				Paint(int x, int y, int w, int h);
		virtual void				Layout();

		virtual bool				MousePressed(int code, int mx, int my, bool bInsideControl = false);
		virtual bool				MouseReleased(int code, int mx, int my);
		virtual void				CursorMoved(int mx, int my);

		// Dragon Drop stuff is in this class, because this is always the
		// top-level panel so all the messages go through it first.
		virtual void				DragonDrop(IDroppable* pDroppable);
		virtual void				AddDroppable(IDroppable* pDroppable);
		virtual void				RemoveDroppable(IDroppable* pDroppable);
		virtual bool				DropDraggable();
		virtual IDraggable*			GetCurrentDraggable() { return m_pDragging?m_pDragging->GetCurrentDraggable():NULL; };
		virtual IDroppable*			GetCurrentDroppable() { return m_pDragging; };

		void						SetFocus(CBaseControl* pControl);

		virtual void				Popup(IPopup* pControl);

		void						SetButtonDown(class CButton* pButton);
		class CButton*				GetButtonDown();

		class CMenuBar*				GetMenuBar() { return m_pMenuBar; };
		class CMenu*				AddMenu(const tstring& sText);

		void						SetLighting(bool bLighting) { m_bUseLighting = bLighting; };

		float						GetFrameTime() { return m_flFrameTime; };
		float						GetTime() { return m_flTime; };

		static CRootPanel*			Get();

		// Should the window close after the selections have been made? (ie team choosing?)
		// If not, it should advance to the next window.
		static void					SetCloseAfter(bool bCloseAfter);
		static bool					GetCloseAfter();

		static void					GetFullscreenMousePos(int& mx, int& my);
		static void					DrawRect(int x, int y, int x2, int y2);

	private:
		static CRootPanel*			s_pRootPanel;

		eastl::vector<IDroppable*>	m_apDroppables;
		IDroppable*					m_pDragging;

		IPopup*						m_pPopup;

		// If the mouse is released over nothing, then try popping this button.
		class CButton*				m_pButtonDown;

		class CBaseControl*			m_pFocus;

		class CMenuBar*				m_pMenuBar;

		float						m_flFrameTime;
		float						m_flTime;

		int							m_iMX;
		int							m_iMY;

		bool						m_bUseLighting;
	};
};

#endif
