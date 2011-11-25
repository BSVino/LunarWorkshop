#include "toy_util.h"

#include <files.h>

#include "toy.h"

#include "toy_offsets.h"

CToyUtil::CToyUtil()
{
	m_aabbBounds.m_vecMins = Vector(999999, 999999, 999999);
	m_aabbBounds.m_vecMaxs = Vector(-999999, -999999, -999999);
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

	return iVerts;
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

bool CToyUtil::Write(const tstring& sFilename)
{
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

	fwrite(&m_aabbBounds, sizeof(m_aabbBounds), 1, fp);

	char iMaterials = m_asTextures.size();
	fwrite(&iMaterials, sizeof(iMaterials), 1, fp);

	size_t iFirstMaterial = TOY_HEADER_SIZE;
	size_t iSizeSoFar = iFirstMaterial;
	for (size_t i = 0; i < m_asTextures.size(); i++)
	{
		fwrite(&iSizeSoFar, sizeof(iSizeSoFar), 1, fp);

		int iVerts = (m_aaflData[i].size()/5);
		TAssert(sizeof(iVerts) == 4);
		fwrite(&iVerts, sizeof(iVerts), 1, fp);

		size_t iSize = 0;
		iSize += MESH_MATERIAL_TEXNAME_LENGTH_SIZE;
		iSize += m_asTextures[i].length()+1;
		iSize += (m_aaflData[i].size()/5)*MESH_MATERIAL_VERTEX_SIZE;

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
			short iLength = m_asTextures[i].length()+1;
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

		int iVerts = m_avecPhysVerts.size();
		fwrite(&iVerts, sizeof(iVerts), 1, fp);
		int iTris = m_aiPhysIndices.size()/3;
		fwrite(&iTris, sizeof(iTris), 1, fp);

		fwrite(m_avecPhysVerts.data(), m_avecPhysVerts.size()*sizeof(Vector), 1, fp);
		fwrite(m_aiPhysIndices.data(), m_aiPhysIndices.size()*sizeof(int), 1, fp);

		fclose(fp);
	}

	return true;
}

void CToyUtil::Read(const tstring& sFilename, CToy* pToy)
{
	FILE* fp = tfopen(sFilename, "r");

	TAssert(fp);
	if (!fp)
		return;

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
}
