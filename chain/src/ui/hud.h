#ifndef CHAIN_HUD_H
#define CHAIN_HUD_H

#include <ui/hudviewport.h>

class CChainHUD : public CHUDViewport
{
	DECLARE_CLASS(CChainHUD, CHUDViewport);

public:
	virtual void	Paint(float x, float y, float w, float h);

	virtual bool	MousePressed(int code, int mx, int my);
};

#endif
