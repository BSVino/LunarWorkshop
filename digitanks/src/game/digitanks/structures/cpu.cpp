#include "cpu.h"

#include <GL/glew.h>

#include <mtrand.h>
#include <strutils.h>

#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include <models/models.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/structures/buffer.h>
#include <digitanks/structures/collector.h>
#include <digitanks/structures/loader.h>
#include <digitanks/units/scout.h>

size_t CCPU::s_iCancelIcon = 0;
size_t CCPU::s_iBuildPSUIcon = 0;
size_t CCPU::s_iBuildBufferIcon = 0;
size_t CCPU::s_iBuildLoaderIcon = 0;
size_t CCPU::s_iBuildInfantryLoaderIcon = 0;
size_t CCPU::s_iBuildTankLoaderIcon = 0;
size_t CCPU::s_iBuildArtilleryLoaderIcon = 0;
size_t CCPU::s_iBuildRogueIcon = 0;
size_t CCPU::s_iInstallIcon = 0;
size_t CCPU::s_iInstallPowerIcon = 0;
size_t CCPU::s_iInstallBandwidthIcon = 0;
size_t CCPU::s_iInstallFleetSupplyIcon = 0;

REGISTER_ENTITY(CCPU);

NETVAR_TABLE_BEGIN(CCPU);
	NETVAR_DEFINE_CALLBACK(bool, m_bProducing, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iTurnsToProduceRogue);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCPU);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewBuild);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, unittype_t, m_ePreviewStructure);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bProducing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsToProduceRogue);
	//size_t						m_iFanModel;	// Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFanRotation);
SAVEDATA_TABLE_END();

void CCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/cpu.obj");
	m_iFanModel = CModelLibrary::Get()->FindModel(L"models/structures/cpu-fan.obj");

	m_flFanRotation = RandomFloat(0, 360);

	m_bProducing = false;

	m_bConstructing = false;
}

