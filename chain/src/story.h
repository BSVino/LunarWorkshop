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
	virtual bool			ShouldRender() const { return true; }

	virtual void			OnRender(class CRenderingContext* pContext, bool bTransparent) const;

protected:
	glgui::CLabel*			m_pText;
};

#endif
