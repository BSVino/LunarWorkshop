#pragma once

#ifdef WITH_EASTL
#include <EASTL/vector.h>

#define TVECTOR_BASE eastl::vector<T>
using eastl::remove;
using eastl::find;
#else
#include <vector>
#include <algorithm>

#define TVECTOR_BASE std::vector<T>
using std::remove;
using std::find;
#endif

template <class T>
class tvector : public TVECTOR_BASE
{
public:
	inline tvector()
		: TVECTOR_BASE()
	{}

public:
#ifndef WITH_EASTL
	using TVECTOR_BASE::push_back;

	T& push_back()
	{
		TVECTOR_BASE::push_back(T());
		return back();
	}

	void set_capacity(size_type n = ~0)
	{
		size_type iSize = size();
		if((n == ~0) || (n <= iSize))
		{
			if(n < iSize)
				resize(n);

			tvector temp(*this); 
			swap(temp);
		}
		else
			resize(n);
	}
#endif
};
