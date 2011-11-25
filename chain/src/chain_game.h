#ifndef CHAIN_GAME_H
#define CHAIN_GAME_H

#include <game/game.h>

class CStory;

class CChainGame : public CGame
{
	REGISTER_ENTITY_CLASS(CChainGame, CGame);

public:
	virtual void				SetupGame(tstring sType);

	virtual void				Precache();

	virtual void				Think();

	class CPlayerCharacter*		GetLocalPlayerCharacter();
	class CChainRenderer*		GetChainRenderer();
	class CChainCamera*			GetChainCamera();

	CStory*						GetStory() const;
	void						SetStory(CStory* pStory);

protected:
	CEntityHandle<CStory>		m_hStory;
};

inline class CChainGame* ChainGame()
{
	CGame* pGame = Game();
	if (!pGame)
		return NULL;

	return static_cast<CChainGame*>(pGame);
}

#endif
