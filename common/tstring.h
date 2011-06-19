#ifndef TINKER_STRING_H
#define TINKER_STRING_H

#include <EASTL/string.h>

typedef eastl::string16 tstring;
typedef tstring::value_type tchar;

#define _T EA_CHAR16

#include "strutils.h"
#include <string>

inline FILE* tfopen(const tstring& sFile, const tstring& sMode)
{
#ifdef _MSC_VER
	return _wfopen_s(sFile.c_str(), sMode.c_str());
#else
	return fopen(convertstring<tchar, char>(sFile).c_str(), convertstring<tchar, char>(sMode).c_str());
#endif
}

inline bool fgetts(tstring& str, FILE* fp)
{
#ifdef _MSC_VER
	static wchar_t szLine[1024];
	tchar* r = fgetws(szLine, 1023, fp);
	str = szLine;
	return r;
#else
	static char szLine[1024];
	char* r = fgets(szLine, 1023, fp);
	str = convertstring<char, tchar>(szLine);
	return r;
#endif
}

inline tchar* tstrncpy(tchar* d, const tchar* s, size_t n)
{
	return std::char_traits<tchar>::copy(d, s, n);
}

inline size_t tstrlen(const tchar* s)
{
	return std::char_traits<tchar>::length(s);
}

inline int tstrncmp(const tchar* s1, const tchar* s2, size_t n)
{
	return std::char_traits<tchar>::compare(s1, s2, n);
}

#endif
