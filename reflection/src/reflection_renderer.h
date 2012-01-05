#ifndef REFLECTION_RENDERER_H
#define REFLECTION_RENDERER_H

#include <renderer/game_renderer.h>
#include <game/entityhandle.h>

class CMirror;

class CReflectionRenderer : public CGameRenderer
{
	DECLARE_CLASS(CReflectionRenderer, CGameRenderer);

public:
					CReflectionRenderer();

public:
	virtual void	Initialize();
	virtual void	LoadShaders();

	virtual void	SetupFrame();
	virtual void	StartRendering();
	virtual void	FinishRendering();
	virtual void	StartRenderingReflection(CMirror* pMirror);
	virtual void	RenderFullscreenBuffers();
	bool			IsRenderingReflection() const { return m_bRenderingReflection; }

	void			SetupShader(CRenderingContext* c, CModel* pModel, size_t iMaterial);

	float			BloomBrightnessCutoff() const { return 1.25f; }

	CFrameBuffer&	GetReflectionBuffer(size_t i);

	virtual bool	ShouldRenderPhysicsDebug() const;

protected:
	eastl::vector<CFrameBuffer>	m_aoReflectionBuffers;

	bool						m_bRenderingReflection;
};

CReflectionRenderer* ReflectionRenderer();

#endif
