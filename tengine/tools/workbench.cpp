#include "workbench.h"

#include <tinker_platform.h>
#include <files.h>

#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <glgui/tree.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>
#include <glgui/checkbox.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <tinker/profiler.h>
#include <textures/materiallibrary.h>
#include <tinker/keys.h>
#include <game/gameserver.h>

#include "manipulator.h"

extern int g_iImportLevelEditor;
extern int g_iImportToyEditor;
extern int g_iImportToyViewer;
// Use this to force import of required entities.
// This shit totally defeats the purpose of having auto registrations.
class CAutoToolsImport
{
public:
	CAutoToolsImport()
	{
		g_iImportLevelEditor = 1;
		g_iImportToyEditor = 1;
		g_iImportToyViewer = 1;
	}
} g_AutoToolsImport = CAutoToolsImport();

void CWorkbenchCamera::Think()
{
	BaseClass::Think();

	Workbench()->GetActiveTool()->CameraThink();
}

TVector CWorkbenchCamera::GetCameraPosition()
{
	return Workbench()->GetActiveTool()->GetCameraPosition();
}

TVector CWorkbenchCamera::GetCameraDirection()
{
	return Workbench()->GetActiveTool()->GetCameraDirection();
}

CWorkbench::CWorkbench()
{
	for (auto it = GetToolRegistration().begin(); it != GetToolRegistration().end(); it++)
		m_apTools.push_back(it->second.m_pfnToolCreator());

	m_bActive = false;

	TAssert(m_apTools.size());
	m_iActiveTool = 0;

	m_pCamera = new CWorkbenchCamera();

	m_pFileMenu = glgui::CRootPanel::Get()->AddMenu("File");

	glgui::CMenu* pToolsMenu = glgui::CRootPanel::Get()->AddMenu("Tools");
	for (size_t i = 0; i < m_apTools.size(); i++)
		pToolsMenu->AddSubmenu(m_apTools[i]->GetToolName(), this, MenuSelected);

	glgui::CRootPanel::Get()->Layout();
}

CWorkbench::~CWorkbench()
{
	delete m_pCamera;
}

bool CWorkbench::KeyPress(int c)
{
	return GetActiveTool()->KeyPress(c);
}

bool CWorkbench::MouseInput(int iButton, int iState)
{
	if (Manipulator()->MouseInput(iButton, iState))
		return true;

	return GetActiveTool()->MouseInput(iButton, iState);
}

void CWorkbench::MouseMotion(int x, int y)
{
	GetActiveTool()->MouseMotion(x, y);
}

void CWorkbench::MouseWheel(int x, int y)
{
	GetActiveTool()->MouseWheel(x, y);
}

void CWorkbench::SetActiveTool(int iTool)
{
	m_pFileMenu->ClearSubmenus();

	if (GetActiveTool())
		GetActiveTool()->Deactivate();

	m_iActiveTool = iTool;

	if (GetActiveTool())
	{
		GetActiveTool()->Activate();
		Application()->SetMouseCursorEnabled(true);
	}
}

void CWorkbench::MenuSelectedCallback(const tstring& sArgs)
{
	eastl::vector<tstring> asTokens;
	strtok(sArgs, asTokens);

	if (!asTokens.size())
		return;

	int iMenu = stoi(asTokens[0]);

	SetActiveTool(iMenu);
}

void CWorkbench::Toggle()
{
	if (!Workbench())
		return;

	if (IsActive())
		Workbench()->Deactivate();
	else
		Workbench()->Activate();
}

bool CWorkbench::IsActive()
{
	if (!Workbench(false))
		return false;

	return Workbench()->m_bActive;
}

void CWorkbench::Activate()
{
	if (!Workbench())
		return;

	Workbench()->m_bWasMouseActive = Application()->IsMouseCursorEnabled();
	Application()->SetMouseCursorEnabled(true);

	Workbench()->m_pFileMenu->ClearSubmenus();

	if (Workbench()->GetActiveTool())
		Workbench()->GetActiveTool()->Activate();

	Workbench()->m_bActive = true;

	glgui::CRootPanel::Get()->GetMenuBar()->SetVisible(true);
}

void CWorkbench::Deactivate()
{
	if (!Workbench())
		return;

	Workbench()->m_bActive = false;

	Application()->SetMouseCursorEnabled(Workbench()->m_bWasMouseActive);

	if (Workbench()->GetActiveTool())
		Workbench()->GetActiveTool()->Deactivate();

	glgui::CRootPanel::Get()->GetMenuBar()->SetVisible(false);
}

void CWorkbench::RenderScene()
{
	if (!Workbench())
		return;

	if (!IsActive())
		return;

	Workbench()->GetActiveTool()->RenderScene();

	Manipulator()->Render();
}

CCamera* CWorkbench::GetCamera()
{
	return Workbench()->m_pCamera;
}

void CWorkbench::RegisterTool(const char* pszTool, ToolCreator pfnToolCreator)
{
	CToolRegistration* pTool = &GetToolRegistration()[pszTool];
	pTool->m_pfnToolCreator = pfnToolCreator;
}

eastl::map<tstring, CWorkbench::CToolRegistration>& CWorkbench::GetToolRegistration()
{
	static eastl::map<tstring, CToolRegistration> aToolRegistration;
	return aToolRegistration;
}

CWorkbenchTool* CWorkbench::GetActiveTool()
{
	if (m_iActiveTool >= m_apTools.size())
		return nullptr;

	return m_apTools[m_iActiveTool];
}

CWorkbench* Workbench(bool bCreate)
{
	// This function won't work unless we're in dev mode.
	// I don't want memory wasted on the level editor for most players.
	if (!CVar::GetCVarBool("developer"))
		return nullptr;

	static bool bCreated = false;

	if (!bCreated && !bCreate)
		return nullptr;

	static CWorkbench* pWorkbench = new CWorkbench();
	bCreated = true;

	return pWorkbench;
}
