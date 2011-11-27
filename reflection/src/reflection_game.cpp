#include "reflection_game.h"

#include <tinker/cvar.h>
#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>
#include <game/entities/kinematic.h>
#include <game/entities/mathgate.h>

#include "reflection_player.h"
#include "reflection_character.h"
#include "reflection_camera.h"
#include "reflection_renderer.h"
#include "reflection_playercharacter.h"
#include "world.h"
#include "token.h"
#include "ui/hud.h"

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

CHUDViewport* CreateHUD()
{
	CHUDViewport* pHUD = new CReflectionHUD();
	return pHUD;
}

tstring GetInitialGameMode()
{
	return "menu";
}

REGISTER_ENTITY(CReflectionGame);

NETVAR_TABLE_BEGIN(CReflectionGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionGame);
INPUTS_TABLE_END();

void CReflectionGame::SetupGame(tstring sType)
{
	if (sType == "level")
	{
		CReflectionPlayer* pPlayer = GameServer()->Create<CReflectionPlayer>("CReflectionPlayer");
		Game()->AddPlayer(pPlayer);

		CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
		pCharacter->SetGlobalOrigin(Vector(0, 0, 0));
		pPlayer->SetCharacter(pCharacter);

		GameServer()->LoadLevel(CVar::GetCVarValue("game_level"));

		Application()->SetMouseCursorEnabled(false);
	}
	else if (sType == "menu")
	{
		Application()->SetMouseCursorEnabled(true);
	}
	else
	{
		TError("Unrecognized game type: " + sType + "\n");
	}
}

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
