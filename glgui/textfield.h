#ifndef TINKER_TEXTFIELD_H
#define TINKER_TEXTFIELD_H

#include "basecontrol.h"

namespace glgui
{
	class CTextField : public CBaseControl
	{
		friend class CRootPanel;

	public:
						CTextField();
		virtual void	Delete() { delete this; };

		virtual void	Paint() { int x = 0, y = 0; GetAbsPos(x, y); Paint(x, y); };
		virtual void	Paint(int x, int y) { Paint(x, y, m_iW, m_iH); };
		virtual void	Paint(int x, int y, int w, int h);
		virtual void	DrawLine(const tchar* pszText, unsigned iLength, int x, int y, int w, int h);

		virtual bool	IsCursorListener() {return true;};

		virtual void	SetFocus(bool bFocus);

		virtual bool	CharPressed(int iKey);
		virtual bool	KeyPressed(int iKey, bool bCtrlDown = false);

		virtual void	SetContentsChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual void	UpdateContentsChangedListener();

		virtual void	FindRenderOffset();

		virtual bool	IsEnabled() {return m_bEnabled;};
		virtual void	SetEnabled(bool bEnabled) {m_bEnabled = bEnabled;};

		virtual void	SetText(const tstring& pszText);
		virtual void	AppendText(const tchar* pszText);
		virtual tstring	GetText();

		virtual void	SetCursorPosition(size_t iPosition);

		virtual void	SetFontFaceSize(int iSize);
		virtual int		GetFontFaceSize() { return m_iFontFaceSize; };

		virtual int		GetTextWidth();
		virtual float	GetTextHeight();
		virtual void	EnsureTextFits();

		virtual Color	GetFGColor();
		virtual void	SetFGColor(Color FGColor);
		virtual void	SetAlpha(int a);

	protected:
		bool			m_bEnabled;
		tstring	m_sText;
		Color			m_FGColor;

		float			m_flBlinkTime;

		int				m_iFontFaceSize;

		size_t			m_iCursor;

		float			m_flRenderOffset;

		IEventListener::Callback m_pfnContentsChangedCallback;
		IEventListener*	m_pContentsChangedListener;
	};
};

#endif
