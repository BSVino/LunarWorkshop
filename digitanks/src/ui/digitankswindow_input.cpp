#include "digitankswindow.h"

#include <GL/glew.h>
#include <GL/glfw.h>

#include <sound/sound.h>
#include "glgui/glgui.h"
#include "digitanks/digitanksgame.h"
#include "instructor.h"
#include "hud.h"
#include "menu.h"
#include <game/digitanks/cpu.h>
#include <game/digitanks/dt_camera.h>

#include <renderer/dissolver.h>

#ifdef _DEBUG
#include <game/digitanks/maintank.h>
#endif

void CDigitanksWindow::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		m_iMouseMoved += (int)(fabs((float)x-m_iMouseStartX) + fabs((float)y-m_iMouseStartY));

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseInput(int iButton, int iState)
{
	if (GetGame() && GetGame()->GetCamera())
	{
		// MouseButton enables camera rotation, so don't send the signal if the feature is disabled.
		if (!m_pInstructor->IsFeatureDisabled(DISABLE_ROTATE))
			GetGame()->GetCamera()->MouseButton(iButton, iState);
	}

	int mx, my;
	glfwGetMousePos(&mx, &my);
	if (iState == GLFW_PRESS)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, mx, my))
			return;
	}
	else
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, mx, my))
			return;
	}

	if (!DigitanksGame())
		return;

	if (iState == GLFW_PRESS)
	{
		if ((iButton == GLFW_MOUSE_BUTTON_1 || iButton == GLFW_MOUSE_BUTTON_2) && DigitanksGame()->GetControlMode() == MODE_FIRE)
		{
			DigitanksGame()->SetControlMode(MODE_NONE);

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_ENERGY);

			return;	// Don't center camera
		}
		else if (iButton == GLFW_MOUSE_BUTTON_2 && DigitanksGame()->GetControlMode() == MODE_BUILD && iState == GLFW_RELEASE && m_iMouseMoved < 30)
		{
			CCPU* pCPU = dynamic_cast<CCPU*>(DigitanksGame()->GetPrimarySelectionStructure());
			if (pCPU && pCPU->IsPreviewBuildValid())
			{
				pCPU->BeginConstruction();
				DigitanksGame()->SetControlMode(MODE_NONE);
			}
		}
	}

	Vector vecMousePosition;
	CBaseEntity* pClickedEntity = NULL;
	bool bFound = GetMouseGridPosition(vecMousePosition, &pClickedEntity);

	if (iButton == GLFW_MOUSE_BUTTON_2)
	{
		if (iState == GLFW_PRESS)
			m_iMouseMoved = 0;
	}

	if (iButton == GLFW_MOUSE_BUTTON_1)
	{
		if (GetGame() && GetGame()->GetCamera())
		{
			DigitanksGame()->GetDigitanksCamera()->SetTarget(vecMousePosition);
			CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
		}
	}

	if (iState == GLFW_PRESS && iButton == GLFW_MOUSE_BUTTON_1)
	{
		if (pClickedEntity)
		{
			CSelectable* pSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

			if (pSelectable)
			{
				if (IsShiftDown())
					DigitanksGame()->GetLocalDigitanksTeam()->AddToSelection(pSelectable);
				else
					DigitanksGame()->GetLocalDigitanksTeam()->SetPrimarySelection(pSelectable);
			}
		}
	}

	if (iButton == GLFW_MOUSE_BUTTON_2 && iState == GLFW_RELEASE && m_iMouseMoved < 30)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->SetDesiredMove();
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->SetDesiredTurn(vecMousePosition);
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
			DigitanksGame()->SetDesiredAim();
	}

	GetHUD()->SetupMenu();
}

void CDigitanksWindow::MouseWheel(int iState)
{
	static int iOldState = 0;

	if (GetGame() && GetGame()->GetCamera())
	{
		if (iState > iOldState)
			DigitanksGame()->GetDigitanksCamera()->ZoomIn();
		else
			DigitanksGame()->GetDigitanksCamera()->ZoomOut();
	}

	iOldState = iState;
}

void CDigitanksWindow::KeyEvent(int c, int e)
{
	if (e == GLFW_PRESS)
		KeyPress(c);
	else
		KeyRelease(c);
}

