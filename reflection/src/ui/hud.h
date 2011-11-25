#ifndef REFLECTION_HUD_H
#define REFLECTION_HUD_H

#include <ui/hudviewport.h>

class CReflectionHUD : public CHUDViewport
{
public:
	virtual void	Paint(float x, float y, float w, float h);
};

#endif
