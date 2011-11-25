#include "chain_game.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>

#include "chain_player.h"
#include "chain_camera.h"
#include "chain_renderer.h"
#include "chain_playercharacter.h"
#include "story.h"
#include "ui/hud.h"

CGame* CreateGame()
{
	return GameServer()->Create<CChainGame>("CChainGame");
}

CRenderer* CreateRenderer()
{
	return new CChainRenderer();
}

CCamera* CreateCamera()
{
	CCamera* pCamera = new CChainCamera();
	return pCamera;
}

CLevel* CreateLevel()
{
	return new CLevel();
}

CHUDViewport* CreateHUD()
{
	CHUDViewport* pHUD = new CChainHUD();
	return pHUD;
}

tstring GetInitialGameMode()
{
	return "demo";
}

REGISTER_ENTITY(CChainGame);

NETVAR_TABLE_BEGIN(CChainGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CChainGame);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CStory>, m_hStory);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CChainGame);
INPUTS_TABLE_END();

void CChainGame::SetupGame(tstring sType)
{
	CChainPlayer* pPlayer = GameServer()->Create<CChainPlayer>("CChainPlayer");
	Game()->AddPlayer(pPlayer);

	CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
	pCharacter->SetGlobalOrigin(Vector(0, 0, 0));
	pPlayer->SetCharacter(pCharacter);

	CStory* pStory = GameServer()->Create<CStory>("CStory");
}

void CChainGame::Precache()
{
}

void CChainGame::Think()
{
	BaseClass::Think();
}

CPlayerCharacter* CChainGame::GetLocalPlayerCharacter()
{
	return static_cast<CPlayerCharacter*>(GetLocalPlayer()->GetCharacter());
}

CChainRenderer* CChainGame::GetChainRenderer()
{
	return static_cast<CChainRenderer*>(GameServer()->GetRenderer());
}

CChainCamera* CChainGame::GetChainCamera()
{
	return static_cast<CChainCamera*>(GameServer()->GetCamera());
}

CStory* CChainGame::GetStory() const
{
	return m_hStory;
}

void CChainGame::SetStory(CStory* pStory)
{
	m_hStory = pStory;
}
