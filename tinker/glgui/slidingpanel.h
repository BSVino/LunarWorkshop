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
									CInnerPanel(CControl<CSlidingContainer> hMaster);
	
		public:
			virtual bool			IsVisible();

			CControl<CSlidingContainer>	m_hMaster;
		};

	public:
									CSlidingPanel(CControl<CSlidingContainer> hParent, char* pszTitle);

	public:
		virtual void				Layout();
		virtual void				Paint(float x, float y, float w, float h);

		virtual CControlHandle		AddControl(CResource<CBaseControl> pControl, bool bToTail = false);

		virtual bool				MousePressed(int iButton, int mx, int my);

		virtual void				SetCurrent(bool bCurrent);

		static float				SLIDER_COLLAPSED_HEIGHT;

	protected:
		bool						m_bCurrent;

		CControl<CLabel>			m_hTitle;

		CControl<CPanel>			m_hInnerPanel;
	};

	class CSlidingContainer : public CPanel
	{
	public:
									CSlidingContainer();
									CSlidingContainer(float x, float y, float w, float h);

	public:
		virtual void				Layout();

		virtual CControlHandle		AddControl(CResource<CBaseControl> pControl, bool bToTail = false);

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