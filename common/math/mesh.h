#pragma once

#include <tmap.h>
#include <tvector.h>
#include "vector.h"

class CConvexHullGenerator
{
public:
	CConvexHullGenerator(const tvector<Vector>& avecPoints);

public:
	const tvector<size_t>&	GetConvexTriangles();

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

	const tvector<Vector>&		m_avecPoints;
	tvector<size_t>				m_aiTriangles;

	tvector<EdgePair>			m_aOpenEdges;
	tmap<size_t, tmap<size_t, bool> >	m_aaCreatedEdges;
};

// Removes verts in a mesh which are coplanar
class CCoplanarPointOptimizer
{
public:
	// Does not remove the optimized verts. See CUnusedPointOptimizer
	static void						OptimizeMesh(const tvector<Vector>& avecPoints, tvector<size_t>& aiTriangles, float flTolerance = 0.001f);
};

// Removes unused points from a mesh, ie points that are not referred to by any triangle.
class CUnusedPointOptimizer
{
public:
	static void						OptimizeMesh(tvector<Vector>& avecPoints, tvector<size_t>& aiTriangles);
};
