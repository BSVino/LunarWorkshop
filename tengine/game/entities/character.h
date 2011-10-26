#ifndef TINKER_CHARACTER_H
#define TINKER_CHARACTER_H

#include <tengine/game/baseentity.h>

class CPlayer;

typedef enum
{
	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,
} movetype_t;

class CCharacter : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCharacter, CBaseEntity);

public:
									CCharacter();

public:
	virtual void					Spawn();
	virtual void					Think();

	void							Move(movetype_t);
	void							StopMove(movetype_t);
	virtual TVector					GetGoalVelocity();
	virtual void					MoveThink();
	virtual void					Jump();

	virtual bool					CanAttack() const;
	virtual void					Attack();
	virtual bool					IsAttacking() const;

	virtual bool					ShouldRender() const { return true; };
	virtual void					PostRender(bool bTransparent) const;
	virtual void					ShowPlayerVectors() const;

	void							SetControllingPlayer(CPlayer* pCharacter);
	CPlayer*						GetControllingPlayer() const;

	virtual TFloat					EyeHeight() const { return 180.0f; }
	virtual TFloat					CharacterSpeed() { return 80.0f; }
	virtual TFloat					JumpStrength() { return 150.0f; }
	virtual inline TVector			GetGlobalGravity() const;

	virtual float					AttackTime() const { return 0.3f; }
	virtual float					AttackDamage() const { return 50; }

	virtual bool					ShouldCollide() const { return false; }

	CBaseEntity*					GetGroundEntity() const { return m_hGround; }
	void							SetGroundEntity(CBaseEntity* pEntity) { m_hGround = pEntity; }
	virtual void					FindGroundEntity();

protected:
	CNetworkedHandle<CPlayer>		m_hControllingPlayer;

	CNetworkedHandle<CBaseEntity>	m_hGround;

	bool							m_bTransformMoveByView;
	Vector							m_vecGoalVelocity;
	Vector							m_vecMoveVelocity;

	float							m_flLastAttack;

	float							m_flMoveSimulationTime;	// This is a higher resolution of game time for physics
	TFloat							m_flMaxStepSize;
};

#endif
