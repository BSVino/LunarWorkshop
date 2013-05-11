#ifndef REFLECTION_GAME_H
#define REFLECTION_GAME_H

#include <game/game.h>

class CReflectionGame : public CGame
{
	REGISTER_ENTITY_CLASS(CReflectionGame, CGame);

public:
	virtual void				SetupGame(tstring sType);

	virtual void				Precache();

	virtual void				Think();

	class CReflectionCharacter*	GetLocalPlayerCharacter();
	class CReflectionRenderer*	GetReflectionRenderer();
	class CReflectionCamera*	GetReflectionCamera();
};

inline class CReflectionGame* ReflectionGame()
{
	CGame* pGame = Game();
	if (!pGame)
		return NULL;

	return static_cast<CReflectionGame*>(pGame);
}

#endif
