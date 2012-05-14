#include "tack_game.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>

#include "tack_player.h"
#include "characters/tack_character.h"
#include "tack_camera.h"
#include "tack_renderer.h"

CGame* CreateGame()
{
	return GameServer()->Create<CTackGame>("CTackGame");
}

CRenderer* CreateRenderer()
{
	return new CTackRenderer();
}

CCamera* CreateCamera()
{
	CCamera* pCamera = new CTackCamera();
	return pCamera;
}

CLevel* CreateLevel()
{
	return new CLevel();
}

REGISTER_ENTITY(CTackGame);

NETVAR_TABLE_BEGIN(CTackGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTackGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTackGame);
INPUTS_TABLE_END();

void CTackGame::Precache()
{
}

void CTackGame::Think()
{
	BaseClass::Think();
}

CTackCharacter* CTackGame::GetLocalPlayerCharacter()
{
	return static_cast<CTackCharacter*>(GetLocalPlayer()->GetCharacter());
}

CTackRenderer* CTackGame::GetTackRenderer()
{
	return static_cast<CTackRenderer*>(GameServer()->GetRenderer());
}

CTackCamera* CTackGame::GetTackCamera()
{
	return static_cast<CTackCamera*>(GameServer()->GetCamera());
}
