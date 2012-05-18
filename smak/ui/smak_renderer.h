#pragma once

#include <renderer/renderer.h>

class CMirror;

class CSMAKRenderer : public CRenderer
{
	DECLARE_CLASS(CSMAKRenderer, CRenderer);

public:
					CSMAKRenderer();

public:
	void			Render();
	void			DrawBackground(CRenderingContext* r);
	void			Render3D();
	void			RenderUV();

	float			BloomBrightnessCutoff() const { return 1.25f; }

protected:
};

CSMAKRenderer* SMAKRenderer();
