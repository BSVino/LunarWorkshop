#ifndef CHAIN_HUD_H
#define CHAIN_HUD_H

#include <memory>

#include <ui/hudviewport.h>

class CChainHUD : public CHUDViewport
{
	DECLARE_CLASS(CChainHUD, CHUDViewport);

public:
					CChainHUD();

public:
	virtual void	Paint(float x, float y, float w, float h);

	virtual bool	KeyPressed(int code, bool bCtrlDown = false);
	virtual bool	MousePressed(int code, int mx, int my);

protected:
	std::shared_ptr<class CChainMenu>	m_pMenu;
};

#endif
