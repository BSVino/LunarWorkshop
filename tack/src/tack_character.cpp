#include "tack_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>

#include "tack_game.h"
#include "tack_renderer.h"

REGISTER_ENTITY(CTackCharacter);

NETVAR_TABLE_BEGIN(CTackCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTackCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flGoalYaw);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRenderYaw);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTackCharacter);
INPUTS_TABLE_END();

void CTackCharacter::Spawn()
{
	BaseClass::Spawn();

	SetGlobalGravity(Vector(0, -9.8f, 0));
	m_flMaxStepSize = 0.1f;

	m_flGoalYaw = m_flRenderYaw = 0;
}

CVar anim_yawspeed("anim_yawspeed", "1000");

void CTackCharacter::Think()
{
	BaseClass::Think();

	if (GetLocalVelocity().LengthSqr() > 0.5f)
	{
		Vector vecVelocity = GetLocalVelocity();
		// Why? Dunno.
		vecVelocity.z = -vecVelocity.z;
		vecVelocity.y = 0;
		m_flGoalYaw = VectorAngles(vecVelocity).y;
	}

	m_flRenderYaw = AngleApproach(m_flGoalYaw, m_flRenderYaw, GameServer()->GetFrameTime()*anim_yawspeed.GetFloat());
}

Matrix4x4 CTackCharacter::GetRenderTransform() const
{
	Matrix4x4 mGlobal = GetGlobalTransform();
	mGlobal.SetAngles(EAngle(0, m_flRenderYaw, 0));
	return mGlobal;
}
