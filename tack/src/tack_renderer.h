#ifndef TACK_RENDERER_H
#define TACK_RENDERER_H

#include <renderer/renderer.h>

class CTackRenderer : public CRenderer
{
	DECLARE_CLASS(CTackRenderer, CRenderer);

public:
					CTackRenderer();

public:
	virtual void	LoadShaders();

	virtual void	StartRendering();

	float			BloomBrightnessCutoff() const { return 1.25f; }
};

#endif
