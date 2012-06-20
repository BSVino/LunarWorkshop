#include "cvar.h"

#include <strutils.h>
#include <tvector.h>

#include <tinker/application.h>

CCommand::CCommand(tstring sName, CommandCallback pfnCallback)
{
	m_sName = sName;
	m_pfnCallback = pfnCallback;

	RegisterCommand(this);
}

void CCommand::Run(tstring sCommand)
{
	tvector<tstring> asTokens;
	tstrtok(sCommand, asTokens);

	if (asTokens.size() == 0)
		return;

	tmap<tstring, CCommand*>::iterator it = GetCommands().find(asTokens[0]);
	if (it == GetCommands().end())
	{
		TMsg("Unrecognized command.\n");
		return;
	}

	CCommand* pCommand = it->second;
	pCommand->m_pfnCallback(pCommand, asTokens, sCommand);
}

tvector<tstring> CCommand::GetCommandsBeginningWith(tstring sFragment)
{
	tvector<tstring> sResults;

	size_t iFragLength = sFragment.length();

	tmap<tstring, CCommand*>& sCommands = GetCommands();
	for (tmap<tstring, CCommand*>::iterator it = sCommands.begin(); it != sCommands.end(); it++)
	{
		if (it->first.substr(0, iFragLength) == sFragment)
			sResults.push_back(it->first);
	}

	return sResults;
}

void CCommand::RegisterCommand(CCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}

void SetCVar(CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	CVar* pCVar = dynamic_cast<CVar*>(pCommand);
	TAssert(pCVar);
	if (!pCVar)
		return;

	if (asTokens.size() > 1)
		pCVar->SetValue(asTokens[1]);

	TMsg(sprintf(tstring("%s = %s\n"), pCVar->GetName().c_str(), pCVar->GetValue().c_str()));
}

CVar::CVar(tstring sName, tstring sValue)
	: CCommand(sName, ::SetCVar)
{
	m_sValue = sValue;

	m_bDirtyValues = true;
}

void CVar::SetValue(tstring sValue)
{
	m_sValue = sValue;

	m_bDirtyValues = true;
}

void CVar::SetValue(int iValue)
{
	m_sValue = sprintf(tstring("%d"), iValue);

	m_bDirtyValues = true;
}

void CVar::SetValue(float flValue)
{
	m_sValue = sprintf(tstring("%f"), flValue);

	m_bDirtyValues = true;
}

bool CVar::GetBool()
{
	if (m_bDirtyValues)
		CalculateValues();

	return m_bValue;
}

int CVar::GetInt()
{
	if (m_bDirtyValues)
		CalculateValues();

	return m_iValue;
}

float CVar::GetFloat()
{
	if (m_bDirtyValues)
		CalculateValues();

	return m_flValue;
}

void CVar::CalculateValues()
{
	if (!m_bDirtyValues)
		return;

	m_flValue = (float)atof(m_sValue.c_str());
	m_iValue = atoi(m_sValue.c_str());
	m_bValue = (m_sValue.comparei("yes") == 0 ||
		m_sValue.comparei("true") == 0 ||
		m_sValue.comparei("on") == 0 ||
		atoi(m_sValue.c_str()) != 0);

	m_bDirtyValues = false;
}

CVar* CVar::FindCVar(tstring sName)
{
	tmap<tstring, CCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	CVar* pVar = dynamic_cast<CVar*>(it->second);
	return pVar;
}

void CVar::SetCVar(tstring sName, tstring sValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(sValue);
}

void CVar::SetCVar(tstring sName, int iValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(iValue);
}

void CVar::SetCVar(tstring sName, float flValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(flValue);
}

tstring CVar::GetCVarValue(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return "";

	return pVar->GetValue();
}

bool CVar::GetCVarBool(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return false;

	return pVar->GetBool();
}

int CVar::GetCVarInt(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetInt();
}

float CVar::GetCVarFloat(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetFloat();
}
