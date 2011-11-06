#ifndef CHAIN_STORY_H
#define CHAIN_STORY_H

#include <game/baseentity.h>
#include <glgui/glgui.h>

class CStory : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CStory, CBaseEntity);

public:
							CStory();
							~CStory();

public:
	virtual void			Spawn();
	virtual void			Think();

	virtual bool			ShouldRender() const { return true; }

	virtual void			OnRender(class CRenderingContext* pContext, bool bTransparent) const;

	void					NextPage();

protected:
	glgui::CLabel*			m_pText;
	size_t					m_iPage;

	float					m_flAlpha;
	float					m_flAlphaGoal;
};

#endif
