#include "game.h"

#include <iostream>
#include <fstream>

#include <mtrand.h>
#include <tinker_platform.h>
#include <configfile.h>
#include <strutils.h>
#include <worklistener.h>

#include <tinker/application.h>
#include <tinker/profiler.h>
#include <renderer/game_renderer.h>
#include <renderer/particles.h>
#include <sound/sound.h>
#include <network/network.h>
#include <network/commands.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>
#include <models/models.h>
#include <textures/texturelibrary.h>
#include <tinker/portals/portal.h>
#include <tinker/lobby/lobby_server.h>
#include <tinker/cvar.h>
#include <ui/gamewindow.h>
#include <physics/physics.h>

#include "camera.h"
#include "level.h"

eastl::map<tstring, CPrecacheItem> CGameServer::s_aPrecacheClasses;
CGameServer* CGameServer::s_pGameServer = NULL;

ConfigFile g_cfgEngine("scripts/engine.cfg");

CGameServer::CGameServer(IWorkListener* pWorkListener)
{
	TAssert(!s_pGameServer);
	s_pGameServer = this;

	m_bAllowPrecaches = false;

	GameNetwork()->SetCallbacks(this, CGameServer::ClientConnectCallback, CGameServer::ClientEnterGameCallback, CGameServer::ClientDisconnectCallback);

	m_pWorkListener = pWorkListener;

	m_iMaxEnts = g_cfgEngine.read("MaxEnts", 1024);

	CBaseEntity::s_apEntityList.resize(m_iMaxEnts);

	m_pCamera = NULL;

	m_iSaveCRC = 0;

	m_bLoading = true;

	m_flHostTime = 0;
	m_flGameTime = 0;
	m_flFrameTime = 0;
	m_flNextClientInfoUpdate = 0;
	m_iFrame = 0;

	size_t iPostSeed = mtrand();

	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	TMsg("Creating physics model... ");
	GamePhysics();	// Just make sure it exists.
	TMsg("Done.\n");

	TMsg("Registering entities... ");

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Registering entities", CBaseEntity::GetEntityRegistration().size());

	size_t i = 0;
	for (eastl::map<tstring, CEntityRegistration>::iterator it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
	{
		CEntityRegistration* pRegistration = &it->second;
		pRegistration->m_pfnRegisterCallback();

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(++i);
	}
	TMsg("Done.\n");

	mtsrand(iPostSeed);

	CBaseEntity::s_iNextEntityListIndex = 0;

	m_iPort = 0;
	m_iClient = NETWORK_LOCAL;

	m_bHalting = false;

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

CGameServer::~CGameServer()
{
	GameNetwork()->SetCallbacks(NULL, NULL, NULL, NULL);

	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Scrubbing database", CBaseEntity::GetEntityRegistration().size());

	DestroyAllEntities(eastl::vector<eastl::string>());

	GamePhysics()->RemoveAllEntities();

	for (size_t i = 0; i < m_apLevels.size(); i++)
		delete m_apLevels[i];

	if (m_pCamera)
		delete m_pCamera;

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	TAssert(s_pGameServer == this);
	s_pGameServer = NULL;
}

void CGameServer::AllowPrecaches()
{
	m_bAllowPrecaches = true;

	CModelLibrary::ResetReferenceCounts();
	CTextureLibrary::ResetReferenceCounts();
	CSoundLibrary::ResetReferenceCounts();
	CParticleSystemLibrary::ResetReferenceCounts();

	for (eastl::map<tstring, CEntityRegistration>::iterator it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
	{
		CEntityRegistration* pRegistration = &it->second;
		pRegistration->m_asPrecaches.clear();
	}
}

void CGameServer::AddToPrecacheList(const tstring& sClass)
{
	TAssert(m_bAllowPrecaches || IsLoading());

	auto it = s_aPrecacheClasses.find(sClass);
	if (it != s_aPrecacheClasses.end())
		return;

	auto it2 = CBaseEntity::GetEntityRegistration().find(sClass);
	if (it2 == CBaseEntity::GetEntityRegistration().end())
		return;

	CPrecacheItem o;
	o.m_sClass = sClass;

	s_aPrecacheClasses[sClass] = o;

	if (it2->second.m_pszParentClass)
		AddToPrecacheList(it2->second.m_pszParentClass);
}

void CGameServer::PrecacheList()
{
	TMsg("Precaching entities... ");

	size_t iPrecaches = s_aPrecacheClasses.size();
	if (m_pWorkListener)
		m_pWorkListener->SetAction("Precaching entities", iPrecaches);

	size_t i = 0;

	for (auto it = s_aPrecacheClasses.begin(); it != s_aPrecacheClasses.end(); it++)
	{
		CPrecacheItem* pPrecacheItem = &it->second;

		CEntityRegistration* pRegistration = &CBaseEntity::GetEntityRegistration()[pPrecacheItem->m_sClass];
		pRegistration->m_pfnPrecacheCallback();

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(++i);
	}

	s_aPrecacheClasses.clear();

	// Do this in this order, dependencies matter
	CParticleSystemLibrary::ClearUnreferenced();
	CModelLibrary::ClearUnreferenced();
	CTextureLibrary::ClearUnreferenced();
	CSoundLibrary::ClearUnreferenced();

	TMsg("Done.\n");
	TMsg(sprintf(tstring("%d models, %d textures, %d sounds and %d particle systems precached.\n"), CModelLibrary::GetNumModelsLoaded(), CTextureLibrary::GetNumTextures(), CSoundLibrary::GetNumSoundsLoaded(), CParticleSystemLibrary::GetNumParticleSystemsLoaded()));

	m_bAllowPrecaches = false;
}

CLIENT_GAME_COMMAND(SendNickname)
{
	TAssert(GameServer());
	if (!GameServer())
		return;

	GameServer()->SetClientNickname(iClient, sParameters);
}

void CGameServer::SetPlayerNickname(const tstring& sNickname)
{
	m_sNickname = sNickname;

	if (GameNetwork()->IsHost() || m_bGotClientInfo)
		SendNickname.RunCommand(m_sNickname);
}

void CGameServer::Initialize()
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	m_bGotClientInfo = false;
	m_bLoading = true;

	TMsg("Initializing game server\n");

	ReadLevels();

	GameNetwork()->ClearRegisteredFunctions();

	RegisterNetworkFunctions();

	DestroyAllEntities(eastl::vector<eastl::string>(), true);

	CParticleSystemLibrary::ClearInstances();

	if (!m_pCamera)
		m_pCamera = CreateCamera();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Pending network actions", 0);
}

void CGameServer::LoadLevel(tstring sFile)
{
	CLevel* pLevel = GetLevel(sFile);

	std::basic_ifstream<tchar> f(convertstring<tchar, char>(sFile).c_str());

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	// Create and name the entities first and add them to this array. This way we avoid a problem where
	// one entity needs to connect to another entity which has not yet been created.
	eastl::map<size_t, CBaseEntity*> apEntities;

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Entity")
		{
			tstring sClass = "C" + pChildData->GetValueTString();

			auto it = CBaseEntity::GetEntityRegistration().find(sClass);
			TAssert(it != CBaseEntity::GetEntityRegistration().end());
			if (it == CBaseEntity::GetEntityRegistration().end())
			{
				TError("Unregistered entity '" + sClass + "'\n");
				continue;
			}

			AddToPrecacheList(sClass);

			CBaseEntity* pEntity = Create<CBaseEntity>(sClass.c_str());

			apEntities[i] = pEntity;

			CData* pNameData = pChildData->FindChild("Name");
			if (pNameData)
				pEntity->SetName(pNameData->GetValueTString());

			// Process outputs here so that they exist when handle callbacks run.
			for (size_t k = 0; k < pChildData->GetNumChildren(); k++)
			{
				CData* pField = pChildData->GetChild(k);

				tstring sHandle = pField->GetKey();
				tstring sValue = pField->GetValueTString();

				if (sHandle == "Output")
				{
					CSaveData* pSaveData = CBaseEntity::GetOutput(pEntity->GetClassName(), sValue);
					TAssert(pSaveData);
					if (!pSaveData)
					{
						TError("Unknown output '" + sValue + "'\n");
						continue;
					}

					tstring sTarget;
					tstring sInput;
					tstring sArgs;
					bool bKill = false;

					for (size_t o = 0; o < pField->GetNumChildren(); o++)
					{
						CData* pOutputData = pField->GetChild(o);

						if (pOutputData->GetKey() == "Target")
							sTarget = pOutputData->GetValueString();
						else if (pOutputData->GetKey() == "Input")
							sInput = pOutputData->GetValueString();
						else if (pOutputData->GetKey() == "Args")
							sArgs = pOutputData->GetValueString();
						else if (pOutputData->GetKey() == "Kill")
							bKill = pOutputData->GetValueBool();
					}

					if (!sTarget.length())
					{
						TAssert(false);
						TError("Output '" + sValue + "' of entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ") is missing a target.\n");
						continue;
					}

					if (!sInput.length())
					{
						TAssert(false);
						TError("Output '" + sValue + "' of entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ") is missing an input.\n");
						continue;
					}

					pEntity->AddOutputTarget(sValue, sTarget, sInput, sArgs, bKill);
				}
			}
		}
	}

	for (auto it = apEntities.begin(); it != apEntities.end(); it++)
	{
		CData* pChildData = pData->GetChild(it->first);
		CBaseEntity* pEntity = it->second;

		for (size_t k = 0; k < pChildData->GetNumChildren(); k++)
		{
			CData* pField = pChildData->GetChild(k);

			tstring sHandle = pField->GetKey();
			tstring sValue = pField->GetValueTString();

			if (sHandle != "Output")
			{
				CSaveData* pSaveData = CBaseEntity::GetSaveDataByHandle(pEntity->GetClassName(), sHandle.c_str());
				TAssert(pSaveData);
				if (!pSaveData)
				{
					TError("Unknown handle '" + sHandle + "'\n");
					continue;
				}

				TAssert(pSaveData->m_pfnUnserializeString);
				if (!pSaveData->m_pfnUnserializeString)
					continue;

				pSaveData->m_pfnUnserializeString(sValue, pSaveData, pEntity);
			}
		}
	}

	delete pData;
}

void CGameServer::ReadLevels()
{
	for (size_t i = 0; i < m_apLevels.size(); i++)
		delete m_apLevels[i];

	m_apLevels.clear();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading meta-structures", 0);

	ReadLevels("levels");

	TMsg(sprintf(tstring("Read %d levels from disk.\n"), m_apLevels.size()));
}

void CGameServer::ReadLevels(tstring sDirectory)
{
	eastl::vector<tstring> asFiles = ListDirectory(sDirectory);

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		tstring sFile = sDirectory + "/" + asFiles[i];

		if (IsFile(sFile) && sFile.substr(sFile.length()-4).compare(".txt") == 0)
			ReadLevelInfo(sFile);

		if (IsDirectory(sFile))
			ReadLevels(sFile);
	}
}

