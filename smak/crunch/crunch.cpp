#include "crunch.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>

#include <raytracer/raytracer.h>

#if 0
#ifdef _DEBUG
#define NORMAL_DEBUG
#endif
#endif

#ifdef NORMAL_DEBUG
#include "ui/modelwindow.h"
#endif

CTexelGenerator::CTexelGenerator(CConversionScene* pScene, eastl::vector<CMaterial>* paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	m_abTexelMask = NULL;

	SetSize(512, 512);

	m_pWorkListener = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_pWorkParallelizer = NULL;
}

CTexelGenerator::~CTexelGenerator()
{
	free(m_abTexelMask);

	ClearMethods();
}

void CTexelGenerator::SetSize(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (m_abTexelMask)
		free(m_abTexelMask);
	m_abTexelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));

	for (size_t i = 0; i < m_apMethods.size(); i++)
		m_apMethods[i]->SetSize(iWidth, iHeight);
}

void CTexelGenerator::SetModels(const eastl::vector<CConversionMeshInstance*>& apHiRes, const eastl::vector<CConversionMeshInstance*>& apLoRes)
{
	m_apLoRes = apLoRes;
	m_apHiRes = apHiRes;
}

void CTexelGenerator::ClearMethods()
{
	for (size_t i = 0; i < m_apMethods.size(); i++)
		delete m_apMethods[i];

	m_apMethods.clear();
}

void CTexelGenerator::AddDiffuse()
{
	CTexelMethod* pMethod = new CTexelDiffuseMethod(this);
	pMethod->SetSize(m_iWidth, m_iHeight);
	m_apMethods.push_back(pMethod);
}

void CTexelGenerator::AddAO(size_t iSamples, bool bRandomize, float flRayFalloff, bool bGroundOcclusion, size_t iBleed)
{
	CTexelMethod* pMethod = new CTexelAOMethod(this, iSamples, bRandomize, flRayFalloff, bGroundOcclusion, iBleed);
	pMethod->SetSize(m_iWidth, m_iHeight);
	m_apMethods.push_back(pMethod);
}

void CTexelGenerator::AddNormal()
{
	CTexelMethod* pMethod = new CTexelNormalMethod(this);
	pMethod->SetSize(m_iWidth, m_iHeight);
	m_apMethods.push_back(pMethod);
}

typedef struct
{
	CTexelGenerator*			pGenerator;
	CConversionMeshInstance*	pMeshInstance;
	CConversionFace*			pFace;
	CConversionVertex*			pV1;
	CConversionVertex*			pV2;
	CConversionVertex*			pV3;
	size_t						x;
	size_t						y;
	raytrace::CRaytracer*		pTracer;
} texel_data_t;

void ComputeAtTexel(void* pVoidData)
{
	texel_data_t* pJobData = (texel_data_t*)pVoidData;

	pJobData->pGenerator->FindHiResMeshLocation(pJobData->pMeshInstance, pJobData->pFace, pJobData->pV1, pJobData->pV2, pJobData->pV3, pJobData->x, pJobData->y, pJobData->pTracer);
}

void CTexelGenerator::Generate()
{
	for (size_t i = 0; i < m_apMethods.size(); i++)
		m_apMethods[i]->PreGenerate();

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction("Building tree", 0);
	}

#ifdef _DEBUG
	if (CModelWindow::Get())
		CModelWindow::Get()->ClearDebugLines();
