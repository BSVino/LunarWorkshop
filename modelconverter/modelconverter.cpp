#include <stdio.h>
#include <string.h>

#include "modelconverter.h"
#include "strutils.h"

#ifdef WITH_ASSIMP
#include <assimp.h>
#endif

CModelConverter::CModelConverter(CConversionScene* pScene)
{
	m_pScene = pScene;
	m_pWorkListener = NULL;

	m_bWantEdges = true;
}

bool CModelConverter::ReadModel(const tstring& sFilename)
{
	tstring sExtension;

	size_t iFileLength = sFilename.length();
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.tolower();

	if (sExtension == ".obj")
		return ReadOBJ(sFilename);
	else if (sExtension == ".sia")
		return ReadSIA(sFilename);
	else if (sExtension == ".dae")
		return ReadDAE(sFilename);
	else
		return ReadAssImp(sFilename);
}

bool CModelConverter::SaveModel(const tstring& sFilename)
{
	tstring sExtension;

	size_t iFileLength = sFilename.length();
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.tolower();

	if (sExtension == ".obj")
		SaveOBJ(sFilename);
	else if (sExtension == ".sia")
		SaveSIA(sFilename);
	else if (sExtension == ".dae")
		SaveDAE(sFilename);
	else
		return false;

	return true;
}

// Takes a path + filename + extension and removes path and extension to return only the filename.
tstring CModelConverter::GetFilename(const tstring& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == '\\' || sFilename[i] == '/')
			iLastChar = i;

	tstring sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == '.')
			iLastChar = i;

	if (iLastChar >= 0)
		return sReturn.substr(0, iLastChar);

	return sReturn;
}

tstring CModelConverter::GetDirectory(const tstring& sFilename)
{
	int iLastSlash = -1;
	int i = -1;
	tstring sResult = sFilename;

	while (++i < (int)sResult.length())
		if (sResult[i] == '\\' || sResult[i] == '/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		sResult[iLastSlash] = '\0';
	else
		return ".";

	return sResult;
}

bool CModelConverter::IsWhitespace(tstring::value_type cChar)
{
	return (cChar == ' ' || cChar == '\t' || cChar == '\r' || cChar == '\n');
}

tstring CModelConverter::StripWhitespace(tstring sLine)
{
	int i = 0;
	while (IsWhitespace(sLine[i]) && sLine[i] != '\0')
		i++;

	int iEnd = ((int)sLine.length())-1;
	while (iEnd >= 0 && IsWhitespace(sLine[iEnd]))
		iEnd--;

	if (iEnd >= -1)
		sLine[iEnd+1] = '\0';

	return sLine.substr(i);
}

tvector<tstring> CModelConverter::GetReadFormats()
{
	tvector<tstring> asFormats;

#ifdef WITH_ASSIMP
	aiString sFormats;
	aiGetExtensionList(&sFormats);

	explode(sFormats.data, asFormats, ";");

	for (size_t i = 0; i < asFormats.size(); i++)	// Get rid of the * that assimp puts at the front
		asFormats[i] = asFormats[i].substr(1);
#else
	asFormats.push_back(".obj");
	asFormats.push_back(".dae");
#endif

	asFormats.push_back(".sia");

	return asFormats;
}