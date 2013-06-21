#include "player.h"

#include <game/entities/game.h>
#include <tinker/cvar.h>
#include <tinker/keys.h>

#include "character.h"

REGISTER_ENTITY(CPlayer);

NETVAR_TABLE_BEGIN(CPlayer);
	NETVAR_DEFINE(CEntityHandle, m_hCharacter);
	NETVAR_DEFINE_CALLBACK(int, m_iClient, &CGame::ClearLocalPlayers);
	NETVAR_DEFINE(tstring, m_sPlayerName);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CCharacter>, m_hCharacter);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iClient);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iInstallID);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_sPlayerName);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayer);
INPUTS_TABLE_END();

CPlayer::CPlayer()
{
	m_iClient = NETWORK_LOCAL;

	m_flLastLesson = -1;

	m_vecJoystickViewVelocity = Vector2D();
}

void NoClip(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (!CVar::GetCVarBool("cheats"))
	{
		TMsg("Noclip is not allowed with cheats off.\n");
		return;
	}

	if (!Game())
		return;

	if (!Game()->GetLocalPlayer())
		return;

	if (!Game()->GetLocalPlayer()->GetCharacter())
		return;

	CCharacter* pCharacter = Game()->GetLocalPlayer()->GetCharacter();
	pCharacter->SetNoClip(!pCharacter->GetNoClip());

	if (pCharacter->GetNoClip())
		TMsg("NoClip ON\n");
	else
		TMsg("NoClip OFF\n");
}

CCommand noclip("noclip", ::NoClip);

CVar m_sensitivity("m_sensitivity", "4");

void CPlayer::MouseMotion(int x, int y)
{
	if (!m_hCharacter)
		return;

	EAngle angDirection = m_hCharacter->GetViewAngles();

	angDirection.y -= (x*m_sensitivity.GetFloat()/20);
	angDirection.p -= (y*m_sensitivity.GetFloat()/20);

	if (angDirection.p > 89)
		angDirection.p = 89;

	if (angDirection.p < -89)
		angDirection.p = -89;

	while (angDirection.y > 180)
		angDirection.y -= 360;

	while (angDirection.y < -180)
		angDirection.y += 360;

	m_hCharacter->SetViewAngles(angDirection);
}

void CPlayer::KeyPress(int c)
{
	if (!m_hCharacter)
		return;

	if (c == 'W')
		m_hCharacter->Move(MOVE_FORWARD);
	if (c == 'S')
		m_hCharacter->Move(MOVE_BACKWARD);
	if (c == 'D')
		m_hCharacter->Move(MOVE_RIGHT);
	if (c == 'A')
		m_hCharacter->Move(MOVE_LEFT);

	if (c == ' ')
		m_hCharacter->Jump();
}

void CPlayer::KeyRelease(int c)
{
	if (!m_hCharacter)
		return;

	if (c == 'W')
		m_hCharacter->StopMove(MOVE_FORWARD);
	if (c == 'S')
		m_hCharacter->StopMove(MOVE_BACKWARD);
	if (c == 'D')
		m_hCharacter->StopMove(MOVE_RIGHT);
	if (c == 'A')
		m_hCharacter->StopMove(MOVE_LEFT);
}

void CPlayer::JoystickButtonPress(int iJoystick, int c)
{
	if (c == TINKER_KEY_JOYSTICK_2)
		m_hCharacter->Jump();
}

CVar joy_move_deadmin("joy_move_deadmin", "0.3");
CVar joy_move_deadmax("joy_move_deadmax", "0.8");

CVar joy_view_deadmin("joy_view_deadmin", "0.2");
CVar joy_view_deadmax("joy_view_deadmax", "0.9");

CVar joy_view_turbo("joy_view_turbo", "4");

