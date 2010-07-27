#include "instructor.h"

#include <glgui/glgui.h>

#include "digitanks/digitank.h"
#include "digitanks/digitanksgame.h"
#include "digitankswindow.h"
#include "renderer/renderer.h"
#include "sound/sound.h"

using namespace glgui;

CInstructor::CInstructor()
{
	m_bActive = true;
	m_pCurrentPanel = NULL;
	m_iLastTutorial = -1;
	Initialize();

	CSoundLibrary::Get()->AddSound("sound/lesson-learned.wav");
}

CInstructor::~CInstructor()
{
	HideTutorial();
	Clear();
}

void CInstructor::Clear()
{
	for (size_t i = 0; i < m_apTutorials.size(); i++)
		delete m_apTutorials[i];
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

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_INTRO_BASICS, new CTutorial(this, TUTORIAL_INTRO_BASICS, POSITION_TOPCENTER, 200, true,
		L"Welcome to Digitanks!\n \nThis tutorial will help you get accustomed to the game.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_SELECTION, new CTutorial(this, TUTORIAL_SELECTION, POSITION_TOPCENTER, 200, true,
		L"TANK SELECTION\n \nThis is your tank. Select it by clicking on it with the left mouse button.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_MOVECAMERA, new CTutorial(this, TUTORIAL_MOVECAMERA, POSITION_TOPCENTER, 220, true,
		L"VIEW CONTROLS\n \nFirst, let's take a look around. Left click on open terrain to re-center the view.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_TURNCAMERA, new CTutorial(this, TUTORIAL_TURNCAMERA, POSITION_TOPCENTER, 210, true,
		L"VIEW CONTROLS\n \nHold down the left mouse button and drag to rotate the view.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_ZOOMCAMERA, new CTutorial(this, TUTORIAL_ZOOMCAMERA, POSITION_TOPCENTER, 200, true,
		L"VIEW CONTROLS\n \nYou can use the mouse wheel to zoom in and out.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_MOVE, new CTutorial(this, TUTORIAL_MOVE, POSITION_TOPLEFT, 300, true,
		L"MOVE YOUR TANK\n \nAn enemy tank is closeby. You are now in 'move mode' and can move your tank closer to get a better shot. You can only move once per turn. Right click inside the yellow area to move your tank.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_AIM, new CTutorial(this, TUTORIAL_AIM, POSITION_TOPLEFT, 200, true,
		L"AIM YOUR TANK\n \nYou're closing in for the kill! Now aim at the enemy tank by right clicking on him.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_RANGE, new CTutorial(this, TUTORIAL_RANGE, POSITION_TOPLEFT, 300, true,
		L"TANK RANGE\n \nYour tank has very good accuracy inside the green area, but outside that your accuracy will decrease. Your tank's maximum range is the red circle.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_BUTTONS, new CTutorial(this, TUTORIAL_BUTTONS, POSITION_TOPLEFT, 200, true,
		L"CONTROL BUTTONS\n \nYou can enter move mode and aim mode at any time by using the buttons on the lower right of the screen.\n \nYou can exit move turn or aim mode by pressing the 'Escape' key.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_ENERGY, new CTutorial(this, TUTORIAL_ENERGY, POSITION_TOPLEFT, 300, true,
		L"ENERGIZE YOUR CANNON\n \nNow choose how much Energy you want to use on your attack. Press the 'Set Energy' button and the Energy sliders will appear next to your tank. Move the slider to the desired level and click the left mouse button to select a setting.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWERPOINTS, new CTutorial(this, TUTORIAL_POWERPOINTS, POSITION_TOPCENTER, 300, true,
		L"TANK ENERGY\n \nEach Digitank has a limited amount of Energy which can be used for attack, defense, or movement. The bars at the bottom of the screen show the selected tank's Energy.\n \nChoose how you use your Energy carefully, because using your Energy in one area means it can't be used in another. You can't save your Energy for the next turn, but any Energy used will be available again next turn.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_ENTERKEY, new CTutorial(this, TUTORIAL_ENTERKEY, POSITION_TOPCENTER, 200, false,
		L"END YOUR TURN\n \nNow press the 'Enter' key to end your turn and fire your tank.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_TURN, new CTutorial(this, TUTORIAL_TURN, POSITION_TOPLEFT, 300, true,
		L"TURN YOUR TANK\n \nGood job, you messed that guy up pretty good! However, his return fire damaged your shields. You can rotate your tank to bring another shield to bear. Turning your tank takes some Energy. Click the 'Turn' button and right click a spot to turn your tank. Your tank will aim its body directly at that spot.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_FINISHHIM, new CTutorial(this, TUTORIAL_FINISHHIM, POSITION_TOPLEFT, 200, false,
		L"FINISH THE JOB\n \nNow let's finish the job. Aim your tank at the enemy again and press enter to end your turn and fire. If you have trouble hitting the tank, try rotating your camera for a better view.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_UPGRADE, new CTutorial(this, TUTORIAL_UPGRADE, POSITION_TOPCENTER, 250, true,
		L"UPGRADE YOUR TANK\n \nYou destroyed the enemy tank, and received a promotion! Your promotion gives you bonus points that you can use to upgrade your tank's Energy. Press the 'Promote' button to show a list of upgrade options, and then select an upgrade.\n \nKilling enemy tanks can grant you additional bonus points.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWERUP, new CTutorial(this, TUTORIAL_POWERUP, POSITION_TOPLEFT, 250, true,
		L"GET THE POWERUP\n \nLook at that! A powerup has appeared next to your tank. Move your tank on top of the powerup and it will turn green. Keep the tank in this position and you'll be able to use the bonus points next turn. Picking up more powerups can grant you additional bonus points.\n \nPress the 'Enter' key to end your turn and pick up the powerup.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_THEEND_BASICS, new CTutorial(this, TUTORIAL_THEEND_BASICS, POSITION_TOPCENTER, 250, false,
		L"END OF TUTORIAL\n \nThat's it! Now is a good time to move on to the Bases tutorial. You can also start a new game by opening the menu with the 'Escape' key. Enjoy Digitanks!")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_INTRO_BASES, new CTutorial(this, TUTORIAL_INTRO_BASES, POSITION_TOPCENTER, 250, true,
		L"Welcome to Digitanks!\n \nThis tutorial will help you learn how to build bases. It is recommended to finish the basics tutorial first.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_CPU, new CTutorial(this, TUTORIAL_CPU, POSITION_TOPCENTER, 250, true,
		L"THE CENTRAL PROCESSING UNIT (CPU)\n \nThis is your CPU. It is your command center. If it is destroyed then you lose the game, so protect it well.\n \nThe CPU is the source of your Network. Your Network is represented by the glowy tendrils that come from your CPU.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_BUFFER, new CTutorial(this, TUTORIAL_BUFFER, POSITION_TOPCENTER, 250, false,
		L"CONSTRUCTING\n \nYou can use your CPU to construct other structures. Construct a Buffer by clicking the 'Build Buffer' button and then right-clicking on open terrain inside your Network.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWER, new CTutorial(this, TUTORIAL_POWER, POSITION_TOPCENTER, 250, false,
		L"POWER\n \nYour buffer is now constructing. Structures take Power to build. Power accumulates every turn, you can see how much power you are accumulating this turn at the top right of the screen. After enough Power has accumulated, the structure is complete. For this tutorial, the build time for your Buffer has been shortened.\n \nPress 'Enter' to complete construction.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_NETWORK, new CTutorial(this, TUTORIAL_NETWORK, POSITION_TOPCENTER, 250, true,
		L"THE NETWORK\n \nBuffers and CPUs extend your Network, marking your terrain. Friendly units inside your Network will receive combat bonuses. You can only build more structures inside your Network.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_PSU, new CTutorial(this, TUTORIAL_PSU, POSITION_TOPCENTER, 250, false,
		L"POWER SUPPLIES\n \nPower Supplies can extract Power from Electronodes. They must be built close to an Electronode, and only one per Electronode can be built. There is an Electronode next to your CPU.\n \nClick the CPU to select it. Then press the 'Build Power Supply' button and right click near the Electronode to build a Power Supply. Press the 'Enter' key to complete construction.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_SUPPLY, new CTutorial(this, TUTORIAL_SUPPLY, POSITION_TOPCENTER, 250, true,
		L"SUPPLY LINES\n \nAll units and structures maintain supply lines to the nearest Buffer or CPU. These lines provide support, such as combat bonuses and health regeneration. They can be broken by moving an enemy unit on them, so be sure to protect them.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_LOADER, new CTutorial(this, TUTORIAL_LOADER, POSITION_TOPCENTER, 250, false,
		L"LOADERS\n \nLoaders are specialized structures that produce combat units. With the CPU selected, build a Loader by pressing one the 'Build Loader' button, selecting any loader, and right clicking on your Network. Then press the 'Enter' key to complete construction.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_EFFICIENCY, new CTutorial(this, TUTORIAL_EFFICIENCY, POSITION_TOPCENTER, 250, true,
		L"EFFICIENCY\n \nBuilding more than two structures off any Buffer or CPU will cause the structures to become inefficient. It's best to spread out your structures so they use many different Buffers. However, the Buffers themselves aren't affected, you can have as many buffers as you want.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_PRODUCING_UNITS, new CTutorial(this, TUTORIAL_PRODUCING_UNITS, POSITION_TOPCENTER, 250, false,
		L"PRODUCING UNITS\n \nNow that your Loader has finished constructing, it can start producing units. Select the Loader and click the build button to begin producing a unit.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_LIMITED_POWER, new CTutorial(this, TUTORIAL_LIMITED_POWER, POSITION_TOPCENTER, 250, true,
		L"LIMITED POWER\n \nSince Power is shared by all structures, building too many structures and units at once can consume too much Power and slow down production. Be sure to pace yourself when issuing commands to construct and produce units.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_THEEND_BASES, new CTutorial(this, TUTORIAL_THEEND_BASES, POSITION_TOPCENTER, 250, false,
		L"END OF TUTORIAL\n \nThat's it! You can start a new game by opening the menu with the 'Escape' key. Enjoy Digitanks!")));
}

