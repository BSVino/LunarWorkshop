#ifndef REFLECTION_RENDERER_H
#define REFLECTION_RENDERER_H

#include <renderer/renderer.h>
#include <game/entityhandle.h>

class CMirror;

class CReflectionRenderer : public CRenderer
{
	DECLARE_CLASS(CReflectionRenderer, CRenderer);

public:
					CReflectionRenderer();

public:
	virtual void	Initialize();
	virtual void	LoadShaders();

	virtual void	SetupFrame();
	virtual void	StartRenderingReflection(CMirror* pMirror);

	void			SetupShader(CRenderingContext* c, CModel* pModel, size_t iMaterial);

	float			BloomBrightnessCutoff() const { return 1.25f; }

	size_t			GetReflectionTexture();
	void			SetMirror(CMirror* pMirror);

protected:
	CFrameBuffer	m_oReflectionBuffer;
	CEntityHandle<CMirror>	m_hMirror;

	bool			m_bRenderingReflection;
};

CReflectionRenderer* ReflectionRenderer();

#endif
