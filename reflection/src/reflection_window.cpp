#include "reflection_window.h"

#include <time.h>

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <mtrand.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/game.h>
#include <glgui/rootpanel.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <game/camera.h>
#include <game/entities/kinematic.h>

#include "reflection_player.h"
#include "reflection_playercharacter.h"
#include "reflection_renderer.h"
#include "world.h"
#include "token.h"
#include "ui/hud.h"

CReflectionWindow::CReflectionWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
}

void CReflectionWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->AddControl(m_pHUD = new CReflectionHUD());

	glgui::CRootPanel::Get()->SetLighting(false);
	glgui::CRootPanel::Get()->Layout();

	SetupReflection();

	GameServer()->SetLoading(false);

	CApplication::Get()->SetMouseCursorEnabled(false);
}

void CReflectionWindow::SetupReflection()
{
	CWorld* pWorld = GameServer()->Create<CWorld>("CWorld");

	CReflectionPlayer* pPlayer = GameServer()->Create<CReflectionPlayer>("CReflectionPlayer");
	Game()->AddPlayer(pPlayer);

	CPlayerCharacter* pCharacter = GameServer()->Create<CPlayerCharacter>("CPlayerCharacter");
	pCharacter->SetGlobalOrigin(Vector(-1, 0, -1));
	pPlayer->SetCharacter(pCharacter);

	CMirror* pMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetGlobalOrigin(Vector(-6, 0, -3));
	pMirror->SetGlobalAngles(EAngle(0, 90, 0));

	pMirror = GameServer()->Create<CMirror>("CMirror");
	pMirror->SetGlobalOrigin(Vector(0, 0, 0));
	pMirror->SetGlobalAngles(EAngle(0, 0, 0));

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
}

void CReflectionWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	SwapBuffers();
}

CReflectionRenderer* CReflectionWindow::GetRenderer()
{
	return static_cast<CReflectionRenderer*>(GameServer()->GetRenderer());
}
