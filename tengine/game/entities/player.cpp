#include "player.h"

#include <tengine/game/game.h>
#include <tinker/cvar.h>

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
}

void NoClip(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	if (!CVar::GetCVarBool("cheats"))
		return;

	if (!Game())
		return;

	if (!Game()->GetLocalPlayer())
		return;

	if (!Game()->GetLocalPlayer()->GetCharacter())
		return;

	CCharacter* pCharacter = Game()->GetLocalPlayer()->GetCharacter();
	pCharacter->SetNoClip(!pCharacter->GetNoClip());
}

CCommand noclip("noclip", ::NoClip);

CVar m_sensitivity("m_sensitivity", "5");

void CPlayer::MouseMotion(int x, int y)
{
	if (m_hCharacter == NULL)
		return;

	EAngle angDirection = m_hCharacter->GetViewAngles();

	angDirection.y += (x/m_sensitivity.GetFloat());
	angDirection.p -= (y/m_sensitivity.GetFloat());

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
	if (m_hCharacter == NULL)
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
	if (m_hCharacter == NULL)
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
