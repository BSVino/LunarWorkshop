#ifndef CHAIN_HUD_H
#define CHAIN_HUD_H

#include <glgui/panel.h>

class CChainHUD : public glgui::CPanel
{
public:
					CChainHUD();

public:
	virtual void	Paint(float x, float y, float w, float h);
};

#endif
