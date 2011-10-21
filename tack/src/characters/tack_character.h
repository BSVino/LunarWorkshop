#ifndef TACK_CHARACTER_H
#define TACK_CHARACTER_H

#include <tengine/game/entities/character.h>

class CTackCharacter : public CCharacter
{
	REGISTER_ENTITY_CLASS(CTackCharacter, CCharacter);

public:
								CTackCharacter();

public:
	virtual void				Spawn();

	virtual void				Think();

	virtual TFloat				EyeHeight() const { return 1.8f; }
	virtual TFloat				CharacterSpeed() { return 7.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }

	virtual Matrix4x4			GetRenderTransform() const;
	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;

	virtual bool				CanAttack() const;

	virtual bool				IsInDamageRecoveryTime() const;
	virtual bool				TakesDamage() const;
	virtual void				OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true);

protected:
	float						m_flGoalYaw;
	float						m_flRenderYaw;
};

#endif
