#ifndef TACK_PLAYER_CHARACTER_H
#define TACK_PLAYER_CHARACTER_H

#include "tack_character.h"

#include "corpse.h"

class CPlayerCharacter : public CTackCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CTackCharacter);

public:
	void						Precache();
	void						Spawn();

	void						Think();
	virtual void				CalculateGoalYaw();

	virtual TVector				GetGoalVelocity();

	virtual float				DamageRecoveryTime() const { return 0.3f; }

	float						CorpseAbsorbDistance() const { return 2; }
	float						CorpseAbsorbTime() const { return 1; }
	bool						AbsorbCorpse();
	void						FinishAbsorbCorpse(bool bCompleted);
	bool						IsAbsorbing() const;

	virtual void				OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true);

	void						UseSpecialAbility();
	special_ability_t			GetSpecialAbility() { return m_eSpecialAbility; }

	void						EatBrains();

protected:
	float						m_flAbsorbStart;
	CEntityHandle<CCorpse>		m_hAbsorbCorpse;

	special_ability_t			m_eSpecialAbility;
};

#endif
