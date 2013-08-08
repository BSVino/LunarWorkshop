#pragma once

#include <renderer/game_renderer.h>
#include <game/entityhandle.h>

class CMirror;

class CGrottoRenderer : public CGameRenderer
{
	DECLARE_CLASS(CGrottoRenderer, CGameRenderer);

public:
					CGrottoRenderer();

public:
	virtual void	Initialize();

	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);
	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	StartRendering(class CRenderingContext* pContext);
	virtual void	FinishRendering(class CRenderingContext* pContext);
	virtual void	StartRenderingReflection(class CRenderingContext* pContext, CMirror* pMirror);
	virtual bool    ModifyShader(const class CBaseEntity* pEntity, class CRenderingContext* c);
	bool            IsRenderingReflection() const { return !!m_pRenderingReflection; }
	CMirror*        GetRenderingReflectionMirror() const { return m_pRenderingReflection; }

	float			BloomBrightnessCutoff() const { return 1.25f; }

	CFrameBuffer&	GetReflectionBuffer(size_t i);

	virtual bool	ShouldRenderPhysicsDebug() const;

protected:
	tvector<CFrameBuffer>		m_aoReflectionBuffers;

	CMirror*        m_pRenderingReflection; // Don't need a handle because mirrors are never destroyed during rendering.
};

CGrottoRenderer* GrottoRenderer();
