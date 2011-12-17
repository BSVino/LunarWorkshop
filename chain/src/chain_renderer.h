#ifndef CHAIN_RENDERER_H
#define CHAIN_RENDERER_H

#include <renderer/renderer.h>
#include <game/entityhandle.h>

class CMirror;

class CChainRenderer : public CRenderer
{
	DECLARE_CLASS(CChainRenderer, CRenderer);

public:
					CChainRenderer();

public:
	virtual void	LoadShaders();

	virtual void	SetupFrame();
	virtual void	DrawBackground() {};
	virtual void	RenderFullscreenBuffers();

	float			BloomBrightnessCutoff() const { return 1.25f; }

	const CFrameBuffer&	GetMouseoverBuffer() { return m_oMouseoverBuffer1; }

protected:
	CFrameBuffer	m_oMouseoverBuffer1;
	CFrameBuffer	m_oMouseoverBuffer2;
};

CChainRenderer* ChainRenderer();

#endif
