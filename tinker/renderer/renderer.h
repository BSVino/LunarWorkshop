#ifndef TINKER_RENDERER_H
#define TINKER_RENDERER_H

#include <EASTL/map.h>
#include <EASTL/vector.h>

#include <tstring.h>
#include <vector.h>
#include <matrix.h>
#include <color.h>
#include <frustum.h>

#include "render_common.h"

typedef enum
{
	FB_DEPTH = (1<<0),
	FB_TEXTURE = (1<<1),
	FB_RENDERBUFFER = (1<<2),
	FB_LINEAR = (1<<3),
	FB_MULTISAMPLE = (1<<4),
} fb_options_e;

class CFrameBuffer
{
public:
					CFrameBuffer();

public:
	unsigned int	m_iWidth;
	unsigned int	m_iHeight;

	unsigned int	m_iRB;
	unsigned int	m_iMap;
	unsigned int	m_iDepth;
	unsigned int	m_iFB;

	Vector2D		m_vecTexCoords[4];
	Vector2D		m_vecVertices[4];

	bool			m_bMultiSample;
};

#define BLOOM_FILTERS 3

class CRenderer
{
	friend class CRenderingContext;

public:
					CRenderer(size_t iWidth, size_t iHeight);

public:
	virtual void	Initialize();
	void			LoadShaders();

	CFrameBuffer	CreateFrameBuffer(size_t iWidth, size_t iHeight, fb_options_e eOptions);

	// PreFrame is run before thinks, physics, etc.
	virtual void	PreFrame();
	virtual void	PostFrame();

	// PreRender is run before the primary rendering starts. Good time to render targets, ie cameras mirrors etc.
	virtual void	PreRender();
	virtual void	PostRender();

	virtual void	ModifyContext(class CRenderingContext* pContext);
	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	DrawBackground(class CRenderingContext* pContext);
	virtual void	StartRendering(class CRenderingContext* pContext);
	virtual void	FinishRendering(class CRenderingContext* pContext);
	virtual void	FinishFrame(class CRenderingContext* pContext);
	virtual void	RenderOffscreenBuffers(class CRenderingContext* pContext);
	virtual void	RenderFullscreenBuffers(class CRenderingContext* pContext);

	virtual float	BloomBrightnessCutoff() const { return 0.6f; }
	void			RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal);

	void			RenderFrameBufferFullscreen(CFrameBuffer* pBuffer);
	void			RenderFrameBufferToBuffer(CFrameBuffer* pSource, CFrameBuffer* pDestination);
	void			RenderRBFullscreen(CFrameBuffer* pSource);
	void			RenderRBToBuffer(CFrameBuffer* pSource, CFrameBuffer* pBuffer);
	void			RenderMapFullscreen(size_t iMap, bool bMapIsMultisample = false);
	void			RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer, bool bMapIsMultisample = false);

	void			SetCameraPosition(Vector vecCameraPosition) { m_vecCameraPosition = vecCameraPosition; };
	void			SetCameraDirection(Vector vecCameraDirection) { m_vecCameraDirection = vecCameraDirection; };
	void			SetCameraUp(Vector vecCameraUp) { m_vecCameraUp = vecCameraUp; };
	void			SetCameraFOV(float flFOV) { m_flCameraFOV = flFOV; };
	void			SetCameraNear(float flNear) { m_flCameraNear = flNear; };
	void			SetCameraFar(float flFar) { m_flCameraFar = flFar; };

	Vector			GetCameraPosition() { return m_vecCameraPosition; };
	Vector			GetCameraDirection() { return m_vecCameraDirection; };
	float			GetCameraFOV() { return m_flCameraFOV; };
	float			GetCameraNear() { return m_flCameraNear; };
	float			GetCameraFar() { return m_flCameraFar; };

	void			FrustumOverride(Vector vecPosition, Vector vecDirection, float flFOV, float flNear, float flFar);
	void			CancelFrustumOverride();
	const Frustum&	GetFrustum() const { return m_oFrustum; }

	Vector			GetCameraVector();
	void			GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp);

	bool			IsSphereInFrustum(const Vector& vecCenter, float flRadius);

	void			SetSize(int w, int h);

	Vector			ScreenPosition(Vector vecWorld);
	Vector			WorldPosition(Vector vecScreen);

	const CFrameBuffer*	GetSceneBuffer() { return &m_oSceneBuffer; }

	bool			HardwareSupported();

	int				ScreenSamples() { return m_iScreenSamples; }

public:
	static size_t	LoadVertexDataIntoGL(size_t iSizeInBytes, float* aflVertices);
	static void		UnloadVertexDataFromGL(size_t iBuffer);
	static size_t	LoadTextureIntoGL(tstring sFilename, int iClamp = 0);
	static size_t	LoadTextureIntoGL(Color* pclrData, int w, int h, int iClamp = 0);
	static void		UnloadTextureFromGL(unsigned int iGLID);
	static size_t	GetNumTexturesLoaded() { return s_iTexturesLoaded; }

	static Color*	LoadTextureData(tstring sFilename, int& w, int& h);
	static void		UnloadTextureData(Color* pData);

protected:
	size_t			m_iWidth;
	size_t			m_iHeight;

	Vector			m_vecCameraPosition;
	Vector			m_vecCameraDirection;
	Vector			m_vecCameraUp;
	float			m_flCameraFOV;
	float			m_flCameraNear;
	float			m_flCameraFar;

	bool			m_bFrustumOverride;
	Vector			m_vecFrustumPosition;
	Vector			m_vecFrustumDirection;
	float			m_flFrustumFOV;
	float			m_flFrustumNear;
	float			m_flFrustumFar;

	double			m_aflModelView[16];
	double			m_aflProjection[16];
	int				m_aiViewport[4];

	Frustum			m_oFrustum;

	Vector2D		m_vecFullscreenTexCoords[6];
	Vector			m_vecFullscreenVertices[6];

	bool			m_bDrawBackground;

	CFrameBuffer	m_oSceneBuffer;

	CFrameBuffer	m_oBloom1Buffers[BLOOM_FILTERS];
	CFrameBuffer	m_oBloom2Buffers[BLOOM_FILTERS];

	bool			m_bUseMultisampleTextures;
	int				m_iScreenSamples;

	static size_t	s_iTexturesLoaded;
};

#endif
