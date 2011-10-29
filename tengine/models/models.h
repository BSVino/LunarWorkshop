#ifndef DT_MODELS_H
#define DT_MODELS_H

#include <EASTL/vector.h>
#include <EASTL/string.h>

#include <modelconverter/convmesh.h>
#include <color.h>

typedef struct
{
	Vector		vecPosition;
	Vector		vecNormal;
	Vector2D	vecUV;
	Color		clrColor;
} Vertex_t;

class CModel
{
public:
							CModel(const tstring& sFilename);
							~CModel();

public:
	void					Load();
	void					LoadSceneIntoBuffer();
	void					LoadSceneNodeIntoBuffer(CConversionSceneNode* pNode, const Matrix4x4& mTransformations = Matrix4x4());
	void					LoadMeshInstanceIntoBuffer(CConversionMeshInstance* pMeshInstance, const Matrix4x4& mTransformations);
	size_t					LoadBufferIntoGL(size_t iMaterial);
	size_t					LoadTextureIntoGL(size_t iMaterial);

public:
	tstring					m_sFilename;
	CConversionScene*		m_pScene;

	eastl::vector< eastl::vector<Vertex_t> >	m_aaVertices;

	// Graphics library handles.
	eastl::vector<size_t>	m_aiTextures;
	eastl::vector<size_t>	m_aiVertexBuffers;
	eastl::vector<size_t>	m_aiVertexBufferSizes;	// How many vertices in this vertex buffer?

	AABB					m_aabbBoundingBox;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	static size_t			GetNumModels() { return Get()->m_apModels.size(); }

	size_t					AddModel(const tstring& sModel);
	size_t					FindModel(const tstring& sModel);
	CModel*					GetModel(size_t i);

public:
	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	eastl::vector<CModel*>	m_apModels;

private:
	static CModelLibrary*	s_pModelLibrary;
};

#endif