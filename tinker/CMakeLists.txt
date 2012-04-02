if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(Freetype REQUIRED)
	include_directories(${FREETYPE_INCLUDE_DIRS})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}/
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tinker
)

set (TINKER_GLGUI_SOURCES
	../glgui/glgui.cpp
	../glgui/basecontrol.cpp
	../glgui/button.cpp
	../glgui/checkbox.cpp
	../glgui/droppablepanel.cpp
	../glgui/filedialog.cpp
	../glgui/label.cpp
	../glgui/menu.cpp
	../glgui/movablepanel.cpp
	../glgui/panel.cpp
	../glgui/picturebutton.cpp
	../glgui/rootpanel.cpp
	../glgui/scrollbar.cpp
	../glgui/slidingpanel.cpp
	../glgui/tree.cpp
	../glgui/textfield.cpp
)

set (TINKER_RENDERER_SOURCES
	renderer/renderer.cpp
	renderer/renderingcontext.cpp
	renderer/shaders.cpp
)

set (TINKER_TEXTURES_SOURCES
	textures/texturelibrary.cpp
	textures/texturesheet.cpp
)

set (TINKER_SOURCES
	shell.cpp
	application.cpp
	console.cpp
	cvar.cpp
	portals/portal_stubs.cpp
	register.cpp
	profiler.cpp

	${CMAKE_SOURCE_DIR}/../ext-deps/gl3w/src/gl3w.c

	${TINKER_GLGUI_SOURCES}
	${TINKER_RENDERER_SOURCES}
	${TINKER_TEXTURES_SOURCES}
)

source_group(GLGUI FILES ${TINKER_GLGUI_SOURCES})
source_group(Renderer FILES ${TINKER_RENDERER_SOURCES})
source_group(Textures FILES ${TINKER_TEXTURES_SOURCES})

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/gl3w/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/glfw-1.3/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/ftgl-gl3/src)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/freetype-2.3.11/include)
endif (WIN32)

add_library(tinker STATIC ${TINKER_SOURCES})
target_link_libraries(tinker lwbase)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECTUABLE} -static-libgcc")
	target_link_libraries (tinker /usr/lib/libc.a)
	target_link_libraries (tinker /usr/lib/libftgl.a)
	target_link_libraries (tinker /usr/lib/libfreetype.a)
	target_link_libraries (tinker /usr/lib/libglfw.a)
	target_link_libraries (tinker X11 Xrandr GL GLU)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (WIN32)
	target_link_libraries(tinker debug ${CMAKE_SOURCE_DIR}/../ext-deps/ftgl-gl3/msvc/build/ftgl_static_D.lib)
	target_link_libraries(tinker optimized ${CMAKE_SOURCE_DIR}/../ext-deps/ftgl-gl3/msvc/build/ftgl_static.lib)
	target_link_libraries(tinker debug ${CMAKE_SOURCE_DIR}/../ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT_D.lib)
	target_link_libraries(tinker optimized ${CMAKE_SOURCE_DIR}/../ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT.lib)
	target_link_libraries(tinker debug ${CMAKE_SOURCE_DIR}/../ext-deps/glfw-1.3/src/Debug/GLFW.lib)
	target_link_libraries(tinker optimized ${CMAKE_SOURCE_DIR}/../ext-deps/glfw-1.3/src/RelWithDebInfo/GLFW.lib)

	target_link_libraries(tinker opengl32 glu32)
endif (WIN32)
