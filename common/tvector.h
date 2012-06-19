#pragma once

#include <EASTL/vector.h>

#include "common.h"

template <class T>
class tvector : public eastl::vector<T>
{
public:
        tvector();
};

template <class T>
tvector<T>::tvector()
	: eastl::vector<T>()
{
}
