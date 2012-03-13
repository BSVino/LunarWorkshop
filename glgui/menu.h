#ifndef TINKER_MENU_H
#define TINKER_MENU_H

#include "panel.h"
#include "button.h"

namespace glgui
{
#define MENU_SPACING 10
#define MENU_HEIGHT 22

	class CMenuBar : public CPanel
	{
	public:
									CMenuBar();

		void						Layout();

		void						SetActive(class CMenu* pMenu);
	};

	class CMenu : public CButton, public IEventListener
	{
		DECLARE_CLASS(CMenu, CButton);

	public:
									CMenu(const tstring& sTitle, bool bSubmenu = false);
		virtual						~CMenu();

	public:
		virtual void				Think();
		virtual void				Layout();
		virtual void				Paint(float x, float y, float w, float h);

		virtual bool				IsCursorListener() { return true; };
		virtual void				CursorIn();
		virtual void				CursorOut();

		virtual void				SetMenuListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		EVENT_CALLBACK(CMenu, Open);
		EVENT_CALLBACK(CMenu, Close);
		EVENT_CALLBACK(CMenu, Clicked);

		void						OpenMenu();
		void						CloseMenu();

		virtual void				AddSubmenu(const tstring& sTitle, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);
		virtual void				ClearSubmenus();

		virtual size_t				GetSelectedMenu();

	protected:
		class CSubmenuPanel : public CPanel
		{
			DECLARE_CLASS(CSubmenuPanel, CPanel);
		public:
									CSubmenuPanel(CMenu* pMenu);

			void					Think();

			virtual bool			IsVisible();

			void					SetFakeHeight(float flFakeHeight) { m_flFakeHeight = flFakeHeight; };

		protected:
			float					m_flFakeHeight;

			eastl::vector<float>	m_aflControlHighlightGoal;
			eastl::vector<float>	m_aflControlHighlight;

			CMenu*					m_pMenu;
		};

		bool						m_bSubmenu;

		float						m_flHighlightGoal;
		float						m_flHighlight;

		float						m_flMenuHighlightGoal;
		float						m_flMenuHighlight;
		float						m_flMenuHeightGoal;
		float						m_flMenuHeight;
		float						m_flMenuSelectionHighlightGoal;
		float						m_flMenuSelectionHighlight;
		FRect						m_MenuSelectionGoal;
		FRect						m_MenuSelection;

		IEventListener::Callback	m_pfnMenuCallback;
		IEventListener*				m_pMenuListener;

		CSubmenuPanel*				m_pMenu;

		eastl::vector<CMenu*>		m_apEntries;
	};
};

#endif
