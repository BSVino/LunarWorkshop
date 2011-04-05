#include "instructor.h"

#include <glgui/glgui.h>
#include <tinker/cvar.h>
#include <datamanager/data.h>

#include "digitanks/units/digitank.h"
#include "digitanks/units/mobilecpu.h"
#include "digitanks/digitanksgame.h"
#include "digitanks/dt_camera.h"
#include "digitankswindow.h"
#include "ui.h"
#include "renderer/renderer.h"
#include "sound/sound.h"
#include "hud.h"

using namespace glgui;

CInstructor::CInstructor()
	: m_EmotionsSheet(L"textures/hud/helper-emotions.txt")
{
	m_bActive = true;
	m_pCurrentPanel = NULL;
	m_sLastTutorial = "";
	Initialize();
	m_eDisabled = DISABLE_NOTHING;

	CSoundLibrary::Get()->AddSound(L"sound/lesson-learned.wav");
}

CInstructor::~CInstructor()
{
	HideTutorial();
	Clear();
}

void CInstructor::Clear()
{
	for (eastl::map<eastl::string, CTutorial*>::iterator it = m_apTutorials.begin(); it != m_apTutorials.end(); it++)
		delete m_apTutorials[it->first];
	m_apTutorials.clear();
}

void CInstructor::Initialize()
{
	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = NULL;

	Clear();

/*
	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_AIM, new CTutorial(this, TUTORIAL_AIM, POSITION_TOPLEFT, 200, true,
		L"CHOOSE A WEAPON\n \nYou're closing in for the kill! Now press the red 'Choose Weapon' button to select your means of destruction. The 'Choose Weapon' button is located in the command dock on the bottom of the screen.\n \nPress one of the options to choose that weapon.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_ATTACK, new CTutorial(this, TUTORIAL_ATTACK, POSITION_TOPLEFT, 300, true,
		L"ATTACK!!!\n \nYou're ready to attack!\n \nYour tank has very good accuracy inside the green area, but outside that your accuracy will decrease. Your tank's maximum range is the red circle.\n \nClick on the enemy tank to fire at him.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_BUTTONS, new CTutorial(this, TUTORIAL_BUTTONS, POSITION_TOPLEFT, 250, true,
		L"CONTROL BUTTONS\n \nGood job, you messed that guy up pretty good! Don't forget you can enter move mode and aim mode at any time by using the buttons in the command dock on the bottom of the screen.\n \nYou can exit move turn or aim mode by pressing the 'Escape' key.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_ENERGY, new CTutorial(this, TUTORIAL_ENERGY, POSITION_TOPLEFT, 300, true,
		L"MIND YOUR ENERGY\n \nEach weapon in your arsenal requires a different amount of energy. The more energy the weapon requires, the less will be available for your shields. Be mindful when you fire more powerful weapons, as you are leaving yourself vulnerable.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_ENTERKEY, new CTutorial(this, TUTORIAL_ENTERKEY, POSITION_TOPCENTER, 200, false,
		L"END YOUR TURN\n \nThat's it! Now press the 'Enter' key to end your turn.\n \nAlternatively, you can depress the large friendly green 'END TURN' button on the bottom right.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_FINISHHIM, new CTutorial(this, TUTORIAL_FINISHHIM, POSITION_TOPLEFT, 200, false,
		L"FINISH THE JOB\n \nYou messed that guy up pretty good! Now let's finish the job. Aim your tank at the enemy again and click on him to fire. If you have trouble hitting the tank, try rotating your camera for a better view.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_UPGRADE, new CTutorial(this, TUTORIAL_UPGRADE, POSITION_TOPCENTER, 250, true,
		L"UPGRADE YOUR TANK\n \nYou destroyed the enemy tank, and received an upgrade! With this you can upgrade your tank's Energy. Press the blinking 'Upgrade' button with the star icon on the bottom right to show a list of upgrade options, and then select an upgrade.\n \nKilling enemy tanks can grant you additional upgrades.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_POWERUP, new CTutorial(this, TUTORIAL_POWERUP, POSITION_TOPLEFT, 250, true,
		L"GET THE POWERUP\n \nLook at that! A powerup has appeared next to your tank. Press the 'Move' button and move your tank on top of the powerup to retrieve it. If you can't move far enough, you may need to end your turn to get your power back.\n \nPicking up more powerups can grant you additional upgrades, just like killing enemy tanks.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_SHIFTSELECT, new CTutorial(this, TUTORIAL_SHIFTSELECT, POSITION_TOPLEFT, 250, true,
		L"SHIFT SELECTION\n \nGood news! Reinforcements have arrived. You are now in command of three friendly tanks.\n \nManaging multiple tanks can be tricky, but you can command them all at the same time by selecting multiple tanks. Hold the 'Shift' key while clicking the tanks to select all three tanks at once.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_BOXSELECT, new CTutorial(this, TUTORIAL_BOXSELECT, POSITION_TOPLEFT, 250, true,
		L"BOX SELECTION\n \nGood work. You can also use box selection to select all three tanks with one click. Use the left mouse button to drag a selection box around all of the tanks so you can issue orders to all three at once.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_THEEND_BASICS, new CTutorial(this, TUTORIAL_THEEND_BASICS, POSITION_TOPCENTER, 250, false,
		L"END OF TUTORIAL\n \nThat's it! Now is a good time to move on to the Bases tutorial. You can also start a new game by opening the menu with the 'Escape' key. Enjoy Digitanks!")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_INTRO_BASES, new CTutorial(this, TUTORIAL_INTRO_BASES, POSITION_TOPCENTER, 250, true,
		L"Welcome to Digitanks!\n \nThis tutorial will help you learn how to build bases. It is recommended to finish the basics tutorial first.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_CPU, new CTutorial(this, TUTORIAL_CPU, POSITION_TOPLEFT, 250, true,
		L"THE CENTRAL PROCESSING UNIT (CPU)\n \nThis is your CPU. It is your command center. If it is destroyed then you lose the game, so protect it well.\n \nThe CPU is the source of your Network. Your Network is represented by the glowy tendrils that come from your CPU.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_BUFFER, new CTutorial(this, TUTORIAL_BUFFER, POSITION_TOPLEFT, 250, false,
		L"CONSTRUCTING\n \nYou can use your CPU to construct other structures. A 'Buffer' is a special structure that extends your Network and buffs your units. Construct a Buffer by clicking the 'Build Buffer' button and then clicking inside the green highlighted area.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_POWER, new CTutorial(this, TUTORIAL_POWER, POSITION_TOPLEFT, 250, false,
		L"POWER\n \nYour Buffer is now constructing. Structures take Power to build. Power accumulates every turn, you can see how much power you have at the top right of the screen. Once the structure is placed it takes a certain number of turns to complete. For this tutorial, the build time for your Buffer has been shortened.\n \nPress 'Enter' to complete construction.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_NETWORK, new CTutorial(this, TUTORIAL_NETWORK, POSITION_TOPCENTER, 250, true,
		L"THE NETWORK\n \nBuffers and CPUs extend your Network, marking your terrain. You can only build more structures inside your Network. Friendly units inside your Network will receive combat bonuses.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_PSU, new CTutorial(this, TUTORIAL_PSU, POSITION_TOPLEFT, 250, false,
		L"POWER SUPPLIES\n \nElectronodes are digital resources that contain Power. Batteries and Power Supplies can extract Power from Electronodes. They must be built directly on top of an Electronode, and only one per Electronode can be built. There is an Electronode next to your CPU.\n \nClick the CPU to select it. Then press the 'Build Power Supply' button and click on the Electronode to build a Power Supply. Press the 'Enter' key to complete construction.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_SUPPLY, new CTutorial(this, TUTORIAL_SUPPLY, POSITION_TOPCENTER, 250, true,
		L"SUPPLY LINES\n \nAll units and structures maintain supply lines to the nearest Buffer or CPU. These lines provide support, such as combat bonuses and health regeneration. They can be broken by moving an enemy unit on them, so be sure to protect them.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_LOADER, new CTutorial(this, TUTORIAL_LOADER, POSITION_TOPLEFT, 250, false,
		L"FACTORIES\n \nFactories are specialized structures that produce combat units. With the CPU selected, build a Factory by pressing the 'Build Factory' button, selecting any factory, and clicking in the green area to build it. Then press the 'Enter' key to complete construction.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_EFFICIENCY, new CTutorial(this, TUTORIAL_EFFICIENCY, POSITION_TOPCENTER, 250, true,
		L"EFFICIENCY\n \nBuilding more than two structures off any Buffer will cause the structures to become inefficient. It's best to spread out your structures so they use many different Buffers. However, the Buffers themselves aren't affected, you can have as many buffers as you want.\n \nThe CPU doesn't have this restriction.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_PRODUCING_UNITS, new CTutorial(this, TUTORIAL_PRODUCING_UNITS, POSITION_TOPLEFT, 250, false,
		L"PRODUCING UNITS\n \nNow that your Factory has finished constructing, it can start producing units. Select the Factory and click the build button to begin producing a unit.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_FLEET_POINTS, new CTutorial(this, TUTORIAL_FLEET_POINTS, POSITION_TOPCENTER, 250, true,
		L"FLEET POINTS\n \nYour fleet points can be seen on the upper right. When tanks are produced, they use up these fleet points. To get more fleet points, just build more buffers.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_THEEND_BASES, new CTutorial(this, TUTORIAL_THEEND_BASES, POSITION_TOPCENTER, 250, false,
		L"END OF TUTORIAL\n \nThat's it! Now is a good time to move on to the Units tutorial. You can return to the main menu by pressing the 'Escape' key.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_INTRO_UNITS, new CTutorial(this, TUTORIAL_INTRO_UNITS, POSITION_TOPCENTER, 250, true,
		L"Welcome to Digitanks!\n \nThis tutorial will help you learn the individual units. It is recommended to finish the basics tutorial first.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_INFANTRY, new CTutorial(this, TUTORIAL_INFANTRY, POSITION_TOPCENTER, 250, true,
		L"RESISTOR\n \nThis unit is a Resistor. They are a mobile support and defense platform.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_FORTIFYING, new CTutorial(this, TUTORIAL_FORTIFYING, POSITION_TOPCENTER, 250, true,
		L"FORTIFYING\n \nResistors can function just fine like normal tanks but their real strength is in their fortification ability. Fortifying increases your tank's attack and shield energy a great deal.\n \nSelect the unit and press the 'Fortify Unit' button to fortify this infantry.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_FORTIFYING2, new CTutorial(this, TUTORIAL_FORTIFYING2, POSITION_TOPCENTER, 250, true,
		L"FORTIFYING\n \nGreat! That energy wall is incredibly strong and regenerates faster when the unit is fortified. Fortified units can't rotate, but if you position them well they won't need to. Resistors also get bonuses to their attack when they're fortified.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_ARTILLERY, new CTutorial(this, TUTORIAL_ARTILLERY, POSITION_TOPCENTER, 250, true,
		L"ARTILLERY\n \nArtillery are a long-range support unit. They are best used to soften up enemy positions before moving in with your tanks. However, they must be deployed before you can use them.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_DEPLOYING, new CTutorial(this, TUTORIAL_DEPLOYING, POSITION_TOPCENTER, 250, false,
		L"DEPLOYING\n \nOnce deployed your artillery can't move, and turning is more expensive, so consider your deploy spot carefully.\n \nWhen you are ready, select the unit and then press the 'Deploy Unit' button.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_DEPLOYING2, new CTutorial(this, TUTORIAL_DEPLOYING2, POSITION_TOPCENTER, 250, false,
		L"DEPLOYING\n \nGood work. Your artillery is deployed, but it won't be ready to use until next turn. Press the 'END TURN' button on the bottom right to proceed.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_FIRE_ARTILLERY, new CTutorial(this, TUTORIAL_FIRE_ARTILLERY, POSITION_TOPLEFT, 250, false,
		L"ATTACK!\n \nNow that your artillery is ready to fire, you'll notice it has an incredible range. It has some drawbacks though, the cone of fire is limited. Also, artillery can't see into the fog of war and will need spotters. You have been given a spotter to help you see your target.\n \nPress the 'Fire' button and click on the red tank to fire.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_ARTILLERY_SHIELDS, new CTutorial(this, TUTORIAL_ARTILLERY_SHIELDS, POSITION_TOPLEFT, 250, true,
		L"SHELLING SHIELDS\n \nArtillery are great for shelling enemy tanks and Resistor positions, but while their EMP shells are extremely effective against shields, they are rather weak against tank hulls and structures. After the enemy shields are down it's time to move in with your tanks.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_ROGUE, new CTutorial(this, TUTORIAL_ROGUE, POSITION_TOPCENTER, 250, true,
		L"THE ROGUE\n \nThe Rogue is a light reconnaissance unit. It lacks shields and can only damage enemies if their shields are down. However, its torpedo attack can be a dangerous threat to enemies.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_TORPEDO, new CTutorial(this, TUTORIAL_TORPEDO, POSITION_TOPCENTER, 250, true,
		L"FIRE TORPEDO NUMBER ONE\n \nThe Rogue's torpedos attack supply lines. Hey look, there's an enemy supply line now! Select the Rogue and press the 'Fire' button. Then click on the supply line to attack it.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_DISCONNECTING_SUPPLIES, new CTutorial(this, TUTORIAL_DISCONNECTING_SUPPLIES, POSITION_TOPCENTER, 250, true,
		L"DISCONNECTING SUPPLY LINES\n \nYou can force an enemy structure to become neutral by destroying its supply line and disconnecting it from the enemy base. Any neutral structure can then be taken over if you build a buffer nearby.\n \nRogue torpedoes can also disable enemy units, preventing them from taking any action for one turn.\n \nClick here to continue.")));

	m_apTutorials.insert(eastl::pair<size_t, CTutorial*>(TUTORIAL_THEEND_UNITS, new CTutorial(this, TUTORIAL_THEEND_UNITS, POSITION_TOPCENTER, 250, false,
		L"END OF TUTORIAL\n \nThat's it! You can start a new game by opening the menu with the 'Escape' key. Enjoy Digitanks!")));*/

	m_apTutorials["artillery-select"] = new CTutorial(this, "artillery-select", "artillery-aim", POSITION_SCENETREE, 150, false, L"< Select a unit");
	m_apTutorials["artillery-select"]->m_flSlideAmount = 1000;
	m_apTutorials["artillery-select"]->m_bSlideX = true;

	m_apTutorials["artillery-aim"] = new CTutorial(this, "artillery-aim", "artillery-chooseweapon", POSITION_BUTTONS, 200, false, L"Press the 'Choose Weapon' button");
	m_apTutorials["artillery-aim"]->m_flSlideAmount = 1000;
	m_apTutorials["artillery-aim"]->m_bSlideX = false;

	m_apTutorials["artillery-chooseweapon"] = new CTutorial(this, "artillery-chooseweapon", "artillery-command", POSITION_TOPCENTER, 200, false, L"Choices!\n \nChoose your weapon.");

	m_apTutorials["artillery-command"] = new CTutorial(this, "artillery-chooseweapon", "", POSITION_TOPCENTER, 200, false, L"Click on an enemy to fire");
	m_apTutorials["artillery-command"]->m_flSlideAmount = 200;
	m_apTutorials["artillery-command"]->m_bSlideX = false;

	m_apTutorials["strategy-select"] = new CTutorial(this, "strategy-select", "strategy-command", POSITION_SCENETREE, 150, false, L"< Select the MCP");
	m_apTutorials["strategy-select"]->m_flSlideAmount = 1000;
	m_apTutorials["strategy-select"]->m_bSlideX = true;

	m_apTutorials["strategy-command"] = new CTutorial(this, "strategy-command", "strategy-deploy", POSITION_TOPCENTER, 200, false, L"Click in the yellow area to move the MCP\n \nTry to choose a location with nearby electronodes, such as this one");
	m_apTutorials["strategy-command"]->m_flSlideAmount = 200;
	m_apTutorials["strategy-command"]->m_bSlideX = false;

	m_apTutorials["strategy-deploy"] = new CTutorial(this, "strategy-deploy", "strategy-buildbuffer", POSITION_BUTTONS, 200, false, L"Press the 'Deploy' button to create a CPU");
	m_apTutorials["strategy-deploy"]->m_flSlideAmount = 1000;
	m_apTutorials["strategy-deploy"]->m_bSlideX = false;

	m_apTutorials["strategy-buildbuffer"] = new CTutorial(this, "strategy-buildbuffer", "strategy-placebuffer", POSITION_BUTTONS, 200, false, L"Choose 'Build Buffer' from the construction options");
	m_apTutorials["strategy-buildbuffer"]->m_flSlideAmount = 1000;
	m_apTutorials["strategy-buildbuffer"]->m_bSlideX = false;

	m_apTutorials["strategy-placebuffer"] = new CTutorial(this, "strategy-placebuffer", "", POSITION_TOPCENTER, 200, false, L"Click inside the green area to place the structure");
	m_apTutorials["strategy-placebuffer"]->m_flSlideAmount = 200;
	m_apTutorials["strategy-placebuffer"]->m_bSlideX = false;
}