void CDigitanksWindow::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c))
		return;

	if (DigitanksGame() && (c == GLFW_KEY_ENTER || c == GLFW_KEY_KP_ENTER))
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->SetDesiredMove();
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->SetDesiredTurn();
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
			DigitanksGame()->SetDesiredAim();
		else if (DigitanksGame()->GetControlMode() == MODE_FIRE)
			DigitanksGame()->SetControlMode(MODE_NONE);

		else if (!m_pInstructor->IsFeatureDisabled(DISABLE_ENTER))
		{
			CSoundLibrary::PlaySound(NULL, "sound/turn.wav");
			DigitanksGame()->EndTurn();
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_ENTERKEY);
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_POWER);
		}
	}

	if (DigitanksGame() && c == GLFW_KEY_TAB)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			if (DigitanksGame()->GetPrimarySelectionTank())
			{
				if (!DigitanksGame()->GetPrimarySelectionTank()->HasDesiredAim())
					DigitanksGame()->SetControlMode(MODE_AIM);
			}
		}
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			if (DigitanksGame()->GetPrimarySelectionTank() && DigitanksGame()->GetPrimarySelectionTank()->HasDesiredAim())
				DigitanksGame()->SetControlMode(MODE_FIRE);
			else
				DigitanksGame()->SetControlMode(MODE_MOVE);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_FIRE)
			DigitanksGame()->SetControlMode(MODE_MOVE);
		else
		{
			if (DigitanksGame()->GetPrimarySelectionTank())
			{
				if (!DigitanksGame()->GetPrimarySelectionTank()->HasDesiredMove())
					DigitanksGame()->SetControlMode(MODE_MOVE);
				else if (!DigitanksGame()->GetPrimarySelectionTank()->HasDesiredAim())
					DigitanksGame()->SetControlMode(MODE_AIM);
			}
		}
	}

	if (c == GLFW_KEY_ESC)
	{
		if (GetMenu()->IsVisible())
			GetMenu()->SetVisible(false);
		else if (DigitanksGame()->GetControlMode() == MODE_NONE || DigitanksGame()->GetPrimarySelection() == NULL)
			GetMenu()->SetVisible(true);
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
	}

	if (c == 'H')
	{
		if (DigitanksGame()->GetLocalDigitanksTeam())
		{
			for (size_t i = 0; i < DigitanksGame()->GetLocalDigitanksTeam()->GetNumMembers(); i++)
			{
				CBaseEntity* pMember = DigitanksGame()->GetLocalDigitanksTeam()->GetMember(i);
				CCPU* pCPU = dynamic_cast<CCPU*>(pMember);
				if (pCPU)
				{
					DigitanksGame()->GetLocalDigitanksTeam()->SetPrimarySelection(pCPU);
					break;
				}
			}
		}
	}

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->KeyDown(c);

	if (c == 'Q')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(0);

	if (c == 'W')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(1);

	if (c == 'E')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(2);

	if (c == 'R')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(3);

	if (c == 'T')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(4);

	if (c == 'A')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(5);

	if (c == 'S')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(6);

	if (c == 'D')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(7);

	if (c == 'F')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(8);

	if (c == 'G')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(9);

	// Cheats from here on out
	if (c == 'X')
		DigitanksGame()->SetRenderFogOfWar(!DigitanksGame()->ShouldRenderFogOfWar());

	if (c == 'C')
		DigitanksGame()->CompleteProductions();

	if (c == 'V')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->GetPrimarySelection()->Delete();
	}

	if (c == 'B')
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();
		for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
		{
			for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
			{
				if (DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y].m_eUpdateClass == UPDATECLASS_EMPTY)
					continue;

				pTeam->DownloadUpdate(x, y, false);
				pTeam->DownloadComplete();
			}
		}
	}

	if (c == 'N')
		CDigitanksWindow::Get()->GetHUD()->SetVisible(!CDigitanksWindow::Get()->GetHUD()->IsVisible());

	if (c == 'M')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->TankSpeak(DigitanksGame()->GetPrimarySelectionTank(), ":D!");
	}
}

void CDigitanksWindow::KeyRelease(int c)
{
	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->KeyUp(c);
}

bool CDigitanksWindow::IsCtrlDown()
{
	return glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL);
}

bool CDigitanksWindow::IsAltDown()
{
	return glfwGetKey(GLFW_KEY_LALT) || glfwGetKey(GLFW_KEY_RALT);
}

bool CDigitanksWindow::IsShiftDown()
{
	return glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT);
}

