#ifndef REFLECTION_HUD_H
#define REFLECTION_HUD_H

#include <ui/hudviewport.h>

class CLevelSelector;

class CReflectionHUD : public CHUDViewport
{
	DECLARE_CLASS(CReflectionHUD, CHUDViewport);

public:
					CReflectionHUD();

public:
	virtual void	Paint(float x, float y, float w, float h);

protected:
	CLevelSelector*		m_pSelector;
};

#endif
