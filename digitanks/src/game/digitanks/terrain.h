#ifndef DT_TERRAIN_H
#define DT_TERRAIN_H

#include <raytracer/raytracer.h>
#include "baseentity.h"

#define TERRAIN_SIZE 200
#define TERRAIN_GEN_SECTORS 4
#define TERRAIN_SECTOR_SIZE (TERRAIN_SIZE/TERRAIN_GEN_SECTORS)

class CTerrain : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTerrain, CBaseEntity);

public:
							CTerrain();
							~CTerrain();

public:
	virtual void			Spawn();

	virtual float			GetBoundingRadius() const { return sqrt(GetMapSize()*GetMapSize() + GetMapSize()*GetMapSize()); };

	void					GenerateTerrainCallLists();
	void					GenerateCallLists();

	virtual void			OnRender();

	float					GetRealHeight(int x, int y);
	float					GetHeight(float x, float y);
	Vector					SetPointHeight(Vector& vecPoint);
	float					GetMapSize() const;
	float					ArrayToWorldSpace(int i);
	int						WorldToArraySpace(float f);

	void					TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit);

	bool					Collide(const Ray& rayTrace, Vector& vecPoint);
	bool					Collide(const Vector& s1, const Vector& s2, Vector& vecPoint);

protected:
	float					m_aflHeights[TERRAIN_SIZE][TERRAIN_SIZE];

	float					m_flHighest;
	float					m_flLowest;

	size_t					m_iCallList;

	raytrace::CRaytracer*	m_pTracer;

	Vector					m_avecTerrainColors[4];

	std::vector<Vector>		m_avecCraterMarks;

	bool					m_abTerrainNeedsRegenerate[TERRAIN_GEN_SECTORS][TERRAIN_GEN_SECTORS];
};

#endif