#include "models.h"

#include <stdio.h>

#include <tinker_platform.h>
#include <files.h>

#include <modelconverter/modelconverter.h>
#include <renderer/renderer.h>
#include <textures/materiallibrary.h>
#include <tinker/application.h>
#include <datamanager/data.h>

tvector<tvector<float> >	g_aaflData;
tvector<float>				g_aflWireframeData;
AABB						g_aabbBounds;

void AddVertex(size_t iMaterial, const Vector& v, const Vector& vn, const Vector2D& vt)
{
	g_aaflData[iMaterial].push_back(v.x);
	g_aaflData[iMaterial].push_back(v.y);
	g_aaflData[iMaterial].push_back(v.z);
	g_aaflData[iMaterial].push_back(vn.x);
	g_aaflData[iMaterial].push_back(vn.y);
	g_aaflData[iMaterial].push_back(vn.z);
	g_aaflData[iMaterial].push_back(vt.x);
	g_aaflData[iMaterial].push_back(vt.y);

	for (int i = 0; i < 3; i++)
	{
		if (v[i] < g_aabbBounds.m_vecMins[i])
			g_aabbBounds.m_vecMins[i] = v[i];
		if (v[i] > g_aabbBounds.m_vecMaxs[i])
			g_aabbBounds.m_vecMaxs[i] = v[i];
	}
}

void AddWireframe(const Vector& v1, const Vector& vn1, const Vector& v2, const Vector& vn2)
{
	g_aflWireframeData.push_back(v1.x);
	g_aflWireframeData.push_back(v1.y);
	g_aflWireframeData.push_back(v1.z);
	g_aflWireframeData.push_back(vn1.x);
	g_aflWireframeData.push_back(vn1.y);
	g_aflWireframeData.push_back(vn1.z);
	g_aflWireframeData.push_back(v2.x);
	g_aflWireframeData.push_back(v2.y);
	g_aflWireframeData.push_back(v2.z);
	g_aflWireframeData.push_back(vn2.x);
	g_aflWireframeData.push_back(vn2.y);
	g_aflWireframeData.push_back(vn2.z);
}

void LoadMesh(CConversionScene* pScene, size_t iMesh)
{
	TAssert(iMesh < pScene->GetNumMeshes());
	if (iMesh >= pScene->GetNumMeshes())
		return;

	// Reserve space for n+1, the last one represents the default material.
	g_aaflData.resize(pScene->GetNumMaterials()+1);

	CConversionMesh* pMesh = pScene->GetMesh(iMesh);

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		size_t iMaterial = pFace->m;
		if (iMaterial == ~0)
			iMaterial = pScene->GetNumMaterials();

		CConversionVertex* pVertex0 = pFace->GetVertex(0);
		CConversionVertex* pVertex1 = pFace->GetVertex(1);
		AddWireframe(pMesh->GetVertex(pVertex0->v), pMesh->GetNormal(pVertex0->v), pMesh->GetVertex(pVertex1->v), pMesh->GetNormal(pVertex1->v));

		for (k = 2; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
			CConversionVertex* pVertex2 = pFace->GetVertex(k);

			AddVertex(iMaterial, pMesh->GetVertex(pVertex0->v), pMesh->GetNormal(pVertex0->v), pMesh->GetUV(pVertex0->vu));
			AddVertex(iMaterial, pMesh->GetVertex(pVertex1->v), pMesh->GetNormal(pVertex1->v), pMesh->GetUV(pVertex1->vu));
			AddVertex(iMaterial, pMesh->GetVertex(pVertex2->v), pMesh->GetNormal(pVertex2->v), pMesh->GetUV(pVertex2->vu));

			AddWireframe(pMesh->GetVertex(pVertex1->v), pMesh->GetNormal(pVertex1->v), pMesh->GetVertex(pVertex2->v), pMesh->GetNormal(pVertex2->v));
		}

		CConversionVertex* pLastVertex = pFace->GetVertex(pFace->GetNumVertices()-1);
		AddWireframe(pMesh->GetVertex(pLastVertex->v), pMesh->GetNormal(pLastVertex->v), pMesh->GetVertex(pVertex0->v), pMesh->GetNormal(pVertex0->v));
	}
}

bool CModel::Load(class CConversionScene* pScene, size_t iMesh)
{
	g_aaflData.clear();
	g_aflWireframeData.clear();
	g_aabbBounds = AABB(Vector(999, 999, 999), Vector(-999, -999, -999));

	LoadMesh(pScene, iMesh);

	size_t iMaterials = g_aaflData.size();

	m_asMaterialStubs.resize(iMaterials);
	m_aiVertexBuffers.resize(iMaterials);
	m_aiVertexBufferSizes.resize(iMaterials);

	m_iVertexWireframeBuffer = CRenderer::LoadVertexDataIntoGL(g_aflWireframeData.size()*4, &g_aflWireframeData[0]);
	m_iVertexWireframeBufferSize = g_aflWireframeData.size()/FloatsPerWireframeVertex();

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (g_aaflData[i].size() == 0)
			continue;

		m_aiVertexBuffers[i] = CRenderer::LoadVertexDataIntoGL(g_aaflData[i].size()*4, &g_aaflData[i][0]);
		m_aiVertexBufferSizes[i] = g_aaflData[i].size()/FloatsPerVertex();

		m_asMaterialStubs[i] = pScene->GetMesh(iMesh)->GetMaterialStub(i)->GetName();
	}

	m_aabbBoundingBox = g_aabbBounds;

	return true;
}