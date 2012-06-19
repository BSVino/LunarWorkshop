#ifndef TINKER_PROFILER_H
#define TINKER_PROFILER_H

#include <EASTL/map.h>

#include <tstring.h>
#include <tvector.h>

#define TPROF(name) CProfileScope _TProf(name);

class CProfileScope
{
public:
								CProfileScope(const tstring& sName);
								~CProfileScope();

public:
	tstring						GetName() { return m_sName; };

protected:
	tstring						m_sName;
};

class CPerfBlock
{
public:
								CPerfBlock(const tstring& sName, CPerfBlock* pParent);

public:
	CPerfBlock*					GetParent() { return m_pParent; };

	CPerfBlock*					GetChild(const tstring& sName);
	CPerfBlock*					AddChild(const tstring& sName);

	void						BeginFrame();

	void						BlockStarted();
	void						BlockEnded();

	tstring						GetName() { return m_sName; };
	double						GetTime() { return m_flTime; };

public:
	CPerfBlock*					m_pParent;

	tstring						m_sName;
	double						m_flTime;

	double						m_flTimeBlockStarted;

	eastl::map<tstring, CPerfBlock*>	m_apPerfBlocks;
};

class CProfiler
{
public:
	static void					BeginFrame();

	static void					PushScope(CProfileScope* pScope);
	static void					PopScope(CProfileScope* pScope);

	static void					Render();

	static bool					IsProfiling() { return s_bProfiling; };

protected:
	static void					PopAllScopes();

	static void					Render(CPerfBlock* pBlock, float& flLeft, float& flTop);

protected:
	static CPerfBlock*			s_pBottomBlock;
	static CPerfBlock*			s_pTopBlock;

	static bool					s_bProfiling;
};

#endif
