#pragma once

#include <EASTL/vector.h>

#include "common.h"

template <class T>
class tvector : public eastl::vector<T>
{
public:
        tvector();
        tvector(tvector<T>&& x);

public:
        tvector<T>& operator=(tvector<T>&& x);
};

template <class T>
tvector<T>::tvector()
	: eastl::vector<T>()
{
}

template <class T>
tvector<T>::tvector(tvector<T>&& x)
{
	this->mpBegin = x.mpBegin;
	this->mpEnd = x.mpEnd;
	this->mpCapacity = x.mpCapacity;
	this->mAllocator = x.mAllocator;

	x.mpBegin = nullptr;
	x.mpEnd = nullptr;
	x.mpCapacity = nullptr;
	x.mAllocator = nullptr;
}

template<class T>
tvector<T>& tvector<T>::operator=(tvector<T>&& x)
{
	if (x == *this)
		return *this;

	// Clear
	swap(tvector<T>());

	this->mpBegin = x.mpBegin;
	this->mpEnd = x.mpEnd;
	this->mpCapacity = x.mpCapacity;
	this->mAllocator = x.mAllocator;

	x.mpBegin = nullptr;
	x.mpEnd = nullptr;
	x.mpCapacity = nullptr;
	x.mAllocator = nullptr;

	return *this;
}
