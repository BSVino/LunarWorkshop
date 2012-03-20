#include "shell.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <time.h>

#include <strutils.h>
#include <tinker_platform.h>
#include <mtrand.h>
#include <tinker/cvar.h>

CShell* CShell::s_pShell = NULL;

CShell::CShell(int argc, char** argv)
{
	s_pShell = this;

	srand((unsigned int)time(NULL));
	mtsrand((size_t)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_sBinaryName = argv[0];

	for (int i = 1; i < argc; i++)
	{
		if (m_apszCommandLine[i][0] == '+')
			CCommand::Run(&m_apszCommandLine[i][1]);
	}
}

CShell::~CShell()
{
}

float CShell::GetTime()
{
	TAssert(false);
	return 0;
}

bool CShell::HasCommandLineSwitch(const char* pszSwitch)
{
	for (size_t i = 0; i < m_apszCommandLine.size(); i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return true;
	}

	return false;
}

const char* CShell::GetCommandLineSwitchValue(const char* pszSwitch)
{
	// -1 to prevent buffer overrun
	for (size_t i = 0; i < m_apszCommandLine.size()-1; i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return m_apszCommandLine[i+1];
	}

	return NULL;
}

void CShell::PrintConsole(const tstring& sText)
{
	puts(convertstring<tchar, char>(sText).c_str());
}

void CShell::PrintError(const tstring& sText)
{
	puts(convertstring<tchar, char>(tstring("ERROR: ") + sText).c_str());
}

void CreateApplicationWithErrorHandling(CreateApplicationCallback pfnCallback, int argc, char** argv)
{
#ifdef _WIN32
#ifndef _DEBUG
	__try
	{
#endif
#endif

		// Put in a different function to avoid warnings and errors associated with object deconstructors and try/catch blocks.
		pfnCallback(argc, argv);

#if defined(_WIN32) && !defined(_DEBUG)
	}
	__except (CreateMinidump(GetExceptionInformation(), "Tinker"), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}
