#include "reflection_player.h"

#include <tengine/game/entities/character.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>

#include "reflection_playercharacter.h"

REGISTER_ENTITY(CReflectionPlayer);

NETVAR_TABLE_BEGIN(CReflectionPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, mirror_t, m_eCurrentMirror);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionPlayer);
INPUTS_TABLE_END();

void CReflectionPlayer::Spawn()
{
	m_eCurrentMirror = MIRROR_VERTICAL;
}

CPlayerCharacter* CReflectionPlayer::GetPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(m_hCharacter.GetPointer());
}

void CReflectionPlayer::MouseMotion(int x, int y)
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

void CReflectionPlayer::MouseInput(int iButton, int iState)
{
	BaseClass::MouseInput(iButton, iState);

	TAssert(GetPlayerCharacter());
	if (!GetPlayerCharacter())
		return;

//	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == 1)
//		GetPlayerCharacter()->PlaceMirror(m_eCurrentMirror);
}

void CReflectionPlayer::KeyPress(int c)
{
	if (m_hCharacter == NULL)
		return;

	if (c == 'E')
		GetPlayerCharacter()->FindItems();

	if (c == '1')
		m_eCurrentMirror = MIRROR_VERTICAL;
	if (c == '2')
		m_eCurrentMirror = MIRROR_HORIZONTAL;

	BaseClass::KeyPress(c);
}
