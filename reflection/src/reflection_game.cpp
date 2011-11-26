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
	return "demo";
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
	CReflectionPlayer* pPlayer = GameServer()->Create<CReflectionPlayer>("CReflectionPlayer");
	Game()->AddPlayer(pPlayer);

	CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
	pCharacter->SetGlobalOrigin(Vector(0, 0, 0));
	pPlayer->SetCharacter(pCharacter);

	GameServer()->LoadLevel("levels/1.txt");

/*
	CToken* pToken = GameServer()->Create<CToken>("CToken");
	pToken->SetModel("models/r.toy");
	pToken->SetGlobalOrigin(Vector(-6, 0, 3));
	pToken->SetGlobalAngles(EAngle(0, 90, 0));
	pToken->SetName("Reflection");
	pToken->SetReflected(true);

	CReceptacle* pReceptacle = GameServer()->Create<CReceptacle>("CReceptacle");
	pReceptacle->SetGlobalOrigin(Vector(-7.5f, 0.8f, 2.5f));
	pReceptacle->SetGlobalAngles(EAngle(45, 180, 0));
	pReceptacle->SetDesiredToken("Reflection");
	pReceptacle->AddOutputTarget("OnNormalToken", "door1", "LerpTo", "-9.26569 3.84284 5.56737");
	pReceptacle->AddOutputTarget("OnNormalTokenRemoved", "door1", "LerpTo", "-9.26569 1.31136 5.56737");

	CKinematic* pDoor = GameServer()->Create<CKinematic>("CKinematic");
	pDoor->SetName("door1");
	pDoor->SetModel("models/door.toy");
	pDoor->SetGlobalOrigin(Vector(-9.26569f, 1.31136f, 5.56737f));
	pDoor->SetLerpTime(0.5f);

	pMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetMirrorType(MIRROR_HORIZONTAL);
	pMirror->SetGlobalOrigin(Vector(-12.391769f, 0.13496184f, 0.55572152f));
	pMirror->SetGlobalAngles(EAngle(0, 0, 0));

	pMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetMirrorType(MIRROR_HORIZONTAL);
	pMirror->SetGlobalOrigin(Vector(-29.102150f, 9.1320658f, -2.0262783f));
	pMirror->SetGlobalAngles(EAngle(180, 0, 0));

	pMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetMirrorType(MIRROR_HORIZONTAL);
	pMirror->SetGlobalOrigin(Vector(-29.102150f, 0.13496184f, -4.55572152f));
	pMirror->SetGlobalAngles(EAngle(0, 0, 0));

	pToken = GameServer()->Create<CToken>("CToken");
	pToken->SetModel("models/powersource.toy");
	pToken->SetGlobalOrigin(Vector(-28.586233f, 0.13499284f, -3.1609130f));
	pToken->SetGlobalAngles(EAngle(0, 90, 0));
	pToken->SetName("ps1");
	pToken->SetReflected(true);

	pReceptacle = GameServer()->Create<CReceptacle>("CReceptacle");
	pReceptacle->SetGlobalOrigin(Vector(-35.042164f, 8.3557692f, 0.0f));
	pReceptacle->SetGlobalAngles(EAngle(-135.0f, 0, 0));
	pReceptacle->SetDesiredToken("ps1");
	pReceptacle->AddOutputTarget("OnNormalToken", "door2", "LerpAnglesTo", "0 0 180");
	pReceptacle->AddOutputTarget("OnReflectedToken", "door2", "LerpAnglesTo", "0 0 0");
	pReceptacle->AddOutputTarget("OnTokenRemoved", "door2", "LerpAnglesTo", "0 0 90");

	pDoor = GameServer()->Create<CKinematic>("CKinematic");
	pDoor->SetName("door2");
	pDoor->SetModel("models/vaultdoor.toy");
	pDoor->SetGlobalOrigin(Vector(-41.62f, 4.62605f, -0.107501f));
	pDoor->SetGlobalAngles(EAngle(0, 0, 90));
	pDoor->SetAngleLerpTime(5);

	// Puzzle 3
	pToken = GameServer()->Create<CToken>("CToken");
	pToken->SetModel("models/powersource.toy");
	pToken->SetGlobalOrigin(Vector(-51, 0, -2.72518f));
	pToken->SetGlobalAngles(EAngle(0, 0, 0));

	pReceptacle = GameServer()->Create<CReceptacle>("CReceptacle");
	pReceptacle->SetGlobalOrigin(Vector(-49.7083f, 0.899958f, -4.45906f));
	pReceptacle->SetGlobalAngles(EAngle(45, -90, 0));
	pReceptacle->AddOutputTarget("OnNormalToken", "p3_add", "InputLeft", "90");
	pReceptacle->AddOutputTarget("OnReflectedToken", "p3_add", "InputLeft", "-90");
	pReceptacle->AddOutputTarget("OnTokenRemoved", "p3_add", "InputLeft", "0");

	pReceptacle = GameServer()->Create<CReceptacle>("CReceptacle");
	pReceptacle->SetGlobalOrigin(Vector(-52.9024f, 0.899958f, -4.45906f));
	pReceptacle->SetGlobalAngles(EAngle(45, -90, 0));
	pReceptacle->AddOutputTarget("OnNormalToken", "p3_add", "InputRight", "90");
	pReceptacle->AddOutputTarget("OnReflectedToken", "p3_add", "InputRight", "-90");
	pReceptacle->AddOutputTarget("OnTokenRemoved", "p3_add", "InputRight", "0");

	CMathGate* pMath = GameServer()->Create<CMathGate>("CMathGate");
	pMath->SetName("p3_add");
	pMath->SetBaseValue(0);
	pMath->AddOutputTarget("OnResult", "p3_rotdoor", "LerpAnglesTo", "0 90 [0]");

	pReceptacle = GameServer()->Create<CReceptacle>("CReceptacle");
	pReceptacle->SetGlobalOrigin(Vector(-55.7814f, 0.899958f, -0.0976079f));
	pReceptacle->SetGlobalAngles(EAngle(45, 180, 0));
	pReceptacle->AddOutputTarget("OnNormalToken", "p3_door", "LerpTo", "-59.8467 3.70249 -0.132385");
	pReceptacle->AddOutputTarget("OnReflectedToken", "p3_door", "LerpTo", "-59.8467 -1.23898 -0.132385");
	pReceptacle->AddOutputTarget("OnTokenRemoved", "p3_door", "LerpTo", "-59.8467 1.31136 -0.132385");

	pMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetGlobalOrigin(Vector(-66.1031f, 0, 3.21579f));
	pMirror->SetGlobalAngles(EAngle(0, 90, 0));

	pDoor = GameServer()->Create<CKinematic>("CKinematic");
	pDoor->SetName("p3_rotdoor");
	pDoor->SetModel("models/vaultdoor.toy");
	pDoor->SetGlobalOrigin(Vector(-51.5633f, 4.62605f, -8.92617f));
	pDoor->SetGlobalAngles(EAngle(0, 90, 0));
	pDoor->SetAngleLerpTime(5);

	pToken = GameServer()->Create<CToken>("CToken");
	pToken->SetModel("models/powersource.toy");
	pToken->SetGlobalOrigin(Vector(-66.3881f, 0.1f, -0.0496612f));
	pToken->SetGlobalAngles(EAngle(0, 0, 0));
	pToken->SetReflected(true);

	pDoor = GameServer()->Create<CKinematic>("CKinematic");
	pDoor->SetName("p3_door");
	pDoor->SetModel("models/door.toy");
	pDoor->SetGlobalOrigin(Vector(-59.8467f, 1.31136f, -0.132385f));
	pDoor->SetLerpTime(0.5f);*/
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
