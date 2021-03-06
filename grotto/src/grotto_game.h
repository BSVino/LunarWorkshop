#pragma once

#include <game/entities/game.h>

class CGrottoGame : public CGame
{
	REGISTER_ENTITY_CLASS(CGrottoGame, CGame);

public:
	virtual void				SetupGame(tstring sType);

	virtual void				Precache();

	virtual void				Think();

	class CGrottoCharacter*	GetLocalPlayerCharacter();
	class CGrottoRenderer*	GetGrottoRenderer();
};

inline class CGrottoGame* GrottoGame()
{
	CGame* pGame = Game();
	if (!pGame)
		return NULL;

	return static_cast<CGrottoGame*>(pGame);
}
