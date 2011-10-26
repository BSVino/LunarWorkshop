#include "tack_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>

#include "tack_game.h"
#include "tack_renderer.h"
#include "corpse.h"

REGISTER_ENTITY(CTackCharacter);

NETVAR_TABLE_BEGIN(CTackCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTackCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flGoalYaw);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRenderYaw);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTackCharacter);
INPUTS_TABLE_END();

CTackCharacter::CTackCharacter()
{
	m_bTransformMoveByView = false;
}

void CTackCharacter::Spawn()
{
	BaseClass::Spawn();

	SetGlobalGravity(Vector(0, -9.8f, 0));
	m_flMaxStepSize = 0.1f;

	m_flGoalYaw = m_flRenderYaw = 0;
}

TVector CTackCharacter::GetGoalVelocity()
{
	if (IsAttacking())
		return TVector();

	if (IsInDamageRecoveryTime())
		return TVector();

	return BaseClass::GetGoalVelocity();
}

CVar anim_yawspeed("anim_yawspeed", "1000");

void CTackCharacter::Think()
{
	BaseClass::Think();

	if (!IsAlive() && !IsInDamageRecoveryTime())
	{
		CreateCorpse();
		Delete();
	}

	CalculateGoalYaw();

	m_flRenderYaw = AngleApproach(m_flGoalYaw, m_flRenderYaw, GameServer()->GetFrameTime()*anim_yawspeed.GetFloat());
	SetGlobalAngles(EAngle(0, m_flRenderYaw, 0));
}

void CTackCharacter::CalculateGoalYaw()
{
	Vector vecYawVelocity = GetGlobalVelocity();
	if (IsInDamageRecoveryTime())
		vecYawVelocity = m_vecGoalVelocity;

	if (vecYawVelocity.LengthSqr() > 0.5f)
	{
		Vector vecVelocity = vecYawVelocity.Normalized();
		vecVelocity.y = 0;
		m_flGoalYaw = VectorAngles(vecVelocity).y;
	}
}

Matrix4x4 CTackCharacter::GetRenderTransform() const
{
	Matrix4x4 mGlobal = GetGlobalTransform();
	mGlobal.SetAngles(EAngle(0, m_flRenderYaw, 0));
	return mGlobal;
}

void CTackCharacter::ModifyContext(class CRenderingContext* pContext, bool bTransparent) const
{
	if (IsInDamageRecoveryTime())
		pContext->SetColor(Color(255, 0, 0));
}

CVar debug_showattack("debug_showattack", "on");

void CTackCharacter::PostRender(bool bTransparent) const
{
	if (debug_showattack.GetBool() && m_flLastAttack >= 0 && GameServer()->GetGameTime() - m_flLastAttack < 0.2f)
	{
		CRenderingContext c(GameServer()->GetRenderer());

		TFloat flAttackSphereRadius = AttackSphereRadius();
		c.Translate(AttackSphereCenter());
		c.Scale(flAttackSphereRadius, flAttackSphereRadius, flAttackSphereRadius);
		c.RenderSphere();
	}
}

bool CTackCharacter::CanAttack() const
{
	if (IsInDamageRecoveryTime())
		return false;

	return BaseClass::CanAttack();
}

Vector CTackCharacter::AttackSphereCenter() const
{
	return GetGlobalOrigin() + GetUpVector()*(EyeHeight()*0.7f) + GetGlobalTransform().GetForwardVector() * AttackSphereRadius();
}

bool CTackCharacter::IsInDamageRecoveryTime() const
{
	if (m_flLastTakeDamage >= 0 && GameServer()->GetGameTime() - m_flLastTakeDamage < DamageRecoveryTime())
		return true;

	return false;
}

bool CTackCharacter::TakesDamage() const
{
	if (m_flLastTakeDamage < 0)
		return BaseClass::TakesDamage();

	if (GameServer()->GetGameTime() - m_flLastTakeDamage < DamageRecoveryTime())
		return false;

	return BaseClass::TakesDamage();
}

CVar game_damagepushback("game_damagepushback", "3");
CVar game_damagepushup("game_damagepushup", "2");

void CTackCharacter::OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	Vector vecAttackDirection;
	if (pInflictor)
		vecAttackDirection = GetGlobalCenter() - pInflictor->GetGlobalCenter();
	else if (pAttacker)
		vecAttackDirection = GetGlobalCenter() - pAttacker->GetGlobalCenter();
	else
		return;

	vecAttackDirection.y = 0;
	vecAttackDirection.Normalize();

	SetGlobalVelocity(vecAttackDirection * game_damagepushback.GetFloat() + Vector(0, 1, 0) * game_damagepushup.GetFloat());
}

void CTackCharacter::CreateCorpse()
{
	CCorpse* pCorpse = GameServer()->Create<CCorpse>("CCorpse");
	pCorpse->SetGlobalTransform(GetGlobalTransform());
	pCorpse->SetModel(GetModel());
	pCorpse->SetSpecialAbility(CorpseAbility());
}

tstring g_asSpecialAbilityNames[] =
{
	"None",
	"Eat Brains",
};

const tstring& SpecialAbilityName(special_ability_t eAbility)
{
	return g_asSpecialAbilityNames[eAbility];
}
