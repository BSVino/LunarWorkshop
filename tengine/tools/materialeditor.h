#pragma once

#include "tool.h"

class CMaterialEditor : public CWorkbenchTool
{
public:
							CMaterialEditor();
	virtual					~CMaterialEditor();

public:
	virtual tstring			GetToolName() { return "Material Editor"; }

protected:
};
