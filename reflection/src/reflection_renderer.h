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

	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);
	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	StartRendering(class CRenderingContext* pContext);
	virtual void	FinishRendering(class CRenderingContext* pContext);
	virtual void	StartRenderingReflection(class CRenderingContext* pContext, CMirror* pMirror);
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