void CInstructor::ReadLesson(const class CData* pData)
{
	eastl::string sLessonName = pData->GetValueString();
	int iPosition = POSITION_TOPCENTER;
	int iWidth = 200;
	eastl::string sNext;
	eastl::string sText;
	eastl::string sButton1Action;
	eastl::string sButton1Text;
	eastl::string sButton2Action;
	eastl::string sButton2Text;
	disable_t eEnable = DISABLE_NOTHING;
	disable_t eDisable = DISABLE_NOTHING;
	Vector2D vecTarget = Vector2D(0,0);		// Origin means do not use
	EAngle angTarget = EAngle(-1,-1,-1);	// Negative values means do not use
	float flDistance = 0;
	eastl::string sEmotion;

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Position")
		{
			eastl::string sPosition = pChildData->GetValueString();
			if (sPosition == "top-center")
				iPosition = POSITION_TOPCENTER;
			else if (sPosition == "top-left")
				iPosition = POSITION_TOPLEFT;
			else if (sPosition == "scene-tree")
				iPosition = POSITION_SCENETREE;
			else if (sPosition == "buttons")
				iPosition = POSITION_BUTTONS;
		}
		else if (pChildData->GetKey() == "Width")
			iWidth = pChildData->GetValueInt();
		else if (pChildData->GetKey() == "Next")
			sNext = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Text")
			sText = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Button1")
			sButton1Action = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Button1Text")
			sButton1Text = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Button2")
			sButton2Action = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Button2Text")
			sButton2Text = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Enable")
		{
			eastl::string sEnable = pChildData->GetValueString();
			int iEnable = eEnable;
			if (sEnable == "view-move")
				iEnable |= DISABLE_VIEW_MOVE;
			else if (sEnable == "view-rotate")
				iEnable |= DISABLE_VIEW_ROTATE;
			else if (sEnable == "enter")
				iEnable |= DISABLE_ENTER;
			else if (sEnable == "buffer")
				iEnable |= DISABLE_BUFFER;
			else if (sEnable == "psu")
				iEnable |= DISABLE_PSU;
			else if (sEnable == "loaders")
				iEnable |= DISABLE_LOADERS;
			eEnable = (disable_t)iEnable;
		}
		else if (pChildData->GetKey() == "Disable")
		{
			eastl::string sDisable = pChildData->GetValueString();
			int iDisable = eDisable;
			if (sDisable == "view-move")
				iDisable |= DISABLE_VIEW_MOVE;
			else if (sDisable == "view-rotate")
				iDisable |= DISABLE_VIEW_ROTATE;
			else if (sDisable == "enter")
				iDisable |= DISABLE_ENTER;
			else if (sDisable == "buffer")
				iDisable |= DISABLE_BUFFER;
			else if (sDisable == "psu")
				iDisable |= DISABLE_PSU;
			else if (sDisable == "loaders")
				iDisable |= DISABLE_LOADERS;
			eDisable = (disable_t)iDisable;
		}
		else if (pChildData->GetKey() == "SetViewTarget")
			vecTarget = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == "SetViewAngle")
			angTarget = pChildData->GetValueEAngle();
		else if (pChildData->GetKey() == "SetViewDistance")
			flDistance = pChildData->GetValueFloat();
		else if (pChildData->GetKey() == "HelperEmotion")
			sEmotion = pChildData->GetValueString();
	}

	m_apTutorials[sLessonName] = new CTutorial(this, sLessonName, sNext, iPosition, iWidth, !!sNext.length(), convertstring<char, char16_t>(sText));
	m_apTutorials[sLessonName]->m_sButton1Text = sButton1Text;
	m_apTutorials[sLessonName]->m_sButton1Action = sButton1Action;
	m_apTutorials[sLessonName]->m_sButton2Text = sButton2Text;
	m_apTutorials[sLessonName]->m_sButton2Action = sButton2Action;
	m_apTutorials[sLessonName]->m_eDisable = eDisable;
	m_apTutorials[sLessonName]->m_eEnable = eEnable;
	m_apTutorials[sLessonName]->m_vecSetViewTarget = vecTarget;
	m_apTutorials[sLessonName]->m_angSetViewAngle = angTarget;
	m_apTutorials[sLessonName]->m_flSetViewDistance = flDistance;
	m_apTutorials[sLessonName]->m_sHelperEmotion = sEmotion;
}

