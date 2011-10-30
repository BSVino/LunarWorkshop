#ifndef TACK_CORPSE_H
#define TACK_CORPSE_H

#include <tengine/game/baseentity.h>

#include "tack_character.h"

class CCorpse : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCorpse, CBaseEntity);

public:
	virtual void			Think();

	void					SetSpecialAbility(special_ability_t eAbility) { m_eAbility = eAbility; };
	special_ability_t		GetSpecialAbility() { return m_eAbility; }

protected:
	special_ability_t		m_eAbility;
};

#endif
