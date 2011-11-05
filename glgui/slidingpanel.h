#ifndef TINKER_SLIDINGPANEL_H
#define TINKER_SLIDINGPANEL_H

#include "panel.h"

namespace glgui
{
	class CSlidingContainer;
	class CSlidingPanel : public CPanel
	{
	public:
		friend class CRootPanel;

		class CInnerPanel : public CPanel
		{
		public:
									CInnerPanel(CSlidingContainer* pMaster);
			
			virtual void			Delete() { delete this; };

			virtual bool			IsVisible();

			CSlidingContainer*		m_pMaster;
		};

									CSlidingPanel(CSlidingContainer* pParent, char* pszTitle);

		virtual void				Delete() { delete this; };

		virtual void				Layout();
		virtual void				Paint(int x, int y, int w, int h);

		virtual void				AddControl(IControl* pControl, bool bToTail = false);

		virtual bool				MousePressed(int iButton, int mx, int my);

		virtual void				SetCurrent(bool bCurrent);

//		virtual void				SetTitle(char* pszNew) { m_pTitle->SetText(pszNew); };
//		virtual void				SetTitle(tchar* pszNew) { m_pTitle->SetText(pszNew); };
//		virtual void				AppendTitle(char* pszNew) { m_pTitle->AppendText(pszNew); };
//		virtual void				AppendTitle(tchar* pszNew) { m_pTitle->AppendText(pszNew); };

		static const int			SLIDER_COLLAPSED_HEIGHT = 30;

	protected:
		bool						m_bCurrent;

//		CLabel*						m_pTitle;

		CPanel*						m_pInnerPanel;
	};

	class CSlidingContainer : public CPanel
	{
	public:
									CSlidingContainer(int x, int y, int w, int h);

		virtual void				Delete() { delete this; };

		virtual void				Layout();

		virtual void				AddControl(IControl* pControl, bool bToTail = false);

		virtual bool				IsCurrent(int iPanel);
		virtual void				SetCurrent(int iPanel);
		virtual bool				IsCurrent(CSlidingPanel* pPanel);
		virtual void				SetCurrent(CSlidingPanel* pPanel);

		virtual bool				IsCurrentValid();

		virtual int					VisiblePanels();

	protected:
		int							m_iCurrent;
	};
};

#endif
