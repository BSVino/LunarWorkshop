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

	float			BloomBrightnessCutoff() const { return 1.25f; }
};

CChainRenderer* ChainRenderer();

#endif
