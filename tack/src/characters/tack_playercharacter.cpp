#include "tack_playercharacter.h"

#include <tinker/application.h>

#include "corpse.h"

REGISTER_ENTITY(CPlayerCharacter);

NETVAR_TABLE_BEGIN(CPlayerCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayerCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flAbsorbStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CCorpse>, m_hAbsorbCorpse);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerCharacter);
INPUTS_TABLE_END();

void CPlayerCharacter::Precache()
{
	PrecacheModel("models/characters/tack.obj");
}

void CPlayerCharacter::Spawn()
{
	m_eSpecialAbility = ABILITY_NONE;

	BaseClass::Spawn();

	SetModel("models/characters/tack.obj");
}

void CPlayerCharacter::Think()
{
	if (IsAbsorbing() && GameServer()->GetGameTime() - m_flAbsorbStart > CorpseAbsorbTime())
		FinishAbsorbCorpse(true);

	BaseClass::Think();
}

void CPlayerCharacter::CalculateGoalYaw()
{
	if (IsAbsorbing())
	{
		Vector vecVelocity = (m_hAbsorbCorpse->GetGlobalCenter()-GetGlobalCenter()).Normalized();
		vecVelocity.y = 0;
		m_flGoalYaw = VectorAngles(vecVelocity).y;
		return;
	}

	BaseClass::CalculateGoalYaw();
}

TVector CPlayerCharacter::GetGoalVelocity()
{
	if (IsAbsorbing())
		return TVector();

	return BaseClass::GetGoalVelocity();
}

bool CPlayerCharacter::AbsorbCorpse()
{
	float flCorpseAbsorbDistance = CorpseAbsorbDistance();
	CCorpse* pNearestCorpse = NULL;
	float flNearestDistanceSqr;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CCorpse* pCorpse = dynamic_cast<CCorpse*>(pEntity);
		if (!pCorpse)
			continue;

		float flDistanceSqr = (pCorpse->GetGlobalCenter() - GetGlobalCenter()).LengthSqr();
		if (flDistanceSqr < flCorpseAbsorbDistance*flCorpseAbsorbDistance)
		{
			if (!pNearestCorpse || flDistanceSqr < flNearestDistanceSqr)
			{
				pNearestCorpse = pCorpse;
				flNearestDistanceSqr = flDistanceSqr;
			}
		}
	}

	if (!pNearestCorpse)
		return false;

	m_flAbsorbStart = GameServer()->GetGameTime();
	m_hAbsorbCorpse = pNearestCorpse;
	return true;
}

void CPlayerCharacter::FinishAbsorbCorpse(bool bCompleted)
{
	if (bCompleted && !!m_hAbsorbCorpse)
	{
		m_eSpecialAbility = m_hAbsorbCorpse->GetSpecialAbility();
		m_hAbsorbCorpse->Delete();
	}

	m_hAbsorbCorpse = NULL;
}

bool CPlayerCharacter::IsAbsorbing() const
{
	return !!m_hAbsorbCorpse.GetPointer();
}

void CPlayerCharacter::OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	if (IsAbsorbing())
		FinishAbsorbCorpse(false);

	BaseClass::OnTakeDamage(pAttacker, pInflictor, eDamageType, flDamage, bDirectHit);
}

void CPlayerCharacter::UseSpecialAbility()
{
	if (m_eSpecialAbility == ABILITY_EATBRAINS)
		EatBrains();
}

void CPlayerCharacter::EatBrains()
{
	if (!CanAttack())
		return;

	m_flLastAttack = GameServer()->GetGameTime();
	m_vecMoveVelocity = TVector();

	TFloat flAttackSphereRadius = AttackSphereRadius();
	TVector vecDamageSphereCenter = AttackSphereCenter();

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t j = 0; j < iMaxEntities; j++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(j);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (!pEntity->TakesDamage())
			continue;

		if (pEntity == this)
			continue;

		TFloat flRadius = pEntity->GetBoundingRadius() + flAttackSphereRadius;
		flRadius = flRadius*flRadius;
		if ((vecDamageSphereCenter - pEntity->GetGlobalCenter()).LengthSqr() > flRadius)
			continue;

		pEntity->TakeDamage(this, this, DAMAGE_GENERIC, 10000);
		m_flHealth = GetTotalHealth();
		return;
	}
}
