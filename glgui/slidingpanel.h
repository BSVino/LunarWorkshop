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
	
		public:
			virtual bool			IsVisible();

			CSlidingContainer*		m_pMaster;
		};

	public:
									CSlidingPanel(CSlidingContainer* pParent, char* pszTitle);

	public:
		virtual void				Layout();
		virtual void				Paint(float x, float y, float w, float h);

		virtual size_t				AddControl(CBaseControl* pControl, bool bToTail = false);

		virtual bool				MousePressed(int iButton, int mx, int my);

		virtual void				SetCurrent(bool bCurrent);

		static float				SLIDER_COLLAPSED_HEIGHT;

	protected:
		bool						m_bCurrent;

		CLabel*						m_pTitle;

		CPanel*						m_pInnerPanel;
	};

	class CSlidingContainer : public CPanel
	{
	public:
									CSlidingContainer();
									CSlidingContainer(float x, float y, float w, float h);

	public:
		virtual void				Layout();

		virtual size_t				AddControl(CBaseControl* pControl, bool bToTail = false);

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
