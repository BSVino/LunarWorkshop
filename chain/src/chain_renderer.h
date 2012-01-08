#ifndef CHAIN_RENDERER_H
#define CHAIN_RENDERER_H

#include <renderer/game_renderer.h>
#include <game/entityhandle.h>

class CMirror;

class CChainRenderer : public CGameRenderer
{
	DECLARE_CLASS(CChainRenderer, CGameRenderer);

public:
					CChainRenderer();

public:
	virtual void	LoadShaders();

	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	RenderFullscreenBuffers(class CRenderingContext* pContext);

	float			BloomBrightnessCutoff() const { return 1.25f; }

	const CFrameBuffer&	GetMouseoverBuffer() { return m_oMouseoverBuffer1; }

protected:
	CFrameBuffer	m_oMouseoverBuffer1;
	CFrameBuffer	m_oMouseoverBuffer2;
};

CChainRenderer* ChainRenderer();

#endif
