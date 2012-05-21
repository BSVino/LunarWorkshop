#pragma once

#include <renderer/renderer.h>
#include <textures/materialhandle.h>

class CMirror;

class CSMAKRenderer : public CRenderer
{
	DECLARE_CLASS(CSMAKRenderer, CRenderer);

public:
					CSMAKRenderer();

public:
	virtual void	Initialize();

	void			Render();
	void			DrawBackground(CRenderingContext* r);
	void			Render3D();
	void			RenderGround();
	void			RenderObjects();
	void			RenderSceneNode(class CConversionSceneNode* pNode);
	void			RenderMeshInstance(class CConversionMeshInstance* pMeshInstance);
	void			RenderLightSource();
	void			RenderUV();

	float			BloomBrightnessCutoff() const { return 1.25f; }

protected:
	Vector			m_vecLightPosition;

	CMaterialHandle	m_hLightHalo;
	CMaterialHandle	m_hLightBeam;
};

CSMAKRenderer* SMAKRenderer();
