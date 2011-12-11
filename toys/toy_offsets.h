#pragma once

#define TOY_HEADER_SIZE 12
#define BASE_AABB_SIZE (4*6)
#define BASE_MATERIAL_TABLE_SIZE 1
#define BASE_MATERIAL_TABLE_OFFSET_SIZE 4
#define BASE_MATERIAL_TABLE_VERTCOUNT_SIZE 4
#define BASE_MATERIAL_TABLE_STRIDE (BASE_MATERIAL_TABLE_OFFSET_SIZE+BASE_MATERIAL_TABLE_VERTCOUNT_SIZE)
#define BASE_SCENE_TABLE_SIZE 4
#define BASE_SCENE_TABLE_STRIDE 4

#define MESH_MATERIAL_TEXNAME_LENGTH_SIZE 2
#define MESH_MATERIAL_VERTEX_SIZE (4*5)

#define PHYS_VERTS_LENGTH_SIZE 4
#define PHYS_TRIS_LENGTH_SIZE 4
#define PHYS_VERT_SIZE sizeof(Vector)

#define AREA_AABB_SIZE (4*6)
#define AREA_VISIBLE_AREAS_SIZE 4
#define AREA_VISIBLE_AREAS_STRIDE 4
#define AREA_VISIBLE_AREA_NAME_SIZE 2
