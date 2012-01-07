#ifndef TENGINE_GAME_RENDERER_H
#define TENGINE_GAME_RENDERER_H

#include <renderer/renderer.h>

class CRenderBatch
{
public:
	const class CBaseEntity*	pEntity;
	class CModel*				pModel;
	Matrix4x4					mTransformation;
	bool						bReverseWinding;
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
	virtual void	LoadShaders();

	virtual void	SetupFrame();
	virtual void	DrawSkybox();
	virtual void	ModifySkyboxContext(class CRenderingContext* c) {};
	virtual void	FinishRendering();

	void			SetSkybox(size_t ft, size_t bk, size_t lf, size_t rt, size_t up, size_t dn);
	void			DisableSkybox();

	bool			ShouldBatchThisFrame() { return m_bBatchThisFrame; }
	void			BeginBatching();
	void			AddToBatch(class CModel* pModel, const class CBaseEntity* pEntity, const Matrix4x4& mTransformations, const Color& clrRender, bool bReverseWinding);
	void			RenderBatches();
	bool			IsBatching() { return m_bBatching; };

	void			ClassifySceneAreaPosition(class CModel* pModel);
	void			FindSceneAreaPosition(class CModel* pModel);
	size_t			GetSceneAreaPosition(class CModel* pModel);

	virtual void	SetupShader(CRenderingContext* c, CModel* pModel, size_t iMaterial);

	const class CBaseEntity*	GetRenderingEntity() { return m_pRendering; }

	virtual bool	ShouldRenderPhysicsDebug() const { return true; };

protected:
	size_t			m_iSkyboxFT;
	size_t			m_iSkyboxLF;
	size_t			m_iSkyboxBK;
	size_t			m_iSkyboxRT;
	size_t			m_iSkyboxDN;
	size_t			m_iSkyboxUP;

	Vector2D		m_avecSkyboxTexCoords[6];
	Vector			m_avecSkyboxFT[6];
	Vector			m_avecSkyboxBK[6];
	Vector			m_avecSkyboxLF[6];
	Vector			m_avecSkyboxRT[6];
	Vector			m_avecSkyboxUP[6];
	Vector			m_avecSkyboxDN[6];

	bool			m_bBatchThisFrame;
	bool			m_bBatching;
	eastl::map<size_t, eastl::vector<CRenderBatch> > m_aBatches;

	const CBaseEntity*	m_pRendering;

	eastl::map<tstring, size_t> m_aiCurrentSceneAreas;

	static size_t	s_iTexturesLoaded;
};

#endif
