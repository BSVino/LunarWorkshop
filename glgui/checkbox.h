#ifndef TINKER_CHECKBOX_H
#define TINKER_CHECKBOX_H

#include "button.h"

namespace glgui
{
	class CCheckBox : public CButton
	{
	public:
						CCheckBox();

	public:
		void			Paint(int x, int y, int w, int h);
	};
};

#endif
