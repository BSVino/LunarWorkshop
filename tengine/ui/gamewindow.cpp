#include "gamewindow.h"

#include <time.h>

#include <tinker_platform.h>
#include <mtrand.h>

#include <tinker/profiler.h>
#include <tinker/cvar.h>
#include <game/gameserver.h>
#include <glgui/rootpanel.h>
#include <game/camera.h>
#include <renderer/renderer.h>
#include <tengine/game/entities/game.h>
#include <ui/hudviewport.h>
#include <game/level.h>
#include <tools/workbench.h>
#include <renderer/particles.h>
#include <renderer/game_renderer.h>
#include <tinker/keys.h>

CGameWindow::CGameWindow(int argc, char** argv)
	: CApplication(argc, argv)
{
	m_bHaveLastMouse = false;
}

void CGameWindow::OpenWindow()
{
	int iScreenWidth, iScreenHeight;
	GetScreenSize(iScreenWidth, iScreenHeight);

#ifdef _DEBUG
	BaseClass::OpenWindow(iScreenWidth*2/3, iScreenHeight*2/3, false, true);
#else
	BaseClass::OpenWindow(iScreenWidth*2/3, iScreenHeight*2/3, false, true);
#endif

	RenderLoading();

	CVar::SetCVar("game_mode", GetInitialGameMode());

	m_pGameServer = new CGameServer();

	m_pRenderer = CreateRenderer();
	m_pRenderer->Initialize();

	mtsrand((size_t)time(NULL));

	glgui::CRootPanel::Get()->AddControl(m_pHUD = CreateHUD());

	glgui::CRootPanel::Get()->SetLighting(false);
	glgui::CRootPanel::Get()->SetSize((float)GetWindowWidth(), (float)GetWindowHeight());
	glgui::CRootPanel::Get()->Layout();

	GameServer()->SetLoading(false);

	CApplication::Get()->SetMouseCursorEnabled(false);
}

CGameWindow::~CGameWindow()
{
	if (m_pRenderer)
		delete m_pRenderer;

	if (m_pGameServer)
		delete m_pGameServer;
}

void LoadLevel(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	if (asTokens.size() == 1)
	{
		if (!GameServer())
		{
			TMsg("Use load_level 'levelpath' to specify the level.\n");
			return;
		}

		CLevel* pLevel = GameServer()->GetLevel(CVar::GetCVarValue("game_level"));

		if (!pLevel)
		{
			TMsg(tstring("Can't find file '") + CVar::GetCVarValue("game_level") + "'.\n");
			return;
		}

		CVar::SetCVar("game_level", pLevel->GetFile());

		// Need to tuck this away since levels are deleted when the GameServer is destroyed
		tstring sGameMode = pLevel->GetGameMode();

		GameWindow()->DestroyGame();
		GameWindow()->CreateGame(sGameMode);
		return;
	}

	CLevel* pLevel = GameServer()->GetLevel(asTokens[1]);

	if (!pLevel)
	{
		TMsg(tstring("Can't find file '") + asTokens[1] + "'.\n");
		return;
	}

	CVar::SetCVar("game_level", pLevel->GetFile());

	// Need to tuck this away since levels are deleted when the GameServer is destroyed
	tstring sGameMode = pLevel->GetGameMode();

	GameWindow()->DestroyGame();
	GameWindow()->CreateGame(sGameMode);

	CApplication::CloseConsole();
}

CCommand level_load("level_load", ::LoadLevel);
CVar game_mode("game_mode", "");		// Are we in the menu or in the game or what?
CVar game_level("game_level", "");

void CGameWindow::CreateGame(const tstring& sRequestedGameMode)
{
	game_mode.SetValue(sRequestedGameMode);

	// Suppress all network commands until the game is done loading.
	GameNetwork()->SetLoading(true);

	RenderLoading();

	mtsrand((size_t)time(NULL));

	const char* pszPort = GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (!m_pGameServer)
	{
		m_pHUD = CreateHUD();
		glgui::CRootPanel::Get()->AddControl(m_pHUD);

		m_pGameServer = new CGameServer();

		if (!m_pRenderer)
		{
			m_pRenderer = CreateRenderer();
			m_pRenderer->Initialize();
		}
	}

	GameServer()->AllowPrecaches();

	GameServer()->SetServerPort(iPort);
	GameServer()->Initialize();

	GameNetwork()->SetCallbacks(m_pGameServer, CGameServer::ClientConnectCallback, CGameServer::ClientEnterGameCallback, CGameServer::ClientDisconnectCallback);

	// Now turn the network on and connect all clients.
	GameNetwork()->SetLoading(false);

	glgui::CRootPanel::Get()->Layout();

	Game()->SetupGame(game_mode.GetValue());

	GameServer()->PrecacheList();

	GameServer()->SetLoading(false);
}

void CGameWindow::DestroyGame()
{
	TMsg("Destroying game.\n");

	RenderLoading();

	if (m_pGameServer)
		delete m_pGameServer;

	if (m_pHUD)
	{
		glgui::CRootPanel::Get()->RemoveControl(m_pHUD);
		delete m_pHUD;
	}

	m_pGameServer = NULL;
	m_pHUD = NULL;
}

