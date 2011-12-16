#ifndef CHAIN_STORY_H
#define CHAIN_STORY_H

#include <game/baseentity.h>
#include <glgui/glgui.h>

class CPage
{
public:
	tstring					m_sLines;
	tstring					m_sNextPage;
};

class CStory : public CBaseEntity, public glgui::IEventListener
{
	REGISTER_ENTITY_CLASS(CStory, CBaseEntity);

public:
							CStory();
							~CStory();

public:
	virtual void			Load(const tstring& sFile);

	virtual void			Spawn();
	virtual void			Think();

	virtual bool			ShouldRender() const { return true; }

	virtual void			OnRender(class CRenderingContext* pContext, bool bTransparent) const;

	void					MousePressed();

	float					LabelScale() const { return 100; }

	void					SetPage(const tstring& sPage);

	EVENT_CALLBACK(CStory, LinkClicked);

protected:
	glgui::CLabel*			m_pText;

	float					m_flAlpha;
	float					m_flAlphaGoal;

	eastl::map<tstring, CPage>	m_asPages;
	tstring					m_sCurrentPage;
	tstring					m_sNextPage;
};

#endif
