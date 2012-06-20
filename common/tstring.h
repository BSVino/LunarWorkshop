#ifndef TINKER_STRING_H
#define TINKER_STRING_H

#ifdef WITH_EASTL
#include <EASTL/string.h>

#define TSTRING_BASE eastl::basic_string<char>
#else
#include <string>

#define TSTRING_BASE std::basic_string<char>
#endif

#include "common.h"

class tstring : public TSTRING_BASE
{
public:
	tstring()
		: TSTRING_BASE()
	{}

	tstring(const char* s)
		: TSTRING_BASE(s)
	{}

	tstring(const TSTRING_BASE& s)
		: TSTRING_BASE(s)
	{}

	tstring(TSTRING_BASE&& s)
		: TSTRING_BASE(s)
	{}

	tstring(size_type n, value_type c)
		: TSTRING_BASE(n, c)
	{}

	tstring(const value_type* b, const value_type* e)
		: TSTRING_BASE(b, e)
	{}

public:
	using TSTRING_BASE::replace;

	inline tstring&	replace(const tstring& f, const tstring& r)
	{
		size_t iPosition;
		while ((iPosition = find(f)) != tstring::npos)
			assign(substr(0, iPosition) + r + (c_str()+iPosition+f.length()));

		return *this;
	}

	inline tstring&	replace(const char* f, const char* r)
	{
		size_t iPosition;
		while ((iPosition = find(f)) != tstring::npos)
			assign(substr(0, iPosition) + r + (c_str()+iPosition+strlen(f)));

		return *this;
	}

	inline tstring&	tolower()
	{
		for (size_t i = 0; i < length(); i++)
			(*this)[i] = ::tolower((*this)[i]);

		return *this;
	}

	inline tstring&	toupper()
	{
		for (size_t i = 0; i < length(); i++)
			(*this)[i] = ::toupper((*this)[i]);

		return *this;
	}

	inline bool startswith(const tstring& sBeginning) const
	{
		if (length() < sBeginning.length())
			return false;

		return (substr(0, sBeginning.length()) == sBeginning);
	}

	inline bool endswith(const tstring& sEnding) const
	{
		if (length() < sEnding.length())
			return false;

		return (substr(length() - sEnding.length()) == sEnding);
	}

	inline bool endswith(const char* sEnding) const
	{
		size_t iEndLength = strlen(sEnding);
		if (length() < iEndLength)
			return false;

		return (substr(length() - iEndLength) == sEnding);
	}

#ifndef WITH_EASTL
	int comparei(const tstring& sOther) const
    {
		tstring s1, s2;
		s1 = *this;
		s2 = sOther;
		return s1.tolower().compare(s2.tolower());
    }
#endif
};

typedef tstring::value_type tchar;

#include "strutils.h"

#endif