void CGameServer::ReadLevelInfo(tstring sFile)
{
	std::basic_ifstream<tchar> f(convertstring<tchar, char>(sFile).c_str());

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CLevel* pLevel = CreateLevel();
	pLevel->SetFile(str_replace(sFile, "\\", "/"));
	pLevel->ReadInfoFromData(pData);
	m_apLevels.push_back(pLevel);

	delete pData;
}

CLevel* CGameServer::GetLevel(tstring sFile)
{
	sFile = str_replace(sFile, "\\", "/");
	for (size_t i = 0; i < m_apLevels.size(); i++)
	{
		CLevel* pLevel = m_apLevels[i];
		tstring sLevelFile = pLevel->GetFile();
		if (sLevelFile == sFile)
			return pLevel;
		if (sLevelFile == sFile + ".txt")
			return pLevel;
		if (sLevelFile == tstring("levels/") + sFile)
			return pLevel;
		if (sLevelFile == tstring("levels/") + sFile + ".txt")
			return pLevel;
	}

	return NULL;
}

void CGameServer::Halt()
{
	m_bHalting = true;
}

void CGameServer::RegisterNetworkFunctions()
{
	GameNetwork()->RegisterFunction("UV", this, UpdateValueCallback, 2, NET_HANDLE, NET_HANDLE);

	GameNetwork()->RegisterFunction("ClientInfo", this, ClientInfoCallback, 2, NET_INT, NET_FLOAT);
	GameNetwork()->RegisterFunction("DestroyEntity", this, DestroyEntityCallback, 1, NET_INT);
	GameNetwork()->RegisterFunction("LoadingDone", this, LoadingDoneCallback, 0);
}

