if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(DevIL REQUIRED)
	include_directories(${IL_INCLUDE_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	.
	../
	../common
	../common/math
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/ftgl-2.1.3~rc5/src)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/freetype-2.3.11/include)
endif (WIN32)

set (SMAK_SOURCES
	main.cpp
	crunch/ao.cpp
	crunch/ao_raytrace.cpp
	crunch/crunch.cpp
	crunch/normal.cpp
	shaders/shaders.cpp
	ui/modelwindow.cpp
	ui/modelwindow_input.cpp
	ui/modelwindow_ui.cpp
	ui/picker.cpp
	ui/scenetree.cpp
)

if (WIN32)
	set (SMAK_SOURCES ${SMAK_SOURCES}
		smak.rc
	)
endif (WIN32)

add_executable(smak ${SMAK_SOURCES})
target_link_libraries (smak tinker modelconverter raytracer)

if (WIN32)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/ext-deps/glfw/support/msvc2010/Debug/GLFW.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/ext-deps/ftgl-2.1.3~rc5/msvc/build/ftgl_static_D.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT_D.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Debug/DevIL.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Debug/ILU.lib)
	target_link_libraries(smak debug ${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release/Debug/pthread.lib)

	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/ext-deps/glfw/support/msvc2010/Release/GLFW.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/ext-deps/ftgl-2.1.3~rc5/msvc/build/ftgl_static.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Release/DevIL.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Release/ILU.lib)
	target_link_libraries(smak optimized ${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release/Release/pthread.lib)
	target_link_libraries(smak optimized dbghelp.lib)

	target_link_libraries(smak opengl32 glu32 iphlpapi ws2_32)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (smak ${IL_LIBRARIES} ${ILU_LIBRARIES})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