void CCPU::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/cpu.obj");
	PrecacheModel(L"models/structures/cpu-fan.obj");

	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iBuildPSUIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-psu.png");
	s_iBuildBufferIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-buffer.png");
	s_iBuildLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-loader.png");
	s_iBuildInfantryLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-infantry-loader.png");
	s_iBuildTankLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-tank-loader.png");
	s_iBuildArtilleryLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-artillery-loader.png");
	s_iBuildRogueIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-rogue.png");
	s_iInstallIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install.png");
	s_iInstallPowerIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-power.png");
	s_iInstallBandwidthIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-bandwidth.png");
	s_iInstallFleetSupplyIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-fleet.png");
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	eastl::string16 p;

	bool bDisableMiniBuffer = !GetDigitanksTeam()->CanBuildMiniBuffers();
	bool bDisableBuffer = !GetDigitanksTeam()->CanBuildBuffers();
	bool bDisableBattery = !GetDigitanksTeam()->CanBuildBatteries();
	bool bDisablePSU = !GetDigitanksTeam()->CanBuildPSUs();
	bool bDisableLoaders = !GetDigitanksTeam()->CanBuildLoaders();

	if (!bDisableLoaders && eMenuMode == MENUMODE_LOADERS)
	{
		if (GetDigitanksTeam()->CanBuildInfantryLoaders())
		{
			pHUD->SetButtonTexture(0, s_iBuildInfantryLoaderIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER))
			{
				pHUD->SetButtonListener(0, CHUD::BuildInfantryLoader);
				pHUD->SetButtonColor(0, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD RESISTOR FACTORY\n \n";
			s += L"This program lets you build Resistor, the main defensive force of your fleet. After fortifying them they gain energy bonuses.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
			pHUD->SetButtonTooltip(0, L"Build Resistor Factory");
		}

		if (GetDigitanksTeam()->CanBuildTankLoaders())
		{
			pHUD->SetButtonTexture(1, s_iBuildTankLoaderIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER))
			{
				pHUD->SetButtonListener(1, CHUD::BuildTankLoader);
				pHUD->SetButtonColor(1, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD DIGITANK FACTORY\n \n";
			s += L"This program lets you build Digitanks, the primary assault force in your fleet.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Build Digitank Factory");
		}

		if (GetDigitanksTeam()->CanBuildArtilleryLoaders())
		{
			pHUD->SetButtonTexture(2, s_iBuildArtilleryLoaderIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER))
			{
				pHUD->SetButtonListener(2, CHUD::BuildArtilleryLoader);
				pHUD->SetButtonColor(2, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD ARTILLERY FACTORY\n \n";
			s += L"This program lets you build Artillery. Once deployed, these units have extreme range and can easily soften enemy defensive positions.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: E";
			pHUD->SetButtonInfo(2, s);
			pHUD->SetButtonTooltip(2, L"Build Artillery Factory");
		}

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
		pHUD->SetButtonTooltip(9, L"Return");
	}
	else
	{
		if (!bDisableMiniBuffer)
		{
			pHUD->SetButtonTexture(5, s_iBuildBufferIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER))
			{
				pHUD->SetButtonListener(5, CHUD::BuildMiniBuffer);
				pHUD->SetButtonColor(5, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD BUFFER\n \n";
			s += L"Buffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. Buffers can later be upgraded to Macro-Buffers.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: A";
			pHUD->SetButtonInfo(5, s);
			pHUD->SetButtonTooltip(5, L"Build Buffer");
		}

		if (!bDisableBuffer)
		{
			pHUD->SetButtonTexture(0, s_iBuildBufferIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER))
			{
				pHUD->SetButtonListener(0, CHUD::BuildBuffer);
				pHUD->SetButtonColor(0, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD MACRO-BUFFER\n \n";
			s += L"Macro-Buffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. Macro-Buffers can be improved by downloading updates.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
			pHUD->SetButtonTooltip(0, L"Build Macro-Buffer");
		}

		if (!bDisableBattery)
		{
			pHUD->SetButtonTexture(6, s_iBuildPSUIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY))
			{
				pHUD->SetButtonListener(6, CHUD::BuildBattery);
				pHUD->SetButtonColor(6, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD BATTERY\n \n";
			s += L"Batteries allow you to harvest Power, which lets you build structures and units more quickly. Batteries can upgraded to Power Supply Units once those have been downloaded from the Updates Grid.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: S";
			pHUD->SetButtonInfo(6, s);
			pHUD->SetButtonTooltip(6, L"Build Battery");
		}

		if (!bDisablePSU)
		{
			pHUD->SetButtonTexture(1, s_iBuildPSUIcon);

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_PSU))
			{
				pHUD->SetButtonListener(1, CHUD::BuildPSU);
				pHUD->SetButtonColor(1, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD POWER SUPPLY UNIT\n \n";
			s += L"PSUs allow you to harvest Power, which lets you build structures and units more quickly.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_PSU));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_PSU))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Build PSU");
		}

		if (!bDisableLoaders)
		{
			pHUD->SetButtonListener(2, CHUD::BuildLoader);
			pHUD->SetButtonTexture(2, s_iBuildLoaderIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));
			pHUD->SetButtonInfo(2, L"OPEN FACTORY CONSTRUCTION MENU\n \nFactories allow you to produce more advanced units.\n \nShortcut: E");
			pHUD->SetButtonTooltip(2, L"Open Factory Menu");
		}

		pHUD->SetButtonTexture(7, s_iBuildRogueIcon);
		if (GetDigitanksTeam()->GetUnusedFleetPoints() && GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(UNIT_SCOUT) && !IsProducing())
		{
			pHUD->SetButtonColor(7, Color(150, 150, 150));
			pHUD->SetButtonListener(7, CHUD::BuildScout);
		}

		eastl::string16 s;
		s += L"BUILD ROGUE\n \n";
		s += L"Rogues are a cheap reconnaisance unit with good speed but no shields. Their torpedo attack allows you to intercept enemy supply lines. Use them to find and slip behind enemy positions and harrass their support!\n \n";
		s += p.sprintf(L"Power to construct: %d Power\n", (int)DigitanksGame()->GetConstructionCost(UNIT_SCOUT));

		if (!GetDigitanksTeam()->GetUnusedFleetPoints())
			s += L"NOT ENOUGH FLEET POINTS\n \n";

		if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(UNIT_SCOUT))
			s += L"NOT ENOUGH POWER\n \n";

		s += L"Shortcut: D";
		pHUD->SetButtonInfo(7, s);
		pHUD->SetButtonTooltip(7, L"Build Rogue");
	}
}

bool CCPU::NeedsOrders()
{
	bool bDisableMiniBuffer = !GetDigitanksTeam()->CanBuildMiniBuffers();
	bool bDisableBuffer = !GetDigitanksTeam()->CanBuildBuffers();
	bool bDisableBattery = !GetDigitanksTeam()->CanBuildBatteries();
	bool bDisablePSU = !GetDigitanksTeam()->CanBuildPSUs();
	bool bDisableLoaders = !GetDigitanksTeam()->CanBuildLoaders();

	if (bDisableMiniBuffer && bDisableBuffer && bDisableBattery && bDisablePSU && bDisableLoaders)
		return BaseClass::NeedsOrders();

	return true;
}

bool CCPU::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

bool CCPU::IsPreviewBuildValid() const
{
	CSupplier* pSupplier = FindClosestSupplier(GetPreviewBuild(), GetTeam());

	if (m_ePreviewStructure == STRUCTURE_PSU || m_ePreviewStructure == STRUCTURE_BATTERY)
	{
		CResource* pResource = CResource::FindClosestResource(GetPreviewBuild(), RESOURCE_ELECTRONODE);
		float flDistance = (pResource->GetOrigin() - GetPreviewBuild()).Length();
		if (flDistance > 8)
			return false;

		if (pResource->HasCollector())
		{
			CCollector* pCollector = pResource->GetCollector();

			if (pCollector->GetTeam() != GetTeam())
				return false;

			if (m_ePreviewStructure == STRUCTURE_BATTERY)
				return false;

			if (m_ePreviewStructure == STRUCTURE_PSU && pCollector->GetUnitType() == STRUCTURE_PSU)
				return false;

			// If we're building a PSU and the structure is a battery, that's cool.
		}
	}
	else
	{
		// Don't allow construction too close to other structures.
		CStructure* pClosestStructure = CBaseEntity::FindClosest<CStructure>(GetPreviewBuild());
		if ((pClosestStructure->GetOrigin() - GetPreviewBuild()).Length() < pClosestStructure->GetBoundingRadius()+5)
			return false;

		if (!pSupplier)
			return false;
	}

	if (!DigitanksGame()->GetTerrain()->IsPointOnMap(GetPreviewBuild()))
		return false;

	if (DigitanksGame()->GetTerrain()->IsPointOverHole(GetPreviewBuild()))
		return false;

	return CSupplier::GetDataFlow(GetPreviewBuild(), GetTeam()) > 0;
}

void CCPU::SetPreviewBuild(Vector vecPreviewBuild)
{
	m_vecPreviewBuild = vecPreviewBuild;

	if (GetPreviewStructure() == STRUCTURE_BATTERY || GetPreviewStructure() == STRUCTURE_PSU)
	{
		CResource* pResource = CResource::FindClosestResource(vecPreviewBuild, RESOURCE_ELECTRONODE);

		if ((pResource->GetOrigin() - vecPreviewBuild).Length() <= 8)
			m_vecPreviewBuild = pResource->GetOrigin();
	}
}

void CCPU::ClearPreviewBuild()
{
	m_vecPreviewBuild = GetOrigin();
}

bool CCPU::BeginConstruction()
{
	if (!IsPreviewBuildValid())
		return false;

	if (GetPowerToConstruct(m_ePreviewStructure, GetPreviewBuild()) > GetDigitanksTeam()->GetPower())
		return false;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = m_ePreviewStructure;
	p.fl3 = GetPreviewBuild().x;
	p.fl4 = GetPreviewBuild().y;
	p.fl5 = GetPreviewBuild().z;

	if (CNetwork::IsHost())
		BeginConstruction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "BeginConstruction", &p);

	// This is used for the bot to see if a build was successful.
	if (CNetwork::IsHost())
		return p.ui1 != ~0;
	else
		return true;
}

