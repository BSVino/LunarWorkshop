#include "toy_util.h"

#include <common.h>
#include <files.h>
#include <tinker_platform.h>
#include <convexhull.h>

#include "toy.h"

#include "toy_offsets.h"

CToyUtil::CToyUtil()
{
	m_aabbBounds.m_vecMins = Vector(999999, 999999, 999999);
	m_aabbBounds.m_vecMaxs = Vector(-999999, -999999, -999999);

	m_flNeighborDistance = 1;
}

void CToyUtil::AddMaterial(const tstring& sTexture)
{
	m_asTextures.push_back(GetFilenameAndExtension(sTexture));
	m_aaflData.push_back();
}

void CToyUtil::AddVertex(size_t iMaterial, Vector vecPosition, Vector2D vecUV)
{
	m_aaflData[iMaterial].push_back(vecPosition.x);
	m_aaflData[iMaterial].push_back(vecPosition.y);
	m_aaflData[iMaterial].push_back(vecPosition.z);
	m_aaflData[iMaterial].push_back(vecUV.x);
	m_aaflData[iMaterial].push_back(vecUV.y);

	for (int i = 0; i < 3; i++)
	{
		if (vecPosition[i] < m_aabbBounds.m_vecMins[i])
			m_aabbBounds.m_vecMins[i] = vecPosition[i];
		if (vecPosition[i] > m_aabbBounds.m_vecMaxs[i])
			m_aabbBounds.m_vecMaxs[i] = vecPosition[i];
	}
}

size_t CToyUtil::GetNumVerts()
{
	size_t iVerts = 0;

	for (size_t i = 0; i < m_aaflData.size(); i++)
		iVerts += m_aaflData[i].size();

	return iVerts/(MESH_MATERIAL_VERTEX_SIZE/sizeof(float));
}

void CToyUtil::AddPhysTriangle(size_t v1, size_t v2, size_t v3)
{
	m_aiPhysIndices.push_back(v1);
	m_aiPhysIndices.push_back(v2);
	m_aiPhysIndices.push_back(v3);
}

void CToyUtil::AddPhysVertex(Vector vecPosition)
{
	m_avecPhysVerts.push_back(vecPosition);
}

size_t CToyUtil::AddSceneArea(const tstring& sFileName)
{
	CToy* pArea = new CToy();
	bool bRead = CToyUtil::Read(GetGameDirectory() + DIR_SEP + sFileName, pArea);

	if (!bRead)
	{
		delete pArea;

		TAssert(bRead);
		TError("Couldn't find scene area " + sFileName + "\n");

		return ~0;
	}

	CSceneArea& oSceneArea = m_asSceneAreas.push_back();

	oSceneArea.m_sFileName = sFileName;
	oSceneArea.m_aabbArea = pArea->GetAABB();

	// I'm visible to myself.
	oSceneArea.m_aiNeighboringAreas.push_back(m_asSceneAreas.size()-1);

	delete pArea;

	return m_asSceneAreas.size()-1;
}

void CToyUtil::AddSceneAreaNeighbor(size_t iSceneArea, size_t iNeighbor)
{
	TAssert(iSceneArea < m_asSceneAreas.size());
	if (iSceneArea >= m_asSceneAreas.size())
		return;

	TAssert(iNeighbor < m_asSceneAreas.size());
	if (iNeighbor >= m_asSceneAreas.size())
		return;

	for (size_t i = 0; i < m_asSceneAreas[iSceneArea].m_aiNeighboringAreas.size(); i++)
	{
		if (m_asSceneAreas[iSceneArea].m_aiNeighboringAreas[i] == iNeighbor)
			return;
	}

	m_asSceneAreas[iSceneArea].m_aiNeighboringAreas.push_back(iNeighbor);
}

void CToyUtil::AddSceneAreaVisible(size_t iSceneArea, size_t iVisible)
{
	TAssert(iSceneArea < m_asSceneAreas.size());
	if (iSceneArea >= m_asSceneAreas.size())
		return;

	TAssert(iVisible < m_asSceneAreas.size());
	if (iVisible >= m_asSceneAreas.size())
		return;

	if (IsVisibleFrom(iSceneArea, iVisible))
		return;

	m_asSceneAreas[iSceneArea].m_aiVisibleAreas.push_back(iVisible);
}

