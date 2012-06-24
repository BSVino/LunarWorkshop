#pragma once

#include <memory>

template<class C>
class CResource : public std::shared_ptr<C>
{
public:
	CResource()
		: std::shared_ptr<C>()
	{
	}

	CResource(C* c)
		: std::shared_ptr<C>(c)
	{
	}

	CResource(std::shared_ptr<C> c)
		: std::shared_ptr<C>(c)
	{
	}

public:
	// No run-time checking. Use only if you're sure about the type.
	template <class T>
	T* DowncastStatic() const
	{
		C* p = get();
		return static_cast<T*>(p);
	}

	operator C*() const
	{
		return get();
	}

	bool operator!() const
	{
		return !get();
	}

	CResource& operator=(CResource&& c)
	{
		if (&c == this)
			return *this;

		reset();
		swap(c);

		return *this;
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

	CHandle(const CResource<C>& r)
		: std::weak_ptr<C>(r)
	{
	}

public:
	using std::weak_ptr<C>::expired;
	using std::weak_ptr<C>::lock;

	template <class T>
	T* Downcast() const
	{
		if (expired())
			return nullptr;

		C* p = lock().get();
		return dynamic_cast<T*>(p);
	}

	// No run-time checking. Use only if you're sure about the type.
	template <class T>
	T* DowncastStatic() const
	{
		if (expired())
			return nullptr;

		C* p = lock().get();
		return static_cast<T*>(p);
	}

	C* Get() const
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

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