#endif

	memset(&m_abTexelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	m_bIsGenerating = true;
	m_bStopGenerating = false;
	m_bDoneGenerating = false;

	raytrace::CRaytracer* pTracer = NULL;

	pTracer = new raytrace::CRaytracer(m_pScene);

	for (size_t	m = 0; m < m_apHiRes.size(); m++)
		pTracer->AddMeshInstance(m_apHiRes[m]);

	pTracer->BuildTree();

	m_pWorkParallelizer = new CParallelizer((JobCallback)::ComputeAtTexel);
	m_pWorkParallelizer->Start();

	float flTotalArea = 0;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			flTotalArea += pFace->GetUVArea();
		}
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Dispatching jobs", (size_t)(flTotalArea*m_iWidth*m_iHeight));

	size_t iRendered = 0;

	eastl::vector<Vector> avecPoints;
	eastl::vector<size_t> aiPoints;

	for (size_t i = 0; i < m_apLoRes.size(); i++)
	{
		CConversionMeshInstance* pMeshInstance = m_apLoRes[i];

		if (!pMeshInstance->GetMesh()->GetNumUVs())
			continue;

		for (size_t f = 0; f < pMeshInstance->GetMesh()->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMeshInstance->GetMesh()->GetFace(f);

			if (pFace->m != ~0)
			{
				if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
					continue;

				CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
				if (pMaterial && !pMaterial->IsVisible())
					continue;
			}

			avecPoints.clear();
			aiPoints.clear();
			for (size_t t = 0; t < pFace->GetNumVertices(); t++)
			{
				avecPoints.push_back(pMeshInstance->GetVertex(pFace->GetVertex(t)->v));
				aiPoints.push_back(t);
			}

			while (avecPoints.size() > 3)
			{
				size_t iEar = FindEar(avecPoints);
				size_t iLast = iEar==0?avecPoints.size()-1:iEar-1;
				size_t iNext = iEar==avecPoints.size()-1?0:iEar+1;
				GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[iLast], aiPoints[iEar], aiPoints[iNext], pTracer, iRendered);
				avecPoints.erase(avecPoints.begin()+iEar);
				aiPoints.erase(aiPoints.begin()+iEar);
				if (m_bStopGenerating)
					break;
			}
			GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[0], aiPoints[1], aiPoints[2], pTracer, iRendered);
			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}

	m_pWorkParallelizer->FinishJobs();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Rendering", m_pWorkParallelizer->GetJobsTotal());

	while (true)
	{
		if (m_pWorkParallelizer->AreAllJobsDone())
			break;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(m_pWorkParallelizer->GetJobsDone());

		if (m_bStopGenerating)
			break;
	}

	delete m_pWorkParallelizer;

	delete pTracer;

	for (size_t i = 0; i < m_apMethods.size(); i++)
		m_apMethods[i]->PostGenerate();

	if (!m_bStopGenerating)
		m_bDoneGenerating = true;
	m_bIsGenerating = false;

	// One last call to let them know we're done.
	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CTexelGenerator::GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	texel_data_t oJob;

	CConversionVertex* pV1 = pFace->GetVertex(v1);
	CConversionVertex* pV2 = pFace->GetVertex(v2);
	CConversionVertex* pV3 = pFace->GetVertex(v3);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	oJob.pMeshInstance = pMeshInstance;
	oJob.pFace = pFace;
	oJob.pV1 = pV1;
	oJob.pV2 = pV2;
	oJob.pV3 = pV3;
	oJob.pTracer = pTracer;
	oJob.pGenerator = this;

	Vector vu1 = pMesh->GetUV(pV1->vu);
	Vector vu2 = pMesh->GetUV(pV2->vu);
	Vector vu3 = pMesh->GetUV(pV3->vu);

	Vector vecLoUV = vu1;
	Vector vecHiUV = vu1;

	if (vu2.x < vecLoUV.x)
		vecLoUV.x = vu2.x;
	if (vu3.x < vecLoUV.x)
		vecLoUV.x = vu3.x;
	if (vu2.x > vecHiUV.x)
		vecHiUV.x = vu2.x;
	if (vu3.x > vecHiUV.x)
		vecHiUV.x = vu3.x;

	if (vu2.y < vecLoUV.y)
		vecLoUV.y = vu2.y;
	if (vu3.y < vecLoUV.y)
		vecLoUV.y = vu3.y;
	if (vu2.y > vecHiUV.y)
		vecHiUV.y = vu2.y;
	if (vu3.y > vecHiUV.y)
		vecHiUV.y = vu3.y;

	size_t iLoX = (size_t)(vecLoUV.x * m_iWidth);
	size_t iLoY = (size_t)(vecLoUV.y * m_iHeight);
	size_t iHiX = (size_t)(vecHiUV.x * m_iWidth);
	size_t iHiY = (size_t)(vecHiUV.y * m_iHeight);

	for (size_t i = iLoX; i <= iHiX; i++)
	{
		for (size_t j = iLoY; j <= iHiY; j++)
		{
			oJob.x = i;
			oJob.y = j;

			m_pWorkParallelizer->AddJob(&oJob, sizeof(oJob));

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(++iRendered);

			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

void CTexelGenerator::FindHiResMeshLocation(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, size_t i, size_t j, raytrace::CRaytracer* pTracer)
{
	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vu1 = pMesh->GetUV(pV1->vu);
	Vector vu2 = pMesh->GetUV(pV2->vu);
	Vector vu3 = pMesh->GetUV(pV3->vu);

	float flU = ((float)i + 0.5f)/(float)m_iWidth;
	float flV = ((float)j + 0.5f)/(float)m_iHeight;

	bool bInside = PointInTriangle(Vector(flU,flV,0), vu1, vu2, vu3);

	if (!bInside)
		return;

	Vector v1 = pMeshInstance->GetVertex(pV1->v);
	Vector v2 = pMeshInstance->GetVertex(pV2->v);
	Vector v3 = pMeshInstance->GetVertex(pV3->v);

	// Find where the UV is in world space.

	// First build 2x2 a "matrix" of the UV values.
	float mta = vu2.x - vu1.x;
	float mtb = vu3.x - vu1.x;
	float mtc = vu2.y - vu1.y;
	float mtd = vu3.y - vu1.y;

	// Invert it.
	float d = mta*mtd - mtb*mtc;
	float mtia =  mtd / d;
	float mtib = -mtb / d;
	float mtic = -mtc / d;
	float mtid =  mta / d;

	// Now build a 2x3 "matrix" of the vertices.
	float mva = v2.x - v1.x;
	float mvb = v3.x - v1.x;
	float mvc = v2.y - v1.y;
	float mvd = v3.y - v1.y;
	float mve = v2.z - v1.z;
	float mvf = v3.z - v1.z;

	// Multiply them together.
	// [a b]   [a b]   [a b]
	// [c d] * [c d] = [c d]
	// [e f]           [e f]
	// Really wish I had a matrix math library about now!
	float mra = mva*mtia + mvb*mtic;
	float mrb = mva*mtib + mvb*mtid;
	float mrc = mvc*mtia + mvd*mtic;
	float mrd = mvc*mtib + mvd*mtid;
	float mre = mve*mtia + mvf*mtic;
	float mrf = mve*mtib + mvf*mtid;

	// These vectors should be the U and V axis in world space.
	Vector vecUAxis(mra, mrc, mre);
	Vector vecVAxis(mrb, mrd, mrf);

	Vector vecUVOrigin = v1 - vecUAxis * vu1.x - vecVAxis * vu1.y;

	Vector vecUVPosition = vecUVOrigin + vecUAxis * flU + vecVAxis * flV;

	Vector vecNormal = pFace->GetNormal(vecUVPosition, pMeshInstance);

	size_t iTexel;
	Texel(i, j, iTexel, false);

	// Maybe use a closest-poly check here to eliminate the need for some raytracing?

	raytrace::CTraceResult trFront;
	bool bHitFront = pTracer->Raytrace(Ray(vecUVPosition, vecNormal), &trFront);

	raytrace::CTraceResult trBack;
	bool bHitBack = pTracer->Raytrace(Ray(vecUVPosition, -vecNormal), &trBack);

#ifdef NORMAL_DEBUG
	if (bHitFront && (vecUVPosition - trFront.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(vecUVPosition, trFront.m_vecHit);
	if (bHitBack && (vecUVPosition - trBack.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(vecUVPosition, trBack.m_vecHit);
#endif

	if (!bHitBack && !bHitFront)
		return;

	raytrace::CTraceResult* trFinal;

	if (bHitFront && !bHitBack)
		trFinal = &trFront;
	else if (bHitBack && !bHitFront)
		trFinal = &trBack;
	else
	{
		float flHitFront = (vecUVPosition - trFront.m_vecHit).LengthSqr();
		float flHitBack = (vecUVPosition - trBack.m_vecHit).LengthSqr();

		if (flHitFront < flHitBack)
			trFinal = &trFront;
		else
			trFinal = &trBack;
	}

#ifdef NORMAL_DEBUG
//	CModelWindow::Get()->AddDebugLine(vecUVPosition, vecUVPosition+vecHitNormal);
	if (bHitFront && (vecUVPosition - trFront.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(trFront.m_vecHit, trFront.m_vecHit + trFront.m_pFace->GetNormal(trFront.m_vecHit, trFront.m_pMeshInstance));
	if (bHitBack && (vecUVPosition - trBack.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(trBack.m_vecHit, trBack.m_vecHit + trBack.m_pFace->GetNormal(trBack.m_vecHit, trBack.m_pMeshInstance));
#endif

	for (size_t i = 0; i < m_apMethods.size(); i++)
	{
		m_apMethods[i]->GenerateTexel(iTexel, pMeshInstance, pFace, pV1, pV2, pV3, trFinal, vecUVPosition, pTracer);
	}
}

bool CTexelGenerator::Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask)
{
	if (w < 0 || h < 0 || w >= tw || h >= th)
		return false;

	iTexel = th*h + w;

	TAssert(iTexel >= 0 && iTexel < tw * th);

	if (abMask && !abMask[iTexel])
		return false;

	return true;
}

bool CTexelGenerator::Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask)
{
	return Texel(w, h, iTexel, m_iWidth, m_iHeight, bUseMask?m_abTexelMask:NULL);
}

size_t CTexelGenerator::GenerateDiffuse(bool bInMedias)
{
	for (size_t i = 0; i < m_apMethods.size(); i++)
	{
		size_t iTexture = m_apMethods[i]->GenerateDiffuse(bInMedias);
		if (iTexture)
			return iTexture;
	}

	return 0;
}

size_t CTexelGenerator::GenerateAO(bool bInMedias)
{
	for (size_t i = 0; i < m_apMethods.size(); i++)
	{
		size_t iAO = m_apMethods[i]->GenerateAO(bInMedias);
		if (iAO)
			return iAO;
	}

	return 0;
}

void CTexelGenerator::GenerateNormal(size_t& iGLId, size_t& iILId, bool bInMedias)
{
	for (size_t i = 0; i < m_apMethods.size(); i++)
	{
		if (m_apMethods[i]->GenerateNormal(iGLId, iILId, bInMedias))
			return;
	}
}

void CTexelGenerator::SaveAll(const tstring& sFilename)
{
	ilEnable(IL_FILE_OVERWRITE);

	for (size_t i = 0; i < m_apMethods.size(); i++)
		m_apMethods[i]->SaveToFile(sFilename);
}

CTexelMethod::CTexelMethod(CTexelGenerator* pGenerator)
{
	m_pGenerator = pGenerator;
}

CTexelMethod::~CTexelMethod()
{
}

void CTexelMethod::SetSize(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

void CTexelMethod::SaveToFile(const tstring& sFilename)
{
	tstring sRealFilename = sFilename.substr(0, sFilename.length()-4) + "-" + FileSuffix() + sFilename.substr(sFilename.length()-4, 4);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, GetData());

	// Formats like PNG and VTF don't work unless it's in integer format.
	ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

	if (!ModelWindow()->IsRegistered() && (m_iWidth > 128 || m_iHeight > 128))
	{
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(128, 128, 1);
	}

	ilSaveImage(convertstring<tchar, ILchar>(sRealFilename).c_str());

	ilDeleteImages(1,&iDevILId);
}

CTexelDiffuseMethod::CTexelDiffuseMethod(CTexelGenerator* pGenerator)
	: CTexelMethod(pGenerator)
{
	m_avecDiffuseValues = NULL;
	m_avecDiffuseGeneratedValues = NULL;
	m_aiDiffuseReads = NULL;
}

CTexelDiffuseMethod::~CTexelDiffuseMethod()
{
	delete[] m_avecDiffuseValues;
	delete[] m_avecDiffuseGeneratedValues;
	delete[] m_aiDiffuseReads;
}

void CTexelDiffuseMethod::SetSize(size_t iWidth, size_t iHeight)
{
	BaseClass::SetSize(iWidth, iHeight);

	if (m_avecDiffuseValues)
	{
		delete[] m_avecDiffuseValues;
		delete[] m_avecDiffuseGeneratedValues;
		delete[] m_aiDiffuseReads;
	}

	// Shadow volume result buffer.
	m_avecDiffuseValues = new Vector[iWidth*iHeight];
	m_avecDiffuseGeneratedValues = new Vector[iWidth*iHeight];
	m_aiDiffuseReads = new size_t[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_avecDiffuseValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_avecDiffuseGeneratedValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_aiDiffuseReads[0], 0, iWidth*iHeight*sizeof(size_t));
}

void CTexelDiffuseMethod::PreGenerate()
{
	const eastl::vector<CConversionMeshInstance*>& apHiRes = m_pGenerator->GetHiResMeshInstances();

	m_aiTextures.resize(m_pGenerator->GetScene()->GetNumMaterials());
	for (size_t i = 0; i < m_aiTextures.size(); i++)
		m_aiTextures[i] = 0;

	for (size_t i = 0; i < apHiRes.size(); i++)
	{
		CConversionMeshInstance* pMeshInstance = apHiRes[i];

		for (eastl::map<size_t, CConversionMaterialMap>::iterator j = pMeshInstance->m_aiMaterialsMap.begin(); j != pMeshInstance->m_aiMaterialsMap.end(); j++)
		{
			size_t iMaterial = pMeshInstance->GetMappedMaterial(j->first)->m_iMaterial;

			if (m_aiTextures[iMaterial])
				continue;

			CConversionMaterial* pMaterial = m_pGenerator->GetScene()->GetMaterial(iMaterial);

			m_aiTextures[iMaterial] = CModelWindow::LoadTexture(pMaterial->GetDiffuseTexture());
		}
	}
}

void CTexelDiffuseMethod::GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, raytrace::CTraceResult* tr, const Vector& vecUVPosition, raytrace::CRaytracer* pTracer)
{
	size_t iTexture = m_aiTextures[tr->m_pMeshInstance->GetMappedMaterial(tr->m_pFace->m)->m_iMaterial];

	if (!iTexture)
		return;

	CConversionMesh* pHitMesh = tr->m_pMeshInstance->GetMesh();

	CConversionVertex* pHitV1 = tr->m_pFace->GetVertex(0);
	CConversionVertex* pHitV2 = tr->m_pFace->GetVertex(1);
	CConversionVertex* pHitV3 = tr->m_pFace->GetVertex(2);

	Vector2D vu1 = pHitMesh->GetUV(pHitV1->vu);
	Vector2D vu2 = pHitMesh->GetUV(pHitV2->vu);
	Vector2D vu3 = pHitMesh->GetUV(pHitV3->vu);

	Vector v1 = tr->m_pMeshInstance->GetVertex(pHitV1->v);
	Vector v2 = tr->m_pMeshInstance->GetVertex(pHitV2->v);
	Vector v3 = tr->m_pMeshInstance->GetVertex(pHitV3->v);

	// Find where the UV is in world space.

	// First build 2x2 a "matrix" of the UV values.
	float mta = vu2.x - vu1.x;
	float mtb = vu3.x - vu1.x;
	float mtc = vu2.y - vu1.y;
	float mtd = vu3.y - vu1.y;

	// Now build a 2x3 "matrix" of the vertices.
	float mva = v2.x - v1.x;
	float mvb = v3.x - v1.x;
	float mvc = v2.y - v1.y;
	float mvd = v3.y - v1.y;
	float mve = v2.z - v1.z;
	float mvf = v3.z - v1.z;

	// Multiply them together.
	// [a b]   [a b c]   [a b c]
	// [c d] * [d e f] = [d e f]
	// Really wish I had a matrix math library about now!
	float mra = mta*mva + mtb*mvd;
	float mrb = mta*mvb + mtb*mve;
	float mrc = mta*mvc + mtb*mvf;
	float mrd = mtc*mva + mtd*mvd;
	float mre = mtc*mvb + mtd*mve;
	float mrf = mtc*mvc + mtd*mvf;

	// These vectors should be the X Y and Z axis in UV space.
	Vector2D vecXAxis(mra, mrd);
	Vector2D vecYAxis(mrb, mre);
	Vector2D vecZAxis(mrc, mrf);

	// The world origin in (u, v) texture space
	Vector2D vecWorldOrigin = vu1 - vecXAxis * v1.x - vecYAxis * v1.y - vecZAxis * v1.z;

	// The position of the traceline's hit in (u, v) texture space
	Vector2D vecWorldPosition = vecWorldOrigin + vecXAxis * tr->m_vecHit.x + vecYAxis * tr->m_vecHit.y + vecZAxis * tr->m_vecHit.z;

	// Mutex may be dead, try to bail before.
	if (m_pGenerator->IsStopped())
		return;

	// Lock for il texture access so multiple threads don't bind different textures and confuse each other.
	m_pGenerator->GetParallelizer()->LockData();

	ilBindImage(iTexture);

	size_t iU = (size_t)((vecWorldPosition.x * ilGetInteger(IL_IMAGE_WIDTH)) - 0.5f);
	size_t iV = (size_t)((vecWorldPosition.y * ilGetInteger(IL_IMAGE_HEIGHT)) - 0.5f);

	iU %= ilGetInteger(IL_IMAGE_WIDTH);
	iV %= ilGetInteger(IL_IMAGE_HEIGHT);

	size_t iColorTexel = iU + iV*ilGetInteger(IL_IMAGE_HEIGHT);
	Color* pclrData = ((Color*)ilGetData());
	Color clrData = pclrData[iColorTexel];

	ilBindImage(0);

	m_avecDiffuseValues[iTexel] += Vector(clrData);
	m_aiDiffuseReads[iTexel]++;
	m_pGenerator->MarkTexelUsed(iTexel);

	m_pGenerator->GetParallelizer()->UnlockData();
}

void CTexelDiffuseMethod::PostGenerate()
{
	for (size_t i = 0; i < m_aiTextures.size(); i++)
	{
		if (m_aiTextures[i])
			ilDeleteImages(1, &m_aiTextures[i]);
	}

	if (m_pGenerator->GetWorkListener())
		m_pGenerator->GetWorkListener()->SetAction("Averaging reads", m_iWidth*m_iHeight);

	// Average out all of the reads.
	for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
	{
		// Don't immediately return, just skip this loop. We have cleanup work to do.
		if (m_pGenerator->IsStopped())
			break;

		if (m_aiDiffuseReads[i])
			m_avecDiffuseValues[i] /= (float)m_aiDiffuseReads[i];
		else
			m_avecDiffuseValues[i] = Vector(0,0,0);

		// When exporting to png sometimes a pure white value will suffer integer overflow.
		m_avecDiffuseValues[i] *= 0.99f;

		if (m_pGenerator->GetWorkListener())
			m_pGenerator->GetWorkListener()->WorkProgress(i);
	}

	if (!m_pGenerator->IsStopped())
		Bleed();
}

void CTexelDiffuseMethod::Bleed()
{
	if (m_pGenerator->GetWorkListener())
		m_pGenerator->GetWorkListener()->SetAction("Bleeding edges", 0);

	for (size_t w = 0; w < m_iWidth; w++)
	{
		for (size_t h = 0; h < m_iHeight; h++)
		{
			Vector vecTotal(0,0,0);
			size_t iTotal = 0;
			size_t iTexel;

			// If the texel has the mask on then it already has a value so skip it.
			if (m_pGenerator->Texel(w, h, iTexel, true))
				continue;

			if (m_pGenerator->Texel(w-1, h-1, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w-1, h, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w-1, h+1, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w, h+1, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w+1, h+1, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w+1, h, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w+1, h-1, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			if (m_pGenerator->Texel(w, h-1, iTexel))
			{
				vecTotal += m_avecDiffuseValues[iTexel];
				iTotal++;
			}

			m_pGenerator->Texel(w, h, iTexel, false);

			if (iTotal)
			{
				vecTotal /= (float)iTotal;
				m_avecDiffuseValues[iTexel] = vecTotal;
			}
		}
	}
}

size_t CTexelDiffuseMethod::GenerateDiffuse(bool bInMedias)
{
	Vector* avecDiffuseValues = m_avecDiffuseValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		avecDiffuseValues = m_avecDiffuseGeneratedValues;

		// Average out all of the reads.
		for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
		{
			// Don't immediately return, just skip this loop. We have cleanup work to do.
			if (m_pGenerator->IsStopped())
				break;

			if (!m_aiDiffuseReads[i])
			{
				avecDiffuseValues[i] = Vector(0,0,0);
				continue;
			}

			avecDiffuseValues[i] = m_avecDiffuseValues[i] / (float)m_aiDiffuseReads[i];

			// When exporting to png sometimes a pure white value will suffer integer overflow.
			avecDiffuseValues[i] *= 0.99f;
		}
	}

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iWidth, (GLint)m_iHeight, GL_RGB, GL_FLOAT, &avecDiffuseValues[0].x);

	return iGLId;
}

void* CTexelDiffuseMethod::GetData()
{
	return &m_avecDiffuseValues[0].x;
}

CTexelAOMethod::CTexelAOMethod(CTexelGenerator* pGenerator, size_t iSamples, bool bRandomize, float flRayFalloff, bool bGroundOcclusion, size_t iBleed)
	: CTexelMethod(pGenerator)
{
	m_avecShadowValues = NULL;
	m_avecShadowGeneratedValues = NULL;
	m_aiShadowReads = NULL;

	m_iSamples = iSamples;
	m_bRandomize = bRandomize;
	m_flRayFalloff = flRayFalloff;
	m_bGroundOcclusion = bGroundOcclusion;
	m_iBleed = iBleed;
}

CTexelAOMethod::~CTexelAOMethod()
{
	delete[] m_avecShadowValues;
	delete[] m_avecShadowGeneratedValues;
	delete[] m_aiShadowReads;
}

void CTexelAOMethod::SetSize(size_t iWidth, size_t iHeight)
{
	BaseClass::SetSize(iWidth, iHeight);

	if (m_avecShadowValues)
	{
		delete[] m_avecShadowValues;
		delete[] m_avecShadowGeneratedValues;
		delete[] m_aiShadowReads;
	}

	// Shadow volume result buffer.
	m_avecShadowValues = new Vector[iWidth*iHeight];
	m_avecShadowGeneratedValues = new Vector[iWidth*iHeight];
	m_aiShadowReads = new size_t[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_avecShadowValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_avecShadowGeneratedValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_aiShadowReads[0], 0, iWidth*iHeight*sizeof(size_t));
}

void CTexelAOMethod::GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, raytrace::CTraceResult* tr, const Vector& vecUVPosition, raytrace::CRaytracer* pTracer)
{
	Vector vecHitNormal = tr->m_pFace->GetNormal(tr->m_vecHit, tr->m_pMeshInstance);

	// Build rotation matrix
	Matrix4x4 m;
	m.SetOrientation(vecHitNormal);

	// Turn it sideways so that pitch 90 is up
	Matrix4x4 m2;
	m2.SetAngles(EAngle(0, 0, -90));

	m *= m2;

	//ModelWindow()->AddDebugLine(vecUVPosition + pFace->GetNormal()*0.01f, vecUVPosition + vecNormal*0.5f, Color(0, 0, 255));

	float flHits = 0;
	float flTotalHits = 0;

	for (size_t x = 0; x < m_iSamples/2; x++)
	{
		float flRandom = 0;
		if (m_bRandomize)
			flRandom = RemapVal((float)(rand()%10000), 0, 10000.0f, -0.5f, 0.5f);

		float flPitch = RemapVal(cos(RemapVal((float)x+flRandom, 0, (float)m_iSamples/2, 0, M_PI/2)), 0, 1, 90, 0);

		float flWeight = sin(flPitch * M_PI/180);

		for (size_t y = 0; y <= m_iSamples; y++)
		{
			flRandom = 0;
			if (m_bRandomize)
				flRandom = RemapVal((float)(rand()%10000), 0, 10000.0f, -0.5f, 0.5f);

			float flYaw = RemapVal((float)y+flRandom, 0, (float)m_iSamples, -180, 180);

			Vector vecDir = AngleVector(EAngle(flPitch, flYaw, 0));

			// Transform relative to the triangle's normal
			Vector vecRay = m * vecDir;

			flTotalHits += flWeight;

			//ModelWindow()->AddDebugLine(vecUVPosition + pFace->GetNormal()*0.01f, vecUVPosition + vecRay.Normalized()*0.1f, vecDir);

			raytrace::CTraceResult tr2;
			if (pTracer->Raytrace(Ray(tr->m_vecHit + vecHitNormal*0.01f, vecRay), &tr2))
			{
				float flDistance = (tr2.m_vecHit - tr->m_vecHit).Length();
				if (m_flRayFalloff < 0)
					flHits += flWeight;
				else
					flHits += flWeight * (1/pow(2, flDistance/m_flRayFalloff));
			}
			else if (m_bGroundOcclusion && vecRay.y < 0)
			{
				// The following math is basically a plane-ray intersection algorithm,
				// with shortcuts made for the assumption of an infinite plane facing straight up.

				Vector n = Vector(0,1,0);

				float a = -(vecUVPosition.y - pMeshInstance->m_pParent->m_oExtends.m_vecMins.y);
				float b = vecRay.y;

				float flDistance = a/b;

				if (flDistance < 1e-4f || m_flRayFalloff < 0)
					flHits += flWeight;
				else
					flHits += flWeight * (1/pow(2, flDistance/m_flRayFalloff));
			}
		}
	}

	// One last ray directly up, it is skipped in the above loop so it's not done 10 times.
	Vector vecDir = AngleVector(EAngle(90, 0, 0));

	// Transform relative to the triangle's normal
	Vector vecRay = m * vecDir;

	//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

	flTotalHits++;

	//ModelWindow()->AddDebugLine(vecUVPosition + pFace->GetNormal()*0.01f, vecUVPosition + vecRay.Normalized()*0.2f, vecDir);

	raytrace::CTraceResult tr2;
	if (pTracer->Raytrace(Ray(tr->m_vecHit + vecHitNormal*0.01f, vecRay), &tr2))
	{
		float flDistance = (tr2.m_vecHit - tr->m_vecHit).Length();
		if (m_flRayFalloff < 0)
			flHits += 1;
		else
			flHits += (1/pow(2, flDistance/m_flRayFalloff));
	}
	else if (m_bGroundOcclusion && vecRay.y < 0)
	{
		// The following math is basically a plane-ray intersection algorithm,
		// with shortcuts made for the assumption of an infinite plane facing straight up.
		float a = -(tr->m_vecHit.y - pMeshInstance->m_pParent->m_oExtends.m_vecMins.y);
		float b = vecRay.y;

		float flDistance = a/b;

		if (flDistance < 1e-4f || m_flRayFalloff < 0)
			flHits += 1;
		else
			flHits += (1/pow(2, flDistance/m_flRayFalloff));
	}

	float flShadowValue = 1 - ((float)flHits / (float)flTotalHits);

	// Mutex may be dead, try to bail before.
	if (m_pGenerator->IsStopped())
		return;

	m_pGenerator->GetParallelizer()->LockData();

	m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);
	m_aiShadowReads[iTexel]++;
	m_pGenerator->MarkTexelUsed(iTexel);

	m_pGenerator->GetParallelizer()->UnlockData();
}

void CTexelAOMethod::PostGenerate()
{
	size_t i;

	if (m_pGenerator->GetWorkListener())
		m_pGenerator->GetWorkListener()->SetAction("Averaging reads", m_iWidth*m_iHeight);

	// Average out all of the reads.
	for (i = 0; i < m_iWidth*m_iHeight; i++)
	{
		// Don't immediately return, just skip this loop. We have cleanup work to do.
		if (m_pGenerator->IsStopped())
			break;

		if (m_aiShadowReads[i])
			m_avecShadowValues[i] /= (float)m_aiShadowReads[i];
		else
			m_avecShadowValues[i] = Vector(0,0,0);

		// When exporting to png sometimes a pure white value will suffer integer overflow.
		m_avecShadowValues[i] *= 0.99f;

		if (m_pGenerator->GetWorkListener())
			m_pGenerator->GetWorkListener()->WorkProgress(i);
	}

	// Somebody get this ao some clotters and morphine, STAT!
	if (!m_pGenerator->IsStopped())
		Bleed();
}

void CTexelAOMethod::Bleed()
{
	bool* abPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));

	if (m_pGenerator->GetWorkListener())
		m_pGenerator->GetWorkListener()->SetAction("Bleeding edges", m_iBleed);

	for (size_t i = 0; i < m_iBleed; i++)
	{
		// This is for pixels that have been set this frame.
		memset(&abPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

		for (size_t w = 0; w < m_iWidth; w++)
		{
			for (size_t h = 0; h < m_iHeight; h++)
			{
				Vector vecTotal(0,0,0);
				size_t iTotal = 0;
				size_t iTexel;

				// If the texel has the mask on then it already has a value so skip it.
				if (m_pGenerator->Texel(w, h, iTexel, true))
					continue;

				if (m_pGenerator->Texel(w-1, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w-1, h, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w-1, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w+1, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w+1, h, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w+1, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (m_pGenerator->Texel(w, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				m_pGenerator->Texel(w, h, iTexel, false);

				if (iTotal)
				{
					vecTotal /= (float)iTotal;
					m_avecShadowValues[iTexel] = vecTotal;
					abPixelMask[iTexel] = true;
				}
			}
		}

		for (size_t p = 0; p < m_iWidth*m_iHeight; p++)
		{
			if (abPixelMask[p])
				m_pGenerator->MarkTexelUsed(p);
		}

		if (m_pGenerator->GetWorkListener())
			m_pGenerator->GetWorkListener()->WorkProgress(i);

		if (m_pGenerator->IsStopped())
			break;
	}

	free(abPixelMask);
}

size_t CTexelAOMethod::GenerateAO(bool bInMedias)
{
	Vector* avecShadowValues = m_avecShadowValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		avecShadowValues = m_avecShadowGeneratedValues;

		// Average out all of the reads.
		for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
		{
			// Don't immediately return, just skip this loop. We have cleanup work to do.
			if (m_pGenerator->IsStopped())
				break;

			if (!m_aiShadowReads[i])
			{
				avecShadowValues[i] = Vector(0,0,0);
				continue;
			}

			avecShadowValues[i] = m_avecShadowValues[i] / (float)m_aiShadowReads[i];

			// When exporting to png sometimes a pure white value will suffer integer overflow.
			avecShadowValues[i] *= 0.99f;
		}
	}

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iWidth, (GLint)m_iHeight, GL_RGB, GL_FLOAT, &avecShadowValues[0].x);

	return iGLId;
}

void* CTexelAOMethod::GetData()
{
	return &m_avecShadowValues[0].x;
}

CTexelNormalMethod::CTexelNormalMethod(CTexelGenerator* pGenerator)
	: CTexelMethod(pGenerator)
{
	m_avecNormalValues = NULL;
	m_avecNormalGeneratedValues = NULL;
}

CTexelNormalMethod::~CTexelNormalMethod()
{
	delete[] m_avecNormalValues;
	delete[] m_avecNormalGeneratedValues;
}

void CTexelNormalMethod::SetSize(size_t iWidth, size_t iHeight)
{
	BaseClass::SetSize(iWidth, iHeight);

	if (m_avecNormalValues)
	{
		delete[] m_avecNormalValues;
		delete[] m_avecNormalGeneratedValues;
	}

	// Shadow volume result buffer.
	m_avecNormalValues = new Vector[iWidth*iHeight];
	m_avecNormalGeneratedValues = new Vector[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_avecNormalValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_avecNormalGeneratedValues[0], 0, iWidth*iHeight*sizeof(Vector));
}

void CTexelNormalMethod::GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, raytrace::CTraceResult* tr, const Vector& vecUVPosition, raytrace::CRaytracer* pTracer)
{
	Vector vecHitNormal = tr->m_pFace->GetNormal(tr->m_vecHit, tr->m_pMeshInstance);

	// Build rotation matrix
	Matrix4x4 mObjectToTangent;

	Vector t = pFace->GetBaseVector(vecUVPosition, 0, pMeshInstance);
	Vector b = pFace->GetBaseVector(vecUVPosition, 1, pMeshInstance);
	Vector n = pFace->GetBaseVector(vecUVPosition, 2, pMeshInstance);

	mObjectToTangent.SetColumn(0, t);
	mObjectToTangent.SetColumn(1, b);
	mObjectToTangent.SetColumn(2, n);
	mObjectToTangent.InvertRT();

	Vector vecTangentNormal = mObjectToTangent*vecHitNormal;

	m_pGenerator->GetParallelizer()->LockData();

	m_avecNormalValues[iTexel] += vecTangentNormal;
	m_pGenerator->MarkTexelUsed(iTexel);

	m_pGenerator->GetParallelizer()->UnlockData();
}

void CTexelNormalMethod::PostGenerate()
{
	Bleed();
	TexturizeValues(m_avecNormalValues);
}

void CTexelNormalMethod::Bleed()
{
	bool* abPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));

	if (m_pGenerator->GetWorkListener())
		m_pGenerator->GetWorkListener()->SetAction("Bleeding edges", 0);

	// This is for pixels that have been set this frame.
	memset(&abPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	for (size_t w = 0; w < m_iWidth; w++)
	{
		for (size_t h = 0; h < m_iHeight; h++)
		{
			Vector vecTotal(0,0,0);
			size_t iTexel;

			// If the texel has the mask on then it already has a value so skip it.
			if (m_pGenerator->Texel(w, h, iTexel, true))
				continue;

			bool bTotal = false;

			if (m_pGenerator->Texel(w-1, h-1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w-1, h, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w-1, h+1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w, h+1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w+1, h+1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w+1, h, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w+1, h-1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (m_pGenerator->Texel(w, h-1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			m_pGenerator->Texel(w, h, iTexel, false);

			if (bTotal)
			{
				m_avecNormalValues[iTexel] = vecTotal;
				m_avecNormalValues[iTexel].Normalize();
				abPixelMask[iTexel] = true;
			}
		}
	}

	for (size_t p = 0; p < m_iWidth*m_iHeight; p++)
	{
		if (abPixelMask[p])
			m_pGenerator->MarkTexelUsed(p);
	}

	free(abPixelMask);
}

void CTexelNormalMethod::TexturizeValues(Vector* avecTexture)
{
	for (size_t x = 0; x < m_iWidth; x++)
	{
		for (size_t y = 0; y < m_iHeight; y++)
		{
			size_t iTexel;

			m_pGenerator->Texel(x, y, iTexel, false);

			avecTexture[iTexel] = m_avecNormalValues[iTexel].Normalized()*0.99f/2 + Vector(0.5f, 0.5f, 0.5f);
		}
	}
}

bool CTexelNormalMethod::GenerateNormal(size_t& iGLId, size_t& iILId, bool bInMedias)
{
	Vector* avecNormalValues = m_avecNormalValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		avecNormalValues = m_avecNormalGeneratedValues;
		TexturizeValues(avecNormalValues);
	}

	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iWidth, (GLint)m_iHeight, GL_RGB, GL_FLOAT, &avecNormalValues[0].x);

	if (!bInMedias)
	{
		ilGenImages(1, &iILId);
		ilBindImage(iILId);
		ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, &avecNormalValues[0].x);
		ilConvertImage(IL_RGB, IL_UNSIGNED_INT);
	}

	return true;
}

void CTexelNormalMethod::SaveToFile(const tstring& sFilename)
{
	tstring sRealFilename = sFilename.substr(0, sFilename.length()-4) + "-" + FileSuffix() + sFilename.substr(sFilename.length()-4, 4);

	eastl::vector<bool> abMaterialSaved((bool)false);
	abMaterialSaved.resize(m_pGenerator->GetScene()->GetNumMaterials());

	for (size_t i = 0; i < m_pGenerator->GetLoResMeshInstances().size(); i++)
	{
		CConversionMeshInstance* pMeshInstance = m_pGenerator->GetLoResMeshInstances()[i];
		for (eastl::map<size_t, CConversionMaterialMap>::iterator j = pMeshInstance->m_aiMaterialsMap.begin(); j != pMeshInstance->m_aiMaterialsMap.end(); j++)
		{
			size_t iMaterial = pMeshInstance->GetMappedMaterial(j->first)->m_iMaterial;

			if (abMaterialSaved[iMaterial])
				continue;

			ModelWindow()->SaveNormal(iMaterial, sRealFilename);

			abMaterialSaved[iMaterial] = true;
		}
	}
}

void* CTexelNormalMethod::GetData()
{
	return &m_avecNormalValues[0].x;
}
