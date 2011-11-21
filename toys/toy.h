#pragma once

/*
	The Toy model format:

	First comes a header looking very much like PNG's ( http://en.wikipedia.org/wiki/Portable_Network_Graphics#Technical_details )

	0x89		- 1 byte with the high bit set
	'TOY'		- 3 bytes, marker
	'BASE'		- 4 bytes, or another four letter marker for what kind of data it is (BASE|MESH)
	0x0D 0x0A	- 2 bytes, dos style ending
	0x1A		- 1 byte, EOF character
	0x0A		- 1 byte, unix style ending

	Base data format - Information about the model

	4 bytes, AABB min x	(of the model in its default pose)
	4 bytes, AABB min y
	4 bytes, AABB min z
	4 bytes, AABB max x
	4 bytes, AABB max y
	4 bytes, AABB max z
	1 byte, number of materials
	∀ material:
		4 bytes, MESH file offset of material
		4 bytes, number of vertices (# of triangles * 3)

	Mesh data format - Stuff that can be freed once it's loaded into the engine. Vertex data, materials, triangles, uv's, normals, etc

	∀ material:
		2 bytes, texture name length (including null termination)
		x bytes, texture file name, null terminated
		∀ vertex:
			4 bytes, x position (float)
			4 bytes, y position (float)
			4 bytes, z position (float)
			4 bytes, u (float)
			4 bytes, v (float)

	Phys data format - Physics stuff that must remain in memory so the physics simulation can use it

	4 bytes, number of verts
	4 bytes, number of triangles
	∀ vertex:
		4 bytes, x position (float)
		4 bytes, y position (float)
		4 bytes, z position (float)
	∀ triangle:
		4 bytes, vertex 1 (int)
		4 bytes, vertex 2 (int)
		4 bytes, vertex 3 (int)
*/

#include <geometry.h>

class CToy
{
public:
				CToy();
				~CToy();

public:
	const AABB&	GetAABB();
	size_t		GetNumMaterials();
	size_t		GetMaterialTextureLength(size_t iMaterial);
	char*		GetMaterialTexture(size_t iMaterial);
	size_t		GetMaterialNumVerts(size_t iMaterial);
	float*		GetMaterialVerts(size_t iMaterial);
	float*		GetMaterialVert(size_t iMaterial, size_t iVert);

	size_t		GetVertexSize();
	size_t		GetVertexPosition();
	size_t		GetVertexUV();

	size_t		GetPhysicsNumVerts();
	size_t		GetPhysicsNumTris();
	float*		GetPhysicsVerts();
	float*		GetPhysicsVert(size_t iVert);
	int*		GetPhysicsTris();
	int*		GetPhysicsTri(size_t iTri);

protected:
	char*		GetMaterial(size_t i);

public:
	char*		AllocateBase(size_t iSize);
	char*		AllocateMesh(size_t iSize);
	void		DeallocateMesh();
	char*		AllocatePhys(size_t iSize);

protected:
	char*		m_pBase;
	char*		m_pMesh;
	char*		m_pPhys;
};
