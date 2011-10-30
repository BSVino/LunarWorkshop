#ifndef REFLECTION_HUD_H
#define REFLECTION_HUD_H

#include <glgui/glgui.h>

class CReflectionHUD : public glgui::CPanel
{
public:
					CReflectionHUD();

public:
	virtual void	Paint(int x, int y, int w, int h);
};

#endif