void CGameServer::ClientConnect(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	ClientConnect(p->i1);
}

void CGameServer::ClientEnterGame(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	ClientEnterGame(p->i1);
}

void CGameServer::LoadingDone(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	m_bLoading = false;
}

void CGameServer::ClientDisconnect(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	ClientDisconnect(p->i1);
}

void CGameServer::ClientConnect(int iClient)
{
	GameNetwork()->CallFunction(iClient, "ClientInfo", iClient, GetGameTime());

	if (GetGame())
		GetGame()->OnClientConnect(iClient);
}

SERVER_GAME_COMMAND(CreateEntity)
{
	if (pCmd->GetNumArguments() < 3)
	{
		TError("CreateEntity with too few arguments.");
		return;
	}

	GameServer()->CreateEntity(pCmd->Arg(0), pCmd->ArgAsUInt(1), pCmd->ArgAsUInt(2));
}

void CGameServer::ClientEnterGame(int iClient)
{
	TMsg(sprintf(tstring("Client %d (") + GameNetwork()->GetClientNickname(iClient) + ") entering game.\n", iClient));

	if (GetGame())
		GetGame()->OnClientEnterGame(iClient);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		::CreateEntity.RunCommand(sprintf(tstring("%s %d %d"), pEntity->GetClassName(), pEntity->GetHandle(), pEntity->GetSpawnSeed()), iClient);
	}

	CGameServerNetwork::UpdateNetworkVariables(iClient, true);

	// Update entities after all creations have been run, so we don't refer to entities that haven't been created yet.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		pEntity->ClientUpdate(iClient);
	}

	GameNetwork()->CallFunction(iClient, "EnterGame");

	GameNetwork()->CallFunction(iClient, "LoadingDone");
}

