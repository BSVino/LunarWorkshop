#ifndef TINKER_BUTTON_H
#define TINKER_BUTTON_H

#include "label.h"

namespace glgui
{
	class CButton : public CLabel
	{
		friend class CRootPanel;
		friend class CSlidingPanel;

	public:
						CButton(const tstring& sText, bool bToggle = false, const tstring& sFont="sans-serif", size_t iSize=13);
						CButton(float x, float y, float w, float h, const tstring& sText, bool bToggle = false, const tstring& sFont="sans-serif", size_t iSize=13);

	public:
		virtual void	Think();

		virtual void	Paint() { CLabel::Paint(); };
		virtual void	Paint(float x, float y, float w, float h);
		virtual void	PaintButton(float x, float y, float w, float h);

		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);
		virtual bool	IsCursorListener() {return true;};
		virtual void	CursorIn();
		virtual void	CursorOut();

		virtual bool	IsToggleButton() {return m_bToggle;};
		virtual void	SetToggleButton(bool bToggle);
		virtual void	SetToggleState(bool bState);
		virtual bool	GetToggleState() {return m_bToggleOn;};

		virtual bool	Push();
		virtual bool	Pop(bool bRegister = true, bool bRevert = false);
		virtual void	SetState(bool bDown, bool bRegister = true);
		virtual bool	GetState() {return m_bDown;};

		virtual void	SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		// Toggle buttons only
		virtual void	SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual IEventListener::Callback	GetClickedListenerCallback() { return m_pfnClickCallback; };
		virtual IEventListener*				GetClickedListener() { return m_pClickListener; };

		virtual bool	IsHighlighted() {return m_flHighlight > 0;};

		virtual void	SetButtonColor(Color clrButton) { m_clrButton = clrButton; };
		virtual void	SetDownColor(Color clrDown) { m_clrDown = clrDown; };

	protected:
		bool			m_bToggle;
		bool			m_bToggleOn;
		bool			m_bDown;
		float			m_flHighlightGoal;
		float			m_flHighlight;

		// Need multiple event listeners? Too bad! Make a list.
		IEventListener::Callback m_pfnClickCallback;
		IEventListener*	m_pClickListener;

		IEventListener::Callback m_pfnUnclickCallback;
		IEventListener*	m_pUnclickListener;

		Color			m_clrButton;
		Color			m_clrDown;
	};
};

#endif
