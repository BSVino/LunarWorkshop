#ifndef SMAK_CRUNCH_H
#define SMAK_CRUNCH_H

#include <modelconverter/convmesh.h>
#include <parallelize.h>
#include "ui/modelwindow.h"
#include <worklistener.h>
#include <common.h>

namespace raytrace
{
	class CRaytracer;
	class CTraceResult;
};

class CTexelMethod
{
public:
							CTexelMethod(class CTexelGenerator* pGenerator);
	virtual 				~CTexelMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			PreGenerate() {};

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer) {};

	virtual void			PostGenerate() {};

	virtual size_t			GenerateDiffuse(bool bInMedias = false) { return 0; };
	virtual bool			GenerateNormal(size_t& iGLId, size_t& iILId, bool bInMedias = false) { return false; };
	virtual size_t			GenerateAO(bool bInMedias = false) { return 0; };

	virtual void			SaveToFile(const tstring& sFilename);

	virtual tstring	FileSuffix() { return ""; };
	virtual void*			GetData() { return NULL; };

protected:
	CTexelGenerator*		m_pGenerator;

	size_t					m_iWidth;
	size_t					m_iHeight;
};

class CTexelDiffuseMethod : public CTexelMethod
{
	DECLARE_CLASS(CTexelDiffuseMethod, CTexelMethod);

public:
							CTexelDiffuseMethod(class CTexelGenerator* pGenerator);
	virtual 				~CTexelDiffuseMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			PreGenerate();

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer);

	virtual void			PostGenerate();
	void					Bleed();

	virtual size_t			GenerateDiffuse(bool bInMedias = false);

	virtual tstring	FileSuffix() { return "diffuse"; };
	virtual void*			GetData();

protected:
	Vector*					m_avecDiffuseValues;
	Vector*					m_avecDiffuseGeneratedValues;
	size_t*					m_aiDiffuseReads;

	tvector<size_t>	m_aiTextures;
};

class CTexelAOMethod : public CTexelMethod
{
	DECLARE_CLASS(CTexelAOMethod, CTexelMethod);

public:
							CTexelAOMethod(class CTexelGenerator* pGenerator, size_t iSamples, bool bRandomize, float flRayFalloff, bool bGroundOcclusion, size_t iBleed);
	virtual 				~CTexelAOMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer);

	virtual void			PostGenerate();
	void					Bleed();

	virtual size_t			GenerateAO(bool bInMedias = false);

	virtual tstring	FileSuffix() { return "ao"; };
	virtual void*			GetData();

protected:
	size_t					m_iSamples;
	bool					m_bRandomize;
	float					m_flRayFalloff;
	bool					m_bGroundOcclusion;
	size_t					m_iBleed;

	Vector*					m_avecShadowValues;
	Vector*					m_avecShadowGeneratedValues;
	size_t*					m_aiShadowReads;
};

class CTexelNormalMethod : public CTexelMethod
{
	DECLARE_CLASS(CTexelNormalMethod, CTexelMethod);

public:
							CTexelNormalMethod(class CTexelGenerator* pGenerator);
	virtual 				~CTexelNormalMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer);

	virtual void			PostGenerate();
	void					Bleed();

	void					TexturizeValues(Vector* avecTexture);
	virtual bool			GenerateNormal(size_t& iGLId, size_t& iILId, bool bInMedias = false);

	virtual void			SaveToFile(const tstring& sFilename);

	virtual tstring	FileSuffix() { return "normal"; };
	virtual void*			GetData();

protected:
	Vector*					m_avecNormalValues;
	Vector*					m_avecNormalGeneratedValues;
};

// Accepts hi and lo res models, bakes the hi to the low res using any of a multitude of mix-and-matchable generators.
// For example, can make an ao + normal or just ao or just normal from hi to lo in one pass.
// All generation in CTexelGenerator works on a texel basis, each triangle is broken down into texels and computed one texel at a time
// by tracing to find the hi res mesh and then doing whatever calculations are needed for that method.
class CTexelGenerator
{
public:
							CTexelGenerator(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);
							~CTexelGenerator();

public:
	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetModels(const tvector<CConversionMeshInstance*>& apHiRes, const tvector<CConversionMeshInstance*>& apLoRes);