void CInstructor::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!bActive)
		HideTutorial();
}

void CInstructor::DisplayFirstTutorial(eastl::string sTutorial)
{
	m_bActive = true;
	m_sLastTutorial = "";
	m_sCurrentTutorial = sTutorial;
	m_eDisabled = DISABLE_NOTHING;
	DisplayTutorial(m_sCurrentTutorial);
}

void CInstructor::NextTutorial()
{
	if (!GetCurrentTutorial())
		return;

	DisplayTutorial(GetCurrentTutorial()->m_sNextTutorial);
}

CVar tutorial_enable("tutorial_enable", "1");

void CInstructor::DisplayTutorial(eastl::string sTutorial)
{
	if (!tutorial_enable.GetBool())
		return;

	if (!m_bActive)
		return;

	if (m_apTutorials.find(sTutorial) == m_apTutorials.end())
	{
		SetActive(false);
		return;
	}

	m_sCurrentTutorial = sTutorial;

	if (DigitanksGame() && m_sLastTutorial != m_sCurrentTutorial)
		DigitanksGame()->OnDisplayTutorial(sTutorial);

	m_sLastTutorial = m_sCurrentTutorial;

	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	int iDisabled = m_eDisabled;
	iDisabled |= m_apTutorials[sTutorial]->m_eDisable;
	iDisabled &= ~m_apTutorials[sTutorial]->m_eEnable;
	m_eDisabled = (disable_t)iDisabled;

	Vector2D vecTarget = m_apTutorials[sTutorial]->m_vecSetViewTarget;
	if (vecTarget.LengthSqr() > 0)
		DigitanksGame()->GetDigitanksCamera()->SetTarget(DigitanksGame()->GetTerrain()->SetPointHeight(Vector(vecTarget.x, 0, vecTarget.y)));

	EAngle angTarget = m_apTutorials[sTutorial]->m_angSetViewAngle;
	if (angTarget.p >= 0 || angTarget.y >= 0 || angTarget.r >= 0)
		DigitanksGame()->GetDigitanksCamera()->SetAngle(angTarget);

	float flDistance = m_apTutorials[sTutorial]->m_flSetViewDistance;
	if (flDistance > 0)
		DigitanksGame()->GetDigitanksCamera()->SetDistance(flDistance);

	m_pCurrentPanel = new CTutorialPanel(m_apTutorials[sTutorial]);
	glgui::CRootPanel::Get()->AddControl(m_pCurrentPanel, true);
}

