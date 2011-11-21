#pragma once

#include "tstring.h"

// Takes a path + filename + extension and removes path and extension to return only the filename.
inline tstring GetFilename(const tstring& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == _T('\\') || sFilename[i] == _T('/'))
			iLastChar = i;

	tstring sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == _T('.'))
			iLastChar = i;

	if (iLastChar >= 0)
		return sReturn.substr(0, iLastChar);

	return sReturn;
}

// Takes a path + filename + extension and removes path to return only the filename and extension.
inline tstring GetFilenameAndExtension(const tstring& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == _T('\\') || sFilename[i] == _T('/'))
			iLastChar = i;

	tstring sReturn = sFilename.c_str() + iLastChar + 1;

	return sReturn;
}

inline tstring GetDirectory(const tstring& sFilename)
{
	int iLastSlash = -1;
	int i = -1;
	tstring sResult = sFilename;

	while (++i < (int)sResult.length())
		if (sResult[i] == _T('\\') || sResult[i] == _T('/'))
			iLastSlash = i;

	if (iLastSlash >= 0)
		return sResult.substr(0, iLastSlash);
	else
		return ".";
}