void CGameServer::ClientDisconnect(int iClient)
{
	if (!GameNetwork()->IsHost() && iClient == GameNetwork()->GetClientID())
	{
		TMsg("Disconnected from server.\n");
	}
	else
	{
		TMsg(sprintf(tstring("Client %d (") + GameNetwork()->GetClientNickname(iClient) + ") disconnected.\n", iClient));

		CApplication::Get()->OnClientDisconnect(iClient);

		if (GetGame())
			GetGame()->OnClientDisconnect(iClient);
	}
}

void CGameServer::SetClientNickname(int iClient, const tstring& sNickname)
{
	if (iClient == GetClientIndex() && Game()->GetNumLocalPlayers())
	{
		Game()->GetLocalPlayer(0)->SetPlayerName(sNickname);
		return;
	}

	for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
	{
		if (Game()->GetPlayer(i)->GetClient() == iClient)
		{
			Game()->GetPlayer(i)->SetPlayerName(sNickname);
			return;
		}
	}

	TMsg(sprintf(tstring("Can't find client %d to give nickname %s.\n"), iClient, sNickname.c_str()));
}

void CGameServer::Think(float flHostTime)
{
	TPROF("CGameServer::Think");

	m_iFrame++;
	m_flFrameTime = flHostTime - m_flHostTime;

	// If the framerate drops, don't let too much happen without the player seeing
	if (GameNetwork()->IsConnected())
	{
		// But not as much in multiplayer games where we need to keep the game time synchronized
		if (m_flFrameTime > 1.0f)
			m_flFrameTime = 1.0f;
	}
	else
	{
		if (m_flFrameTime > 0.15f)
			m_flFrameTime = 0.15f;
	}

	m_flGameTime += m_flFrameTime;

	m_flHostTime = flHostTime;

	if (GameNetwork()->IsConnected() && GameNetwork()->IsHost() && m_flHostTime > m_flNextClientInfoUpdate)
	{
		m_flNextClientInfoUpdate = m_flHostTime + 5.0f;

		size_t iClientsConnected = GameNetwork()->GetClientsConnected();
		for (size_t i = 0; i < iClientsConnected; i++)
		{
			size_t iClient = GameNetwork()->GetClientConnectionId(i);
			if (iClient == ~0)
				continue;

			GameNetwork()->CallFunction(iClient, "ClientInfo", iClient, GetGameTime());
		}
	}

	// Erase anything deleted last frame.
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		delete m_ahDeletedEntities[i];

	m_ahDeletedEntities.clear();

	CNetwork::Think();

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t i = 0; i < iMaxEntities; i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		pEntity->Think();

		if (m_bHalting)
			break;
	}

	Simulate();

	Think();

	if (GameNetwork()->IsHost())
		CGameServerNetwork::UpdateNetworkVariables(NETWORK_TOCLIENTS);

	if (GameNetwork()->IsHost())
	{
		for (size_t i = 0; i < iMaxEntities; i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			if (!pEntity->HasIssuedClientSpawn())
				pEntity->IssueClientSpawn();

			if (m_bHalting)
				break;
		}
	}

	CParticleSystemLibrary::Simulate();

	TPortal_Think();
}