void CInstructor::ShowTutorial()
{
	DisplayTutorial(m_sCurrentTutorial);
}

void CInstructor::HideTutorial()
{
	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
		m_pCurrentPanel = NULL;
	}
}

void CInstructor::FinishedTutorial(eastl::string sTutorial, bool bForceNext)
{
	if (sTutorial != m_sCurrentTutorial)
		return;

	if (m_pCurrentPanel)
	{
		// Only play the sound if the current panel is showing so we don't play it multiple times.
		CSoundLibrary::PlaySound(NULL, L"sound/lesson-learned.wav");

		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = NULL;

	if (m_apTutorials[sTutorial]->m_bAutoNext || bForceNext)
		NextTutorial();

	// If we get to the end here then we turn off the instructor as we have finished completely.
	if (GetCurrentTutorial() && GetCurrentTutorial()->m_bKillOnFinish)
		SetActive(false);
}

disable_t CInstructor::GetDisabledFeatures()
{
	return m_eDisabled;
}

bool CInstructor::IsFeatureDisabled(disable_t eFeature)
{
	if (!GetActive())
		return false;

	return !!(GetDisabledFeatures()&eFeature);
}

CTutorial::CTutorial(CInstructor* pInstructor, eastl::string sTutorial, eastl::string sNextTutorial, int iPosition, int iWidth, bool bAutoNext, eastl::string16 sText)
{
	m_pInstructor = pInstructor;
	m_sTutorialName = sTutorial;
	m_sNextTutorial = sNextTutorial;
	m_iPosition = iPosition;
	m_iWidth = iWidth;
	m_bAutoNext = bAutoNext;
	m_sText = sText;
	m_bKillOnFinish = false;
	m_flSlideAmount = 0;
	m_bSlideX = true;

	m_vecSetViewTarget = Vector2D(0, 0);
	m_angSetViewAngle = EAngle(-1, -1, -1);
	m_flSetViewDistance = 0;
}

CTutorialPanel::CTutorialPanel(CTutorial* pTutorial)
	: CPanel(0, 0, 100, 100)
{
	m_pTutorial = pTutorial;

	m_pText = new CLabel(0, 0, m_pTutorial->m_iWidth, 1000, L"");
	m_pText->SetText(str_replace(pTutorial->m_sText, L"\\n", L"\n"));
	m_pText->SetPos(10, 0);
	m_pText->SetSize(m_pTutorial->m_iWidth, (int)m_pText->GetTextHeight() + 10);
	m_pText->SetWrap(true);
	m_pText->SetAlign(CLabel::TA_MIDDLECENTER);
	m_pText->SetFont(L"text");
	AddControl(m_pText);

	m_pText->ComputeLines();

	if (m_pTutorial->m_sButton1Text.length() > 0)
	{
		m_pButton1 = new CButton(0, 0, 60, 35, convertstring<char, char16_t>(m_pTutorial->m_sButton1Text));
		m_pButton1->SetButtonColor(g_clrPanel);
		m_pButton1->SetFont(L"text");
		m_pButton1->SetClickedListener(this, Button1);
		AddControl(m_pButton1);
	}
	else
		m_pButton1 = NULL;

	if (m_pTutorial->m_sButton2Text.length() > 0)
	{
		m_pButton2 = new CButton(0, 0, 60, 35, convertstring<char, char16_t>(m_pTutorial->m_sButton2Text));
		m_pButton2->SetButtonColor(g_clrPanel);
		m_pButton2->SetFont(L"text");
		m_pButton2->SetClickedListener(this, Button2);
		AddControl(m_pButton2);
	}
	else
		m_pButton2 = NULL;

	if (m_pButton1 && m_pButton2)
	{
		m_pButton1->SetPos(m_pText->GetWidth()-160, m_pText->GetHeight()+10);
		m_pButton2->SetPos(m_pText->GetWidth()-80, m_pText->GetHeight()+10);
		SetSize(m_pText->GetWidth()+20, m_pText->GetHeight() + 55);
	}
	else if (m_pButton1)
	{
		m_pButton1->SetPos(m_pText->GetWidth()/2 - m_pButton1->GetWidth()/2, m_pText->GetHeight()+10);
		SetSize(m_pText->GetWidth()+20, m_pText->GetHeight() + 55);
	}
	else
		SetSize(m_pText->GetWidth()+20, m_pText->GetHeight());

	switch (pTutorial->m_iPosition)
	{
	case CInstructor::POSITION_TOPCENTER:
		SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pTutorial->m_iWidth/2, 100);
		break;

	case CInstructor::POSITION_BUTTONS:
		SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pTutorial->m_iWidth/2 + 150, glgui::CRootPanel::Get()->GetHeight()-160-GetHeight());
		break;

	case CInstructor::POSITION_SCENETREE:
		SetPos(150, 100);
		break;

	case CInstructor::POSITION_TOPLEFT:
		SetPos(100, 100);
		break;
	}

	if (GameServer())
		m_flStartTime = GameServer()->GetGameTime();
	else
		m_flStartTime = 0;
}

