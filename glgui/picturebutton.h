#ifndef TINKER_PICTUREBUTTON_H
#define TINKER_PICTUREBUTTON_H

#include "button.h"

namespace glgui
{
	class CPictureButton : public CButton
	{
	public:
						CPictureButton(const tstring& sText, size_t iTexture = 0, bool bToggle = false);

	public:
		virtual void	Paint() { CButton::Paint(); };
		virtual void	Paint(int x, int y, int w, int h);

		virtual void	SetTexture(size_t iTexture);
		virtual void	SetSheetTexture(size_t iSheet, int sx, int sy, int sw, int sh, int tw, int th);
		virtual void	SetSheetTexture(size_t iSheet, const Rect& rArea, int tw, int th);
		virtual void	ShowBackground(bool bShow) { m_bShowBackground = bShow; };

	protected:
		size_t			m_iTexture;
		bool			m_bShowBackground;

		bool			m_bSheet;
		int				m_iSX;
		int				m_iSY;
		int				m_iSW;
		int				m_iSH;
		int				m_iTW;
		int				m_iTH;
	};
};

#endif