void CGameServer::Simulate()
{
	Game()->Simulate();

	if (!Game()->ShouldRunSimulation())
		return;

	GamePhysics()->Simulate();
}

CVar r_cullfrustum("r_frustumculling", "on");

void CGameServer::Render()
{
	TPROF("CGameServer::Render");

	if (!m_pCamera)
		return;

	m_pCamera->Think();

	CRenderer* pRenderer = GameWindow()->GetRenderer();

	pRenderer->SetCameraPosition(m_pCamera->GetCameraPosition());
	pRenderer->SetCameraTarget(m_pCamera->GetCameraTarget());
	pRenderer->SetCameraUp(m_pCamera->GetCameraUp());
	pRenderer->SetCameraFOV(m_pCamera->GetCameraFOV());
	pRenderer->SetCameraNear(m_pCamera->GetCameraNear());
	pRenderer->SetCameraFar(m_pCamera->GetCameraFar());

	pRenderer->SetupFrame();
	pRenderer->StartRendering();

	RenderEverything();

	pRenderer->FinishRendering();
	pRenderer->FinishFrame();
}

void CGameServer::RenderEverything()
{
	CGameRenderer* pRenderer = GetRenderer();

	m_apRenderList.reserve(CBaseEntity::GetNumEntities());
	m_apRenderList.clear();

	bool bFrustumCulling = r_cullfrustum.GetBool();

	// None of these had better get deleted while we're doing this since they're not handles.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (!pEntity->ShouldRender())
			continue;

		if (bFrustumCulling && !pRenderer->IsSphereInFrustum(pEntity->GetGlobalCenter(), (float)pEntity->GetBoundingRadius()))
			continue;

		m_apRenderList.push_back(pEntity);
	}

	pRenderer->BeginBatching();

	// First render all opaque objects
	size_t iEntites = m_apRenderList.size();
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderList[i]->Render(false);

	pRenderer->RenderBatches();

	// Now render all transparent objects. Should really sort this back to front but meh for now.
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderList[i]->Render(true);

	CParticleSystemLibrary::Render();
}

void CGameServer::GenerateSaveCRC(size_t iInput)
{
	mtsrand(m_iSaveCRC^iInput);
	m_iSaveCRC = mtrand();
}

void CGameServer::SaveToFile(const tchar* pFileName)
{
	if (!GameServer())
		return;

	std::ofstream o;
	o.open(convertstring<tchar, char>(pFileName).c_str(), std::ios_base::binary|std::ios_base::out);

	o.write("GameSave", 8);

	CGameServer* pGameServer = GameServer();

	o.write((char*)&pGameServer->m_iSaveCRC, sizeof(pGameServer->m_iSaveCRC));

	o.write((char*)&pGameServer->m_flGameTime, sizeof(pGameServer->m_flGameTime));

	eastl::vector<CBaseEntity*> apSaveEntities;
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		apSaveEntities.push_back(pEntity);
	}

	size_t iEntities = apSaveEntities.size();
	o.write((char*)&iEntities, sizeof(iEntities));

	for (size_t i = 0; i < apSaveEntities.size(); i++)
	{
		CBaseEntity::SerializeEntity(o, apSaveEntities[i]);
	}
}