void CCPU::BeginConstruction(CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	// Overload this so that things this function calls get replicated.
	CNetwork::SetRunningClientFunctions(false);

	unittype_t ePreviewStructure = (unittype_t)p->i2;
	Vector vecPreview(p->fl3, p->fl4, p->fl5);

	if (GetPowerToConstruct(ePreviewStructure, vecPreview) > GetDigitanksTeam()->GetPower())
		return;

	if (ePreviewStructure == STRUCTURE_PSU)
	{
		CResource* pResource = CResource::FindClosestResource(vecPreview, RESOURCE_ELECTRONODE);
		if (pResource->HasCollector())
		{
			CCollector* pCollector = pResource->GetCollector();
			if (pCollector->GetTeam() == GetTeam() && pCollector->GetUnitType() == STRUCTURE_BATTERY)
			{
				pCollector->BeginUpgrade();
				return;
			}
		}
	}

	CStructure* pConstructing = NULL;

	if (ePreviewStructure == STRUCTURE_MINIBUFFER)
	{
		pConstructing = GameServer()->Create<CMiniBuffer>("CMiniBuffer");
	}
	else if (ePreviewStructure == STRUCTURE_BUFFER)
	{
		if (!GetDigitanksTeam()->CanBuildBuffers())
			return;

		pConstructing = GameServer()->Create<CBuffer>("CBuffer");
	}
	else if (ePreviewStructure == STRUCTURE_BATTERY)
	{
		pConstructing = GameServer()->Create<CBattery>("CBattery");
	}
	else if (ePreviewStructure == STRUCTURE_PSU)
	{
		if (!GetDigitanksTeam()->CanBuildPSUs())
			return;

		pConstructing = GameServer()->Create<CCollector>("CCollector");
	}
	else if (ePreviewStructure == STRUCTURE_INFANTRYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildInfantryLoaders())
			return;

		pConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(pConstructing);
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_INFANTRY);
	}
	else if (ePreviewStructure == STRUCTURE_TANKLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildTankLoaders())
			return;

		pConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(pConstructing);
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_TANK);
	}
	else if (ePreviewStructure == STRUCTURE_ARTILLERYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildArtilleryLoaders())
			return;

		pConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(pConstructing);
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_ARTILLERY);
	}

	if (CNetwork::IsHost())
		p->ui1 = pConstructing->GetHandle();

	GetDigitanksTeam()->ConsumePower(GetPowerToConstruct(ePreviewStructure, vecPreview));

	pConstructing->BeginConstruction(vecPreview);
	GetTeam()->AddEntity(pConstructing);
	pConstructing->SetSupplier(FindClosestSupplier(vecPreview, GetTeam()));
	pConstructing->GetSupplier()->AddChild(pConstructing);

	pConstructing->SetOrigin(vecPreview);
	if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
	{
		Vector vecPSU = vecPreview;

		CResource* pResource = CBaseEntity::FindClosest<CResource>(vecPSU);

		if (pResource)
		{
			if ((pResource->GetOrigin() - vecPSU).Length() <= 8)
				pConstructing->SetOrigin(pResource->GetOrigin());
		}
	}

	CSupplier* pSupplier = dynamic_cast<CSupplier*>(pConstructing);
	if (pSupplier)
		pSupplier->GiveDataStrength((size_t)pSupplier->GetSupplier()->GetDataFlow(pSupplier->GetOrigin()));

	CCollector* pCollector = dynamic_cast<CCollector*>(pConstructing);
	if (pCollector)
	{
		pCollector->SetResource(CResource::FindClosestResource(vecPreview, pCollector->GetResourceType()));
		pCollector->GetResource()->SetCollector(pCollector);
	}

	GetDigitanksTeam()->CountProducers();

	size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();

	if (ePreviewStructure == STRUCTURE_BUFFER && iTutorial == CInstructor::TUTORIAL_BUFFER)
	{
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_BUFFER);
		DigitanksWindow()->GetInstructor()->NextTutorial();
	}

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_PLACEBUFFER, true);

	pConstructing->FindGround();

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
				pConstructing->InstallUpdate(x, y);
		}
	}

	DigitanksGame()->GetTerrain()->CalculateVisibility();

	if (DigitanksGame()->GetTurn() == 0 && GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
	{
		// Let's see our action items.
		DigitanksGame()->AllowActionItems(true);
		DigitanksGame()->GetActionItems().clear();
		DigitanksGame()->AddActionItem(NULL, ACTIONTYPE_WELCOME);
		DigitanksGame()->AddActionItem(NULL, ACTIONTYPE_CONTROLS);
		DigitanksGame()->AddActionItem(NULL, ACTIONTYPE_DOWNLOADUPDATES);

		for (size_t i = 0; i < GetDigitanksTeam()->GetNumTanks(); i++)
			DigitanksGame()->AddActionItem(GetDigitanksTeam()->GetTank(i), ACTIONTYPE_UNITORDERS);

		DigitanksGame()->AllowActionItems(false);
		DigitanksWindow()->GetHUD()->ShowActionItem(ACTIONTYPE_WELCOME);

		CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
		pInstructor->SetActive(false);
	}

	DigitanksWindow()->GetHUD()->Layout();
}

