#pragma once

#include <ui/hudviewport.h>

class CLevelSelector;

class CRiftPartyHUD : public CHUDViewport
{
	DECLARE_CLASS(CRiftPartyHUD, CHUDViewport);

public:
					CRiftPartyHUD();

public:
	virtual void	Paint(float x, float y, float w, float h);

protected:
	CLevelSelector*		m_pSelector;
};
