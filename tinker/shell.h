#pragma once

#include <EASTL/string.h>

#include <tvector.h>
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
	virtual double				GetTime();

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	const tstring&				GetBinaryName() { return m_sBinaryName; }

	virtual void				PrintConsole(const tstring& sText);
	virtual void				PrintError(const tstring& sText);

	static inline CShell*		Get() { return s_pShell; };

protected:
	tstring						m_sBinaryName;
	tvector<const char*>		m_apszCommandLine;

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
