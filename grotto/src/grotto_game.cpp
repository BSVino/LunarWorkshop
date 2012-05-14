#include "grotto_game.h"

#include <tinker/cvar.h>
#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>
#include <game/entities/kinematic.h>
#include <game/entities/mathgate.h>

#include "grotto_player.h"
#include "grotto_character.h"
#include "grotto_camera.h"
#include "grotto_renderer.h"
#include "grotto_playercharacter.h"
#include "token.h"
#include "ui/hud.h"

CGame* CreateGame()
{
	return GameServer()->Create<CGrottoGame>("CGrottoGame");
}

CResource<CLevel> CreateLevel()
{
	return new CLevel();
}

CHUDViewport* CreateHUD()
{
	CHUDViewport* pHUD = new CGrottoHUD();
	return pHUD;
}

tstring GetInitialGameMode()
{
	return "menu";
}

REGISTER_ENTITY(CGrottoGame);

NETVAR_TABLE_BEGIN(CGrottoGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrottoGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoGame);
INPUTS_TABLE_END();

void CGrottoGame::SetupGame(tstring sType)
{
	if (sType == "level")
	{
		CGrottoPlayer* pPlayer = GameServer()->Create<CGrottoPlayer>("CGrottoPlayer");
		Game()->AddPlayer(pPlayer);

		CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
		pCharacter->SetGlobalOrigin(Vector(0, 0, 0));
		pPlayer->SetCharacter(pCharacter);

		GameServer()->LoadLevel(CVar::GetCVarValue("game_level"));

		pCharacter->MoveToPlayerStart();

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

void CGrottoGame::Precache()
{
}

void CGrottoGame::Think()
{
	BaseClass::Think();
}

CGrottoCharacter* CGrottoGame::GetLocalPlayerCharacter()
{
	CPlayer* pPlayer = GetLocalPlayer();
	if (!pPlayer)
		return nullptr;

	return static_cast<CGrottoCharacter*>(pPlayer->GetCharacter());
}

CGrottoRenderer* CGrottoGame::GetGrottoRenderer()
{
	return static_cast<CGrottoRenderer*>(GameServer()->GetRenderer());
}
