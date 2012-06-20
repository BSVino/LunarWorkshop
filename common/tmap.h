#pragma once

#ifdef WITH_EASTL
#include <EASTL/map.h>

#define TMAP_BASE eastl::map<T, U>
#else
#include <map>

#define TMAP_BASE std::map<T, U>
#endif

template <class T, class U>
class tmap : public TMAP_BASE
{
public:
	inline tmap()
		: TMAP_BASE()
	{}
};
