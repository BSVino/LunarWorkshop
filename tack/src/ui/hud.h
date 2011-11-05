#ifndef TACK_HUD_H
#define TACK_HUD_H

#include <glgui/panel.h>

class CTackHUD : public glgui::CPanel
{
public:
					CTackHUD();

public:
	virtual void	Paint(int x, int y, int w, int h);
};

#endif
