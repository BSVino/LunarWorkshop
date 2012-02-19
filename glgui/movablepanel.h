#pragma once

#include <glgui/panel.h>
#include <glgui/button.h>
#include <glgui/selector.h>

#define HEADER_HEIGHT 16

namespace glgui
{
	class CCloseButton : public glgui::CButton
	{
	public:
								CCloseButton() : glgui::CButton(0, 0, 10, 10, "") {};

	public:
		virtual void			Paint() { glgui::CButton::Paint(); };
		virtual void			Paint(float x, float y, float w, float h);
	};

	class CMinimizeButton : public glgui::CButton
	{
	public:
								CMinimizeButton() : glgui::CButton(0, 0, 10, 10, "") {};

	public:
		virtual void			Paint() { glgui::CButton::Paint(); };
		virtual void			Paint(float x, float y, float w, float h);
	};

	class CMovablePanel : public glgui::CPanel, public glgui::IEventListener
	{
		DECLARE_CLASS(CMovablePanel, glgui::CPanel);

	public:
								CMovablePanel(const tstring& sName);
								~CMovablePanel();

		virtual void			Layout();

		virtual void			Think();

		virtual void			PaintBackground(float x, float y, float w, float h);
		virtual void			Paint(float x, float y, float w, float h);

		virtual bool			MousePressed(int iButton, int mx, int my);
		virtual bool			MouseReleased(int iButton, int mx, int my);

		virtual void			HasCloseButton(bool bHasClose) { m_bHasCloseButton = bHasClose; };
		virtual void			Minimize();

		virtual bool			IsChildVisible(IControl* pChild);

		virtual void			SetClearBackground(bool bClearBackground);
		virtual void			SetHeaderColor(const Color& clrHeader) { m_clrHeader = clrHeader; }

		EVENT_CALLBACK(CMovablePanel, MinimizeWindow);
		EVENT_CALLBACK(CMovablePanel, CloseWindow);

	protected:
		int						m_iMouseStartX;
		int						m_iMouseStartY;
		float					m_flStartX;
		float					m_flStartY;
		bool					m_bMoving;

		bool					m_bHasCloseButton;
		bool					m_bMinimized;
		float					m_flNonMinimizedHeight;
		float					m_flMinimizeTransitionLerp;

		bool					m_bClearBackground;

		glgui::CLabel*			m_pName;

		CCloseButton*			m_pCloseButton;
		CMinimizeButton*		m_pMinimizeButton;

		Color					m_clrHeader;
	};
}