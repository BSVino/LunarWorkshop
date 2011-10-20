#ifndef TACK_GAME_H
#define TACK_GAME_H

#include <game/game.h>

class CTackGame : public CGame
{
	REGISTER_ENTITY_CLASS(CTackGame, CGame);

public:
	virtual void			Precache();

	virtual void			Think();

	class CTackCharacter*	GetLocalPlayerCharacter();
	class CTackRenderer*	GetTackRenderer();
	class CTackCamera*		GetTackCamera();
};

inline class CTackGame* TackGame()
{
	CGame* pGame = Game();
	if (!pGame)
		return NULL;

	return static_cast<CTackGame*>(pGame);
}

#endif
