#ifndef TINKER_PICTUREBUTTON_H
#define TINKER_PICTUREBUTTON_H

#include "button.h"

#include <textures/materialhandle.h>

namespace glgui
{
	class CPictureButton : public CButton
	{
	public:
						CPictureButton(const tstring& sText, const CMaterialHandle& hMaterial = CMaterialHandle(), bool bToggle = false);

	public:
		virtual void	Paint() { CButton::Paint(); };
		virtual void	Paint(float x, float y, float w, float h);

		virtual void	SetTexture(const CMaterialHandle& hMaterial);
		virtual void	SetSheetTexture(const CMaterialHandle& hMaterial, int sx, int sy, int sw, int sh, int tw, int th);
		virtual void	SetSheetTexture(const CMaterialHandle& hMaterial, const Rect& rArea, int tw, int th);
		virtual void	ShowBackground(bool bShow) { m_bShowBackground = bShow; };

	protected:
		CMaterialHandle	m_hMaterial;
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