void ReloadLevel(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	GameWindow()->ReloadLevel();
}

CCommand level_reload("level_reload", ::ReloadLevel);

void CGameWindow::ReloadLevel()
{
	GameServer()->SetLoading(true);

	// Suppress all network commands until the game is done loading.
	GameNetwork()->SetLoading(true);

	RenderLoading();

	mtsrand((size_t)time(NULL));

	CParticleSystemLibrary::ClearInstances();
	GameServer()->DestroyAllEntities(eastl::vector<eastl::string>(), true);

	// Now turn the network on and connect all clients.
	GameNetwork()->SetLoading(false);

	glgui::CRootPanel::Get()->Layout();

	Game()->SetupGame(game_mode.GetValue());

	GameServer()->SetLoading(false);
}

void CGameWindow::Restart(tstring sGameMode)
{
	m_sRestartGameMode = sGameMode;
	GameServer()->Halt();
}

void CGameWindow::Run()
{
	CreateGame(GetInitialGameMode());

	while (IsOpen())
	{
		CProfiler::BeginFrame();

		if (GameServer()->IsHalting())
		{
			DestroyGame();
			CreateGame(m_sRestartGameMode);
		}

		if (true)
		{
			TPROF("CGameWindow::Run");

			PreFrame();

			float flTime = GetTime();
			if (GameServer())
			{
				if (GameServer()->IsLoading())
				{
					// Pump the network
					CNetwork::Think();
					RenderLoading();
					continue;
				}
				else if (GameServer()->IsClient() && !GameNetwork()->IsConnected())
				{
					DestroyGame();
					CreateGame(m_sRestartGameMode);
				}
				else
				{
					GameServer()->Think(flTime);
					Render();
				}
			}

			PostFrame();
		}

		CProfiler::Render();
		SwapBuffers();
	}
}

void CGameWindow::PreFrame()
{
	GameServer()->GetRenderer()->PreFrame();
}

void CGameWindow::PostFrame()
{
	GameServer()->GetRenderer()->PostFrame();
}

void CGameWindow::Render()
{
	if (!GameServer())
		return;

	TPROF("CGameWindow::Render");

	GameServer()->Render();

	if (true)
	{
		TPROF("GUI");
		glgui::CRootPanel::Get()->Think(GameServer()->GetGameTime());
		glgui::CRootPanel::Get()->Paint(0, 0, (float)m_iWindowWidth, (float)m_iWindowHeight);
	}
}

bool CGameWindow::KeyPress(int c)
{
	if (BaseClass::KeyPress(c))
		return true;

	if (CWorkbench::IsActive())
	{
		if (Workbench()->KeyPress(c))
			return true;
	}

	if (GameServer() && GameServer()->GetCamera())
	{
		if (GameServer()->GetCamera()->KeyDown(c))
			return true;
	}

	if (Game())
	{
		for (size_t i = 0; i < Game()->GetNumLocalPlayers(); i++)
		{
			CPlayer* pPlayer = Game()->GetLocalPlayer(i);
			pPlayer->KeyPress(c);
		}
	}

	if (c == TINKER_KEY_F2)
	{
		if (CWorkbench::IsActive() || CVar::GetCVarBool("cheats"))
			CWorkbench::Toggle();
	}

	return false;
}

void CGameWindow::KeyRelease(int c)
{
	BaseClass::KeyRelease(c);

	if (GameServer() && GameServer()->GetCamera())
	{
		if (GameServer()->GetCamera()->KeyUp(c))
			return;
	}

	if (Game())
	{
		for (size_t i = 0; i < Game()->GetNumLocalPlayers(); i++)
		{
			CPlayer* pPlayer = Game()->GetLocalPlayer(i);
			pPlayer->KeyRelease(c);
		}
	}
}

void CGameWindow::MouseMotion(int x, int y)
{
	if (!HasFocus())
	{
		m_bHaveLastMouse = false;
		return;
	}

	BaseClass::MouseMotion(x, y);

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->MouseInput(x, y);

	if (Game() && m_bHaveLastMouse)
	{
		int dx = x - m_iLastMouseX;
		int dy = y - m_iLastMouseY;

		for (size_t i = 0; i < Game()->GetNumLocalPlayers(); i++)
		{
			CPlayer* pPlayer = Game()->GetLocalPlayer(i);
			pPlayer->MouseMotion(dx, dy);
		}
	}

	m_bHaveLastMouse = true;
	m_iLastMouseX = x;
	m_iLastMouseY = y;
}

bool CGameWindow::MouseInput(int iButton, int iState)
{
	if (BaseClass::MouseInput(iButton, iState))
		return true;

	if (CWorkbench::IsActive())
	{
		if (Workbench()->MouseInput(iButton, iState))
			return true;
	}

	if (GameServer() && GameServer()->GetCamera())
	{
		if (GameServer()->GetCamera()->MouseButton(iButton, iState))
			return true;
	}

	if (Game())
	{
		for (size_t i = 0; i < Game()->GetNumLocalPlayers(); i++)
		{
			CPlayer* pPlayer = Game()->GetLocalPlayer(i);
			pPlayer->MouseInput(iButton, iState);
		}
	}

	return false;
}
