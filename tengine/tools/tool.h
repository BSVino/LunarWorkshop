#pragma once

#include <tengine_config.h>
#include <vector.h>
#include <tstring.h>

class CWorkbenchTool
{
public:
							CWorkbenchTool() {};
	virtual					~CWorkbenchTool() {};

public:
	virtual bool			KeyPress(int c) { return false; }
	virtual bool			MouseInput(int iButton, int iState) { return false; }

	virtual void			Activate() {};
	virtual void			Deactivate() {};

	virtual void			RenderScene() {};

	virtual void			CameraThink() {};
	virtual TVector			GetCameraPosition() { return Vector(-10, 0, 0); }
	virtual TVector			GetCameraDirection() { return Vector(1, 0, 0); }

	virtual tstring			GetToolName() { return "Invalid Tool"; }
};
