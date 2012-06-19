#ifndef Lw_MODELCONVERTER_H
#define LW_MODELCONVERTER_H

#include <iostream>
#include <fstream>

#include <tvector.h>
#include <worklistener.h>
#include "convmesh.h"

class CModelConverter
{
public:
						CModelConverter(CConversionScene* pScene);

public:
	bool				ReadModel(const tstring& sFilename);

	bool				ReadOBJ(const tstring& sFilename);
	void				ReadMTL(const tstring& sFilename);

	// SIA and its utility functions.
	bool				ReadSIA(const tstring& sFilename);
	const tchar*		ReadSIAMat(const tchar* pszLine, const tchar* pszEnd, CConversionSceneNode* pScene, const tstring& sFilename);
	const tchar*		ReadSIAShape(const tchar* pszLine, const tchar* pszEnd, CConversionSceneNode* pScene, bool bCare = true);

	bool				ReadDAE(const tstring& sFilename);
	void				ReadDAESceneTree(class FCDSceneNode* pNode, CConversionSceneNode* pScene);

	bool				SaveModel(const tstring& sFilename);

	void				SaveOBJ(const tstring& sFilename);
	void				SaveSIA(const tstring& sFilename);
	void				SaveDAE(const tstring& sFilename);
	void				SaveDAEScene(class FCDSceneNode* pNode, CConversionSceneNode* pScene);

	void				WriteSMDs(const tstring& sFilename = "");
	void				WriteSMD(size_t iMesh, const tstring& sFilename = "");

	tstring				GetFilename(const tstring& sFilename);
	tstring				GetDirectory(const tstring& sFilename);
	bool				IsWhitespace(tstring::value_type cChar);
	tstring				StripWhitespace(tstring sLine);

	void				SetScene(CConversionScene* pScene) { m_pScene = pScene; };
	CConversionScene*	GetScene() { return m_pScene; };

	void				SetWorkListener(IWorkListener* pWorkListener) { m_pWorkListener = pWorkListener; }

	void				SetWantEdges(bool bWantEdges) { m_bWantEdges = bWantEdges; }

protected:
	CConversionScene*	m_pScene;

	IWorkListener*		m_pWorkListener;

	bool				m_bWantEdges;
};

#endif
