#pragma once

#include <tengine_config.h>
#include <vector.h>
#include <tstring.h>

#include <tinker/keys.h>

namespace glgui
{
	class CMenu;
};

class CWorkbenchTool
{
public:
							CWorkbenchTool() {};
	virtual					~CWorkbenchTool() {};

public:
	virtual bool			KeyPress(int c) { return false; }
	virtual bool			MouseInput(int iButton, tinker_mouse_state_t iState) { return false; }
	virtual void			MouseMotion(int x, int y) {};
	virtual void			MouseWheel(int x, int y) {};

	virtual void			Activate() {};
	virtual void			Deactivate() {};

	virtual void			RenderScene() {};

	virtual void			CameraThink() {};
	virtual TVector			GetCameraPosition() { return Vector(-10, 0, 0); }
	virtual TVector			GetCameraDirection() { return Vector(1, 0, 0); }

	virtual tstring			GetToolName() { return "Invalid Tool"; }

	glgui::CMenu*			GetFileMenu();
};