float CCPU::GetPowerToConstruct(unittype_t eStructure, Vector vecLocation)
{
	float flPowerToConstruct = DigitanksGame()->GetConstructionCost(eStructure);

	return flPowerToConstruct;
}

void CCPU::BeginRogueProduction()
{
	if (IsProducing())
		return;

	if (DigitanksGame()->GetConstructionCost(UNIT_SCOUT) > GetDigitanksTeam()->GetPower())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		BeginRogueProduction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "BeginRogueProduction", &p);
}

void CCPU::BeginRogueProduction(class CNetworkParameters* p)
{
	if (IsProducing())
		return;

	if (DigitanksGame()->GetConstructionCost(UNIT_SCOUT) > GetDigitanksTeam()->GetPower())
		return;

	if (!GetDigitanksTeam()->GetUnusedFleetPoints())
		return;

	m_iTurnsToProduceRogue = 1;
	m_bProducing = true;
	GetDigitanksTeam()->ConsumePower(DigitanksGame()->GetConstructionCost(UNIT_SCOUT));

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CCPU::StartTurn()
{
	BaseClass::StartTurn();

	if (m_bProducing)
	{
		m_iTurnsToProduceRogue -= (size_t)1;
		if (m_iTurnsToProduceRogue == (size_t)0)
		{
			if (CNetwork::IsHost())
			{
				CDigitank* pTank = GameServer()->Create<CScout>("CScout");
				pTank->SetOrigin(GetOrigin());
				GetTeam()->AddEntity(pTank);

				for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
				{
					for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
					{
						if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
							pTank->DownloadComplete(x, y);
					}
				}

				// Face him toward the center.
				pTank->Move(pTank->GetOrigin() + -GetOrigin().Normalized()*15);
				pTank->Turn(VectorAngles(-GetOrigin().Normalized()));

				pTank->StartTurn();
			}

			m_bProducing = false;

			DigitanksGame()->AppendTurnInfo(L"Production finished on Rogue");

			DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITREADY);
		}
		else
		{
			DigitanksGame()->AppendTurnInfo(sprintf(L"Producing Rogue (%d turns left)", m_iTurnsToProduceRogue.Get()));
		}
	}
}