	void					ClearMethods();
	void					AddDiffuse();
	void					AddAO(size_t iSamples, bool bRandomize, float flRayFalloff, bool bGroundOcclusion, size_t iBleed);
	void					AddNormal();

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };
	IWorkListener*			GetWorkListener() { return m_pWorkListener; };

	void					Generate();
	void					GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, class raytrace::CRaytracer* pTracer, size_t& iRendered);
	void					FindHiResMeshLocation(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, size_t i, size_t j, raytrace::CRaytracer* pTracer);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);
	bool					Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask = NULL);

	CParallelizer*			GetParallelizer() { return m_pWorkParallelizer; }
	CConversionScene*		GetScene() { return m_pScene; }

	void					MarkTexelUsed(size_t iTexel) { m_abTexelMask[iTexel] = true; }

	size_t					GenerateDiffuse(bool bInMedias = false);
	size_t					GenerateAO(bool bInMedias = false);
	void					GenerateNormal(size_t& iGLId, size_t& iILId, bool bInMedias = false);

	void					SaveAll(const tstring& sFilename);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

	const tvector<CConversionMeshInstance*>&	GetHiResMeshInstances() { return m_apHiRes; }
	const tvector<CConversionMeshInstance*>&	GetLoResMeshInstances() { return m_apLoRes; }

protected:
	CConversionScene*		m_pScene;
	tvector<CMaterial>*		m_paoMaterials;

	tvector<CConversionMeshInstance*>	m_apHiRes;
	tvector<CConversionMeshInstance*>	m_apLoRes;

	tvector<CTexelMethod*>	m_apMethods;

	size_t					m_iWidth;
	size_t					m_iHeight;

	IWorkListener*			m_pWorkListener;

	bool*					m_abTexelMask;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	CParallelizer*			m_pWorkParallelizer;
};

typedef enum
{
	AOMETHOD_NONE = 0,
	AOMETHOD_RENDER,
	AOMETHOD_RAYTRACE,
	AOMETHOD_SHADOWMAP,
} aomethod_t;

class CAOGenerator
{
public:
							CAOGenerator(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);
							~CAOGenerator();

public:
	void					SetMethod(aomethod_t eMethod) { m_eAOMethod = eMethod; };
	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetUseTexture(bool bUseTexture) { m_bUseTexture = bUseTexture; };
	void					SetBleed(size_t iBleed) { m_iBleed = iBleed; };
	void					SetRenderPreviewViewport(int x, int y, int w, int h);
	void					SetUseFrontBuffer(bool bUseFrontBuffer) { m_bUseFrontBuffer = bUseFrontBuffer; };
	void					SetSamples(size_t iSamples) { m_iSamples = iSamples; };
	void					SetRandomize(bool bRandomize) { m_bRandomize = bRandomize; };
	void					SetCreaseEdges(bool bCreaseEdges) { m_bCreaseEdges = bCreaseEdges; };
	void					SetGroundOcclusion(bool bGroundOcclusion) { m_bGroundOcclusion = bGroundOcclusion; };
	void					SetRayFalloff(float flRayFalloff) { m_flRayFalloff = flRayFalloff; };

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };

	void					ShadowMapSetupScene();
	void					ShadowMapSetupSceneNode(CConversionSceneNode* pNode, class GLUtesselator* pTesselator, bool bDepth);
	void					RenderSetupScene();
	void					RenderSetupSceneNode(CConversionSceneNode* pNode, class GLUtesselator* pTesselator);
	void					Generate();
	void					GenerateShadowMaps();
	void					GenerateByTexel();
	void					GenerateNodeByTexel(CConversionSceneNode* pNode, class raytrace::CRaytracer* pTracer, size_t& iRendered);
	void					GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, class raytrace::CRaytracer* pTracer, size_t& iRendered);
	Vector					RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pFace);
	void					DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace);
	void					AccumulateTexture(size_t iTexture);
	void					Bleed();

	void					RaytraceSetupThreads();
	void					RaytraceCleanupThreads();
	void					RaytraceSceneMultithreaded(class raytrace::CRaytracer* pTracer, Vector vecPosition, Vector vecDirection, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel);
	void					RaytraceSceneFromPosition(class raytrace::CRaytracer* pTracer, Vector vecPosition, Vector vecDirection, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel);
	void					RaytraceJoinThreads();

	size_t					GenerateTexture(bool bInMedias = false);
	void					SaveToFile(const tchar* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

protected:
	CConversionScene*		m_pScene;
	tvector<CMaterial>*		m_paoMaterials;

	size_t					m_iWidth;
	size_t					m_iHeight;
	bool					m_bUseTexture;
	size_t					m_iBleed;
	size_t					m_iSamples;
	bool					m_bRandomize;
	bool					m_bCreaseEdges;
	bool					m_bGroundOcclusion;
	float					m_flRayFalloff;
	int						m_iRPVX;
	int						m_iRPVY;
	int						m_iRPVW;
	int						m_iRPVH;
	bool					m_bUseFrontBuffer;
	aomethod_t				m_eAOMethod;

	IWorkListener*			m_pWorkListener;

	size_t					m_iPixelDepth;
	float*					m_pPixels;
	bool*					m_bPixelMask;

	unsigned int			m_iSceneList;

	Vector*					m_avecShadowValues;
	Vector*					m_avecShadowGeneratedValues;
	size_t*					m_aiShadowReads;
	float					m_flLowestValue;
	float					m_flHighestValue;
	size_t					m_iAOFB;

	bool					m_bIsGenerating;
	bool					m_bIsBleeding;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	CParallelizer*			m_pRaytraceParallelizer;
};

