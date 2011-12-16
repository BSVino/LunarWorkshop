#pragma once

#include <EASTL/vector.h>
#include <EASTL/map.h>

#include "vector.h"

class CConvexHullGenerator
{
public:
	CConvexHullGenerator(const eastl::vector<Vector>& avecPoints);

public:
	const eastl::vector<size_t>&	GetConvexTriangles();

protected:
	void			CreateConvex();
	size_t			FindLowestPoint();
	size_t			FindNextPoint(size_t p1, size_t p2);

	bool			EdgeExists(size_t p1, size_t p2);
	void			AddEdge(size_t p1, size_t p2);

protected:
	struct EdgePair
	{
		EdgePair(size_t P1, size_t P2)
		{
			p1 = P1;
			p2 = P2;
		}

		size_t p1;
		size_t p2;
	};

	eastl::vector<Vector>		m_avecPoints;
	eastl::vector<size_t>		m_aiTriangles;

	eastl::vector<EdgePair>		m_aOpenEdges;
	eastl::map<size_t, eastl::map<size_t, bool> >	m_aaCreatedEdges;
};