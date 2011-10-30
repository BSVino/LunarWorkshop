#ifndef REFLECTION_RENDERER_H
#define REFLECTION_RENDERER_H

#include <renderer/renderer.h>

class CReflectionRenderer : public CRenderer
{
	DECLARE_CLASS(CReflectionRenderer, CRenderer);

public:
					CReflectionRenderer();

public:
	virtual void	LoadShaders();

	virtual void	StartRendering();

	float			BloomBrightnessCutoff() const { return 1.25f; }
};

#endif
