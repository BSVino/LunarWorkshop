#include "projectile.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <maths.h>

#include <renderer/renderer.h>
#include <renderer/particles.h>

#include "digitanksgame.h"
#include "camera.h"

REGISTER_ENTITY(CProjectile);

CProjectile::CProjectile()
{
	m_flTimeCreated = DigitanksGame()?DigitanksGame()->GetGameTime():0;
	m_flTimeExploded = 0;

	m_bFallSoundPlayed = false;
}

void CProjectile::Precache()
{
	PrecacheParticleSystem(L"shell-trail");
	PrecacheSound("sound/bomb-drop.wav");
	PrecacheSound("sound/explosion.wav");
}

void CProjectile::Think()
{
	if (MakesSounds() && GetVelocity().y < 10.0f && !m_bFallSoundPlayed && m_flTimeExploded == 0.0f)
	{
		EmitSound("sound/bomb-drop.wav");
		SetSoundVolume("sound/bomb-drop.wav", 0.5f);
		m_bFallSoundPlayed = true;
	}

	if (DigitanksGame()->GetGameTime() - m_flTimeCreated > 5.0f && m_flTimeExploded == 0.0f)
		Delete();

	else if (m_flTimeExploded != 0.0f && DigitanksGame()->GetGameTime() - m_flTimeExploded > 2.0f)
		Delete();
}

void CProjectile::ModifyContext(class CRenderingContext* pContext)
{
	if (m_flTimeExploded > 0.0f)
	{
		pContext->UseFrameBuffer(Game()->GetRenderer()->GetExplosionBuffer()->m_iFB);
	}
}

void CProjectile::OnRender()
{
	if (m_flTimeExploded == 0.0f)
	{
		glColor4ubv(Color(255, 255, 255));
		glutSolidSphere(ShellRadius(), 4, 4);
	}
	else
	{
		float flAlpha = RemapValClamped(DigitanksGame()->GetGameTime()-m_flTimeExploded, 0.2f, 1.2f, 1, 0);
		if (flAlpha > 0)
		{
			glColor4ubv(Color(255, 255, 255, (int)(flAlpha*255)));
			glutSolidSphere(4.0f, 20, 10);
		}
	}
}

void CProjectile::OnDeleted()
{
	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
}

bool CProjectile::ShouldTouch(CBaseEntity* pOther) const
{
	if (m_flTimeExploded != 0)
		return false;

	if (pOther == m_hOwner)
		return false;

	if (pOther->GetCollisionGroup() == CG_TANK)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);
		if (!pTank)	// ?
			return false;

		if (pTank->GetTeam() == m_hOwner->GetTeam())
			return false;

		return true;
	}

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}

bool CProjectile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	switch (pOther->GetCollisionGroup())
	{
	case CG_TANK:
		vecPoint = GetOrigin();
		if ((pOther->GetOrigin() - GetOrigin()).LengthSqr() < pOther->GetBoundingRadius()*pOther->GetBoundingRadius())
			return true;
		break;

	case CG_TERRAIN:
		return DigitanksGame()->GetTerrain()->Collide(GetLastOrigin(), GetOrigin(), vecPoint);
	}

	return false;
};

void CProjectile::Touching(CBaseEntity* pOther)
{
	if (dynamic_cast<CTerrain*>(pOther))
	{
		if (ShouldExplode())
			pOther->TakeDamage(m_hOwner, this, m_flDamage);
	}
	else
		pOther->TakeDamage(m_hOwner, this, m_flDamage, ShouldExplode());

	SetVelocity(Vector());
	SetGravity(Vector());

	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);

	bool bHit = false;
	
	if (ShouldExplode())
	{
		bHit = DigitanksGame()->Explode(m_hOwner, this, 4, m_flDamage, pOther, m_hOwner->GetTeam());
		m_flTimeExploded = Game()->GetGameTime();
		Game()->GetCamera()->Shake(GetOrigin(), 3);
	}
	else
		m_flTimeExploded = 1;	// Remove immediately.

	if (MakesSounds())
	{
		StopSound("sound/bomb-drop.wav");
		EmitSound("sound/explosion.wav");
	}

	if (dynamic_cast<CTerrain*>(pOther) && !bHit)
		m_hOwner->Speak(TANKSPEECH_MISSED);
}

void CProjectile::SetOwner(CDigitank* pOwner)
{
	m_hOwner = pOwner;
	SetOrigin(pOwner->GetOrigin() + Vector(0, 1, 0));
	SetSimulated(true);
	SetCollisionGroup(CG_POWERUP);

	m_iParticleSystem = CreateParticleSystem();
	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
}

size_t CProjectile::CreateParticleSystem()
{
	return CParticleSystemLibrary::AddInstance(L"shell-trail", GetOrigin());
}

REGISTER_ENTITY(CShell);

REGISTER_ENTITY(CInfantryFlak);

size_t CInfantryFlak::CreateParticleSystem()
{
	return ~0;
}
