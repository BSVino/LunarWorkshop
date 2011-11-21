#ifndef DT_MODELS_H
#define DT_MODELS_H

#include <EASTL/vector.h>
#include <EASTL/string.h>

#include <color.h>
#include <geometry.h>
#include <tstring.h>

class CToy;

class CModel
{
public:
							CModel(const tstring& sFilename);
							~CModel();

public:
	void					Load();
	size_t					LoadBufferIntoGL(size_t iMaterial);

public:
	tstring					m_sFilename;
	CToy*					m_pToy;

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