#include "profiler.h"

#include <GL/glew.h>

#include <strutils.h>

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>

CVar prof_enable("prof_enable", "no");

CProfileScope::CProfileScope(const eastl::string& sName)
{
	m_sName = sName;

	CProfiler::PushScope(this);
}

CProfileScope::~CProfileScope()
{
	CProfiler::PopScope(this);
}

CPerfBlock::CPerfBlock(const eastl::string& sName, CPerfBlock* pParent)
{
	m_pParent = pParent;
	m_sName = sName;
	m_flTime = 0;
}

CPerfBlock* CPerfBlock::GetChild(const eastl::string& sName)
{
	eastl::map<eastl::string, CPerfBlock*>::iterator it = m_apPerfBlocks.find(sName);

	if (it == m_apPerfBlocks.end())
		return NULL;

	return it->second;
}

CPerfBlock* CPerfBlock::AddChild(const eastl::string& sName)
{
	CPerfBlock* pChild = new CPerfBlock(sName, this);
	m_apPerfBlocks[sName] = pChild;
	return pChild;
}

void CPerfBlock::BeginFrame()
{
	m_flTime = 0;

	for (eastl::map<eastl::string, CPerfBlock*>::iterator it = m_apPerfBlocks.begin(); it != m_apPerfBlocks.end(); it++)
		it->second->BeginFrame();
}

void CPerfBlock::BlockStarted()
{
	m_flTimeBlockStarted = CApplication::Get()->GetTime();
}

void CPerfBlock::BlockEnded()
{
	float flTimeBlockEnded = CApplication::Get()->GetTime();

	m_flTime += flTimeBlockEnded - m_flTimeBlockStarted;
}

CPerfBlock* CProfiler::s_pTopBlock = NULL;
CPerfBlock* CProfiler::s_pBottomBlock = NULL;
bool CProfiler::s_bProfiling = false;

void CProfiler::BeginFrame()
{
	s_bProfiling = prof_enable.GetBool();

	if (s_pBottomBlock)
		s_pBottomBlock->BeginFrame();

	// Just in case.
	s_pTopBlock = NULL;
}

void CProfiler::PushScope(CProfileScope* pScope)
{
	if (!IsProfiling())
		return;

	CPerfBlock* pBlock = NULL;

	if (!s_pTopBlock)
	{
		if (!s_pBottomBlock)
			s_pBottomBlock = new CPerfBlock(pScope->GetName(), NULL);

		pBlock = s_pBottomBlock;
	}
	else
	{
		pBlock = s_pTopBlock->GetChild(pScope->GetName());

		if (!pBlock)
			pBlock = s_pTopBlock->AddChild(pScope->GetName());
	}

	pBlock->BlockStarted();
	s_pTopBlock = pBlock;
}

void CProfiler::PopScope(CProfileScope* pScope)
{
	if (!IsProfiling())
		return;

	TAssert(s_pTopBlock);
	if (!s_pTopBlock)
		return;

	TAssert(pScope->GetName() == s_pTopBlock->GetName());

	s_pTopBlock->BlockEnded();

	s_pTopBlock = s_pTopBlock->GetParent();
}

void CProfiler::PopAllScopes()
{
	if (!IsProfiling())
		return;

	CPerfBlock* pBlock = s_pTopBlock;

	while (pBlock)
	{
		pBlock->BlockEnded();
		pBlock = pBlock->GetParent();
	}

	s_pTopBlock = NULL;
}

void CProfiler::Render()
{
	if (!IsProfiling())
		return;

	if (!s_pBottomBlock)
		return;

	PopAllScopes();

	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flCurrLeft = flWidth - 400;
	float flCurrTop = 200;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, flWidth, flHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	glColor4ubv(Color(255, 255, 255, 255));

	glgui::CBaseControl::PaintRect(flCurrLeft, flCurrTop, 400, 800, Color(0, 0, 0, 150), 5, true);

	Render(s_pBottomBlock, flCurrLeft, flCurrTop);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CProfiler::Render(CPerfBlock* pBlock, float& flLeft, float& flTop)
{
	flLeft += 15;
	flTop += 15;

	Color clrBlock(255, 255, 255);
	if (pBlock->GetTime() < 0.005)
		clrBlock = Color(255, 255, 255, 150);

	glgui::CBaseControl::PaintRect(flLeft, flTop+1, pBlock->GetTime()*5000, 1, clrBlock);

	tstring sName = convertstring<char, tchar>(pBlock->GetName());
	sName += sprintf(tstring(": %d ms"), (int)(pBlock->GetTime()*1000));
	glColor4ubv(clrBlock);
	glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, (float)flLeft, (float)flTop);

	for (eastl::map<eastl::string, CPerfBlock*>::iterator it = pBlock->m_apPerfBlocks.begin(); it != pBlock->m_apPerfBlocks.end(); it++)
		Render(it->second, flLeft, flTop);

	flLeft -= 15;
}
