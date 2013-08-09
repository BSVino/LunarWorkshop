#include "grotto_player.h"

#include <tengine/game/entities/character.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>

#include "grotto_playercharacter.h"

REGISTER_ENTITY(CGrottoPlayer);

NETVAR_TABLE_BEGIN(CGrottoPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrottoPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, mirror_t, m_eCurrentMirror);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoPlayer);
INPUTS_TABLE_END();

void CGrottoPlayer::Spawn()
{
	m_eCurrentMirror = MIRROR_VERTICAL;
}

CPlayerCharacter* CGrottoPlayer::GetPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(m_hCharacter.GetPointer());
}

void CGrottoPlayer::MouseMotion(int x, int y)
{
	if (!GetPlayerCharacter())
		return;

	if (Application()->IsMouseCursorEnabled())
		return;

	if (GetPlayerCharacter()->IsReflected(REFLECTION_VERTICAL) && GetPlayerCharacter()->IsReflected(REFLECTION_LATERAL))
		BaseClass::MouseMotion(-x, -y);
	else if (GetPlayerCharacter()->IsReflected(REFLECTION_VERTICAL))
		BaseClass::MouseMotion(x, -y);
	else if (GetPlayerCharacter()->IsReflected(REFLECTION_LATERAL))
		BaseClass::MouseMotion(-x, y);
	else
		BaseClass::MouseMotion(x, y);
}

void CGrottoPlayer::MouseInput(int iButton, int iState)
{
	BaseClass::MouseInput(iButton, iState);

	TAssert(GetPlayerCharacter());
	if (!GetPlayerCharacter())
		return;

//	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == 1)
//		GetPlayerCharacter()->PlaceMirror(m_eCurrentMirror);
}

void CGrottoPlayer::KeyPress(int c)
{
	if (m_hCharacter == NULL)
		return;

	if (c == '1')
		m_eCurrentMirror = MIRROR_VERTICAL;
	if (c == '2')
		m_eCurrentMirror = MIRROR_HORIZONTAL;

	BaseClass::KeyPress(c);
}

void CGrottoPlayer::JoystickButtonPress(int iJoystick, int c)
{
	BaseClass::JoystickButtonPress(iJoystick, c);
}

void CGrottoPlayer::JoystickAxis(int iJoystick, int iAxis, float flValue, float flChange)
{
	if (!GetPlayerCharacter())
		return;

	if (iAxis == 3)
	{
		if (GetPlayerCharacter()->IsReflected(REFLECTION_VERTICAL))
			BaseClass::JoystickAxis(iJoystick, iAxis, -flValue, -flChange);
		else
			BaseClass::JoystickAxis(iJoystick, iAxis, flValue, flChange);
	}
	else if (iAxis == 4)
	{
		if (GetPlayerCharacter()->IsReflected(REFLECTION_LATERAL))
			BaseClass::JoystickAxis(iJoystick, iAxis, -flValue, -flChange);
		else
			BaseClass::JoystickAxis(iJoystick, iAxis, flValue, flChange);
	}
	else
		BaseClass::JoystickAxis(iJoystick, iAxis, flValue, flChange);
}
