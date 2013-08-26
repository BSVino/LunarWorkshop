#pragma once

#include "riftparty_character.h"

class CCharacterCamera;

class CPlayerCharacter : public CRiftPartyCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CRiftPartyCharacter);

public:
	void						Precache();
	void						Spawn();

	void                        Think();

	virtual TFloat              CharacterAcceleration() { return 20.0f; }

protected:
	CEntityHandle<CCharacterCamera> m_hCamera;
};
