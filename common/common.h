#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

#ifdef __GNUC__

#include <csignal>

#define TDebugBreak() \
	::raise(SIGTRAP); \

#else

#define TDebugBreak() \
	__debugbreak(); \

#endif

#ifdef _DEBUG

#define TAssert(x) \
{ \
	if (!(x)) \
	{ \
		TMsg("Assert failed: " #x "\n"); \
		TDebugBreak(); \
	} \
} \

#define TAssertNoMsg(x) \
{ \
	if (!(x)) \
	{ \
		TDebugBreak(); \
	} \
} \

#else

#define TAssert(x) \
{ \
	if (!(x)) \
		TMsg("Assert failed: " #x "\n"); \
} \

#define TAssertNoMsg(x) \
{ \
} \

#endif

// If you hit this, the code is either incomplete or untested.
#define TUnimplemented() TAssert(false)

#if __GNUC__ < 4 || __GNUC_MINOR__ < 6

const                        // this is a const object...
class {
public:
  template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

#endif

#endif
