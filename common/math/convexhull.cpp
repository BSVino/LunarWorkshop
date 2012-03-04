#include "convexhull.h"

#include <common.h>
#include <tstring.h>
#include <geometry.h>

CConvexHullGenerator::CConvexHullGenerator(const eastl::vector<Vector>& avecPoints)
{
	m_avecPoints = avecPoints;

	// Reserve memory up front to avoid costly allocations
	m_aiTriangles.reserve(m_avecPoints.size());
}

void CConvexHullGenerator::CreateConvex()
{
	size_t i1 = FindLowestPoint();
	size_t i2 = FindNextPoint(i1, -1);

	AddEdge(i2, i1);

#if defined(_DEBUG) && defined(DEBUG_CONVEXNESS)
	Vector vecCenter;
	for (size_t i = 0; i < m_avecPoints.size(); i++)
		vecCenter += m_avecPoints[i];
	vecCenter /= (float)m_avecPoints.size();
#endif

	while (m_aOpenEdges.size())
	{
		i1 = m_aOpenEdges.back().p1;
		i2 = m_aOpenEdges.back().p2;
		m_aOpenEdges.pop_back();

		if (!EdgeExists(i1, i2))
		{
			size_t i3 = FindNextPoint(i1, i2);

#if defined(_DEBUG) && defined(DEBUG_CONVEXNESS)
			Vector v1 = m_avecPoints[i1];
			Vector v2 = m_avecPoints[i2];
			Vector v3 = m_avecPoints[i3];
			Vector n = (v2-v1).Cross(v3-v1).Normalized();
			TAssert(n.Dot(vecCenter-v1) < 0);
#endif

			m_aiTriangles.push_back(i1);
			m_aiTriangles.push_back(i2);
			m_aiTriangles.push_back(i3);

			AddEdge(i1, i2);
			AddEdge(i2, i3);
			AddEdge(i3, i1);
		}
	}
}

size_t CConvexHullGenerator::FindLowestPoint()
{
	size_t iIndex = 0;
	for (size_t i = 1; i < m_avecPoints.size(); i++)
	{
		if (m_avecPoints[i].z < m_avecPoints[iIndex].z)
			iIndex = i;
		else if (m_avecPoints[i].z == m_avecPoints[iIndex].z)
		{
			if (m_avecPoints[i].y < m_avecPoints[iIndex].y)
				iIndex = i;
			else if (m_avecPoints[i].x < m_avecPoints[iIndex].x)
				iIndex = i;

			else
				TAssertNoMsg(m_avecPoints[i].y != m_avecPoints[iIndex].y || m_avecPoints[i].x != m_avecPoints[iIndex].x);	// duplicate point
		}
	}

	return iIndex;
}

size_t CConvexHullGenerator::FindNextPoint(size_t p1, size_t p2)
{
	Vector v1 = m_avecPoints[p1];
	Vector v2;
	if (p2 == ~0)
		v2 = v1 - Vector(1, 0, 1);
	else
		v2 = m_avecPoints[p2];

	Vector vecEdge = (v2 - v1).Normalized();

	size_t iHighest = -1;
	Vector vecHighestNormal, vecHighest;

	for (size_t i = 0; i < m_avecPoints.size(); i++)
	{
		if (i == p1 || i == p2)
			continue;

		iHighest = i;
		vecHighest = m_avecPoints[iHighest];
		vecHighestNormal = (vecHighest - v1).Cross(vecEdge);
		break;
	}

	for (size_t i = 0; i < m_avecPoints.size(); i++)
	{
		Vector vecCandidate = m_avecPoints[i];

		float flDot = vecHighestNormal.Dot(vecCandidate - v1);

		if (flDot < -0.0001f)
		{
			iHighest = i;
			vecHighest = m_avecPoints[iHighest];
			vecHighestNormal = (vecHighest - v1).Cross(vecEdge);
		}
	}

	return iHighest;
}

const eastl::vector<size_t>& CConvexHullGenerator::GetConvexTriangles()
{
	if (!m_aiTriangles.size())
		CreateConvex();

	return m_aiTriangles;
}

bool CConvexHullGenerator::EdgeExists(size_t p1, size_t p2)
{
	return m_aaCreatedEdges[p1][p2];
}

void CConvexHullGenerator::AddEdge(size_t p1, size_t p2)
{
	m_aaCreatedEdges[p1][p2] = true;

	if (!EdgeExists(p2, p1))
		m_aOpenEdges.push_back(EdgePair(p2, p1));
}
