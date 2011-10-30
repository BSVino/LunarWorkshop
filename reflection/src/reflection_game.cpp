#include "reflection_game.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>

#include "reflection_player.h"
#include "reflection_character.h"
#include "reflection_camera.h"
#include "reflection_renderer.h"

CGame* CreateGame()
{
	return GameServer()->Create<CReflectionGame>("CReflectionGame");
}

CRenderer* CreateRenderer()
{
	return new CReflectionRenderer();
}

CCamera* CreateCamera()
{
	CCamera* pCamera = new CReflectionCamera();
	return pCamera;
}

CLevel* CreateLevel()
{
	return new CLevel();
}

REGISTER_ENTITY(CReflectionGame);

NETVAR_TABLE_BEGIN(CReflectionGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionGame);
INPUTS_TABLE_END();

void CReflectionGame::Precache()
{
}

void CReflectionGame::Think()
{
	BaseClass::Think();
}

CReflectionCharacter* CReflectionGame::GetLocalPlayerCharacter()
{
	return static_cast<CReflectionCharacter*>(GetLocalPlayer()->GetCharacter());
}

CReflectionRenderer* CReflectionGame::GetReflectionRenderer()
{
	return static_cast<CReflectionRenderer*>(GameServer()->GetRenderer());
}

CReflectionCamera* CReflectionGame::GetReflectionCamera()
{
	return static_cast<CReflectionCamera*>(GameServer()->GetCamera());
}
