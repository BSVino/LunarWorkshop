#ifndef TACK_CHARACTER_H
#define TACK_CHARACTER_H

#include <tengine/game/entities/character.h>

typedef enum
{
	ABILITY_NONE = 0,
	ABILITY_EATBRAINS,
} special_ability_t;

const tstring& SpecialAbilityName(special_ability_t eAbility);

class CTackCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CTackCharacter, CCharacter);

public:
								CTackCharacter();

public:
	virtual void				Spawn();

	virtual TVector				GetGoalVelocity();
	virtual void				Think();
	virtual void				CalculateGoalYaw();

	virtual TFloat				EyeHeight() const { return 1.8f; }
	virtual TFloat				BaseCharacterSpeed() { return 7.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }
	virtual float				DamageRecoveryTime() const { return 0.8f; }

	virtual Matrix4x4			GetRenderTransform() const;
	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;
	virtual void				PostRender(bool bTransparent) const;

	virtual bool				CanAttack() const;
	virtual float				AttackSphereRadius() const { return 0.4f; }
	virtual TVector				AttackSphereCenter() const;

	virtual bool				IsInDamageRecoveryTime() const;
	virtual bool				TakesDamage() const;
	virtual void				OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true);

	virtual special_ability_t	CorpseAbility() { return ABILITY_NONE; }	// What special ability is available in my corpse?
	virtual void				CreateCorpse();

protected:
	float						m_flGoalYaw;
	float						m_flRenderYaw;
};

#endif