bool CGameServer::LoadFromFile(const tchar* pFileName)
{
	if (!GameServer())
		return false;

	GameServer()->Initialize();

	// Erase all existing entites. We're going to load in new ones!
	GameServer()->DestroyAllEntities();

	std::ifstream i;
	i.open(convertstring<tchar, char>(pFileName).c_str(), std::ios_base::binary|std::ios_base::in);

	char szTag[8];
	i.read(szTag, 8);
	if (strncmp(szTag, "GameSave", 8) != 0)
		return false;

	CGameServer* pGameServer = GameServer();

	size_t iLoadCRC;
	i.read((char*)&iLoadCRC, sizeof(iLoadCRC));

	if (iLoadCRC != pGameServer->m_iSaveCRC)
		return false;

	i.read((char*)&pGameServer->m_flGameTime, sizeof(pGameServer->m_flGameTime));

	size_t iEntities;
	i.read((char*)&iEntities, sizeof(iEntities));

	for (size_t j = 0; j < iEntities; j++)
	{
		if (!CBaseEntity::UnserializeEntity(i))
			return false;
	}

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		if (!CBaseEntity::GetEntity(i))
			continue;

		CBaseEntity::GetEntity(i)->ClientEnterGame();
	}

	Game()->EnterGame();

	if (GameServer()->GetWorkListener())
		GameServer()->GetWorkListener()->SetAction("Encountering resistance", 0);

	GameServer()->SetLoading(false);

	return true;
}

CEntityHandle<CBaseEntity> CGameServer::Create(const char* pszEntityName)
{
	TAssert(GameNetwork()->IsHost());

	if (!GameNetwork()->ShouldRunClientFunction())
		return CEntityHandle<CBaseEntity>();

	CEntityHandle<CBaseEntity> hEntity(CreateEntity(pszEntityName));

	TAssert(!GameNetwork()->IsConnected());
	// The below causes entities to be created twice on the server. Wasn't a
	// problem with Digitanks, but is a problem now that I'm adding physics.
	// If I ever go back to multiplayer, the code needs to be split into a
	// client portion and a server portion to help minimize bugs like this.
	//::CreateEntity.RunCommand(sprintf(tstring("%s %d %d"), pszEntityName, hEntity->GetHandle(), hEntity->GetSpawnSeed()));

	AddToPrecacheList(pszEntityName);

	return hEntity;
}

size_t CGameServer::CreateEntity(const tstring& sClassName, size_t iHandle, size_t iSpawnSeed)
{
	if (CVar::GetCVarBool("net_debug"))
		TMsg(tstring("Creating entity: ") + sClassName + "\n");

	auto it = CBaseEntity::GetEntityRegistration().find(sClassName);
	if (it == CBaseEntity::GetEntityRegistration().end())
	{
		TAssert(!"Entity does not exist. Did you forget to REGISTER_ENTITY ?");
		return ~0;
	}

	CBaseEntity::s_iOverrideEntityListIndex = iHandle;
	iHandle = it->second.m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);
	hEntity->m_sClassName = sClassName;

	size_t iPostSeed = mtrand();

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(mtrand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->SetSpawnTime(GameServer()->GetGameTime());

	hEntity->Spawn();

	mtsrand(iPostSeed);

	if (dynamic_cast<CGame*>(hEntity.GetPointer()))
		m_hGame = CEntityHandle<CGame>(hEntity->GetHandle());

	return iHandle;
}

void CGameServer::Delete(CBaseEntity* pEntity)
{
	TAssert(GameNetwork()->IsHost() || IsLoading());
	if (!(GameNetwork()->IsHost() || IsLoading()))
		TMsg("WARNING: CGameServer::Delete() when not host or not loading.\n");

	if (GameNetwork()->IsHost())
		GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "DestroyEntity", pEntity->GetHandle());

	CNetworkParameters p;
	p.i1 = (int)pEntity->GetHandle();
	DestroyEntity(CONNECTION_GAME, &p);
}

