#pragma once

#include <memory>

template<class C>
class CResource : public std::shared_ptr<C>
{
public:
	CResource(C* c)
		: std::shared_ptr<C>(c)
	{
	}
};

template<class C>
class CHandle : public std::weak_ptr<C>
{
public:
	CHandle()
		: std::weak_ptr<C>()
	{
	}

	CHandle(CResource<C>& r)
		: std::weak_ptr<C>(r)
	{
	}

public:
	C* operator->()
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	const C* operator->() const
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	operator C*() const
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	bool operator!() const
	{
		return expired();
	}
};
