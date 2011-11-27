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
	size_t					m_iReferences;

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

	static size_t			AddModel(const tstring& sModel);
	static size_t			FindModel(const tstring& sModel);
	static CModel*			GetModel(size_t i);
	static void				ReleaseModel(const tstring& sModel);

	static void				ResetReferenceCounts();
	static void				ClearUnreferenced();

	static void				LoadAllIntoPhysics();

	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	eastl::vector<CModel*>	m_apModels;

private:
	static CModelLibrary*	s_pModelLibrary;
};

#endif