void CGameServer::DestroyEntity(int iConnection, CNetworkParameters* p)
{
	CBaseEntity* pEntity = CBaseEntity::GetEntity(p->i1);

	if (!pEntity)
		return;

	CSoundLibrary::EntityDeleted(pEntity);

	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (pEntity == m_ahDeletedEntities[i])
			return;

	pEntity->OnDeleted();

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pNotify = CBaseEntity::GetEntity(i);

		if (!pNotify)
			continue;

		pNotify->OnDeleted(pEntity);
	}

	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);
}

void CGameServer::DestroyAllEntities(const eastl::vector<eastl::string>& asSpare, bool bRemakeGame)
{
	if (!GameNetwork()->IsHost() && !IsLoading())
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Locating dead nodes", GameServer()->GetMaxEntities());

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		bool bSpare = false;
		for (size_t j = 0; j < asSpare.size(); j++)
		{
			if (asSpare[j] == pEntity->GetClassName())
			{
				bSpare = true;
				break;
			}
		}

		if (bSpare)
			continue;

		pEntity->Delete();

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Clearing buffers", GameServer()->m_ahDeletedEntities.size());

	for (size_t i = 0; i < GameServer()->m_ahDeletedEntities.size(); i++)
	{
		delete GameServer()->m_ahDeletedEntities[i];

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	GameServer()->m_ahDeletedEntities.clear();

	if (CBaseEntity::GetNumEntities() == 0)
		CBaseEntity::s_iNextEntityListIndex = 0;

	if (bRemakeGame && GameNetwork()->IsHost())
		m_hGame = CreateGame();
}

void CGameServer::UpdateValue(int iConnection, CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (!hEntity)
		return;

	CNetworkedVariableData* pVarData = hEntity->GetNetworkVariable((char*)p->m_pExtraData);

	if (!pVarData)
		return;

	CNetworkedVariableBase* pVariable = pVarData->GetNetworkedVariableBase(hEntity);

	if (!pVariable)
		return;

	void* pDataStart = (unsigned char*)p->m_pExtraData + strlen((char*)p->m_pExtraData)+1;
	pVariable->Unserialize(p->m_iExtraDataSize - (size_t)strlen((char*)p->m_pExtraData) - 1, pDataStart);

	if (pVarData->m_pfnChanged)
		pVarData->m_pfnChanged(pVariable);
}

void CGameServer::ClientInfo(int iConnection, CNetworkParameters* p)
{
	if (m_iClient != p->i1)
		CGame::ClearLocalPlayers(NULL);

	m_iClient = p->i1;
	float flNewGameTime = p->fl2;
	if (flNewGameTime - m_flGameTime > 0.1f)
		TMsg(sprintf(tstring("New game time from server %.1f different!\n"), flNewGameTime - m_flGameTime));

	m_flGameTime = flNewGameTime;

	// Can't send any client commands until we've gotten the client info because we need m_iClient filled out properly.
	if (!m_bGotClientInfo)
	{
		GameNetwork()->SetRunningClientFunctions(false);
		SendNickname.RunCommand(m_sNickname);
	}

	m_bGotClientInfo = true;
}

CGameRenderer* CGameServer::GetRenderer()
{
	return static_cast<CGameRenderer*>(GameWindow()->GetRenderer());
}

CGame* CGameServer::GetGame()
{
	return m_hGame;
}

void ShowStatus(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	TMsg(eastl::string("Level: ") + CVar::GetCVarValue("game_level") + "\n");
	TMsg(convertstring<tchar, char>(sprintf(tstring("Clients: %d Entities: %d/%d\n"), GameNetwork()->GetClientsConnected(), CBaseEntity::GetNumEntities(), GameServer()->GetMaxEntities())));

	for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
	{
		const CPlayer* pPlayer = Game()->GetPlayer(i);
		if (!pPlayer)
			continue;

		if (pPlayer->GetClient() < 0)
			TMsg("Local: ");
		else
			TMsg(convertstring<tchar, char>(sprintf(tstring("%d: "), pPlayer->GetClient())));

		TMsg(pPlayer->GetPlayerName());

		TMsg("\n");
	}
}

CCommand status("status", ::ShowStatus);

void KickPlayer(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	if (!asTokens.size())
		return;

	GameNetwork()->DisconnectClient(stoi(asTokens[0]));
}

CCommand kick("kick", ::KickPlayer);