bool CToyUtil::IsVisibleFrom(size_t iSceneArea, size_t iVisible)
{
	for (size_t i = 0; i < m_asSceneAreas[iSceneArea].m_aiVisibleAreas.size(); i++)
	{
		if (m_asSceneAreas[iSceneArea].m_aiVisibleAreas[i] == iVisible)
			return true;
	}

	return false;
}

void CToyUtil::CalculateVisibleAreas()
{
	AABB aabbExpand(Vector(-m_flNeighborDistance, -m_flNeighborDistance, -m_flNeighborDistance)/2, Vector(m_flNeighborDistance, m_flNeighborDistance, m_flNeighborDistance)/2);

	// First auto-detect neighbors. Naive O(n(n-1)/2) distance check.
	for (size_t i = 0; i < m_asSceneAreas.size(); i++)
	{
		// We can skip j <= i since we add neighbors reciprocally
		for (size_t j = i+1; j < m_asSceneAreas.size(); j++)
		{
			// Instead of finding the actual distance, just expand each box by m_flNeighborDistance/2 in every direction and test intersection. It's easier!
			AABB aabbBounds1 = m_asSceneAreas[i].m_aabbArea + aabbExpand;
			AABB aabbBounds2 = m_asSceneAreas[j].m_aabbArea + aabbExpand;

			if (aabbBounds1.Intersects(aabbBounds2))
			{
				AddSceneAreaNeighbor(i, j);
				AddSceneAreaNeighbor(j, i);
				continue;
			}
		}
	}

	for (size_t i = 0; i < m_asSceneAreas.size(); i++)
	{
		for (size_t j = 0; j < m_asSceneAreas[i].m_aiNeighboringAreas.size(); j++)
		{
			size_t iNeighbor = m_asSceneAreas[i].m_aiNeighboringAreas[j];

			// I can always see my neighbors
			AddSceneAreaVisible(i, iNeighbor);
			AddSceneAreaVisible(iNeighbor, i);

			AddVisibleNeighbors(i, iNeighbor);
		}
	}
}