void CPlayer::JoystickAxis(int iJoystick, int iAxis, float flValue, float flChange)
{
	//TMsg(sprintf("%d %d %f %f\n", iJoystick, iAxis, flValue, flChange));

	float flDeadMoveMin = joy_move_deadmin.GetFloat();
	float flDeadMoveMax = joy_move_deadmax.GetFloat();

	float flDeadViewMin = joy_view_deadmin.GetFloat();
	float flDeadViewMax = joy_view_deadmax.GetFloat();

	if (iAxis == 0)
	{
		if (flValue > flDeadMoveMin)
			m_hCharacter->SetGoalVelocityLeft(-RemapValClamped(flValue, flDeadMoveMin, flDeadMoveMax, flDeadMoveMin, 1.0f));
		else if (flValue < -flDeadMoveMin)
			m_hCharacter->SetGoalVelocityLeft(-RemapValClamped(flValue, -flDeadMoveMin, -flDeadMoveMax, -flDeadMoveMin, -1.0f));
		else
			m_hCharacter->SetGoalVelocityLeft(0);
	}
	else if (iAxis == 1)
	{
		if (flValue > flDeadMoveMin)
			m_hCharacter->SetGoalVelocityForward(RemapValClamped(flValue, flDeadMoveMin, flDeadMoveMax, flDeadMoveMin, 1.0f));
		else if (flValue < -flDeadMoveMin)
			m_hCharacter->SetGoalVelocityForward(RemapValClamped(flValue, -flDeadMoveMin, -flDeadMoveMax, -flDeadMoveMin, -1.0f));
		else
			m_hCharacter->SetGoalVelocityForward(0);
	}
	else if (iAxis == 3)
	{
		if (flValue > flDeadViewMin)
		{
			if (flValue > flDeadViewMax)
				m_vecJoystickViewVelocity.y = RemapValClamped(flValue, flDeadViewMax, 1.0f, 1.0f, joy_view_turbo.GetFloat());
			else
				m_vecJoystickViewVelocity.y = RemapValClamped(flValue, flDeadViewMin, flDeadViewMax, flDeadViewMin, 1.0f);
		}
		else if (flValue < -flDeadViewMin)
		{
			if (flValue < -flDeadViewMax)
				m_vecJoystickViewVelocity.y = RemapValClamped(flValue, -flDeadViewMax, -1.0f, -1.0f, -joy_view_turbo.GetFloat());
			else
				m_vecJoystickViewVelocity.y = RemapValClamped(flValue, -flDeadViewMin, -flDeadViewMax, -flDeadViewMin, -1.0f);
		}
		else
			m_vecJoystickViewVelocity.y = 0;
	}
	else if (iAxis == 4)
	{
		if (flValue > flDeadViewMin)
		{
			if (flValue > flDeadViewMax)
				m_vecJoystickViewVelocity.x = RemapValClamped(flValue, flDeadViewMax, 1.0f, 1.0f, joy_view_turbo.GetFloat());
			else
				m_vecJoystickViewVelocity.x = RemapValClamped(flValue, flDeadViewMin, flDeadMoveMax, flDeadViewMin, 1.0f);
		}
		else if (flValue < -flDeadViewMin)
		{
			if (flValue < -flDeadViewMax)
				m_vecJoystickViewVelocity.x = RemapValClamped(flValue, -flDeadViewMax, -1.0f, -1.0f, -joy_view_turbo.GetFloat());
			else
				m_vecJoystickViewVelocity.x = RemapValClamped(flValue, -flDeadViewMin, -flDeadViewMax, -flDeadViewMin, -1.0f);
		}
		else
			m_vecJoystickViewVelocity.x = 0;
	}
}

CVar joy_sensitivity("joy_sensitivity", "8");

void CPlayer::Think()
{
	BaseClass::Think();

	Instructor_Think();

	if (m_vecJoystickViewVelocity.LengthSqr() > 0)
		MouseMotion((int)(m_vecJoystickViewVelocity.x*joy_sensitivity.GetFloat()), (int)(m_vecJoystickViewVelocity.y*joy_sensitivity.GetFloat()));
}

void CPlayer::SetCharacter(CCharacter* pCharacter)
{
	m_hCharacter = pCharacter;
	if (pCharacter)
		pCharacter->SetControllingPlayer(this);
}

CCharacter* CPlayer::GetCharacter() const
{
	return m_hCharacter;
}

void CPlayer::SetClient(int iClient)
{
	m_iClient = iClient;
}
