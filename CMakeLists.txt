cmake_minimum_required (VERSION 2.8)

set (CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_overrides.cmake)
set (CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_overrides.cmake)

include (${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils.cmake)

project (lw)

if (WIN32)
	set (CMAKE_EXE_LINKER_FLAGS_DEBUG				"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE				"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO		"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_DEBUG			"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL		"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_RELEASE			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO	"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_DEBUG			"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL		"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_RELEASE			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO	"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
endif (WIN32)

option(BUILD_SMAK "Build SMAK" OFF)

option(WITH_NETWORKING "Build with networking enabled." ON)

if (NOT WITH_NETWORKING)
	add_definitions(-DTINKER_NO_NETWORKING)
endif (NOT WITH_NETWORKING)

if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-std=c++0x)
endif (CMAKE_COMPILER_IS_GNUCC)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_CRT_NON_CONFORMING_SWPRINTFS)
	add_definitions(-DEA_CHAR16_NATIVE=1)
	add_definitions(-D_UNICODE)
	add_definitions(-DFREEGLUT_STATIC)
	add_definitions(-DFTGL_LIBRARY_STATIC)
endif (WIN32)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

add_definitions(-DTINKER_UNLOCKED)
add_definitions(-DTINKER_NO_REGISTRATION)

add_subdirectory (lwbase)

add_subdirectory (modelconverter)
add_subdirectory (raytracer)
add_subdirectory (toys)
add_subdirectory (tools)

add_subdirectory (tinker)
add_subdirectory (tengine)

#add_subdirectory (reflection)
add_subdirectory (grotto)

if (BUILD_CHAIN)
	add_subdirectory (chain)
endif ()

if (BUILD_SMAK)
	add_subdirectory (smak)
endif ()

add_subdirectory (tests)
