#ifndef TENGINE_GAME_RENDERER_H
#define TENGINE_GAME_RENDERER_H

#include <renderer/renderer.h>
#include <textures/materialhandle.h>
#include <textures/texturehandle.h>

class CRenderBatch
{
public:
	const class CBaseEntity*	pEntity;
	class CModel*				pModel;
	Matrix4x4					mTransformation;
	bool						bWinding;
	Color						clrRender;
	size_t						iMaterial;
};

class CGameRenderer : public CRenderer
{
	DECLARE_CLASS(CGameRenderer, CRenderer);

	friend class CGameRenderingContext;

public:
					CGameRenderer(size_t iWidth, size_t iHeight);

public:
	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	DrawSkybox(class CRenderingContext* pContext);
	virtual void	ModifySkyboxContext(class CRenderingContext* c) {};
	virtual void	FinishRendering(class CRenderingContext* pContext);

	void			SetSkybox(const CTextureHandle& ft, const CTextureHandle& bk, const CTextureHandle& lf, const CTextureHandle& rt, const CTextureHandle& up, const CTextureHandle& dn);
	void			DisableSkybox();

	bool			ShouldBatchThisFrame() { return m_bBatchThisFrame; }
	void			BeginBatching();
	void			AddToBatch(class CModel* pModel, const class CBaseEntity* pEntity, const Matrix4x4& mTransformations, const Color& clrRender, bool bWinding);
	void			RenderBatches();
	bool			IsBatching() { return m_bBatching; };

	void			ClassifySceneAreaPosition(class CModel* pModel);
	void			FindSceneAreaPosition(class CModel* pModel);
	size_t			GetSceneAreaPosition(class CModel* pModel);

	const class CBaseEntity*	GetRenderingEntity() { return m_pRendering; }

	virtual bool	ShouldRenderPhysicsDebug() const { return true; };

protected:
	CTextureHandle	m_hSkyboxFT;
	CTextureHandle	m_hSkyboxLF;
	CTextureHandle	m_hSkyboxBK;
	CTextureHandle	m_hSkyboxRT;
	CTextureHandle	m_hSkyboxDN;
	CTextureHandle	m_hSkyboxUP;

	Vector2D		m_avecSkyboxTexCoords[6];
	Vector			m_avecSkyboxFT[6];
	Vector			m_avecSkyboxBK[6];
	Vector			m_avecSkyboxLF[6];
	Vector			m_avecSkyboxRT[6];
	Vector			m_avecSkyboxUP[6];
	Vector			m_avecSkyboxDN[6];

	bool			m_bBatchThisFrame;
	bool			m_bBatching;
	eastl::map<CMaterialHandle, eastl::vector<CRenderBatch> > m_aBatches;

	const CBaseEntity*	m_pRendering;

	eastl::map<tstring, size_t> m_aiCurrentSceneAreas;

	static size_t	s_iTexturesLoaded;
};

#endif
