#ifndef REFLECTION_HUD_H
#define REFLECTION_HUD_H

#include <glgui/panel.h>

class CReflectionHUD : public glgui::CPanel
{
public:
					CReflectionHUD();

public:
	virtual void	Paint(float x, float y, float w, float h);
};

#endif
