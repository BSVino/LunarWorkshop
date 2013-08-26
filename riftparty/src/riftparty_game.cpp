#include "riftparty_game.h"

#include <tinker/cvar.h>
#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>
#include <game/entities/kinematic.h>
#include <game/entities/mathgate.h>

#include "riftparty_player.h"
#include "riftparty_character.h"
#include "riftparty_camera.h"
#include "riftparty_renderer.h"
#include "riftparty_playercharacter.h"
#include "ui/hud.h"

CGame* CreateGame()
{
	return GameServer()->Create<CRiftPartyGame>("CRiftPartyGame");
}

CCamera* CreateCamera()
{
	CCamera* pCamera = new CRiftPartyCamera();
	return pCamera;
}

CResource<CLevel> CreateLevel()
{
	return CResource<CLevel>(new CLevel());
}

CHUDViewport* CreateHUD()
{
	CHUDViewport* pHUD = new CRiftPartyHUD();
	return pHUD;
}

tstring GetInitialGameMode()
{
	return "level";
}

REGISTER_ENTITY(CRiftPartyGame);

NETVAR_TABLE_BEGIN(CRiftPartyGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CRiftPartyGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CRiftPartyGame);
INPUTS_TABLE_END();

void CRiftPartyGame::SetupGame(tstring sType)
{
	if (sType == "level")
	{
		CRiftPartyPlayer* pPlayer = GameServer()->Create<CRiftPartyPlayer>("CRiftPartyPlayer");
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

void CRiftPartyGame::Precache()
{
}

void CRiftPartyGame::Think()
{
	BaseClass::Think();
}

CRiftPartyCharacter* CRiftPartyGame::GetLocalPlayerCharacter()
{
	CPlayer* pPlayer = GetLocalPlayer();
	if (!pPlayer)
		return nullptr;

	return static_cast<CRiftPartyCharacter*>(pPlayer->GetCharacter());
}

CRiftPartyRenderer* CRiftPartyGame::GetRiftPartyRenderer()
{
	return static_cast<CRiftPartyRenderer*>(GameServer()->GetRenderer());
}

pfnConditionsMet Game_GetInstructorConditions(const tstring& sConditions)
{
	return false;
}