// Add any neighbors of iVisible which are visible to iArea's visible set.
void CToyUtil::AddVisibleNeighbors(size_t iArea, size_t iVisible)
{
	if (iArea == iVisible)
		return;

	eastl::vector<Vector> avecPoints;

	for (size_t i = 0; i < m_asSceneAreas[iVisible].m_aiNeighboringAreas.size(); i++)
	{
		size_t iOther = m_asSceneAreas[iVisible].m_aiNeighboringAreas[i];

		// If this area is already visible, we can skip it to prevent extra work and recursion.
		if (IsVisibleFrom(iArea, iOther))
			continue;

		// Form a convex hull from the bounding boxes of iArea and i
		avecPoints.clear();

		Vector vecMins = m_asSceneAreas[iArea].m_aabbArea.m_vecMins;
		Vector vecMaxs = m_asSceneAreas[iArea].m_aabbArea.m_vecMaxs;
		avecPoints.push_back(vecMins);
		avecPoints.push_back(Vector(vecMins.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(vecMaxs);

		vecMins = m_asSceneAreas[iOther].m_aabbArea.m_vecMins;
		vecMaxs = m_asSceneAreas[iOther].m_aabbArea.m_vecMaxs;
		avecPoints.push_back(vecMins);
		avecPoints.push_back(Vector(vecMins.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(vecMaxs);

		CConvexHullGenerator c(avecPoints);

		const eastl::vector<size_t>& avecConvexTriangles = c.GetConvexTriangles();

		// Test to see if iVisible intersects that hull
		AABB aabbShrunkBounds = m_asSceneAreas[iVisible].m_aabbArea + AABB(Vector(0.1f, 0.1f, 0.1f), Vector(-0.1f, -0.1f, -0.1f));	// Shrink the bounds a tad so touching bounds on the far side don't count.
		bool bIntersect = ConvexHullIntersectsAABB(aabbShrunkBounds, avecPoints, avecConvexTriangles);

		if (bIntersect)
		{
			AddSceneAreaVisible(iArea, iOther);
			AddSceneAreaVisible(iOther, iArea);

			// Calling it recursively this way may allow visibility that doesn't exist, eg, in a chain a -> b -> c -> d, a can see d through c but not through b.
			// I'm willing to accept that for now if it doesn't become a problem.
			AddVisibleNeighbors(iArea, iOther);
		}
	}
}

const unsigned char g_szBaseHeader[] =
{
	'\x89',
	'T',
	'O',
	'Y',
	'B',
	'A',
	'S',
	'E',
	'\x0D',
	'\x0A',
	'\x1A',
	'\x0A',
};

const unsigned char g_szMeshHeader[] =
{
	'\x89',
	'T',
	'O',
	'Y',
	'M',
	'E',
	'S',
	'H',
	'\x0D',
	'\x0A',
	'\x1A',
	'\x0A',
};

const unsigned char g_szPhysHeader[] =
{
	'\x89',
	'T',
	'O',
	'Y',
	'P',
	'H',
	'Y',
	'S',
	'\x0D',
	'\x0A',
	'\x1A',
	'\x0A',
};

const unsigned char g_szAreaHeader[] =
{
	'\x89',
	'T',
	'O',
	'Y',
	'A',
	'R',
	'E',
	'A',
	'\x0D',
	'\x0A',
	'\x1A',
	'\x0A',
};

bool CToyUtil::Write(const tstring& sFilename)
{
	CalculateVisibleAreas();

	for (size_t i = m_asTextures.size()-1; i < m_asTextures.size(); i--)
	{
		// Must have at least one vertex or you get the boot.
		if (!m_aaflData[i].size())
		{
			m_aaflData.erase(m_aaflData.begin()+i);
			m_asTextures.erase(m_asTextures.begin()+i);
		}
	}

	FILE* fp = tfopen(sFilename, "w");

	TAssert(fp);
	if (!fp)
		return false;

	fwrite(g_szBaseHeader, sizeof(g_szBaseHeader), 1, fp);

	TAssert(sizeof(m_aabbBounds) == 4*6);
	fwrite(&m_aabbBounds, sizeof(m_aabbBounds), 1, fp);

	uint8_t iMaterials = m_asTextures.size();
	fwrite(&iMaterials, sizeof(iMaterials), 1, fp);

	uint32_t iFirstMaterial = TOY_HEADER_SIZE;
	uint32_t iSizeSoFar = iFirstMaterial;
	for (size_t i = 0; i < m_asTextures.size(); i++)
	{
		fwrite(&iSizeSoFar, sizeof(iSizeSoFar), 1, fp);

		uint32_t iVerts = (m_aaflData[i].size()/5);
		fwrite(&iVerts, sizeof(iVerts), 1, fp);

		uint32_t iSize = 0;
		iSize += MESH_MATERIAL_TEXNAME_LENGTH_SIZE;
		iSize += m_asTextures[i].length()+1;
		iSize += (m_aaflData[i].size()/5)*MESH_MATERIAL_VERTEX_SIZE;

		iSizeSoFar += iSize;
	}

	uint32_t iSceneAreas = m_asSceneAreas.size();
	fwrite(&iSceneAreas, sizeof(iSceneAreas), 1, fp);

	uint32_t iFirstScene = TOY_HEADER_SIZE;
	iSizeSoFar = iFirstScene;
	for (size_t i = 0; i < m_asSceneAreas.size(); i++)
	{
		fwrite(&iSizeSoFar, sizeof(iSizeSoFar), 1, fp);

		size_t iSize = 0;
		iSize += AREA_AABB_SIZE;
		iSize += AREA_VISIBLE_AREAS_SIZE;
		iSize += m_asSceneAreas[i].m_aiVisibleAreas.size()*AREA_VISIBLE_AREAS_STRIDE;
		iSize += AREA_VISIBLE_AREA_NAME_SIZE;
		iSize += m_asSceneAreas[i].m_sFileName.length()+1;

		iSizeSoFar += iSize;
	}

	fclose(fp);

	if (m_aaflData.size())
	{
		fp = tfopen(sFilename.substr(0, sFilename.length()-4) + ".mesh.toy", "w");

		TAssert(fp);
		if (!fp)
			return false;

		fwrite(g_szMeshHeader, sizeof(g_szMeshHeader), 1, fp);

		iMaterials = m_asTextures.size();

		for (size_t i = 0; i < m_asTextures.size(); i++)
		{
			uint16_t iLength = m_asTextures[i].length()+1;
			fwrite(&iLength, sizeof(iLength), 1, fp);
			fwrite(m_asTextures[i].c_str(), iLength, 1, fp);

			fwrite(m_aaflData[i].data(), m_aaflData[i].size()*sizeof(float), 1, fp);
		}

		fclose(fp);
	}

	if (m_avecPhysVerts.size())
	{
		fp = tfopen(sFilename.substr(0, sFilename.length()-4) + ".phys.toy", "w");

		TAssert(fp);
		if (!fp)
			return false;

		fwrite(g_szPhysHeader, sizeof(g_szPhysHeader), 1, fp);

		uint32_t iVerts = m_avecPhysVerts.size();
		fwrite(&iVerts, sizeof(iVerts), 1, fp);
		uint32_t iTris = m_aiPhysIndices.size()/3;
		fwrite(&iTris, sizeof(iTris), 1, fp);

		fwrite(m_avecPhysVerts.data(), m_avecPhysVerts.size()*sizeof(Vector), 1, fp);
		fwrite(m_aiPhysIndices.data(), m_aiPhysIndices.size()*sizeof(uint32_t), 1, fp);

		fclose(fp);
	}

	if (m_asSceneAreas.size())
	{
		fp = tfopen(sFilename.substr(0, sFilename.length()-4) + ".area.toy", "w");

		TAssert(fp);
		if (!fp)
			return false;

		fwrite(g_szAreaHeader, sizeof(g_szAreaHeader), 1, fp);

		for (size_t i = 0; i < m_asSceneAreas.size(); i++)
		{
			fwrite(&m_asSceneAreas[i].m_aabbArea, sizeof(m_asSceneAreas[i].m_aabbArea), 1, fp);

			uint32_t iAreasVisible = m_asSceneAreas[i].m_aiVisibleAreas.size();
			fwrite(&iAreasVisible, sizeof(iAreasVisible), 1, fp);

			fwrite(m_asSceneAreas[i].m_aiVisibleAreas.data(), m_asSceneAreas[i].m_aiVisibleAreas.size()*sizeof(uint32_t), 1, fp);

			uint16_t iFileLength = m_asSceneAreas[i].m_sFileName.length()+1;
			fwrite(&iFileLength, sizeof(iFileLength), 1, fp);
			fwrite(m_asSceneAreas[i].m_sFileName.c_str(), iFileLength, 1, fp);
		}

		fclose(fp);
	}

	return true;
}

bool CToyUtil::Read(const tstring& sFilename, CToy* pToy)
{
	FILE* fp = tfopen(sFilename, "r");

	TAssert(fp);
	if (!fp)
		return false;

	fseek(fp, 0L, SEEK_END);
	long iToySize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char* pBuffer = pToy->AllocateBase(iToySize);

	fread(pBuffer, iToySize, 1, fp);

	TAssert(memcmp(pBuffer, g_szBaseHeader, sizeof(g_szBaseHeader)) == 0);

	fclose(fp);

	if (pToy->GetNumMaterials())
	{
		fp = tfopen(sFilename.substr(0, sFilename.length()-4) + ".mesh.toy", "r");

		fseek(fp, 0L, SEEK_END);
		iToySize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		char* pBuffer = pToy->AllocateMesh(iToySize);

		fread(pBuffer, iToySize, 1, fp);

		TAssert(memcmp(pBuffer, g_szMeshHeader, sizeof(g_szMeshHeader)) == 0);

		fclose(fp);
	}

	fp = tfopen(sFilename.substr(0, sFilename.length()-4) + ".phys.toy", "r");
	if (fp)
	{
		fseek(fp, 0L, SEEK_END);
		iToySize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		char* pBuffer = pToy->AllocatePhys(iToySize);

		fread(pBuffer, iToySize, 1, fp);

		TAssert(memcmp(pBuffer, g_szPhysHeader, sizeof(g_szPhysHeader)) == 0);

		fclose(fp);
	}

	if (pToy->GetNumSceneAreas())
	{
		fp = tfopen(sFilename.substr(0, sFilename.length()-4) + ".area.toy", "r");

		fseek(fp, 0L, SEEK_END);
		iToySize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		char* pBuffer = pToy->AllocateArea(iToySize);

		fread(pBuffer, iToySize, 1, fp);

		TAssert(memcmp(pBuffer, g_szAreaHeader, sizeof(g_szAreaHeader)) == 0);

		fclose(fp);
	}

	return true;
}
