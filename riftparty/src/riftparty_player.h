#pragma once

#include <tengine/game/entities/player.h>

class CRiftPartyPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CRiftPartyPlayer, CPlayer);

public:
	virtual void					Spawn();

	class CPlayerCharacter*			GetPlayerCharacter();
};