class CNormalGenerator
{
public:
							CNormalGenerator(CConversionScene* pScene, tvector<CMaterial>* paoMaterials);
							~CNormalGenerator();

public:
	void					Think();

	void					SaveToFile(const tchar* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);
	bool					Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask = NULL);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating || m_aflNormal2Texels != NULL; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

	void					NormalizeHeightValue(size_t x, size_t y);
	void					GeneratePass(int x, int y);
	void					Setup();
	bool					IsSettingUp();
	bool					IsSetupComplete();
	float					GetSetupProgress();

	void					SetNormalTextureDepth(float flDepth) { m_flNormalTextureDepth = flDepth; };
	void					SetNormalTextureHiDepth(float flDepth) { m_flNormalTextureHiDepth = flDepth; };
	void					SetNormalTextureMidDepth(float flDepth) { m_flNormalTextureMidDepth = flDepth; };
	void					SetNormalTextureLoDepth(float flDepth) { m_flNormalTextureLoDepth = flDepth; };
	void					SetNormalTexture(bool bNormalTexture, size_t iMaterial);
	size_t					GetGenerationMaterial() { return m_iMaterial; }
	void					UpdateNormal2();
	void					StartGenerationJobs();
	void					RegenerateNormal2Texture();
	bool					IsNewNormal2Available();
	bool					IsGeneratingNewNormal2();
	float					GetNormal2GenerationProgress();
	void					GetNormalMap2(size_t& iNormal2, size_t& iNormal2IL);

protected:
	CConversionScene*		m_pScene;
	tvector<CMaterial>*		m_paoMaterials;

	IWorkListener*			m_pWorkListener;

	bool*					m_bPixelMask;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	size_t					m_iMaterial;
	float					m_flNormalTextureDepth;
	float					m_flNormalTextureHiDepth;
	float					m_flNormalTextureMidDepth;
	float					m_flNormalTextureLoDepth;
	size_t					m_iNormal2GLId;
	size_t					m_iNormal2ILId;
	size_t					m_iNormal2Width;
	size_t					m_iNormal2Height;
	float*					m_aflTextureTexels;
	float*					m_aflMidPassTexels;
	float*					m_aflLowPassTexels;
	float*					m_aflNormal2Texels;
	bool					m_bNewNormal2Available;
	bool					m_bNormal2Generated;
	bool					m_bNormal2Changed;

	CParallelizer*			m_pGenerationParallelizer;
	CParallelizer*			m_pNormal2Parallelizer;
};

#endif