void CCPU::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::OnRender(pContext, bTransparent);

	if (m_iFanModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	CRenderingContext r(GameServer()->GetRenderer());
	if (GetTeam())
		r.SetColorSwap(GetTeam()->GetColor());
	else
		r.SetColorSwap(Color(255, 255, 255, 255));

	float flVisibility = GetVisibility();

	if (flVisibility < 1 && !bTransparent)
		return;

	if (flVisibility == 1 && bTransparent)
		return;

	if (bTransparent)
	{
		r.SetAlpha(GetVisibility());
		if (r.GetAlpha() < 1)
			r.SetBlend(BLEND_ALPHA);
	}

	m_flFanRotation -= 100 * GameServer()->GetFrameTime();
	r.Rotate(m_flFanRotation, Vector(0, 1, 0));

	r.RenderModel(m_iFanModel);
}

void CCPU::RenderBuildableArea()
{
	BaseClass::RenderBuildableArea();

	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return;

	if (m_ePreviewStructure != STRUCTURE_BUFFER && m_ePreviewStructure != STRUCTURE_MINIBUFFER)
		return;

	if (!IsPreviewBuildValid())
		return;

	size_t iInitialDataStrength;
	if (m_ePreviewStructure == STRUCTURE_BUFFER)
		iInitialDataStrength = CBuffer::InitialBufferDataStrength();
	else
		iInitialDataStrength = CMiniBuffer::InitialMiniBufferDataStrength();

	float flRadius = sqrt((float)iInitialDataStrength/M_PI) + 5;	// 5 is the bounding radius for both structures

	CRenderingContext c(GameServer()->GetRenderer());
	c.Translate(GetPreviewBuild());
	c.Scale(flRadius, flRadius, flRadius);
	c.RenderSphere();
}

