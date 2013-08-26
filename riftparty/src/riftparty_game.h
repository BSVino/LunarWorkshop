#pragma once

#include <game/entities/game.h>

class CRiftPartyGame : public CGame
{
	REGISTER_ENTITY_CLASS(CRiftPartyGame, CGame);

public:
	virtual void				SetupGame(tstring sType);

	virtual void				Precache();

	virtual void				Think();

	class CRiftPartyCharacter*	GetLocalPlayerCharacter();
	class CRiftPartyRenderer*	GetRiftPartyRenderer();
};

inline class CRiftPartyGame* RiftPartyGame()
{
	CGame* pGame = Game();
	if (!pGame)
		return NULL;

	return static_cast<CRiftPartyGame*>(pGame);
}
