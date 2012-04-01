#pragma once

#include <tstring.h>
#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/camera.h>

#include "tool.h"

class CWorkbenchCamera : public CCamera
{
	DECLARE_CLASS(CWorkbenchCamera, CCamera);

public:
	virtual void	Think();

	virtual TVector	GetCameraPosition();
	virtual TVector	GetCameraDirection();
};

class CWorkbench : public glgui::IEventListener
{
	friend class CWorkbenchCamera;
	friend class CWorkbenchTool;

public:
	typedef CWorkbenchTool* (*ToolCreator)();

public:
							CWorkbench();
	virtual					~CWorkbench();

public:
	bool					KeyPress(int c);
	bool					MouseInput(int iButton, int iState);
	void					MouseMotion(int x, int y);
	void					MouseWheel(int x, int y);

	void					SetActiveTool(int iTool);

	EVENT_CALLBACK(CWorkbench, MenuSelected);

public:
	static void				Toggle();
	static bool				IsActive();

	static void				Activate();
	static void				Deactivate();

	static void				RenderScene();

	static CCamera*			GetCamera();

	static void				RegisterTool(const char* pszTool, ToolCreator pfnToolCreator);

protected:
	class CToolRegistration
	{
	public:
		ToolCreator			m_pfnToolCreator;
	};

	static eastl::map<tstring, CToolRegistration>& GetToolRegistration();

	CWorkbenchTool*			GetActiveTool();

protected:
	bool					m_bActive;
	bool					m_bWasMouseActive;

	CWorkbenchCamera*		m_pCamera;

	eastl::vector<CWorkbenchTool*>	m_apTools;
	size_t					m_iActiveTool;

	glgui::CMenu*			m_pFileMenu;
};

CWorkbench* Workbench(bool bCreate=true);

#define REGISTER_WORKBENCH_TOOL(tool) \
class CRegisterWorkbench##tool \
{ \
public: \
	static CWorkbenchTool* Create##tool() \
	{ \
		return new C##tool(); \
	} \
	CRegisterWorkbench##tool() \
	{ \
		CWorkbench::RegisterTool(#tool, &Create##tool); \
	} \
} g_RegisterWorkbench##tool = CRegisterWorkbench##tool(); \
int g_iImport##tool = 0; \

