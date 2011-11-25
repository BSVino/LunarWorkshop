#ifndef CHAIN_HUD_H
#define CHAIN_HUD_H

#include <ui/hudviewport.h>

class CChainHUD : public CHUDViewport
{
public:
	virtual void	Paint(float x, float y, float w, float h);
};

#endif
