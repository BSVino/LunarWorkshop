#pragma once

#include <EASTL/vector.h>
#include <tstring.h>
#include <vector.h>
#include <geometry.h>

class CToy;

// For reading and writing .toy files
class CToyUtil
{
public:
					CToyUtil();

public:
	void			AddMaterial(const tstring& sTexture);
	size_t			GetNumMaterials() { return m_asTextures.size(); }
	void			AddVertex(size_t iMaterial, Vector vecPosition, Vector2D vecUV);
	size_t			GetNumVerts();

	void			AddPhysTriangle(size_t v1, size_t v2, size_t v3);
	void			AddPhysVertex(Vector vecPosition);
	size_t			GetNumPhysIndices() { return m_aiPhysIndices.size(); }

	bool			Write(const tstring& sFileName);

	static void		Read(const tstring& sFileName, CToy* pToy);

protected:
	eastl::vector<tstring>					m_asTextures;
	eastl::vector<eastl::vector<float> >	m_aaflData;
	AABB									m_aabbBounds;
	eastl::vector<size_t>					m_aiPhysIndices;
	eastl::vector<Vector>					m_avecPhysVerts;
};