void CInstructor::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!bActive)
		HideTutorial();
	else
		Initialize();
}

void CInstructor::DisplayFirstBasicsTutorial()
{
	m_iLastTutorial = -1;
	m_iCurrentTutorial = TUTORIAL_INTRO_BASICS;
	DisplayTutorial(m_iCurrentTutorial);
}

void CInstructor::DisplayFirstBasesTutorial()
{
	m_iLastTutorial = -1;
	m_iCurrentTutorial = TUTORIAL_INTRO_BASES;
	DisplayTutorial(m_iCurrentTutorial);
}

void CInstructor::NextTutorial()
{
	DisplayTutorial(++m_iCurrentTutorial);
}

void CInstructor::DisplayTutorial(size_t iTutorial)
{
	if (!m_bActive)
		return;

	if (m_apTutorials.find(iTutorial) == m_apTutorials.end())
	{
		SetActive(false);
		return;
	}

	// May not skip or go back tutorials!
	if (iTutorial < m_iCurrentTutorial || iTutorial > m_iCurrentTutorial+1)
		return;

	m_iCurrentTutorial = iTutorial;

	if (DigitanksGame() && m_iLastTutorial != m_iCurrentTutorial)
		DigitanksGame()->OnDisplayTutorial(iTutorial);

	m_iLastTutorial = m_iCurrentTutorial;

	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = new CTutorialPanel(m_apTutorials[iTutorial]);
	glgui::CRootPanel::Get()->AddControl(m_pCurrentPanel, true);
}