void CCPU::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";
	s += L"CENTRAL PROCESSING UNIT\n";
	s += L"Command center\n \n";

	if (IsProducing())
	{
		s += L"[Producing Rogue]\n";
		s += sprintf(L"Turns left: %d\n \n", m_iTurnsToProduceRogue.Get());
	}

	s += p.sprintf(L"Power Supplied: %.1f\n", Power());
	s += p.sprintf(L"Fleet Points: %d\n", FleetPoints());
	s += p.sprintf(L"Bandwidth: %.1f\n", Bandwidth());
	s += p.sprintf(L"Network Size: %d\n", (int)GetDataFlowRadius());
	s += p.sprintf(L"Efficiency: %d\n", (int)(GetChildEfficiency()*100));
}

void CCPU::OnDeleted()
{
	eastl::vector<CBaseEntity*> apDeleteThese;

	for (size_t i = 0; i < GetTeam()->GetNumMembers(); i++)
	{
		CBaseEntity* pMember = GetTeam()->GetMember(i);
		if (pMember == this)
			continue;

		apDeleteThese.push_back(pMember);
	}

	if (GameServer()->IsLoading())
		return;

	for (size_t i = 0; i < apDeleteThese.size(); i++)
	{
		CBaseEntity* pMember = apDeleteThese[i];

		if (!pMember)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pMember);
		// Delete? I meant... repurpose.
		if (pStructure && !pStructure->IsConstructing())
		{
			GetTeam()->RemoveEntity(pStructure);
		}
		else
		{
			bool bColorSwap = (pStructure || dynamic_cast<CDigitank*>(pMember));
			CModelDissolver::AddModel(pMember, bColorSwap?&GetTeam()->GetColor():NULL);
			pMember->Delete();
		}
	}
}
