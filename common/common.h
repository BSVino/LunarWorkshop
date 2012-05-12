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

#endif
