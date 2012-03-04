#pragma once

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <common.h>
#include <vector.h>
#include <color.h>
#include <configfile.h>

class CShell
{
public:
								CShell(int argc, char** argv);
	virtual 					~CShell();

public:
	virtual float				GetTime();

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	virtual void				PrintConsole(const tstring& sText);
	virtual void				PrintError(const tstring& sText);

	static inline CShell*		Get() { return s_pShell; };

protected:
	eastl::vector<const char*>	m_apszCommandLine;

	static CShell*				s_pShell;
};

inline CShell* Shell()
{
	return CShell::Get();
}

// Tinker messages and errors
#define TMsg Shell()->PrintConsole
#define TError Shell()->PrintError

typedef void (*CreateApplicationCallback)(int argc, char** argv);
void CreateApplicationWithErrorHandling(CreateApplicationCallback pfnCallback, int argc, char** argv);
