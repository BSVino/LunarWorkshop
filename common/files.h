#pragma once

#include "tstring.h"
#include "tinker_platform.h"

// Takes a path + filename + extension and removes path and extension to return only the filename.
inline tstring GetFilename(const tstring& sFilename)
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

// Takes a path + filename + extension and removes path to return only the filename and extension.
inline tstring GetFilenameAndExtension(const tstring& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == '\\' || sFilename[i] == '/')
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
		if (sResult[i] == '\\' || sResult[i] == '/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		return sResult.substr(0, iLastSlash);
	else
		return ".";
}

inline tstring ToForwardSlashes(const tstring& sFilename)
{
	tstring sResult = sFilename;

	for (size_t i = 0; i < sResult.length(); i++)
		if (sResult[i] == '\\')
			sResult[i] = '/';

	return sResult;
}

inline tstring GetRelativePath(const tstring& sPath, const tstring& sFrom)
{
	tstring sAbsolutePath = FindAbsolutePath(sPath);
	tstring sAbsoluteFrom = FindAbsolutePath(sFrom);

	int iIdentical = 0;

	while ((int)sAbsolutePath.length() >= iIdentical && (int)sAbsoluteFrom.length() >= iIdentical && sAbsolutePath[iIdentical] == sAbsoluteFrom[iIdentical])
		iIdentical++;

	tstring sBasePath = sAbsolutePath.substr(iIdentical+1);
	tstring sBaseFrom = sAbsoluteFrom.substr(iIdentical);

	if (!sBaseFrom.length())
		return sBasePath;

	size_t iDirectories = 1;
	for (size_t i = 0; i < sBaseFrom.length(); i++)
	{
		if (sBaseFrom[i] == '/' || sBaseFrom[i] == '\\')
			iDirectories++;
	}

	tstring sResult;
	for (size_t i = 0; i < iDirectories; i++)
		sResult += "../";

	return sResult + sBasePath;
}

inline void CreateDirectory(const tstring& sPath)
{
	tstring sSubPath = sPath;

	if (IsDirectory(sPath))
		return;

	eastl::vector<tstring> asPaths;
	while (true)
	{
		sSubPath = GetDirectory(sSubPath);
		if (sSubPath == ".")
			break;

		asPaths.push_back(sSubPath);
	}

	for (size_t i = 0; i < asPaths.size(); i++)
	{
		if (IsDirectory(asPaths[asPaths.size()-i-1]))
			continue;

		CreateDirectoryNonRecursive(asPaths[asPaths.size()-i-1]);
	}

	CreateDirectoryNonRecursive(sPath);
}

inline bool IsAbsolutePath(const tstring& sPath)
{
	tstring sTrimmedPath = trim(sPath);

	tchar cFirst = sTrimmedPath[0];
	if (cFirst > 'A' && cFirst < 'Z' || cFirst > 'a' && cFirst < 'z')
	{
		if (sTrimmedPath[1] == ':')
		{
			if (sTrimmedPath[2] == '\\' || sTrimmedPath[2] == '/')
				return true;
		}
	}

	if (cFirst == '/')
		return true;

	if (cFirst == '\\')
		return true;

	if (cFirst == '~')
		return true;

	return false;
}
