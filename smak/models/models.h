#pragma once

#include <tvector.h>
#include <color.h>
#include <geometry.h>
#include <tstring.h>

#include <textures/materialhandle.h>

class CModel
{
public:
							CModel();
							~CModel();

public:
	bool					Load(class CConversionScene* pScene, size_t iMesh);
	size_t					LoadBufferIntoGL(size_t iMaterial);

	void					Unload();
	void					UnloadBufferFromGL(size_t iBuffer);

	size_t					FloatsPerVertex() { return 14; }
	size_t					Stride() { return FloatsPerVertex()*sizeof(float); }
	size_t					PositionOffset() { return 0; }
	size_t					NormalsOffset() { return 3*sizeof(float); }
	size_t					TangentsOffset() { return 6*sizeof(float); }
	size_t					BitangentsOffset() { return 9*sizeof(float); }
	size_t					TexCoordOffset() { return 12*sizeof(float); }

	size_t					FloatsPerWireframeVertex() { return 6; }
	size_t					WireframeStride() { return FloatsPerWireframeVertex()*sizeof(float); }
	size_t					WireframePositionOffset() { return 0; }
	size_t					WireframeNormalsOffset() { return 3*sizeof(float); }

	size_t					FloatsPerUVVertex() { return 2; }
	size_t					UVStride() { return FloatsPerUVVertex()*sizeof(float); }
	size_t					UVPositionOffset() { return 0; }

public:
	size_t					m_iReferences;

	tvector<tstring>		m_asMaterialStubs;
	tvector<size_t>			m_aiVertexBuffers;
	tvector<size_t>			m_aiVertexBufferSizes;	// How many vertices in this vertex buffer?
	size_t					m_iVertexWireframeBuffer;
	size_t					m_iVertexWireframeBufferSize;
	size_t					m_iVertexUVBuffer;
	size_t					m_iVertexUVBufferSize;

	AABB					m_aabbBoundingBox;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	static size_t			GetNumModelsLoaded() { return Get()->m_iModelsLoaded; }

	static void				AddModel(class CConversionScene* pScene, size_t iMesh);
	static CModel*			GetModel(size_t i);
	static void				ReleaseModel(size_t i);
	static void				UnloadModel(size_t i);

	static void				ResetReferenceCounts();
	static void				ClearUnreferenced();

	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	tvector<CModel*>		m_apModels;
	size_t					m_iModelsLoaded;

private:
	static CModelLibrary*	s_pModelLibrary;
};
