#ifndef TINKER_STRING_H
#define TINKER_STRING_H

#include <EASTL/string.h>

#include "common.h"

typedef eastl::string tstring;
typedef tstring::value_type tchar;

#include "strutils.h"
#include <string>

inline FILE* tfopen(const tstring& sFile, const tstring& sMode)
{
	tstring sBinaryMode = sMode;
	bool bHasB = false;
	for (size_t i = 0; i < sBinaryMode.length(); i++)
	{
		if (sMode[i] == 'b')
		{
			bHasB = true;
			break;
		}
	}

	// Open all files in binary mode to preserve unicodeness.
	if (!bHasB)
		sBinaryMode = sMode + "b";

	return fopen(sFile.c_str(), convertstring<tchar, char>(sBinaryMode).c_str());
}

inline bool fgetts(tstring& str, FILE* fp)
{
	static char szLine[1024];
	char* r = fgets(szLine, 1023, fp);

	if (!r)
		return false;

	str = szLine;
	return !!r;
}

inline tchar* tstrncpy(tchar* d, size_t d_size, const tchar* s, size_t n)
{
#ifdef _WIN32
	return std::char_traits<tchar>::_Copy_s(d, d_size, s, n);
#else
	if (d_size < n)
		n = d_size;

	return std::char_traits<tchar>::copy(d, s, n);
#endif
}

inline size_t tstrlen(const tchar* s)
{
	return std::char_traits<tchar>::length(s);
}

inline int tstrncmp(const tchar* s1, const tchar* s2, size_t n)
{
	return std::char_traits<tchar>::compare(s1, s2, n);
}

inline void TMsgStdOut(const tstring& sOut)
{
	puts(sOut.c_str());
}

inline void TErrorStdOut(const tstring& sOut)
{
	puts(("ERROR: " + sOut).c_str());
}

#endif
