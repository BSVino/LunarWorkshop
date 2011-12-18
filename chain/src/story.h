#ifndef CHAIN_STORY_H
#define CHAIN_STORY_H

#include <game/baseentity.h>
#include <glgui/glgui.h>

class CPage
{
public:
	tstring					m_sLines;
	tstring					m_sNextPage;
	tstring					m_sPrevPage;
};

class CStory : public CBaseEntity, public glgui::IEventListener
{
	REGISTER_ENTITY_CLASS(CStory, CBaseEntity);

public:
							CStory();
							~CStory();

public:
	virtual void			Load(const tstring& sFile);

	virtual void			Precache();
	virtual void			Spawn();
	virtual void			Think();

	virtual bool			ShouldRender() const { return true; }

	virtual void			OnRender(class CRenderingContext* pContext, bool bTransparent) const;

	void					MousePressed();

	float					LabelScale() const { return 100; }

	size_t					GetNumPages() const { return m_asPages.size(); }
	const tstring&			GetPageID(size_t i) const;
	void					SetPage(const tstring& sPage);
	CPage*					GetCurrentPage() { return &m_asPages[m_sCurrentPage]; }
	void					GoToNextPage();
	void					GoToPrevPage();

	float					GetAlpha() { return m_flAlpha; }

	EVENT_CALLBACK(CStory, LinkClicked);
	EVENT_CALLBACK(CStory, SectionHovered);

protected:
	glgui::CLabel*			m_pText;

	float					m_flAlpha;
	float					m_flAlphaGoal;

	eastl::map<tstring, CPage>	m_asPages;
	tstring					m_sCurrentPage;
	tstring					m_sNextPage;

	class GoalValue
	{
	public:
		float		m_flGoal;
		float		m_flValue;
	};

	eastl::map<tstring, GoalValue>	m_aflHighlightedSections;
};

#endif