void CInstructor::ShowTutorial()
{
	DisplayTutorial(m_iCurrentTutorial);
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

void CInstructor::FinishedTutorial(size_t iTutorial)
{
	if (iTutorial != m_iCurrentTutorial)
		return;

	if (m_pCurrentPanel)
	{
		// Only play the sound if the current panel is showing so we don't play it multiple times.
		CSoundLibrary::PlaySound(NULL, "sound/lesson-learned.wav");

		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = NULL;

	if (m_apTutorials[iTutorial]->m_bAutoNext)
		m_apTutorials[iTutorial]->m_pInstructor->NextTutorial();
}

disable_t CInstructor::GetDisabledFeatures()
{
	int iDisabled = 0;

	if (GetCurrentTutorial() < TUTORIAL_TURNCAMERA)
		iDisabled |= DISABLE_ROTATE;

	if (GetCurrentTutorial() < TUTORIAL_ENTERKEY)
		iDisabled |= DISABLE_ENTER;

	if (GetCurrentTutorial() < TUTORIAL_BUFFER)
		iDisabled |= DISABLE_BUFFER;

	if (GetCurrentTutorial() < TUTORIAL_PSU)
		iDisabled |= DISABLE_PSU;

	if (GetCurrentTutorial() < TUTORIAL_LOADER)
		iDisabled |= DISABLE_LOADERS;

	// Keep people from killing the enemy tank before the turn dialogue goes away.
	if (GetCurrentTutorial() == TUTORIAL_TURN)
		iDisabled |= DISABLE_ENTER;

	return (disable_t)iDisabled;
}

bool CInstructor::IsFeatureDisabled(disable_t eFeature)
{
	if (!GetActive())
		return false;

	return !!(GetDisabledFeatures()&eFeature);
}

CTutorial::CTutorial(CInstructor* pInstructor, size_t iTutorial, int iPosition, int iWidth, bool bAutoNext, std::wstring sText)
{
	m_pInstructor = pInstructor;
	m_iTutorial = iTutorial;
	m_iPosition = iPosition;
	m_iWidth = iWidth;
	m_bAutoNext = bAutoNext;
	m_sText = sText;
}

CTutorialPanel::CTutorialPanel(CTutorial* pTutorial)
	: CPanel(0, 0, 100, 100)
{
	m_pTutorial = pTutorial;

	m_pText = new CLabel(0, 0, m_pTutorial->m_iWidth, 1000, "");
	m_pText->SetText(pTutorial->m_sText.c_str());
	m_pText->SetPos(10, 10);
	m_pText->SetSize(m_pTutorial->m_iWidth, (int)m_pText->GetTextHeight());
	m_pText->SetWrap(true);
	m_pText->SetAlign(CLabel::TA_MIDDLECENTER);
	AddControl(m_pText);

	SetSize(m_pText->GetWidth()+20, m_pText->GetHeight()+20);

	switch (pTutorial->m_iPosition)
	{
	case CInstructor::POSITION_TOPCENTER:
		SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pTutorial->m_iWidth/2, 100);
		break;

	case CInstructor::POSITION_POWERBARS:
		SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pTutorial->m_iWidth/2 + 50, glgui::CRootPanel::Get()->GetHeight()-160-GetHeight());
		break;

	case CInstructor::POSITION_ACTIVETANK:
	{
		CSelectable* pSelection = DigitanksGame()->GetCurrentSelection();

		if (!pSelection)
			break;

		Vector vecCurrentSelectable = Game()->GetRenderer()->ScreenPosition(pSelection->GetOrigin());

		SetPos((int)vecCurrentSelectable.x - GetWidth() - 100, (int)vecCurrentSelectable.y - 50);
		if (GetLeft() < 0)
			SetPos(0, GetTop());
		if (GetTop() < 0)
			SetPos(GetLeft(), 0);
		break;
	}

	case CInstructor::POSITION_TOPLEFT:
		SetPos(100, 100);
		break;
	}
}

void CTutorialPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h);

	CPanel::Paint(x, y, w, h);
}

bool CTutorialPanel::MousePressed(int code, int mx, int my)
{
	SetVisible(false);
	if (m_pTutorial->m_bAutoNext)
		m_pTutorial->m_pInstructor->NextTutorial();
	return true;
}
