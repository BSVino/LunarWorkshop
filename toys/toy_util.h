#pragma once

#include <EASTL/vector.h>
#include <tstring.h>
#include <vector.h>
#include <geometry.h>

class CToy;

class CSceneArea
{
public:
	AABB					m_aabbArea;
	eastl::vector<uint32_t>	m_aiNeighboringAreas;
	eastl::vector<uint32_t>	m_aiVisibleAreas;
	tstring					m_sFileName;
};

// For reading and writing .toy files
class CToyUtil
{
public:
					CToyUtil();

public:
	tstring			GetGameDirectory() const { return m_sGameDirectory; }
	void			SetGameDirectory(const tstring& sGame) { m_sGameDirectory = sGame; }

	tstring			GetOutputDirectory() const { return m_sOutputDirectory; }
	void			SetOutputDirectory(const tstring& sOutput) { m_sOutputDirectory = sOutput; }

	tstring			GetOutputFile() const { return m_sOutputFile; }
	void			SetOutputFile(const tstring& sOutput) { m_sOutputFile = sOutput; }

	void			AddMaterial(const tstring& sTexture);
	size_t			GetNumMaterials() { return m_asTextures.size(); }
	void			AddVertex(size_t iMaterial, Vector vecPosition, Vector2D vecUV);
	size_t			GetNumVerts();

	void			AddPhysTriangle(size_t v1, size_t v2, size_t v3);
	void			AddPhysVertex(Vector vecPosition);
	size_t			GetNumPhysIndices() { return m_aiPhysIndices.size(); }
	size_t			GetNumPhysVerts() { return m_avecPhysVerts.size(); }

	size_t			AddSceneArea(const tstring& sFileName);
	void			AddSceneAreaNeighbor(size_t iSceneArea, size_t iNeighbor);
	void			AddSceneAreaVisible(size_t iSceneArea, size_t iVisible);
	bool			IsVisibleFrom(size_t iSceneArea, size_t iVisible);
	size_t			GetNumSceneAreas() { return m_asSceneAreas.size(); }

	AABB			GetBounds() { return m_aabbBounds; };

	void			SetNeighborDistance(float flDistance) { m_flNeighborDistance = flDistance; };
	void			UseGlobalTransformations(bool bGlobal = true) { m_bUseLocalTransforms = !bGlobal; };
	void			UseLocalTransformations(bool bLocal = true) { m_bUseLocalTransforms = bLocal; };
	bool			IsUsingLocalTransformations() const { return m_bUseLocalTransforms; };

	void			AddVisibleNeighbors(size_t iArea, size_t iVisible);
	void			CalculateVisibleAreas();
	bool			Write(const tstring& sFileName);

	static bool		Read(const tstring& sFileName, CToy* pToy);

protected:
	tstring					m_sGameDirectory;
	tstring					m_sOutputDirectory;
	tstring					m_sOutputFile;

	eastl::vector<tstring>					m_asTextures;
	eastl::vector<eastl::vector<float> >	m_aaflData;
	AABB									m_aabbBounds;
	eastl::vector<uint32_t>					m_aiPhysIndices;
	eastl::vector<Vector>					m_avecPhysVerts;
	eastl::vector<CSceneArea>				m_asSceneAreas;

	float					m_flNeighborDistance;	// How far can an area be considered a neighbor in the finding neighbors check?
	bool					m_bUseLocalTransforms;
};
