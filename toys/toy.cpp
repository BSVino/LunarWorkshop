#include "toy.h"

#include <stdlib.h>
#include <stdio.h>

#include <common.h>

#include "toy_offsets.h"

CToy::CToy()
{
	m_pBase = nullptr;
	m_pMesh = nullptr;
	m_pPhys = nullptr;
}

CToy::~CToy()
{
	if (m_pBase)
		free(m_pBase);

	if (m_pMesh)
		free(m_pMesh);

	if (m_pPhys)
		free(m_pPhys);
}

const AABB& CToy::GetAABB()
{
	if (!m_pBase)
	{
		static AABB aabb;
		return aabb;
	}

	return *((AABB*)(m_pBase+TOY_HEADER_SIZE));
}

size_t CToy::GetNumMaterials()
{
	if (!m_pBase)
		return 0;

	return (int)*((char*)(m_pBase+TOY_HEADER_SIZE+BASE_AABB_SIZE));
}

size_t CToy::GetMaterialTextureLength(size_t i)
{
	return (int)*((short*)GetMaterial(i));
}

char* CToy::GetMaterialTexture(size_t i)
{
	return GetMaterial(i)+MESH_MATERIAL_TEXNAME_LENGTH_SIZE;
}

size_t CToy::GetMaterialNumVerts(size_t i)
{
	size_t iMaterialTableEntry = TOY_HEADER_SIZE+BASE_AABB_SIZE+BASE_MATERIAL_TABLE_SIZE+i*BASE_MATERIAL_TABLE_STRIDE;
	size_t iMaterialVertCount = (size_t)*((size_t*)(m_pBase+iMaterialTableEntry+BASE_MATERIAL_TABLE_OFFSET_SIZE));
	return iMaterialVertCount;
}

float* CToy::GetMaterialVerts(size_t iMaterial)
{
	char* pVerts = GetMaterial(iMaterial)+MESH_MATERIAL_TEXNAME_LENGTH_SIZE+GetMaterialTextureLength(iMaterial);
	return (float*)pVerts;
}

float* CToy::GetMaterialVert(size_t iMaterial, size_t iVert)
{
	char* pVerts = GetMaterial(iMaterial)+MESH_MATERIAL_TEXNAME_LENGTH_SIZE+GetMaterialTextureLength(iMaterial);
	return (float*)(pVerts + iVert*MESH_MATERIAL_VERTEX_SIZE);
}

size_t CToy::GetVertexSize()
{
	return MESH_MATERIAL_VERTEX_SIZE;
}

size_t CToy::GetVertexPosition()
{
	return 0;
}

size_t CToy::GetVertexUV()
{
	return 3*4;
}

size_t CToy::GetPhysicsNumVerts()
{
	if (!m_pPhys)
		return 0;

	return (int)*((int*)(m_pPhys+TOY_HEADER_SIZE));
}

size_t CToy::GetPhysicsNumTris()
{
	if (!m_pPhys)
		return 0;

	return (int)*((int*)(m_pPhys+TOY_HEADER_SIZE+PHYS_VERTS_LENGTH_SIZE));
}

float* CToy::GetPhysicsVerts()
{
	if (!m_pPhys)
		return nullptr;

	return (float*)(m_pPhys+TOY_HEADER_SIZE+PHYS_VERTS_LENGTH_SIZE+PHYS_TRIS_LENGTH_SIZE);
}

float* CToy::GetPhysicsVert(size_t iVert)
{
	if (!m_pPhys)
		return nullptr;

	return GetPhysicsVerts() + iVert*3;
}

int* CToy::GetPhysicsTris()
{
	if (!m_pPhys)
		return nullptr;

	size_t iVertsSize = GetPhysicsNumVerts()*PHYS_VERT_SIZE;
	return (int*)(m_pPhys+TOY_HEADER_SIZE+PHYS_VERTS_LENGTH_SIZE+PHYS_TRIS_LENGTH_SIZE+iVertsSize);
}

int* CToy::GetPhysicsTri(size_t iTri)
{
	if (!m_pPhys)
		return nullptr;

	return GetPhysicsTris() + iTri*3;
}

char* CToy::GetMaterial(size_t i)
{
	TAssert(m_pMesh);
	if (!m_pMesh)
		return nullptr;

	size_t iMaterialTableEntry = TOY_HEADER_SIZE+BASE_AABB_SIZE+BASE_MATERIAL_TABLE_SIZE+i*BASE_MATERIAL_TABLE_STRIDE;
	size_t iMaterialOffset = (size_t)*((size_t*)(m_pBase+iMaterialTableEntry));
	return m_pMesh+iMaterialOffset;
}

char* CToy::AllocateBase(size_t iSize)
{
	if (m_pBase)
		free(m_pBase);

	if (m_pMesh)
	{
		free(m_pMesh);
		m_pMesh = nullptr;
	}

	m_pBase = (char*)malloc(iSize);
	return m_pBase;
}

char* CToy::AllocateMesh(size_t iSize)
{
	if (m_pMesh)
		free(m_pMesh);

	m_pMesh = (char*)malloc(iSize);
	return m_pMesh;
}

void CToy::DeallocateMesh()
{
	if (m_pMesh)
		free(m_pMesh);

	m_pMesh = nullptr;
}

char* CToy::AllocatePhys(size_t iSize)
{
	if (m_pPhys)
		free(m_pPhys);

	m_pPhys = (char*)malloc(iSize);
	return m_pPhys;
}
