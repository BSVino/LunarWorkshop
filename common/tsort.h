#pragma once

#ifdef WITH_EASTL
#include <EASTL/sort.h>

using eastl::sort;
#else
#include <algorithm>

using std::sort;
#endif
