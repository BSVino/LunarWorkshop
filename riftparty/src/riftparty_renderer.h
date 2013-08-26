#pragma once

#include <renderer/game_renderer.h>
#include <game/entityhandle.h>

class CRiftPartyRenderer : public CGameRenderer
{
	DECLARE_CLASS(CRiftPartyRenderer, CGameRenderer);

public:
					CRiftPartyRenderer();

public:
	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);
	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	FinishRendering(class CRenderingContext* pContext);

	float			BloomBrightnessCutoff() const { return 1.25f; }
};

CRiftPartyRenderer* RiftPartyRenderer();
