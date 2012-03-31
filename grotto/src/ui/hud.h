#include "grotto_game.h"

#include <ui/hudviewport.h>

class CLevelSelector;

class CGrottoHUD : public CHUDViewport
{
	DECLARE_CLASS(CGrottoHUD, CHUDViewport);

public:
					CGrottoHUD();

public:
	virtual void	Paint(float x, float y, float w, float h);

	virtual bool	KeyPressed(int code, bool bCtrlDown = false);

protected:
	CLevelSelector*		m_pSelector;
};