void CTutorialPanel::Paint(int x, int y, int w, int h)
{
	if (DigitanksWindow()->GetMenu() && DigitanksWindow()->GetMenu()->IsVisible())
		return;

	if (m_pTutorial->m_sTutorialName == "strategy-deploy")
	{
		CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();
		if (!pSelection)
			return;

		if (!dynamic_cast<CMobileCPU*>(pSelection))
			return;
	}

	if (m_pTutorial->m_iPosition == CInstructor::POSITION_BUTTONS && DigitanksWindow()->GetHUD()->IsButtonInfoVisible())
		return;

	if (m_pTutorial->m_iPosition == CInstructor::POSITION_SCENETREE)
		x += (int)(Lerp(Oscillate(GameServer()->GetGameTime(), 1.0f), 0.8f)*20);

	if (m_pTutorial->m_iPosition == CInstructor::POSITION_BUTTONS)
		y -= (int)(Lerp(Oscillate(GameServer()->GetGameTime(), 1.0f), 0.8f)*20);

	if (m_pTutorial->m_flSlideAmount > 0)
	{
		if (m_pTutorial->m_bSlideX)
			x += (int)(Lerp(RemapValClamped(GameServer()->GetGameTime() - m_flStartTime, 0, 1, 1, 0), 0.2f) * m_pTutorial->m_flSlideAmount);
		else
			y += (int)(Lerp(RemapValClamped(GameServer()->GetGameTime() - m_flStartTime, 0, 1, 1, 0), 0.2f) * m_pTutorial->m_flSlideAmount);
	}

	CRootPanel::PaintRect(x, y, w, h);

	CPanel::Paint(x, y, w, h);

	if (m_pTutorial->m_sTutorialName == "strategy-command")
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CHUD::PaintHUDSheet("ElectronodeExample", x + w + 10, y + h/2 - 222/2, 145, 211);
	}

	if (m_pTutorial->m_sHelperEmotion.length())
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CHUD::PaintSheet(DigitanksWindow()->GetInstructor()->GetEmotionsSheet(), m_pTutorial->m_sHelperEmotion, x - 108, y + h/2 - 256/2, 128, 256);
	}
}

bool CTutorialPanel::MousePressed(int code, int mx, int my)
{
	if (BaseClass::MousePressed(code, mx, my))
		return true;

	if (m_pTutorial->m_sButton1Action.length() == 0 && m_pTutorial->m_sButton2Action.length() == 0)
	{
		SetVisible(false);
		if (m_pTutorial->m_bAutoNext)
			m_pTutorial->m_pInstructor->NextTutorial();
		return true;
	}

	return false;
}

void CTutorialPanel::Button1Callback()
{
	if (m_pTutorial->m_sButton1Action.length() > 0)
		m_pTutorial->m_pInstructor->DisplayTutorial(m_pTutorial->m_sButton1Action);
}

void CTutorialPanel::Button2Callback()
{
	if (m_pTutorial->m_sButton2Action.length() > 0)
		m_pTutorial->m_pInstructor->DisplayTutorial(m_pTutorial->m_sButton2Action);
}
